// GENERATED from tests/vectors/number.json (kind=item) by tools/gen-test-cases.py. DO NOT EDIT.
// 34 of 37 official test cases included in this phase
// (3 non-item header_type, 0 unsupported type, deferred to a later phase).

struct GeneratedTestCase {
    const char* name;
    const char* raw;
    size_t raw_len;  // use this, not strlen(raw) -- raw may contain embedded NUL
    bool must_fail;
    sfv::Item expected;  // meaningful only if !must_fail
    const char* canonical;  // meaningful only if !must_fail
    size_t canonical_len;  // same NUL caveat as raw_len
};

inline const std::vector<GeneratedTestCase>& kNumberTestCases() {
    static const std::vector<GeneratedTestCase> cases = {
        {"basic integer", "42", 2, false, sfv::Item{sfv::BareItem{int64_t{42}}, []{ sfv::Parameters p; return p; }()}, "42", 2},
        {"zero integer", "0", 1, false, sfv::Item{sfv::BareItem{int64_t{0}}, []{ sfv::Parameters p; return p; }()}, "0", 1},
        {"negative zero", "-0", 2, false, sfv::Item{sfv::BareItem{int64_t{0}}, []{ sfv::Parameters p; return p; }()}, "0", 1},
        {"double negative zero", "--0", 3, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"negative integer", "-42", 3, false, sfv::Item{sfv::BareItem{int64_t{-42}}, []{ sfv::Parameters p; return p; }()}, "-42", 3},
        {"leading 0 integer", "042", 3, false, sfv::Item{sfv::BareItem{int64_t{42}}, []{ sfv::Parameters p; return p; }()}, "42", 2},
        {"leading 0 negative integer", "-042", 4, false, sfv::Item{sfv::BareItem{int64_t{-42}}, []{ sfv::Parameters p; return p; }()}, "-42", 3},
        {"leading 0 zero", "00", 2, false, sfv::Item{sfv::BareItem{int64_t{0}}, []{ sfv::Parameters p; return p; }()}, "0", 1},
        {"comma", "2,3", 3, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"negative non-DIGIT first character", "-a23", 4, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"sign out of place", "4-2", 3, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"whitespace after sign", "- 42", 4, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"long integer", "123456789012345", 15, false, sfv::Item{sfv::BareItem{int64_t{123456789012345}}, []{ sfv::Parameters p; return p; }()}, "123456789012345", 15},
        {"long negative integer", "-123456789012345", 16, false, sfv::Item{sfv::BareItem{int64_t{-123456789012345}}, []{ sfv::Parameters p; return p; }()}, "-123456789012345", 16},
        {"too long integer", "1234567890123456", 16, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"negative too long integer", "-1234567890123456", 17, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"simple decimal", "1.23", 4, false, sfv::Item{sfv::BareItem{sfv::Decimal::from_scaled(1230)}, []{ sfv::Parameters p; return p; }()}, "1.23", 4},
        {"negative decimal", "-1.23", 5, false, sfv::Item{sfv::BareItem{sfv::Decimal::from_scaled(-1230)}, []{ sfv::Parameters p; return p; }()}, "-1.23", 5},
        {"decimal, whitespace after decimal", "1. 23", 5, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"decimal, whitespace before decimal", "1 .23", 5, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"negative decimal, whitespace after sign", "- 1.23", 6, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"tricky precision decimal", "123456789012.1", 14, false, sfv::Item{sfv::BareItem{sfv::Decimal::from_scaled(123456789012100)}, []{ sfv::Parameters p; return p; }()}, "123456789012.1", 14},
        {"double decimal decimal", "1.5.4", 5, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"adjacent double decimal decimal", "1..4", 4, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"decimal with three fractional digits", "1.123", 5, false, sfv::Item{sfv::BareItem{sfv::Decimal::from_scaled(1123)}, []{ sfv::Parameters p; return p; }()}, "1.123", 5},
        {"negative decimal with three fractional digits", "-1.123", 6, false, sfv::Item{sfv::BareItem{sfv::Decimal::from_scaled(-1123)}, []{ sfv::Parameters p; return p; }()}, "-1.123", 6},
        {"decimal with zero fractional digits", "1.", 2, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"decimal with four fractional digits", "1.1234", 6, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"negative decimal with four fractional digits", "-1.1234", 7, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"decimal with thirteen integer digits", "1234567890123.0", 15, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"negative decimal with thirteen integer digits", "-1234567890123.0", 16, true, sfv::Item{int64_t{0}, sfv::Parameters{}}, "", 0},
        {"decimal with 1 significant digit and 1 insignificant digit", "1.20", 4, false, sfv::Item{sfv::BareItem{sfv::Decimal::from_scaled(1200)}, []{ sfv::Parameters p; return p; }()}, "1.2", 3},
        {"decimal with 1 significant digit and 2 insignificant digits", "1.200", 5, false, sfv::Item{sfv::BareItem{sfv::Decimal::from_scaled(1200)}, []{ sfv::Parameters p; return p; }()}, "1.2", 3},
        {"decimal with 2 significant digits and 1 insignificant digit", "1.230", 5, false, sfv::Item{sfv::BareItem{sfv::Decimal::from_scaled(1230)}, []{ sfv::Parameters p; return p; }()}, "1.23", 4},
    };
    return cases;
}
