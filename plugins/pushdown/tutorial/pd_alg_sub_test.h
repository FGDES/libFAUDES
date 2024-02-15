/** @file pd_alg_nb_sub_a_test.h  Unit Tests */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#ifndef FAUDES_PD_ALG_NB_SUB_A_TEST_H
#define FAUDES_PD_ALG_NB_SUB_A_TEST_H

#include "corefaudes.h"
#include "pd_include.h"
#include "pd_test_util.h"

namespace faudes {


/**
 * Test if all states were renamed correctly (merge attribute is set correctly).
 */
void TestRenameStatesRenaming();

/**
 * Test if the number of states did not change.
 */
void TestRenameStatesNumberOfStates();

/**
 * Test if all stack symbols were renamed correctly
 */
void TestRenameStackSymbolsRenamingStackSymbols();

/**
 * Test if the number of stack symbols did not change.
 */
void TestRenameStackSymbolsNumberOfStackSymbols();

/**
 * Test if the stack symbols in pop and push of transitions were changed correctly
 */
void TestRenameStackSymbolsRenamingTransitions();

/**
 * Tests if there are any lambda popping edges left
 */
void TestRep0NoLambdaPop();

/**
 * Test if resulting generator has a transition able to pop all stack symbols,
 * but only if the orignal generator had at least one lambda transition
 */
void TestRep0AllExpectingTransition();

/**
 * Test for the right number of transitions and states in the generator after
 * applying Rep2
 */
void TestRemoveMulPopNumberOfStatesTransitions();


/**
 * Test if all transitions are either read only, pop only or push only
 */
void TestSPDAReadPopPushOnly();

/**
 * Test if the number of transitions and states is correct after calling Rpp
 */
void TestSPDANumberStatesTransitions();

/**
 * Test if the states are split into active and passive states and if the number of
 * states has been doubled.
 */
void TestNdaActivePassive();

/**
 * Test if transition start and end states are set as expected
 */
void TestNdaTransitions();

/**
 * All AnnotatePdStates Tests
 */
void TestAnnotatePdStates();

/**
 * All RenG tests.
 */
void TestRenameStackSymbols();

/**
 * All RemoveLambdaPop tests
 */
void TestRemoveLambdaPop();

/**
 * All SPDA tests
 */
void TestSPDA();

/**
 * All RemoveMultPop tests
 */
void TestRemoveMultPop();

/**
 * All Nda tests
 */
void TestNda();


/**
 * Filter a nonterminal from a vector of terminals and nonterminals.
 */
void TestFilterMixedGrammarSymbols();

/**
 * filter from an empty word
 */
void TestFilterNothing();

/**
 * Find an eliminable symbol with Rnpp1 and an empty set of nonterminals
 */
void TestRnpp1FindSymbolsEmptySet();

/**
 * Find an eliminable symbol with Rnpp1 and a nonempty set of nonterminals
 */
void TestRnpp1FindSymbolsNonemptySet();

/**
 * Find all eliminable symbols with Rnppl and an empty set of nonterminals
 */
void TestRnpplFindSymbolsEmptySet();

/**
 * Find all eliminable symbolswith Rnppl and a nonempty set of nonterminals
 */
void TestRnpplFindSymbolsNonemptySet();

/**
 * Find all eliminable symbols with Rnppl and a set already containing all
 * eliminable symbols
 */
void TestRnpplFindSymbolsCompleteSet();

/**
 * Remove all noneliminable nonterminals and all productions containing
 * noneliminable nonterminals
 */
void TestRnppGrammar1();

/**
 * Remove all noneliminable nonterminals and all productions containing
 * noneliminable nonterminals
 */
void TestRnppGrammar2();

/**
 * Try to remove all noneliminable nonterminals and all productions containing
 * noneliminable nonterminals from an empty grammar.
 */
void TestRnppEmptyGrammar();

/**
 * Test if all terminals have been set correctly and match the generator's events.
 */ 
void TestSp2LrTerminals();

/**
 * Test if all possible nonterminals were generated.
 */
void TestSp2LrNonterminals();

/**
 * Test if all expected productions for read, pop and push transitions and for the final 
 * states were generated.
 */
void TestSp2LrProductions();

/**
 * 
 */
void TestRupProductions();

/**
 * 
 */
void TestRupNonterminals();

/**
 * All Filter tests
 */
void TestFilter();

/**
 * All Rnpp1 tests
 */
void TestRnpp1();

/**
 * All Rnppl tests
 */
void TestRnppl();

/**
 * All Rnpp tests
 */
void TestRnpp();

/**
 * All Sp2Lr tests
 */
void TestSp2Lr();


/**
 * All Rup tests
 */
void TestRup();

} // namespace faudes

#endif
