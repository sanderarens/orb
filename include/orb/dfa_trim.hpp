#ifndef ORB_DFA_TRIM_H
#define ORB_DFA_TRIM_H

#include <algorithm>
#include <cassert>
#include <stack>
#include <vector>

#include <orb/dfa.hpp>
#include <orb/alphabet.hpp>

namespace orb {

namespace detail {
  [[nodiscard]] constexpr std::vector<bool> accessibility(const std::vector<std::vector<std::size_t>> &adjacency, std::vector<bool> reached, std::stack<std::span<const std::size_t>> stack)
  {
    while (!stack.empty()) {
      const std::span transitions = stack.top();
      stack.pop();
      if (const auto it = std::ranges::find_if(transitions, [&](std::size_t i) { return !reached[i]; }); it != transitions.cend()) {
        reached[*it] = true;
        stack.emplace(it + 1, transitions.end());
        stack.emplace(adjacency[*it]);
      }
    }

    return reached;
  }

  [[nodiscard]] constexpr std::vector<bool> accessibility(const std::vector<std::vector<std::size_t>> &adjacency, std::vector<bool> reached)
  {
    std::stack<std::span<const std::size_t>> stack;
    for (std::size_t i = reached.size(); i-- != 0;) {
      if (reached[i]) { stack.emplace(adjacency[i]); }
    }
    return accessibility(adjacency, std::move(reached), std::move(stack));
  }

  [[nodiscard]] constexpr std::vector<bool> accessibility(const std::vector<std::vector<std::size_t>> &adjacency, std::size_t start)
  {
    std::vector<bool> reached(adjacency.size());
    reached[start] = true;
    return accessibility(adjacency, std::move(reached), std::stack<std::span<const std::size_t>>{ { adjacency[start] } });
  }
}// namespace detail

template<alphabet A> [[nodiscard]] constexpr dfa<A> trim(const dfa<A> &dfa)
{
  std::vector<std::vector<std::size_t>> adjacency(dfa.size());
  std::vector<std::vector<std::size_t>> coadjacency(dfa.size());
  for (std::size_t state_index = 0; state_index != dfa.size(); ++state_index) {
    for (const std::size_t next_state_index : dfa[state_index]) {
      if (next_state_index != invalid) {
        adjacency[state_index].push_back(next_state_index);
        coadjacency[next_state_index].push_back(state_index);
      }
    }
  }

  const std::vector<bool> accessible = detail::accessibility(adjacency, 0UZ);
  const std::vector<bool> coaccessible = detail::accessibility(coadjacency, dfa.accepting());

  std::vector<bool> trim(dfa.size());
  for (std::size_t i = 0; i != dfa.size(); ++i) { trim[i] = accessible[i] && coaccessible[i]; }

  return filter(dfa, trim);
}
}// namespace orb

#endif// ORB_DFA_TRIM_H