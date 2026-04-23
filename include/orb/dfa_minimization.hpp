#ifndef ORB_DFA_MINIMIZATION_H
#define ORB_DFA_MINIMIZATION_H

#include "dfa.hpp"
#include "dfa_trim.hpp"

namespace orb {

template <alphabet A>
dfa<A> minimize(const dfa<A>& dfa)
{
  // Step 1: remove dead and unreachable states
  const auto trim_dfa = trim(dfa);

  // Step 2: merge nondistinguishable states,
  // TODO: implement Hopcroft minimization algorithm
  return trim_dfa;
}

}// namespace orb

#endif// ORB_DFA_MINIMIZATION_H