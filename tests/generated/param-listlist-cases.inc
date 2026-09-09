// GENERATED from tests/vectors/param-listlist.json (kind=list) by tools/gen-test-cases.py. DO NOT EDIT.
// 3 of 3 official test cases included in this phase
// (0 non-list header_type, 0 unsupported type, deferred to a later phase).

struct GeneratedTestCase {
    const char* name;
    const char* raw;
    size_t raw_len;  // use this, not strlen(raw) -- raw may contain embedded NUL
    bool must_fail;
    sfv::List expected;  // meaningful only if !must_fail
    const char* canonical;  // meaningful only if !must_fail
    size_t canonical_len;  // same NUL caveat as raw_len
};

inline const std::vector<GeneratedTestCase>& kParamListListTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"parameterised inner list", "(abc_123);a=1;b=2, cdef_456", 27, false, sfv::List{sfv::Member{sfv::InnerList{std::vector<sfv::Item>{sfv::Item{sfv::BareItem{sfv::Token{"abc_123"}}, []{ sfv::Parameters p; return p; }()}}, []{ sfv::Parameters p; p.set("a", sfv::BareItem{int64_t{1}}); p.set("b", sfv::BareItem{int64_t{2}}); return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"cdef_456"}}, []{ sfv::Parameters p; return p; }()}}}, "(abc_123);a=1;b=2, cdef_456", 27},
        {"parameterised inner list item", "(abc_123;a=1;b=2;cdef_456)", 26, false, sfv::List{sfv::Member{sfv::InnerList{std::vector<sfv::Item>{sfv::Item{sfv::BareItem{sfv::Token{"abc_123"}}, []{ sfv::Parameters p; p.set("a", sfv::BareItem{int64_t{1}}); p.set("b", sfv::BareItem{int64_t{2}}); p.set("cdef_456", sfv::BareItem{true}); return p; }()}}, []{ sfv::Parameters p; return p; }()}}}, "(abc_123;a=1;b=2;cdef_456)", 26},
        {"parameterised inner list with parameterised item", "(abc_123;a=1;b=2);cdef_456", 26, false, sfv::List{sfv::Member{sfv::InnerList{std::vector<sfv::Item>{sfv::Item{sfv::BareItem{sfv::Token{"abc_123"}}, []{ sfv::Parameters p; p.set("a", sfv::BareItem{int64_t{1}}); p.set("b", sfv::BareItem{int64_t{2}}); return p; }()}}, []{ sfv::Parameters p; p.set("cdef_456", sfv::BareItem{true}); return p; }()}}}, "(abc_123;a=1;b=2);cdef_456", 26},
    };
    return cases;
}
