#include <orb/algebra.hpp>
#include "orb/power.hpp"

#include <functional>

static_assert(orb::monoid_operation<std::plus<>, int>);

struct trivial { friend auto operator<=>(const trivial&, const trivial&) = default; };

constexpr trivial trivial_op(const trivial& /*unused*/, const trivial& /*unused*/) noexcept { return trivial{}; }
using trivial_op_type = decltype(trivial_op);
constexpr trivial identity_element(trivial_op_type /*unused*/) noexcept { return trivial{}; }
constexpr auto inverse_operation(trivial_op_type /*unused*/) noexcept { return [](const trivial&) { return trivial{}; }; }

static_assert(orb::inverse_operation(trivial_op)(trivial{}) == orb::identity_element(trivial_op));
static_assert(orb::commutative_group_operation<trivial_op_type, trivial>);

static_assert(orb::commutative_group_operation<std::plus<>, int>);
static_assert(orb::power(2, 0, std::plus{}) == 0);
static_assert(orb::power(2, 3, std::plus{}) == 2 + 2 + 2);
static_assert(orb::power(2, 4, std::multiplies{}) == 2 * 2 * 2 * 2);