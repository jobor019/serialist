

#ifndef SERIALIST_LOOPER_EXCEPTIONS_H
#define SERIALIST_LOOPER_EXCEPTIONS_H

#include <stdexcept>

class IOError : public std::runtime_error {
public:
    explicit IOError(const char* err) : std::runtime_error(err) {}
};


// ==============================================================================================

class ParameterError : public std::runtime_error {
public:
    explicit ParameterError(const char* err) : std::runtime_error(err) {}
};

#endif //SERIALIST_LOOPER_EXCEPTIONS_H
