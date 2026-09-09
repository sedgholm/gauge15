#!/usr/bin/env python3
"""gen-test-cases.py

Converts an official httpwg/structured-field-tests JSON conformance file
into a C++ header containing an array of test cases built directly out
of this library's own sfv:: types (sfv::Item, sfv::List, etc.) -- not
out of a generic JSON representation. This means the test binaries never
need a JSON parsing dependency at all: the "JSON-ness" of the input data
is fully resolved at codegen time, in Python.

PHASE STATUS: supports header_type "item" and "list" (Phase 4 added
"list", including Inner Lists and their own Parameters). "dictionary" is
not yet supported (Phase 5) and its cases are counted and reported, not
silently dropped -- the generated .inc file's own header comment states
exactly how many of a file's cases are included this phase.

Usage:
  python3 gen-test-cases.py <input.json> <output.inc> <cpp_array_name> [kind]

`kind` is "item" (default), "list", or "dictionary", selecting which
header_type this invocation extracts from the input file (a single JSON
file can be run multiple times, once per kind, if it mixes several --
e.g. number.json and token.json each have a handful of "list"-typed
cases alongside mostly "item"-typed ones, and key-generated.json mixes
"dictionary" and "list").
"""

import base64
import json
import sys
from pathlib import Path


def http_field_line_join(raw_lines):
    """RFC 9651 section 4.2: multiple field lines with the same name are
    combined into one comma-separated value per RFC 9110 section 5.2,
    before parsing ever sees them."""
    return ", ".join(raw_lines)


def cpp_string_literal(s):
    """Returns a complete, quoted C++ string literal for `s` (a Python str
    where each character represents one byte, 0-255 -- i.e. produced via
    .decode('latin-1') for raw binary data). Includes the surrounding
    quotes itself, unlike a plain escaper, because it sometimes needs to
    SPLIT the literal into multiple adjacent-concatenated pieces:
    a "\\xHH" hex escape in C++ greedily consumes any following character
    that looks like a hex digit, extending the escape rather than ending
    it (e.g. "\\x001" would be parsed as the single hex escape \\x001,
    not \\x00 followed by the character '1'). Since arbitrary Byte
    Sequence test data can contain any byte followed by any other byte,
    this must be defended against generically, not just spot-fixed for
    cases observed in the current test files -- a hex escape is always
    followed by a literal break (closing and reopening the string) when
    the next character would otherwise be misread as part of it.
    """
    out = ['"']
    last_was_hex_escape = False
    for ch in s:
        code = ord(ch)
        is_hex_digit_char = ch in "0123456789abcdefABCDEF"
        if last_was_hex_escape and is_hex_digit_char:
            out.append('" "')  # close, then reopen: breaks the escape sequence
        last_was_hex_escape = False

        if ch == "\\":
            out.append("\\\\")
        elif ch == '"':
            out.append('\\"')
        elif ch == "\n":
            out.append("\\n")
        elif ch == "\t":
            out.append("\\t")
        elif code < 0x20 or code > 0x7E:
            out.append(f"\\x{code:02x}")
            last_was_hex_escape = True
        else:
            out.append(ch)
    out.append('"')
    return "".join(out)


def try_decode_base64_lenient(b64_text):
    """Standard base64 decode, tolerating missing padding (pads to the
    next multiple of 4 first) -- the same leniency RFC 9651 4.2.7
    requires of a real parser. Returns None on genuine failure rather
    than raising, so callers can fall back to another source."""
    try:
        padded = b64_text + "=" * (-len(b64_text) % 4)
        return base64.b64decode(padded, validate=False)
    except Exception:
        return None


def try_decode_base32_lenient(b32_text):
    """The httpwg/structured-field-tests suite encodes the `binary`
    __type's "value" field in Base32 (RFC 4648 Section 6), NOT the Base64
    used by the actual Structured Fields wire format -- confirmed by
    comparing e.g. "NBSWY3DP" (Base32) against "aGVsbG8=" (Base64) for
    the same underlying bytes, "hello". Using the wrong one of the two
    here would silently produce wrong expected bytes for every binary
    test case rather than failing loudly, so this is named and
    documented separately from the base64 decoder above on purpose."""
    try:
        padded = b32_text + "=" * (-len(b32_text) % 8)
        return base64.b32decode(padded, casefold=True)
    except Exception:
        return None


def extract_raw_wire_binary_bytes(raw_lines):
    """Given the test case's own `raw` field (list of wire-format lines),
    finds a ':...:'-delimited Byte Sequence and decodes its content. Used
    as a fallback when the test suite's `expected.value` field doesn't
    decode as clean base64 -- the wire form is unambiguous by
    construction (it's exactly what parse_byte_sequence itself consumes),
    so this sidesteps needing to second-guess the test suite's own JSON
    encoding convention for that field."""
    joined = http_field_line_join(raw_lines)
    start = joined.find(":")
    end = joined.find(":", start + 1)
    if start == -1 or end == -1:
        return None
    return try_decode_base64_lenient(joined[start + 1 : end])


def emit_bare_item_cpp(value, raw_lines_for_fallback=None):
    """Given a JSON-decoded bare-item value (per the test suite's own
    encoding rules), returns a C++ expression constructing the matching
    sfv::BareItem. Returns None if this bare item type isn't supported
    yet (caller should skip the whole test case in that event)."""
    if isinstance(value, bool):
        return f"sfv::BareItem{{{'true' if value else 'false'}}}"
    if isinstance(value, int):
        return f"sfv::BareItem{{int64_t{{{value}}}}}"
    if isinstance(value, float):
        # Reconstruct the exact scaled-by-1000 integer from the JSON
        # float. The test suite's own Decimal values are always exactly
        # representable to 3 fractional digits by construction (that's
        # the type being tested), so round() here recovers the intended
        # exact value despite float's binary imprecision.
        scaled = round(value * 1000)
        return f"sfv::BareItem{{sfv::Decimal::from_scaled({scaled})}}"
    if isinstance(value, str):
        # Plain JSON string == the test suite's encoding for Structured
        # Fields' String type (Section 3.3.3).
        return f"sfv::BareItem{{std::string({cpp_string_literal(value)})}}"
    if isinstance(value, dict) and "__type" in value:
        type_tag = value["__type"]
        if type_tag == "token":
            return f'sfv::BareItem{{sfv::Token{{{cpp_string_literal(value["value"])}}}}}'
        if type_tag == "binary":
            raw_bytes = try_decode_base32_lenient(value["value"])
            if raw_bytes is None and raw_lines_for_fallback is not None:
                # See extract_raw_wire_binary_bytes's docstring: at least
                # one official test case ("non-ASCII binary") has an
                # expected.value field that isn't valid Base32 under any
                # padding interpretation. Falling back to decoding the
                # unambiguous raw wire form (Base64, per the actual wire
                # format) recovers the correct expected bytes without
                # guessing at the test suite's intent for that field.
                raw_bytes = extract_raw_wire_binary_bytes(raw_lines_for_fallback)
            if raw_bytes is None:
                return None
            escaped = cpp_string_literal(raw_bytes.decode("latin-1"))
            return f"sfv::BareItem{{sfv::ByteSequence{{{escaped}}}}}"
        if type_tag == "date":
            return f'sfv::BareItem{{sfv::Date{{int64_t{{{value["value"]}}}}}}}'
        if type_tag == "displaystring":
            # The test suite's "value" is already-decoded Unicode text
            # (a plain Python/JSON string); DisplayString stores UTF-8
            # bytes internally, so re-encode it here.
            utf8_bytes = value["value"].encode("utf-8").decode("latin-1")
            escaped = cpp_string_literal(utf8_bytes)
            return f"sfv::BareItem{{sfv::DisplayString{{{escaped}}}}}"
        return None
    return None


def emit_parameters_cpp(params_json):
    """params_json is a JSON array of [key, value] pairs (per the test
    suite's Parameters encoding). Returns a C++ expression constructing
    the matching sfv::Parameters, or None if any value uses an
    unsupported bare item type."""
    entries = []
    for key, value in params_json:
        item_cpp = emit_bare_item_cpp(value)
        if item_cpp is None:
            return None
        entries.append((key, item_cpp))

    lines = ["[]{ sfv::Parameters p;"]
    for key, item_cpp in entries:
        lines.append(f"p.set({cpp_string_literal(key)}, {item_cpp});")
    lines.append("return p; }()")
    return " ".join(lines)


def emit_item_tuple_cpp(item_json, raw_lines_for_fallback=None):
    """item_json is [bare_item_value, parameters_array] (the test suite's
    Item encoding -- used both for top-level Item tests and for each
    Item inside an Inner List's items array). Returns a C++ expression
    constructing the matching sfv::Item, or None if unsupported."""
    bare_value, params_json = item_json
    bare_cpp = emit_bare_item_cpp(bare_value, raw_lines_for_fallback)
    if bare_cpp is None:
        return None
    params_cpp = emit_parameters_cpp(params_json)
    if params_cpp is None:
        return None
    return f"sfv::Item{{{bare_cpp}, {params_cpp}}}"


def emit_member_cpp(member_json, raw_lines_for_fallback=None):
    """member_json is [value_or_items, parameters_array] -- the test
    suite's encoding for a List/Dictionary member. `value_or_items` is
    EITHER a bare item value (a plain Item member) OR a JSON array of
    [bare_item, params] tuples (an Inner List's items) -- discriminated
    by whether it's a list. Returns a C++ expression constructing the
    matching sfv::Member, or None if unsupported."""
    value_or_items, params_json = member_json

    if isinstance(value_or_items, list):
        item_cpps = []
        for item_json in value_or_items:
            item_cpp = emit_item_tuple_cpp(item_json, raw_lines_for_fallback)
            if item_cpp is None:
                return None
            item_cpps.append(item_cpp)
        params_cpp = emit_parameters_cpp(params_json)
        if params_cpp is None:
            return None
        items_vector = "std::vector<sfv::Item>{" + ", ".join(item_cpps) + "}"
        return f"sfv::Member{{sfv::InnerList{{{items_vector}, {params_cpp}}}}}"

    item_cpp = emit_item_tuple_cpp([value_or_items, params_json], raw_lines_for_fallback)
    if item_cpp is None:
        return None
    return f"sfv::Member{{{item_cpp}}}"


def emit_list_cpp(list_json, raw_lines_for_fallback=None):
    """list_json is the full "expected" array for a header_type=="list"
    test case: a JSON array of member encodings (see emit_member_cpp).
    Returns a C++ expression constructing the matching sfv::List, or None
    if any member is unsupported."""
    member_cpps = []
    for member_json in list_json:
        member_cpp = emit_member_cpp(member_json, raw_lines_for_fallback)
        if member_cpp is None:
            return None
        member_cpps.append(member_cpp)
    return "sfv::List{" + ", ".join(member_cpps) + "}"


def emit_dictionary_cpp(dict_json, raw_lines_for_fallback=None):
    """dict_json is the full "expected" array for a header_type==
    "dictionary" test case: a JSON array of [key, member_json] pairs.
    Returns a C++ expression constructing the matching sfv::Dictionary
    (built via a lambda that calls .set() in order, so insertion order --
    which OrderedMap treats as significant -- matches the test data),
    or None if any member is unsupported."""
    entries = []
    for key, member_json in dict_json:
        member_cpp = emit_member_cpp(member_json, raw_lines_for_fallback)
        if member_cpp is None:
            return None
        entries.append((key, member_cpp))

    lines = ["[]{ sfv::Dictionary d;"]
    for key, member_cpp in entries:
        lines.append(f"d.set({cpp_string_literal(key)}, {member_cpp});")
    lines.append("return d; }()")
    return " ".join(lines)


def main():
    if len(sys.argv) not in (4, 5):
        print(
            f"usage: {sys.argv[0]} <input.json> <output.inc> <cpp_array_name> [kind: item|list]",
            file=sys.stderr,
        )
        sys.exit(1)

    input_path, output_path, array_name = sys.argv[1], sys.argv[2], sys.argv[3]
    kind = sys.argv[4] if len(sys.argv) == 5 else "item"
    assert kind in ("item", "list", "dictionary"), f"unsupported kind {kind!r}"

    data = json.loads(Path(input_path).read_text(encoding="utf-8"))

    cpp_type = {
        "item": "sfv::Item",
        "list": "sfv::List",
        "dictionary": "sfv::Dictionary",
    }[kind]
    placeholder_expr = {
        "item": "sfv::Item{int64_t{0}, sfv::Parameters{}}",
        "list": "sfv::List{}",
        "dictionary": "sfv::Dictionary{}",
    }[kind]

    emitted = []
    skipped_wrong_header_type = 0
    skipped_unsupported_bare_item = 0

    for test in data:
        name = test["name"]
        header_type = test["header_type"]

        if header_type != kind:
            skipped_wrong_header_type += 1
            continue

        raw_joined = http_field_line_join(test["raw"]) if "raw" in test else None
        must_fail = test.get("must_fail", False)

        if must_fail:
            emitted.append(
                {"name": name, "raw": raw_joined, "must_fail": True, "expected_cpp": None,
                 "canonical": None}
            )
            continue

        if kind == "item":
            expected_cpp = emit_item_tuple_cpp(test["expected"], test.get("raw"))
        elif kind == "list":
            expected_cpp = emit_list_cpp(test["expected"], test.get("raw"))
        else:
            expected_cpp = emit_dictionary_cpp(test["expected"], test.get("raw"))

        if expected_cpp is None:
            skipped_unsupported_bare_item += 1
            continue

        canonical = test.get("canonical")
        canonical_joined = (
            http_field_line_join(canonical) if canonical is not None else raw_joined
        )

        emitted.append(
            {"name": name, "raw": raw_joined, "must_fail": False, "expected_cpp": expected_cpp,
             "canonical": canonical_joined}
        )

    print(
        f"# {input_path} (kind={kind}): {len(emitted)} emitted, "
        f"{skipped_wrong_header_type} skipped (header_type != {kind}), "
        f"{skipped_unsupported_bare_item} skipped (unsupported type)",
        file=sys.stderr,
    )

    out = []
    out.append(f"// GENERATED from {input_path} (kind={kind}) by tools/gen-test-cases.py. DO NOT EDIT.")
    out.append(f"// {len(emitted)} of {len(data)} official test cases included in this phase")
    out.append(f"// ({skipped_wrong_header_type} non-{kind} header_type, "
               f"{skipped_unsupported_bare_item} unsupported type, deferred to a later phase).")
    out.append("")
    out.append("struct GeneratedTestCase {")
    out.append("    const char* name;")
    out.append("    const char* raw;")
    out.append("    size_t raw_len;  // use this, not strlen(raw) -- raw may contain embedded NUL")
    out.append("    bool must_fail;")
    out.append(f"    {cpp_type} expected;  // meaningful only if !must_fail")
    out.append("    const char* canonical;  // meaningful only if !must_fail")
    out.append("    size_t canonical_len;  // same NUL caveat as raw_len")
    out.append("};")
    out.append("")
    out.append(f"inline const std::vector<GeneratedTestCase>& {array_name}() {{")
    out.append("    static const std::vector<GeneratedTestCase> cases = {")
    for t in emitted:
        raw_cpp = cpp_string_literal(t["raw"]) if t["raw"] is not None else '""'
        raw_len = len(t["raw"]) if t["raw"] is not None else 0
        name_cpp = cpp_string_literal(t["name"])
        if t["must_fail"]:
            out.append(
                f"        {{{name_cpp}, {raw_cpp}, {raw_len}, true, {placeholder_expr}, \"\", 0}},"
            )
        else:
            canonical_cpp = cpp_string_literal(t["canonical"])
            canonical_len = len(t["canonical"]) if t["canonical"] is not None else 0
            out.append(
                f'        {{{name_cpp}, {raw_cpp}, {raw_len}, false, '
                f'{t["expected_cpp"]}, {canonical_cpp}, {canonical_len}}},'
            )
    out.append("    };")
    out.append("    return cases;")
    out.append("}")
    out.append("")

    Path(output_path).write_text("\n".join(out), encoding="utf-8")


if __name__ == "__main__":
    main()
