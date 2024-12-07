
#ifndef TESTUTILS_EXCEPTIONS_H
#define TESTUTILS_EXCEPTIONS_H

#include <stdexcept>

namespace serialist::test {
class test_error : public std::runtime_error {
public:
    explicit test_error(const char* err) : std::runtime_error(err) {}

    explicit test_error(const std::string& err) : std::runtime_error(err) {}
};
}

#endif //TESTUTILS_EXCEPTIONS_H
