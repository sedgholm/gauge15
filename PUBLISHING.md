# Publishing gauge15 on Codeberg

Commands for taking this repo from an unversioned worktree to a tagged
release on Codeberg, in order.

## Tools

- git
- cmake >= 3.16, g++ >= 11 or clang++ >= 12 (C++20)
- python3, only if you regenerate `single_include/` or the generated tests
- a Codeberg account and an empty repo (codeberg.org → New repository,
  leave "Initialize this repository" off)

Codeberg's SSH runs on port 2222, so the remote is
`ssh://git@codeberg.org:2222/<you>/gauge15.git`; the exact URL is on the
repo page under "Clone". HTTPS works too, with username + token.

## 1. License

`LICENSE` has a placeholder copyright line. Put your name or handle in and
commit it before publishing.

Note: RFC 9651's code components are Revised BSD (IETF Trust). This is an
independent MIT implementation, which is fine, but don't copy the RFC's
code text in without the BSD attribution.

## 2. .gitignore

The worktree contains ~1 MB compiled binaries (`out/`, `test`, `hclient`,
`bench/bench`, `fuzz/fuzz-*`) that must not be committed:

```gitignore
/build/
/out/
test
hclient
bench/bench
fuzz/fuzz-item
fuzz/fuzz-list
fuzz/fuzz-dictionary
fuzz/findings/
```

## 3. Commit

```sh
git init -b main
git add .
git commit -m "Initial commit: RFC 9651 structured field values for HTTP"
```

If git has no identity set:

```sh
git config user.name  "Your Name"
git config user.email "you@example.com"
```

## 4. Push

```sh
git remote add origin ssh://git@codeberg.org:2222/<you>/gauge15.git
git push -u origin main
```

## 5. Build and test before tagging

```sh
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The CI strict-warning pass is cheap to run now, while CI itself may not
run at all (step 8):

```sh
for f in include/sfv/*.hpp include/sfv/detail/*.hpp; do
  clang++ -std=c++20 -Wall -Wextra -Wpedantic -Wconversion \
    -Wshadow -Wsign-conversion -Wold-style-cast -Werror \
    -c -x c++ "$f" -o /dev/null -Iinclude
done
```

## 6. Regenerate the single header

```sh
python3 tools/amalgamate.py > single_include/sfv/sfv.hpp
g++ -std=c++20 -Isingle_include -Itests tests/test-key-generated.cpp -o /tmp/t && /tmp/t
```

If the file changed, commit it together with the source change that caused
it.

## 7. Tag and release

```sh
git tag -a v1.0.0 -m "gauge15 1.0.0"
git push origin v1.0.0
```

In the web UI: Releases → Create new release → pick the tag → notes →
attach `single_include/sfv/sfv.hpp`. Forgejo releases are tag-bound, so
tag first. The single header is the only file users need, and since
Codeberg has no C++ package registry, the release is the channel.

## 8. CI

`.github/workflows/ci.yml` is GitHub Actions syntax and has never run.
Codeberg runs Forgejo Actions, which is broadly compatible and enabled on
codeberg.org's hosted runners — the workflow *may* work as-is, but treat
the first run as untested. Alternatively Woodpecker CI reads a
`.woodpecker.yml`:

```yaml
pipeline:
  build-and-test:
    image: docker.io/library/gcc:13
    commands:
      - cmake -B build -DCMAKE_BUILD_TYPE=Release
      - cmake --build build --parallel
      - ctest --test-dir build --output-on-failure
```

After one passing run, update the README line that says CI has not run.

## 9. Stale README after publishing

- "this project has never actually been pushed to GitHub" — it now lives
  on Codeberg.
- CI "not run yet" — after step 8.

Optional: a push mirror from Codeberg to GitHub (Settings → Repositories →
Mirrors) lets the GitHub Actions workflow actually run and gives you a
presence on both forges.
