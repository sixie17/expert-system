#include <iostream>
#include <cstring>

class NullException :  public std::exception {
    private:
        const std::string method;

    public: 
        ~NullException() = default;
        NullException(std::string method) : method(method) {}
        const char* what() {
            return (this->method + " Null Pointer Exception").c_str();
        }
};