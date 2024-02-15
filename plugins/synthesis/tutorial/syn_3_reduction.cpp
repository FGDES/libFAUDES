/** @file syn_3_reduction.cpp

Tutorial, std monolitic synthesis. 


This tutorial uses two simple examples to illustrate the 
application of supervisor reduction as initially proposed 
by Su and Wonham


@ingroup Tutorials 

@include syn_3_reduction.cpp

*/

#include "libfaudes.h"


// we make the faudes namespace available to our program
using namespace faudes;




/////////////////
// main program
/////////////////

int main() {

  // Needed generators
  System plant1, sup1, plant2, sup2, reduced1, reduced2; 
 
  // First example for supervisor reduction
  plant1.Read("data/plant1.gen");
  plant1.Write("tmp_syn_3_plant1.gen");
  sup1.Read("data/sup1.gen");
  sup1.Write("tmp_syn_3_sup1.gen");
  bool success = SupReduce(plant1,sup1,reduced1);
  reduced2.Write("tmp_syn_3_reduced1.gen");
  // Report to console
  std::cout << "##############################################\n";
  std::cout << "# tutorial, supervisor reduction successful: " << success << std::endl;
  reduced1.DWrite();
  std::cout << "##############################################\n";

  // Second example for supervisor reduction
  plant2.Read("data/plant2.gen");
  plant2.Write("tmp_syn_3_plant2.gen");
  sup2.Read("data/sup2.gen");
  sup2.Write("tmp_syn_3_sup2.gen");
  success = SupReduce(plant2,sup2,reduced2);
  reduced2.Write("tmp_syn_3_reduced2.gen");
  // Report to console
  std::cout << "##############################################\n";
  std::cout << "# tutorial, supervisor reduction successful: " << success << std::endl;
  reduced2.DWrite();
  std::cout << "##############################################\n";
  return 0;
}

