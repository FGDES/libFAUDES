/** @file hyb_2_abstraction.cpp  

Tutorial, hybrid systems plugin. 
This tutorial demonstrates how to compute a finite abstraction.

@ingroup Tutorials 

@include hyb_3_abstraction.cpp

*/

#include "libfaudes.h"

// make the faudes namespace available to our program
using namespace faudes;


/** Run the tutorial */
int main() {



  /** load from file */
  LinearHybridAutomaton  lha("data/hyb_lhautomaton.gen");

  // Report to console
  std::cout << "################################\n";
  std::cout << "# linear hybrid automaton from file: \n";
  lha.Write();
  std::cout << "################################\n";
  std::cout << "# Valid() returns " << lha.Valid() << "\n";
  std::cout << "################################\n";


  // CompatibleStates
  LhaCompatibleStates* comp= new LhaCompatibleStates(lha);
  comp->InitialiseConstraint();

  // Experiment
  Experiment*  exp = new Experiment(lha.Alphabet());
  exp->InitialStates(comp);

  // Do some refinements by node
  exp->RefineAt(exp->Root());
  exp->RefineAt(2);
  exp->DWrite();
  
  // Do some refinements by event sequence
  std::deque< Idx > seq;
  seq.push_back(lha.EventIndex("north"));
  seq.push_back(lha.EventIndex("south"));
  seq.push_back(lha.EventIndex("north"));
  seq.push_back(lha.EventIndex("south"));
  seq.push_back(lha.EventIndex("north"));
  seq.push_back(lha.EventIndex("south"));
  seq.push_back(lha.EventIndex("north"));
  seq.push_back(lha.EventIndex("south"));
  exp->RefineSequence(seq);
  exp->DWrite();

  // Do l-complete abstraction from scratch (time variant)
  comp = new LhaCompatibleStates(lha);
  comp->InitialiseConstraint();
  exp = new Experiment(lha.Alphabet());
  exp->InitialStates(comp);
  LbdAbstraction tvabs;
  tvabs.Experiment(exp);
  tvabs.RefineUniformly(7);
  tvabs.TvAbstraction().GraphWrite("tmp_abs3tv.png");
 
  // Do l-complete abstraction from scratch (time invariant)
  comp = new LhaCompatibleStates(lha);
  comp->InitialiseFull();
  exp = new Experiment(lha.Alphabet());
  exp->InitialStates(comp);
  LbdAbstraction tivabs;
  tivabs.Experiment(exp);
  tivabs.RefineUniformly(7);
  tivabs.TivAbstraction().GraphWrite("tmp_abs3tiv.png");
 
  // compose both for finest result
  Generator result;
  result.StateNamesEnabled(false);
  Product(tvabs.TvAbstraction(),tivabs.TivAbstraction(),result);
  StateMin(result,result);
  result.GraphWrite("tmp_abs3.png");
  
  // done
  return 0;
}



