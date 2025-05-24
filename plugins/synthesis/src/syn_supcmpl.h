/** @file syn_supcmpl.h Supremal complete sublanguage for infinite time behaviours */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010, 2025  Thomas Moor

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

 
#ifndef FAUDES_WSUPCON_H
#define FAUDES_WSUPCON_H

#include "corefaudes.h"
#include <stack>

namespace faudes {
    




/**
 * Supremal controllable and complete sublanguage
 *
 * Given a plant and a specification, this function computes a realisation of 
 * the supremal controllable and complete sublange. This version consideres the
 * generated languages (ignores the marking). In particular, this implies that
 * the result is prefix closed. It is returned as generated language.
 * 
 * Starting with a product composition of plant and specification, the implementation
 * iteratively remove states that either contradict controllability or completeness.
 * Removal of states is continued until no contradicting states are left. 
 * Thus, the result is indeed controllable and complete. The algorithm was
 * proposed in 
 *
 * R. Kumar, V. Garg, and S.I. Marcus. On supervisory control of
 * sequential behaviors. IEEE Transactions on Automatic Control,
 * Vol. 37: pp.1978-1985, 1992.
 *
 * The paper proves supremality of the result. Provided that the corresponding 
 * omega language of the specification is closed, the result of the above algorithm 
 * also realises the least restrictive closed loop behaviour of the corresponding 
 * omega language control problem.
 *
 * Parameter restrictions: both generators must be deterministic and 
 * have the same alphabet. The result will be accessible and deterministic.
 *
 *
 * @param rPlantGen
 *   Plant G
 * @param rCAlph
 *   Controllable events
 * @param rSpecGen
 *   Specification Generator E
 * @param rResGen
 *   Reference to resulting Generator
 *
 * @exception Exception
 *   - alphabets of generators don't match (id 100)
 *   - plant nondeterministic (id 201)
 *   - spec nondeterministic (id 203)
 *   - plant and spec nondeterministic (id 204)
 *
 * @ingroup SynthesisPlugIn
 *
 */
extern FAUDES_API void SupConCmplClosed(
  const Generator& rPlantGen, 
  const EventSet&  rCAlph,
  const Generator& rSpecGen, 
  Generator& rResGen);



/**
 * Supremal controllable and complete sublanguage.
 *
 * This is the RTI wrapper for   
 * SupConCmplClosed(const Generator&, const EventSet&, const Generator&, Generator&).
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
 *   Reference to resulting Generator
 *
 * @exception Exception
 *   Alphabets of generators don't match (id 100)
 *   plant nondeterministic (id 201)
 *   spec nondeterministic (id 203)
 *   plant and spec nondeterministic (id 204)
 *
 * @ingroup SynthesisPlugIn
 */
extern FAUDES_API void SupConCmplClosed(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen);


/**
 * Supremal controllable and complete sublanguage
 *
 *
 * Given a plant and a specification, this function computes a realisation of 
 * the supremal controllable and complete sublange. This version consideres the
 * marked languages. 
 *
 * Starting with a product composition of plant and specification, the implementation
 * iteratively remove states that contradict controllability  or completeness or that
 * are not coaccessible. Removal of states is continued until no contradicting states are left. 
 * Thus, the result is indeed controllable, complete and coaccessible. 
 *
 * Considering the marked languages implies that only strings that simultanuosly 
 * reach a marking can survive the above procedure. From an omega-languages perspective, 
 * this is of limited use. However, in the special situation that the specification
 * is relatively closed w.r.t. the plant, we can replace the specification by its
 * prefix closure befor invoking SupConComplNB. In this situation we claim that
 * the procedure returns a realisation of the the least restrictive closed loop behaviour 
 * of the corresponding omega language control problem. 
 *
 *
 * @param rPlantGen
 *   Plant G
 * @param rCAlph
 *   Controllable events
 * @param rSpecGen
 *   Specification Generator E
 * @param rResGen
 *   Reference to resulting Generator
 *
 * @exception Exception
 *   - alphabets of generators don't match (id 100)
 *   - plant nondeterministic (id 201)
 *   - spec nondeterministic (id 203)
 *   - plant and spec nondeterministic (id 204)
 *
 * @ingroup SynthesisPlugIn
 *
 */
extern FAUDES_API void SupConCmpl(
  const Generator& rPlantGen, 
  const EventSet&  rCAlph,
  const Generator& rSpecGen, 
  Generator& rResGen);



/**
 * Supremal controllable and complete sublanguage.
 *
 * This is the RTI wrapper for   
 * SupConCmpl(const Generator&, const EventSet&, const Generator&, Generator&).
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
 *   Reference to resulting Generator
 *
 * @exception Exception
 *   Alphabets of generators don't match (id 100)
 *   plant nondeterministic (id 201)
 *   spec nondeterministic (id 203)
 *   plant and spec nondeterministic (id 204)
 *
 * @ingroup SynthesisPlugIn
 */
extern FAUDES_API void SupConCmpl(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen);


/**
 * Supremal controllable, normal and complete sublanguage.
 *
 *
 * SupConNormCmpl computes the supremal sublanguage
 * of language K (marked by rSpecGen) that 
 * - is controllable w.r.t. the language L (marked by rPlantGen);
 * - has a prefix closure that is normal w.r.t. the closure of L
 * - is complete
 *
 * The implementation is based on an iteration by Yoo, Lafortune and Lin 
 * "A uniform approach for computing supremal sublanguages arising 
 * in supervisory control theory", 2002, further developped in 
 * Moor, Baier, Yoo, Lin, and Lafortune "On the computation of supremal 
 * sublanguages relevant to supervisory control, WODES 2012. The relationship 
 * to the supervision of omega languages under partial observation is discussed
 * as an example in the WODES 2012 paper.
 *
 * Parameters have to be deterministic, result is deterministic.
 *
 *
 * @param rPlantGen
 *   Plant L
 * @param rCAlph
 *   Controllable events
 * @param rOAlph
 *   Observable events
 * @param rSpecGen
 *   Specification Generator E
 * @param rResGen
 *   Reference to resulting Generator
 *
 * @exception Exception
 *   - alphabets of generators don't match (id 100)
 *   - plant nondeterministic (id 201)
 *   - spec nondeterministic (id 203)
 *   - plant and spec nondeterministic (id 204)
 *
 * @ingroup SynthesisPlugIn
 *
 */
extern FAUDES_API void SupConNormCmpl(
  const Generator& rPlantGen, 
  const EventSet&  rCAlph,
  const EventSet&  rOAlph,
  const Generator& rSpecGen, 
  Generator& rResGen);


/**
 * Supremal controllable, normal and complete sublanguage.
 *
 * This is the RTI wrapper for   
 * SupConNormCmpl(const Generator&, const EventSet&, const EventSet&, const Generator&, Generator&).
 * Event attributes are taken from the plant argument.
 * If the result is specified as a System, attributes will be copied
 * from the plant argument.
 *
 * @param rPlantGen
 *   Plant System
 * @param rSpecGen
 *   Specification Generator
 * @param rResGen
 *   Reference to resulting Generator
 *
 * @exception Exception
 *   Alphabets of generators don't match (id 100)
 *   plant nondeterministic (id 201)
 *   spec nondeterministic (id 203)
 *   plant and spec nondeterministic (id 204)
 *
 * @ingroup SynthesisPlugIn
 */
extern FAUDES_API void SupConNormCmpl(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen);




} // namespace faudes

#endif 


