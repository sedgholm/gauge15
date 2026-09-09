// GENERATED from tests/vectors/token.json (kind=item) by tools/gen-test-cases.py. DO NOT EDIT.
// 3 of 6 official test cases included in this phase
// (3 non-item header_type, 0 unsupported type, deferred to a later phase).

struct GeneratedTestCase {
    const char* name;
    const char* raw;
    size_t raw_len;  // use this, not strlen(raw) -- raw may contain embedded NUL
    bool must_fail;
    sfv::Item expected;  // meaningful only if !must_fail
    const char* canonical;  // meaningful only if !must_fail
    size_t canonical_len;  // same NUL caveat as raw_len
};

inline const std::vector<GeneratedTestCase>& kTokenTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"basic token - item", "a_b-c.d3:f%00/*", 15, false, sfv::Item{sfv::BareItem{sfv::Token{"a_b-c.d3:f%00/*"}}, []{ sfv::Parameters p; return p; }()}, "a_b-c.d3:f%00/*", 15},
        {"token with capitals - item", "fooBar", 6, false, sfv::Item{sfv::BareItem{sfv::Token{"fooBar"}}, []{ sfv::Parameters p; return p; }()}, "fooBar", 6},
        {"token starting with capitals - item", "FooBar", 6, false, sfv::Item{sfv::BareItem{sfv::Token{"FooBar"}}, []{ sfv::Parameters p; return p; }()}, "FooBar", 6},
    };
    return cases;
}
