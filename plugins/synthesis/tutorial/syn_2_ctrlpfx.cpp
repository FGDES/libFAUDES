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

  // Have uncontrollabe events
  EventSet siguc=sys.UncontrollableEvents();

  // Report to console
  std::cout << "################################\n";
  std::cout << "# generator  \n";
  sys.DWrite();
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

  // record result
  FAUDES_TEST_DUMP("ifac2020 example",cfx);
    
  /////////////////////////////////////////////
  // Demo of implementing SupCon as nu-mu iteration
  // (also along the line of our IFAC 2020 contribution)
  /////////////////////////////////////////////

  // read problem data L and E
  System plant("./data/syn_eleplant.gen");
  Generator spec("./data/syn_elespec.gen");
  Alphabet sigall =  plant.Alphabet() + spec.Alphabet();
  Alphabet sigctrl = plant.ControllableEvents() + (spec.Alphabet()-plant.Alphabet());
  
  // closded-loop candidate: mark L cap E
  Generator sup;
  Automaton(spec,sup);
  sup.StateNamesEnabled(false);
  ParallelLive(plant,sup,sup);

  // take controllability prefix
  std::cout << "################################\n";
  std::cout << "# computing controllability prefix\n"; 
  std::cout << "# state count: " << sup.Size() << std::endl;
  CtrlPfxOperator xcfxop_Y_X(sup,sigall-sigctrl);
  MuIteration xcfxop_Y_muX(xcfxop_Y_X);
  NuIteration xcfxop_nuY_muX(dynamic_cast<StateSetOperator&>(xcfxop_Y_muX));
  StateSet xcfx;
  xcfxop_nuY_muX.Evaluate(xcfx);
  std::cout << "# state count of fixpoint: " << xcfx.Size() << std::endl;

  // derive supremal closed-loop behaviour
  sup.InjectMarkedStates(xcfx);
  SupClosed(sup,sup);
  Trim(sup);
  Parallel(sup,spec,sup);
  Parallel(sup,plant,sup);
  std::cout << "# closed-loop statistics" << std::endl;
  sup.SWrite();

  // validate against std SupCon
  std::cout << "# validate w.r.t. std implementation of SupCon\n";
  Generator ssup;
  InvProject(plant,sigall);
  plant.SetControllable(sigctrl);
  InvProject(spec,sigall);
  ssup.StateNamesEnabled(false);
  SupCon(plant,spec,ssup);
  StateMin(ssup,ssup);
  ssup.SWrite();
  bool eql=LanguageEquality(ssup,sup);
  if(eql)
    std::cout << "# closed-loop behaviours match (tast case pass)\n";
  else    
    std::cout << "# closed-loop behaviours do not match (test case FAIL)\n";
  std::cout << "################################\n";
  std::cout << std::endl;

  // record result
  FAUDES_TEST_DUMP("elesup",sup);
  FAUDES_TEST_DUMP("elessup",ssup);
  FAUDES_TEST_DUMP("validate",eql);

  // validate
  FAUDES_TEST_DIFF();
}

