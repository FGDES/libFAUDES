/** @file pd_alg_cc_test.h  Unit Tests */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#ifndef FAUDES_PD_ALG_CC_TEST_H
#define FAUDES_PD_ALG_CC_TEST_H

#include "corefaudes.h"
#include "pd_include.h"
#include "pd_test_util.h"

namespace faudes {

/**
 * Test if Transient chooses the right states
 */
void TestTransientStates();

/**
 * Test if Transient chooses the right states
 */
void TestTransientStatesEmpty();

/**
 * Test if uncontrollable ears are correctly removed. 
 */
void TestRnceRemoveEars();

/**
 * test for intersect events
 */
void TestIntersectEventsAll();  

/**
 * test for the the right number of states and for correctly set merge states
 */
void TestTimesStates();

/**
 * Test if all stack symbols and the stack bottom are set correctly
 */
void TestTimesStackSymbols();

/**
 * Test if the number of lambda transitions and the total number of transitions
 * is right
 */
void TestTimesTransitions();

/**
 * Test if the stack bottom remains unchanged and all stack symbols from the 
 * old generator are in the split generator
 */
void TestSplitStackSymbols();

/**
 * Test if the states have been split correctly into heads and ears
 */
void TestSplitStates();

/**
 * Test if all transitions originating at heads end at ears and if the pop/push 
 * of the transition is correctly set.
 */
void TestSplitTransitionsHeadToEar();

/**
 * Test if all transitions originating at ears end at heads and if the pop 
 * of the transition is correctly set.
 */
void TestSplitTransitionsEarToHead();

/**
 * Test Te with transitions that are unreachable
 */
void TestTsUnreachable();

/**
 * Test if Ac makes the test generator accessible
 */
void TestAcAccessible();

/**
 * All Transient tests
 */
void TestTransient();

/**
 * All Rnce tests
 */
void TestRnce();

/**
 * all Times tests
 */
void TestTimes();

/**
 * all IntersectEvents tests
 */
void TestIntersectEvents();

/**
 * all Split tests
 */
void TestSplit();

/**
 * all Ac tests
 */
void TestAc();

} // namespace faudes

#endif
