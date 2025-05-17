/** @file omg_buechifnct.h 

Operations on omega languages accepted by Buechi automata

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


#ifndef FAUDES_OMG_BUECHIFNCT_H
#define FAUDES_OMG_BUECHIFNCT_H

#include "corefaudes.h"

namespace faudes {


/**
 * Trim generator w.r.t Buechi acceptance
 *
 * This function removes states such that the generator becomes
 * omega trim while not affecting the induced omega language. 
 *
 * The implementation first makes the generator accessible
 * and then iteratively removes state that either 
 * never reach a marked state or that are guaranteed to eventually
 * reach a terminal state. There might be a more efficient 
 * approach.
 *
 * @param rGen
 *   Automaton to trim
 *
 * @return 
 *   True if resulting generator contains at least one initial state and at least one marked state.
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API bool BuechiTrim(vGenerator& rGen);

  
/**
 * Trim generator w.r.t Buechi acceptance
 *
 * This is a API wrapper for BuechiTrim(vGenerator&).
 *
 * @param rGen
 *   Automaton to trim (const ref)
 * @param rRes
 *   Resulting automaton
 * @return 
 *   True if resulting generator contains at least one initial state and at least one marked state.
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API bool BuechiTrim(const vGenerator& rGen, vGenerator& rRes);

/**
 * Check if generator is omega-trim.
 *
 * Returns true if all states are accessible, coacessible, and
 * have a successor state.
 *
 * @return
 *   True if generator is omega-trim
 */
extern FAUDES_API bool IsBuechiTrim(const vGenerator& rGen);


/**
 * Product composition for Buechi automata
 *
 * Referring to the Buechi acceptance condition, the resulting genarator 
 * accepts all those inifinite words that are accepted by both, G1 and G2.
 * This implementation extends the usual product state space by a flag to indentify 
 * executions with alternating marking.
 *
 * @param rGen1
 *   First generator
 * @param rGen2
 *   Second generator
 * @param rResGen
 *   Reference to resulting product composition generator
 *
 *
 * @ingroup OmgPlugin
 *
 */
extern FAUDES_API void BuechiProduct(const Generator& rGen1, const Generator& rGen2, Generator& rResGen);


/**
 * Product composition for Buechi automata
 *
 * See also BuechiProduct(const Generator&, const Generator&, Generator&).
 * This version tries to be transparent on event attributes: if
 * argument attributes match and if the result can take the respective
 * attributes, then they are copied; it is considered an error if 
 * argument attributes do not match.
 *
 * @param rGen1
 *   First generator
 * @param rGen2
 *   Second generator
 * @param rResGen
 *   Reference to resulting product composition generator
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void aBuechiProduct(const Generator& rGen1, const Generator& rGen2, Generator& rResGen);


/**
 * Parallel composition with relaxed acceptance condition.
 *
 * This version of the parallel composition relaxes the synchronisation of the acceptance
 * condition (marking). It requires that the omega extension of the generated language
 * has infinitely many prefixes that comply to the marked languages of G1 and G2, referring
 * to the projection on the respective alphabet.
 * It does however not require the synchronous acceptance. 
 *
 * @param rGen1
 *   First generator
 * @param rGen2
 *   Second generator
 * @param rResGen
 *   Reference to resulting parallel composition generator
 *
 *
 * @ingroup OmgPlugin
 *
 */
extern FAUDES_API void BuechiParallel(const Generator& rGen1, const Generator& rGen2, Generator& rResGen);


/**
 * Parallel composition with relaxed acceptance condition.
 *
 * See also BuechiParallel(const Generator&, const Generator&, Generator&).
 * This version tries to be transparent on event attributes: if
 * argument attributes match and if the result can take the respective
 * attributes, then they are copied; it is considered an error if 
 * argument attributes do not match.
 *
 * @param rGen1
 *   First generator
 * @param rGen2
 *   Second generator
 * @param rResGen
 *   Reference to resulting composition generator
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void aBuechiParallel(const Generator& rGen1, const Generator& rGen2, Generator& rResGen);


/**
 * Topological closure.
 *
 * This function computes the topological closure the omega language 
 * Bm accepted by the Buechi automaton rGen. 
 *
 * Method:
 * First, BuechiTrim is called to erase all states of rGen that do not
 * contribute to Bm. Then, all remaining states are marked.
 *
 * No restrictions on parameter.
 *
 *
 * @param rGen
 *   Generator that realizes Bm to which omega closure is applied
 *
 * <h4>Example:</h4>
 * <table>
 * <tr> <td> Generator G </td> <td> BuechiClosure(G) </td> </tr>
 * <tr>
 * <td> @image html tmp_omg_bclosure_g.png </td>
 * <td> @image html tmp_omg_bclosure_gRes.png </td>
 * </tr>
 * </table>
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API void BuechiClosure(Generator& rGen);


/**
 * Test for topologically closed omega language.
 *
 * This function tests whether the omega language Bm(G) realized by the specified
 * Buechi automata is topologically closed. 
 *
 * Method:
 * First, compute the Buechi-trim state set and restrict the discussion to that set.
 * Then, omega-closedness is equivalent to the non-existence on a non-trivial SCC 
 * with no marked states. 
 *
 * @param rGen
 *   Generator that realizes Bm to which omega closure is applied
 * @return
 *   True <> Bm(G) is omega closed
 *
 * @ingroup OmgPlugin
 */
extern FAUDES_API bool IsBuechiClosed(const Generator& rGen);

/**
 * Test for relative marking, omega langauges.
 *
 * Tests whether the omega language Bm(GCand) 
 * is relatively marked w.r.t.
 * the omega language Bm(GPlant). The formal definition of this property
 * requires
 *
 * closure(Bm(GCand)) ^ Bm(GPlant) <= Bm(GCand).
 *
 * The implementation first performs the product composition 
 * of the two generators with product state space QPlant x QCand and 
 * generated language L(GPlant x GCand) = L(Plant) ^ L(Cand). It then investigates 
 * all SCCs that do not
 * contain a state that corresponds to GCand-marking. If and only if
 * none of the considered SCCs has a GPlant marking, the function
 * returns true.
 *
 * The arguments GCand and GPlant are required to be deterministic and omega trim.
 *
 * @param rGenPlant
 *   Generator GPlant
 * @param rGenCand
 *   Generator GCand
 *
 * @exception Exception
 *   - alphabets of generators don't match (id 100)
 *   - arguments are not omega-trim (id 201, only if FAUDES_CHECKED is set)
 *   - arguments are non-deterministic (id 202, only if FAUDES_CHECKED is set)
 *
 *
 * @return 
 *   true / false
 *
 * @ingroup SynthesisPlugIn
 */
extern FAUDES_API bool IsBuechiRelativelyMarked(const Generator& rGenPlant, const Generator& rGenCand);

/**
 * Test for relative closedness, omega languages.
 *
 * Tests whether the omega language Bm(GCand) 
 * is relatively closed w.r.t.
 * the omega language Bm(GPlant). The formal definition of this property
 * requires
 *
 * closure(Bm(GCand)) ^ Bm(GPlant) =  Bm(GCand).
 *
 *
 * The implementation first performs the product composition 
 * of the two generators with product state space QPlant x QCand and 
 * generated language L(GPlant x GCand) = L(GPlant) ^ L(GCand). It uses the composition
 * to test the follwing three conditions:
 * - L(GCand) subseteq L(GPlant);
 * - no SCC of GPlant x GCand without GCand-marking contains a state with GPlant-marking; and
 * - no SCC of GPlant x GCand without GPlant-marking contains a state with GCand-marking.
 * If and only if all three tests are passed, the function
 * returns true.
 *
 * The arguments GCand and GPlant are required to be deterministic and omega trim.
 *
 * @param rGenPlant
 *   Generator GPlant
 * @param rGenCand
 *   Generator GCand
 *
 *
 * @exception Exception
 *   - alphabets of generators don't match (id 100)
 *   - arguments are not omega trim (id 201, only if FAUDES_CHECKED is set)
 *   - arguments are non-deterministic (id 202, only if FAUDES_CHECKED is set)
 *
 *
 * @return 
 *   true / false
 *
 * @ingroup SynthesisPlugIn
 */
extern FAUDES_API bool IsBuechiRelativelyClosed(const Generator& rGenPlant, const Generator& rGenCand);

/**
 * Test for relative closedness, omega languages.
 *
 * This variant does not perform consitency tests on the parameters. Neither
 * does it handle the special cases on empty arguments.
 * See also IsBuechiRelativelyClosed(const Generator&, const Generator& )
 *
 * @param rGenPlant
 *   Generator GPlant
 * @param rGenCand
 *   Generator GCand
 *
 */
extern FAUDES_API bool IsBuechiRelativelyClosedUnchecked(const Generator& rGenPlant, const Generator& rGenCand);


} // namespace faudes

#endif 

