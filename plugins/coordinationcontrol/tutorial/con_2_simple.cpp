/** @file con_2_simple.cpp

Tutorial, coordination control for prefix-closed languages

@ingroup Tutorials 

@include con_2_simple.cpp

*/

#include "libfaudes.h"

// we make the faudes namespace available to our program
using namespace faudes;

/////////////////
// main program
/////////////////

int main(void) {

  // Local systems g1, g2, g3, and a global specification spec
  System g1, g2, g3, spec;
  // Set of controllable events and the initial ek event set
  EventSet ec, ek;
  // Vectors of local systems and resulting supervisors
  GeneratorVector gVector, resVector;
  // Computer supervised coordinator
  System coord;

  g1.Read("data/gen1.gen");
  g2.Read("data/gen2.gen");
  g3.Read("data/gen3.gen");
  spec.Read("data/spec.gen");
  ec.Read("data/ec.alph"); 
  ek.Read("data/ek0.alph"); 
  
  gVector.Append(g1);
  gVector.Append(g2);
  gVector.Append(g3);

  bool result = SupConditionalControllable(spec,gVector,ec,ek,resVector,coord);

  FAUDES_TEST_DUMP("computed ok?",result);

  FAUDES_TEST_DIFF();
}

