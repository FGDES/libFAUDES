#include "libfaudes.h"


// we make the faudes namespace available to our program
using namespace faudes;



int main() {


  // Compose plant dynamics from two very simple machines 
  Generator tempgen, cb1, cb2;
  System cplant; 
  tempgen.Read("data/omg_6_cbplant.gen");
  tempgen.DWrite();
  tempgen.Version("1",cb1);
  tempgen.Version("2",cb2);
  Parallel(cb1,cb2,cplant);
  
  // Declare controllable events
  EventSet contevents;
  contevents.Insert("a_1");
  cplant.SetControllable(contevents);
  
  EventSet allEvents = cplant.Alphabet();
  cplant.ClrObservable(allEvents);

  EventSet obsevents;
  obsevents.Insert("a_1");
  obsevents.Insert("b_2");
  cplant.SetObservable(obsevents);
  // Write to file
  cplant.Name("two belts");
  cplant.Write("tmp_Doublebelts.gen");
  cplant.DWrite();

  // Read spec 
  RabinAutomaton spec;
  spec.Read("data/omg_6_cbspec.gen");
  spec.Name("Specification Belt");

  InvProject(spec,cplant.Alphabet()); 
  spec.DWrite();
  spec.Write("tmp_SpecBelt.gen");

  // Synthesis complete - ready to call RabinCtrlPartialObs

  RabinAutomaton epsObserved;
  RabinCtrlPartialObs(cplant,spec,epsObserved);
  epsObserved.DWrite();
  epsObserved.Write("tmp_ObservedBelt.gen");
  return 0;
}