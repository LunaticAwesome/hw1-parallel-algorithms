#pragma once
#include <cstddef>
#include <utility>
#include <parlay/random.h>
#include <parlay/parallel.h>
#include <parlay/sequence.h>

template<bool PARALLEL, size_t PAR_THRESHOLD, typename T>
void quick_sort_impl(T* arr, size_t lo, size_t hi, parlay::random_generator gen)
{
  if (hi - lo <= 1)
  {
    return;
  }

  const size_t n   = hi - lo;
  size_t ind       = gen() % n;
  size_t end       = hi - 1;
  std::swap(arr[lo], arr[lo + ind]);

  size_t cur = lo;
  while (cur < end)
  {
    if (arr[cur] > arr[cur + 1])
    {
      std::swap(arr[cur], arr[cur + 1]);
      ++cur;
    }
    else
    {
      std::swap(arr[cur + 1], arr[end]);
      --end;
    }
  }

  const size_t left_lo  = lo;
  const size_t left_hi  = cur;
  const size_t right_lo = cur + 1;
  const size_t right_hi = hi;

  if constexpr (PARALLEL)
  {
    if (n > PAR_THRESHOLD)
    {
      parlay::par_do(
        [&]() { quick_sort_impl<PARALLEL, PAR_THRESHOLD, T>(arr, left_lo,  left_hi,  gen[left_lo]);  },
        [&]() { quick_sort_impl<PARALLEL, PAR_THRESHOLD, T>(arr, right_lo, right_hi, gen[right_hi]); }
      );
    }
    else
    {
      quick_sort_impl<PARALLEL, PAR_THRESHOLD, T>(arr, left_lo,  left_hi,  gen);
      quick_sort_impl<PARALLEL, PAR_THRESHOLD, T>(arr, right_lo, right_hi, gen);
    }
  }
  else
  {
    quick_sort_impl<PARALLEL, PAR_THRESHOLD, T>(arr, left_lo,  left_hi,  gen);
    quick_sort_impl<PARALLEL, PAR_THRESHOLD, T>(arr, right_lo, right_hi, gen);
  }
}

template<typename T>
void quick_sort_seq(parlay::sequence<T>& arr)
{
  auto gen = parlay::random_generator();
  quick_sort_impl<false, 100'000, T>(arr.data(), 0, arr.size(), gen);
}

template<typename T>
void quick_sort_par(parlay::sequence<T>& arr)
{
  auto gen = parlay::random_generator();
  quick_sort_impl<true, 100'000, T>(arr.data(), 0, arr.size(), gen);
}