/** @file omg_rabinctrl.h Controller synthesis for Rabin automata */

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


#ifndef FAUDES_OMG_RABINCTRL_H
#define FAUDES_OMG_RABINCTRL_H

#include "corefaudes.h"
#include "omg_rabinaut.h"

namespace faudes {



/**
 * Controllability prefix for Rabin automata.
 *
 * Computation of the controllability prefix as proposed by 
 * Thistle/Wonham in "Supervision of infinite behavior of Discrete Event Systems, 1994,
 * using the fixpoint iteration from "Control of w-Automata, Church's Problem, and the
 * Emptiness Problem for Tree w-Automata", 1992.
 *
 * @param rRAut
 *   Automaton to control
 * @param rSigmaCtrl
 *   Set of controllable events
 * @param rCtrlPfx
 *   State set that marks the controllability prefix.
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void RabinCtrlPfx(
  const RabinAutomaton& rRAut, const EventSet& rSigmaCtrl,
  StateSet& rCtrlPfx);

/**
 * Controllability prefix for Rabin automata.
 *
 * API wrapper to return the controllability prefix as generator.
 *
 * @param rRAut
 *   Automaton to control
 * @param rSigmaCtrl
 *   Set of controllable events
 * @param rCtrlPfx
 *   Generator that  marks the controllability prefix.
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void RabinCtrlPfx(
  const RabinAutomaton& rRAut, const EventSet& rSigmaCtrl,
  Generator& rCtrlPfx);


/**
 * Controllability prefix for Rabin automata.
 *
 * API wrapper to return a controller
 *
 * @param rRAut
 *   Automaton to control
 * @param rSigmaCtrl
 *   Set of controllable events
 * @param rController
 *   Map from states to enabled events.
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void RabinCtrlPfx(
  const RabinAutomaton& rRAut, const EventSet& rSigmaCtrl,
  TaIndexSet<EventSet>& rController);

  
/**
 * Omega-synthesis w.r.t. Buechi/Rabin acceptance condition
 *
 * Computation of the supremal oemga-controllable sublanguage as proposed by 
 * Thistle/Wonham in "Control of w-Automata, Church's Problem, and the Emptiness 
 * Problem for Tree w-Automata", 1992, and, here, applied to the specific case 
 * of deterministic Buechi plangt and a Rabin specification.
 *
 * Parameter restrictions: both generators must be deterministic and 
 * refer to the same alphabet.
 *
 *
 * @param rBPlant
 *   Plant to accept L w.r.t. Buechi acceptance
 * @param rCAlph
 *   Controllable events
 * @param rRSpec
 *   Specification to accept E  w.r.t. Rabin  acceptance
 * @param rRes
 *   Resulting RabinAutomaton to accept the
 *   supremal controllable sunlanguage
 *
 * @exception Exception
 *   - alphabets of generators don't match (id 100)
 *   - plant nondeterministic (id 201)
 *   - spec nondeterministic (id 203)
 *   - plant and spec nondeterministic (id 204)
 *
 * @ingroup OmgPlugin
 *
 */
extern FAUDES_API void SupRabinCon(
  const Generator& rBPlant, 
  const EventSet&  rCAlph,
  const RabinAutomaton& rRSpec, 
  RabinAutomaton& rRes);



/**
 * Omega-synthesis w.r.t. Buechi acceptance
 *
 * This is the RTI wrapper for   
 * SupRabinCon(const Generator&, const EventSet&, const Generator&, Generator&).
 * Controllability attributes are taken from the plant argument.
 * Attributes will be copied from the plant argument.
 *
 *
 * @param rBPlant
 *   System to accept L w.r.t. Buechi acceptance
 * @param rRSpec
 *   Specification to accept E  w.r.t. Rabin  acceptance
 * @param rResGen
 *   Reference to resulting RabinAutomaton to accept the
 *   supremal controllable sunlanguage
 *
 * @exception Exception
 *   Alphabets of generators don't match (id 100)
 *   plant nondeterministic (id 201)
 *   spec nondeterministic (id 203)
 *   plant and spec nondeterministic (id 204)
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void SupRabinCon(
  const System& rBPlant, 
  const RabinAutomaton& rRSpec, 
  RabinAutomaton& rRes);


/**
 * Omega-synthesis w.r.t. Buechi/Rabin acceptance condition
 *
 * Computation of the "greedy" controller that runs for early acceptance.
 * The result is a Buechi automaton of the closed-loop behaviour under
 * greedy control. The closure is readily utilised as a controller.
 *
 * Parameter restrictions: both generators must be deterministic and 
 * refer to the same alphabet.
 *
 * @param rBPlant
 *   Plant to accept L w.r.t. Buechi acceptance
 * @param rCAlph
 *   Controllable events
 * @param rRSpec
 *   Specification to accept E  w.r.t. Rabin  acceptance
 * @param rRes
 *   Closed-loop behaviour under greedy control.
 *
 * @exception Exception
 *   - alphabets of generators don't match (id 100)
 *   - plant nondeterministic (id 201)
 *   - spec nondeterministic (id 203)
 *   - plant and spec nondeterministic (id 204)
 *
 * @ingroup OmgPlugin
 *
 */
extern FAUDES_API void RabinCtrl(
  const Generator& rBPlant, 
  const EventSet&  rCAlph,
  const RabinAutomaton& rRSpec, 
  Generator& rRes);


/**
 * Omega-synthesis w.r.t. Buechi/Rabin acceptance condition
 *
 * Computation of the "greedy" supervisor that runs for early acceptance; this
 * variant retrieves controllabillity arguments from the specified plant.
 * The result is a Buechi automaton of the closed-loop behaviour under
 * greedy control. The closure is readily utilised as a controller.
 *
 * Parameter restrictions: both generators must be deterministic and 
 * refer to the same alphabet.
 *
 * @param rBPlant
 *   Plant to accept L w.r.t. Buechi acceptance
 * @param rRSpec
 *   Specification to accept E  w.r.t. Rabin  acceptance
 * @param rRes
 *   Closed-loop behaviour udner greedy control.
 *
 * @exception Exception
 *   - alphabets of generators don't match (id 100)
 *   - plant nondeterministic (id 201)
 *   - spec nondeterministic (id 203)
 *   - plant and spec nondeterministic (id 204)
 *
 *
 * @ingroup OmgPlugin
 *
 */
extern FAUDES_API void RabinCtrl(
  const System& rBPlant, 
  const RabinAutomaton& rRSpec, 
  Generator& rRes);


/**
 * Omega-synthesis w.r.t. Buechi/Rabin acceptance condition
 *
 * Computation of a controller that respects next to the common upper bound
 * specification also a lower bound. Only on exit of the lower-bound, the greedy
 * controlle is activated.
 * The result returned is a Buechi automaton of the closed-loop behaviour. The
 * closure is readily utilised as a controller.
 *
 * Parameter restrictions: both generators must be deterministic and 
 * refer to the same alphabet. The lower bound argument is interpreted as the generated
 * omega-language intersected with the plant behaviour. The effective lower bound
 * must not exceed the supremal omega-controllable sublanguage of the upper bound
 * specification. This is currently not checked.
 *
 * @param rBPlant
 *   Plant to accept L w.r.t. Buechi acceptance
 * @param rGLSpec
 *   Specification to be generate the lower bound A
 * @param rRUSpec
 *  Specification to accept the upper bound E  w.r.t. Rabin  acceptance
 * @param rRes
 *   Closed-loop behaviour.
 *
 * @exception Exception
 *   - alphabets of generators don't match (id 100)
 *   - plant nondeterministic (id 201)
 *   - spec nondeterministic (id 203)
 *   - plant and spec nondeterministic (id 204)
 *
 *
 * @ingroup OmgPlugin
 *
 */
extern FAUDES_API void RabinCtrl(
  const Generator& rBPlant, 
  const EventSet& rCAlph, 
  const Generator& rGLSpec, 
  const RabinAutomaton& rRUSpec, 
  Generator& rRes);
  
/**
 * Omega-synthesis w.r.t. Buechi/Rabin acceptance condition
 *
 * Computation of a controller that respects next to the common upper bound
 * specification also a lower bound. Only on exit of the lower-bound, the greedy
 * controlle is activated. This is an API wrapper to obtain controllability attributes
 * from the plant parameter.
 * The result returned is a Buechi automaton of the closed-loop behaviour. The
 * closure is readily utilised as a controller.
 *
 * Parameter restrictions: both generators must be deterministic and 
 * refer to the same alphabet. The lower bound argument is interpreted as the generated
 * omega-language intersected with the plant behaviour. The effective lower bound
 * must not exceed the supremal omega-controllable sublanguage of the upper bound
 * specification. This is currently not checked.
 *
 * @param rBPlant
 *   System to accept L w.r.t. Buechi acceptance
 * @param rGLSpec
 *   Specification to be generate the lower bound A
 * @param rRUSpec
 *  Specification to accept the upper bound E  w.r.t. Rabin  acceptance
 * @param rRes
 *   Closed-loop behaviour.
 *
 * @exception Exception
 *   - alphabets of generators don't match (id 100)
 *   - plant nondeterministic (id 201)
 *   - spec nondeterministic (id 203)
 *   - plant and spec nondeterministic (id 204)
 *
 *
 * @ingroup OmgPlugin
 *
 */
extern FAUDES_API void RabinCtrl(
  const System& rBPlant, 
  const Generator& rGLSpec, 
  const RabinAutomaton& rRUSpec, 
  Generator& rRes);
  

} // namespace faudes

#endif 

