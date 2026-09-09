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

#ifndef SFV_API_HPP
#define SFV_API_HPP

#include <string_view>

#include "gauge15/parser.hpp"
#include "gauge15/result.hpp"
#include "gauge15/serializer.hpp"
#include "gauge15/types.hpp"

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

#endif  // SFV_API_HPP
