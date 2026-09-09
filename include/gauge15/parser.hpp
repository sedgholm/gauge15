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

#ifndef SFV_PARSER_HPP
#define SFV_PARSER_HPP

#include <charconv>
#include <cstdint>
#include <string_view>

#include "gauge15/detail/base64.hpp"
#include "gauge15/detail/utf8.hpp"
#include "gauge15/result.hpp"
#include "gauge15/types.hpp"

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

#endif  // SFV_PARSER_HPP
