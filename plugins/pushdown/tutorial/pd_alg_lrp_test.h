/** @file pd_alg_lrp_test.h  Unit Tests */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#ifndef FAUDES_PD_ALG_LRP_TEST_H
#define FAUDES_PD_ALG_LRP_TEST_H

#include "corefaudes.h"
#include "pd_include.h"
#include "pd_test_util.h"

namespace faudes {

/**
 * Test if GeneratorGoto finds the right successor.
 */
void TestGeneratorGotoOneSuccessor();

/**
 * Test if GeneratorGoto finds no successor when none is to be found.
 */
void TestGeneratorGotoNoSuccessor();

/**
 * Test if a word fully matches a sequence of states when a match is possible
 */
void TestGeneratorGotoSeqFull();

/**
 * Test if a partial state sequence is generated when no full match for the word
 * is possible.
 */
void TestGeneratorGotoSeqPartial();

/**
 * Test if the number of shift rules is correct and test for the correct number of 
 * occurences of terminals and each action's left-hand side element's right-hand side.
 */
void TestLrpShiftRulesNumberAndTerminals();

/**
 * Test if the number of reduce rules is correct and test for the correct number of 
 * occurences of terminals and each action element's right-hand side.
 */
void TestLrpReduceRulesNumberAndTerminals();

/**
 * Test if Lrp generates a parser with the correct rules.
 */
void TestLrpActions();


/**
 * Test if Lrp generates a parser with the correct number of nonterminals and terminals
 * for a certain grammar
 */
void TestLrpNonterminalsTerminals();

/**
 * Test if all states are set correctly and whether the correct ones are used in
 * transitions.
 */
void TestLrParser2EPDAStates();

/**
 * Test if the number of transitions is correct and test if the number of transitions
 * per event is as expected.
 */
void TestLrParser2EPDATransitions();

/**
 * Test if the augment symbol transitions were successfully removed from the generator
 * and the new number of transitions is correct.
 */
void TestDimNoAugment();

/**
 * Test if final states are set correctly after the removal auf the augment symbol
 */
void TestDimNewFinalStates();
  
/**
 * All GeneratorGoto Tests
 */
void TestGeneratorGoto();

/**
 * All GeneratorGotoSeq Tests
 */
void TestGeneratorGotoSeq();

/**
 * All LrpShiftRules Tests
 */
void TestLrpShiftRules();

/**
 * All LrpReduceRules Tests
 */
void TestLrpReduceRules();

/**
 * All Lrp Tests
 */
void TestLrp();

/**
 * All LrParser2EPDA Tests
 */
void TestLrParser2EPDA();

/**
 * All Dim Tests
 */
void TestDim();

} // namespace faudes

#endif
