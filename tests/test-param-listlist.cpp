// test-param-listlist.cpp
//
// Runs the official structured-field-tests param-listlist.json
// conformance cases (all 3) through parse_list()/serialize_list() --
// these specifically exercise Parameters at both the Inner List level
// and on individual Items within an Inner List simultaneously.

#include "gauge15/sfv.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

#include "generated/param-listlist-cases.inc"

namespace {

int g_failures = 0;
int g_checks = 0;

void run_case(const GeneratedTestCase& tc) {
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
        std::cerr << "FAIL [" << tc.name << "]: serialize mismatch, got '"
                  << (ser.ok() ? ser.value() : ser.error().message) << "' want '" << tc.canonical
                  << "'\n";
    }
}

}  // namespace

int main() {
    for (const auto& tc : kParamListListTestCases()) {
        run_case(tc);
    }
    std::cout << (g_checks - g_failures) << "/" << g_checks
              << " checks passed (param-listlist.json conformance)\n";
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
