// test-token.cpp
//
// Runs the official structured-field-tests token.json conformance cases
// through parse_item()/serialize_item() (3 Item-typed cases) AND
// parse_list()/serialize_list() (3 List-typed cases) -- together, ALL 6
// of the file's cases, closing out what Phase 2 originally deferred.

#include "gauge15/sfv.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace item_cases {
#include "generated/token-cases.inc"
}  // namespace item_cases

namespace list_cases {
#include "generated/token-list-cases.inc"
}  // namespace list_cases

namespace {

int g_failures = 0;
int g_checks = 0;

void run_item_case(const item_cases::GeneratedTestCase& tc) {
    std::string_view input(tc.raw, tc.raw_len);
    auto parse_result = sfv::parse_item(input);

    if (tc.must_fail) {
        ++g_checks;
        bool fully_ok = parse_result.ok();
        if (fully_ok) {
            ++g_failures;
            std::cerr << "FAIL [" << tc.name << "]: expected parse failure, but succeeded\n";
        }
        return;
    }

    ++g_checks;
    if (!parse_result.ok()) {
        ++g_failures;
        std::cerr << "FAIL [" << tc.name << "]: expected success, got failure\n";
        return;
    }

    ++g_checks;
    if (!(parse_result.value() == tc.expected)) {
        ++g_failures;
        std::cerr << "FAIL [" << tc.name << "]: parsed value does not match expected\n";
    }

    ++g_checks;
    auto ser = sfv::serialize(tc.expected);
    if (!ser.ok() || ser.value() != std::string_view(tc.canonical, tc.canonical_len)) {
        ++g_failures;
        std::cerr << "FAIL [" << tc.name << "]: serialize mismatch\n";
    }
}

void run_list_case(const list_cases::GeneratedTestCase& tc) {
    std::string_view input(tc.raw, tc.raw_len);
    auto parse_result = sfv::parse_list(input);

    if (tc.must_fail) {
        ++g_checks;
        bool fully_ok = parse_result.ok();
        if (fully_ok) {
            ++g_failures;
            std::cerr << "FAIL [" << tc.name << "]: expected parse failure, but succeeded\n";
        }
        return;
    }

    ++g_checks;
    if (!parse_result.ok()) {
        ++g_failures;
        std::cerr << "FAIL [" << tc.name << "]: expected success, got failure\n";
        return;
    }

    ++g_checks;
    if (!(parse_result.value() == tc.expected)) {
        ++g_failures;
        std::cerr << "FAIL [" << tc.name << "]: parsed value does not match expected\n";
    }

    ++g_checks;
    auto ser = sfv::serialize(tc.expected);
    if (!ser.ok() || ser.value() != std::string_view(tc.canonical, tc.canonical_len)) {
        ++g_failures;
        std::cerr << "FAIL [" << tc.name << "]: serialize mismatch\n";
    }
}

}  // namespace

int main() {
    for (const auto& tc : item_cases::kTokenTestCases()) {
        run_item_case(tc);
    }
    for (const auto& tc : list_cases::kTokenListTestCases()) {
        run_list_case(tc);
    }
    std::cout << (g_checks - g_failures) << "/" << g_checks
              << " checks passed (token.json conformance, all 6 cases)\n";
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
