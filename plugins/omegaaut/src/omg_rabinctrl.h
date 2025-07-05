/** @file omg_rabinctrl.h

Controller synthesis for Rabin automata

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
 * @param rPlantGen
 *   Plant G
 * @param rCAlph
 *   Controllable events
 * @param rSpecGen
 *   Specification Generator E
 * @param rResGen
 *   Reference to resulting Generator to realize
 *   the supremal closed-loop behaviour.
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
 * @param rPlantGen
 *   Plant System
 * @param rSpecGen
 *   Specification Generator
 * @param rResGen
 *   Reference to resulting Generator to realize
 *   the supremal closed-loop behaviour.
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

  

} // namespace faudes

#endif 

