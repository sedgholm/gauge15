// GENERATED from tests/vectors/boolean.json (kind=item) by tools/gen-test-cases.py. DO NOT EDIT.
// 12 of 12 official test cases included in this phase
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

inline const std::vector<GeneratedTestCase>& kBooleanTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"basic true boolean", "?1", 2, false, sfv::Item{sfv::BareItem{true}, []{ sfv::Parameters p; return p; }()}, "?1", 2},
        {"basic false boolean", "?0", 2, false, sfv::Item{sfv::BareItem{false}, []{ sfv::Parameters p; return p; }()}, "?0", 2},
        {"unknown boolean", "?Q", 2, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"whitespace boolean", "? 1", 3, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"negative zero boolean", "?-0", 3, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"T boolean", "?T", 2, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"F boolean", "?F", 2, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"t boolean", "?t", 2, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"f boolean", "?f", 2, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"spelled-out True boolean", "?True", 5, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"spelled-out False boolean", "?False", 6, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"boolean prefix only", "?", 1, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
    };
    return cases;
}
