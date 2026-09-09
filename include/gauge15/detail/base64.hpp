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

#ifndef SFV_DETAIL_BASE64_HPP
#define SFV_DETAIL_BASE64_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "gauge15/result.hpp"

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

#endif  // SFV_DETAIL_BASE64_HPP
