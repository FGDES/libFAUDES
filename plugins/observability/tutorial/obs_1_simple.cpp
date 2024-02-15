/** @file syn_1_simple.cpp

Tutorial, std monolitic synthesis. 


This tutorial uses a very simple example to illustrate the 
std monolithic controller synthesis
as originally proposed by Ramadge and Wonham.


@ingroup Tutorials 

@include syn_1_simple.cpp

*/

#include "libfaudes.h"


// we make the faudes namespace available to our program
using namespace faudes;




/////////////////
// main program
/////////////////

int main() {

  System plant, spec;
  EventSet highAlph, obsAlph;
  // The first plant/spec violates the condition
  plant.Read("data/generator1.gen");
  plant.GraphWrite("data/generator1.png");
  spec.Read("data/generator1.gen");
  highAlph.Read("data/highAlph1.alph");
  obsAlph.Read("data/obs1.alph");
  bool result = LocalObservationConsistency(plant,spec,highAlph,obsAlph);
  std::cout << "result for generator1 is: " << result << std::endl;
  // The second plant/spec fulfills the condition
  plant.Read("data/generator2.gen");
  plant.GraphWrite("data/generator2.png");
  spec.Read("data/generator2.gen");
  result = LocalObservationConsistency(plant,spec,highAlph,obsAlph);
  std::cout << "result for generator2 is: " << result << std::endl;

  return 0;
}

