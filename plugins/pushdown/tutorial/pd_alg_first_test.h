/** @file pd_alg_first_test.h  Unit Tests */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#ifndef FAUDES_PD_ALG_FIRST_TEST_H
#define FAUDES_PD_ALG_FIRST_TEST_H

#include "corefaudes.h"
#include "pd_include.h"
#include "pd_test_util.h"

namespace faudes {

  
/**
 * Test all postfixes of a given word.
 */
void TestPostClPostfixes();

/**
 * Test if Fds returns all expected words.
 */
void TestFdsWords();

/**
 * Test if the mapping function produced by FirstL is correct.
 */
void TestFirstLMap();

/**
 * Test for some selected word if the correct set of terminals is returned.
 */
void TestFirstASelectedWords();

/**
 * Test for some selected word if the correct set of terminals is returned.
 */
void TestFirstSelectedWords();

/**
 * Test FirstRed with a consistent word as input
 */
void TestFirstRedConsistentWord();

/**
 * Test FirstRed with an inconsistent word as input
 */
void TestFirstRedInconsistentWord();

/**
 * Filter a nonterminal from a vector of terminals and nonterminals.
 */
void TestFilter1MixedGrammarSymbols();

/**
 * filter from an empty word
 */
void TestFilter1Nothing();

/**
 * Test with only Terminals as input
 */
void TestFirstAllTerminalsOnly();

/**
 * Test with Nonterminals and a reducible word as input
 */
void TestFirstAllNonterminalsReducible();

/**
 * Test with Nonterminals and an irreducible word as input
 */
void TestFirstAllNonterminalsIrreducible();

/**
 * Test with k = 0 and a word that results in an empty first set
 */
void TestFirstLeq1ZeroEmpty();

/**
 * Test with k = 0 and a word that results in a nonempty first set
 */
void TestFirstLeq1ZeroNonempty();

/**
 * Test with k = 1 and a word that results in a nonempty first set
 */
void TestFirstLeq1OneNonempty();

/**
 * Test with k = 2 and a word that results in a nonempty first set
 */
void TestFirstLeq1TwoNonempty();

/**
 * All PostCl tests
 */
void TestPostCl();

/**
 * All Fds tests
 */
void TestFds();

/**
 * All FirstL tests. Includes testing of First1, because First1 needs to be called
 * recursively in order to be tested correctly. FirstL does nothing else than calling
 * First1 recursively.
 */
void TestFirstL();

/**
 * All FirstA tests
 */
void TestFirstA();

/**
 * All First tests
 */
void TestFirst();

/**
 * All FirstRed tests
 */
void TestFirstRed();

/**
 * All Filter1 tests
 */
void TestFilter1();

/**
 * All FirstAll tests
 */
void TestFirstAll();

/**
 * All FirstLeq1 tests
 */
void TestFirstLeq1();

} // namespace faudes

#endif