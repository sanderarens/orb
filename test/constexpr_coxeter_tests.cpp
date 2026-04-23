#include <catch2/catch_test_macros.hpp>

#include <orb/algebra.hpp>
#include <orb/coxeter.hpp>

TEST_CASE("Coxeter group operations are computed with constexpr", "[coxeter]")
{
  STATIC_REQUIRE(orb::group_operation<orb::coxeter::multiplication, orb::coxeter::group_element>);
  STATIC_REQUIRE(orb::coxeter::multiplication{}(orb::coxeter::group_element{"abc"}, orb::coxeter::group_element{"def"}) == orb::coxeter::group_element{"abcdef"});
  STATIC_REQUIRE(orb::inverse_operation(orb::coxeter::multiplication{})(orb::coxeter::group_element{"abc"}) == orb::coxeter::group_element{"cba"});
}