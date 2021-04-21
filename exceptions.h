#ifndef SIK1__EXCEPTIONS_H_
#define SIK1__EXCEPTIONS_H_

#include <stdexcept>

class ConnectionLost : public std::runtime_error {
  public:
    explicit ConnectionLost(std::string &&str) : std::runtime_error(str) {}
};

class IncorrectRequestFormat : public std::runtime_error {
  public:
    explicit IncorrectRequestFormat(std::string &&str) : std::runtime_error(str) {}
};

#endif //SIK1__EXCEPTIONS_H_
