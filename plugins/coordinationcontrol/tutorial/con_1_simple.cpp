/** @file con_1_simple.cpp

Tutorial, coordination control

@ingroup Tutorials 

@include con_1_simple.cpp

*/

#include "libfaudes.h"

// we make the faudes namespace available to our program
using namespace faudes;

/////////////////
// main program
/////////////////

int main(void) {

  System g1, g2, g3, gk, spec, plant;
  EventSet e1, e2, e3, ek, ec;
  EventSetVector eVector;
  GeneratorVector gVector, specV;

  g1.Read("data/gen1.gen");
  g2.Read("data/gen2.gen");
  g3.Read("data/gen3.gen");
  spec.Read("data/spec.gen");
  
  gVector.Append(g1);
  gVector.Append(g2);
  gVector.Append(g3);
  aParallel(gVector,plant);

  e1.Read("data/e1.alph");
  e2.Read("data/e2.alph");
  e3.Read("data/e3.alph");
  ek.Read("data/ek.alph");
  ec.Read("data/ec.alph"); // controllable events

  eVector.Append(e1);  
  eVector.Append(e2);
  eVector.Append(e3);

  System proof;

  bool result1 = IsConditionalDecomposable(spec,eVector,ek,proof);

  if (result1) {
  	std::cout << "The language is conditionally decomposable." << std::endl;
  }
  else {
  	std::cout << "The language is not conditionally decomposable." << std::endl 
		<< "The proof strings are showin in the \'proof\' generator." << std::endl;
  }

  FAUDES_TEST_DUMP("is cond decomp",result1);

  //Coordinator
  Project(plant,ek,gk);

  //PkK, PikK, i=1,2,3
  System PkK, P1kK, P2kK, P3kK;
  Project(spec,ek,PkK);
  Project(spec,e1+ek,P1kK);
  Project(spec,e2+ek,P2kK);
  Project(spec,e3+ek,P3kK);

  specV.Append(P1kK);
  specV.Append(P2kK);
  specV.Append(P3kK);

  bool result2 = IsConditionalControllable(specV,PkK,gVector,gk,ec);


  if (result2) {
  	std::cout << "The language is conditionally controllable." << std::endl;
  }
  else {
  	std::cout << "The language is not conditionally controllable." << std::endl;
  }


  bool result3 = IsConditionalClosed(specV,PkK,gVector,gk);
 
  FAUDES_TEST_DUMP("is cond ctrl",result2);

  if (result3) {
  	std::cout << "The language is conditionally closed." << std::endl;
  }
  else {
  	std::cout << "The language is not conditionally closed." << std::endl;
  }

  FAUDES_TEST_DUMP("is cond closed",result3);

  FAUDES_TEST_DIFF();
}

