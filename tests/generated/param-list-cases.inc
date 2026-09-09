// GENERATED from tests/vectors/param-list.json (kind=list) by tools/gen-test-cases.py. DO NOT EDIT.
// 20 of 20 official test cases included in this phase
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

inline const std::vector<GeneratedTestCase>& kParamListTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"basic parameterised list", "abc_123;a=1;b=2; cdef_456, ghi;q=9;r=\"+w\"", 41, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"abc_123"}}, []{ sfv::Parameters p; p.set("a", sfv::BareItem{int64_t{1}}); p.set("b", sfv::BareItem{int64_t{2}}); p.set("cdef_456", sfv::BareItem{true}); return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"ghi"}}, []{ sfv::Parameters p; p.set("q", sfv::BareItem{int64_t{9}}); p.set("r", sfv::BareItem{std::string("+w")}); return p; }()}}}, "abc_123;a=1;b=2;cdef_456, ghi;q=9;r=\"+w\"", 40},
        {"single item parameterised list", "text/html;q=1.0", 15, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"text/html"}}, []{ sfv::Parameters p; p.set("q", sfv::BareItem{sfv::Decimal::from_scaled(1000)}); return p; }()}}}, "text/html;q=1.0", 15},
        {"missing parameter value parameterised list", "text/html;a;q=1.0", 17, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"text/html"}}, []{ sfv::Parameters p; p.set("a", sfv::BareItem{true}); p.set("q", sfv::BareItem{sfv::Decimal::from_scaled(1000)}); return p; }()}}}, "text/html;a;q=1.0", 17},
        {"missing terminal parameter value parameterised list", "text/html;q=1.0;a", 17, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"text/html"}}, []{ sfv::Parameters p; p.set("q", sfv::BareItem{sfv::Decimal::from_scaled(1000)}); p.set("a", sfv::BareItem{true}); return p; }()}}}, "text/html;q=1.0;a", 17},
        {"no whitespace parameterised list", "text/html,text/plain;q=0.5", 26, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"text/html"}}, []{ sfv::Parameters p; return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"text/plain"}}, []{ sfv::Parameters p; p.set("q", sfv::BareItem{sfv::Decimal::from_scaled(500)}); return p; }()}}}, "text/html, text/plain;q=0.5", 27},
        {"whitespace before = parameterised list", "text/html, text/plain;q =0.5", 28, true, sfv::List{}, "", 0},
        {"whitespace after = parameterised list", "text/html, text/plain;q= 0.5", 28, true, sfv::List{}, "", 0},
        {"whitespace before ; parameterised list", "text/html, text/plain ;q=0.5", 28, true, sfv::List{}, "", 0},
        {"whitespace after ; parameterised list", "text/html, text/plain; q=0.5", 28, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"text/html"}}, []{ sfv::Parameters p; return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"text/plain"}}, []{ sfv::Parameters p; p.set("q", sfv::BareItem{sfv::Decimal::from_scaled(500)}); return p; }()}}}, "text/html, text/plain;q=0.5", 27},
        {"extra whitespace parameterised list", "text/html  ,  text/plain;  q=0.5;  charset=utf-8", 48, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"text/html"}}, []{ sfv::Parameters p; return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"text/plain"}}, []{ sfv::Parameters p; p.set("q", sfv::BareItem{sfv::Decimal::from_scaled(500)}); p.set("charset", sfv::BareItem{sfv::Token{"utf-8"}}); return p; }()}}}, "text/html, text/plain;q=0.5;charset=utf-8", 41},
        {"two lines parameterised list", "text/html, text/plain;q=0.5", 27, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"text/html"}}, []{ sfv::Parameters p; return p; }()}}, sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"text/plain"}}, []{ sfv::Parameters p; p.set("q", sfv::BareItem{sfv::Decimal::from_scaled(500)}); return p; }()}}}, "text/html, text/plain;q=0.5", 27},
        {"trailing comma parameterised list", "text/html,text/plain;q=0.5,", 27, true, sfv::List{}, "", 0},
        {"empty item parameterised list", "text/html,,text/plain;q=0.5,", 28, true, sfv::List{}, "", 0},
        {"duplicate parameter with different positions", "a;b=1;c=2;b=3", 13, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"a"}}, []{ sfv::Parameters p; p.set("b", sfv::BareItem{int64_t{3}}); p.set("c", sfv::BareItem{int64_t{2}}); return p; }()}}}, "a;b=3;c=2", 9},
        {"parameter ordering", "a;m;z;t", 7, false, sfv::List{sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"a"}}, []{ sfv::Parameters p; p.set("m", sfv::BareItem{true}); p.set("z", sfv::BareItem{true}); p.set("t", sfv::BareItem{true}); return p; }()}}}, "a;m;z;t", 7},
        {"trailing semicolon in parameters", "a;b=1;", 6, true, sfv::List{}, "", 0},
        {"trailing semicolon and space in parameters", "a;b=1; ", 7, true, sfv::List{}, "", 0},
        {"empty parameter key", "a;=1", 4, true, sfv::List{}, "", 0},
        {"parameter prefix only", "a;", 2, true, sfv::List{}, "", 0},
        {"parameter prefix and space only", "a; ", 3, true, sfv::List{}, "", 0},
    };
    return cases;
}
