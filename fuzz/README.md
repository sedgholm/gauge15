# fuzz/

libFuzzer harnesses for the three top-level parse functions. Each one
checks two properties, not just "doesn't crash":

1. **No crash, no ASan/UBSan finding**, on any byte sequence libFuzzer
   generates — including invalid UTF-8, embedded NULs, arbitrary binary
   garbage. Rejecting malformed input cleanly (returning an `Error`) is
   success; a crash is the only thing being hunted for.
2. **Parse → serialize → reparse idempotency**: if parsing succeeds,
   serializing the result and re-parsing *that* must also succeed and
   produce an equal value. This would catch a serializer producing
   syntax its own parser can't read back, or a parser accepting
   something it then serializes into a different value — a real
   correctness property, not just crash-freedom.

## Results so far

Run on this machine (1 vCPU, Intel Xeon @ 2.10GHz), Clang 18.1.3,
`-fsanitize=fuzzer,address,undefined`, seeded from all 14 vendored
conformance JSON files (834 seed inputs total across the three corpora):

| Target | Runs | Crashes found |
|---|---|---|
| `fuzz-item` (`sfv::parse_item`) | 4,308,490 | 0 |
| `fuzz-list` (`sfv::parse_list`) | 2,651,305 | 0 |
| `fuzz-dictionary` (`sfv::parse_dictionary`) | 3,067,617 | 0 |
| **Total** | **10,027,412** | **0** |

That's ~2 minutes per target. This is real signal, not a claim — but it
is 2 minutes per target, not the hours/days a project would run this in
an actual CI fuzzing lane (e.g. OSS-Fuzz). Longer runs, and runs on
different hardware/OS, could still find something these didn't; treat
"10 million runs, zero crashes" as meaningfully more confidence than
"conformance tests pass," not as "proven correct."

One real, legitimate finding *did* come out of getting the toolchain
working: building with `-Wconversion -Wsign-conversion` (stricter than
the `-Wall -Wextra -Wpedantic` the test suites build with) surfaced 5
genuine signedness-conversion warnings in `base64.hpp` and
`serializer.hpp` — harmless in practice (values were always in valid
byte range) but worth fixing cleanly, and fixed before this fuzzing run.

## Building

Requires Clang with libFuzzer support (`libclang-rt-<version>-dev` on
Debian/Ubuntu; GCC does not support `-fsanitize=fuzzer`):

```sh
clang++ -std=c++20 -O1 -g -I../include -fsanitize=fuzzer,address,undefined \
  fuzz-item.cpp -o fuzz-item
# same for fuzz-list.cpp, fuzz-dictionary.cpp
```

## Running

```sh
./fuzz-item corpus/item -max_total_time=120
./fuzz-list corpus/list -max_total_time=120
./fuzz-dictionary corpus/dictionary -max_total_time=120
```

Any crash gets written to `findings/<target>-crash-<hash>`; re-running
`./fuzz-item findings/item-crash-<hash>` reproduces it directly for
debugging. Longer runs (hours) are straightforward: bump
`-max_total_time`, or drop it and let the run continue until stopped.

## Corpus

`corpus/{item,list,dictionary}/` are seeded from every `raw` value in
the vendored conformance JSON (`tests/vectors/*.json`), split by which
top-level type each file's cases target. This gives libFuzzer's mutation
engine real, spec-relevant starting points (valid syntax, deliberately
malformed syntax, edge-case values) instead of an empty corpus it would
otherwise need far more time to discover structure from scratch.
