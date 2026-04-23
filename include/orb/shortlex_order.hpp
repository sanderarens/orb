#ifndef ORB_SHORTLEX_H
#define ORB_SHORTLEX_H

#include <algorithm>
#include <ranges>

namespace orb {

template <typename Comp = std::ranges::less>
struct shortlex_order
{
  [[no_unique_address]] Comp comp{};

  template <std::ranges::input_range R1, std::ranges::input_range R2>
  [[nodiscard]] constexpr bool operator()(R1&& lhs, R2&& rhs) const
  {
    if (const std::size_t lhs_size = std::ranges::size(lhs), rhs_size = std::ranges::size(rhs); lhs_size != rhs_size) { return lhs_size < rhs_size; }
    return std::ranges::lexicographical_compare(std::forward<R1>(lhs), std::forward<R2>(rhs), comp);
  }
};
}// namespace orb

#endif// ORB_SHORTLEX_H