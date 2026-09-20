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

  public:
    ~MissingOperationValue() = default;
    MissingOperationValue(std::string method) : method(method) {}

    const char* what() const noexcept override {
      return (this->method + "MissingOperationValue").c_str();
    }
};



class MissingOperation: public std::exception {
  const std::string method;
  const std::string side;

  public:
    ~MissingOperation() = default;
    MissingOperation(std::string method, std::string side): method(method), side(side) {}
    

    const char *what() const noexcept override {
      return (this->method + "MissingOperation on " + this->side).c_str();
    }
};
