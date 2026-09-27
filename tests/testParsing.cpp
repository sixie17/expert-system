#include "TestSuite.hpp"
#include "expert_system/ast/AtomicNode.hpp"
#include "expert_system/ast/NotNode.hpp"
#include "expert_system/ast/OpNode.hpp"
#include "expert_system/exceptions/Exceptions.hpp"
#include "expert_system/parser/Lexer.hpp"
#include "expert_system/parser/Parser.hpp"
#include <memory>
#include <stdexcept>
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
 * @brief Lexes and parses a line without the ParseError wrapping of
 * Parser::parseLine, so the original exception type can be checked.
 */
static std::unique_ptr<IASTNode> parse(const std::string &line) {
    Lexer lexer(line);
    std::vector<Token> tokens = lexer.tokenize();
    return Parser::parseTokens(tokens);
}

static void testValidLines(TestSuite &suite) {
    struct Case {
        std::string line;
        std::string expected;
    };
    const std::vector<Case> cases = {
        // basic operations
        {"A", "A"},
        {"A => B", "(A => B)"},
        {"A + B => C", "((A + B) => C)"},
        {"A | B => C", "((A | B) => C)"},
        {"A ^ B => C", "((A ^ B) => C)"},
        {"A + B <=> C", "((A + B) <=> C)"},
        {"A + B + C => D", "(((A + B) + C) => D)"},
        {"A => B + C", "(A => (B + C))"},
        // spacing and comments
        {"A+B=>C", "((A + B) => C)"},
        {"   A   +   B   =>   C   ", "((A + B) => C)"},
        {"A + B => C # A and B implies C", "((A + B) => C)"},
        // not
        {"!A => B", "(!A => B)"},
        {"A + !B => F", "((A + !B) => F)"},
        {"E + F => !V", "((E + F) => !V)"},
        {"!!A => B", "(!!A => B)"},
        {"!!!A => B", "(!!!A => B)"},
        {"!(A + B) => C", "(!(A + B) => C)"},
        {"!!(A | B) => C", "(!!(A | B) => C)"},
        // parentheses
        {"A + (B + C) => D", "((A + (B + C)) => D)"},
        {"((A + B) + C) => D", "(((A + B) + C) => D)"},
        {"(((A))) => B", "(A => B)"},
        {"A + !(B | !!C) => !D", "((A + !(B | !!C)) => !D)"},
        {"!(!(A + B) | C) => E", "(!(!(A + B) | C) => E)"},
        {"A + !B => !(C | D)", "((A + !B) => !(C | D))"},
        // nested logic operations (propositional logic)
        {"(A => B) | (A => C)", "((A => B) | (A => C))"},
        {"!(A => B) => C", "(!(A => B) => C)"},
        {"A => (B <=> !C)", "(A => (B <=> !C))"},
        {"A => (B => C)", "(A => (B => C))"},
        {"((A => B) => C) + D => E", "((((A => B) => C) + D) => E)"},
    };

    for (const auto &[line, expected] : cases) {
        try {
            auto node = parse(line);
            suite.expectEqual("valid: " + line, expected, toString(node.get()));
        } catch (const std::exception &error) {
            suite.check("valid: " + line, false, std::string("unexpected exception: ") + error.what());
        }
    }
}

static void testEmptyLines(TestSuite &suite) {
    for (const std::string line : {"", "   ", "# only a comment", "   # indented comment"}) {
        try {
            suite.check("empty: \"" + line + "\"", parse(line) == nullptr, "expected nullptr");
        } catch (const std::exception &error) {
            suite.check("empty: \"" + line + "\"", false, std::string("unexpected exception: ") + error.what());
        }
    }
}

template <typename E>
static void expectParseThrow(TestSuite &suite, const std::vector<std::string> &lines) {
    for (const auto &line : lines)
        suite.expectThrow<E>("invalid: " + line, [&line] { (void)parse(line); });
}

static void testInvalidLines(TestSuite &suite) {
    // an operand is missing
    expectParseThrow<MissingOperationValue>(suite, {
        "A !",
        "!",
        "! + B",
        "!)",
        "()",
        "A + ()",
        "=> B",
        "A =>",
        "A + => B",
        "A => B +",
        "A + C => + C",
        "+ A",
        "A +",
        "A (=>) B",
        "A !=> B",
        "(A =>) | B",
    });

    // an operation is missing between two operands
    expectParseThrow<MissingOperation>(suite, {
        "A B",
        "A !B",
        "A (B)",
        "(A) B",
        "A => B C",
    });

    // unbalanced parentheses
    expectParseThrow<UnclosedParenthese>(suite, {
        "(A + B",
        "((A + B) => C",
        ")",
        "A + B) => C",
        "!(A",
    });

    // chained logic operations must be parenthesized
    expectParseThrow<InvalidLogicOperation>(suite, {
        "A => B => C",
        "A <=> B => C",
        "A => B <=> C",
        "(A => B => C)",
        "A + (B => C => D)",
    });

    // unknown characters are rejected by the lexer
    expectParseThrow<std::runtime_error>(suite, {
        "A & B => C",
        "a + B => C",
        "A + 1 => C",
    });
}

static void testParseLine(TestSuite &suite) {
    suite.expectNoThrow("parseLine: valid line", [] {
        (void)Parser::parseLine("A + B => C", 1);
    });

    // errors are wrapped in a ParseError carrying the line number
    try {
        (void)Parser::parseLine("A + => B", 7);
        suite.check("parseLine: error has line number", false, "nothing was thrown");
    } catch (const ParseError &error) {
        suite.expectEqual("parseLine: getLineNumber", static_cast<size_t>(7), error.getLineNumber());
        suite.check("parseLine: message starts with line number",
                    std::string(error.what()).starts_with("line 7: "),
                    std::string("message: ") + error.what());
    } catch (const std::exception &error) {
        suite.check("parseLine: error has line number", false,
                    std::string("wrong exception thrown: ") + error.what());
    }

    suite.expectThrow<ParseError>("parseLine: lexer error is wrapped", [] {
        (void)Parser::parseLine("A & B", 3);
    });
}

int main() {
    TestSuite suite("Parsing");
    testValidLines(suite);
    testEmptyLines(suite);
    testInvalidLines(suite);
    testParseLine(suite);
    return suite.summary();
}
