#include "sfv/sfv.hpp"
#include <iostream>
#include <vector>
#include <string>

int main()
{
    std::vector<std::string> tests =
    {
        // ---------- Empty ----------
        "",
        " ",
        "\t",
        "    ",

        // ---------- Leading/trailing commas ----------
        ",",
        ",,",
        ",a",
        "a,",
        ",a,",
        "a,,b",
        ",,a,,",

        // ---------- Weird whitespace ----------
        "a =1",
        "a= 1",
        "a = 1",
        "a\t=1",
        "a=\t1",
        "a,\tb",
        "a,\n b",
        "a,\r\nb",

        // ---------- Huge integer overflow ----------
        "999999999999999999999999999999999999999",
        "-99999999999999999999999999999999999999",

        // ---------- Decimal overflow ----------
        "999999999999999999999999.999",
        "-999999999999999999999999.999",

        // ---------- Decimal syntax ----------
        ".5",
        "1.",
        "1.0000",
        "01.0",
        "-01.0",

        // ---------- String escapes ----------
        "\"\\\"",
        "\"\\\\\\\\\\\\\\\\\"",
        "\"\\a\"",
        "\"\\u1234\"",
        "\"\\x41\"",
        "\"\\\"\\\"\\\"\"",

        // ---------- Byte sequences ----------
        ":",
        "::",
        ":=:",
        ":AA:",
        ":A=:",
        ":====:",
        ":////////:",
        ":AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA:",

        // ---------- Display strings ----------
        "%\"\"",
        "%\"hello\"",
        "%\"😀\"",
        "%\"مرحبا\"",
        "%\"こんにちは\"",

        // invalid UTF-8
        std::string("%\"") + char(0xF0) + char(0x80) + char(0x80) + char(0x80) + "\"",

        // ---------- Dates ----------
        "@0",
        "@-1",
        "@999999999999999",
        "@999999999999999999999999",

        // ---------- Parameters ----------
        "abc;",
        "abc;;",
        "abc;a",
        "abc;a=",
        "abc;a==1",
        "abc;a=?2",
        "abc;a=:AA==:",

        // ---------- Lists ----------
        "()",
        "( )",
        "(1)",
        "(1 2 3)",
        "((1))",
        "(1,(2))",
        "(1 2",
        "1 2)",

        // ---------- Dictionaries ----------
        "a",
        "a=?1",
        "a=1,b",
        "a=1,,b=2",
        "a=1,b=2,",
        "a=1,b=2,,",
        "a=1,a=2,a=3,a=4",

        // ---------- NUL ----------
        std::string("abc\0xyz",7),

        // ---------- Long token ----------
        std::string(500000,'a'),

        // ---------- Long quoted string ----------
        "\"" + std::string(500000,'x') + "\"",
    };

    for (size_t i=0;i<tests.size();++i)
    {
        auto r1 = sfv::parse_item(tests[i]);
        auto r2 = sfv::parse_list(tests[i]);
        auto r3 = sfv::parse_dictionary(tests[i]);

        std::cout
            << "[" << i << "] "
            << "item=" << r1.ok()
            << " list=" << r2.ok()
            << " dict=" << r3.ok()
            << '\n';

        if (r1.ok())
        {
            auto s = sfv::serialize(r1.value());
            if (!s.ok())
                std::cout << "  SERIALIZER FAILED!\n";
        }

        if (r2.ok())
        {
            auto s = sfv::serialize(r2.value());
            if (!s.ok())
                std::cout << "  SERIALIZER FAILED!\n";
        }

        if (r3.ok())
        {
            auto s = sfv::serialize(r3.value());
            if (!s.ok())
                std::cout << "  SERIALIZER FAILED!\n";
        }
    }
}
