#pragma once
#include <string>

/**
 * @brief Represents the type of a token. ATOMIC, OP, COMMENT.
 * @enum TokenType
 * @var Variable A-Z
 * @var AND +
 * @var OR |
 * @var XOR ^
 * @var NOT !
 * @var IMPLIES =>
 * @var IFAOF <=>
 * @var LPAREN (
 * @var RPAREN )
 * @var END_OF_LINE End of input
 */
enum class TokenType {
    VARIABLE,      // A-Z
    AND,           // +
    OR,            // |
    XOR,           // ^
    NOT,           // !
    IMPLIES,       // =>
    IFAOF,         // <=>
    LPAREN,        // (
    RPAREN,        // )
    INITIAL_FACTS, // = (when NOT followed by >)
    QUERIES,       // ?
    END_OF_LINE    // End of input
};

struct Token {
  TokenType type;
  std::string value;

  // Helper for debugging/printing
  std::string toString() const { return "Token(" + value + ")"; }
};
