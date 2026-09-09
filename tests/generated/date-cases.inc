// GENERATED from tests/vectors/date.json (kind=item) by tools/gen-test-cases.py. DO NOT EDIT.
// 17 of 17 official test cases included in this phase
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

inline const std::vector<GeneratedTestCase>& kDateTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"date - 1970-01-01 00:00:00", "@0", 2, false, sfv::Item{sfv::BareItem{sfv::Date{int64_t{0}}}, []{ sfv::Parameters p; return p; }()}, "@0", 2},
        {"date - 2022-08-04 01:57:13", "@1659578233", 11, false, sfv::Item{sfv::BareItem{sfv::Date{int64_t{1659578233}}}, []{ sfv::Parameters p; return p; }()}, "@1659578233", 11},
        {"date - 1917-05-30 22:02:47", "@-1659578233", 12, false, sfv::Item{sfv::BareItem{sfv::Date{int64_t{-1659578233}}}, []{ sfv::Parameters p; return p; }()}, "@-1659578233", 12},
        {"date - 2^31", "@2147483648", 11, false, sfv::Item{sfv::BareItem{sfv::Date{int64_t{2147483648}}}, []{ sfv::Parameters p; return p; }()}, "@2147483648", 11},
        {"date - 2^32", "@4294967296", 11, false, sfv::Item{sfv::BareItem{sfv::Date{int64_t{4294967296}}}, []{ sfv::Parameters p; return p; }()}, "@4294967296", 11},
        {"interoperability max date - 9999-12-31 00:00:00", "@253402214400", 13, false, sfv::Item{sfv::BareItem{sfv::Date{int64_t{253402214400}}}, []{ sfv::Parameters p; return p; }()}, "@253402214400", 13},
        {"interoperability min date - 0001-01-01 00:00:00", "@-62135596800", 13, false, sfv::Item{sfv::BareItem{sfv::Date{int64_t{-62135596800}}}, []{ sfv::Parameters p; return p; }()}, "@-62135596800", 13},
        {"date - decimal", "@1659578233.12", 14, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"syntactic max date - 999,999,999,999,999", "@999999999999999", 16, false, sfv::Item{sfv::BareItem{sfv::Date{int64_t{999999999999999}}}, []{ sfv::Parameters p; return p; }()}, "@999999999999999", 16},
        {"syntactic min date - -999,999,999,999,999", "@-999999999999999", 17, false, sfv::Item{sfv::BareItem{sfv::Date{int64_t{-999999999999999}}}, []{ sfv::Parameters p; return p; }()}, "@-999999999999999", 17},
        {"too large syntactic date - 1,000,000,000,000,000", "@1000000000000000", 17, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"too small syntactic date - -1,000,000,000,000,000", "@-1000000000000000", 18, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"date with negative zero", "@-0", 3, false, sfv::Item{sfv::BareItem{sfv::Date{int64_t{0}}}, []{ sfv::Parameters p; return p; }()}, "@0", 2},
        {"date - empty", "@", 1, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"date - sign only", "@-", 2, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"date - space", "@ 12345678", 10, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"date - non-digit", "@abc", 4, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
    };
    return cases;
}
