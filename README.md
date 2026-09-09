# gauge15

A from-scratch, header-only C++20 implementation of **RFC 9651,
"Structured Field Values for HTTP"** — the syntax used inside HTTP header
values like `Cache-Control`, `Accept-CH`, and `Priority`. Think
`nlohmann/json`, but for this one specific, much smaller grammar.

This does **not** do any networking. It has no idea what HTTP, TCP, or a
socket is. Given a header's raw text value (e.g. `"max-age=3600,
must-revalidate"`), it parses that into structured data, and can
serialize structured data back into that text form. What reads the bytes
off the wire — a web server, a proxy, `libcurl` — is a separate concern
entirely.

## Why RFC 9651, not RFC 8941?

RFC 8941 (Feb 2021) was **obsoleted by RFC 9651** (Sept 2024), which adds
two new types (Date, Display String) and is backward compatible — a
9651-conformant parser can parse anything valid under 8941. The official
community test suite (see below) already targets 9651. Building against
the superseded version would have meant shipping something already out
of date.

## Status

| Phase | Scope | Status |
|---|---|---|
| 1 | Core types, Integer, Decimal, Boolean, Key, Parameters, Item | ✅ done |
| 2 | String, Token, Byte Sequence | ✅ done |
| 3 | Date, Display String (the RFC 9651 additions) | ✅ done |
| 4 | List, Inner List | ✅ done |
| 5 | Dictionary | ✅ done |
| 6 | Top-level `parse()`/`serialize()` entry points | ✅ done |
| — | Hardening: fuzzing, cross-compiler checks, real benchmarks | ✅ done |
| 7 | Single-header amalgamation; docs polish | ✅ done |

**The library is functionally complete and has been hardened beyond the
conformance suite alone.** `sfv::parse_list`, `sfv::parse_dictionary`,
`sfv::parse_item`, and `sfv::serialize()` are the four functions a real
caller actually needs — everything under `sfv::detail::` is available
but is the implementation those four are built from, not the primary
interface.

**1,395 checks pass** across all six phases combined, clean under
`-fsanitize=address,undefined` — and as of Phase 6, every one of those
1,377 pre-existing conformance checks was re-routed through the new
top-level API (not just the 18 new ones in `test-api.cpp`), so the whole
existing test corpus now validates it for free. See "Testing &
Validation" below for what's been checked beyond that: fuzzing,
cross-compiler builds, stricter warnings, and real measured performance.

## Where the test data comes from

RFC 9651 Appendix B explicitly names a community-maintained conformance
suite: **[github.com/httpwg/structured-field-tests](https://github.com/httpwg/structured-field-tests)**.
This project vendors those JSON files directly (`tests/vectors/*.json`)
and converts them into C++ test cases at codegen time
(`tools/gen-test-cases.py`) — so test binaries never need a JSON parsing
dependency; the JSON is fully resolved into `sfv::` types (`sfv::Item`,
`sfv::BareItem`, etc.) before any C++ is compiled.

`gen-test-cases.py` is deliberately honest about what it can't yet
handle: cases needing a not-yet-implemented type (List/Dictionary
header_type, or a bare item type from a later phase) are **counted and
reported**, not silently skipped — the generated `.inc` file's own header
comment states exactly how many of the file's cases are included this
phase (e.g. `34 of 37`, with the other 3 being List-typed, deferred to
the List phase).

## Building

```sh
mkdir build && cd build
cmake ..
make
ctest
```

No CMake handy? Compiles directly too (it's header-only — no `.cpp` to
link against for the library itself):

```sh
g++ -std=c++20 -Iinclude -Itests tests/test-number-decimal.cpp -o test-number-decimal
./test-number-decimal
```

**Just want to drop it into your own project?** Copy the one generated
file `single_include/gauge15/sfv.hpp` — no other files needed, same API. See
`single_include/README.md` for how it's generated and verified (it's
tested against the exact same 1,395-check suite as the multi-file
source, not just eyeballed for looking right).

## Testing & Validation

This section is meant to be read honestly: what's actually been
verified, what tooling exists but hasn't been run at scale yet, and what
genuinely hasn't been done.

**Verified directly, in this environment:**
- **1,395 conformance checks** against the official
  `httpwg/structured-field-tests` suite (see "Where the test data comes
  from" above), covering every data type and container RFC 9651 defines.
- **Both GCC 13.3 and Clang 18.1** build the entire test suite with
  **zero warnings** under `-Wall -Wextra -Wpedantic`, and all 1,395
  checks pass identically on both compilers.
- **Zero warnings** on every header under a much stricter set —
  `-Wconversion -Wshadow -Wsign-conversion -Wold-style-cast` — after
  fixing 5 genuine (if harmless-in-practice) signedness-conversion
  issues these flags surfaced in `base64.hpp` and `serializer.hpp`.
- **10,027,412 fuzzing executions** (libFuzzer, ASan+UBSan) across the
  three top-level parse functions, seeded from all 834 unique inputs in
  the vendored conformance data, **zero crashes**. Each harness also
  checks a real correctness property beyond crash-freedom: parsing,
  serializing, and re-parsing must round-trip to an equal value. See
  `fuzz/README.md` for exact numbers per target and how to run longer
  sessions yourself.
- **Measured performance** (not claimed): see `bench/bench.cpp` — e.g.
  ~580,000 `parse_dictionary` calls/sec on a typical 58-byte header, on
  one core of the machine this was developed on. Run `./bench/bench`
  yourself; single-machine numbers are a data point, not a portable
  benchmark result.

**Exists but not yet exercised at real scale:**
- `.github/workflows/ci.yml` defines a GCC/Clang/MSVC × Linux/macOS/
  Windows matrix, plus the strict-warnings check and a 60-second fuzz
  smoke test on every push — but this project has never actually been
  pushed to GitHub, so **that workflow has not run yet**. MSVC in
  particular has not been tested at all; the code avoids GNU/Clang-
  specific extensions (checked directly, not assumed), which is a good
  sign, not a guarantee.
- 10 million fuzzing runs across ~2 minutes per target is real signal,
  clearly more confidence than conformance tests alone, but it is not
  the hours/days of continuous fuzzing (e.g. OSS-Fuzz) a security-
  critical parser of untrusted network input would ideally get before
  being trusted at scale.

**Not done, and no amount of one-session work substitutes for it:**
- Real-world usage. No amount of testing replaces months of production
  traffic finding the inputs nobody thought to generate.

## A quick look

```cpp
#include "gauge15/sfv.hpp"

auto result = sfv::parse_item("42;foo=?1");

if (result.ok()) {
    const sfv::Item& item = result.value();
    // item.value holds sfv::BareItem{int64_t{42}}
    // item.params holds {"foo": true}
}
```

```cpp
// A List (e.g. an Accept-CH header value), including an Inner List with
// its own Parameters:
auto list_result = sfv::parse_list("sugar, tea, (\"milk\" \"honey\");source=farm");

if (list_result.ok()) {
    const sfv::List& list = list_result.value();
    // list[0] and list[1] are Items (Token{"sugar"}, Token{"tea"})
    // list[2] is an InnerList: items = [String{"milk"}, String{"honey"}],
    //                          params = {"source": Token{"farm"}}
}
```

```cpp
// A List (e.g. an Accept-CH header value), including an Inner List with
// its own Parameters:
std::string_view list_input = "sugar, tea, (\"milk\" \"honey\");source=farm";
auto list_result = sfv::detail::parse_list(list_input, 0);

if (list_result.ok()) {
    const sfv::List& list = list_result.value();
    // list[0] and list[1] are Items (Token{"sugar"}, Token{"tea"})
    // list[2] is an InnerList: items = [String{"milk"}, String{"honey"}],
    //                          params = {"source": Token{"farm"}}
}
```

```cpp
// A Dictionary (e.g. a Priority header value), including the Boolean-
// true shorthand ("c" below means c=?1):
auto dict_result = sfv::parse_dictionary("u=1, a=?0, c");

if (dict_result.ok()) {
    const sfv::Dictionary& dict = dict_result.value();
    // dict.get("u") -> Item{BareItem{int64_t{1}}, {}}
    // dict.get("c") -> Item{BareItem{true}, {}}
}
```

```cpp
// Serializing back to wire form, and the empty-container convention:
auto ser = sfv::serialize(dict_result.value());
// ser.value() == "u=1, a=?0, c"

sfv::List empty;
auto empty_ser = sfv::serialize(empty);
// empty_ser.value() == "" -- an empty List/Dictionary means "omit this
// header entirely," which is the caller's decision, not this library's;
// check list.empty() / dict.empty() yourself before emitting a header.
```

## Design notes

- **`Decimal` is fixed-point, not `double`.** RFC 9651's Decimal type is
  defined with an exact 3-digit fractional limit and is meant to
  round-trip exactly; IEEE 754 binary64 can't represent many such values
  exactly (0.1, for instance). `Decimal` stores `value * 1000` as an
  `int64_t` instead, so nothing is lost. See `types.hpp`'s comment on it.
- **`Result<T>`, not exceptions, for parse failure.** RFC 9651 section
  1.1 is explicit that parse failure is a normal, single defined outcome
  ("fail the entire operation"), not an exceptional circumstance —
  modeling that with exceptions would mean wrapping every parse call in
  `try`/`catch` just to check success. `Result<T>` is a minimal, self-
  rolled stand-in for `std::expected` (which is C++23; this project
  targets C++20).
- **`Token` is a wrapper struct; `String` is plain `std::string`.** RFC
  9651 Appendix B calls this out directly: preserving the Token/String
  distinction in a language whose native string type doesn't have that
  distinction requires a wrapper on (at least) one side.
- **`OrderedMap<V>`, not `std::map`.** Both Parameters and Dictionary are
  specified as *ordered* maps with unique keys, where re-setting an
  existing key overwrites its value **in place** rather than moving it to
  the end. Neither property holds for `std::map`/`std::unordered_map`. A
  flat `vector<pair<string, V>>` is also the right performance choice at
  the sizes involved — the RFC's own minimum guarantees are only tens to
  low hundreds of entries, where a linear scan beats a tree or hash table.
- **A real, from-scratch UTF-8 validator** (`gauge15/detail/utf8.hpp`), needed
  because RFC 9651's Display String parsing algorithm requires genuinely
  rejecting malformed input, not just accepting any byte sequence:
  truncated multi-byte sequences, overlong encodings (e.g. encoding 'A'
  as two bytes when one suffices — a classic filter-bypass trick),
  surrogate code points (which UTF-8 must never encode), and code points
  beyond U+10FFFF are all explicitly tested for and rejected.
- **A layering detail caught by the tests, not assumed:** RFC 9651's
  "discard leading SP" step (section 4.2, step 2) belongs to the
  top-level `parse()` entry point, NOT to Parsing a List (4.2.1) itself
  — confirmed by the official `"leading SP list"` test case failing
  until the test harness (standing in for the not-yet-built Phase 6
  top-level function) added that discard itself, rather than baking it
  into `parse_list()` and quietly drifting from what 4.2.1's own
  algorithm actually says.
- **A real bug in the test infrastructure itself, caught by the largest
  conformance file, not assumed away:** `key-generated.json`'s "0x00 as a
  single-character dictionary key" case (and 3 siblings) failed with
  "expected parse failure, but succeeded." The parser was correct — the
  bug was that `GeneratedTestCase::raw` was a bare `const char*`, and
  constructing a `std::string_view` from that calls `strlen()`, which
  stops at the first embedded NUL byte. So input containing a literal
  NUL silently became an *empty* string_view, which trivially "succeeds"
  as an empty Dictionary. Fixed at the root, in the codegen script:
  every generated test case now carries an explicit `raw_len` (and
  `canonical_len`) alongside the pointer, and every test harness
  constructs `std::string_view(tc.raw, tc.raw_len)` instead of relying
  on implicit `const char*` conversion.
- **`sfv::detail::` vs. `sfv::`:** every per-container algorithm
  (`detail::parse_list`, `detail::serialize_item`, etc.) matches its RFC
  subsection exactly and does no more — no whitespace handling, no
  full-consumption check, since 4.2.1/4.1.1/etc. don't specify those.
  The top-level `sfv::parse_list`/`parse_dictionary`/`parse_item` and
  `sfv::serialize()` (`api.hpp`, RFC 9651 section 4.2's top-level steps
  1-8) are thin wrappers adding exactly what's missing. When Phase 6
  landed, every one of the 1,377 pre-existing conformance checks was
  retargeted from `sfv::detail::*` to the new top-level functions
  (removing the manual leading-SP-discard workarounds Phases 4-5 had
  needed) — so the whole existing corpus validates the top-level API too,
  not just the 18 checks in `test-api.cpp` written specifically for it.
- **Every function name matches an RFC subsection.** `parse_integer_or_
  decimal` implements section 4.2.4, `serialize_decimal` implements
  4.1.5, and so on — the RFC and the code can be read side by side.
- **Base64 is self-rolled** (`gauge15/detail/base64.hpp`), not a dependency —
  verified against all 7 official RFC 4648 test vectors, plus RFC 9651's
  own specific leniency requirement (missing "=" padding must be
  tolerated on decode; non-zero pad bits must not cause failure).
- **A real bug in the codegen script, caught by the tests, not assumed
  away:** the official test suite encodes the `binary` `__type`'s
  "value" field in **Base32**, not Base64 — confirmed by comparing e.g.
  `"NBSWY3DP"` (Base32) against `"aGVsbG8="` (Base64) for the same
  underlying bytes, `"hello"`. Decoding it as Base64 first produced
  wrong expected bytes silently (tests failed with the WRONG error --
  "value doesn't match" -- not a crash), which is exactly the kind of
  mistake real conformance testing is supposed to catch. See
  `tools/gen-test-cases.py`'s `try_decode_base32_lenient` docstring.

## Project layout

```
include/gauge15/            the library itself (header-only)
  types.hpp               data model (Section 3)
  parser.hpp               parsing algorithms (Section 4.2)
  serializer.hpp           serialization algorithms (Section 4.1)
  api.hpp                   top-level parse()/serialize() entry points
  result.hpp                Result<T> / Error
  sfv.hpp                    convenience header including all of the above
  detail/base64.hpp        self-rolled base64 codec (Byte Sequence)
  detail/utf8.hpp           self-rolled UTF-8 validator (Display String)
tests/                    one test file per conformance JSON file, plus
                         test-api.cpp for top-level-specific behavior
tests/vectors/            vendored official conformance JSON (14 files)
tests/generated/          codegen output (checked in, no JSON dep at test time)
tools/gen-test-cases.py  JSON conformance file -> C++ test cases
                         (supports "item", "list", and "dictionary"
                         header_type kinds)
fuzz/                    libFuzzer harnesses (see fuzz/README.md) +
                         seeded corpora, one per top-level type
bench/bench.cpp          real throughput measurements (see "Testing &
                         Validation" above), not claimed numbers
.github/workflows/ci.yml GCC/Clang/MSVC x Linux/macOS/Windows matrix
                         (not yet run -- see "Testing & Validation")
single_include/gauge15/sfv.hpp  generated single-header distribution
                             (see single_include/README.md)
tools/amalgamate.py     generates single_include/gauge15/sfv.hpp from
                         include/gauge15/*.hpp
```

## License

MIT — see `LICENSE`. (The copyright line there has a placeholder for
the actual name/organization; fill that in before distributing.)
