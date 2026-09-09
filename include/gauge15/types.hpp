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

#ifndef SFV_TYPES_HPP
#define SFV_TYPES_HPP

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

#endif  // SFV_TYPES_HPP
