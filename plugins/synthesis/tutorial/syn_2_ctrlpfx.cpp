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
  std::cout << std::endl;

  //Instantiate my operator
  CtrlPfxOperator cfxop_Y_X(sys,siguc);

  // Have the inner mu-iteration
  MuIteration cfxop_Y_muX(cfxop_Y_X);

  // Have the outer nu--iteration
  NuIteration cfxop_nuY_muX(dynamic_cast<StateSetOperator&>(cfxop_Y_muX));
  
  // run
  std::cout << "################################\n";
  std::cout << "# running nu-mu nested fixpoint iteration: " << cfxop_Y_muX.Name() << std::endl;
  StateSet cfx;
  cfxop_nuY_muX.Evaluate(cfx);
  std::cout << "# resulting fixpoint: " << std::endl;
  cfx.Write();
  std::cout << "################################\n";
  std::cout << std::endl;
  

  return 0;
}

