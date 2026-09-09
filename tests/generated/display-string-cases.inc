// GENERATED from tests/vectors/display-string.json (kind=item) by tools/gen-test-cases.py. DO NOT EDIT.
// 22 of 22 official test cases included in this phase
// (0 non-item header_type, 0 unsupported type, deferred to a later phase).

struct GeneratedTestCase {
    const char* name;
    const char* raw;
    size_t raw_len;  // use this, not strlen(raw) -- raw may contain embedded NUL
    bool must_fail;
    sfv::Item expected;  // meaningful only if !must_fail
    const char* canonical;  // meaningful only if !must_fail
    size_t canonical_len;  // same NUL caveat as raw_len
};

inline const std::vector<GeneratedTestCase>& kDisplayStringTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"basic display string (ascii content)", "%\"foo bar\"", 10, false, sfv::Item{sfv::BareItem{sfv::DisplayString{"foo bar"}}, []{ sfv::Parameters p; return p; }()}, "%\"foo bar\"", 10},
        {"all printable ascii", "%\" !%22#$%25&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\"", 102, false, sfv::Item{sfv::BareItem{sfv::DisplayString{" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"}}, []{ sfv::Parameters p; return p; }()}, "%\" !%22#$%25&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\"", 102},
        {"non-ascii display string (uppercase escaping)", "%\"f%C3%BC%C3%BC\"", 16, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"non-ascii display string (lowercase escaping)", "%\"f%c3%bc%c3%bc\"", 16, false, sfv::Item{sfv::BareItem{sfv::DisplayString{"f\xc3\xbc\xc3\xbc"}}, []{ sfv::Parameters p; return p; }()}, "%\"f%c3%bc%c3%bc\"", 16},
        {"non-ascii display string (unescaped)", "%\"f\xfc\xfc\"", 6, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"tab in display string", "%\"\t\"", 4, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"newline in display string", "%\"\n\"", 4, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"single quoted display string", "%'foo'", 6, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"unquoted display string", "%foo", 4, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"display string missing initial quote", "%foo\"", 5, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"unbalanced display string", "%\"foo", 5, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"display string quoting", "%\"foo %22bar%22 \\ baz\"", 22, false, sfv::Item{sfv::BareItem{sfv::DisplayString{"foo \"bar\" \\ baz"}}, []{ sfv::Parameters p; return p; }()}, "%\"foo %22bar%22 \\ baz\"", 22},
        {"bad display string escaping", "%\"foo %a", 8, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"bad display string utf-8 (invalid 2-byte seq)", "%\"%c3%28\"", 9, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"bad display string utf-8 (invalid sequence id)", "%\"%a0%a1\"", 9, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"bad display string utf-8 (invalid hex)", "%\"%g0%1w\"", 9, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"bad display string utf-8 (invalid 3-byte seq)", "%\"%e2%28%a1\"", 12, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"bad display string utf-8 (invalid 4-byte seq)", "%\"%f0%28%8c%28\"", 15, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"BOM in display string", "%\"BOM: %ef%bb%bf\"", 17, false, sfv::Item{sfv::BareItem{sfv::DisplayString{"BOM: \xef\xbb\xbf"}}, []{ sfv::Parameters p; return p; }()}, "%\"BOM: %ef%bb%bf\"", 17},
        {"two lines display string", "%\"foo, bar\"", 11, false, sfv::Item{sfv::BareItem{sfv::DisplayString{"foo, bar"}}, []{ sfv::Parameters p; return p; }()}, "%\"foo, bar\"", 11},
        {"truncated display string escape (zero digits)", "%\"%\"", 4, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"over-encoded display string", "%\"%61\"", 6, false, sfv::Item{sfv::BareItem{sfv::DisplayString{"a"}}, []{ sfv::Parameters p; return p; }()}, "%\"a\"", 4},
    };
    return cases;
}
