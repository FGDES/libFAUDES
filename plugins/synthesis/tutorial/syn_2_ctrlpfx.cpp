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
  // Demo of mu/nu fixpoint iterations
  // (this is the example used in out IFAC2020 contribution)
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

  
  /////////////////////////////////////////////
  // Demo of implementing SupCon as nu-mu iteration
  // (also along the line of our IFAC 2020 contribution)
  /////////////////////////////////////////////

  // read problem data L and E
  System plant("./data/syn_eleplant.gen");
  Generator spec("./data/syn_elespec.gen");

  // first closded-loop candidate: mark L cap E
  Generator sup;
  Automaton(spec,sup);
  ParallelLive(plant,sup,sup);

  // take controllability prefix
  std::cout << "################################\n";
  CtrlPfxOperator xcfxop_Y_X(sup,plant.UncontrollableEvents());
  MuIteration xcfxop_Y_muX(xcfxop_Y_X);
  NuIteration xcfxop_nuY_muX(dynamic_cast<StateSetOperator&>(xcfxop_Y_muX));
  StateSet xcfx;
  xcfxop_nuY_muX.Evaluate(cfx);
  std::cout << "# resulting fixpoint: " << std::endl;
  xcfx.Write();
  std::cout << "################################\n";
  std::cout << std::endl;

  sup.RestrictStates(cfx);
  sup.GraphWrite("tmp_elesup.png");
  sup.SWrite();

  
  return 0;
}

