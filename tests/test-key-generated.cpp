// test-key-generated.cpp
//
// Runs the official structured-field-tests key-generated.json
// conformance cases -- the largest single conformance file (640 cases
// total) -- through both parse_dictionary()/serialize_dictionary() (384
// dictionary-typed cases) and parse_list()/serialize_list() (256
// list-typed cases). These specifically exercise Key parsing edge cases
// (RFC 9651 4.2.3.3): valid/invalid characters, length limits, and the
// lcalpha-only (no uppercase) rule.

#include "gauge15/sfv.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace dict_cases {
#include "generated/key-generated-dict-cases.inc"
}  // namespace dict_cases

namespace list_cases {
#include "generated/key-generated-list-cases.inc"
}  // namespace list_cases

namespace {

int g_failures = 0;
int g_checks = 0;

void run_dict_case(const dict_cases::GeneratedTestCase& tc) {
    std::string_view input(tc.raw, tc.raw_len);
        auto parse_result = sfv::parse_dictionary(input);

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
    for (const auto& tc : dict_cases::kKeyGeneratedDictTestCases()) {
        run_dict_case(tc);
    }
    for (const auto& tc : list_cases::kKeyGeneratedListTestCases()) {
        run_list_case(tc);
    }
    std::cout << (g_checks - g_failures) << "/" << g_checks
              << " checks passed (key-generated.json conformance, all 640 cases)\n";
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
