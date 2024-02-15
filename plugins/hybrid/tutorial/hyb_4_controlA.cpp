/** @file hyb_4_control.cpp  

Tutorial, hybrid systems plugin. This tutorial demonstrates
how to compute a finite abstraction for the purpose of
control.

@ingroup Tutorials 

@include hyb_4_Control.cpp

*/

#include "libfaudes.h"

// make the faudes namespace available to our program
using namespace faudes;


/** Run the tutorial */
int main() {



  /** load from file */
  LinearHybridAutomaton  lha("data/hyb_lhioautomaton.gen");

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

  /*

  // compute reachable states per event
  std::map< Idx, HybridStateSet > ustates;
  LhaReach(lha,istates,ustates); 

  // dump successor states
  std::cout << "################################\n";
  std::cout << "# dumping successor u-states\n";
  std::map< Idx, HybridStateSet >::iterator uit=ustates.begin();
  std::map< Idx, HybridStateSet >::iterator uit_end=ustates.end();
  for(;uit!=uit_end;++uit){
    std::cout << "### event " << lha.EventName(uit->first) << "\n";
    uit->second.DWrite(lha);
  }
  std::cout << "################################\n";

  // compute reachable states per event
  std::map< Idx, HybridStateSet > ystates;
  istates.Assign(ustates[lha.EventIndex("u_north_west")]);
  LhaReach(lha,istates,ystates); 

  // dump successor states
  std::cout << "################################\n";
  std::cout << "# dumping successor y-states\n";
  std::map< Idx, HybridStateSet >::iterator yit=ystates.begin();
  std::map< Idx, HybridStateSet >::iterator yit_end=ystates.end();
  for(;yit!=yit_end;++yit){
    std::cout << "### event " << lha.EventName(yit->first) << "\n";
    yit->second.DWrite(lha);  
  }
  std::cout << "################################\n";

  // compute reachable states per event
  istates.Assign(ystates[lha.EventIndex("y_north")]);
  LhaReach(lha,istates,ustates); 

  // dump successor states
  std::cout << "################################\n";
  std::cout << "# dumping successor u-states\n";
  uit=ustates.begin();
  uit_end=ustates.end();
  for(;uit!=uit_end;++uit){
    std::cout << "### event " << lha.EventName(uit->first) << "\n";
    uit->second.DWrite(lha);
  }
  std::cout << "################################\n";


  // compute reachable states per event
  istates.Assign(ustates[lha.EventIndex("u_south_west")]);
  LhaReach(lha,istates,ystates); 

  // dump successor states
  std::cout << "################################\n";
  std::cout << "# dumping successor y-states\n";
  yit=ystates.begin();
  yit_end=ystates.end();
  for(;yit!=yit_end;++yit){
    std::cout << "### event " << lha.EventName(yit->first) << "\n";
    yit->second.DWrite(lha);
  }
  std::cout << "################################\n";
  */
  // done
  return 0;
}



