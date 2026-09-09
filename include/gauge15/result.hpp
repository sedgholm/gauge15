// result.hpp
//
// A minimal, self-rolled Result<T> (success value XOR error) used
// throughout this library instead of exceptions.
//
// Why not exceptions: RFC 9651 section 1.1, "Intentionally Strict
// Processing", is explicit that parse failure has exactly one defined
// outcome -- fail the whole operation -- and that this is a normal,
// expected result, not an exceptional circumstance. Modeling that with
// exceptions would mean every caller wraps every parse call in a
// try/catch merely to check "did this string parse," which is both
// slower (exceptions are not free, even when a compiler is good at it)
// and more awkward than checking a boolean.
//
// Why not std::expected: this library targets C++20; std::expected is
// C++23. This is deliberately the smallest possible stand-in, not a
// general-purpose utility -- it exists so the rest of the library has
// something to return, not to be reused elsewhere.

#ifndef SFV_RESULT_HPP
#define SFV_RESULT_HPP

#include <string>
#include <utility>
#include <variant>

namespace sfv {

// Describes why a parse or serialize operation failed. `position` is the
// byte offset into the original input at which the failure was detected,
// for callers that want to point at the exact spot (e.g. in an error
// message or a test failure report). It is set to input.size() (i.e.
// "at/after the end") for failures detected only once input is
// exhausted, such as an unterminated string.
struct Error {
    std::string message;
    size_t position;
};

template <typename T>
class Result {
public:
    Result(T value) : storage_(std::move(value)) {}
    Result(Error error) : storage_(std::move(error)) {}

    bool ok() const { return std::holds_alternative<T>(storage_); }
    explicit operator bool() const { return ok(); }

    // Precondition: ok(). Asserts in debug builds if called on an error
    // Result -- callers are expected to check ok() first, the same way
    // std::optional::value() (without exceptions enabled) or
    // std::expected::value() would be used.
    const T& value() const& { return std::get<T>(storage_); }
    T& value() & { return std::get<T>(storage_); }
    T&& value() && { return std::get<T>(std::move(storage_)); }

    const Error& error() const& { return std::get<Error>(storage_); }

private:
    std::variant<T, Error> storage_;
};

}  // namespace sfv

#endif  // SFV_RESULT_HPP
