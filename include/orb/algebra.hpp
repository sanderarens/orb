#ifndef ALGEBRA_HPP
#define ALGEBRA_HPP

#include <concepts>
#include <functional>
#include <orb/orb_export.hpp>

namespace orb {

template<typename Op, typename T>
concept transformation = std::regular<T> && std::is_invocable_r_v<T, Op, T>;

template<typename Op, typename T>
concept binary_operation = std::regular<T> && std::is_invocable_r_v<T, Op, T, T>;

template<typename Op, typename T>
concept associative_binary_operation = binary_operation<Op, T>;

template<typename Op, typename T>
concept commutative_binary_operation = binary_operation<Op, T>;

namespace cpo {
  template<typename Op>
  concept adl_identity_element = requires(Op op) { identity_element(op); };

  struct identity_element_fn
  {
    template<int identity_element>
    struct convert
    {
      template<std::integral T>
      constexpr operator T() const { return T{identity_element}; }
    };

    static constexpr convert<0> operator()(const std::plus<>& /*unused*/) { return convert<0>{}; }

    template<std::integral T>
    static constexpr T operator()(const std::plus<T>& /*unused*/) { return T{0}; }

    static constexpr convert<1> operator()(const std::multiplies<>& /*unused*/) { return convert<1>{}; }

    template<std::integral T>
    static constexpr T operator()(const std::multiplies<T>& /*unused*/) { return T{1}; }

    template<typename Op>
    static constexpr auto operator()(const Op& op) requires adl_identity_element<Op>
    {
      return identity_element(op);
    }
  };

  template<typename Op>
  concept adl_inverse_operation = requires(Op op) { inverse_operation(op); };

  struct inverse_operation_fn
  {
    template<typename T>
    static constexpr auto operator()(const std::plus<T>& /*unused*/) { return std::negate<T>{}; }

    static constexpr auto operator()(const std::plus<>& /*unused*/) { return std::negate{}; }

    struct reciprocal
    {
      template<std::integral T>
      static constexpr T operator()(const T& x) { return T{1} / x; } // TODO: I don't think this is correct yet for std::multiplies<> in general
    };

    template<typename T>
    constexpr auto operator()(const std::multiplies<T>& /*unused*/) const { return reciprocal{}; }

    constexpr auto operator()(const std::multiplies<>& /*unused*/) const { return reciprocal{}; }

    template<typename Op>
    constexpr auto operator()(const Op& op) const requires adl_inverse_operation<Op>
    {
      return inverse_operation(op);
    }
  };
}// namespace cpo

inline constexpr cpo::identity_element_fn identity_element;
inline constexpr cpo::inverse_operation_fn inverse_operation;

template<typename Op, typename T>
concept monoid_operation = associative_binary_operation<Op, T> && requires(Op op)
{
  { orb::identity_element(op) } -> std::convertible_to<T>;
};

template<typename Op, typename T>
concept commutative_monoid_operation = monoid_operation<Op, T> && commutative_binary_operation<Op, T>;

template<typename Op, typename T>
concept group_operation = monoid_operation<Op, T> && requires(Op op)
{
  { orb::inverse_operation(op) } -> transformation<T>;
};

template<typename Op, typename T>
concept commutative_group_operation = group_operation<Op, T> && commutative_binary_operation<Op, T>;

}// namespace orb

#endif
