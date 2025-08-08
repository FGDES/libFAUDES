/** @file omg_rabinctrlrk.h Synthesis omega languages accepted by Rabin automata */

/* FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2025 Changming Yang, Thomas Moor

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


/*
This is an alternative implementation to compute the controllable set
aka the controllability prefix following more closely the literature.
This contribution is highly appreciated. However, I can not merge right
now, since the comming main branch considers liveness properties of the
plant which are not covered in this otherwise very tidy variant. So we
keep the both variants until we figured how to resolve best.

TM, 2025/07
*/

#ifndef FAUDES_OMG_RABINCTRLRK_H
#define FAUDES_OMG_RABINCTRLRK_H

#include "corefaudes.h"
#include "omg_rabinaut.h"

namespace faudes {

/**
 * State feedback mapping for Rabin control synthesis.
 * 
 * Maps each controllable state to its corresponding control pattern (subset of events).
 * This implements the state feedback controller from Theorem 6.4 in Thistle/Wonham 1994.
 */
typedef TaIndexSet<EventSet> StateFeedbackMap;

/**
 * State ranking information for fixpoint computation.
 * 
 * Records the level at which each state was added during the nested fixpoint iteration.
 * Used for constructing the state feedback controller according to Theorem 6.4.
 */
class StateRanking : public AttributeVoid {
public:
  int muLevel;     // Outer mu-iteration level (i)
  int nuLevel;     // Inner nu-iteration level (j) 
  int branchType;  // Which branch of the p-reach operator (0 or 1)
  
  StateRanking() : AttributeVoid(), muLevel(0), nuLevel(0), branchType(0) {}
  StateRanking(int mu, int nu, int branch) : AttributeVoid(), muLevel(mu), nuLevel(nu), branchType(branch) {}
  
  // Lexicographic comparison for state ranking
  bool operator<(const StateRanking& other) const {
    if (muLevel != other.muLevel) return muLevel < other.muLevel;
    if (nuLevel != other.nuLevel) return nuLevel < other.nuLevel;
    return branchType < other.branchType;
  }
  
  // Required by AttributeVoid interface
  virtual bool IsDefault(void) const {
    return muLevel == 0 && nuLevel == 0 && branchType == 0;
  }
  
  // Assignment operator
  StateRanking& operator=(const StateRanking& other) {
    if (this != &other) {
      muLevel = other.muLevel;
      nuLevel = other.nuLevel;
      branchType = other.branchType;
    }
    return *this;
  }
};

/**
 * State ranking map for control synthesis.
 * 
 * Maps each state to its ranking information during fixpoint computation.
 */
typedef TaIndexSet<StateRanking> StateRankingMap;


/**
 * Controllability prefix with state feedback for Rabin automata.
 *
 * Extended version that computes both the controllability prefix and 
 * the corresponding state feedback controller, implementing Theorem 6.4
 * from Thistle/Wonham 1994.
 *
 * @param rRAut
 *   Automaton to control
 * @param rSigmaCtrl
 *   Set of controllable events
 * @param rCtrlPfx
 *   State set that marks the controllability prefix
 * @param rStateFeedback
 *   State feedback mapping: for each controllable state, provides the control pattern
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void RabinCtrlPfxWithFeedback(
  const RabinAutomaton& rRAut, const EventSet& rSigmaCtrl,
  TaIndexSet<EventSet>& rController);




} // namespace faudes

#endif 

