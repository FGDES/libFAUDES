/** @file omg_buechictrl.h Supremal controllable sublanguage w.r.t. Buechi acceptance */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010,2025  Thomas Moor

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

 
#ifndef FAUDES_OMG_BUECHICTRL_H
#define FAUDES_OMG_BUECHICTRL_H

#include "corefaudes.h"
#include "syn_include.h"
#include <stack>

namespace faudes {
    



/**
 * Test omega controllability
 *
 * Tests whether the candidate supervisor H is omega controllable w.r.t.
 * the plant G, where omega-languages are interpreted by Buechi acceptance condition.
 * This implementation invokes IsControllable and IsBuechiRelativelyClosed.
 * A future implementation may be more efficient.
 * 
 * Parameter restrictions: both generators must be deterministic, omega-trim and 
 * have the same alphabet.
 *
 *
 * @param rPlantGen
 *   Plant G
 * @param rCAlph
 *   Controllable events
 * @param rSupCandGen
 *   Supervisor candidate H
 *
 * @exception Exception
 *   - Alphabets of generators don't match (id 100)
 *   - Arguments are not omega trim (id 201, only if FAUDES_CHECKED is set)
 *   - Arguments are non-deterministic (id 202, only if FAUDES_CHECKED is set)
 *
 * @return 
 *   true / false
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API bool IsBuechiControllable(
  const Generator& rPlantGen, 
  const EventSet&  rCAlph, 
  const Generator& rSupCandGen);


/**
 * Test omega-controllability. 
 *
 * Tests whether the candidate supervisor h is omega controllable w.r.t.
 * the plant g; this is a System wrapper for IsOmegaControllable.
 *
 * @param rPlantGen
 *   Plant g generator
 * @param rSupCandGen
 *   Supervisor candidate h generator 
 *
 * @exception Exception
 *   - Alphabets of generators don't match (id 100)
 *   - Arguments are not omega trim (id 201, only if FAUDES_CHECKED is set)
 *   - Arguments are non-deterministic (id 202, only if FAUDES_CHECKED is set)
 *
 * @return 
 *   true / false
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API bool IsBuechiControllable(
  const System& rPlantGen, 
  const Generator& rSupCandGen);





/**
 * Omega-synthesis w.r.t. Buechi acceptance condition
 *
 * Computation of the supremal oemga-controllable sublanguage as proposed by 
 * Thistle/Wonham in "Control of w-Automata, Church's Problem, and the Emptiness 
 * Problem for Tree w-Automata", 1992, and, here, applied to the specific case 
 * of deterministic Buechi automata. In the given setting, the result matches the 
 * limit of the controllable prefix intersected with the plant and specification 
 * omega-languages.
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
extern FAUDES_API void SupBuechiCon(
  const Generator& rPlantGen, 
  const EventSet&  rCAlph,
  const Generator& rSpecGen, 
  Generator& rResGen);



/**
 * Omega-synthesis w.r.t. Buechi acceptance
 *
 * This is the RTI wrapper for   
 * SupBuchiCon(const Generator&, const EventSet&, const Generator&, Generator&).
 * Controllability attributes are taken from the plant argument.
 * If the result is specified as a System, attributes will be copied
 * from the plant argument.
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
extern FAUDES_API void SupBuechiCon(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen);


/**
 * Omega-synthesis w.r.t. Buechi accptance
 *
 * This procedure first computes the supremal omega-controllable sublanguage as proposed by
 * J. Thistle, 1992, applied to the specific case of deterministoc Buechi automata.
 * It then applies a control pattern to obtain a relatively topologically-closed result,
 * i.e., the topological closure of the result can be used as a supervisor.
 *
 * Parameter restrictions: both generators must be deterministic and 
 * have the same alphabet.
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
 *   the closed-loop behaviour.
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
extern FAUDES_API void BuechiCon(
  const Generator& rPlantGen, 
  const EventSet&  rCAlph,
  const Generator& rSpecGen, 
  Generator& rResGen);



/**
 * Omega-synthesis 
 *
 * This is the RTI wrapper for   
 * BuechiCon(const Generator&, const EventSet&, const Generator&, Generator&).
 * Controllability attributes are taken from the plant argument.
 * If the result is specified as a System, attributes will be copied
 * from the plant argument.
 *
 *
 * @param rPlantGen
 *   Plant System
 * @param rSpecGen
 *   Specification Generator
 * @param rResGen
 *   Reference to resulting Generator to realize
 *   the closed-loop behaviour.
 *
 * @exception Exception
 *   Alphabets of generators don't match (id 100)
 *   plant nondeterministic (id 201)
 *   spec nondeterministic (id 203)
 *   plant and spec nondeterministic (id 204)
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void BuechiCon(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen);


/**
 * Omega-synthesis for partial observation (experimental!)
 *
 * Variation of supremal omega-controllable sublanguage to address 
 * normality requirements in the context of partial observation. The test
 * used in this implementation is sufficient but presumably to necessary.
 * Thus, the function in general return only a subset of the relevant controllable 
 * prefix.
 *
 * Parameter restrictions: both generators must be deterministic and 
 * have the same alphabet.
 *
 *
 * @param rPlantGen
 *   Plant G
 * @param rCAlph
 *   Controllable events
 * @param rOAlph
 *   Observable events
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
extern FAUDES_API void SupBuechiConNorm(
  const Generator& rPlantGen, 
  const EventSet&  rCAlph,
  const EventSet&  rOAlph,
  const Generator& rSpecGen, 
  Generator& rResGen);



/**
 * Omega-synthesis for partial observation (experimental!)
 *
 *
 * This is the RTI wrapper for   
 * SupBuechiConNorm(const Generator&, const EventSet&, const Generator&, Generator&).
 * Controllability attributes and observability attributes are taken from the plant argument.
 * If the result is specified as a System, attributes will be copied
 * from the plant argument.
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
extern FAUDES_API void SupBuechiConNorm(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen);



/**
 * Omega-synthesis for partial observation (experimental!)
 *
 * Variation of supremal controllable prefix under partial observation.
 * This variation applies a control pattern to obtain a relatively closed and 
 * omega-normal result. The latter properties are validated and an exception
 * is thrown on an error. Thus, this function should not produce "false-positives".
 * However, since it is derived from SupBuechiConNorm(), is may return the
 * empty languages even if a non-empty controller exists.
 *
 * Parameter restrictions: both generators must be deterministic and 
 * have the same alphabet.
 *
 *
 * @param rPlantGen
 *   Plant G
 * @param rCAlph
 *   Controllable events
 * @param rOAlph
 *   Observable events
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
extern FAUDES_API void BuechiConNorm(
  const Generator& rPlantGen, 
  const EventSet&  rOAlph,
  const EventSet&  rCAlph,
  const Generator& rSpecGen, 
  Generator& rResGen);



/**
 * Omega-synthesis for partial observation (experimental!)
 *
 * This is the RTI wrapper for   
 * BuechiConNorm(const Generator&, const EventSet&, const Generator&, Generator&).
 * Controllability attributes are taken from the plant argument.
 * If the result is specified as a System, attributes will be copied
 * from the plant argument.
 *
 *
 * @param rPlantGen
 *   Plant System
 * @param rSpecGen
 *   Specification Generator
 * @param rResGen
 *   Reference to resulting Generator to realize
 *   the closed-loop behaviour.
 *
 * @exception Exception
 *   Alphabets of generators don't match (id 100)
 *   plant nondeterministic (id 201)
 *   spec nondeterministic (id 203)
 *   plant and spec nondeterministic (id 204)
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void BuechiConNorm(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen);




} // namespace faudes

#endif 


