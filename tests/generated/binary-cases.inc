// GENERATED from tests/vectors/binary.json (kind=item) by tools/gen-test-cases.py. DO NOT EDIT.
// 15 of 15 official test cases included in this phase
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

inline const std::vector<GeneratedTestCase>& kBinaryTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"basic binary", ":aGVsbG8=:", 10, false, sfv::Item{sfv::BareItem{sfv::ByteSequence{"hello"}}, []{ sfv::Parameters p; return p; }()}, ":aGVsbG8=:", 10},
        {"empty binary", "::", 2, false, sfv::Item{sfv::BareItem{sfv::ByteSequence{""}}, []{ sfv::Parameters p; return p; }()}, "::", 2},
        {"padding at beginning", ":=aGVsbG8=:", 11, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"padding in middle", ":a=GVsbG8=:", 11, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"bad padding", ":aGVsbG8:", 9, false, sfv::Item{sfv::BareItem{sfv::ByteSequence{"hello"}}, []{ sfv::Parameters p; return p; }()}, ":aGVsbG8=:", 10},
        {"bad padding dot", ":aGVsbG8.:", 10, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"bad end delimiter", ":aGVsbG8=", 9, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"extra whitespace", ":aGVsb G8=:", 11, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"all whitespace", ":    :", 6, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"extra chars", ":aGVsbG!8=:", 11, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"suffix chars", ":aGVsbG8=!:", 11, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"non-zero pad bits", ":iZ==:", 6, false, sfv::Item{sfv::BareItem{sfv::ByteSequence{"\x89"}}, []{ sfv::Parameters p; return p; }()}, ":iQ==:", 6},
        {"non-ASCII binary", ":/+Ah:", 6, false, sfv::Item{sfv::BareItem{sfv::ByteSequence{"\xff\xe0!"}}, []{ sfv::Parameters p; return p; }()}, ":/+Ah:", 6},
        {"base64url binary", ":_-Ah:", 6, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"binary prefix only", ":", 1, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
    };
    return cases;
}
