#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <orb/alphabet.hpp>
#include <orb/dfa.hpp>
#include <orb/dfa_minimization.hpp>
#include <orb/dfa_trim.hpp>
#include <orb/shortlex_order.hpp>

namespace {
constexpr std::string to_string(std::span<const char> word) { return { word.begin(), word.end() }; }
}

// https://polytope.miraheze.org/wiki/Rectangular_symmetry
TEST_CASE("Deterministic finite automata for rectangular symmetry", "[dfa_rectangular_symmetry]")
{
  using namespace std::literals;

  const orb::dfa dfa(orb::arithmetic_sequence{ 'a', 'c' }, std::vector{ 1UZ, 2UZ, orb::invalid, 2UZ, orb::invalid, orb::invalid });

  REQUIRE(dfa.accepts(""sv));
  REQUIRE(dfa.accepts("a"sv));
  REQUIRE(dfa.accepts("ab"sv));
  REQUIRE(dfa.accepts("b"sv));
  REQUIRE_FALSE(dfa.accepts("ba"sv));
  REQUIRE_FALSE(dfa.accepts("ba"sv));

  const std::vector lex_language{std::from_range, orb::dfs_view{dfa} | std::views::transform(to_string)};
  REQUIRE(lex_language == std::vector{""s, "a"s, "ab"s, "b"s});
  REQUIRE(std::ranges::is_sorted(lex_language));

  const std::vector shortlex_language{std::from_range, orb::bfs_view{dfa} | std::views::transform(to_string)};

  REQUIRE(shortlex_language == std::vector{""s, "a"s, "b"s, "ab"s});
  REQUIRE(std::ranges::is_sorted(shortlex_language, orb::shortlex_order{}));
}

TEST_CASE("Deterministic finite automata enumeration", "[dfa_enumerate]")
{
  using namespace std::literals;

  const orb::dfa dfa(orb::arithmetic_sequence{'a', 'c'}, std::vector{ 0UZ, 1UZ, 0UZ, 1UZ });

  REQUIRE(dfa.accepts(""sv));
  REQUIRE(dfa.accepts("a"sv));
  REQUIRE(dfa.accepts("aa"sv));
  REQUIRE(dfa.accepts("ab"sv));
  REQUIRE(dfa.accepts("b"sv));
  REQUIRE(dfa.accepts("ba"sv));
  REQUIRE(dfa.accepts("bb"sv));

  const std::vector lex_language{std::from_range, orb::dfs_view{dfa, 2} | std::views::transform(to_string)};

  REQUIRE(lex_language == std::vector{""s, "a"s, "aa"s, "ab"s, "b"s, "ba"s, "bb"s});
  REQUIRE(std::ranges::is_sorted(lex_language));

  const std::vector shortlex_language{std::from_range, orb::bfs_view{dfa, 2} | std::views::transform(to_string)};

  REQUIRE(shortlex_language == std::vector{""s, "a"s, "b"s, "aa"s, "ab"s, "ba"s, "bb"s});
  REQUIRE(std::ranges::is_sorted(shortlex_language, orb::shortlex_order{}));
}

TEST_CASE("Trim dfa", "[dfa_trim]")
{
  using namespace std::literals;

  // Note: this is the same dfa as for rectangular symmetry, but with the invalid transitions replaced with a transition to a dead state and an unreachable state added.
  const orb::dfa dfa(orb::arithmetic_sequence{ 'a', 'c' }, std::vector{ 1UZ, 2UZ, 3UZ, 2UZ, 3UZ, 3UZ, 3UZ, 3UZ, 1UZ, 2UZ }, std::vector { true, true, true, false, true });
  const orb::dfa trim_dfa = trim(dfa);

  REQUIRE(trim_dfa.size() == 3);

  REQUIRE(trim_dfa.accepts(0));
  REQUIRE(trim_dfa.accepts(1));
  REQUIRE(trim_dfa.accepts(2));

  REQUIRE(trim_dfa[0, 'a'] == 1);
  REQUIRE(trim_dfa[0, 'b'] == 2);
  REQUIRE(trim_dfa[1, 'a'] == orb::invalid);
  REQUIRE(trim_dfa[1, 'b'] == 2);
  REQUIRE(trim_dfa[2, 'a'] == orb::invalid);
  REQUIRE(trim_dfa[2, 'b'] == orb::invalid);
}

TEST_CASE("Minimize dfa", "[dfa_minimization]")
{
  using namespace std::literals;

  // Note: this is the same dfa as for rectangular symmetry, but with the invalid transitions replaced with a transition to a dead state and an unreachable state added.
  const orb::dfa dfa(orb::arithmetic_sequence{ 'a', 'c' }, std::vector{ 1UZ, 2UZ, 3UZ, 2UZ, 3UZ, 3UZ, 3UZ, 3UZ, 1UZ, 2UZ }, std::vector { true, true, true, false, true });
  const orb::dfa trim_dfa = minimize(dfa);

  REQUIRE(trim_dfa.size() == 3);

  REQUIRE(trim_dfa.accepts(0));
  REQUIRE(trim_dfa.accepts(1));
  REQUIRE(trim_dfa.accepts(2));

  REQUIRE(trim_dfa[0, 'a'] == 1);
  REQUIRE(trim_dfa[0, 'b'] == 2);
  REQUIRE(trim_dfa[1, 'a'] == orb::invalid);
  REQUIRE(trim_dfa[1, 'b'] == 2);
  REQUIRE(trim_dfa[2, 'a'] == orb::invalid);
  REQUIRE(trim_dfa[2, 'b'] == orb::invalid);
}
