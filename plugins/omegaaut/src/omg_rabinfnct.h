/** @file omg_rabinfnct.h

Operations regarding omega languages accepted by Rabin automata

*/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010, 2025 Thomas Moor

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */


#ifndef FAUDES_OMG_RABINFNCT_H
#define FAUDES_OMG_RABINFNCT_H

#include "corefaudes.h"
#include "omg_rabinaut.h"

namespace faudes {


/**
 * Live states  w.r.t a Rabin pair.
 *
 * A state is considered live if it allows for a future path that such
 * that the acceptance condition is met,
 *
 * The implementation is along the following line
 * - initialise LSet the ISet
 * - run a nu iteration on LSet to figure the largest existential invariant
 * - run a mu iteration on LSet*RSet to restrict LSet to states which can reach RSet
 * - repeat the last to steps until a fix point is attained
 * - run one more mu iteration on LSet to extend to the bachward reach
 *
 * @param rTransRel
 *   Trasition systej to operate on
 * @param rRevTransRel
 *   Reverse sorted variant
 * @param rRPair
 *   Rabin pair to consider
 * @param rInv
 *   Resulting set of live states
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void RabinLiveStates(
  const TransSet& rTransRel,
  const TransSetX2EvX1& rRevTransRel,
  const RabinPair& rRPair,
  StateSet& rInv);


/**
 * Live states  w.r.t a Rabin pair.
 *
 * API wrapper.
 *
 * @param rRAut
 *  Trasition system to operate on
 * @param rRPair
 *   Rabin pair to consider
 * @param rInv
 *   Resulting set of live states
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void RabinLiveStates(
  const vGenerator& rRAut,
  const RabinPair& rRPair,
  StateSet& rInv);

/**
 * Live states  w.r.t a Rabin acceptance condition.
 *
 * Iterates over all Rabin pairs and Returns the
 * union of all live states. 
 *
 * @param rRAut
 *  Trasition system to operate on
 * @param rInv
 *   Resulting set of live states
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void RabinLiveStates(const RabinAutomaton& rRAut, StateSet& rInv);


/**
 * Trim generator w.r.t. Rabin acceptance
 *
 * This function computes the set of states that contribute to the accepted
 * omega language. The current implementation returns the union of all states
 * that are live w.r.t. a Rabin pair, restricted to reachable states.
 *
 * @param rRAut
 *   Automaton to consider
 * @param rInv
 *   Resulting set of trim statess
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void RabinTrimSet(const RabinAutomaton& rRAut, StateSet& rTrim);

  
/**
 * Trim generator w.r.t Rabin acceptance
 *
 * Restrict the automaton to the set of trim states; see also RaibLiveStates and RabinTrimSet
 *
 * @param rRAut
 *   Automaton to trim
 * @return 
 *   True if resulting generator contains at least one initial state.
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API bool RabinTrim(RabinAutomaton& rRAut);

/**
 * Trim generator w.r.t Rabin acceptance
 *
 * Restrict the automaton to the set of trim states; see also RaibLiveStates and RabinTrimSet
 *
 * @param rRAut
 *   Automaton to trim
 * @param rRes
 *   Resulting trimmed automaton
 * @return 
 *   True if resulting generator contains at least one initial state.
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API bool RabinTrim(const RabinAutomaton& rRAut, RabinAutomaton& rRes);

/**
 * Construct Rabin-Buechi automata.
 *
 * Given two automata rRAut and rBAut, this function constructs the accessible product state set
 * and corresponding syncronizsed transitions. For the generated languages, this is exactly the same
 * as the common product operation. It then lifts the two individual acceptance conditions to the
 * product state set. If both rRAut and rBAut are full, so is the result. In that case, the
 * result accepts by its Rabin acceptance condition runs that are accepted by rRAut and by
 * its Buechi acceptance condition those path thet are accepted by rBAut.
 *
 * The intended use case is when rRAut is a liveness specification and rBAut is a plant with liveness
 * guarantees.
 *
 * @param rRAut
 *   Rabin automaton
 * @param rBAut
 *   Buechi automaton
 * @param rRes
 *   Resulting product automaton
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void  RabinBuechiAutomaton(const RabinAutomaton& rRAut, const Generator& rBAut,  RabinAutomaton& rRes);

/**
 * Product composition for a Rabin automaton with a Buechi automaton
 *
 * Referring to each acceptance condition specified by rRAut and rBAut, the resulting
 * Rabin automaton accepts all those runs, that are accepted by both rRAut and rBAut.
 * To support supervisory control, this function sets the marking of the result as a lifted
 * variant of rBAut.
 *
 * This implementation extends the usual product state space by a flag to indentify executions
 * with alternating marking. This restricts the applicability to one Rabin pair only. Future
 * revisions may drop this restriction.
 *
 * @param rRAut
 *   First automaton
 * @param rBAut
 *   Second automaton
 * @param rResGen
 *   Reference to resulting product composition
 *
 *
 * @ingroup OmgPlugin
 *
 */
extern FAUDES_API void RabinBuechiProduct(const RabinAutomaton& rRAut, const Generator& rBAut, RabinAutomaton& rRes);



} // namespace faudes

#endif 

