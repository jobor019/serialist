

#ifndef SERIALIST_LOOPER_EXCEPTIONS_H
#define SERIALIST_LOOPER_EXCEPTIONS_H

#include <stdexcept>

class IOError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};


// ==============================================================================================

class ParameterError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

#endif //SERIALIST_LOOPER_EXCEPTIONS_H
