/** @file op_ex_verification.cpp

    Tutorial, verification of properties of natural projections. 

    This tutorial explains the routines for verifying if the Lm-observer condition, output control consistency (OCC) 
    or local control consistency (LCC) are fulfilled for a given natural projection. The computations are carried out 
    with a given generator and a given high-level alphabet. 

    @ingroup Tutorials

    @include op_ex_verification.cpp
*/

#include <stdio.h>
#include <iostream>
#include "libfaudes.h"
#include "op_include.h"

// make libFAUDES namespace available
using namespace faudes;


int main(int argc, char* argv[]) {
  //////////////////////////////////////////
  //Verification of the Lm-observer property
  //////////////////////////////////////////
	
  // construct original Generator from file
  System genOrig = Generator("data/ex_verification/ex_isnot_observer.gen");
  // construct abstraction alphabet from file
  EventSet highAlph = EventSet("data/ex_verification/ex_isnot_observer.alph", "Alphabet");
  // verify if the natural projection on highAlph is an Lm-observer and output the result. 
  // In this example, the Lm-observer property is violated.
  bool observer = IsObs(genOrig, highAlph);
  std::cout << "##################################\n";
  std::cout << "# Observer verification result: " << observer << std::endl;
  std::cout << "##################################\n";


  // read a new generator and a new high-level alphabet
  genOrig.Read("data/ex_verification/ex_is_observer.gen");
  highAlph.Read("data/ex_verification/ex_is_observer.alph", "Alphabet");
  // verify if the natural projection on the current highAlph is an Lm-observer and output the result. 
  // In this example, the Lm-observer property is fulfilled.
  observer = IsObs(genOrig, highAlph);
  std::cout << "##################################\n";
  std::cout << "Observer verification result: " << observer << std::endl;
  std::cout << "##################################\n";
	
  ///////////////////////////////////////////////////////////////
  //Verification of the output control consistency (OCC) property
  ///////////////////////////////////////////////////////////////
	
  //verify if the natural projection on highAlph fulfills OCC for the previously read generator and alphabet. 
  // In this example, OCC is violated.
  bool occ = IsOCC(genOrig, highAlph);
  std::cout << "###########################\n";
  std::cout << "OCC verification result: " << occ << std::endl;
  std::cout << "###########################\n";

  // read a new generator that fulfills OCC together with the previous highAlph
  genOrig.Read("data/ex_verification/ex_is_occ.gen");
  //Verification of the output control consistency (OCC) property. In this case, OCC is fulfilled.
  occ = IsOCC(genOrig, highAlph);
  std::cout << "###########################\n";
  std::cout << "OCC verification result: " << occ << std::endl;
  std::cout << "###########################\n";
	
  //////////////////////////////////////////////////////////////
  //Verification of the local control consistency (LCC) property
  //////////////////////////////////////////////////////////////
	
  //verify if the natural projection on highAlph fulfills LCC. In this case, LCC is fulfilled (OCC implies LCC).
  bool lcc = IsLCC(genOrig, highAlph);
  std::cout << "###########################\n";
  std::cout << "lcc verification result: " << lcc << std::endl;
  std::cout << "###########################\n";
  // read a new generator that does not fulfill LCC with highAlph.
  genOrig.Read("data/ex_verification/ex_is_observer.gen");
  //verify if the natural projection on highAlph fulfills LCC. In this case, LCC is violated.
  lcc = IsLCC(genOrig, highAlph);
  std::cout << "###########################\n";
  std::cout << "lcc verification result: " << lcc << std::endl;
  std::cout << "###########################\n";
  // read a new generator that fulfills LCC together with highAlph
  genOrig.Read("data/ex_verification/ex_is_lcc.gen");
  //Verification of the local control consistency (LCC) property. In this case, LCC is fulfilled.
  lcc = IsLCC(genOrig, highAlph);
  std::cout << "###########################\n";
  std::cout << "lcc verification result: " << lcc << std::endl;
  std::cout << "###########################\n";

  return 0;
}
