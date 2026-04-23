#ifndef POWER_HPP
#define POWER_HPP

#include <orb/algebra.hpp>

#include <concepts>

namespace orb {

template<typename T, std::integral N, associative_binary_operation<T> Op>
[[nodiscard]] constexpr T power_accumulate_positive(T r, T a, N n, const Op &op)
{
  while (true) {
    if (n % N{2} != N{0}) {
      r = op(r, a);
      if (n == T{1}) { return r; }
    }
    a = op(a, a);
    n /= N{2};
  }
}

template<typename T, std::integral N, associative_binary_operation<T> Op>
[[nodiscard]] constexpr T positive_power(T a, N n, const Op& op)
{
  while (n % N{2} == N{0}) {
    n /= N{2};
    a = op(a, a);
  }
  n /= N{2};
  if (n == N{0}) { return a; }
  return power_accumulate_positive(a, op(a, a), n, op);
}

template<typename T, std::integral N, associative_binary_operation<T> Op>
[[nodiscard]] constexpr T power(T a, N n, const Op& op)
{
  return power_fast(a, n, op);
}

template<typename T, std::integral N, monoid_operation<T> Op>
[[nodiscard]] constexpr T power(T a, N n, const Op& op)
{
  if (n == N{0}) { return identity_element(op); }
  return power_fast(a, n, op);
}

template<typename T, std::signed_integral N, group_operation<T> Op>
[[nodiscard]] constexpr T power(T a, N n, const Op& op)
{
  if (n == N{0}) { return identity_element(op); }
  a = positive_power(a, n, op);
  if (n < N{0}) { a = inverse_operation(op)(a); }
  return a;
}
}// namespace orb

#endif