#include <catch2/catch_test_macros.hpp>

#include <orb/alphabet.hpp>

TEST_CASE("Arithmetic sequence constexpr tests", "[alphabet]")
{
  constexpr orb::arithmetic_sequence abc{'a', 'd'};
  STATIC_REQUIRE(orb::alphabet<decltype(abc)>);
  STATIC_REQUIRE(abc.symbols[0] == 'a');
  STATIC_REQUIRE(abc.symbols[1] == 'b');
  STATIC_REQUIRE(abc.symbols[2] == 'c');
  STATIC_REQUIRE(abc.indices['a'] == 0);
  STATIC_REQUIRE(abc.indices['b'] == 1);
  STATIC_REQUIRE(abc.indices['c'] == 2);

  constexpr orb::arithmetic_sequence onetwothree{1, 4};
  STATIC_REQUIRE(orb::alphabet<decltype(onetwothree)>);
  STATIC_REQUIRE(onetwothree.symbols[0] == 1);
  STATIC_REQUIRE(onetwothree.symbols[1] == 2);
  STATIC_REQUIRE(onetwothree.symbols[2] == 3);
  STATIC_REQUIRE(onetwothree.indices[1] == 0);
  STATIC_REQUIRE(onetwothree.indices[2] == 1);
  STATIC_REQUIRE(onetwothree.indices[3] == 2);
}