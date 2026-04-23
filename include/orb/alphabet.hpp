#ifndef ORB_ALPHABET_H
#define ORB_ALPHABET_H

#include <ranges>
#include <unordered_map>
#include <vector>

namespace orb {

template <typename A>
concept alphabet = requires(A alphabet, typename A::symbol_type symbol, typename A::index_type index)
{
  requires std::ranges::random_access_range<decltype(alphabet.symbols)>;
  { alphabet.symbols[index] } -> std::convertible_to<typename A::symbol_type>;
  { alphabet.indices[symbol] } -> std::convertible_to<typename A::index_type>;
  { alphabet.size() } -> std::unsigned_integral;
};

template <std::integral S>
class arithmetic_sequence
{
  class indices
  {
    S first_;
  public:
    constexpr explicit indices(S first) : first_{ first } {}
    [[nodiscard]] constexpr auto operator[](S symbol) const { return symbol - first_; }
  };

public:
  std::ranges::iota_view<S, S> symbols;
  indices indices;
  using symbol_type = S;
  using index_type = std::ranges::range_difference_t<decltype(symbols)>;

  constexpr arithmetic_sequence(S value, S bound) : symbols{ value, bound }, indices{ value } {}
  [[nodiscard]] constexpr auto size() const { return symbols.size(); }
};

template <typename S>
class general_alphabet : std::ranges::view_interface<general_alphabet<S>>
{
public:
  using symbol_type = S;
  using index_type = std::vector<S>::difference_type;

private:
  std::vector<S> symbols_;

  class indices
  {
    std::unordered_map<S, index_type> indices_;

    public:
    constexpr explicit indices(const std::vector<S> &symbols)
    {
      indices_.reserve(symbols.size());
      for (const auto [symbol_index, symbol] : symbols | std::views::enumerate) { indices_[symbol] = symbol_index; }
    }

    [[nodiscard]] constexpr auto operator[](S symbol) const { return indices_.at(symbol); }
  };

public:
  std::span<const S> symbols;
  indices indices;

  explicit constexpr general_alphabet(std::vector<S> alphabet) : symbols_{ std::move(alphabet) }, symbols{ symbols_ }, indices{ symbols_ }
  {
  }

  [[nodiscard]] constexpr auto size() const { return symbols.size(); }
};

}// namespace orb

#endif// ORB_ALPHABET_H