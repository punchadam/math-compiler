#include <sstream>

struct Error {
    size_t pos;
    std::stringstream message;
};