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
 * @param rCtrlPfx
 *   State set that marks the controllability prefix.
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void RabinCtrlPfx(
  const RabinAutomaton& rRAut, const EventSet& rSigmaCtrl,
  StateSet& rCtrlPfx);


} // namespace faudes

#endif 

