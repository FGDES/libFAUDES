#include "libfaudes.h"
#undef FD_DF

// we make the faudes namespace available to our program
using namespace faudes;



int main() {


  // Compose plant dynamics from two very simple machines 
  System plant; 
  plant.Read("data/omg_6_cbplant.gen");

  // Declare controllable events
  
  EventSet allEvents = plant.Alphabet();
  plant.ClrObservable(allEvents);

  // Read spec 
  RabinAutomaton spec;
  spec.Read("data/omg_6_cbspec1.gen");
  InvProject(spec,plant.Alphabet()); 

  // Synthesis complete - ready to call RabinCtrlPartialObs
  
  EventSet contevents;
  contevents.Insert("a_1");
  contevents.Insert("b_2");
  

  EventSet obsevents;
  obsevents.Insert("a_1");
  obsevents.Insert("b_2");

  /*
  RabinAutomaton Product;
  RabinBuechiProduct(spec,plant,Product);
  Product.SetControllable(contevents);

  EventSet allProductEvents = Product.Alphabet();
  Product.ClrObservable(allProductEvents);
  Product.SetObservable(obsevents);

  Product.Name("Product of Plant and Spec");
  Product.Write("tmp_6_ProductBelt.gen");

  RabinAutomaton NRA;
  EpsObservation(Product, NRA);
  NRA.Name("Epsilon Observed Product");
  NRA.Write("tmp_EpsObservedBelt.gen");
  NRA.DWrite();
  
  RabinAutomaton epsObserved;
  PseudoDet(NRA, epsObserved);
  epsObserved.Name("Determinized Epsilon Observed Product");
  epsObserved.DWrite();
  epsObserved.Write("tmp_DetEpsObservedBelt.gen");
  epsObserved.GraphWrite("tmp_DetEpsObservedBelt.png");
  */

  

  RabinAutomaton epsObserved;
  RabinCtrlPartialObs(spec,contevents,obsevents,plant,epsObserved);
  epsObserved.DWrite();
  epsObserved.Write("tmp_6_ObservedBelt.gen");
  epsObserved.GraphWrite("tmp_6_ObservedBelt.png");


  StateSet ctrlpfx;
  RabinCtrlPfx(epsObserved,contevents,ctrlpfx);
  epsObserved.WriteStateSet(ctrlpfx);
  epsObserved.RestrictStates(ctrlpfx);
  // SupClosed(cand)
  epsObserved.RabinAcceptanceWrite();
  epsObserved.GraphWrite("tmp_6_rabinctrlDB.png");
  Generator test=epsObserved;
  test.StateNamesEnabled(false);
  test.InjectMarkedStates(epsObserved.RabinAcceptance().Begin()->RSet());
  StateMin(test,test);
  test.GraphWrite("tmp_6_consubsetDB_test.png");

  return 0;
}
