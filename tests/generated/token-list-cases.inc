// GENERATED from tests/vectors/token.json (kind=list) by tools/gen-test-cases.py. DO NOT EDIT.
// 3 of 6 official test cases included in this phase
// (3 non-list header_type, 0 unsupported type, deferred to a later phase).

struct GeneratedTestCase {
    const char* name;
    const char* raw;
    size_t raw_len;  // use this, not strlen(raw) -- raw may contain embedded NUL
    bool must_fail;
    sfv::List expected;  // meaningful only if !must_fail
    const char* canonical;  // meaningful only if !must_fail
    size_t canonical_len;  // same NUL caveat as raw_len
};

inline const std::vector<GeneratedTestCase>& kTokenListTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"basic token - list", "a_b-c3/*", 8, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"a_b-c3/*"}}, []{ sfv::Parameters p; return p; }()}}}, "a_b-c3/*", 8},
        {"token with capitals - list", "fooBar", 6, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"fooBar"}}, []{ sfv::Parameters p; return p; }()}}}, "fooBar", 6},
        {"token starting with capitals - list", "FooBar", 6, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"FooBar"}}, []{ sfv::Parameters p; return p; }()}}}, "FooBar", 6},
    };
    return cases;
}
