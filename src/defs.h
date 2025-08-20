#pragma once

#include <cmath>
#include <cstdint>
#include <optional>
#include <utility>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef size_t usize;

#define USIZE_MAX SIZE_MAX
#define BIT(x) 1 << (x)
#define KB(x) ((usize)1024 * x)
#define MB(x) ((usize)1024 * KB(x))
#define GB(x) ((usize)1024 * MB(x))

template <typename F> struct Defer {
    Defer(F f) : f(f) {}
    ~Defer() { f(); }
    F f;
};

template <typename F> Defer<F> makeDefer(F f) { return Defer<F>(f); };

#define __defer(line) defer_##line
#define _defer(line) __defer(line)

struct defer_dummy {};
template <typename F> Defer<F> operator+(defer_dummy, F&& f) {
    return makeDefer<F>(std::forward<F>(f));
}

#define defer auto _defer(__LINE__) = defer_dummy() + [&]()

template <typename T>
concept IntegerType =
    std::same_as<T, i8> || std::same_as<T, i16> || std::same_as<T, i32> || std::same_as<T, i64> ||
    std::same_as<T, u8> || std::same_as<T, u16> || std::same_as<T, u32> || std::same_as<T, u64>;

template <IntegerType T> inline std::optional<T> int_from_str(const char* str) {
    char* endptr;
    int64_t num = strtol(str, &endptr, 10);
    if (endptr == str || *endptr != '\0') {
        return std::nullopt;
    }

    // Check bounds for the target type
    if constexpr (std::is_signed_v<T>) {
        if (num < std::numeric_limits<T>::min() || num > std::numeric_limits<T>::max()) {
            return std::nullopt;
        }
    } else {
        if (num < 0 || static_cast<u64>(num) > std::numeric_limits<T>::max()) {
            return std::nullopt;
        }
    }

    return static_cast<T>(num);
}

template <typename T>
concept FloatType = std::same_as<T, f32> || std::same_as<T, f64>;

template <FloatType T> inline std::optional<T> float_from_str(const char* str) {
    char* endptr;
    double num = strtod(str, &endptr);
    if (endptr == str || *endptr != '\0') {
        return std::nullopt;
    }

    // Check for overflow/underflow
    if (num == HUGE_VAL || num == -HUGE_VAL) {
        return std::nullopt;
    }

    // Check bounds for f32
    if constexpr (std::same_as<T, f32>) {
        if (num > std::numeric_limits<f32>::max() || num < std::numeric_limits<f32>::lowest()) {
            return std::nullopt;
        }
    }

    return static_cast<T>(num);
}
