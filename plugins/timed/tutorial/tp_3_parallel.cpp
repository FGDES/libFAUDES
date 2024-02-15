/** @file tp_3_parallel.cpp

Tutorial, timed parallel composition.
Demonstratates synchronous composition of two timed automata by function faudes::TParallel

@ingroup Tutorials 

@include tp_3_parallel.cpp

*/

#include "libfaudes.h"


// for simplicity we make the faudes namespace available to our program
using namespace faudes;



/////////////////
// main program
/////////////////

int main() {

  // read two generators ...
  TimedGenerator g1("data/tsimplemachine.gen");
  TimedGenerator g2("data/tbuffer.gen");

  // ... and perform parallel composition
  TimedGenerator gres;
  TParallel(g1,g2,gres);

  // Report
  std::cout << "######################################\n";
  std::cout << "# t simple machine with buffer  \n";
  gres.DWrite();
  std::cout << "######################################\n";

  // Save to file
  gres.Write("tmp_tmachinewithbuffer.gen");

  // Test protocol
  FAUDES_TEST_DUMP("composition",gres.ToText());

  return 0;
}



