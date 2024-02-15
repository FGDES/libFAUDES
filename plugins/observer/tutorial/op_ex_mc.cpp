/** @file op_ex_mc.cpp

Tutorial to demonstarte the verification of mutual controllability.

@ingroup Tutorials

*/


#include <stdio.h>
#include <iostream>
#include "libfaudes.h"


// make libFAUDES namespace available
using namespace faudes;


int main(int argc, char* argv[]) {

  // read two generators from file input
  System gen1("./data/ex_mc/ex_mc1.gen");
  System gen2("./data/ex_mc/ex_mc2.gen");
	
  // Verify mutual controllability of gen1 and gen2. In this case, mutual controllabiity is violated.
  bool mc = IsMutuallyControllable(gen1,gen2);
  std::cout << "#################################################\n";
  std::cout << "# Mutual controllability verification - result: " << mc << std::endl;
  std::cout << "#################################################\n";
	
  // read one further generator from file input
  System gen3("./data/ex_mc/ex_mc3.gen");
  // Verify mutual controllability of gen3 and gen2. In this case, mutual controllability holds.
  mc = IsMutuallyControllable(gen3,gen2);
  std::cout << "##################################################\n";
  std::cout << "# Mutual controllability verification - result: " << mc << std::endl;
  std::cout << "##################################################\n";

  	
  return 0;
}
