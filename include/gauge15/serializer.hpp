// serializer.hpp
//
// Implements RFC 9651 Section 4.1, "Serializing Structured Fields":
//   https://www.rfc-editor.org/rfc/rfc9651.html#section-4.1
//
// Mirrors parser.hpp's structure: each function matches one numbered
// subsection of 4.1 by name, so the RFC and implementation can be read
// side by side.

#ifndef SFV_SERIALIZER_HPP
#define SFV_SERIALIZER_HPP

#include <cstdint>
#include <string>

#include "gauge15/detail/base64.hpp"
#include "gauge15/detail/utf8.hpp"
#include "gauge15/parser.hpp"  // for is_lcalpha/is_digit/is_alpha/is_tchar character-class predicates
#include "gauge15/result.hpp"
#include "gauge15/types.hpp"

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

#endif  // SFV_SERIALIZER_HPP
