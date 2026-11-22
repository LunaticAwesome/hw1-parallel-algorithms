#include <iostream>
#include <iomanip>
#include <chrono>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <csignal>
#include <parlay/sequence.h>
#include "quick_sort.h"

parlay::sequence<unsigned long long> generate_random_sequence(size_t n, size_t seed) {
  parlay::random_generator gen(seed);
  parlay::sequence<unsigned long long> arr(n);
  for (size_t i = 0; i < n; ++i) arr[i] = gen();
  return arr;
}

template <typename F>
double time_ms(F&& f) {
  auto start = std::chrono::high_resolution_clock::now();
  f();
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double, std::milli>(end - start).count();
}

struct PairResult { double seq_ms; double par_ms; };

std::vector<PairResult> run_benchmark(size_t n, size_t runs) {
  std::cout << "=== QuickSort Benchmark (" << runs << " arrays, " << parlay::num_workers() << " workers) ===\n";
  std::cout << "    Array size: " << n << " elements\n";

  std::vector<PairResult> results;
  results.reserve(runs);

  for (size_t i = 0; i < runs; ++i) {
    std::cout << "\n--- Array " << (i + 1) << "/" << runs << " ---\n";

    auto base = generate_random_sequence(n, 42 + i);

    auto seq_copy = base;
    double seq_ms = time_ms([&]() { quick_sort_seq(seq_copy); });
    if (!std::ranges::is_sorted(seq_copy)) {
      throw std::runtime_error("Sequential produced unsorted result");
    }
    std::cout << "  sequential: " << std::fixed << std::setprecision(2)
              << seq_ms << " ms\n";

    auto par_copy = base;
    double par_ms = time_ms([&]() { quick_sort_par(par_copy); });
    if (!std::ranges::is_sorted(par_copy)) {
      throw std::runtime_error("Parallel produced unsorted result");
    }
    std::cout << "  parallel:   " << std::fixed << std::setprecision(2)
              << par_ms << " ms\n";

    results.push_back({seq_ms, par_ms});
  }

  return results;
}

void tests()
{
  parlay::sequence<unsigned long long> result = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  {
    parlay::sequence<unsigned long long> in = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    quick_sort_seq(in);

    if (in != result)
    {
      throw std::runtime_error("Test seq sort of sorted array failed");
    }
  }

  {
    parlay::sequence<unsigned long long> in = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    quick_sort_par(in);

    if (in != result)
    {
      throw std::runtime_error("Test seq sort of sorted array failed");
    }
  }

  {
    parlay::sequence<unsigned long long> in = {1, 0, 2, 3, 4, 5, 7, 6, 8, 9};

    quick_sort_seq(in);

    if (in != result)
    {
      throw std::runtime_error("Test seq sort of small unsorted array failed");
    }
  }

  {
    parlay::sequence<unsigned long long> in = {1, 0, 2, 3, 4, 5, 7, 6, 8, 9};

    quick_sort_par(in);

    if (in != result)
    {
      throw std::runtime_error("Test seq sort of small unsorted array failed");
    }
  }

  {
    parlay::sequence<unsigned long long> in = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

    quick_sort_seq(in);

    if (in != result)
    {
      throw std::runtime_error("Test seq sort of unsorted array failed");
    }
  }

  {
    parlay::sequence<unsigned long long> in = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

    quick_sort_par(in);

    if (in != result)
    {
      throw std::runtime_error("Test seq sort of small unsorted array failed");
    }
  }


  {
    parlay::sequence<unsigned long long> in = {3, 3, 2, 2, 1, 1};
    parlay::sequence<unsigned long long> res = {1, 1, 2, 2, 3, 3};

    quick_sort_seq(in);

    if (in != res)
    {
      throw std::runtime_error("Test seq sort of unsorted array with repeats failed");
    }
  }

  {
    parlay::sequence<unsigned long long> in = {3, 3, 2, 2, 1, 1};
    parlay::sequence<unsigned long long> res = {1, 1, 2, 2, 3, 3};

    quick_sort_par(in);

    if (in != res)
    {
      throw std::runtime_error("Test seq sort of unsorted array with repeats failed");
    }
  }

  std::cout << "All tests passed!\n";

}

int main() {
  size_t n    = 100'000'000;
  size_t runs = 5;

  tests();

  auto results = run_benchmark(n, runs);

  std::vector<double> seq_times, par_times;
  for (auto& r : results) { seq_times.push_back(r.seq_ms); par_times.push_back(r.par_ms); }

  double seq_avg = std::reduce(seq_times.begin(), seq_times.end()) / seq_times.size();
  double par_avg = std::reduce(par_times.begin(), par_times.end()) / par_times.size();

  std::cout << "\n=== Summary ===\n";
  std::cout << std::fixed << std::setprecision(2);
  std::cout << "Sequential avg: " << seq_avg << " ms\n";
  std::cout << "Parallel   avg: " << par_avg << " ms\n";
  if (par_avg > 0.0) {
    std::cout << "Speedup:        " << (seq_avg / par_avg) << "x\n";
  }
  return 0;
}