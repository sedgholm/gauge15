// GENERATED from tests/vectors/param-dict.json (kind=dictionary) by tools/gen-test-cases.py. DO NOT EDIT.
// 14 of 14 official test cases included in this phase
// (0 non-dictionary header_type, 0 unsupported type, deferred to a later phase).

struct GeneratedTestCase {
    const char* name;
    const char* raw;
    size_t raw_len;  // use this, not strlen(raw) -- raw may contain embedded NUL
    bool must_fail;
    sfv::Dictionary expected;  // meaningful only if !must_fail
    const char* canonical;  // meaningful only if !must_fail
    size_t canonical_len;  // same NUL caveat as raw_len
};

inline const std::vector<GeneratedTestCase>& kParamDictTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"basic parameterised dict", "abc=123;a=1;b=2, def=456, ghi=789;q=9;r=\"+w\"", 44, false, []{ sfv::Dictionary d; d.set("abc", sfv::Member{sfv::Item{sfv::BareItem{int64_t{123}}, []{ sfv::Parameters p; p.set("a", sfv::BareItem{int64_t{1}}); p.set("b", sfv::BareItem{int64_t{2}}); return p; }()}}); d.set("def", sfv::Member{sfv::Item{sfv::BareItem{int64_t{456}}, []{ sfv::Parameters p; return p; }()}}); d.set("ghi", sfv::Member{sfv::Item{sfv::BareItem{int64_t{789}}, []{ sfv::Parameters p; p.set("q", sfv::BareItem{int64_t{9}}); p.set("r", sfv::BareItem{std::string("+w")}); return p; }()}}); return d; }(), "abc=123;a=1;b=2, def=456, ghi=789;q=9;r=\"+w\"", 44},
        {"single item parameterised dict", "a=b; q=1.0", 10, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"b"}}, []{ sfv::Parameters p; p.set("q", sfv::BareItem{sfv::Decimal::from_scaled(1000)}); return p; }()}}); return d; }(), "a=b;q=1.0", 9},
        {"list item parameterised dictionary", "a=(1 2); q=1.0", 14, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::InnerList{std::vector<sfv::Item>{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}, sfv::Item{sfv::BareItem{int64_t{2}}, []{ sfv::Parameters p; return p; }()}}, []{ sfv::Parameters p; p.set("q", sfv::BareItem{sfv::Decimal::from_scaled(1000)}); return p; }()}}); return d; }(), "a=(1 2);q=1.0", 13},
        {"missing parameter value parameterised dict", "a=3;c;d=5", 9, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{int64_t{3}}, []{ sfv::Parameters p; p.set("c", sfv::BareItem{true}); p.set("d", sfv::BareItem{int64_t{5}}); return p; }()}}); return d; }(), "a=3;c;d=5", 9},
        {"terminal missing parameter value parameterised dict", "a=3;c=5;d", 9, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{int64_t{3}}, []{ sfv::Parameters p; p.set("c", sfv::BareItem{int64_t{5}}); p.set("d", sfv::BareItem{true}); return p; }()}}); return d; }(), "a=3;c=5;d", 9},
        {"no whitespace parameterised dict", "a=b;c=1,d=e;f=2", 15, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"b"}}, []{ sfv::Parameters p; p.set("c", sfv::BareItem{int64_t{1}}); return p; }()}}); d.set("d", sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"e"}}, []{ sfv::Parameters p; p.set("f", sfv::BareItem{int64_t{2}}); return p; }()}}); return d; }(), "a=b;c=1, d=e;f=2", 16},
        {"whitespace before = parameterised dict", "a=b;q =0.5", 10, true, sfv::Dictionary{}, "", 0},
        {"whitespace after = parameterised dict", "a=b;q= 0.5", 10, true, sfv::Dictionary{}, "", 0},
        {"whitespace before ; parameterised dict", "a=b ;q=0.5", 10, true, sfv::Dictionary{}, "", 0},
        {"whitespace after ; parameterised dict", "a=b; q=0.5", 10, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"b"}}, []{ sfv::Parameters p; p.set("q", sfv::BareItem{sfv::Decimal::from_scaled(500)}); return p; }()}}); return d; }(), "a=b;q=0.5", 9},
        {"extra whitespace parameterised dict", "a=b;  c=1  ,  d=e; f=2; g=3", 27, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"b"}}, []{ sfv::Parameters p; p.set("c", sfv::BareItem{int64_t{1}}); return p; }()}}); d.set("d", sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"e"}}, []{ sfv::Parameters p; p.set("f", sfv::BareItem{int64_t{2}}); p.set("g", sfv::BareItem{int64_t{3}}); return p; }()}}); return d; }(), "a=b;c=1, d=e;f=2;g=3", 20},
        {"two lines parameterised list", "a=b;c=1, d=e;f=2", 16, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"b"}}, []{ sfv::Parameters p; p.set("c", sfv::BareItem{int64_t{1}}); return p; }()}}); d.set("d", sfv::Member{sfv::Item{sfv::BareItem{sfv::Token{"e"}}, []{ sfv::Parameters p; p.set("f", sfv::BareItem{int64_t{2}}); return p; }()}}); return d; }(), "a=b;c=1, d=e;f=2", 16},
        {"trailing comma parameterised list", "a=b; q=1.0,", 11, true, sfv::Dictionary{}, "", 0},
        {"empty item parameterised list", "a=b; q=1.0,,c=d", 15, true, sfv::Dictionary{}, "", 0},
    };
    return cases;
}
