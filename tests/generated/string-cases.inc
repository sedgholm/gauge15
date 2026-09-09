// GENERATED from tests/vectors/string.json (kind=item) by tools/gen-test-cases.py. DO NOT EDIT.
// 14 of 14 official test cases included in this phase
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

inline const std::vector<GeneratedTestCase>& kStringTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"basic string", "\"foo bar\"", 9, false, sfv::Item{sfv::BareItem{std::string("foo bar")}, []{ sfv::Parameters p; return p; }()}, "\"foo bar\"", 9},
        {"empty string", "\"\"", 2, false, sfv::Item{sfv::BareItem{std::string("")}, []{ sfv::Parameters p; return p; }()}, "\"\"", 2},
        {"long string", "\"foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo \"", 262, false, sfv::Item{sfv::BareItem{std::string("foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo ")}, []{ sfv::Parameters p; return p; }()}, "\"foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo \"", 262},
        {"whitespace string", "\"   \"", 5, false, sfv::Item{sfv::BareItem{std::string("   ")}, []{ sfv::Parameters p; return p; }()}, "\"   \"", 5},
        {"non-ascii string", "\"f\xfc\xfc\"", 5, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"tab in string", "\"\t\"", 3, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"newline in string", "\" \n \"", 5, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"single quoted string", "'foo'", 5, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"unbalanced string", "\"foo", 4, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"string quoting", "\"foo \\\"bar\\\" \\\\ baz\"", 20, false, sfv::Item{sfv::BareItem{std::string("foo \"bar\" \\ baz")}, []{ sfv::Parameters p; return p; }()}, "\"foo \\\"bar\\\" \\\\ baz\"", 20},
        {"bad string quoting", "\"foo \\,\"", 8, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"ending string quote", "\"foo \\\"", 7, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"abruptly ending string quote", "\"foo \\", 6, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"two lines string", "\"foo, bar\"", 10, false, sfv::Item{sfv::BareItem{std::string("foo, bar")}, []{ sfv::Parameters p; return p; }()}, "\"foo, bar\"", 10},
    };
    return cases;
}
