#include "expert_system/parser/Lexer.hpp"
#include <stdexcept>

char Lexer::current() const {
  if (pos >= text.length())
    return '\0';
  return text[pos];
}

// Peeks ahead by 'offset' characters without advancing pos
char Lexer::peek(size_t offset) const {
  if (pos + offset >= text.length())
    return '\0';
  return text[pos + offset];
}

void Lexer::advance(size_t count) { pos += count; }

void Lexer::skipWhitespace() {
  while (current() != '\0' && std::isspace(current())) {
    advance();
  }
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;

  while (current() != '\0') {
    skipWhitespace();

    // End of line reached after whitespace
    if (current() == '\0')
      break;

    // 1. Handle Comments (#) - Stop lexing the rest of the line
    if (current() == '#') {
      break;
    }

    // 2. Multi-character Operators (Must check longest first!)
    if (current() == '<' && peek(1) == '=' && peek(2) == '>') {
      tokens.push_back({TokenType::IFAOF, "<=>"});
      advance(3);
      continue;
    }

    if (current() == '=' && peek(1) == '>') {
      tokens.push_back({TokenType::IMPLIES, "=>"});
      advance(2);
      continue;
    }

    if (current() == '=') {
      tokens.push_back({TokenType::INITIAL_FACTS, "="});
      advance();
      continue;
    }

    if (current() == '?') {
      tokens.push_back({TokenType::QUERIES, "?"});
      advance();
      continue;
    }

    if (current() == '+') {
      tokens.push_back({TokenType::AND, "+"});
      advance();
      continue;
    }
    if (current() == '|') {
      tokens.push_back({TokenType::OR, "|"});
      advance();
      continue;
    }
    if (current() == '^') {
      tokens.push_back({TokenType::XOR, "^"});
      advance();
      continue;
    }
    if (current() == '!') {
      tokens.push_back({TokenType::NOT, "!"});
      advance();
      continue;
    }
    if (current() == '(') {
      tokens.push_back({TokenType::LPAREN, "("});
      advance();
      continue;
    }
    if (current() == ')') {
      tokens.push_back({TokenType::RPAREN, ")"});
      advance();
      continue;
    }

    // 4. Variables (Uppercase letters A-Z)
    if (std::isupper(current())) {
      tokens.push_back({TokenType::VARIABLE, std::string(1, current())});
      advance();
      continue;
    }

    // 5. Unrecognized Character Error
    throw std::runtime_error(
        std::string("Lexer Error: Unrecognized character '") + current() +
        "' at index " + std::to_string(pos));
  }

  // Always append an END token so the parser knows when to stop
  tokens.push_back({TokenType::END_OF_LINE, ""});
  return tokens;
}
