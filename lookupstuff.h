#ifndef LOOKUPSTUFF_H
#define LOOKUPSTUFF_H

#include <string>
#include <vector>
#include <optional>
#include <stdint.h>
#include <variant>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

struct Number {
    std::variant<double, i64> value;
    bool isInt = false;
    //bool isInt() { return std::holds_alternative<i64>(value); }
};

enum class SupportedCommands {
    left, right,
    frac,
    sqrt,
    cdot, times, div,
    sin, cos, tan, csc, sec, cot,
    log, ln, exp,
    pi, infty, e,
    sum, prod,
    integrate, lim
};

#endif