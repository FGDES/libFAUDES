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
  belt1.Write("tmp_6_belt1.gen");
  belt1.GraphWrite("tmp_6_belt1.png");
  Generator belt2; 
  belt2.Read("data/omg_6_belt2.gen");
  belt2.Write("tmp_6_belt2.gen");
  belt2.GraphWrite("tmp_6_belt2.png");

  System plant;
  Parallel(belt1,belt2,plant);
  plant.DWrite();
  plant.Write("tmp_6_plant.gen");
  //plant.GraphWrite("tmp_6_plant.png");
  // Declare controllable and observable events
  EventSet allEvents = plant.Alphabet();
  plant.ClrObservable(allEvents);
 
  // Read spec 
  RabinAutomaton spec;
  spec.Read("data/omg_6_cbspec12.gen");
  spec.GraphWrite("tmp_6_spec.png");
  InvProject(spec,plant.Alphabet());
  spec.Write("tmp_6_spec.gen");


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
  ProductPS.Write("tmp_6_Product.gen");
  ProductPS.DWrite();
  ProductPS.GraphWrite("tmp_6_Product.png");
   

  RabinAutomaton NRA;
  EpsObservation(ProductPS, NRA);
  NRA.Name("Epsilon Observed Product");
  NRA.Write("tmp_6_EpsObservedProduct.gen");
  NRA.DWrite();
  NRA.GraphWrite("tmp_6_EpsObservedProduct.png");
  RabinAutomaton DRA;
  PseudoDet(NRA, DRA);
  DRA.Name("Determinized Epsilon Observed Product");
  DRA.DWrite();
  DRA.Write("tmp_6_DRA.gen");
  DRA.GraphWrite("tmp_6_DRA.png");

  // // Standard partial observable control synthesis
  // // std::cout << "====== Standard Synthesis ======" << std::endl;
  // // RabinAutomaton epsObserved;
  // // RabinCtrlPartialObs(spec,contevents,obsevents,belt1,epsObserved);
  // // epsObserved.DWrite();
  // // epsObserved.Write("tmp_6_ObservedBelt.gen");
  // // epsObserved.GraphWrite("tmp_6_ObservedBelt.png");
  
  // Remove self-loof eps transitions and relink the state only with eps transition
  RabinAutomaton relinkDRA;
  RemoveEps(DRA, relinkDRA);
  relinkDRA.DWrite();
  relinkDRA.Write("tmp_6_relinkDRA.gen");
  relinkDRA.GraphWrite("tmp_6_relinkDRA.png");
  
  TaIndexSet<EventSet> controller;
  RabinCtrlPfx(relinkDRA,contevents,controller);
  relinkDRA.WriteStateSet(controller);
  std::cout << std::endl;
  
  // Test the new CloseLoopSpecification function
  System CLspec;
  ControlAut(relinkDRA,controller,CLspec);
  CLspec.DWrite();
  CLspec.GraphWrite("tmp_6_Controller.png");
  CLspec.Write("tmp_6_Controller.gen");

  // Check if the closed-loop controller is deterministic
  if (CLspec.IsDeterministic()) {
      std::cout << "✓ CloseLoopController is Deterministic" << std::endl;
  } else {
      std::cout << "✗ CloseLoopController is NOT Deterministic" << std::endl;
  }

  // ====== Controllability Verification: IsControllable(Plant, Controller) ======
  System CLspecProjected = CLspec;
  InvProject(CLspecProjected, plant.Alphabet());
  
  bool controllable = IsControllable(plant, CLspecProjected);
  
  if (controllable) {
      std::cout << "✓ CONTROLLABILITY VERIFIED: Controller is controllable w.r.t. Plant" << std::endl;
  } else {
      std::cout << "✗ CONTROLLABILITY FAILED: Controller is NOT controllable w.r.t. Plant" << std::endl;
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
  supervisedSystem.Write("tmp_6_supervisedSys.gen");

  // Perform language inclusion verification: L(V/G) ⊆ L(E)
  bool result = RabinLanguageInclusion(supervisedSystem, spec);
  
  if (result) {
      std::cout << "✓ VERIFICATION SUCCESSFUL: L(V/G) ⊆ L(E)" << std::endl;
      std::cout << "The supervised system satisfies the specification." << std::endl;
  } else {
      std::cout << "✗ VERIFICATION FAILED: L(V/G) ⊄ L(E)" << std::endl;
      std::cout << "The supervised system does not satisfy the specification." << std::endl;
  }

}
