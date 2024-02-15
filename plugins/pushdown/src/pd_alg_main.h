/** @file pd_alg_main.h  Top-Level functions*/


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013/14 Ramon Barakat, Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/


#ifndef FAUDES_PD_ALG_MAIN_H
#define FAUDES_PD_ALG_MAIN_H

#include "corefaudes.h"
#include "pd_pdgenerator.h"
#include "pd_alg_sub.h"

#include "pd_debug.h"
#include "pd_scopelogger.h"

namespace faudes {

/**
   * Construct a minimal restrictive sound controller, based on the Algorithm presented by
   *  Sven Schneider and Anne-Kathrin Schmuck.
   *
   * Alphabets of specification and plant are expected to be equal.
   *
   * @param spec
   *    Reference to specification
   * @param plant
   *    Reference to plant
   * @param rRes
   *    Reference to resulting minimal restrictive controller
   * @param debug
   * 	  Set true, to print out single steps of the function
   *
   * @return
   */
extern FAUDES_API void PushdownConstructController(const PushdownGenerator& rSpec,const System& rPlant, PushdownGenerator& rRes, bool debug = false);

/**
 * Create a nonblocking product generator of a pushdown generator and a regular generator.
 * For that, ConstructControllerPreCombine(...) will create the product generator and call the
 * PushdownBlockfree(...) function.
 *
 * @param rSpec
 * 	  Reference to pushdown generator
 * @param rPlant
 *    Reference to regular generator
 * @param rRes
 *    Reference to resulting nonblocking product generator
 * @param debug
 * 	  Set true, to print out single steps of the function
 *
 * @return
 */
extern FAUDES_API void ConstructControllerPreCombine(const PushdownGenerator& rSpec, const System& rPlant, PushdownGenerator& rRes, bool debug = false);

/**
 * Looped execution to construct a minimal restrictive controller.
 * Alphabets of specification and plant are expected to be equal.
 *
 * @param rContr
 *    Reference to controller candidate
 * @param rPlant
 *    Reference to plant
 * @param rRes
 *    Reference to resulting minimal restrictive controller
 * @param loopcounter
 *    Number to count number of iterations
 * @param debug
 * 	  Set true, to print out single steps of the function
 *
 * @return
 *    number of iterations of ConstructControllerLoop
 */
extern FAUDES_API int ConstructControllerLoop(const PushdownGenerator& rContr, const System& rPlant, PushdownGenerator& rRes, int loopcounter = 0, bool debug = false);

/**
 * Make a pushdown generator nonblocking.
 * This means, that the function will solve blocking issues.
 *
 * Technical Details:
 * The Algorithm converts the given pushdown generator to a (LR(1)-) grammar,
 * removes unreachable productions and reconverts the grammar (via LR-parser) to an automaton.
 * This will remodel the pushdown generator and may replace states and stack symbols with new ones.
 *
 * The marked language does not change
 *
 * @param rPd
 *    Reference to pushdown generator to make nonblocking
 * @param rResPd
 *    Reference to resulting nonblocking pushdown generator
 * @param debug
 * 	  Set true, to print out single steps of the function
 *
 * @return
 */
extern FAUDES_API void PushdownBlockfree(const PushdownGenerator& rPd, PushdownGenerator& rResPd, bool debug = false);


/**
 * Construct the accessible part of a pushdown generator
 *
 * Technical Details:
 * The Algorithm converts the given pushdown generator to a context free grammar (CFG) and
 * removes unreachable productions. From the reduced CFG accessible states and transitions can be identified.
 *
 * By creating only reducible productions (see Sp2Lr() ) the function will also remove not coaccessible states.
 * To avoid this, set 'coacc' to false.
 *
 * @param rPd
 *    Reference to pushdown generator to make accessible
 * @param resPd
 *    Reference to resulting accessible pushdown generator
 * @param coacc
 *    Set false, to avoid that not coaccessible states will be removed
 * @param debug
 * 	  Set true, to print out single steps of the function.
 *
 * @return
 */
extern FAUDES_API void PushdownAccessible(const PushdownGenerator& pd, PushdownGenerator& resPd, bool coacc = true, bool debug = false);


/**
 * Get all states that are the starting state of a lambda reading transition
 *
 * @param pd
 *    the generator
 * @return
 *    the states
 */
extern FAUDES_API StateSet Transient(const PushdownGenerator& pd);

/**
 * Remove non-controllable ears from a generator. The initial state must not be an ear.
 * Times and Split must have been executed on the generator.
 *
 * @param pd
 *    the pushdown generator with ears
 * @param s
 *    the system that was used in the intersection operation to generate the
 * pushdown generator
 * @return
 *    pushdowngenerator without noncontrollable ears
 */
extern FAUDES_API PushdownGenerator Rnce(const PushdownGenerator& pd, const System& s);

/**
 * synchronous product generator of a pushdown generator and a regular generator
 *
 * @param reg
 *    the regualar generator
 * @param pd
 *    the pushdown generator
 * @return
 *    the synchronous product
 */
extern FAUDES_API PushdownGenerator Times(const System& reg, const PushdownGenerator& pd);

/**
 * Adds the intersection of events of the first two generators to the
 * result generator. The function considers observability and controllability,
 * so observability and controllability attributes will be kept.
 *
 * @param s
 *    first generator
 * @param pd
 *    second generator
 * @param rPd
 *    result generator
 */
extern FAUDES_API void IntersectEvents(const System& s, const PushdownGenerator& pd, PushdownGenerator& rPd);

/**
 * Splits the states of a pushdown generator into heads and ears. Each ear is
 * associated with a stack symbol.
 *
 * Transitions will always be either from ear to head or from head to ear.
 * Transitions from ear to head will always pop the stack symbol associated
 * with the ear. Transitions from head to ear will always read lambda and
 * pop and push the stack symbol associated with the ear.
 *
 * @param pd
 *    pushdown generator to be split
 * @return
 *    the split pushdown generator
 */
extern FAUDES_API PushdownGenerator Split(const PushdownGenerator& pd);



/**
 * Sets controllability and observability flags for a pushdown generator's events to
 * the event flags of another pushdown generator
 *
 * @param correctPd
 *    pushdown generator with correct flags
 * @param pd
 *    this generator's event flags will be set to the correctPd's event flags
 */
extern FAUDES_API void CorrectEvents(const PushdownGenerator& correctPd, PushdownGenerator& pd);


/**
 * from http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
 * Calculates time difference x - y. For debugging and time measurement.
 *
 * @param result
 *    the time difference is stored here
 * @param x
 *    the time x
 * @param y
 *    the time y
 * @return
 *    1 if x - y is negative, else 0
 */
int timeval_subtract (timeval* result, timeval* x, timeval* y);



  

} // namespace faudes

#endif
