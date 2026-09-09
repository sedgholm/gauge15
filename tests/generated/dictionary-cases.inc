// GENERATED from tests/vectors/dictionary.json (kind=dictionary) by tools/gen-test-cases.py. DO NOT EDIT.
// 26 of 26 official test cases included in this phase
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

inline const std::vector<GeneratedTestCase>& kDictionaryTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"basic dictionary", "en=\"Applepie\", da=:w4ZibGV0w6ZydGUK:", 36, false, []{ sfv::Dictionary d; d.set("en", sfv::Member{sfv::Item{sfv::BareItem{std::string("Applepie")}, []{ sfv::Parameters p; return p; }()}}); d.set("da", sfv::Member{sfv::Item{sfv::BareItem{sfv::ByteSequence{"\xc3\x86" "blet\xc3\xa6rte\n"}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "en=\"Applepie\", da=:w4ZibGV0w6ZydGUK:", 36},
        {"empty dictionary", "", 0, false, []{ sfv::Dictionary d; return d; }(), "", 0},
        {"single item dictionary", "a=1", 3, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=1", 3},
        {"list item dictionary", "a=(1 2)", 7, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::InnerList{std::vector<sfv::Item>{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}, sfv::Item{sfv::BareItem{int64_t{2}}, []{ sfv::Parameters p; return p; }()}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=(1 2)", 7},
        {"single list item dictionary", "a=(1)", 5, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::InnerList{std::vector<sfv::Item>{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=(1)", 5},
        {"empty list item dictionary", "a=()", 4, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::InnerList{std::vector<sfv::Item>{}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=()", 4},
        {"no whitespace dictionary", "a=1,b=2", 7, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}); d.set("b", sfv::Member{sfv::Item{sfv::BareItem{int64_t{2}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=1, b=2", 8},
        {"extra whitespace dictionary", "a=1 ,  b=2", 10, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}); d.set("b", sfv::Member{sfv::Item{sfv::BareItem{int64_t{2}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=1, b=2", 8},
        {"tab separated dictionary", "a=1\t,\tb=2", 9, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}); d.set("b", sfv::Member{sfv::Item{sfv::BareItem{int64_t{2}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=1, b=2", 8},
        {"leading whitespace dictionary", "     a=1 ,  b=2", 15, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}); d.set("b", sfv::Member{sfv::Item{sfv::BareItem{int64_t{2}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=1, b=2", 8},
        {"whitespace before = dictionary", "a =1, b=2", 9, true, sfv::Dictionary{}, "", 0},
        {"whitespace after = dictionary", "a=1, b= 2", 9, true, sfv::Dictionary{}, "", 0},
        {"two lines dictionary", "a=1, b=2", 8, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}); d.set("b", sfv::Member{sfv::Item{sfv::BareItem{int64_t{2}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=1, b=2", 8},
        {"missing value dictionary", "a=1, b, c=3", 11, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}); d.set("b", sfv::Member{sfv::Item{sfv::BareItem{true}, []{ sfv::Parameters p; return p; }()}}); d.set("c", sfv::Member{sfv::Item{sfv::BareItem{int64_t{3}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=1, b, c=3", 11},
        {"all missing value dictionary", "a, b, c", 7, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{true}, []{ sfv::Parameters p; return p; }()}}); d.set("b", sfv::Member{sfv::Item{sfv::BareItem{true}, []{ sfv::Parameters p; return p; }()}}); d.set("c", sfv::Member{sfv::Item{sfv::BareItem{true}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a, b, c", 7},
        {"start missing value dictionary", "a, b=2", 6, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{true}, []{ sfv::Parameters p; return p; }()}}); d.set("b", sfv::Member{sfv::Item{sfv::BareItem{int64_t{2}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a, b=2", 6},
        {"end missing value dictionary", "a=1, b", 6, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}); d.set("b", sfv::Member{sfv::Item{sfv::BareItem{true}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=1, b", 6},
        {"missing value with params dictionary", "a=1, b;foo=9, c=3", 17, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}); d.set("b", sfv::Member{sfv::Item{sfv::BareItem{true}, []{ sfv::Parameters p; p.set("foo", sfv::BareItem{int64_t{9}}); return p; }()}}); d.set("c", sfv::Member{sfv::Item{sfv::BareItem{int64_t{3}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=1, b;foo=9, c=3", 17},
        {"explicit true value with params dictionary", "a=1, b=?1;foo=9, c=3", 20, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{int64_t{1}}, []{ sfv::Parameters p; return p; }()}}); d.set("b", sfv::Member{sfv::Item{sfv::BareItem{true}, []{ sfv::Parameters p; p.set("foo", sfv::BareItem{int64_t{9}}); return p; }()}}); d.set("c", sfv::Member{sfv::Item{sfv::BareItem{int64_t{3}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=1, b;foo=9, c=3", 17},
        {"trailing comma dictionary", "a=1, b=2,", 9, true, sfv::Dictionary{}, "", 0},
        {"empty item dictionary", "a=1,,b=2,", 9, true, sfv::Dictionary{}, "", 0},
        {"duplicate key dictionary", "a=1,b=2,a=3", 11, false, []{ sfv::Dictionary d; d.set("a", sfv::Member{sfv::Item{sfv::BareItem{int64_t{3}}, []{ sfv::Parameters p; return p; }()}}); d.set("b", sfv::Member{sfv::Item{sfv::BareItem{int64_t{2}}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "a=3, b=2", 8},
        {"dictionary key ordering", "m, z, t", 7, false, []{ sfv::Dictionary d; d.set("m", sfv::Member{sfv::Item{sfv::BareItem{true}, []{ sfv::Parameters p; return p; }()}}); d.set("z", sfv::Member{sfv::Item{sfv::BareItem{true}, []{ sfv::Parameters p; return p; }()}}); d.set("t", sfv::Member{sfv::Item{sfv::BareItem{true}, []{ sfv::Parameters p; return p; }()}}); return d; }(), "m, z, t", 7},
        {"numeric key dictionary", "a=1,1b=2,a=1", 12, true, sfv::Dictionary{}, "", 0},
        {"uppercase key dictionary", "a=1,B=2,a=1", 11, true, sfv::Dictionary{}, "", 0},
        {"bad key dictionary", "a=1,b!=2,a=1", 12, true, sfv::Dictionary{}, "", 0},
    };
    return cases;
}
