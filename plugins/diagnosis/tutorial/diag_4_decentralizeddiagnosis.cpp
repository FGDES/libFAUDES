/** @file diag_4_decentralizeddiagnosis.cpp
Illustrate decentralized diagnosability test and synthesis. 
@ingroup Tutorials
@include diag_4_decentralizeddiagnosis.cpp
*/

#include "libfaudes.h"

using namespace faudes;


int main(void) {

  // Declare common variables
  System plant, spec;  
  EventSet *alph1, *alph2;
  EventSetVector alphVector;
  std::string report;

  // **********************************************************************
  //
  // Decentralized Language-Diagnosability Verification (condition is not fulfilled)
  //
	
  // Report to console
  std::cout << "################################\n";
  std::cout << "# decentralized language-diagnosability (system 1)\n";
  std::cout << "# a) read data \n";

  // Read subsystems and associated specifications and abstraction alphabets from files
  plant = System("data/diag_system_4_decentralized1.gen");
  spec = System("data/diag_specification_4_decentralized1.gen");
  
  // Write subsystems and specifications to gen files (for html docu)
  plant.Write("tmp_diag_system_4_decentralized1.gen");
  spec.Write("tmp_diag_specification_4_decentralized1.gen");

  // Write subsystems and specifications to png files (for inspection)
  plant.GraphWrite("tmp_demo_system_4_decentralized1.png");
  spec.GraphWrite("tmp_demo_specification_4_decentralized1.png");

  // Relevant abstractions for the modular diagnosability verification (for HTML docu)

  alph1 = new EventSet("data/diag_obsAlph_4_decentralized1_1.alph");
  alph2 = new EventSet("data/diag_obsAlph_4_decentralized1_2.alph");
  alphVector.Append(alph1 );
  alphVector.Append(alph2 );


  // Report to console
  std::cout << "# b) run decentralized diagnosability test (expect result FAIL)\n";

  // Test for modular diagnosability of the overall system
  bool ok=IsCoDiagnosable(plant, spec, alphVector);
  if(ok) {
    std::cout << "The overall system G is co-diagnosable with respect to overall specification K." << std::endl;
  } else {
    std::cout << "The overall system G is not co-diagnosable with respect to overall specification K." << std::endl;
  }
  // Compute local decentralzed diagnosers for the example
  GeneratorVector diagVector;
  DecentralizedDiagnoser(plant,spec,alphVector,diagVector);
  // Write diagnosers to .gen file (for html docu)
  diagVector.At(0).Write("tmp_diag_diagnoser_4_decentralized1_1.gen");
  diagVector.At(1).Write("tmp_diag_diagnoser_4_decentralized1_2.gen");
  // Write diagnosers to png (for inspection)
  diagVector.At(0).GraphWrite("tmp_demo_diagnoser_4_decentralized1_1.png");
  diagVector.At(1).GraphWrite("tmp_demo_diagnoser_4_decentralized1_2.png");
  
  // Record test case
  FAUDES_TEST_DUMP("decentralized 1",ok);

    // **********************************************************************
  //
  // Decentralized Language-Diagnosability Verification (condition is fulfilled)
  //
    // Report to console
  std::cout << "################################\n";
  std::cout << "# decentralized language-diagnosability (system 2)\n";
  std::cout << "# a) read data \n";

  // Read subsystems and associated specifications and abstraction alphabets from files
  plant = System("data/diag_system_4_decentralized2.gen");
  spec = System("data/diag_specification_4_decentralized2.gen");
  
  // Write subsystems and specifications to gen files (for html docu)
  plant.Write("tmp_diag_system_4_decentralized2.gen");
  spec.Write("tmp_diag_specification_4_decentralized2.gen");

  // Write subsystems and specifications to png files (for inspection)
  plant.GraphWrite("tmp_demo_system_4_decentralized2.png");
  spec.GraphWrite("tmp_demo_specification_4_decentralized2.png");

  // Relevant abstractions for the modular diagnosability verification (for HTML docu)
  alphVector.Clear();
  alph1 = new EventSet("data/diag_obsAlph_4_decentralized2_1.alph");
  alph2 = new EventSet("data/diag_obsAlph_4_decentralized2_2.alph");
  alphVector.Append(alph1 );
  alphVector.Append(alph2 );

  // Report to console
  std::cout << "# b) run decentralized diagnosability test (expect result FAIL)\n";

  // Test for modular diagnosability of the overall system
  ok=IsCoDiagnosable(plant, spec, alphVector);
  if(ok) {
    std::cout << "The overall system G is co-diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  } else {
    std::cout << "The overall system G is not co-diagnosable with respect to overall specification K." << std::endl;
    std::cout << "The overall system G is not co-diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  }
  diagVector.Clear();
  // Compute decentralized diagnosers for the local sites 
  DecentralizedDiagnoser(plant,spec,alphVector,diagVector);
  // Write diagnosers to .gen file (for html docu)
  diagVector.At(0).Write("tmp_diag_diagnoser_4_decentralized2_1.gen");
  diagVector.At(1).Write("tmp_diag_diagnoser_4_decentralized2_2.gen");
  // Write diagnosers to png (for inspection)
  diagVector.At(0).GraphWrite("tmp_demo_diagnoser_4_decentralized2_1.png");
  diagVector.At(1).GraphWrite("tmp_demo_diagnoser_4_decentralized2_2.png");
  // Record test case
  FAUDES_TEST_DUMP("decentralized 2",ok);

  // **********************************************************************
  //
  // Decentralized Language-Diagnosability Verification (condition is not fulfilled)
  //
    // Report to console
  std::cout << "################################\n";
  std::cout << "# decentralized language-diagnosability (system 3)\n";
  std::cout << "# a) read data \n";

  // Read subsystems and associated specifications and abstraction alphabets from files
  plant = System("data/diag_system_2_language.gen");
  spec = System("data/diag_specification_2_language.gen");

  // Write subsystems and specifications to png files (for inspection)
  plant.GraphWrite("tmp_demo_system_2_language.png");
  spec.GraphWrite("tmp_demo_specification_2_language.png");

  // Relevant abstractions for the modular diagnosability verification (for HTML docu)
  alphVector.Clear();
  alph1 = new EventSet;
  alph1->Insert("alpha");
  alph1->Insert("beta");
  alph2 = new EventSet();
  alph2->Insert("alpha");
  alphVector.Append(alph1 );
  alphVector.Append(alph2 );

  // Report to console
  std::cout << "# b) run decentralized diagnosability test (expect result FAIL)\n";

  // Test for modular diagnosability of the overall system
  ok=IsCoDiagnosable(plant, spec, alphVector);
  if(ok) {
    std::cout << "The overall system G is co-diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  } else {
    std::cout << "The overall system G is not co-diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  }
  
  // Record test case
  FAUDES_TEST_DUMP("decentralized 3",ok);
 
 
   // **********************************************************************
  //
  // Decentralized Language-Diagnosability Verification for Application Example (condition is  fulfilled)
  //
    // Report to console
  std::cout << "################################\n";
  std::cout << "# decentralized language-diagnosability (system sf/c1)\n";
  std::cout << "# a) read data \n";

  // Read subsystems and associated specifications and abstraction alphabets from files
  System sf,c1,sfc1Spec;
  sf = System("data/diag_system_4_decentralized_sf.gen");
  c1 = System("data/diag_system_4_decentralized_c1.gen");
  
  // Write generators (for html docu)
  sf.Write("tmp_diag_system_4_decentralized_sf.gen");
  c1.Write("tmp_diag_system_4_decentralized_c1.gen");
  // Write generators (for inspection)
  sf.GraphWrite("tmp_demo_system_4_decentralized_sf.png");
  c1.GraphWrite("tmp_demo_system_4_decentralized_c1.png");
  sfc1Spec = Generator("data/diag_specification_4_decentralized_sfc1.gen");
  aParallel(sf,c1,plant);
  aParallel(plant,sfc1Spec,spec);

  // Write subsystems and specifications to png files (for inspection)
  plant.GraphWrite("tmp_demo_system_4_decentralized_sfc1.png");
  spec.GraphWrite("tmp_demo_specification_4_decentralized_sfc1.png");

  // Observable events for the system components
  alphVector.Clear();
  alph1 = new EventSet();
  *alph1 = plant.ObservableEvents() * sf.Alphabet();
  alph2 = new EventSet();
  *alph2 = plant.ObservableEvents() * c1.Alphabet();
  alphVector.Append(alph1 );
  alphVector.Append(alph2 );

  // Report to console
  std::cout << "# b) run decentralized diagnosability test (expect result PASS)\n";

  // Test for modular diagnosability of the overall system
  ok=IsCoDiagnosable(plant, spec, alphVector);
  if(ok) {
    std::cout << "The overall system G is co-diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  } else {
    std::cout << "The overall system G is not co-diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  }
  // Compute decentralized diagnosers for the local sites 
  SystemVector plantVector;
  plantVector.Append(&sf);
  plantVector.Append(&c1);
  diagVector.Clear();
  DecentralizedModularDiagnoser(plantVector,spec,diagVector);
  // Write diagnosers to .gen file (for html docu)
  diagVector.At(0).Write("tmp_diag_diagnoser_4_decentralized_sf.gen");
  diagVector.At(1).Write("tmp_diag_diagnoser_4_decentralized_c1.gen");
  // Write diagnosers to png (for inspection)
  diagVector.At(0).GraphWrite("tmp_demo_diagnoser_4_decentralized_sf.png");
  diagVector.At(1).GraphWrite("tmp_demo_diagnoser_4_decentralized_c1.png"); 
  // Record test case
  FAUDES_TEST_DUMP("decentralized sf/c1",ok);

  
  // // **********************************************************************
  //
  // Abstraction-based Decentralized Language-Diagnosability Verification for Application Example (condition is  fulfilled)
  //
    // Report to console
  std::cout << "################################\n";
  std::cout << "# Abstraction-based decentralized language-diagnosability (system sf/c1)\n";
  std::cout << "# a) read data \n";

  // Compute abstracted system components and verify the loop-preserving observer condition
  System sfAbst, c1Abst;
  EventSet abstAlph;
  abstAlph.Read("data/diag_abstAlph_4_decentralized_sf.alph","Alphabet");
  if(IsLoopPreservingObserver(sf,abstAlph) )
	std::cout << "The abstraction for SF is a loop-preserving observer" << std::endl;
  else 
	std::cout << "The abstraction for SF is not a loop-preserving observer" << std::endl;
  aProject(sf,abstAlph,sfAbst);
  sfAbst.Write("tmp_diag_system_4_decentralized_sf_abstracted.gen");
  sfAbst.GraphWrite("tmp_demo_system_4_decentralized_sf_abstracted.png");
  abstAlph.Read("data/diag_abstAlph_4_decentralized_c1.alph","Alphabet");
  if(IsLoopPreservingObserver(c1,abstAlph) )
	std::cout << "The abstraction for C1 is a loop-preserving observer" << std::endl;
  else 
	std::cout << "The abstraction for C1 is not a loop-preserving observer" << std::endl;

  aProject(c1,abstAlph,c1Abst);
  c1Abst.Write("tmp_diag_system_4_decentralized_c1_abstracted.gen");
  c1Abst.GraphWrite("tmp_demo_system_4_decentralized_c1_abstracted.png");
  sfc1Spec = Generator("data/diag_specification_4_decentralized_sfc1.gen");
  aParallel(sfAbst,c1Abst,plant);
  aParallel(plant,sfc1Spec,spec);

  // Write subsystems and specifications to png files (for inspection)
  plant.GraphWrite("tmp_demo_system_4_decentralized_sfc1_abstracted.png");
  spec.GraphWrite("tmp_demo_specification_4_decentralized_sfc1_abstract.png");

  // Observable events for the system components
  alphVector.Clear();
  *alph1 = plant.ObservableEvents() * sfAbst.Alphabet();
  *alph2 = plant.ObservableEvents() * c1Abst.Alphabet();
  alphVector.Append(alph1);
  alphVector.Append(alph2);

  // Report to console
  std::cout << "# b) run decentralized diagnosability test (expect result PASS)\n";

  // Test for modular diagnosability of the overall system
  ok=IsCoDiagnosable(plant, spec, alphVector);
  if(ok) {
    std::cout << "The overall system G is co-diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  } else {
    std::cout << "The overall system G is not co-diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  }
  // // Record test case
  // FAUDES_TEST_DUMP("decentralized abstracted sf/c1",ok);

  
  // // Done
  // cout << endl;
  return 0;
}
