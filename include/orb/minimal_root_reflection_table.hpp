#ifndef ORB_MINIMAL_ROOT_REFLECTION_TABLE_H
#define ORB_MINIMAL_ROOT_REFLECTION_TABLE_H

#include "dfa.hpp"


#include <algorithm>
#include <vector>

#include <orb/alphabet.hpp>

namespace orb {

enum class root_type : std::int8_t { simple, minimal, positive, negative };

// TODO: construction of the table from the coxeter graph/matrix, see Casselman's paper
template <alphabet A>
class minimal_root_reflection_table
{
  std::vector<std::size_t> transitions_;
  std::size_t minimal_roots_size_;

public:
  A alphabet;

  // transitions is of size #minimal_roots * #alphabet. All indices < #minimal_roots are of type minimal roots, an index == #minimal_roots is of positive type and all others are of type negative.
  constexpr minimal_root_reflection_table(A abc, std::vector<std::size_t> transitions)
    : transitions_{ std::move(transitions) }, minimal_roots_size_{ transitions_.size() / abc.size() }, alphabet{ std::move(abc) }
  {
    assert(transitions_.size() * minimal_roots_size_ * alphabet.size());
  }

  [[nodiscard]] constexpr std::size_t operator[](std::size_t state_index, A::index_type symbol_index) const {
    return transitions_[state_index * alphabet.size() + static_cast<std::size_t>(symbol_index)];
  }

  [[nodiscard]] constexpr std::size_t operator[](std::size_t state_index, A::symbol_type symbol) const {
    return (*this)[state_index, alphabet.indices[symbol]];
  }

  [[nodiscard]] constexpr root_type type(std::size_t state_index) const
  {
    if (state_index < alphabet.size()) { return root_type::simple; }
    if (state_index < minimal_roots_size_) { return root_type::minimal; }
    if (state_index == minimal_roots_size_) { return root_type::positive; }
    return root_type::negative;
  }

  [[nodiscard]] constexpr bool is_minimal_root(std::size_t state_index) const
  {
    return state_index < minimal_roots_size_;
  }

  [[nodiscard]] constexpr std::size_t minimal_roots_size() const noexcept
  {
    return minimal_roots_size_;
  }
};

template<alphabet A>
class shortlex
{
  minimal_root_reflection_table<A> table_;

  template <std::forward_iterator I>
  struct find_exchange_site_result
  {
    I exchange;
    std::optional<typename A::symbol_type> symbol;
  };

  template <std::forward_iterator I>
  constexpr find_exchange_site_result<I> find_exchange_site(I first, I last, A::symbol_type s) const
  {
    I exchange = first;
    auto root = static_cast<std::size_t>(table_.alphabet.indices[s]);

    for (; first != last; ++first) {
      auto s_i = *first;
      root = table_[root, s_i];
      switch (table_.type(root)) {
      case root_type::simple:
        if (root < static_cast<std::size_t>(table_.alphabet.indices[s_i])) {
          s = table_.alphabet.symbols[static_cast<A::index_type>(root)];
          exchange = first + 1;
        }
        break;
      case root_type::minimal:
        break;
      case root_type::positive:
        return {exchange, s};
      case root_type::negative:
        return {first, {}};
      }
    }
    return {exchange, s};
  }

public:
  constexpr explicit shortlex(minimal_root_reflection_table<A> table) : table_{ std::move(table) } {}

  [[nodiscard]] constexpr std::vector<typename A::symbol_type> operator()(std::vector<typename A::symbol_type> w, A::symbol_type s) const
  {
    const auto [rexchange, symbol] = find_exchange_site(w.crbegin(), w.crend(), s);
    const auto exchange = rexchange.base();
    symbol ? w.insert(exchange, *symbol) : w.erase(exchange - 1);
    return w;
  }

  [[nodiscard]] constexpr std::vector<typename A::symbol_type> operator()(std::vector<typename A::symbol_type> lhs, const std::vector<typename A::symbol_type>& rhs) const
  {
    for (auto s : rhs) { lhs = (*this)(std::move(lhs), s); }
    return lhs;
  }

  [[nodiscard]] constexpr std::vector<typename A::symbol_type> operator()(std::vector<typename A::symbol_type> w) const
  {
    // Equivalent to
    // return (*this)({}, w);
    // But does everything in place, so we don't need to (re)allocate

    auto it = w.begin();
    while (it != w.end()) {
      const auto [rexchange, symbol] = find_exchange_site(std::reverse_iterator{ it }, w.rend(), *it);
      const auto exchange = rexchange.base();
      if (symbol) {
        std::move_backward(exchange, it, ++it);
        *exchange = *symbol;
      } else {
        it = std::move(exchange, it, exchange - 1);
        it = w.erase(it, it + 2);
      }
    }

    return w;
  }
};

// TODO: put everything shortlex related in orb::shortlex namespace?
// On https://personal.math.ubc.ca/~cass/research/pdf/banff.pdf page 14.
// it is mentioned that there's a slight modification of the subsets of the minimal roots that represent the state of the dfa with fewer states
// by adding linear combinations of the simple roots up to s and a modified union operation.
// It's mentioned that these linear combinations are in the set of positive roots, but these are not finite for non-finite coxeter group,
// should these be minimal roots instead?
// It's also not yet clear how we can take linear combinations from the minimal roots table,
// we should have the components of these minimal roots w.r.s. to the simple roots.
template<alphabet A>
[[nodiscard]] constexpr dfa<A> make_shortlex_dfa(const minimal_root_reflection_table<A>& table)
{
  const A& alphabet = table.alphabet;

  std::vector transitions(alphabet.size(), invalid);

  struct state
  {
    std::size_t state_index{};
    std::size_t symbol_index{};
    std::vector<bool> minimal_roots_subset;
  };
  std::stack<state> stack;

  std::unordered_map<std::vector<bool>, std::size_t> enumeration;
  {
    const auto& [state_index, symbol_index, minimal_roots_subset] = stack.emplace(0, 0, std::vector<bool>(table.minimal_roots_size()));
    enumeration[minimal_roots_subset] = 0;
  }

  std::vector<std::vector<bool>> minimal_roots_subset_precalculated(alphabet.size(), std::vector<bool>(table.minimal_roots_size()));
  for (std::size_t s = 0; s != table.alphabet.size(); ++s) {
    minimal_roots_subset_precalculated[s][s] = true;
    for (std::size_t minimal_roots_index = 0; minimal_roots_index != s; ++minimal_roots_index) {
      minimal_roots_subset_precalculated[s][table[minimal_roots_index, static_cast<A::index_type>(s)]] = true;
    }
  }

  while (!stack.empty()) {
    auto& [state_index, symbol_index, minimal_roots_subset] = stack.top();

    for (; symbol_index != alphabet.size(); ++symbol_index) {
      if (minimal_roots_subset[symbol_index]) { continue; }

      std::vector<bool> next_minimal_roots_subset = minimal_roots_subset_precalculated[symbol_index];
      for (std::size_t minimal_roots_index = symbol_index; minimal_roots_index != table.minimal_roots_size(); ++minimal_roots_index) {
        if (minimal_roots_subset[minimal_roots_index] && table.is_minimal_root(table[minimal_roots_index, static_cast<A::index_type>(symbol_index)])) {
          next_minimal_roots_subset[table[minimal_roots_index, static_cast<A::index_type>(symbol_index)]] = true;
        }
      }

      if (const auto it = enumeration.find(next_minimal_roots_subset); it != enumeration.end()) {
        transitions[state_index * alphabet.size() + symbol_index] = it->second;
      } else {
        const std::size_t next_state_index = enumeration.size();
        stack.emplace(next_state_index, 0, next_minimal_roots_subset);
        enumeration.emplace(std::move(next_minimal_roots_subset), next_state_index);
        transitions.resize(enumeration.size() * alphabet.size(), invalid);
        transitions[state_index * alphabet.size() + symbol_index] = next_state_index;
        break;
      }
    }

    if (symbol_index == alphabet.size()) { stack.pop(); }
  }

  return { alphabet, std::move(transitions) };
}

}// namespace orb

#endif// ORB_MINIMAL_ROOT_REFLECTION_TABLE_H