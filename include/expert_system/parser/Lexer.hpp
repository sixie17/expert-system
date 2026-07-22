#pragma once

#include "Token.hpp"
#include <vector>

class Lexer {
private:
  std::string text;
  size_t pos = 0;

  char current() const;

  // Peeks ahead by 'offset' characters without advancing pos
  char peek(size_t offset) const;

  void advance(size_t count = 1);

  void skipWhitespace();

public:
  Lexer(std::string input_text) : text(std::move(input_text)) {}
  std::vector<Token> tokenize();
};
