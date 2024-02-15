/** @file hyb_1_hautomata.cpp  

Tutorial, hybrid systems plugin. 
This tutorial demonstrates how to access hybrid automata attributes.

@ingroup Tutorials 

@include hyb_1_hautomata.cpp

*/

#include "libfaudes.h"

// make the faudes namespace available to our program
using namespace faudes;


/** Run the tutorial */
int main() {



  // ///////////////////////////
  //
  // Section 1: parameter types
  // 
  //

  // Construct a matrix from file
  faudes::Matrix  matA("data/hyb_matrix.txt");

  // Write matrix to file
  matA.Write("tmp_hyb_1_matrix.txt");

  // Set to identity 5x5
  matA.Identity(5);

  // Access entries, faudes style (here: copy column)
  for(int i=0; i<matA.RowCount(); i++) 
    matA.At(i,1, matA.At(i,2));

  // Access entries, cosmetic operator (here: copy row)
  for(int j=0; j<matA.ColumnCount(); j++) 
    matA(1,j) = matA(2,j);

  // Write matrix to console
  matA.Write();

  // Report to console
  std::cout << "################################\n";
  std::cout << "# Some matrix: \n";
  matA.Write();
  std::cout << "################################\n";

  // Construct a polyhedron from file
  faudes::Polyhedron polyP("data/hyb_polyhedron.txt");

  // Report to console
  std::cout << "################################\n";
  std::cout << "# Some polyhedron: \n";
  polyP.Write();
  std::cout << "################################\n";

  // ///////////////////////////
  //
  // Section 2: linear hybrid automata
  // 
  //

  // Construct a linear hybrid automata  from file
  LinearHybridAutomaton  lha("data/hyb_lhautomaton.gen");

  // Report to console
  std::cout << "################################\n";
  std::cout << "# linear hybrid automaton from file: \n";
  lha.Write();
  std::cout << "################################\n";
  std::cout << "# Valid() returns " << lha.Valid() << "\n";
  std::cout << "################################\n";

  // access some invariant
  const faudes::Polyhedron& inv =lha.Invariant("Working");

  // access some rate
  const faudes::Polyhedron& rhs =lha.Rate("Working");

  // access some reset
  Transition ftrans=lha.TransitionByNames("Resting","alpha","Working");
  const LinearRelation& res =lha.Reset(ftrans);

  // access some guard
  const faudes::Polyhedron& grd =lha.Guard(ftrans);

  // Report to console
  std::cout << "################################\n";
  std::cout << "# state attributes\n";
  inv.Write();
  rhs.Write();
  std::cout << "################################\n";
  std::cout << "# transition attributes\n";
  grd.Write();
  res.Write();
  std::cout << "################################\n";


  // done
  return 0;
}



