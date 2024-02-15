/** @file diag_2_languagediagnosis.cpp
Illustrate diagnosability with respect to a specification language. 
@ingroup Tutorials 
@include diag_2_languagediagnosis.cpp
*/

#include "libfaudes.h"

using namespace faudes;


int main(void) {

  // **********************************************************************
  //
  // Language-Diagnosability Verification 
  //
	
  // Declare common variables
  System gen, specGen;
  EventSet highEvents;
  Diagnoser diagnoser;
  std::string reportString;
	
  // Report to console
  std::cout << "################################\n";
  std::cout << "# languag-diagnosability, system_2_language, specifiation_2_language \n";
  std::cout << "# a) read data \n";
  
  // Read input generators for plant and specification
  gen.Read("data/diag_system_2_language.gen");
  specGen.Read("data/diag_specification_2_language.gen");	

  // Write input generator to gen file for html docu
  gen.Write("tmp_diag_system_2_language.gen"); 
  specGen.Write("tmp_diag_specification_2_language.gen");

  // Report to console
  std::cout << "run language-diagnosability test (expect result FAIL)\n";
  
  // Test generator gen for diagnosability with respect to failure partition failureTypes
  bool isdiag = IsLanguageDiagnosable(gen,specGen,reportString);
  if(isdiag){
    std::cout << "System is diagnosable." << std::endl;
  } 
  else {
    std::cout << "System is not diagnosable." << std::endl;
    std::cout << reportString << std::endl;
  }

  // Record test case
  FAUDES_TEST_DUMP("test lang diag",isdiag);

  // **********************************************************************
  //
  // Language-Diagnoser Computation 
  //

  // Report to console
  std::cout << "# language-diagnoser, system_2_language, specifiation_2_language \n";

  // Synthesis
  LanguageDiagnoser(gen,specGen,diagnoser);

  // Write diagnoser to gen file for html docu
  diagnoser.Write("tmp_diag_diagnoser_2.gen");
	
  // Record test case
  FAUDES_TEST_DUMP("test lang diag",diagnoser);

  // **********************************************************************
  //
  // Abstraction-based Diagnosis - Loop-Preserving Observer
  //

  // Report to console
  std::cout << "################################\n";
  std::cout << "# language-diagnoser, abstraction based diagnosis\n";

  // Read input generator and abstraction alphabet
  gen.Read("data/diag_system_2_abstraction_fails.gen");
  highEvents.Read("data/diag_abstrAlph_2_abstraction_fails.alph","Alphabet");	

  // Write input generator to gen file for html docu
  gen.Write("tmp_diag_system_2_abstraction_fails.gen");

  // Test
  std::cout << "# test loop-preserving observer property (expect FAIL)\n";
  bool islpo = IsLoopPreservingObserver(gen,highEvents);
  if(islpo){
    std::cout << "Abstraction is loop-preserving observer." << std::endl;
  } 
  else {
    std::cout << "Abstraction is not a loop-preserving observer." << std::endl;
  }

  std::cout << "################################\n";
  std::cout << "# compute loop-preserving observer, system_2_abstraction_fails \n";	
  EventSet highAlph;
  LoopPreservingObserver(gen,highEvents,highAlph);
  std::cout << "# Abstraction Alphabet \n"; 
  highAlph.Write();  

  // Record test case
  FAUDES_TEST_DUMP("observer fail",islpo);
  FAUDES_TEST_DUMP("observer pass",highAlph);

  return 0;
}
