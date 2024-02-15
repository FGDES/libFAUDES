/** @file pd_alg_lrm_test.h  Unit Tests */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#ifndef FAUDES_PD_ALG_LRM_TEST_H
#define FAUDES_PD_ALG_LRM_TEST_H

#include "corefaudes.h"
#include "pd_include.h"
#include "pd_test_util.h"

namespace faudes {

/**
 * Test Desc11 with a terminal at the beginning of afterDot
 */
void TestDesc11Terminal();

/**
 * Test Desc11 with a nonterminal at the beginning of afterDot
 */
void TestDesc11Nonterminal();

/**
 * Test Desc with some configurations
 */
void TestDescSelectedConfigs();

/**
 * Test PassesX by shifting the dot over a nonterminal.
 */
void TestPassesXNonterminal();

/**
 * Test PassesX by shifting the dot over a terminal.
 */
void TestPassesXTerminal();

/**
 * Test PassesX by trying to shift the dot over nothing or a nonexistent symbol.
 */
void TestPassesXNoShift();

/**
 * Test if Lrm1 finds the correct following configuration set when only one must be
 * found.
 */
void TestLrm1FindOne();

/**
 * Test if Lrm1 finds three following configuration sets when three must be
 * found.
 */
void TestLrm1FindThree();

/**
 * Test if LrmLoop generates the correct number of transitions and states for 
 * the "$ a^n b $" grammar
 */
void TestLrmLoopAnB();

/**
 * Test Desc Initial if it finds the two correct starting configurations
 */
void TestDescInitialFindTwo();

/**
 * Test if Lrm generates a GotoMachine with the correct number of transitions and
 * states for the "$ a^n b $" grammar
 */
void TestLrmGr1();

/**
 * Test if Lrm generates a GotoMachine with the correct number of transitions and
 * states for a specific. //TODO dont know what to expecet as a result
 */
void TestLrmGr2();

/**
 * Augment a grammar which should result in a successful augmentation
 */
void TestAugSuccess();

/**
 * All Desc11 Tests
 */
void TestDesc11();

/**
 * All Desc Tests. Includes Testing Desc1 since Desc1 is just an iterator.
 */
void TestDesc();

/**
 * All PassesX Tests.
 */
void TestPassesX();

/**
 * All Lrm1 Tests.
 */
void TestLrm1();

/**
 * All LrmLoop Tests
 */
void TestLrmLoop();

/**
 * All DescInitial Tests
 */
void TestDescInitial();

/**
 * All Lrm Tests
 */
void TestLrm();

/**
 * All Aug Tests
 */
void TestAug();

} // namespace faudes

#endif