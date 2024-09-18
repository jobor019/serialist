

#ifndef SERIALIST_LOOPER_EXCEPTIONS_H
#define SERIALIST_LOOPER_EXCEPTIONS_H

#include <stdexcept>

namespace serialist {

class IOError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};


// ==============================================================================================

class ParameterError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};


class ParameterCastError : public ParameterError {
public:
    using ParameterError::ParameterError;
};

class ParameterMissingError : public ParameterError {
public:
    using ParameterError::ParameterError;
};

class ParameterTypeMissingError : public ParameterError {
public:
    using ParameterError::ParameterError;
};



// ==============================================================================================

class TimeDomainError : public  std::runtime_error {
public:
    using runtime_error::runtime_error;
};


} // namespace serialist


#endif //SERIALIST_LOOPER_EXCEPTIONS_H
