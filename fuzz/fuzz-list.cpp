// fuzz-list.cpp
//
// libFuzzer target for sfv::parse_list(). See fuzz-item.cpp's header
// comment for the two properties checked (crash-freedom, and parse ->
// serialize -> reparse idempotency); this is the same harness shape
// applied to List/Inner List instead of Item.

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <string_view>

#include "gauge15/sfv.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string_view input(reinterpret_cast<const char*>(data), size);

    auto result = sfv::parse_list(input);
    if (!result.ok()) return 0;

    auto serialized = sfv::serialize(result.value());
    if (!serialized.ok()) {
        assert(false && "parsed successfully but failed to serialize");
        return 0;
    }

    auto reparsed = sfv::parse_list(serialized.value());
    assert(reparsed.ok() && "serialized output of a successful parse must itself parse");
    assert(reparsed.value() == result.value() &&
           "re-parsing serialized output must produce an equal value");

    return 0;
}
