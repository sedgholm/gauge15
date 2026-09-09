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

#ifndef SFV_DETAIL_UTF8_HPP
#define SFV_DETAIL_UTF8_HPP

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

#endif  // SFV_DETAIL_UTF8_HPP
