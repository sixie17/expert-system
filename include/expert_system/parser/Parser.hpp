#pragma once

#include "expert_system/ast/IASTNode.hpp"
#include "Token.hpp"
#include <memory>
#include <optional>
#include <span>
#include <stack>
#include <string>

/**
 * @brief Builds an Abstract Syntax Tree from a line of rules.
 *
 * Stateless: every method is static, so it is used as Parser::parseLine(line)
 * without creating an instance.
 *
 * Each line is tokenized and built left to right using a node stack
 * (kind of like RPN):
 * @example A => B
 *   A finds the stack empty so it is pushed directly,
 *   => pops A, sets it as its left node and is pushed back,
 *   B finds the OpNode on top of the stack and becomes its right node.
 */
class Parser {
    private:
        using TokenIt = std::span<const Token>::iterator;
        using NodeStack = std::stack<std::unique_ptr<IASTNode>>;

        /**
         * @brief Parses a single operand starting at `it`: a VARIABLE, a
         * parenthesized group, or a NOT applied to another operand
         * (recursive, so !!!A works).
         * @param it Current token, left on the last token consumed by the operand.
         * @param end End of the token range.
         * @return The operand subtree.
         * @throws MissingOperationValue if no operand is found.
         * @throws UnclosedParenthese if a '(' has no matching ')'.
         */
        static std::unique_ptr<IASTNode> parseOperand(TokenIt &it, TokenIt end);

        /**
         * @brief Places a parsed operand in the tree: pushed if the stack is
         * empty, otherwise set as the right node of the pending OpNode on top.
         * @throws MissingOperation if there is no pending OpNode to attach to.
         */
        static void attachOperand(std::unique_ptr<IASTNode> node, NodeStack &nodeStack);

        /**
         * @brief True for binary operator tokens (AND, OR, XOR, IMPLIES, ...).
         */
        static bool isOpToken(const Token &token);

        /**
         * @brief True if the node is a binary OpNode (not ATOMIC nor NOT).
         */
        static bool isOpNode(const std::unique_ptr<IASTNode> &node);

        /**
         * @brief True for IMPLIES and IFAOF tokens.
         */
        static bool isLogicOperation(const Token &token);

        /**
         * @brief Maps a token type to its AST operation, if it has one.
         */
        static constexpr std::optional<OpType> tokenToOp(TokenType token);

        /**
         * @brief Parses a range of tokens into an AST.
         * @param nested True inside parentheses or on either side of =>/<=>,
         * where a logic operation (=>, <=>) is not allowed.
         * @throws InvalidLogicOperation if a logic operation appears while nested.
         */
        static std::unique_ptr<IASTNode> parseTokens(std::span<const Token> tokens, bool nested);

    public:
        Parser() = delete;

        /**
         * @brief Tokenizes and parses a single line.
         * @param line The line to parse.
         * @param lineNumber The line's position in the input file (1-based),
         * reported in error messages.
         * @return The root of the AST, or nullptr for an empty line.
         * @throws ParseError wrapping any lexing/parsing error with the line number.
         */
        [[nodiscard]] static std::unique_ptr<IASTNode> parseLine(const std::string &line, size_t lineNumber);

        /**
         * @brief Parses a range of tokens into an AST.
         *
         * A logic operation (=>, <=>) splits the line: everything before it is
         * the left node, everything after it the right node, so it is always
         * the root. It may appear at most once and never inside parentheses.
         * @return The root of the AST, or nullptr if there are no tokens.
         */
        [[nodiscard]] static std::unique_ptr<IASTNode> parseTokens(std::span<const Token> tokens);
};
