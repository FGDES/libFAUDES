#include "libfaudes.h"
#undef FD_DF

// we make the faudes namespace available to our program
using namespace faudes;



int main() {

  // Test case selection
  std::cout << "====== Partial Observable Rabin Control ======" << std::endl;

  // Compose belt1 dynamics from two very simple machines 
  Generator belt1; 
  belt1.Read("data/omg_6_belt1.gen");
  belt1.Write("tmp_omg_6_belt1.gen");
  Generator belt2; 
  belt2.Read("data/omg_6_belt2.gen");
  belt2.Write("tmp_omg_6_belt2.gen");
  System plant;
  Parallel(belt1,belt2,plant);
  plant.DWrite();
  plant.Write("tmp_omg_6_plant.gen");
  
  // Declare controllable and observable events
  EventSet allEvents = plant.Alphabet();
  plant.ClrObservable(allEvents);

  // Read spec 
  RabinAutomaton spec;
  spec.Read("data/omg_6_cbspec12.gen");
  InvProject(spec,plant.Alphabet());
  spec.Write("tmp_omg_6_spec.gen");


  // Synthesis complete - ready to call RabinCtrlPartialObs
  
  EventSet contevents;
  contevents.Insert("a");
  //contevents.Insert("d");
  contevents.Insert("h");
  
  EventSet obsevents;
  obsevents.Insert("a");
  //obsevents.Insert("d");       
  obsevents.Insert("h"); 

  plant.SetControllable(contevents);
  EventSet plantEvents = plant.Alphabet();
  plant.ClrObservable(plantEvents);
  plant.SetObservable(obsevents);


  RabinAutomaton ProductPS;
  RabinBuechiProduct(spec,plant,ProductPS);
  ProductPS.SetControllable(contevents);
  EventSet ProductEvents = ProductPS.Alphabet();
  ProductPS.ClrObservable(ProductEvents);
  ProductPS.SetObservable(obsevents);

  ProductPS.Name("Product of plant and Spec");
  ProductPS.Write("tmp_omg_6_Product.gen");
  ProductPS.DWrite();

  // record test case
  FAUDES_TEST_DUMP("plant", plant);
  FAUDES_TEST_DUMP("spec", spec);
  FAUDES_TEST_DUMP("cand", ProductPS);
  

  RabinAutomaton NRA;
  EpsObservation(ProductPS, NRA);
  NRA.Name("Epsilon Observed Product");
  NRA.Write("tmp_omg_6_EpsObservedProduct.gen");
  NRA.DWrite();
  RabinAutomaton DRA;
  PseudoDet(NRA, DRA);
  DRA.Name("Determinized Epsilon Observed Product");
  DRA.DWrite();
  DRA.Write("tmp_omg_6_DRA.gen");

  // record test case
  FAUDES_TEST_DUMP("eps cand", NRA);
  FAUDES_TEST_DUMP("det cand", DRA);


  // // Standard partial observable control synthesis
  // // std::cout << "====== Standard Synthesis ======" << std::endl;
  // // RabinAutomaton epsObserved;
  // // RabinCtrlPartialObs(spec,contevents,obsevents,belt1,epsObserved);
  // // epsObserved.DWrite();
  // // epsObserved.Write("tmp_omg_6_ObservedBelt.gen");
  // // epsObserved.GraphWrite("tmp_omg_6_ObservedBelt.png");
  
  // Remove self-loof eps transitions and relink the state only with eps transition
  RabinAutomaton relinkDRA;
  RemoveEps(DRA, relinkDRA);
  relinkDRA.DWrite();
  relinkDRA.Write("tmp_omg_6_relinkDRA.gen");
  
  TaIndexSet<EventSet> controller;
  RabinCtrlPfx(relinkDRA,contevents,controller);
  relinkDRA.WriteStateSet(controller);
  std::cout << std::endl;

  // record
  FAUDES_TEST_DUMP("controller", controller);

  // Test the new CloseLoopSpecification function
  System CLspec;
  ControlAut(relinkDRA,controller,CLspec);
  CLspec.DWrite();
  CLspec.Write("tmp_omg_6_Controller.gen");

  // Check if the closed-loop controller is deterministic
  bool cldet= CLspec.IsDeterministic();
  if (cldet) {
      std::cout << "CloseLoopController is Deterministic" << std::endl;
  } else {
      std::cout << "CloseLoopController is NOT Deterministic (testcase FAILED)" << std::endl;
  }

  // ====== Controllability Verification: IsControllable(Plant, Controller) ======
  System CLspecProjected = CLspec;
  InvProject(CLspecProjected, plant.Alphabet());
  
  bool controllable = IsControllable(plant, CLspecProjected);
  
  if (controllable) {
      std::cout << "CONTROLLABILITY VERIFIED: Controller is controllable w.r.t. Plant" << std::endl;
  } else {
      std::cout << "CONTROLLABILITY FAILED: Controller is NOT controllable w.r.t. Plant" << std::endl;
  }

  // ====== Language Inclusion Verification: L(V/G) ⊆ L(E) ======

  CLspec.Name("Controller (V)");
  plant.Name("Plant (G)");
  spec.Name("Specification (E)");
  
  // Compute V/G (supervised system) 
  System supervisedSystem;
  InvProject(CLspec, plant.Alphabet());
  Product(CLspec, plant, supervisedSystem);
  supervisedSystem.Name("V/G (Supervised System)");
  supervisedSystem.Write("tmp_omg_6_supervisedSys.gen");

  // Perform language inclusion verification: L(V/G) ⊆ L(E)
  bool result = RabinLanguageInclusion(supervisedSystem, spec);
  
  if (result) {
      std::cout << "VERIFICATION SUCCESSFUL: L(V/G) ⊆ L(E)" << std::endl;
      std::cout << "The supervised system satisfies the specification." << std::endl;
  } else {
      std::cout << "VERIFICATION FAILED: L(V/G) ⊄ L(E)" << std::endl;
      std::cout << "The supervised system does not satisfy the specification." << std::endl;
  }


  // graphical output
  if(DotReady()) {
    belt2.GraphWrite("tmp_omg_6_belt2.png");
    belt1.GraphWrite("tmp_omg_6_belt1.png");
    spec.GraphWrite("tmp_omg_6_spec.png");
    DRA.GraphWrite("tmp_omg_6_DRA.png");
    NRA.GraphWrite("tmp_omg_6_EpsObservedProduct.png");
    ProductPS.GraphWrite("tmp_omg_6_Product.png");
    relinkDRA.GraphWrite("tmp_omg_6_relinkDRA.png");
    CLspec.GraphWrite("tmp_omg_6_Controller.png");
  }
  
  // record
  FAUDES_TEST_DUMP("validate controlability", controllable);
  FAUDES_TEST_DUMP("validate determinism", cldet);
  FAUDES_TEST_DUMP("validate languageinclusion", result);

  // validate
  FAUDES_TEST_DIFF()

}
