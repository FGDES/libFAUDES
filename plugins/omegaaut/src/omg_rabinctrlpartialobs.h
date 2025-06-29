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
#include "omg_controlpattern.h"
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

} // namespace faudes

#endif // FAUDES_OMG_RABINCTRLPARTIALOBS_H