/** @file omg_rabinctrlpartialobs.h 
 * 
 * Rabin control synthesis under partial observation
 * 
 * This module implements high-level API functions for supervisory control
 * synthesis of Rabin automata under partial observation constraints.
 * 
 * @ingroup OmegaautPlugin
 */

#ifndef FAUDES_OMG_RABINCTRLPARTIALOBS_H
#define FAUDES_OMG_RABINCTRLPARTIALOBS_H

#include "libfaudes.h"
#include "omg_rabinaut.h"
#include "omg_pseudodet.h"

namespace faudes {

/**
 * @brief Rabin control synthesis under partial observation (System interface)
 *
 * This function performs supervisory control synthesis for Rabin automata
 * under partial observation. It automatically extracts controllable and 
 * observable events from the System's event attributes, then combines the 
 * plant and specification, expands to control patterns, handles unobservable 
 * events, and applies pseudo-determinization to produce a deterministic supervisor.
 *
 * The synthesis algorithm performs the following steps:
 * 1. Extract controllable and observable events from System attributes
 * 2. Compute synchronous product of plant and specification
 * 3. Expand alphabet to control patterns for controllability
 * 4. Apply epsilon observation for unobservable events
 * 5. Pseudo-determinize the result to obtain a deterministic supervisor
 * 6. Trim the result to remove unreachable states
 *
 * @param rPlant
 *   Plant model (System with controllable/observable event attributes)
 * @param rSpec
 *   Specification (Rabin automaton)
 * @param rSupervisor
 *   Output supervisor (deterministic Rabin automaton)
 *
 * @exception Exception
 *   - Plant has no controllable events (id 300)
 *   - Plant has no observable events (id 301)
 *   - Empty specification language (id 302)
 *   - Synthesis failed - no valid supervisor exists (id 303)
 *   - Algorithm complexity limits exceeded (id 304)
 */
FAUDES_API void RabinCtrlPartialObs(const System& rPlant, 
                                   const RabinAutomaton& rSpec, 
                                   RabinAutomaton& rSupervisor);

/**
 * @brief Rabin control synthesis under partial observation (with explicit event sets)
 *
 * This function performs supervisory control synthesis for Rabin automata
 * under partial observation with explicitly specified controllable and observable events.
 *
 * @param rPlant
 *   Plant model (Rabin automaton)
 * @param rControllableEvents
 *   Set of controllable events
 * @param rObservableEvents
 *   Set of observable events
 * @param rSpec
 *   Specification (Rabin automaton)
 * @param rSupervisor
 *   Output supervisor (deterministic Rabin automaton)
 *
 * @exception Exception
 *   - Empty controllable event set (id 300)
 *   - Empty observable event set (id 301)
 *   - Controllable events not subset of plant alphabet (id 305)
 *   - Observable events not subset of plant alphabet (id 306)
 *   - Empty specification language (id 302)
 *   - Synthesis failed - no valid supervisor exists (id 303)
 *   - Algorithm complexity limits exceeded (id 304)
 */
FAUDES_API void RabinCtrlPartialObs(const RabinAutomaton& rPlant, 
                                   const EventSet& rControllableEvents,
                                   const EventSet& rObservableEvents,
                                   const RabinAutomaton& rSpec, 
                                   RabinAutomaton& rSupervisor);

/**
 * @brief Check consistency of control problem setup
 *
 * This function validates that the plant, specification, and event sets
 * are consistent for partial observation control synthesis.
 *
 * @param rPlant Plant automaton
 * @param rSpec Specification automaton
 * @param rControllableEvents Set of controllable events
 * @param rObservableEvents Set of observable events
 *
 * @exception Exception
 *   - Alphabets of plant and specification do not match (id 100)
 *   - Controllable events not subset of alphabet (id 305)
 *   - Observable events not subset of alphabet (id 306)
 *   - Plant or specification is nondeterministic (id 201-204)
 */
FAUDES_API void RabinCtrlPartialObsConsistencyCheck(const RabinAutomaton& rPlant,
                                                   const RabinAutomaton& rSpec,
                                                   const EventSet& rControllableEvents,
                                                   const EventSet& rObservableEvents);

/**
 * @brief Extract controllable and observable events from System attributes
 *
 * Helper function to extract event sets from a System with event attributes.
 *
 * @param rSys System with event attributes
 * @param rControllableEvents Output: extracted controllable events
 * @param rObservableEvents Output: extracted observable events
 */
FAUDES_API void ExtractEventAttributes(const System& rSys,
                                      EventSet& rControllableEvents,
                                      EventSet& rObservableEvents);

/**
 * @brief Apply controller to filter transitions and create Buchi automaton
 *
 * This function takes a deterministic Rabin automaton and a controller map,
 * filters out transitions not allowed by the controller, ignores Rabin
 * acceptance conditions, and marks all states to create a Buchi automaton.
 *
 * @param rsDRA Input deterministic Rabin automaton
 * @param rController Controller mapping states to allowed events
 * @param rRes Output Buchi automaton (all states marked)
 */
FAUDES_API void ControlAut(const RabinAutomaton& rsDRA,
                          const TaIndexSet<EventSet>& rController,
                          Generator& rRes);

/**
 * @brief Epsilon observation for Rabin automata
 * 
 * This function performs epsilon observation on a Rabin automaton by replacing
 * all unobservable events with corresponding epsilon events. Each control pattern
 * gets its own epsilon event to maintain the control pattern structure.
 * 
 * @param rGen 
 *   Input Rabin automaton
 * @param rRes
 *   Output Rabin automaton with epsilon events replacing unobservable events
 */
FAUDES_API void EpsObservation(const RabinAutomaton& rGen, RabinAutomaton& rRes);

/**
 * @brief Replace unobservable events with epsilon events for System types
 *
 * This function creates a System where all unobservable events are replaced
 * with a single epsilon event. This is useful for converting systems to a form
 * where unobservable behavior is represented uniformly as epsilon transitions.
 *
 * @param rGen 
 *   Input System
 * @param rRes
 *   Output System with epsilon events replacing unobservable events
 */
FAUDES_API void EpsObservation(const System& rGen, System& rRes);

/**
 * @brief Create muted automaton by removing specified states and their transitions
 *
 * This function creates a copy of the input automaton with specified states 
 * and all their associated transitions removed. This is useful for language
 * inclusion verification algorithms.
 *
 * @param rOriginal 
 *   Input automaton
 * @param rStatesToMute
 *   Set of states to be removed (muted)
 * @return
 *   New automaton with muted states removed
 */
FAUDES_API Generator CreateMutedAutomaton(const Generator& rOriginal, const StateSet& rStatesToMute);

/**
 * @brief Verify language inclusion for Rabin automata
 *
 * This function verifies whether the language of a Büchi automaton is included
 * in the language of a Rabin automaton using a 7-step verification algorithm.
 * It performs SCC analysis and checks Rabin acceptance conditions.
 *
 * @param rGenL
 *   Büchi automaton (generator with all states marked)
 * @param rRabK
 *   Rabin automaton representing the target language
 * @return
 *   True if L(genL) ⊆ L(rabK), false otherwise
 */
FAUDES_API bool RabinLanguageInclusion(const System& rGenL, const RabinAutomaton& rRabK);

} // namespace faudes

#endif // FAUDES_OMG_RABINCTRLPARTIALOBS_H