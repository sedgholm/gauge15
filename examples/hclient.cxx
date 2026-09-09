// hclient.cpp -- fetch a URL, validate its RFC 9651 Structured Field headers.
//
// Usage: hclient <url>
// Build: g++ -std=c++20 -O2 -I. hclient.cpp -o hclient -lcurl

#include "gauge15/sfv.hpp"

#include <curl/curl.h>

#include <algorithm>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace {

enum class Kind { Item, List, Dictionary };

const std::map<std::string, Kind> kKnownHeaders = {
    {"accept-ch", Kind::List},
    {"cache-status", Kind::List},
    {"proxy-status", Kind::List},
    {"priority", Kind::Dictionary},
    {"repr-digest", Kind::Dictionary},
    {"content-digest", Kind::Dictionary},
    {"want-repr-digest", Kind::Dictionary},
    {"want-content-digest", Kind::Dictionary},
    {"signature-input", Kind::Dictionary},
    {"signature", Kind::Dictionary},
    {"sec-ch-ua", Kind::List},
    {"sec-ch-ua-mobile", Kind::Item},
    {"sec-ch-ua-platform", Kind::Item},
    {"sec-ch-ua-full-version-list", Kind::List},
    {"sec-ch-prefers-color-scheme", Kind::Item},
};

std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t");
    size_t b = s.find_last_not_of(" \t\r\n");
    return a == std::string::npos ? "" : s.substr(a, b - a + 1);
}

const char* kind_name(Kind k) {
    switch (k) {
        case Kind::Item: return "Item";
        case Kind::List: return "List";
        case Kind::Dictionary: return "Dictionary";
    }
    return "?";
}

struct Response {
    bool ok = false;
    std::string error;
    std::string status_line;
    std::string effective_url;
    std::vector<std::pair<std::string, std::string>> headers;
};

size_t on_header(char* buf, size_t size, size_t n, void* user) {
    size_t len = size * n;
    auto* resp = static_cast<Response*>(user);
    std::string line(buf, len);
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();

    if (line.rfind("HTTP/", 0) == 0) {
        resp->headers.clear();  // new hop (redirect) -- keep only the final one
        resp->status_line = line;
        return len;
    }
    size_t colon = line.find(':');
    if (colon == std::string::npos) return len;
    resp->headers.emplace_back(trim(line.substr(0, colon)), trim(line.substr(colon + 1)));
    return len;
}

size_t on_body(char*, size_t size, size_t n, void*) { return size * n; }

Response fetch(const std::string& url) {
    Response resp;
    CURL* curl = curl_easy_init();
    if (!curl) {
        resp.error = "curl_easy_init failed";
        return resp;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, on_header);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_body);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "hclient/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        resp.error = curl_easy_strerror(rc);
        curl_easy_cleanup(curl);
        return resp;
    }

    char* effective = nullptr;
    curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effective);
    if (effective) resp.effective_url = effective;

    curl_easy_cleanup(curl);
    resp.ok = true;
    return resp;
}

std::map<std::string, std::string> combine(const std::vector<std::pair<std::string, std::string>>& lines) {
    std::map<std::string, std::string> out;
    for (const auto& [name, value] : lines) {
        std::string key = lower(name);
        auto it = out.find(key);
        if (it == out.end()) {
            out[key] = value;
        } else {
            it->second += ", " + value;
        }
    }
    return out;
}

void inspect(const std::string& name, const std::string& raw, Kind kind) {
    std::printf("%s: %s\n", name.c_str(), raw.c_str());
    std::printf("  type: %s\n", kind_name(kind));

    std::string canonical;
    bool ok = false, roundtrip = false;
    sfv::Error error{"", 0};

    auto check = [&](auto parse_fn) {
        auto r = parse_fn(raw);
        ok = r.ok();
        if (!ok) {
            error = r.error();
            return;
        }
        auto ser = sfv::serialize(r.value());
        if (!ser.ok()) return;
        canonical = ser.value();
        auto reparsed = parse_fn(canonical);
        roundtrip = reparsed.ok() && reparsed.value() == r.value();
    };

    switch (kind) {
        case Kind::Item: check([](const std::string& s) { return sfv::parse_item(s); }); break;
        case Kind::List: check([](const std::string& s) { return sfv::parse_list(s); }); break;
        case Kind::Dictionary: check([](const std::string& s) { return sfv::parse_dictionary(s); }); break;
    }

    if (!ok) {
        std::printf("  parse: FAIL (%s)\n", error.message.c_str());
    } else {
        std::printf("  parse: OK\n");
        std::printf("  canonical: %s\n", canonical.c_str());
        std::printf("  roundtrip: %s\n", roundtrip ? "OK" : "FAIL");
    }
    std::printf("\n");
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s <url>\n", argv[0]);
        return 1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    Response resp = fetch(argv[1]);
    curl_global_cleanup();

    if (!resp.ok) {
        std::fprintf(stderr, "error: %s\n", resp.error.c_str());
        return 1;
    }

    std::printf("%s\n%s\n\n", resp.effective_url.c_str(), resp.status_line.c_str());

    auto headers = combine(resp.headers);
    int found = 0;
    for (const auto& [name, value] : headers) {
        auto it = kKnownHeaders.find(name);
        if (it == kKnownHeaders.end()) continue;
        inspect(name, value, it->second);
        ++found;
    }
    if (found == 0) {
        std::printf("no structured field headers found\n");
    }

    return 0;
}
