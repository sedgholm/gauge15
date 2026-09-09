// fuzz-item.cpp
//
// libFuzzer target for sfv::parse_item(), the top-level Item entry
// point. Two properties are checked, not just "doesn't crash":
//
//   1. No crash, no UBSan/ASan finding, on ANY byte sequence libFuzzer
//      generates -- including invalid UTF-8, embedded NULs, arbitrary
//      binary garbage. This is the main point: parse_item() must reject
//      malformed input cleanly (return an Error), never crash on it.
//
//   2. If parsing succeeds, serializing the result and re-parsing THAT
//      must also succeed and must produce an equal value (parse . serialize
//      is idempotent from the second application onward). This is a
//      genuinely meaningful correctness property beyond "doesn't crash" --
//      it would catch, for example, a serializer that produces syntax its
//      own parser can't read back, or a parser that accepts something it
//      then serializes into a DIFFERENT value.
//
// Build (see fuzz/README.md for the exact commands):
//   clang++ -std=c++20 -Iinclude -fsanitize=fuzzer,address,undefined \
//     fuzz/fuzz-item.cpp -o fuzz/fuzz-item
// Run:
//   ./fuzz/fuzz-item fuzz/corpus/item -max_total_time=120

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <string_view>

#include "gauge15/sfv.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string_view input(reinterpret_cast<const char*>(data), size);

    auto result = sfv::parse_item(input);
    if (!result.ok()) return 0;  // rejecting malformed input is success, not a finding

    auto serialized = sfv::serialize(result.value());
    if (!serialized.ok()) {
        // A value that parsed successfully but can't be serialized back
        // would be a real bug (every field the parser can produce should
        // be representable on the wire) -- but Display String's UTF-8
        // validity check is enforced on BOTH parse and serialize
        // identically, and every other type's serializer only fails on
        // states the parser itself can't produce (e.g. an Integer
        // outside the representable range) -- so reaching here at all
        // indicates a genuine parser/serializer mismatch worth
        // investigating, not a case to silently accept.
        assert(false && "parsed successfully but failed to serialize");
        return 0;
    }

    auto reparsed = sfv::parse_item(serialized.value());
    assert(reparsed.ok() && "serialized output of a successful parse must itself parse");
    assert(reparsed.value() == result.value() &&
           "re-parsing serialized output must produce an equal value");

    return 0;
}
