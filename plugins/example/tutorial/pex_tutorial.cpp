/** @file pex_tutorial.cpp  

Tutorial, example plugin. This tutorial demonstrates
how to organize tutorials in plugins.

@ingroup Tutorials 

@include pex_tutorial.cpp

*/

#include "libfaudes.h"

// make the faudes namespace available to our program
using namespace faudes;


/** Run the tutorial */
int main() {

  // read generator from file
  Generator gen("data/g_notacc.gen");

  // run algorithm
  AlternativeAccessible(gen);

  // write result
  gen.Write("tmp_altacc.gen");

  // Record test case
  FAUDES_TEST_DUMP("test alt access",gen)

  // Validate result
  FAUDES_TEST_DIFF()

  // done
  return 0;
}



