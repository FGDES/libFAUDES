/** @file ios_1_system.cpp  

Tutorial, I/O systems


@ingroup Tutorials 

@include ios_1_system.cpp

*/

#include "libfaudes.h"

// make the faudes namespace available to our program
using namespace faudes;



/////////////////
// main program
/////////////////

int main() {

  ////////////////////////////////////////////
  // Constructors 
  ////////////////////////////////////////////

  // construct from file
  IoSystem sys1("data/ios_system1.gen");

  // copy construct
  IoSystem sys2(sys1);

  // assign 
  IoSystem sys3 = sys2;

  // inspect result on console
  std::cout << "################################\n";
  std::cout << "# tutorial, io system 1 \n";
  sys3.Write();
  std::cout << "################################\n";

  // graph output requires,  "dot" binary in $PATH
  try {
    sys1.GraphWrite("tmp_ios_system1.svg");
  } catch(faudes::Exception& exception) {
    std::cout << "warning: cannot execute graphviz' dot. " << std::endl;
  } 

  // output for doku
  sys1.Write("tmp_ios_system1.gen");


  ///////////////////////////////////////////////
  // Inspect/Edit I/O attributes
  ///////////////////////////////////////////////

  // events, result on console
  std::cout << "################################\n";
  std::cout << "# input events, output events\n";
  sys3.InputEvents().Write();
  sys3.OutputEvents().Write();
  std::cout << "################################\n";

  // maniputlate some state attributes
  // (to configure set state, see also IoSystem())
  sys3.SetOutputState("s1");
  sys3.SetInputState("s3");
  sys3.ClrOutputState("s2");

  // swap event status all together
  EventSet inevs = sys3.InputEvents();
  EventSet outevs = sys3.OutputEvents();
  sys3.ClrOutputEvent(outevs);
  sys3.SetInputEvent(outevs);
  sys3.ClrInputEvent(inevs);
  sys3.SetOutputEvent(inevs);

  // inspect result on console
  std::cout << "################################\n";
  std::cout << "# tutorial, manipulated io system 1 \n";
  sys3.Write();
  std::cout << "################################\n";
       
  ///////////////////////////////////////////////
  // Test I/O structure
  ///////////////////////////////////////////////

  // the test will set the state attributes accordingly
  bool isio=IsIoSystem(sys1);

  // sys1 from file will pass the test
  if(!isio) std::cout << "#### test FAILED (test case error) \n";

  // report
  std::cout << "################################\n";
  std::cout << "# tutorial, sys1 test io properties \n";
  sys1.InputStates().Write();
  sys1.OutputStates().Write();
  std::cout << "################################\n";

  // however, the input is not free
  bool isfree=IsInputLocallyFree(sys1);
  if(isfree) std::cout << "#### test PASSED (test case error) \n";
  IoFreeInput(sys1);

  // report
  std::cout << "################################\n";
  std::cout << "# tutorial, sys1 with free input\n";
  sys1.Write();
  std::cout << "################################\n";

  // output for doku
  sys1.Write("tmp_ios_system1f.gen");

  // however, the input is still not omega-free
  StateSet qerr;
  bool iswfree=IsInputOmegaFree(sys1,qerr);
  if(iswfree) std::cout << "#### test PASSED (test case error) \n";

  // report
  std::cout << "################################\n";
  std::cout << "# tutorial, sys1 error states\n";
  sys1.WriteStateSet(qerr);
  std::cout << "################################\n";

  // Record testcase
  FAUDES_TEST_DUMP("sys1", sys1);
  FAUDES_TEST_DUMP("isio", isio);
  FAUDES_TEST_DUMP("isfree", isfree);
  FAUDES_TEST_DUMP("iswfree", iswfree);
  FAUDES_TEST_DUMP("qerr", qerr);

  return 0 ;

}




