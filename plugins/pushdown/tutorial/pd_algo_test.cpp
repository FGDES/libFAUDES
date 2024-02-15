/** @file pd_algo_test.cpp  

Test, pushdown plugin. Run this file to test functionality of the synthesis algorithm.

@ingroup Pushdown

@include pd_algo_test.cpp

*/

/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "libfaudes.h"
// ndont need this, automatica with libfaudes.h
// #include "pd_include.h"
#include "pd_alg_sub_test.h"
#include "pd_alg_first_test.h"
#include "pd_alg_lrp_test.h"
#include "pd_alg_lrm_test.h"
#include "pd_alg_main_test.h"
#include "pd_test_util.h"

// make the faudes namespace available to our program
using namespace faudes;


/**
 * visual only test for LangK
 */
void TestLangK(){
  
  PushdownGenerator g1 = TestGenerator1();
  LangK lk(g1);
  lk.FindLangK(6);
}

/**
 * All Tests
 */
void Tests(){
  TestAnnotatePdStates();
  TestRenameStackSymbols();
  TestRemoveLambdaPop();
  TestFilter();
  TestFilter1();
  TestRnpp1();
  TestRnppl();
  TestRnpp();
  TestTimes();
  TestSplit();
  TestSPDA();
  TestRemoveMultPop();
  TestTransient();
  TestRnce();
  TestNda();
  TestSp2Lr();
  TestRup();
  TestPostCl();
  TestFds();
  TestFirstL();
  TestFirstA();
  TestFirst();
  TestFirstRed();
  TestFirstAll();
  TestFirstLeq1();
  TestDesc11();
  TestDesc();
  TestPassesX();
  TestLrm1();
  TestLrmLoop();
  TestDescInitial();
  TestLrm();
  TestGeneratorGoto();
  TestGeneratorGotoSeq();
  TestAug();
  TestLrpShiftRules();
  TestLrpReduceRules();
  TestLrp();
  TestLrParser2EPDA();
  TestDim();
  TestAc();
//  //TestLangK();
}


/** Run the tests */
int main() {

  Tests();

  
  return 0;
}










