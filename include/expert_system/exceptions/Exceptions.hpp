#pragma once
#include <exception>
#include <string>

class NullException : public std::exception {
private:
    // Store the final error message, not just the method name
    const std::string full_message;

public: 
    ~NullException() = default;
    
    // Concatenate the message ONCE during construction
    NullException(const std::string& method) 
        : full_message(method + " Null Pointer Exception") {}
        
    // Safely return a pointer to the member variable's buffer
    const char* what() const noexcept override {
        return this->full_message.c_str();
    }
};


class MissingOperationValue: public std::exception {
  const std::string method;
  std::string full_message;

  public:
    ~MissingOperationValue() = default;
    MissingOperationValue(std::string method) : method(method), full_message(method + "MissingOperationValue") {}

    const char* what() const noexcept override { 
      return this->full_message.c_str();
    }
};



class MissingOperation: public std::exception {
  const std::string method;
  const std::string side;
  std::string full_message;

  public:
    ~MissingOperation() = default;
    MissingOperation(std::string method, std::string side): method(method), side(side), full_message(method + "MissingOperation on " + side) {}
    

    const char *what() const noexcept override {
      return this->full_message.c_str();
    }
};

class UnclosedParenthese: public std::exception {
  const std::string method;
  std::string full_message;

  public:
    ~UnclosedParenthese() = default;
    UnclosedParenthese(std::string method) : method(method), full_message(method + "UnclosedParenthese") {}

    const char* what() const noexcept override { 
      return this->full_message.c_str();
    }
};



class InvalidLogicOperation: public std::exception {
  const std::string method;
  std::string full_message;

  public:
    ~InvalidLogicOperation() = default;
    InvalidLogicOperation(std::string method) : method(method), full_message(method + "InvalidLogicOperation: => and <=> must appear once, outside parentheses and not after !") {}

    const char* what() const noexcept override {
      return this->full_message.c_str();
    }
};

/**
 * @brief Wraps any error raised while lexing/parsing a line with its line number.
 */
class ParseError: public std::exception {
  const size_t lineNumber;
  std::string full_message;

  public:
    ~ParseError() = default;
    ParseError(size_t lineNumber, const std::string &reason) : lineNumber(lineNumber), full_message("line " + std::to_string(lineNumber) + ": " + reason) {}

    size_t getLineNumber() const noexcept {
      return this->lineNumber;
    }

    const char* what() const noexcept override {
      return this->full_message.c_str();
    }
};
