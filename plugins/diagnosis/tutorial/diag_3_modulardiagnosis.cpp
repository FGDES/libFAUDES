/** @file diag_3_modulardiagnosis.cpp
Illustrate modular diagnosability test and synthesis. 
@ingroup Tutorials
@include diag_3_modulardiagnosis.cpp
*/

#include "libfaudes.h"

using namespace faudes;


int main(void) {

  // Declare common variables
  System *g1, *g2, *k1, *k2;
  System g12, k12; 
  SystemVector plant;
  GeneratorVector spec;
  GeneratorVector diag;    
  std::string report;

  // **********************************************************************
  //
  // Modular Language-Diagnosability Verification (condition is fulfilled)
  //
	
  // Report to console
  std::cout << "################################\n";
  std::cout << "# modular language-diagnosability (system 1/2)\n";
  std::cout << "# a) read data \n";

  // Read subsystems and associated specifications and abstraction alphabets from files
  g1 = new System("data/diag_system_3_modular1.gen");
  g2 = new System("data/diag_system_3_modular2.gen");
  k1 = new System("data/diag_specification_3_modular1.gen");
  k2 = new System("data/diag_specification_3_modular2.gen");
  
  // Write subsystems and specifications to gen files (for html docu)
  g1->Write("tmp_diag_system_3_modular1.gen");
  g2->Write("tmp_diag_system_3_modular2.gen");
  k1->Write("tmp_diag_specification_3_modular1.gen");
  k2->Write("tmp_diag_specification_3_modular2.gen");

  // Relevant abstractions for the modular diagnosability verification (for HTML docu)
  EventSet abstAlph;
  abstAlph.Read("data/diag_abstrAlph_3_modular12.alph");
  Generator g1Abst, g2Abst;
  Project(*g1,abstAlph,g1Abst);
  Project(*g2,abstAlph,g2Abst);
  g1Abst.Write("tmp_diag_system_3_modular1_hat.gen");
  g2Abst.Write("tmp_diag_system_3_modular2_hat.gen"); 

  // Set up vector containers
  plant.Append(g1);
  plant.Append(g2);
  spec.Append(k1);
  spec.Append(k2);
  // Fix ownership 
  plant.TakeOwnership();
  spec.TakeOwnership();

  // Report to console
  std::cout << "# b) run modular diagnosability test (expect result PASS)\n";

  // Test for modular diagnosability of the overall system
  bool ok=IsModularDiagnosable(plant, spec, report);
  if(ok) {
    std::cout << "The overall system G is modularly diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  } else {
    std::cout << "The overall system G is not modularly diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  }
  
  // Record test case
  FAUDES_TEST_DUMP("modular 1/2",ok);


  // **********************************************************************
  //
  // Modular Diagnoser Computation 
  //
  
  // Report to console
  std::cout << "# c) compute modular diagnoser\n";
  
  // Diagnoser synthesis
  ModularDiagnoser(plant,spec,diag,report);

  // Write diagnoser moduls to gen files (for html docu)
  diag.At(0).Write("tmp_diag_diagnoser_3_modular1.gen");
  diag.At(1).Write("tmp_diag_diagnoser_3_modular2.gen");

  // Record test case
  FAUDES_TEST_DUMP("modular 1/2",diag);

  // Report to console
  std::cout << "# done \n";
  std::cout << "################################\n";

  // Clear vectors
  // Note: includes releasing member memory
  plant.Clear();
  spec.Clear();


  // **********************************************************************
  // **********************************************************************
  //
  // Modular Diagnosability Verification (Condition is not fulfilled)
  //

  // Report to console
  std::cout << "################################\n";
  std::cout << "# modular diagnosability (system 3/4)\n";
  std::cout << "# a) read data \n";
  
  // Read subsystems and associated specifications and abstraction alphabets from files
  g1 = new System("data/diag_system_3_modular3.gen");
  g2 = new System("data/diag_system_3_modular4.gen");
  k1 = new System("data/diag_specification_3_modular3.gen");
  k2 = new System("data/diag_specification_3_modular4.gen");
  
  // Write subsystems and specifications to gen files (for html docu)
  g1->Write("tmp_diag_system_3_modular3.gen");
  g2->Write("tmp_diag_system_3_modular4.gen");
  k1->Write("tmp_diag_specification_3_modular3.gen");
  k2->Write("tmp_diag_specification_3_modular4.gen");

  // Set up vector containers
  plant.Append(g1);
  plant.Append(g2);
  spec.Append(k1);
  spec.Append(k2);
  // Fix ownership
  plant.TakeOwnership();
  spec.TakeOwnership();
  
  // Report to console
  std::cout << "# b) run modular diagnosability test (expect result FAIL)\n";

  // Test for modular diagnosability of the overall system
  ok=IsModularDiagnosable(plant, spec, report);
  if(ok) {
    std::cout << "The overall system G is modularly diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  } else {
    std::cout << "The overall system G is not modularly diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  }
  
  // Record test case
  FAUDES_TEST_DUMP("modular 3/4",ok);

  // Report to console
  std::cout << "# done \n";
  std::cout << "################################\n";

  // Clear vectors
  // Note: includes releasing member memory
  plant.Clear();
  spec.Clear();

  // **********************************************************************
  // **********************************************************************
  //
  // Modular diagnosability Verification (application example) 
  //

  // Report to console
  std::cout << "################################\n";
  std::cout << "# modular diagnosability (system sf/cb1a)\n";
  std::cout << "# a) read data \n";

  // Read subsystems and associated specifications and abstraction alphabets from files
  g1 = new System("data/diag_system_3_modular_sf.gen");
  g2 = new System("data/diag_system_3_modular_c1.gen");
  k1 = new System("data/diag_specification_3_modular_sf.gen");
  k2 = new System("data/diag_specification_3_modular_c1.gen");
  
  // Write subsystems and specifications to gen files (for html docu)
  g1->Write("tmp_diag_system_3_modular_sf.gen");
  g2->Write("tmp_diag_system_3_modular_c1.gen");
  k1->Write("tmp_diag_specification_3_modular_sf.gen");
  k2->Write("tmp_diag_specification_3_modular_c1.gen");

  // Set up vector containers
  plant.Append(g1);
  plant.Append(g2);
  spec.Append(k1);
  spec.Append(k2);
  // Fix ownership
  plant.TakeOwnership();
  spec.TakeOwnership();
  
  // Report to console
  std::cout << "# b) run diagnosability test (expect result PASS)\n";

  // Test for decentralized diagnosability of the overall system
  if(IsModularDiagnosable(plant, spec, report)) {
    std::cout << "The overall system G is modularly diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  } else {
    std::cout << "The overall system G is not modularly diagnosable with respect to overall specification K." << std::endl;
    std::cout << report << std::endl;
  }
  
  // **********************************************************************
  //
  // Modular Diagnoser Computation 
  //
  
  // Report to console
  std::cout << "# c) compute modular diagnoser\n";
  diag.Clear();
  // Diagnoser synthesis
  ModularDiagnoser(plant,spec,diag,report);

  // Write diagnoser moduls to gen files (for html docu)
  diag.At(0).Write("tmp_diag_diagnoser_3_modular_sf.gen");
  diag.At(1).Write("tmp_diag_diagnoser_3_modular_c1.gen");

  // Record test case
  FAUDES_TEST_DUMP("modular sf/c1",diag);
  
  // Report to console
  std::cout << "# done \n";
  std::cout << "################################\n";

  // Clear vectors
  // Note: includes releasing member memory
  plant.Clear();
  spec.Clear();
  
  // Done
  std::cout << std::endl;
  return 0;
}
