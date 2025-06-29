/** @file omg_5_rabindet.cpp
 *
 * Example usage of pseudo-determinization algorithm for Rabin automata
 *
 * This example demonstrates how to use the PseudoDet function to convert
 * nondeterministic Rabin automata to deterministic ones.
 *
 * @ingroup Tutorials
 * @include omg_5_rabindet.cpp
 */

 #include "libfaudes.h"
 
 using namespace faudes;

 
 int main(void) {
     
     std::cout << "======== Pseudo-Determinization Example ========" << std::endl;

         // Read nondeterministic Rabin automaton from file
         RabinAutomaton NRA;
         NRA.Read("pseudotest/test5.gen");
         
         NRA.DWrite();
         //NRA.RabinGraphWrite("NRA.png");
         // Apply pseudo-determinization algorithm

         RabinAutomaton DRA;
         PseudoDet(NRA, DRA);

         DRA.DWrite();
         //DRA.RabinGraphWrite("DRA.png");

	 // record test case
         FAUDES_TEST_DUMP("pseudo det result", DRA);

	 // compare with our records
         FAUDES_TEST_DIFF();

         return 0;
     }
