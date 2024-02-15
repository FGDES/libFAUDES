/** @file ios_2_synthesis.cpp  

Tutorial, I/O system synthesis

This tutorial runs the two examples from 
the techical report Moor/Schmidt/Wittmann, 2010.
It thereby demonstrates the use of the provided
functions for the synthesis of I/O controllers.

@ingroup Tutorials 

@include ios_2_synthesis.cpp

*/


#include "libfaudes.h"


// make the faudes namespace available to our program
using namespace faudes;



/////////////////
// main program
/////////////////

int main() {

  // Read plant and speification generators
  IoSystem planta("data/ios_planta.gen");
  Generator speca("data/ios_speca.gen");

  // Adjust specification alphabet
  InvProject(speca,planta.Alphabet()); 
  
  // Run synthesis algorithm
  IoSystem supera;
  supera.StateNamesEnabled(false);
  IoSynthesis(planta,speca,supera);
  aStateMin(supera,supera);

  // Report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, supervisor a \n";
  supera.DWrite();
  std::cout << "################################\n";


  // Read plant and speification generators
  IoSystem plantb("data/ios_plantb.gen");
  Generator specb("data/ios_specb.gen");

  // Adjust specification alphabet
  InvProject(specb,plantb.Alphabet()); 
  
  // Run synthesis algorithm
  IoSystem superb;
  superb.StateNamesEnabled(false);
  IoSynthesisNB(plantb,specb,superb);
  aStateMin(superb,superb);

  // Report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, supervisor b \n";
  superb.DWrite();
  std::cout << "################################\n";

  // Record testcase
  FAUDES_TEST_DUMP("supA",supera);
  FAUDES_TEST_DUMP("supB",superb);

  // Done
  return 0;
}



