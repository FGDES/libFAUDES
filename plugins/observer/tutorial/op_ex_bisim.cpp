/** @file op_ex_bisim.cpp

Tutorial, computation of the coarsest quasi-congruence

This tutorial demonstrates the use of the Bisimulation class for computing the 
coarsest quasi-congruence on a given automaton. The theoretical background for the
provided methods can be found in
J.-C. Fernandez, "An implementation of an efficient algorithm for
bisimulation equivalence," Science of Computer Programming, vol. 13,
pp. 219-236, 1990.

@ingroup Tutorials

@include op_ex_bisim.cpp
*/



#include <stdio.h>
#include <iostream>
#include "libfaudes.h"

using namespace faudes;


int main(int argc, char* argv[]) {

  // read original generator from file input 
  Generator genOrig("data/ex_bisim/ex_bisim.gen");
  // Write the original generator to console
  std::cout << "#####################\n";
  std::cout << "# original generator \n";
  std::cout << "#####################\n";
  genOrig.DWrite();
  // create an empty map<Idx,Idx>
  // it is passed by reference to the fuction ComputeBisimulation and will map the index of 
  // each state (first Idx) to the index of its equivalence class (second Idx)
  std::map<Idx,Idx> mapStateToPartition;
	
  // create an empty Generator. It is passed by reference to the function ComputeBisimulation
  // and will hold the coarsest quasi-congruence on genOrig, where each equivalence class is represented by a state
  Generator genPart;
	
  // call the function ComputeBisimulation which computes the coarses quasi-congruence
  ComputeBisimulation(genOrig, mapStateToPartition, genPart);
	
  // Write the resulting generator that holds the quasi-congruence to console
  std::cout << "#########################################\n";
  std::cout << "# generator holding the quasi-congruence\n";
  std::cout << "#########################################\n";
  genPart.DWrite();
  // Write the resulting generator to file output 
  genPart.Write("./results/ex_bisim/genPart.gen");
  // output the map from states in the original generator to equivalence classes (states in genPart)
  std::map<Idx,Idx>::const_iterator mIt, mEndIt;
  mIt = mapStateToPartition.begin();
  mEndIt = mapStateToPartition.end();
  std::cout << "##################################################\n";
  std::cout << "# map from original states to equivalence classes\n";
  for( ; mIt != mEndIt; mIt++){
    std::cout << "state: " << mIt->first << " class: " << mIt->second << "\n";
  }
  std::cout << "##################################################\n";

  return 0;
}
