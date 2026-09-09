// test-api.cpp
//
// Tests the specific behaviors that belong to the TOP-LEVEL parse/
// serialize functions (api.hpp) rather than the per-container algorithms
// underneath them: leading/trailing SP tolerance (RFC 9651 4.2 steps 2
// and 6) and the full-consumption check (step 7). The official
// conformance files exercise these somewhat (e.g. list.json's "leading
// SP list"), but this file isolates them explicitly and covers all
// three top-level types (List, Dictionary, Item) uniformly, since a
// per-file conformance test wouldn't necessarily cover the same edge
// case for all three.

#include "gauge15/sfv.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int g_failures = 0;
int g_checks = 0;

void expect(const std::string& what, bool actual, bool expected) {
    ++g_checks;
    if (actual != expected) {
        ++g_failures;
        std::cerr << "FAIL [" << what << "]: expected " << expected << ", got " << actual
                  << "\n";
    }
}

void expect_eq(const std::string& what, const std::string& actual, const std::string& expected) {
    ++g_checks;
    if (actual != expected) {
        ++g_failures;
        std::cerr << "FAIL [" << what << "]: expected '" << expected << "', got '" << actual
                  << "'\n";
    }
}

void expect_position(const std::string& what, const sfv::Result<sfv::Item>& result,
                     size_t expected) {
    ++g_checks;
    if (result.ok()) {
        ++g_failures;
        std::cerr << "FAIL [" << what << "]: expected parse failure, but succeeded\n";
        return;
    }
    if (result.error().position != expected) {
        ++g_failures;
        std::cerr << "FAIL [" << what << "]: expected error at " << expected << ", got "
                  << result.error().position << "\n";
    }
}

}  // namespace

int main() {
    // --- Leading SP tolerance, all three top-level types ---
    expect("leading-sp/list", sfv::parse_list("  42, 43").ok(), true);
    expect("leading-sp/dictionary", sfv::parse_dictionary("  a=1").ok(), true);
    expect("leading-sp/item", sfv::parse_item("  42").ok(), true);

    // --- Trailing SP tolerance, all three top-level types ---
    expect("trailing-sp/list", sfv::parse_list("42, 43  ").ok(), true);
    expect("trailing-sp/dictionary", sfv::parse_dictionary("a=1  ").ok(), true);
    expect("trailing-sp/item", sfv::parse_item("42  ").ok(), true);

    // Leading/trailing SP doesn't change the parsed VALUE, only whether
    // it's accepted -- confirm the value itself is identical either way.
    {
        auto with_sp = sfv::parse_item("  42  ");
        auto without_sp = sfv::parse_item("42");
        expect("leading-trailing-sp/value-unchanged",
               with_sp.ok() && without_sp.ok() && with_sp.value() == without_sp.value(), true);
    }

    // --- Full-consumption check: trailing non-SP content must fail, for
    // all three top-level types (this is NOT the same as trailing SP,
    // which is tolerated -- this is genuine leftover content) ---
    expect("trailing-garbage/list", sfv::parse_list("42, 43 garbage").ok(), false);
    expect("trailing-garbage/dictionary", sfv::parse_dictionary("a=1 garbage").ok(), false);
    expect("trailing-garbage/item", sfv::parse_item("42 garbage").ok(), false);

    // A comma right after a valid Item is trailing garbage at the Item
    // level (Items don't have comma syntax at all) -- distinguishes
    // "extra content after a complete value" from "malformed value".
    expect("trailing-garbage/item-comma", sfv::parse_item("42,").ok(), false);

    // Numeric signs: RFC 9651 allows a leading '-' for Integer/Decimal
    // and Date values, but there is no leading '+' form.
    expect("number-sign/minus-integer", sfv::parse_item("-42").ok(), true);
    expect("number-sign/plus-integer-fails", sfv::parse_item("+42").ok(), false);
    expect("number-sign/minus-date", sfv::parse_item("@-42").ok(), true);
    expect("number-sign/plus-date-fails", sfv::parse_item("@+42").ok(), false);

    // Error offsets should point at the byte where the failure is
    // detected, including after top-level leading SP and inside
    // parameter parsing.
    expect_position("error-position/trailing-garbage", sfv::parse_item("  42 garbage"), 5);
    expect_position("error-position/parameter-key", sfv::parse_item("42; =1"), 4);
    expect_position("error-position/parameter-value", sfv::parse_item("42;foo=+"), 7);

    // --- serialize() overload resolution and round-tripping ---
    {
        auto list_result = sfv::parse_list("a, b, c");
        auto ser = sfv::serialize(list_result.value());
        expect_eq("serialize/list-roundtrip", ser.value(), "a, b, c");
    }
    {
        auto dict_result = sfv::parse_dictionary("a=1, b=2");
        auto ser = sfv::serialize(dict_result.value());
        expect_eq("serialize/dictionary-roundtrip", ser.value(), "a=1, b=2");
    }
    {
        auto item_result = sfv::parse_item("42;foo=bar");
        auto ser = sfv::serialize(item_result.value());
        expect_eq("serialize/item-roundtrip", ser.value(), "42;foo=bar");
    }

    // --- Empty List/Dictionary serialize to the empty string (the
    // "omit this field entirely" signal a caller acts on -- see
    // api.hpp's header comment for why that's the caller's job, not a
    // special return value from serialize() itself) ---
    {
        auto ser = sfv::serialize(sfv::List{});
        expect_eq("serialize/empty-list", ser.value(), "");
    }
    {
        auto ser = sfv::serialize(sfv::Dictionary{});
        expect_eq("serialize/empty-dictionary", ser.value(), "");
    }

    // --- An all-whitespace input is empty-after-trimming for List/
    // Dictionary (valid, empty container) but invalid for Item (an Item
    // always needs a bare item; there's nothing there to parse) ---
    expect("all-whitespace/list-is-empty", sfv::parse_list("   ").ok(), true);
    expect("all-whitespace/item-fails", sfv::parse_item("   ").ok(), false);

    std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed (top-level API)\n";
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
