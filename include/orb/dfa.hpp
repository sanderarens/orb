#ifndef ORB_DFA_H
#define ORB_DFA_H

#include <algorithm>
#include <cassert>
#include <queue>
#include <stack>
#include <vector>

#include <orb/alphabet.hpp>

namespace orb {

inline constexpr std::size_t invalid = -1UZ;

// https://en.wikipedia.org/wiki/Deterministic_finite_automaton
// We define the starting state as 0.
template <alphabet A>
class dfa
{
  std::vector<std::size_t> transitions_;
  std::vector<bool> accepting_;

public:
  A alphabet;

  constexpr dfa(A abc, std::vector<std::size_t> transitions, std::vector<bool> accepting)
    : transitions_{ std::move(transitions) }, accepting_{ std::move(accepting) }, alphabet{ std::move(abc) }
  {
    assert(transitions_.size() == accepting_.size() * alphabet.size());
  }

  // Special case with only accepting states
  constexpr dfa(A abc, std::vector<std::size_t> transitions)
    : transitions_{ std::move(transitions) }, accepting_(transitions_.size() / abc.size(), true), alphabet{ std::move(abc) }
  {
    assert(transitions_.size() == accepting_.size() * alphabet.size());
  }

  [[nodiscard]] constexpr std::size_t operator[](std::size_t state_index, std::size_t symbol_index) const
  {
    return transitions_[state_index * alphabet.size() + symbol_index];
  }

  [[nodiscard]] constexpr std::size_t operator[](std::size_t state_index, A::symbol_type symbol) const {
    return (*this)[state_index, static_cast<std::size_t>(alphabet.indices[symbol])];
  }

  [[nodiscard]] constexpr std::span<const std::size_t> operator[](std::size_t state_index) const {
    return {transitions_.cbegin() + static_cast<std::vector<std::size_t>::difference_type>(state_index * alphabet.size()), alphabet.size()};
  }

  [[nodiscard]] constexpr bool accepts(std::size_t state_index) const
  {
    return state_index != invalid && accepting_[state_index];
  }

  [[nodiscard]] constexpr bool accepts(std::span<const typename A::symbol_type> word) const
  {
    std::size_t state_index = 0;
    for (typename A::symbol_type symbol : word) {
      state_index = (*this)[state_index, symbol];
      if (!accepts(state_index)) { return false; }
    }
    return true;
  }

  [[nodiscard]] constexpr const std::vector<bool>& accepting() const
  {
    return accepting_;
  }

  [[nodiscard]] constexpr std::size_t size() const noexcept
  {
    return accepting_.size();
  }
};

// Keeps only the states that are in the mask. The states in the new dfa will have other indices if the new dfa is smaller than the original.
template <alphabet A>
[[nodiscard]] dfa<A> filter(const dfa<A>& dfa, const std::vector<bool>& mask)
{
  const std::size_t compressed_size = static_cast<std::size_t>(std::ranges::count(mask, true));
  if (compressed_size == mask.size()) { return dfa; }
  std::vector<std::size_t> transitions;
  std::vector<bool> accepting;

  transitions.reserve(compressed_size * dfa.alphabet.size());
  accepting.reserve(compressed_size);
  for (std::size_t state_index = 0; state_index != dfa.size(); ++state_index) {
    if (mask[state_index]) {
      for (const std::size_t next_state_index : dfa[state_index]) {
        transitions.push_back(next_state_index == invalid || !mask[next_state_index] ? invalid : next_state_index);
      }
      accepting.push_back(dfa.accepts(state_index));
    }
  }

  return { dfa.alphabet, std::move(transitions), std::move(accepting) };
}

// Preorder depth-first traversal up to a given word size. Also known as backtrack search.
// The resulting words are lexicographically ordered.
// This has the advantage of storing only 1 word at any time.
template <alphabet A>
class dfs_view
{
  const dfa<A>* dfa_{};
  std::stack<std::size_t> stack_{{{}}};
  std::vector<typename A::symbol_type> word_{};
  std::size_t max_size_{};

  constexpr void advance()
  {
    if (word_.size() == max_size_) { stack_.pop(); }

    while (!stack_.empty()) {
      std::size_t symbol_index = 0;
      if (stack_.size() == word_.size()) {
        symbol_index = static_cast<std::size_t>(dfa_->alphabet.indices[word_.back()]) + 1;
        word_.pop_back();
      }

      const auto transitions = (*dfa_)[stack_.top()];
      auto first = transitions.cbegin();
      auto last = transitions.cend();
      const auto it = std::ranges::find_if(first + static_cast<decltype(transitions)::difference_type>(symbol_index), last, [dfa=dfa_](std::size_t state_index){ return dfa->accepts(state_index); });
      if (it != last) {
        stack_.push(*it);
        word_.push_back(dfa_->alphabet.symbols[static_cast<A::index_type>(it - first)]);
        return;
      }

      stack_.pop();
    }
  }

public:
  constexpr explicit dfs_view(const dfa<A> &dfa, std::size_t max_size = -1UZ) : dfa_{ &dfa }, max_size_{ max_size } {}

  class iterator
  {
    friend class dfs_view;
    dfs_view* dfs_view_;

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::vector<typename A::symbol_type>;

    constexpr explicit iterator(dfs_view& dfa) : dfs_view_{ &dfa } {}

    constexpr const value_type& operator*() const noexcept
    {
      return dfs_view_->word_;
    }

    constexpr iterator& operator++()
    {
      dfs_view_->advance();
      return *this;
    }

    constexpr iterator operator++(int) const
    {
      iterator temp(*this);
      ++*this;
      return temp;
    }
  };

  struct sentinel
  {
  };

  constexpr friend bool operator==(iterator iter, sentinel)
  {
    return iter.dfs_view_->stack_.empty();
  }

  [[nodiscard]] constexpr iterator begin() { return iterator{*this}; }
  [[nodiscard]] constexpr static sentinel end() noexcept { return {}; }
};

// Breadth-first traversal up to a given word size.
// The resulting words are shortlex ordered.
// This can take up a lot of memory if searching long words are allowed (size(alphabet)^max_size).
template <alphabet A>
class bfs_view
{
  const dfa<A>* dfa_{};
  struct state { std::size_t state_index{}; std::vector<typename A::symbol_type> word{}; };
  std::queue<state> queue_{{{}}};
  std::size_t max_size_{};

  constexpr void advance()
  {
    const auto [state_index, word] = queue_.front();
    queue_.pop();
    if (word.size() == max_size_) { return; }
    for (const auto [symbol_index, next_state_index] : (*dfa_)[state_index] | std::views::enumerate) {
      if (dfa_->accepts(next_state_index)) {
        std::vector next_word = word;
        next_word.push_back(dfa_->alphabet.symbols[static_cast<A::index_type>(symbol_index)]);
        queue_.emplace(next_state_index, std::move(next_word));
      }
    }
  }

public:
  constexpr explicit bfs_view(const dfa<A> &dfa, std::size_t max_size = -1UZ) : dfa_{ &dfa }, max_size_{ max_size } {}

  class iterator
  {
    friend class bfs_view;
    bfs_view* bfs_view_;

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::vector<typename A::symbol_type>;

    constexpr explicit iterator(bfs_view& dfa) : bfs_view_{ &dfa } {}

    constexpr const value_type& operator*() const noexcept
    {
      return bfs_view_->queue_.front().word;
    }

    constexpr iterator& operator++()
    {
      bfs_view_->advance();
      return *this;
    }

    constexpr iterator operator++(int) const
    {
      iterator temp(*this);
      ++*this;
      return temp;
    }
  };

  struct sentinel
  {
  };

  constexpr friend bool operator==(iterator iter, sentinel)
  {
    return iter.bfs_view_->queue_.empty();
  }

  [[nodiscard]] constexpr iterator begin() { return iterator{*this}; }
  [[nodiscard]] constexpr static sentinel end() noexcept { return {}; }
};

}// namespace orb

#endif// ORB_DFA_H