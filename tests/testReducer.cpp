#include "TestSuite.hpp"
#include "expert_system/ast/AtomicNode.hpp"
#include "expert_system/ast/NotNode.hpp"
#include "expert_system/ast/OpNode.hpp"
#include "expert_system/exceptions/Exceptions.hpp"
#include "expert_system/parser/Lexer.hpp"
#include "expert_system/parser/Parser.hpp"
#include "expert_system/parser/Reducer.hpp"
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Serializes an AST to a fully parenthesized string so its shape can
 * be compared, e.g A + B => C gives ((A + B) => C) and !!A gives !!A.
 */
static std::string toString(const IASTNode *node) {
    if (!node)
        return "<null>";
    if (auto atomic = dynamic_cast<const AtomicNode *>(node))
        return atomic->getSymbol();
    if (auto notNode = dynamic_cast<const NotNode *>(node))
        return notNode->getSymbol() + toString(notNode->getRightNode());
    if (auto opNode = dynamic_cast<const OpNode *>(node))
        return "(" + toString(opNode->getLeftNode()) + " " + opNode->getSymbol()
             + " " + toString(opNode->getRightNode()) + ")";
    return "<unknown>";
}

/**
 * @brief Lexes, parses and reduces a line.
 */
static std::unique_ptr<IASTNode> reduce(const std::string &line) {
    Lexer lexer(line);
    std::vector<Token> tokens = lexer.tokenize();
    return Reducer::reduce(Parser::parseTokens(tokens));
}

struct Case {
    std::string line;
    std::string expected;
};

static void runCases(TestSuite &suite, const std::string &label, const std::vector<Case> &cases) {
    for (const auto &[line, expected] : cases) {
        try {
            auto node = reduce(line);
            suite.expectEqual(label + ": " + line, expected, toString(node.get()));
        } catch (const std::exception &error) {
            suite.check(label + ": " + line, false, std::string("unexpected exception: ") + error.what());
        }
    }
}

static void testNothingToReduce(TestSuite &suite) {
    runCases(suite, "unchanged", {
        {"A", "A"},
        {"!A", "!A"},
        {"A => B", "(A => B)"},
        {"A + B => C", "((A + B) => C)"},
        {"!A + B => !C", "((!A + B) => !C)"},
        {"(A => B) | (A => C)", "((A => B) | (A => C))"},
    });
}

static void testCancelation(TestSuite &suite) {
    // even number of negations cancel out, odd number leaves one
    runCases(suite, "cancel", {
        {"!!A", "A"},
        {"!!!A", "!A"},
        {"!!!!A", "A"},
        {"!!!!!A", "!A"},
    });

    // cancelation happens wherever the negation sits in the tree
    runCases(suite, "cancel nested", {
        {"!!A => B", "(A => B)"},
        {"A => !!B", "(A => B)"},
        {"!!A => !!B", "(A => B)"},
        {"!!!A => !!!B", "(!A => !B)"},
        {"A + !!B => C", "((A + B) => C)"},
        {"!!A + !!B + !!C => D", "(((A + B) + C) => D)"},
        {"A | (B ^ !!C) => D", "((A | (B ^ C)) => D)"},
        {"(!!A => B) | (A => !!!C)", "((A => B) | (A => !C))"},
    });

    // a canceled negation over a parenthesized group exposes the group,
    // which must itself be reduced
    runCases(suite, "cancel group", {
        {"!!(A + B)", "(A + B)"},
        {"!!(A + B) => C", "((A + B) => C)"},
        {"!!(!!A | B) => C", "((A | B) => C)"},
        {"!!(A | !!!B) => C", "((A | !B) => C)"},
        {"!!(!!(A + B)) => C", "((A + B) => C)"},
        {"!!!!(A ^ B) => C", "((A ^ B) => C)"},
    });
}

static void testDistribution(TestSuite &suite) {
    // one rule per operator
    runCases(suite, "distribute", {
        {"!(A + B)", "(!A | !B)"},
        {"!(A | B)", "(!A + !B)"},
        {"!(A ^ B)", "(!A ^ B)"},
        {"!(A => B)", "(A + !B)"},
        {"!(A <=> B)", "(A ^ B)"},
    });

    // negations already inside the group combine with the distributed one
    runCases(suite, "distribute nested", {
        {"!(A + !B)", "(!A | B)"},
        {"!(!A | !B)", "(A + B)"},
        {"!(A + (B | C))", "(!A | (!B + !C))"},
        {"!(A + !(B | C))", "(!A | (B | C))"},
        {"!((A + B) | (C + D))", "((!A | !B) + (!C | !D))"},
        {"!(!A ^ B)", "(A ^ B)"},
        {"!(A => !B)", "(A + B)"},
        {"!((A + B) => C)", "((A + B) + !C)"},
        {"!(A <=> !(B + C))", "(A ^ (!B | !C))"},
    });

    // cancelation and distribution together
    runCases(suite, "cancel and distribute", {
        {"!!!(A | B)", "(!A + !B)"},
        {"!!(A + B)", "(A + B)"},
        {"!(!!A + B)", "(!A | !B)"},
        {"!!(!(A | B))", "(!A + !B)"},
    });

    // inside rules, the rule itself is kept
    runCases(suite, "distribute in rule", {
        {"!(A + B) => C", "((!A | !B) => C)"},
        {"A => !(B + C)", "(A => (!B | !C))"},
        {"!(A | B) <=> !(C ^ D)", "((!A + !B) <=> (!C ^ D))"},
        {"A + !(B | C) => !D", "((A + (!B + !C)) => !D)"},
        {"(A => B) | !(A => C)", "((A => B) | (A + !C))"},
    });
}

static void testIdempotence(TestSuite &suite) {
    // reducing an already reduced expression changes nothing
    for (const std::string line : {"!(A + B) => C", "!(!(A + B) | C) => E", "!!!(A ^ !B) <=> !(C => D)"}) {
        try {
            const std::string once = toString(reduce(line).get());
            suite.expectEqual("idempotent: " + line, once, toString(reduce(once).get()));
        } catch (const std::exception &error) {
            suite.check("idempotent: " + line, false, std::string("unexpected exception: ") + error.what());
        }
    }
}

static void testInvalidInput(TestSuite &suite) {
    suite.expectThrow<NullException>("reduce: null root", [] {
        (void)Reducer::reduce(nullptr);
    });
}

int main() {
    TestSuite suite("Reducer");
    testNothingToReduce(suite);
    testCancelation(suite);
    testDistribution(suite);
    testIdempotence(suite);
    testInvalidInput(suite);
    return suite.summary();
}

