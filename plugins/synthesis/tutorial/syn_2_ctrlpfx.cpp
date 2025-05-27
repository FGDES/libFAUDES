/** @file syn_2_ctrlpfx.cpp

Tutorial, controllability prefix 


@ingroup Tutorials 

@include syn_2_ctrlpfx.cpp

*/

#include "libfaudes.h"


// we make the faudes namespace available to our program
using namespace faudes;




/////////////////
// main program
/////////////////

int main() {

  /////////////////////////////////////////////
  // Demo of mu/nu fixpoint ieterations
  /////////////////////////////////////////////
  

  // Read example data from file
  System sys("data/syn_2_ctrlpfx.gen");

  // Have unconllabe events
  EventSet siguc=sys.UncontrollableEvents();

  // Report to console
  std::cout << "################################\n";
  std::cout << "# generator  \n";
  sys.DWrite();
  siguc.DWrite();
  std::cout << "################################\n";

  //Instantiate my operator
  CtrlPfxOperator cfxop_Y_X(sys,siguc);

  // Have the inner mu-iteration
  MuIteration cfxop_Y_muX(cfxop_Y_X);

  // Have the outer nu--iteration
  NuIteration cfxop_nuY_muX(dynamic_cast<StateSetOperator&>(cfxop_Y_muX));
  
  // run
  std::cout << "nu-mu nested fixpoint operator: " << cfxop_Y_muX.Name() << std::endl;
  StateSet cfx;
  StateSetVector args;
  cfxop_nuY_muX.Evaluate(args,cfx);

  return 0;
}

