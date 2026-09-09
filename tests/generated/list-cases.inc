// GENERATED from tests/vectors/list.json (kind=list) by tools/gen-test-cases.py. DO NOT EDIT.
// 11 of 11 official test cases included in this phase
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

inline const std::vector<GeneratedTestCase>& kListTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"basic list", "1, 42", 5, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{int64_t{42}}, []{ sfv::Parameters p; return p; }()}}}, "1, 42", 5},
        {"empty list", "", 0, false, sfv::List{}, "", 0},
        {"leading SP list", "  42, 43", 8, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{int64_t{42}}, []{ sfv::Parameters p; return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{int64_t{43}}, []{ sfv::Parameters p; return p; }()}}}, "42, 43", 6},
        {"single item list", "42", 2, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{int64_t{42}}, []{ sfv::Parameters p; return p; }()}}}, "42", 2},
        {"no whitespace list", "1,42", 4, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{int64_t{42}}, []{ sfv::Parameters p; return p; }()}}}, "1, 42", 5},
        {"extra whitespace list", "1 , 42", 6, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{int64_t{42}}, []{ sfv::Parameters p; return p; }()}}}, "1, 42", 5},
        {"tab separated list", "1\t,\t42", 6, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{int64_t{42}}, []{ sfv::Parameters p; return p; }()}}}, "1, 42", 5},
        {"two line list", "1, 42", 5, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{int64_t{42}}, []{ sfv::Parameters p; return p; }()}}}, "1, 42", 5},
        {"trailing comma list", "1, 42,", 6, true, sfv::List{}, "", 0},
        {"empty item list", "1,,42", 5, true, sfv::List{}, "", 0},
        {"empty item list (multiple field lines)", "1, , 42", 7, true, sfv::List{}, "", 0},
    };
    return cases;
}
