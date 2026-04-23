#ifndef ORB_COXETER_H
#define ORB_COXETER_H

#include <string>

namespace orb {

namespace coxeter {

  using group_element = std::string;

  struct multiplication
  {
    [[nodiscard]] static constexpr group_element operator()(const group_element &lhs, const group_element &rhs)
    {
      return group_element{ lhs + rhs };// TODO: bring to shortlex form
    }
  };

  struct invert
  {
    [[nodiscard]] static constexpr group_element operator()(const group_element &w)
    {
      return { w.crbegin(), w.crend() };
    }
  };

  [[nodiscard]] constexpr group_element identity_element(const multiplication & /*unused*/) { return {}; }

  [[nodiscard]] constexpr invert inverse_operation(const multiplication & /*unused*/) { return {}; }
}// namespace coxeter
}// namespace orb

#endif// ORB_COXETER_H