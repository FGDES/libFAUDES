/** @file tp_2_constraints.cpp

Tutorial, class faudes::TimeConstraint. This tutorial demonstrates
basic usage of faudes::TimeConstraint objects

@ingroup Tutorials 

@include tp_2_constraints.cpp

*/


#include "libfaudes.h"


// make the faudes namespace available to our program
using namespace faudes;

/*

General Description of the TimeConstraints Class.

TimeConstraints provides a container of ElemConstraints, the latter modeling Alur's
elementary clock contraints. Each ElemConstraint is a simple inequality that restricts
a clock e.g. "clock7 less than 120".  In this library, clocks are required to have 
symbolic names that are resolved via a SymbolTable reference that is member of TimeConstraints. 
By default, the reference is set to a global SymbolTable. This is the same behaviour
as with events.


*/



/////////////////
// main program
/////////////////

int main() {

  ////////////////////////////////////////////////////
  // constraints file io and inspection
  ////////////////////////////////////////////////////

  // Create a timeconstraint from file
  TimeConstraint tc1("data/timeconstraint.txt");

  // Create an empty timeconstraint 
  TimeConstraint tc2;

  // Inspect on console
  std::cout << "######################################\n";
  std::cout << "# a timeconstraint \n";
  tc1.DWrite();
  std::cout << "######################################\n";

  // Write the constraint to a file 
  tc1.Write("tmp_timeconstraint.txt");


  // Get set of active clocks
  ClockSet clocks1=tc1.ActiveClocks();
  
  // Iterate over all clocks, inspect and convert to time interval
  std::cout << "######################################\n";
  std::cout << "# inspection ...  \n";
  ClockSet::Iterator cit;
  for(cit = clocks1.Begin(); cit != clocks1.End(); cit++) {
    std::cout << "clock: " << clocks1.SymbolicName(*cit) << "\n";
    TimeConstraint::Iterator tit;
    for(tit = tc1.Begin(*cit); tit != tc1.End(*cit); tit++) {
      std::cout << "elem constraint: " << tc1.EStr(*tit) << "\n";
    }
    TimeInterval tint=tc1.Interval(*cit);
    std::cout << "time interval: " << tint.Str() << "\n";
  }
  std::cout << "######################################\n";
  

  ////////////////////////////////////////////////////
  // edit constraint
  ////////////////////////////////////////////////////

  // Some interaction 
  tc1.Insert("clockC", ElemConstraint::LessThan,    125);
  tc1.Insert("clockC", ElemConstraint::GreaterThan,  10);
  tc1.Insert("clockC", ElemConstraint::GreaterEqual, 10); // redundent, but still added
  tc1.Insert("clockB", ElemConstraint::GreaterThan, 110);
  tc1.Insert("clockB", ElemConstraint::GreaterThan, 110); // double, not added to set 
  tc1.Insert("clockB", ElemConstraint::LessEqual,   200); 
  tc1.Insert("clockB", ElemConstraint::LessEqual,   210); // redundent, but still added
  tc1.Insert("clockB", ElemConstraint::LessThan,    220); // redundent, but still added

  // Report
  std::cout << "######################################\n";
  std::cout << "# after various insertions \n";
  tc1.DWrite();
  std::cout << "######################################\n";

  // Erase an elementary constraint
  tc1.Erase("clockB", ElemConstraint::LessEqual,   200); 

  // Report
  std::cout << "######################################\n";
  std::cout << "# after erase \n";
  tc1.DWrite();
  std::cout << "######################################\n";

  // Introduce constraints to represent a valid interval
  TimeInterval tint; // to become "(150,300]"
  tint.LB(150);
  tint.UB(300);
  tint.UBincl(true);
  tc2.Interval("clockB",tint);
  tc2.Interval("clockD",tint);
  
  // Report
  std::cout << "######################################\n";
  std::cout << "# another constraint \n";
  tc2.DWrite();
  std::cout << "######################################\n";


  // Merge two time constraints (aka intersection)
  tc1.Insert(tc2);

  // Remove redundant elem. constraints
  tc1.Minimize();

  // Report
  std::cout << "######################################\n";
  std::cout << "# minimal version \n";
  tc1.DWrite();
  std::cout << "######################################\n";

  // Test protocol
  FAUDES_TEST_DUMP("final constraint",tc1.ToString());


  return 0;
}



