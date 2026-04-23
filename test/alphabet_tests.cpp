#include <catch2/catch_test_macros.hpp>

#include <vector>

#include <orb/alphabet.hpp>

TEST_CASE("General alphabet constexpr tests", "[alphabet]")
{
  const orb::general_alphabet abc{std::vector{'a', 'b', 'c'}};
  REQUIRE(orb::alphabet<decltype(abc)>);
  REQUIRE(abc.symbols[0] == 'a');
  REQUIRE(abc.symbols[1] == 'b');
  REQUIRE(abc.symbols[2] == 'c');
  REQUIRE(abc.indices['a'] == 0);
  REQUIRE(abc.indices['b'] == 1);
  REQUIRE(abc.indices['c'] == 2);

  const orb::general_alphabet onetwothree{std::vector{1, 2, 3}};
  REQUIRE(orb::alphabet<decltype(onetwothree)>);
  REQUIRE(onetwothree.symbols[0] == 1);
  REQUIRE(onetwothree.symbols[1] == 2);
  REQUIRE(onetwothree.symbols[2] == 3);
  REQUIRE(onetwothree.indices[1] == 0);
  REQUIRE(onetwothree.indices[2] == 1);
  REQUIRE(onetwothree.indices[3] == 2);
}
