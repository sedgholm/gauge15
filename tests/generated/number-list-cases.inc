// GENERATED from tests/vectors/number.json (kind=list) by tools/gen-test-cases.py. DO NOT EDIT.
// 3 of 37 official test cases included in this phase
// (34 non-list header_type, 0 unsupported type, deferred to a later phase).

struct GeneratedTestCase {
    const char* name;
    const char* raw;
    size_t raw_len;  // use this, not strlen(raw) -- raw may contain embedded NUL
    bool must_fail;
    sfv::List expected;  // meaningful only if !must_fail
    const char* canonical;  // meaningful only if !must_fail
    size_t canonical_len;  // same NUL caveat as raw_len
};

inline const std::vector<GeneratedTestCase>& kNumberListTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"long integer followed by comma", "123456789012345, 1", 18, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{int64_t{123456789012345}}, []{ sfv::Parameters p; return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}}, "123456789012345, 1", 18},
        {"too long integer followed by comma", "1234567890123456, 1", 19, true, sfv::List{}, "", 0},
        {"decimal, followed by comma", "123456789012.123, 1.1", 21, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Decimal::from_scaled(123456789012123)}, []{ sfv::Parameters p; return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{sfv::Decimal::from_scaled(1100)}, []{ sfv::Parameters p; return p; }()}}}, "123456789012.123, 1.1", 21},
    };
    return cases;
}
