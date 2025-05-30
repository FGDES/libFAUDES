/** @file syn_functions.h Misc functions related to synthesis */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010  Thomas Moor

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

 
#ifndef FAUDES_SYN_FUNCTIONS_H
#define FAUDES_SYN_FUNCTIONS_H

#include "corefaudes.h"

namespace faudes {
    


/**
 * Test for relative marking.
 *
 * Tests whether the language Lm(GCand) is relatively marked w.r.t.
 * the language Lm(GPlant). The formal definition of this property
 * requires
 *
 * closure(Lm(GCand)) ^ Lm(GPlant) <=  Lm(GCand).
 *
 * The implementation tests
 *
 * L(GCand) ^ Lm(GPlant) <= Lm(GCand)
 * 
 * by first performing the product composition and then inspecting
 * the marking to require
 *
 * ( forall accessible (qPlant,qCand) ) [ qPlant in QPlant_m implies qCand in QCand_m ].
 *
 * In general, the test is only sufficient.
 * Provided the arguments are trim and deterministic, the test
 * is sufficient and necessary. 
 *
 * @param rGenPlant
 *   Generator GPlant
 * @param rGenCand
 *   Generator GCand
 *
 *
 * @exception Exception
 *   - alphabets of generators don't match (id 100)
 *   - arguments are not trim (id 201, only if FAUDES_CHECKED is set)
 *   - arguments are non-deterministic (id 202, only if FAUDES_CHECKED is set)
 *
 * @return 
 *   true / false
 *
 * @ingroup SynthesisPlugIn
 */
extern FAUDES_API bool IsRelativelyMarked(const Generator& rGenPlant, const Generator& rGenCand);


/**
 * Test for relative prefix-closedness.
 *
 * Tests whether the language Lm(GCand) is relatively closed w.r.t.
 * the language Lm(GPant). The formal definition of this property
 * requires
 *
 * closure(Lm(GCand)) ^ Lm(GPlant) = Lm(GCand).
 *
 * The implementation tests
 *
 * L(GCand) ^ Lm(GPland) = Lm(GCand)
 * 
 * by performing the product composition and by testing
 *
 * - for L(GCand) subseteq L(GPlant), and
 * - ( forall accessible (qPland,qCand) ) [ qPlant in QPlant_m if and only if qCand in QCand_m ].
 *
 * In general, the test is only sufficient.
 * Provided the arguments are trim and deterministic, the test
 * is sufficient and necessary. 
 *
 * @param rGenPlant
 *   Generator GPlant
 * @param rGenCand
 *   Generator GCand
 *
 * @exception Exception
 *   - alphabets of generators don't match (id 100)
 *   - arguments are not trim (id 201, only if FAUDES_CHECKED is set)
 *   - arguments are non-deterministic (id 202, only if FAUDES_CHECKED is set)
 *
 * @return 
 *   true / false
 *
 * @ingroup SynthesisPlugIn
 */
extern FAUDES_API bool IsRelativelyClosed(const Generator& rGenPlant, const Generator& rGenCand);

/**
 * Supremal Relatively Closed Sublanguage
 *
 * Computes the supremal sublanguage of the specification E that is relatively closed 
 * w.r.t. the plant G. The result is given as a trim deterministic generator that 
 * may be used as a specification for a subsequent controller design via SupCon.
 *
 * The implementation removes states from the product GxE that conflict with
 * relative closedness. From the known formula supR(E)= (L^E) - (L-E)Sigma*, we know that 
 * the supremal sublanguage can be realized as a subautomaton of GxE. Thus, we conclude 
 * that our implementation indeed returns the supremum.
 *
 * Parameter restrictions: both generators must be deterministic and 
 * have the same alphabet.
 *
 *
 * @param rPlantGen
 *   Plant G
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
extern FAUDES_API void SupRelativelyClosed(
  const Generator& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen);


/**
 * Supremal Relatively Closed Sublanguage (internal function)
 *
 * 
 * This version of SupRelativelyClosed performs no consistency test of the given parameter.
 * It set's up a "composition map" as in the parallel composition, however,
 * the map may still contain states that have been removed from the result
 * to obtain controllability.
 *
 * Parameter restrictions: both generators must be deterministic and 
 * have the same alphabet.
 *
 *
 * @param rPlantGen
 *   Plant G
 * @param rSpecGen
 *   Specification Generator E
 * @param rCompositionMap
 *   Record parallel composition state mapping
 * @param rResGen
 *   Reference to resulting Generator, 
 *
 */
extern FAUDES_API void SupRelativelyClosedUnchecked(
  const Generator& rPlantGen, 
  const Generator& rSpecGen, 
  std::map< std::pair<Idx,Idx>,Idx >& rCompositionMap, 
  Generator& rResGen);


// legacy wrapper (pre 2.33d)  
extern FAUDES_API bool IsRelativelyPrefixClosed(const Generator& rGenPlant, const Generator& rGenCand);
extern FAUDES_API void SupRelativelyPrefixClosed(const Generator& rPlantGen, const Generator& rSpecGen, 
  Generator& rResGen);



} // namespace faudes

#endif 


