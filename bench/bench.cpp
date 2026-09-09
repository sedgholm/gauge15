// bench.cpp
//
// Measures actual parse/serialize throughput on realistic HTTP
// Structured Field header values -- not a claim, a number produced by
// running the code. Each benchmark: warm up, then run enough iterations
// to get a stable median across repeated trials (throughput
// measurements on a shared/virtualized CPU are noisy trial-to-trial;
// reporting the median of several trials is more honest than a single
// run's number).
//
// A `volatile` sink prevents the compiler from proving the parsed/
// serialized result is unused and optimizing the whole loop away --
// without it, a sufficiently aggressive optimizer could legally reduce
// this entire file to a no-op, since nothing observable depends on the
// result.
//
// Build: g++ -std=c++20 -O2 -Iinclude bench/bench.cpp -o bench/bench
// Run:   ./bench/bench

#include "gauge15/sfv.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

namespace {

volatile size_t g_sink = 0;  // see file header comment

using Clock = std::chrono::steady_clock;

double median(std::vector<double> values) {
    std::sort(values.begin(), values.end());
    return values[values.size() / 2];
}

// Runs `fn()` repeatedly for approximately `target_seconds`, returning
// (iterations, elapsed_seconds) from a single trial.
template <typename Fn>
std::pair<long long, double> run_trial(Fn&& fn, double target_seconds) {
    auto start = Clock::now();
    long long iterations = 0;
    double elapsed = 0.0;
    // Chunk the timing check so we're not calling steady_clock::now()
    // on every single iteration for very fast operations (that overhead
    // would itself dominate the measurement for a sub-microsecond parse).
    constexpr long long kChunk = 1000;
    while (elapsed < target_seconds) {
        for (long long i = 0; i < kChunk; ++i) {
            fn();
        }
        iterations += kChunk;
        elapsed = std::chrono::duration<double>(Clock::now() - start).count();
    }
    return {iterations, elapsed};
}

template <typename Fn>
void report(const std::string& name, size_t bytes_per_op, Fn&& fn, int trials = 5,
            double target_seconds = 0.3) {
    // Warm up once, unmeasured (first-touch page faults, branch
    // predictor warmup, etc. would otherwise bias the first trial low).
    run_trial(fn, 0.05);

    std::vector<double> ops_per_sec;
    for (int t = 0; t < trials; ++t) {
        auto [iterations, elapsed] = run_trial(fn, target_seconds);
        ops_per_sec.push_back(static_cast<double>(iterations) / elapsed);
    }

    double median_ops = median(ops_per_sec);
    double mb_per_sec = (median_ops * static_cast<double>(bytes_per_op)) / (1024.0 * 1024.0);

    std::printf("%-38s %12.0f ops/sec   %8.1f MB/sec   (median of %d trials)\n", name.c_str(),
                median_ops, mb_per_sec, trials);
}

}  // namespace

int main() {
    std::printf("gauge15 benchmark -- measured on this machine, not a portable claim.\n");
    std::printf("Build: -O2, single-threaded, median of repeated trials.\n\n");

    // Realistic Structured Field header values, roughly representative
    // of what real HTTP headers using this format look like in practice
    // (Accept-CH, Cache-Control-style Dictionary, Priority, a Cookie-
    // like list of tokens with parameters).
    const std::string small_item = "42";
    const std::string typical_item = "\"gzip\";q=1.0";
    const std::string typical_list = "sec-ch-ua-platform, sec-ch-ua-mobile, sec-ch-ua";
    const std::string typical_dict =
        "u=1, i, a=?0, s=\"text/plain\";charset=\"utf-8\", max-age=3600";
    const std::string large_list = [] {
        std::string s;
        for (int i = 0; i < 50; ++i) {
            if (i) s += ", ";
            s += "\"token-" + std::to_string(i) + "\";weight=" + std::to_string(i % 10);
        }
        return s;
    }();

    std::printf("--- parse() ---\n");
    report("parse_item (small integer)", small_item.size(),
           [&] { g_sink += sfv::parse_item(small_item).ok(); });
    report("parse_item (token + param)", typical_item.size(),
           [&] { g_sink += sfv::parse_item(typical_item).ok(); });
    report("parse_list (3 tokens)", typical_list.size(),
           [&] { g_sink += sfv::parse_list(typical_list).ok(); });
    report("parse_dictionary (5 mixed members)", typical_dict.size(),
           [&] { g_sink += sfv::parse_dictionary(typical_dict).ok(); });
    report("parse_list (50 params, ~500 bytes)", large_list.size(),
           [&] { g_sink += sfv::parse_list(large_list).ok(); });

    std::printf("\n--- serialize() (parsing once outside the timed loop) ---\n");
    auto item_value = sfv::parse_item(typical_item).value();
    auto list_value = sfv::parse_list(typical_list).value();
    auto dict_value = sfv::parse_dictionary(typical_dict).value();
    auto large_list_value = sfv::parse_list(large_list).value();

    report("serialize (token + param)", typical_item.size(),
           [&] { g_sink += sfv::serialize(item_value).ok(); });
    report("serialize (3-token list)", typical_list.size(),
           [&] { g_sink += sfv::serialize(list_value).ok(); });
    report("serialize (5-member dictionary)", typical_dict.size(),
           [&] { g_sink += sfv::serialize(dict_value).ok(); });
    report("serialize (50-param list, ~500 bytes)", large_list.size(),
           [&] { g_sink += sfv::serialize(large_list_value).ok(); });

    std::printf("\n--- what this means concretely ---\n");
    auto [iters, elapsed] = run_trial(
        [&] { g_sink += sfv::parse_dictionary(typical_dict).ok(); }, 1.0);
    double ops_per_sec = static_cast<double>(iters) / elapsed;
    std::printf(
        "At %.0f parses/sec for a typical %zu-byte Dictionary header,\n"
        "a single core could handle ~%.0f such headers/sec if parsing were\n"
        "the only work being done (real servers do far more per request,\n"
        "so this is an upper bound on this one operation, not a request-\n"
        "handling capacity estimate).\n",
        ops_per_sec, typical_dict.size(), ops_per_sec);

    return static_cast<int>(g_sink % 2);  // touch the sink so it's not provably dead
}
