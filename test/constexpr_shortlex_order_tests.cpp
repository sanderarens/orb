#include <catch2/catch_test_macros.hpp>

#include <string_view>

#include <orb/shortlex_order.hpp>

TEST_CASE("Shortlex comparison are computed with constexpr", "[shortlex]")
{
  using namespace std::literals;
  constexpr orb::shortlex_order shortlex;
  STATIC_REQUIRE_FALSE(shortlex("abc"sv, "abc"sv));
  STATIC_REQUIRE(shortlex("abc"sv, "abd"sv));
  STATIC_REQUIRE(shortlex("abd"sv, "abcd"sv));
}