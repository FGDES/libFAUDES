/** @file hyb_2_reachability.cpp  

Tutorial, hybrid systems plugin. 
This tutorial demonstrates reachability analysis on linear hybrid automata.

@ingroup Tutorials 

@include hyb_2_reachability.cpp

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

  // get initial state
  HybridStateSet istates;
  StateSet::Iterator qit=lha.InitStatesBegin();
  StateSet::Iterator qit_end=lha.InitStatesEnd();
  for(;qit!=qit_end;++qit){
    Polyhedron* poly = new Polyhedron(lha.InitialConstraint(*qit));
    PolyIntersection(lha.StateSpace(),*poly);
    PolyFinalise(*poly);
    istates.Insert(*qit,poly);
  }
  std::cout << "################################\n";
  std::cout << "# dumping initial states\n";
  istates.DWrite(lha);    
  std::cout << "################################\n";


  // loop maxmimal 50 jumps with alternating north/south events
  for(int i=1; i<=50; ++ i) {

    // compute reachable states per event
    std::map< Idx, HybridStateSet* > sstates;
    LhaReach(lha,istates,sstates); 

    // dump successor states
    std::cout << "################################\n";
    std::cout << "# dumping successor stage " << i << "\n";
    std::map< Idx, HybridStateSet* >::iterator sit=sstates.begin();
    std::map< Idx, HybridStateSet* >::iterator sit_end=sstates.end();
    for(;sit!=sit_end;++sit){
      std::cout << "### event " << lha.EventName(sit->first) << "\n";
      sit->second->DWrite(lha);
    }
    std::cout << "################################\n";

    // choose next event to track
    std::string evstr;
    if(i % 2 ==1)  evstr="north";
    else evstr="south";
    Idx ev = lha.EventIndex(evstr);
    if(sstates.find(ev)==sstates.end()) break;
    istates.Assign(*sstates[ev]);    
  }

  // done
  return 0;
}



