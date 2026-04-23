#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "orb/alphabet.hpp"
#include "orb/dfa.hpp"
#include "orb/minimal_root_reflection_table.hpp"

namespace {
constexpr std::size_t minus = -1UZ;
constexpr std::string_view to_string_view(std::span<const char> word) { return std::string_view{word}; }
constexpr std::vector<char> to_vector(const char* word) { return {std::from_range, std::string_view{word}}; }

template<orb::alphabet A>
void print_shortlex(const orb::dfa<A>& shortlex_dfa, std::size_t max_word_length = -1UZ)
{
  for (const auto [index, word] : orb::bfs_view(shortlex_dfa, max_word_length) | std::views::enumerate) {
    std::println("{}: {}", index, to_string_view(word));
  }
}
}// namespace

// https://personal.math.ubc.ca/~cass/research/pdf/roots.pdf p.7-8
TEST_CASE("Minimal root reflection table", "[minimal_root_reflection_table]")
{
  constexpr std::size_t plus = 7UZ;

  // clang-format off
  constexpr std::array transitions = {
    minus, 4UZ, 6UZ,
    3UZ, minus, 5UZ,
    6UZ, 5UZ, minus,
    1UZ, 3UZ, plus,
    4UZ, 0UZ, plus,
    plus, 2UZ, 1UZ,
    2UZ, plus, 0UZ
  };
  // clang-format on

  const orb::minimal_root_reflection_table minimal_root_reflection_table(orb::arithmetic_sequence{'a', 'd'}, {transitions.cbegin(), transitions.cend()});
  REQUIRE(minimal_root_reflection_table.type(minimal_root_reflection_table[0, 0]) == orb::root_type::negative);
  REQUIRE(minimal_root_reflection_table.type(minimal_root_reflection_table[1, 1]) == orb::root_type::negative);
  REQUIRE(minimal_root_reflection_table.type(minimal_root_reflection_table[2, 2]) == orb::root_type::negative);
  REQUIRE(minimal_root_reflection_table.type(minimal_root_reflection_table[0, 2]) == orb::root_type::minimal);
  REQUIRE(minimal_root_reflection_table.type(minimal_root_reflection_table[3, 2]) == orb::root_type::positive);
  REQUIRE(minimal_root_reflection_table.type(minimal_root_reflection_table[6, 0]) == orb::root_type::simple);
  REQUIRE(minimal_root_reflection_table.type(minimal_root_reflection_table[6, 2]) == orb::root_type::simple);

  const orb::shortlex shortlex{ minimal_root_reflection_table };
  REQUIRE(to_string_view(shortlex({}, 'a')) == "a");
  REQUIRE(shortlex({ 'a' }, 'a').empty());
  REQUIRE(to_string_view(shortlex({ 'b' }, 'a')) == "ba");

  const orb::dfa shortlex_dfa = make_shortlex_dfa(minimal_root_reflection_table);
  REQUIRE(shortlex_dfa.size() == 14);
  constexpr std::size_t max_word_length = 3UZ;
  print_shortlex(shortlex_dfa, max_word_length);
  std::vector<std::string> shortlex_words = orb::bfs_view{ shortlex_dfa, max_word_length } | std::ranges::to<std::vector<std::string>>();
  REQUIRE(shortlex_words.size() == 20);
  REQUIRE(shortlex_words[0].empty());
  REQUIRE(shortlex_words[1] == "a");
  REQUIRE(shortlex_words[2] == "b");
  REQUIRE(shortlex_words[3] == "c");
  REQUIRE(shortlex_words[4] == "ab");
  REQUIRE(shortlex_words[5] == "ac");
  REQUIRE(shortlex_words[6] == "ba");
  REQUIRE(shortlex_words[7] == "bc");
  REQUIRE(shortlex_words[8] == "ca");
  REQUIRE(shortlex_words[9] == "cb");
  REQUIRE(shortlex_words[10] == "aba");
  REQUIRE(shortlex_words[11] == "abc");
  REQUIRE(shortlex_words[12] == "aca");
  REQUIRE(shortlex_words[13] == "acb");
  REQUIRE(shortlex_words[14] == "bab");
  REQUIRE(shortlex_words[15] == "bac");
  REQUIRE(shortlex_words[16] == "bca");
  REQUIRE(shortlex_words[17] == "bcb");
  REQUIRE(shortlex_words[18] == "cab");
  REQUIRE(shortlex_words[19] == "cba");
}

TEST_CASE("Minimal root reflection table A2", "[minimal_root_reflection_table]")
{
  const orb::minimal_root_reflection_table minimal_root_reflection_table(orb::arithmetic_sequence{'a', 'c'}, { minus, 2UZ, 2UZ, minus, 1UZ, 0UZ });
  REQUIRE(minimal_root_reflection_table.type(minimal_root_reflection_table[0, 0]) == orb::root_type::negative);
  REQUIRE(minimal_root_reflection_table.type(minimal_root_reflection_table[1, 1]) == orb::root_type::negative);
  REQUIRE(minimal_root_reflection_table.type(minimal_root_reflection_table[0, 1]) == orb::root_type::minimal);
  REQUIRE(minimal_root_reflection_table.type(minimal_root_reflection_table[1, 0]) == orb::root_type::minimal);
  REQUIRE(minimal_root_reflection_table.type(minimal_root_reflection_table[2, 0]) == orb::root_type::simple);
  REQUIRE(minimal_root_reflection_table.type(minimal_root_reflection_table[2, 1]) == orb::root_type::simple);

  const orb::shortlex shortlex{minimal_root_reflection_table};
  REQUIRE(to_string_view(shortlex({}, 'a')) == "a");
  REQUIRE(to_string_view(shortlex({}, 'b')) == "b");
  REQUIRE(shortlex({ 'a' }, 'a').empty());
  REQUIRE(shortlex({ 'b' }, 'b').empty());
  REQUIRE(to_string_view(shortlex({ 'a' }, 'b')) == "ab");
  REQUIRE(to_string_view(shortlex({ 'b' }, 'a')) == "ba");
  REQUIRE(to_string_view(shortlex(to_vector("ab"), 'a')) == "aba");
  REQUIRE(to_string_view(shortlex(to_vector("ba"), 'b')) == "aba");
  REQUIRE(to_string_view(shortlex(to_vector("aba"), 'b')) == "ba");
  REQUIRE(shortlex(to_vector(""), to_vector("")).empty());
  REQUIRE(to_string_view(shortlex(to_vector("ba"), to_vector("ba"))) == "ab");
  REQUIRE(to_string_view(shortlex(to_vector("ab"), to_vector("ab"))) == "ba");
  REQUIRE(shortlex(to_vector("")).empty());
  REQUIRE(shortlex(to_vector("aa")).empty());
  REQUIRE(shortlex(to_vector("bb")).empty());
  REQUIRE(shortlex(to_vector("ababab")).empty());
  REQUIRE(shortlex(to_vector("bababa")).empty());
  REQUIRE(to_string_view(shortlex(to_vector("babab"))) == "a");
  REQUIRE(to_string_view(shortlex(to_vector("ababa"))) == "b");
  REQUIRE(to_string_view(shortlex(to_vector("baba"))) == "ab");
  REQUIRE(to_string_view(shortlex(to_vector("abab"))) == "ba");
  REQUIRE(to_string_view(shortlex(to_vector("aba"))) == "aba");
  REQUIRE(to_string_view(shortlex(to_vector("bab"))) == "aba");
  REQUIRE(to_string_view(shortlex(to_vector("ab"))) == "ab");
  REQUIRE(to_string_view(shortlex(to_vector("ba"))) == "ba");
  REQUIRE(to_string_view(shortlex(to_vector("a"))) == "a");
  REQUIRE(to_string_view(shortlex(to_vector("b"))) == "b");

  const orb::dfa shortlex_dfa = make_shortlex_dfa(minimal_root_reflection_table);
  REQUIRE(shortlex_dfa.size() == 4);
  print_shortlex(shortlex_dfa);
  std::vector<std::string> shortlex_words = orb::bfs_view{ shortlex_dfa } | std::ranges::to<std::vector<std::string>>();
  REQUIRE(shortlex_words.size() == 6);
  REQUIRE(shortlex_words[0].empty());
  REQUIRE(shortlex_words[1] == "a");
  REQUIRE(shortlex_words[2] == "b");
  REQUIRE(shortlex_words[3] == "ab");
  REQUIRE(shortlex_words[4] == "ba");
  REQUIRE(shortlex_words[5] == "aba");
}