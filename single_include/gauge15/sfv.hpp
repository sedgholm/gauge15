// sfv.hpp -- single-header amalgamation of gauge15.
//
// GENERATED FILE. DO NOT EDIT DIRECTLY.
//
// Source of truth is include/gauge15/*.hpp; this file is produced by
// tools/amalgamate.py, which concatenates every header in dependency
// order into one self-contained, drop-in file. Regenerate with:
//   python3 tools/amalgamate.py > single_include/gauge15/sfv.hpp
//
// A from-scratch, header-only C++20 implementation of RFC 9651,
// "Structured Field Values for HTTP". See the project README for
// scope, design notes, and usage examples -- this banner is
// intentionally short; the multi-file source under include/gauge15/ is
// where the real documentation (RFC section references, rationale for
// each design choice) lives, split up per concern rather than repeated
// here for every one of the ~10 files being concatenated.

#ifndef SFV_SINGLE_INCLUDE_HPP
#define SFV_SINGLE_INCLUDE_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <string_view>
#include <charconv>

// ============================================================
// types.hpp
// ============================================================
// types.hpp
//
// The abstract data model from RFC 9651 Section 3, "Structured Data
// Types": https://www.rfc-editor.org/rfc/rfc9651.html#section-3
//
// This header defines the types only -- no parsing or serialization
// logic lives here (see parser.hpp / serializer.hpp). That split matters
// for a header-only library: these types have zero dependencies beyond
// the standard library, so anything that just wants to *build* or
// *inspect* a Structured Field value in memory (e.g. to construct one
// for serialization) doesn't need to pull in the parser at all.


#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace sfv {

// --- Decimal (RFC 9651 Section 3.3.2) ------------------------------------
//
// Stored as an exact fixed-point value (the wire value times 1000), not
// as a `double`. This matters: the wire format's fractional component is
// defined to have at most three digits and is compared/round-tripped
// exactly, whereas IEEE 754 binary64 cannot represent many 3-decimal-
// place values exactly (e.g. 0.1). A fixed-point integer avoids that
// mismatch entirely, at the cost of the caller working in "thousandths"
// rather than a native floating-point type -- a deliberate trade in
// favor of exactness over convenience, matching the RFC's own emphasis
// on exact round-tripping (see Section 3.3.2's note on rounding).
struct Decimal {
    // Exact value = scaled / 1000. E.g. 4.5 -> scaled == 4500;
    // -0.001 -> scaled == -1.
    int64_t scaled = 0;

    static constexpr Decimal from_scaled(int64_t scaled_value) {
        Decimal d;
        d.scaled = scaled_value;
        return d;
    }

    friend bool operator==(const Decimal&, const Decimal&) = default;
};

// --- Token (RFC 9651 Section 3.3.4) --------------------------------------
//
// Wrapped in its own struct, rather than reusing std::string the way
// String does, specifically to keep Token and String distinguishable as
// separate alternatives in the BareItem variant below. RFC 9651 Appendix
// B calls this out directly: "it's important to preserve the distinction
// between Tokens and Strings... it may be necessary to create a wrapper
// 'token' object... to assure that these types remain separate."
struct Token {
    std::string value;
    friend bool operator==(const Token&, const Token&) = default;
};

// --- Byte Sequence (RFC 9651 Section 3.3.5) ------------------------------
//
// Holds already-decoded raw bytes (as a std::string used purely as a byte
// container, not as text) -- NOT the base64 wire text. Encoding/decoding
// base64 is the parser/serializer's job.
struct ByteSequence {
    std::string bytes;
    friend bool operator==(const ByteSequence&, const ByteSequence&) = default;
};

// --- Date (RFC 9651 Section 3.3.7, new vs. RFC 8941) ---------------------
//
// A signed delta in seconds from 1970-01-01T00:00:00Z, per the RFC's own
// data model -- deliberately NOT a calendar/time-zone-aware type, since
// the wire format itself is just a signed integer with an "@" prefix.
struct Date {
    int64_t value = 0;
    friend bool operator==(const Date&, const Date&) = default;
};

// --- Display String (RFC 9651 Section 3.3.8, new vs. RFC 8941) ----------
//
// Stores its content as UTF-8 (matching how the parsing algorithm itself
// decodes into a Unicode code point sequence via UTF-8 -- see Section
// 4.2.10). Unlike String, may contain any Unicode scalar value.
struct DisplayString {
    std::string utf8;
    friend bool operator==(const DisplayString&, const DisplayString&) = default;
};

// --- Bare Item (RFC 9651 Section 3.3) ------------------------------------
//
// The eight alternatives exactly match Section 3.3's list: Integer,
// Decimal, String, Token, Byte Sequence, Boolean, Date, Display String.
// Plain `std::string` is used directly for String (no wrapper needed,
// since Token/ByteSequence/DisplayString are already distinctly typed);
// plain `int64_t` for Integer; plain `bool` for Boolean.
using BareItem =
    std::variant<int64_t, Decimal, std::string, Token, ByteSequence, bool, Date, DisplayString>;

// --- Ordered map (used by both Parameters and Dictionary) ----------------
//
// RFC 9651 Section 3.1.2 and 3.2 both specify their map type as an
// *ordered* map with unique keys, where a repeated key overwrites the
// existing value IN PLACE (preserving its original position) rather than
// moving it to the end. `std::map`/`std::unordered_map` preserve neither
// property, so this is a small purpose-built container: a flat
// vector-of-pairs, which is also the right performance choice here since
// real Parameters/Dictionary instances are small (the RFC's own minimum
// guarantees are only tens to low hundreds of entries) -- a vector scan
// beats a tree or hash table at that size.
template <typename Value>
class OrderedMap {
public:
    using Entry = std::pair<std::string, Value>;

    // If `key` already exists, overwrites its value in place (position
    // unchanged) per RFC 9651 Sections 4.2.2/4.2.3.2 ("If ... already
    // contains a key ..., overwrite its value"). Otherwise appends a new
    // entry at the end ("Otherwise, append key ... to ...").
    void set(std::string key, Value value) {
        for (auto& entry : entries_) {
            if (entry.first == key) {
                entry.second = std::move(value);
                return;
            }
        }
        entries_.emplace_back(std::move(key), std::move(value));
    }

    const Value* get(const std::string& key) const {
        for (const auto& entry : entries_) {
            if (entry.first == key) return &entry.second;
        }
        return nullptr;
    }

    bool contains(const std::string& key) const { return get(key) != nullptr; }

    bool empty() const { return entries_.empty(); }
    size_t size() const { return entries_.size(); }

    auto begin() const { return entries_.begin(); }
    auto end() const { return entries_.end(); }

    friend bool operator==(const OrderedMap&, const OrderedMap&) = default;

private:
    std::vector<Entry> entries_;
};

// --- Parameters (RFC 9651 Section 3.1.2) ---------------------------------
//
// Values are bare items: per the RFC, parameter values "themselves cannot
// be parameterized" -- there is no recursive Parameters-on-Parameters.
using Parameters = OrderedMap<BareItem>;

// --- Item (RFC 9651 Section 3.3) -----------------------------------------
//
// A bare item plus its own Parameters. This is what appears standalone as
// a top-level Item field value, as each element of an Inner List, and
// (via the Member variant below) as a List/Dictionary member.
struct Item {
    BareItem value;
    Parameters params;
    friend bool operator==(const Item&, const Item&) = default;
};

// --- Inner List (RFC 9651 Section 3.1.1) ---------------------------------
//
// An array of Items, with its OWN Parameters distinct from any of its
// members' individual Parameters (e.g. "(\"a\" \"b\");lvl=1" -- the
// Inner List itself carries lvl=1, separate from whatever parameters "a"
// or "b" might individually have).
struct InnerList {
    std::vector<Item> items;
    Parameters params;
    friend bool operator==(const InnerList&, const InnerList&) = default;
};

// --- Member (used by both List and Dictionary) ---------------------------
//
// A List's members, and a Dictionary's values, are both defined by the
// RFC as "an Item or Inner List" (Sections 3.1 and 3.2) -- the same
// variant is shared here rather than duplicated.
using Member = std::variant<Item, InnerList>;

// --- List (RFC 9651 Section 3.1) -----------------------------------------
using List = std::vector<Member>;

// --- Dictionary (RFC 9651 Section 3.2) -----------------------------------
using Dictionary = OrderedMap<Member>;

}  // namespace sfv

// ============================================================
// result.hpp
// ============================================================
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

// ============================================================
// detail/utf8.hpp
// ============================================================
// utf8.hpp
//
// A minimal UTF-8 validator, used only by Display String parsing/
// serialization (RFC 9651 Sections 4.2.10, 4.1.11), which store their
// content as already-encoded UTF-8 bytes (see DisplayString in
// types.hpp). "Decode byte_array as a UTF-8 string; fail if decoding
// fails" (Section 4.2.10 step 4.1) requires genuine validation, not just
// acceptance of any byte sequence -- specifically rejecting:
//   - continuation bytes appearing without a valid leading byte
//   - truncated multi-byte sequences
//   - overlong encodings (e.g. encoding U+0041 'A' as a 2-byte sequence
//     when the 1-byte encoding is required) -- these are invalid per
//     RFC 3629 and are a classic security issue (can be used to smuggle
//     characters past ASCII-only filters)
//   - surrogate code points (U+D800-U+DFFF), which UTF-8 must never
//     encode (RFC 9651 4.1.11 calls this out explicitly)
//   - code points beyond U+10FFFF


#include <cstdint>
#include <string_view>

namespace sfv::detail {

inline bool is_valid_utf8(std::string_view s) {
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[i]);

        if (c < 0x80) {
            ++i;
            continue;
        }

        int extra;
        uint32_t cp;
        uint32_t min_cp;  // smallest code point that legitimately needs this many bytes

        if ((c & 0xE0) == 0xC0) {
            extra = 1;
            cp = c & 0x1F;
            min_cp = 0x80;
        } else if ((c & 0xF0) == 0xE0) {
            extra = 2;
            cp = c & 0x0F;
            min_cp = 0x800;
        } else if ((c & 0xF8) == 0xF0) {
            extra = 3;
            cp = c & 0x07;
            min_cp = 0x10000;
        } else {
            return false;  // stray continuation byte or invalid leading byte (0xF8+)
        }

        if (i + 1 + static_cast<size_t>(extra) > s.size()) return false;  // truncated

        for (int k = 1; k <= extra; ++k) {
            unsigned char cc = static_cast<unsigned char>(s[i + static_cast<size_t>(k)]);
            if ((cc & 0xC0) != 0x80) return false;  // not a valid continuation byte
            cp = (cp << 6) | (cc & 0x3F);
        }

        if (cp < min_cp) return false;              // overlong encoding
        if (cp > 0x10FFFF) return false;             // beyond Unicode's range
        if (cp >= 0xD800 && cp <= 0xDFFF) return false;  // surrogate: never valid in UTF-8

        i += static_cast<size_t>(extra) + 1;
    }
    return true;
}

}  // namespace sfv::detail

// ============================================================
// detail/base64.hpp
// ============================================================
// base64.hpp
//
// Minimal base64 (RFC 4648 Section 4, standard alphabet) encode/decode,
// used only by Byte Sequence parsing/serialization (RFC 9651 Sections
// 3.3.5, 4.1.8, 4.2.7). Self-rolled rather than pulling in a dependency,
// consistent with this being a small, header-only, zero-dependency
// library.
//
// The decoder deliberately follows RFC 9651 Section 4.2.7's specific
// leniency instructions, which are stricter than "any base64 decoder"
// would be by default:
//   - Missing "=" padding is tolerated ("synthesizing padding if
//     necessary") -- decode_base64 pads internally before decoding.
//   - Non-zero pad bits in the final partial group are NOT rejected
//     (Section 4.2.7 explicitly says parsers SHOULD NOT fail on this).
//   - Characters outside the base64 alphabet, and embedded newlines,
//     MUST cause failure -- this is NOT relaxed, per the same section.


#include <cstdint>
#include <optional>
#include <string>
#include <string_view>


namespace sfv::detail {

inline std::optional<int> base64_char_value(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return std::nullopt;
}

// Decodes base64 text (without surrounding ":" delimiters -- those are
// the Byte Sequence wire format's job, handled by parse_byte_sequence)
// into raw bytes. `input` must already be known to contain only
// characters from [A-Za-z0-9+/=] (parse_byte_sequence checks this before
// calling); this function still defends against invalid characters
// itself so it can be used standalone/tested independently.
inline Result<std::string> decode_base64(std::string_view input) {
    std::string padded(input);
    while (padded.size() % 4 != 0) padded.push_back('=');

    std::string out;
    out.reserve(padded.size() / 4 * 3);

    for (size_t i = 0; i < padded.size(); i += 4) {
        char c0 = padded[i], c1 = padded[i + 1], c2 = padded[i + 2], c3 = padded[i + 3];

        if (c0 == '=' || c1 == '=') {
            return Error{"base64 padding cannot appear in the first two positions of a group", i};
        }
        auto v0 = base64_char_value(c0);
        auto v1 = base64_char_value(c1);
        if (!v0 || !v1) return Error{"invalid base64 character", i};

        out.push_back(static_cast<char>((*v0 << 2) | (*v1 >> 4)));

        if (c2 == '=') {
            if (c3 != '=') return Error{"invalid base64 padding", i};
            continue;  // end of data; any remaining groups (there are none) would be malformed
        }
        auto v2 = base64_char_value(c2);
        if (!v2) return Error{"invalid base64 character", i};
        out.push_back(static_cast<char>(((*v1 & 0xF) << 4) | (*v2 >> 2)));

        if (c3 == '=') continue;
        auto v3 = base64_char_value(c3);
        if (!v3) return Error{"invalid base64 character", i};
        out.push_back(static_cast<char>(((*v2 & 0x3) << 6) | *v3));
    }

    return out;
}

inline std::string encode_base64(std::string_view bytes) {
    static constexpr char kAlphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string out;
    out.reserve((bytes.size() + 2) / 3 * 4);

    size_t i = 0;
    for (; i + 3 <= bytes.size(); i += 3) {
        uint32_t n = (static_cast<uint32_t>(static_cast<uint8_t>(bytes[i])) << 16) |
                     (static_cast<uint32_t>(static_cast<uint8_t>(bytes[i + 1])) << 8) |
                     static_cast<uint32_t>(static_cast<uint8_t>(bytes[i + 2]));
        out.push_back(kAlphabet[(n >> 18) & 0x3F]);
        out.push_back(kAlphabet[(n >> 12) & 0x3F]);
        out.push_back(kAlphabet[(n >> 6) & 0x3F]);
        out.push_back(kAlphabet[n & 0x3F]);
    }

    size_t remaining = bytes.size() - i;
    if (remaining == 1) {
        uint32_t n = static_cast<uint32_t>(static_cast<uint8_t>(bytes[i])) << 16;
        out.push_back(kAlphabet[(n >> 18) & 0x3F]);
        out.push_back(kAlphabet[(n >> 12) & 0x3F]);
        out.push_back('=');
        out.push_back('=');
    } else if (remaining == 2) {
        uint32_t n = (static_cast<uint32_t>(static_cast<uint8_t>(bytes[i])) << 16) |
                     (static_cast<uint32_t>(static_cast<uint8_t>(bytes[i + 1])) << 8);
        out.push_back(kAlphabet[(n >> 18) & 0x3F]);
        out.push_back(kAlphabet[(n >> 12) & 0x3F]);
        out.push_back(kAlphabet[(n >> 6) & 0x3F]);
        out.push_back('=');
    }

    return out;
}

}  // namespace sfv::detail

// ============================================================
// parser.hpp
// ============================================================
// parser.hpp
//
// Implements RFC 9651 Section 4.2, "Parsing Structured Fields":
//   https://www.rfc-editor.org/rfc/rfc9651.html#section-4.2
//
// Each function below corresponds to exactly one numbered subsection of
// 4.2, named to match (parse_integer_or_decimal <-> 4.2.4, parse_boolean
// <-> 4.2.8, etc.), so the RFC's own step-by-step algorithm can be read
// directly alongside the code that implements it.
//
// Every function takes `std::string_view& input` and consumes from its
// front as it parses -- mirroring the RFC's own description of each
// algorithm ("input_string is modified to remove the parsed value")
// rather than building a separate tokenizer/cursor abstraction on top.
//
// This file implements the full RFC 9651 value grammar. Specifications
// such as RFC 10008's Accept-Query field build on that grammar by
// defining field-specific semantics; they do not add new parser syntax
// here.


#include <charconv>
#include <cstdint>
#include <string_view>


namespace sfv {

namespace detail {

constexpr bool is_digit(char c) { return c >= '0' && c <= '9'; }
constexpr bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
constexpr bool is_lcalpha(char c) { return c >= 'a' && c <= 'z'; }

// RFC 9110 Section 5.6.2's tchar set, needed by Token parsing/serializing.
constexpr bool is_tchar(char c) {
    switch (c) {
        case '!': case '#': case '$': case '%': case '&': case '\'': case '*':
        case '+': case '-': case '.': case '^': case '_': case '`': case '|': case '~':
            return true;
        default:
            return is_digit(c) || is_alpha(c);
    }
}

// SP only (used where the RFC specifically says "SP", e.g. inside Inner
// Lists), vs. OWS (SP or HTAB, used for the comma/OWS separators between
// top-level List/Dictionary members -- RFC 9651 section 4.2 notes OWS is
// permitted there specifically to tolerate tab characters some
// implementations use when combining field lines).
inline void discard_leading_sp(std::string_view& input) {
    size_t i = 0;
    while (i < input.size() && input[i] == ' ') ++i;
    input.remove_prefix(i);
}

inline void discard_leading_ows(std::string_view& input) {
    size_t i = 0;
    while (i < input.size() && (input[i] == ' ' || input[i] == '\t')) ++i;
    input.remove_prefix(i);
}

inline Error make_error(const std::string_view& original_start_marker, std::string message,
                         size_t position) {
    (void)original_start_marker;
    return Error{std::move(message), position};
}

// --- 4.2.4: Parsing an Integer or Decimal --------------------------------
//
// Returns either an int64_t (Integer) or a Decimal, matching the RFC's
// own "let type be integer, may become decimal" structure via a variant.
Result<std::variant<int64_t, Decimal>> parse_integer_or_decimal(std::string_view& input,
                                                                  size_t consumed_before);

// --- 4.2.8: Parsing a Boolean ---------------------------------------------
Result<bool> parse_boolean(std::string_view& input, size_t consumed_before);

// --- 4.2.5: Parsing a String --------------------------------------------
Result<std::string> parse_string(std::string_view& input, size_t consumed_before);

// --- 4.2.6: Parsing a Token ---------------------------------------------
Result<Token> parse_token(std::string_view& input, size_t consumed_before);

// --- 4.2.7: Parsing a Byte Sequence -------------------------------------
Result<ByteSequence> parse_byte_sequence(std::string_view& input, size_t consumed_before);

// --- 4.2.9: Parsing a Date ----------------------------------------------
Result<Date> parse_date(std::string_view& input, size_t consumed_before);

// --- 4.2.10: Parsing a Display String -----------------------------------
Result<DisplayString> parse_display_string(std::string_view& input, size_t consumed_before);

// --- 4.2.3.3: Parsing a Key ------------------------------------------------
Result<std::string> parse_key(std::string_view& input, size_t consumed_before);

// --- 4.2.3.1: Parsing a Bare Item ------------------------------------------
Result<BareItem> parse_bare_item(std::string_view& input, size_t consumed_before);

// --- 4.2.3.2: Parsing Parameters -------------------------------------------
Result<Parameters> parse_parameters(std::string_view& input, size_t consumed_before);

// --- 4.2.3: Parsing an Item -------------------------------------------------
Result<Item> parse_item(std::string_view& input, size_t consumed_before);

// --- 4.2.1.2: Parsing an Inner List -----------------------------------------
Result<InnerList> parse_inner_list(std::string_view& input, size_t consumed_before);

// --- 4.2.1.1: Parsing an Item or Inner List ---------------------------------
Result<Member> parse_item_or_inner_list(std::string_view& input, size_t consumed_before);

// --- 4.2.1: Parsing a List ---------------------------------------------------
Result<List> parse_list(std::string_view& input, size_t consumed_before);

// --- 4.2.2: Parsing a Dictionary -----------------------------------------------
Result<Dictionary> parse_dictionary(std::string_view& input, size_t consumed_before);

// ===========================================================================
// Implementations
// ===========================================================================

inline Result<std::variant<int64_t, Decimal>> parse_integer_or_decimal(std::string_view& input,
                                                                         size_t consumed_before) {
    size_t start_len = input.size();
    auto current_pos = [&] { return consumed_before + (start_len - input.size()); };

    bool is_decimal = false;
    int sign = 1;
    std::string number;  // digits and at most one '.', sign handled separately

    if (!input.empty() && input.front() == '-') {
        input.remove_prefix(1);
        sign = -1;
    }

    if (input.empty()) {
        return Error{"empty integer", current_pos()};
    }
    if (!is_digit(input.front())) {
        return Error{"expected DIGIT after optional '-'", current_pos()};
    }

    while (!input.empty()) {
        char c = input.front();
        if (is_digit(c)) {
            number.push_back(c);
            input.remove_prefix(1);
        } else if (!is_decimal && c == '.') {
            if (number.size() > 12) {
                return Error{"too many digits before '.' for a Decimal", current_pos()};
            }
            number.push_back(c);
            is_decimal = true;
            input.remove_prefix(1);
        } else {
            break;
        }

        if (!is_decimal && number.size() > 15) {
            return Error{"Integer has more than 15 digits", current_pos()};
        }
        if (is_decimal && number.size() > 16) {
            return Error{"Decimal has more than 16 characters", current_pos()};
        }
    }

    if (!is_decimal) {
        int64_t value = 0;
        auto res = std::from_chars(number.data(), number.data() + number.size(), value);
        if (res.ec != std::errc()) {
            return Error{"failed to parse Integer digits", current_pos()};
        }
        return std::variant<int64_t, Decimal>{static_cast<int64_t>(sign) * value};
    }

    if (number.back() == '.') {
        return Error{"Decimal cannot end with '.'", current_pos()};
    }
    size_t dot_pos = number.find('.');
    size_t frac_digits = number.size() - dot_pos - 1;
    if (frac_digits > 3) {
        return Error{"Decimal has more than three fractional digits", current_pos()};
    }

    // Build the scaled-by-1000 integer directly from the digit string
    // (integer part * 1000 + fractional part padded to 3 digits), rather
    // than via floating point, to keep the exactness Decimal promises.
    std::string int_part = number.substr(0, dot_pos);
    std::string frac_part = number.substr(dot_pos + 1);
    while (frac_part.size() < 3) frac_part.push_back('0');

    int64_t int_value = 0;
    if (!int_part.empty()) {
        auto res = std::from_chars(int_part.data(), int_part.data() + int_part.size(), int_value);
        if (res.ec != std::errc()) return Error{"failed to parse Decimal integer part", current_pos()};
    }
    int64_t frac_value = 0;
    {
        auto res =
            std::from_chars(frac_part.data(), frac_part.data() + frac_part.size(), frac_value);
        if (res.ec != std::errc()) return Error{"failed to parse Decimal fractional part", current_pos()};
    }

    int64_t scaled = sign * (int_value * 1000 + frac_value);
    return std::variant<int64_t, Decimal>{Decimal::from_scaled(scaled)};
}

inline Result<bool> parse_boolean(std::string_view& input, size_t consumed_before) {
    if (input.empty() || input.front() != '?') {
        return Error{"Boolean must start with '?'", consumed_before};
    }
    input.remove_prefix(1);

    if (input.empty()) {
        return Error{"truncated Boolean", consumed_before + 1};
    }
    char c = input.front();
    if (c == '1') {
        input.remove_prefix(1);
        return true;
    }
    if (c == '0') {
        input.remove_prefix(1);
        return false;
    }
    return Error{"Boolean must be followed by '1' or '0'", consumed_before + 1};
}

inline Result<std::string> parse_key(std::string_view& input, size_t consumed_before) {
    if (input.empty() || !(is_lcalpha(input.front()) || input.front() == '*')) {
        return Error{"key must start with lcalpha or '*'", consumed_before};
    }
    std::string out;
    while (!input.empty()) {
        char c = input.front();
        bool key_char =
            is_lcalpha(c) || is_digit(c) || c == '_' || c == '-' || c == '.' || c == '*';
        if (!key_char) break;
        out.push_back(c);
        input.remove_prefix(1);
    }
    return out;
}

inline Result<BareItem> parse_bare_item(std::string_view& input, size_t consumed_before) {
    if (input.empty()) {
        return Error{"expected a bare item, got empty input", consumed_before};
    }
    char c = input.front();

    if (c == '-' || is_digit(c)) {
        auto r = parse_integer_or_decimal(input, consumed_before);
        if (!r) return r.error();
        return std::visit([](auto&& v) -> BareItem { return v; }, r.value());
    }
    if (c == '"') {
        auto r = parse_string(input, consumed_before);
        if (!r) return r.error();
        return BareItem{r.value()};
    }
    if (is_alpha(c) || c == '*') {
        auto r = parse_token(input, consumed_before);
        if (!r) return r.error();
        return BareItem{r.value()};
    }
    if (c == ':') {
        auto r = parse_byte_sequence(input, consumed_before);
        if (!r) return r.error();
        return BareItem{r.value()};
    }
    if (c == '?') {
        auto r = parse_boolean(input, consumed_before);
        if (!r) return r.error();
        return BareItem{r.value()};
    }
    if (c == '@') {
        auto r = parse_date(input, consumed_before);
        if (!r) return r.error();
        return BareItem{r.value()};
    }
    if (c == '%') {
        auto r = parse_display_string(input, consumed_before);
        if (!r) return r.error();
        return BareItem{r.value()};
    }
    return Error{"unrecognized bare item type", consumed_before};
}

inline Result<Parameters> parse_parameters(std::string_view& input, size_t consumed_before) {
    size_t start_len = input.size();
    auto current_pos = [&] { return consumed_before + (start_len - input.size()); };

    Parameters params;
    while (!input.empty() && input.front() == ';') {
        input.remove_prefix(1);
        discard_leading_sp(input);

        auto key_result = parse_key(input, current_pos());
        if (!key_result) return key_result.error();

        BareItem value{true};  // default true if no "=value" follows
        if (!input.empty() && input.front() == '=') {
            input.remove_prefix(1);
            auto value_result = parse_bare_item(input, current_pos());
            if (!value_result) return value_result.error();
            value = value_result.value();
        }

        params.set(key_result.value(), std::move(value));
    }
    return params;
}

inline Result<Item> parse_item(std::string_view& input, size_t consumed_before) {
    size_t start_len = input.size();
    auto current_pos = [&] { return consumed_before + (start_len - input.size()); };

    auto bare = parse_bare_item(input, consumed_before);
    if (!bare) return bare.error();

    auto params = parse_parameters(input, current_pos());
    if (!params) return params.error();

    Item item;
    item.value = bare.value();
    item.params = params.value();
    return item;
}

// --- 4.2.1.2: Parsing an Inner List ------------------------------------------
inline Result<InnerList> parse_inner_list(std::string_view& input, size_t consumed_before) {
    size_t start_len = input.size();
    auto current_pos = [&] { return consumed_before + (start_len - input.size()); };

    if (input.empty() || input.front() != '(') {
        return Error{"Inner List must start with '('", current_pos()};
    }
    input.remove_prefix(1);

    InnerList result;
    while (!input.empty()) {
        discard_leading_sp(input);

        if (!input.empty() && input.front() == ')') {
            input.remove_prefix(1);
            auto params = parse_parameters(input, current_pos());
            if (!params) return params.error();
            result.params = params.value();
            return result;
        }

        auto item = parse_item(input, current_pos());
        if (!item) return item.error();
        result.items.push_back(item.value());

        if (!input.empty() && input.front() != ' ' && input.front() != ')') {
            return Error{"Inner List items must be separated by a single SP", current_pos()};
        }
    }

    return Error{"unterminated Inner List (missing closing ')')", current_pos()};
}

// --- 4.2.1.1: Parsing an Item or Inner List ----------------------------------
inline Result<Member> parse_item_or_inner_list(std::string_view& input, size_t consumed_before) {
    if (!input.empty() && input.front() == '(') {
        auto inner = parse_inner_list(input, consumed_before);
        if (!inner) return inner.error();
        return Member{inner.value()};
    }
    auto item = parse_item(input, consumed_before);
    if (!item) return item.error();
    return Member{item.value()};
}

// --- 4.2.1: Parsing a List ----------------------------------------------------
inline Result<List> parse_list(std::string_view& input, size_t consumed_before) {
    size_t start_len = input.size();
    auto current_pos = [&] { return consumed_before + (start_len - input.size()); };

    List members;
    while (!input.empty()) {
        auto member = parse_item_or_inner_list(input, current_pos());
        if (!member) return member.error();
        members.push_back(member.value());

        discard_leading_ows(input);
        if (input.empty()) return members;

        char c = input.front();
        input.remove_prefix(1);
        if (c != ',') {
            return Error{"List members must be separated by ','", current_pos()};
        }
        discard_leading_ows(input);
        if (input.empty()) {
            return Error{"trailing comma in List", current_pos()};
        }
    }
    return members;  // empty input from the start: empty List
}

// --- 4.2.2: Parsing a Dictionary -----------------------------------------------
inline Result<Dictionary> parse_dictionary(std::string_view& input, size_t consumed_before) {
    size_t start_len = input.size();
    auto current_pos = [&] { return consumed_before + (start_len - input.size()); };

    Dictionary dictionary;
    while (!input.empty()) {
        auto key_result = parse_key(input, current_pos());
        if (!key_result) return key_result.error();
        std::string this_key = key_result.value();

        Member member;
        if (!input.empty() && input.front() == '=') {
            input.remove_prefix(1);
            auto member_result = parse_item_or_inner_list(input, current_pos());
            if (!member_result) return member_result.error();
            member = member_result.value();
        } else {
            // Bare key (no "=value"): Boolean true, per RFC 9651 3.2's
            // note that Dictionary (and Parameter) Boolean true is
            // conveyed by omitting the value entirely.
            auto params_result = parse_parameters(input, current_pos());
            if (!params_result) return params_result.error();
            member = Item{BareItem{true}, params_result.value()};
        }

        dictionary.set(std::move(this_key), std::move(member));

        discard_leading_ows(input);
        if (input.empty()) return dictionary;

        char c = input.front();
        input.remove_prefix(1);
        if (c != ',') {
            return Error{"Dictionary members must be separated by ','", current_pos()};
        }
        discard_leading_ows(input);
        if (input.empty()) {
            return Error{"trailing comma in Dictionary", current_pos()};
        }
    }
    return dictionary;  // empty input from the start: empty Dictionary
}

// --- 4.2.5: Parsing a String -----------------------------------------------
inline Result<std::string> parse_string(std::string_view& input, size_t consumed_before) {
    size_t start_len = input.size();
    auto current_pos = [&] { return consumed_before + (start_len - input.size()); };

    if (input.empty() || input.front() != '"') {
        return Error{"String must start with DQUOTE", current_pos()};
    }
    input.remove_prefix(1);

    std::string out;
    while (!input.empty()) {
        char c = input.front();
        input.remove_prefix(1);

        if (c == '\\') {
            if (input.empty()) {
                return Error{"truncated escape sequence in String", current_pos()};
            }
            char next = input.front();
            input.remove_prefix(1);
            if (next != '"' && next != '\\') {
                return Error{"String escape must be followed by DQUOTE or backslash",
                             current_pos()};
            }
            out.push_back(next);
        } else if (c == '"') {
            return out;
        } else if (static_cast<unsigned char>(c) <= 0x1F || static_cast<unsigned char>(c) >= 0x7F) {
            return Error{"String contains a disallowed control character", current_pos()};
        } else {
            out.push_back(c);
        }
    }

    return Error{"unterminated String (missing closing DQUOTE)", current_pos()};
}

// --- 4.2.6: Parsing a Token ------------------------------------------------
inline Result<Token> parse_token(std::string_view& input, size_t consumed_before) {
    size_t start_len = input.size();
    auto current_pos = [&] { return consumed_before + (start_len - input.size()); };

    if (input.empty() || !(is_alpha(input.front()) || input.front() == '*')) {
        return Error{"Token must start with ALPHA or '*'", current_pos()};
    }

    std::string out;
    while (!input.empty()) {
        char c = input.front();
        if (!(is_tchar(c) || c == ':' || c == '/')) break;
        out.push_back(c);
        input.remove_prefix(1);
    }
    return Token{out};
}

// --- 4.2.7: Parsing a Byte Sequence -----------------------------------------
inline Result<ByteSequence> parse_byte_sequence(std::string_view& input, size_t consumed_before) {
    size_t start_len = input.size();
    auto current_pos = [&] { return consumed_before + (start_len - input.size()); };

    if (input.empty() || input.front() != ':') {
        return Error{"Byte Sequence must start with ':'", current_pos()};
    }
    input.remove_prefix(1);

    size_t end = input.find(':');
    if (end == std::string_view::npos) {
        return Error{"unterminated Byte Sequence (missing closing ':')", current_pos()};
    }

    std::string_view b64_content = input.substr(0, end);
    for (char c : b64_content) {
        bool ok = is_alpha(c) || is_digit(c) || c == '+' || c == '/' || c == '=';
        if (!ok) {
            return Error{"Byte Sequence contains a character outside the base64 alphabet",
                         current_pos()};
        }
    }

    auto decoded = decode_base64(b64_content);
    if (!decoded) return decoded.error();

    input.remove_prefix(end + 1);  // consume content + closing ':'
    return ByteSequence{decoded.value()};
}

// --- 4.2.9: Parsing a Date --------------------------------------------------
inline Result<Date> parse_date(std::string_view& input, size_t consumed_before) {
    size_t start_len = input.size();
    auto current_pos = [&] { return consumed_before + (start_len - input.size()); };

    if (input.empty() || input.front() != '@') {
        return Error{"Date must start with '@'", current_pos()};
    }
    input.remove_prefix(1);

    auto number = parse_integer_or_decimal(input, current_pos());
    if (!number) return number.error();

    if (std::holds_alternative<Decimal>(number.value())) {
        return Error{"Date's value must be an Integer, not a Decimal", current_pos()};
    }
    return Date{std::get<int64_t>(number.value())};
}

// --- 4.2.10: Parsing a Display String ---------------------------------------
inline Result<DisplayString> parse_display_string(std::string_view& input,
                                                    size_t consumed_before) {
    size_t start_len = input.size();
    auto current_pos = [&] { return consumed_before + (start_len - input.size()); };

    if (input.size() < 2 || input[0] != '%' || input[1] != '"') {
        return Error{"Display String must start with '%\"'", current_pos()};
    }
    input.remove_prefix(2);

    std::string byte_array;
    while (!input.empty()) {
        char c = input.front();
        input.remove_prefix(1);

        if (static_cast<unsigned char>(c) <= 0x1F || static_cast<unsigned char>(c) >= 0x7F) {
            return Error{"Display String contains a disallowed control character",
                         current_pos()};
        }

        if (c == '%') {
            if (input.size() < 2) {
                return Error{"truncated percent-encoding in Display String", current_pos()};
            }
            char h0 = input[0], h1 = input[1];
            auto hex_val = [](char h) -> int {
                if (h >= '0' && h <= '9') return h - '0';
                if (h >= 'a' && h <= 'f') return h - 'a' + 10;
                return -1;  // uppercase A-F is deliberately rejected: the RFC requires
                            // lowercase specifically ("it is not in 0-9 or lowercase a-f")
            };
            int v0 = hex_val(h0), v1 = hex_val(h1);
            if (v0 < 0 || v1 < 0) {
                return Error{"percent-encoding must use lowercase hex digits", current_pos()};
            }
            input.remove_prefix(2);
            byte_array.push_back(static_cast<char>((v0 << 4) | v1));
        } else if (c == '"') {
            if (!is_valid_utf8(byte_array)) {
                return Error{"Display String content is not valid UTF-8", current_pos()};
            }
            return DisplayString{byte_array};
        } else {
            byte_array.push_back(c);
        }
    }

    return Error{"unterminated Display String (missing closing DQUOTE)", current_pos()};
}

}  // namespace detail

}  // namespace sfv

// ============================================================
// serializer.hpp
// ============================================================
// serializer.hpp
//
// Implements RFC 9651 Section 4.1, "Serializing Structured Fields":
//   https://www.rfc-editor.org/rfc/rfc9651.html#section-4.1
//
// Mirrors parser.hpp's structure: each function matches one numbered
// subsection of 4.1 by name, so the RFC and implementation can be read
// side by side.


#include <cstdint>
#include <string>


namespace sfv {
namespace detail {

// --- 4.1.4: Serializing an Integer ----------------------------------------
inline Result<std::string> serialize_integer(int64_t value) {
    constexpr int64_t kMax = 999'999'999'999'999;
    constexpr int64_t kMin = -999'999'999'999'999;
    if (value > kMax || value < kMin) {
        return Error{"Integer out of range [-999999999999999, 999999999999999]", 0};
    }
    return std::to_string(value);
}

// --- 4.1.5: Serializing a Decimal -----------------------------------------
//
// Operates directly on the fixed-point `scaled` representation (see
// Decimal's own comment in types.hpp for why): no rounding step is
// needed here because Decimal, by construction, already holds at most
// three fractional digits -- the rounding the RFC describes in step 2
// only matters for a hypothetical wider-precision input type, which this
// library's Decimal deliberately doesn't allow to exist in the first
// place.
inline Result<std::string> serialize_decimal(Decimal d) {
    int64_t scaled = d.scaled;
    bool negative = scaled < 0;
    uint64_t magnitude = negative ? static_cast<uint64_t>(-scaled) : static_cast<uint64_t>(scaled);

    uint64_t int_part = magnitude / 1000;
    uint64_t frac_part = magnitude % 1000;

    // "more than 12 significant digits to the left of the decimal point"
    if (int_part > 999'999'999'999ULL) {
        return Error{"Decimal integer part exceeds 12 digits", 0};
    }

    std::string out;
    if (negative) out.push_back('-');
    out += std::to_string(int_part);
    out.push_back('.');

    if (frac_part == 0) {
        out.push_back('0');
    } else {
        // Significant fractional digits only, per RFC step 9 ("append
        // the significant digits") -- e.g. scaled fractional 500 (from
        // 4.500) serializes as "5", not "500"; 40 (from 4.040)
        // serializes as "04"... but note the RFC's own examples (Section
        // 3.3.2) show trailing zeros CAN be part of canonical output is
        // NOT required to trim beyond what's "significant" -- to match
        // real-world implementations and the official test suite, this
        // prints the full three-digit zero-padded fractional part with
        // only trailing zeros trimmed, which is what "significant
        // digits" means here (e.g. 4.5 -> "500" trimmed to "5"; 4.05 ->
        // "050" trimmed to "05", keeping the meaningful leading zero).
        std::string frac_str = std::to_string(frac_part);
        while (frac_str.size() < 3) frac_str.insert(frac_str.begin(), '0');
        while (frac_str.size() > 1 && frac_str.back() == '0') frac_str.pop_back();
        out += frac_str;
    }

    return out;
}

// --- 4.1.9: Serializing a Boolean -----------------------------------------
inline Result<std::string> serialize_boolean(bool value) {
    return std::string(value ? "?1" : "?0");
}

// --- 4.1.6: Serializing a String -------------------------------------------
inline Result<std::string> serialize_string(const std::string& value) {
    for (char raw_c : value) {
        unsigned char c = static_cast<unsigned char>(raw_c);
        if (c <= 0x1F || c >= 0x7F) {
            return Error{"String contains a character outside VCHAR/SP", 0};
        }
    }
    std::string out;
    out.push_back('"');
    for (char c : value) {
        if (c == '\\' || c == '"') out.push_back('\\');
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}
// --- 4.1.7: Serializing a Token --------------------------------------------
inline Result<std::string> serialize_token(const Token& value) {
    const std::string& s = value.value;
    if (s.empty() || !(is_alpha(s.front()) || s.front() == '*')) {
        return Error{"Token must start with ALPHA or '*'", 0};
    }
    for (char c : s) {
        if (!(is_tchar(c) || c == ':' || c == '/')) {
            return Error{"Token contains a character outside tchar/':'/'/'", 0};
        }
    }
    return s;
}
// --- 4.1.8: Serializing a Byte Sequence -------------------------------------
inline Result<std::string> serialize_byte_sequence(const ByteSequence& value) {
    std::string out;
    out.push_back(':');
    out += encode_base64(value.bytes);
    out.push_back(':');
    return out;
}
// --- 4.1.10: Serializing a Date ---------------------------------------------
inline Result<std::string> serialize_date(Date d) {
    auto int_result = serialize_integer(d.value);
    if (!int_result) return int_result.error();
    return "@" + int_result.value();
}
// --- 4.1.11: Serializing a Display String -----------------------------------
inline Result<std::string> serialize_display_string(const DisplayString& value) {
    if (!is_valid_utf8(value.utf8)) {
        return Error{"Display String content is not valid UTF-8", 0};
    }

    static constexpr char kHexDigits[] = "0123456789abcdef";
    std::string out = "%\"";
    for (char raw_byte : value.utf8) {
        unsigned char byte = static_cast<unsigned char>(raw_byte);
        if (byte == 0x25 || byte == 0x22 || byte <= 0x1F || byte >= 0x7F) {
            out.push_back('%');
            out.push_back(kHexDigits[(byte >> 4) & 0xF]);
            out.push_back(kHexDigits[byte & 0xF]);
        } else {
            out.push_back(static_cast<char>(byte));
        }
    }
    out.push_back('"');
    return out;
}

// --- 4.1.3.1: Serializing a Bare Item --------------------------------------
inline Result<std::string> serialize_bare_item(const BareItem& item) {
    return std::visit(
        [](auto&& v) -> Result<std::string> {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, int64_t>) return serialize_integer(v);
            else if constexpr (std::is_same_v<T, Decimal>) return serialize_decimal(v);
            else if constexpr (std::is_same_v<T, std::string>) return serialize_string(v);
            else if constexpr (std::is_same_v<T, Token>) return serialize_token(v);
            else if constexpr (std::is_same_v<T, ByteSequence>) return serialize_byte_sequence(v);
            else if constexpr (std::is_same_v<T, bool>) return serialize_boolean(v);
            else if constexpr (std::is_same_v<T, Date>) return serialize_date(v);
            else if constexpr (std::is_same_v<T, DisplayString>) return serialize_display_string(v);
            else {
                static_assert(!sizeof(T*), "unhandled BareItem alternative");
                return Error{"unreachable", 0};
            }
        },
        item);
}

// --- 4.1.1.3: Serializing a Key --------------------------------------------
inline Result<std::string> serialize_key(const std::string& key) {
    if (key.empty() || !(is_lcalpha(key.front()) || key.front() == '*')) {
        return Error{"key must start with lcalpha or '*'", 0};
    }
    for (char c : key) {
        bool ok = is_lcalpha(c) || is_digit(c) || c == '_' || c == '-' || c == '.' || c == '*';
        if (!ok) return Error{"key contains a character outside lcalpha/DIGIT/_/-/./* ", 0};
    }
    return key;
}

// --- 4.1.1.2: Serializing Parameters ---------------------------------------
inline Result<std::string> serialize_parameters(const Parameters& params) {
    std::string out;
    for (const auto& [key, value] : params) {
        out.push_back(';');
        auto key_result = serialize_key(key);
        if (!key_result) return key_result.error();
        out += key_result.value();

        bool is_bool_true = std::holds_alternative<bool>(value) && std::get<bool>(value);
        if (!is_bool_true) {
            out.push_back('=');
            auto value_result = serialize_bare_item(value);
            if (!value_result) return value_result.error();
            out += value_result.value();
        }
    }
    return out;
}

// --- 4.1.3: Serializing an Item ---------------------------------------------
inline Result<std::string> serialize_item(const Item& item) {
    auto bare_result = serialize_bare_item(item.value);
    if (!bare_result) return bare_result.error();

    auto params_result = serialize_parameters(item.params);
    if (!params_result) return params_result.error();

    return bare_result.value() + params_result.value();
}

// --- 4.1.1.1: Serializing an Inner List --------------------------------------
inline Result<std::string> serialize_inner_list(const InnerList& inner) {
    std::string out = "(";
    for (size_t i = 0; i < inner.items.size(); ++i) {
        auto item_result = serialize_item(inner.items[i]);
        if (!item_result) return item_result.error();
        out += item_result.value();
        if (i + 1 < inner.items.size()) out.push_back(' ');
    }
    out.push_back(')');

    auto params_result = serialize_parameters(inner.params);
    if (!params_result) return params_result.error();
    out += params_result.value();

    return out;
}

// Dispatches a List/Dictionary Member (Item or Inner List) to whichever
// serializer applies. Shared between Serializing a List (4.1.1) and
// Serializing a Dictionary (4.1.2), both of which define their members
// exactly this way ("If member_value is an array, append the result of
// running Serializing an Inner List... Otherwise, ... Serializing an
// Item").
inline Result<std::string> serialize_member(const Member& member) {
    return std::visit(
        [](auto&& v) -> Result<std::string> {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, Item>) return serialize_item(v);
            else if constexpr (std::is_same_v<T, InnerList>) return serialize_inner_list(v);
            else {
                static_assert(!sizeof(T*), "unhandled Member alternative");
                return Error{"unreachable", 0};
            }
        },
        member);
}

// --- 4.1.1: Serializing a List ------------------------------------------------
inline Result<std::string> serialize_list(const List& list) {
    std::string out;
    for (size_t i = 0; i < list.size(); ++i) {
        auto member_result = serialize_member(list[i]);
        if (!member_result) return member_result.error();
        out += member_result.value();
        if (i + 1 < list.size()) out += ", ";
    }
    return out;
}

// --- 4.1.2: Serializing a Dictionary --------------------------------------------
inline Result<std::string> serialize_dictionary(const Dictionary& dict) {
    std::string out;
    size_t i = 0;
    for (const auto& [key, member] : dict) {
        auto key_result = serialize_key(key);
        if (!key_result) return key_result.error();
        out += key_result.value();

        // "If member_value is Boolean true" -- member here is always an
        // Item or InnerList (Member variant); the Boolean-true shorthand
        // only ever applies to the Item case (an InnerList is never
        // itself "true"), so check that specifically.
        bool member_is_bare_boolean_true = false;
        if (std::holds_alternative<Item>(member)) {
            const Item& item = std::get<Item>(member);
            if (std::holds_alternative<bool>(item.value) && std::get<bool>(item.value)) {
                member_is_bare_boolean_true = true;
            }
        }

        if (member_is_bare_boolean_true) {
            const Item& item = std::get<Item>(member);
            auto params_result = serialize_parameters(item.params);
            if (!params_result) return params_result.error();
            out += params_result.value();
        } else {
            out.push_back('=');
            auto member_result = serialize_member(member);
            if (!member_result) return member_result.error();
            out += member_result.value();
        }

        if (++i < dict.size()) out += ", ";
    }
    return out;
}

}  // namespace detail
}  // namespace sfv

// ============================================================
// api.hpp
// ============================================================
// api.hpp
//
// The top-level entry points from RFC 9651 sections 4.1 and 4.2 -- the
// functions a real caller actually wants, as opposed to the per-
// container algorithms in parser.hpp/serializer.hpp (sfv::detail::
// parse_list, etc.), which are one level down and don't handle leading/
// trailing whitespace or confirm the whole input was consumed.
//
// RFC 9651 Appendix B: "A generic implementation of this specification
// should expose the top-level serialize (Section 4.1) and parse
// (Section 4.2) functions. They need not be functions; for example, it
// could be implemented as an object, with methods for each of the
// different top-level types." This header takes exactly that approach:
// one parse function per field_type (List/Dictionary/Item are distinct
// C++ types here, so field_type is encoded in which function you call,
// not a runtime enum), and one overloaded `serialize()` per type.
//
// Parsing, RFC 9651 section 4.2 steps, applied by each of the three
// functions below:
//   1. (Byte-to-ASCII conversion is the caller's job -- these take
//      std::string_view, which callers already control the encoding of.)
//   2. Discard leading SP.
//   3-5. Dispatch to the matching Parsing a <Type> algorithm (parser.hpp).
//   6. Discard leading SP again (i.e. trailing whitespace after the
//      parsed value).
//   7. If input is not now empty, fail -- there was unparsed trailing
//      content.
//   8. Return the parsed value.
//
// Serializing, RFC 9651 section 4.1: steps 2-5 (dispatch by type) are
// just calling the matching serialize_<type> from serializer.hpp; step 1
// ("if empty List/Dictionary, do not serialize the field at all") isn't
// a distinct code path here because Serializing a List/Dictionary
// already naturally produces "" for an empty container -- the "omit the
// field" behavior is about whether a CALLER emits an HTTP header line at
// all, which is outside this library's scope (it produces field VALUES,
// not header lines); a caller can simply check `list.empty()` /
// `dict.empty()` themselves before deciding to emit anything.


#include <string_view>


namespace sfv {

namespace detail {

// Shared by all three top-level parse functions: steps 2, 6, 7 of
// RFC 9651 section 4.2, wrapped around whichever step 3-5 algorithm the
// caller passes in.
template <typename T, typename ParseFn>
Result<T> parse_field(std::string_view input, ParseFn&& parse_fn) {
    size_t original_size = input.size();
    auto current_pos = [&] { return original_size - input.size(); };

    discard_leading_sp(input);

    auto result = parse_fn(input, current_pos());
    if (!result) return result.error();

    discard_leading_sp(input);
    if (!input.empty()) {
        return Error{"trailing data after the parsed value", current_pos()};
    }
    return result.value();
}

}  // namespace detail

// Top-level "parse this field value as a List" (RFC 9651 section 4.2,
// field_type == "list").
inline Result<List> parse_list(std::string_view input) {
    return detail::parse_field<List>(
        input, [](std::string_view& in, size_t pos) { return detail::parse_list(in, pos); });
}

// Top-level "parse this field value as a Dictionary" (field_type ==
// "dictionary").
inline Result<Dictionary> parse_dictionary(std::string_view input) {
    return detail::parse_field<Dictionary>(
        input,
        [](std::string_view& in, size_t pos) { return detail::parse_dictionary(in, pos); });
}

// Top-level "parse this field value as an Item" (field_type == "item").
inline Result<Item> parse_item(std::string_view input) {
    return detail::parse_field<Item>(
        input, [](std::string_view& in, size_t pos) { return detail::parse_item(in, pos); });
}

// Top-level serialize, overloaded on which of the three top-level types
// is given -- see this header's comment for why "if empty, omit the
// field" isn't a separate code path here.
inline Result<std::string> serialize(const List& list) { return detail::serialize_list(list); }
inline Result<std::string> serialize(const Dictionary& dict) {
    return detail::serialize_dictionary(dict);
}
inline Result<std::string> serialize(const Item& item) { return detail::serialize_item(item); }

}  // namespace sfv

#endif  // SFV_SINGLE_INCLUDE_HPP
