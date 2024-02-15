/** @file syn_2_omega.cpp

Synthesis for omega languages.

@ingroup Tutorials 

@include syn_2_omega.cpp

*/

#include "libfaudes.h"


// we make the faudes namespace available to our program
using namespace faudes;


/////////////////
// main program
/////////////////


int main() {


  /////////////////////////////////////////////
  // Omega control (closed supervisors)
  // 
  // The A-B-Machine is a machine that can run process A (event a) or process B (event b). 
  // Per operation, the machine  may succeed (event c) of fail (event d). However, it is 
  // guaranteed to suceed eventually. We have three variations:
  //
  // 1: the std case 
  // 2: process B exhausts the machine: it will succeed in process B once and then
  //    fail on further processes B until process A was run.
  // 3: process B breaks the machine: it will succeed in process B once and
  //    then fail in any further processes
  //
  // We synthesise controllers for 3 variant specifications
  //
  // 1. Alternate in successfully processing A and B
  // 2. Keep on eventually succeeding in some process
  // 3. Start with process A, eventually switch to B, repeat
  //
  /////////////////////////////////////////////


  // Read A-B-Machines and specifications
  System machineab1("data/wmachineab1.gen");
  System machineab2("data/wmachineab2.gen");
  System machineab3("data/wmachineab3.gen");
  Generator specab1("data/wspecab1.gen");
  Generator specab2("data/wspecab2.gen");
  Generator specab3("data/wspecab3.gen");

  // Fix lazy specifications by intersection with plant
  Generator specab11; OmegaProduct(specab1,machineab1,specab11); specab11.OmegaTrim();
  Generator specab21; OmegaProduct(specab2,machineab1,specab21); specab21.OmegaTrim();
  Generator specab31; OmegaProduct(specab3,machineab1,specab31); specab31.OmegaTrim();
  Generator specab12; OmegaProduct(specab1,machineab2,specab12); specab12.OmegaTrim();
  Generator specab22; OmegaProduct(specab2,machineab2,specab22); specab22.OmegaTrim();
  Generator specab32; OmegaProduct(specab3,machineab2,specab32); specab32.OmegaTrim();
  Generator specab13; OmegaProduct(specab1,machineab3,specab13); specab13.OmegaTrim();
  Generator specab23; OmegaProduct(specab2,machineab3,specab23); specab23.OmegaTrim();
  Generator specab33; OmegaProduct(specab3,machineab3,specab33); specab33.OmegaTrim();

  // Report result to console
  std::cout << "################################\n";
  std::cout << "# fixed lazy specifications by omega-trim \n# intersection with plant, state count\n";
  std::cout << " w.r.t a-b-machine 1 (std case): \n  ";
  std::cout << " #" << specab11.Size() << " #" << specab21.Size() << " #" << specab31.Size() << "\n";
  std::cout << " w.r.t a-b-machine 2 (b exhausts the machine): \n  ";
  std::cout << " #" << specab12.Size() << " #" << specab22.Size() << " #" << specab32.Size() << "\n";
  std::cout << " w.r.t a-b-machine 3 (b breaks the machine): \n  ";
  std::cout << " #" << specab13.Size() << " #" << specab23.Size() << " #" << specab33.Size() << "\n";
  std::cout << "################################\n";



  // Test (relative closedness)
  bool rcl1_1 = IsRelativelyOmegaClosed(machineab1,specab11);
  bool rcl2_1 = IsRelativelyOmegaClosed(machineab1,specab21);
  bool rcl3_1 = IsRelativelyOmegaClosed(machineab1,specab31);
  bool rcl1_2 = IsRelativelyOmegaClosed(machineab2,specab12);
  bool rcl2_2 = IsRelativelyOmegaClosed(machineab2,specab22);
  bool rcl3_2 = IsRelativelyOmegaClosed(machineab2,specab32);
  bool rcl1_3 = IsRelativelyOmegaClosed(machineab3,specab13);
  bool rcl2_3 = IsRelativelyOmegaClosed(machineab3,specab23);
  bool rcl3_3 = IsRelativelyOmegaClosed(machineab3,specab33);


  // Report result to console
  std::cout << "################################\n";
  std::cout << "# omega relative closedness w.r.t. ab-machines\n";
  std::cout << " w.r.t a-b-machine 1 (std case): \n  ";
  if(rcl1_1) std::cout << " passed "; else  std::cout << " failed ";
  if(rcl2_1) std::cout << " passed "; else  std::cout << " failed ";
  if(rcl3_1) std::cout << " passed "; else  std::cout << " failed ";
  std::cout << "\n";
  std::cout << " w.r.t a-b-machine 2 (b exhausts the machine): \n  ";
  if(rcl1_2) std::cout << " passed "; else  std::cout << " failed ";
  if(rcl2_2) std::cout << " passed "; else  std::cout << " failed ";
  if(rcl3_2) std::cout << " passed "; else  std::cout << " failed ";
  std::cout << "\n";
  std::cout << " w.r.t a-b-machine 3 (b breaks the machine): \n  ";
  if(rcl1_3) std::cout << " passed "; else  std::cout << " failed ";
  if(rcl2_3) std::cout << " passed "; else  std::cout << " failed ";
  if(rcl3_3) std::cout << " passed "; else  std::cout << " failed ";
  std::cout << "\n";
  std::cout << "################################\n";

  // Record test case
  FAUDES_TEST_DUMP("RelClosed_1_1",rcl1_1);
  FAUDES_TEST_DUMP("RelClosed_2_1",rcl2_1);
  FAUDES_TEST_DUMP("RelClosed_3_1",rcl3_1);
  FAUDES_TEST_DUMP("RelClosed_1_2",rcl1_2);
  FAUDES_TEST_DUMP("RelClosed_2_2",rcl2_2);
  FAUDES_TEST_DUMP("RelClosed_3_2",rcl3_2);
  FAUDES_TEST_DUMP("RelClosed_1_3",rcl1_3);
  FAUDES_TEST_DUMP("RelClosed_2_3",rcl2_3);
  FAUDES_TEST_DUMP("RelClosed_3_3",rcl3_3);

  // Close specification
  Generator specab11c=specab11; PrefixClosure(specab11c);
  Generator specab12c=specab12; PrefixClosure(specab12c);
  Generator specab21c=specab21; PrefixClosure(specab21c);
  Generator specab23c=specab23; PrefixClosure(specab23c); 

  // Synthesise supervisors
  Generator supab11; SupConCmplNB(machineab1,specab11c,supab11);
  Generator supab21; SupConCmplNB(machineab1,specab21c,supab21);
  Generator supab12; SupConCmplNB(machineab2,specab12c,supab12);
  Generator supab23; SupConCmplNB(machineab3,specab23c,supab23);

  // Report result to console
  std::cout << "################################\n";
  std::cout << "# omega synthesis statistics\n";
  supab11.SWrite();
  supab12.SWrite();
  supab21.SWrite();
  supab23.SWrite();
  std::cout << "################################\n";

  // Record test case
  FAUDES_TEST_DUMP("Sup1_1",supab11);
  FAUDES_TEST_DUMP("Sup1_2",supab12);
  FAUDES_TEST_DUMP("Sup2_1",supab21);
  FAUDES_TEST_DUMP("Sup2_3",supab23);

 
 // Test controllability
  std::cout << "################################\n";
  std::cout << "# omega controllability:\n";
  if(IsOmegaControllable(machineab1,supab11))
    std::cout << "# supervisor11: passed (expected)\n";
  else 
    std::cout << "# supervisor11: failed (test case error)\n";
  if(IsOmegaControllable(machineab2,supab12))
    std::cout << "# supervisor12: passed (expected)\n";
  else 
    std::cout << "# supervisor12: failed (test case error)\n";
  if(IsOmegaControllable(machineab1,supab21))
    std::cout << "# supervisor21: passed (expected)\n";
  else 
    std::cout << "# supervisor21: failed (test case error)\n";
  if(IsOmegaControllable(machineab3,supab23))
    std::cout << "# supervisor23: passed (expected)\n";
  else 
    std::cout << "# supervisor23: failed (test case error)\n";
  std::cout << "################################\n";


  // Prepare graphical output for documentation, I
  supab11.StateNamesEnabled(false);
  supab21.StateNamesEnabled(false);
  supab12.StateNamesEnabled(false);
  supab23.StateNamesEnabled(false);
  StateMin(supab11,supab11);
  StateMin(supab21,supab21);
  StateMin(supab12,supab12);
  StateMin(supab23,supab23);

  // Prepare graphical output for documentation, II
  machineab1.Write("tmp_syn_2_machineab1.gen");
  machineab2.Write("tmp_syn_2_machineab2.gen");
  machineab3.Write("tmp_syn_2_machineab3.gen");
  specab1.Write("tmp_syn_2_specab1.gen");
  specab2.Write("tmp_syn_2_specab2.gen");
  specab3.Write("tmp_syn_2_specab3.gen");
  supab11.Write("tmp_syn_2_supab11.gen");
  supab21.Write("tmp_syn_2_supab21.gen");
  supab12.Write("tmp_syn_2_supab12.gen");
  supab23.Write("tmp_syn_2_supab23.gen");

  //Generator prodab22; Product(machineab2,specab22,prodab22);
  //prod22.Write("tmp_syn_2_prodab22.gen");



  /////////////////////////////////////////////
  // Marking supervisor omega control 
  // 
  // 
  ////////////////////////////////////////////

  // Run the above test cases
  Generator msupab11; OmegaSupConNB(machineab1,specab1,msupab11);
  Generator msupab21; OmegaSupConNB(machineab1,specab2,msupab21);
  Generator msupab31; OmegaSupConNB(machineab1,specab3,msupab31);
  Generator msupab12; OmegaSupConNB(machineab2,specab1,msupab12);
  Generator msupab22; OmegaSupConNB(machineab2,specab2,msupab22);
  Generator msupab32; OmegaSupConNB(machineab2,specab3,msupab32);
  Generator msupab13; OmegaSupConNB(machineab3,specab1,msupab13);
  Generator msupab23; OmegaSupConNB(machineab3,specab2,msupab23);
  Generator msupab33; OmegaSupConNB(machineab3,specab3,msupab33);
  
  // Report statistics
  std::cout << "################################\n";
  std::cout << "# omega synthesis statistics\n";
  msupab11.SWrite();
  msupab21.SWrite();
  msupab31.SWrite();
  msupab12.SWrite();
  msupab22.SWrite();
  msupab32.SWrite();
  msupab13.SWrite();
  msupab23.SWrite();
  msupab33.SWrite();
  std::cout << "################################\n";


  // Record test case
  FAUDES_TEST_DUMP("OSup1_1",msupab11);
  FAUDES_TEST_DUMP("OSup2_1",msupab21);
  FAUDES_TEST_DUMP("OSup3_1",msupab31);
  FAUDES_TEST_DUMP("OSup1_2",msupab12);
  FAUDES_TEST_DUMP("OSup2_2",msupab22);
  FAUDES_TEST_DUMP("OSup3_2",msupab32);
  FAUDES_TEST_DUMP("OSup1_3",msupab13);
  FAUDES_TEST_DUMP("OSup2_3",msupab23);
  FAUDES_TEST_DUMP("OSup3_3",msupab33);

  // Prepare graphical output for documentation, I
  msupab11.StateNamesEnabled(false);
  msupab21.StateNamesEnabled(false);
  msupab31.StateNamesEnabled(false);
  msupab12.StateNamesEnabled(false);
  msupab22.StateNamesEnabled(false);
  msupab32.StateNamesEnabled(false);
  msupab13.StateNamesEnabled(false);
  msupab23.StateNamesEnabled(false);
  msupab33.StateNamesEnabled(false);
  StateMin(msupab11,msupab11);
  StateMin(msupab21,msupab21);
  StateMin(msupab31,msupab31);
  StateMin(msupab12,msupab12);
  StateMin(msupab22,msupab22);
  StateMin(msupab32,msupab32);
  StateMin(msupab13,msupab13);
  StateMin(msupab23,msupab23);
  StateMin(msupab33,msupab33);

  // Prepare graphical output II
  //msupab11.Write("tmp_syn_2_msupab11.gen");
  //msupab21.Write("tmp_syn_2_msupab21.gen");
  msupab31.Write("tmp_syn_2_msupab31.gen");
  //msupab12.Write("tmp_syn_2_msupab12.gen");
  //msupab22.Write("tmp_syn_2_msupab22.gen");
  msupab32.Write("tmp_syn_2_msupab32.gen");
  //msupab13.Write("tmp_syn_2_msupab13.gen");
  //msupab23.Write("tmp_syn_2_msupab23.gen");
  msupab33.Write("tmp_syn_2_msupab33.gen");


  /////////////////////////////////////////////
  // Debugging/testing examples
  /////////////////////////////////////////////

  // Read example plant and spec
  System ex1plant("data/wplant1.gen");
  Generator ex1spec("data/wspec1.gen");

  // Run omega synthesis procedure
  Generator ex1super;
  OmegaSupConNB(ex1plant,ex1spec,ex1super);
  Generator ex1controller;
  OmegaConNB(ex1plant,ex1spec,ex1controller);

  // verify
  bool cntr1 = IsControllable(ex1plant,ex1controller);
  bool closed1 = IsRelativelyOmegaClosed(ex1plant,ex1controller);

  // Prepare graphical output for documentation
  ex1plant.Write("tmp_syn_2_wplant1.gen");
  ex1spec.Write("tmp_syn_2_wspec1.gen");
  ex1super.StateNamesEnabled(false);
  ex1super.Write("tmp_syn_2_wsuper1.gen");

  // Report result to console
  std::cout << "################################\n";
  std::cout << "# extended omega synthesis example 1 \n";
  ex1super.DWrite();
  ex1controller.DWrite();
  std::cout << "prefix controllability: " << cntr1 << std::endl;
  std::cout << "rel. top. closed: " << closed1 << std::endl;
  std::cout << "################################\n";


  // Read example plant and spec
  System ex2plant("data/wplant2.gen");
  Generator ex2spec("data/wspec2.gen");

  // Fix spec alphabet
  InvProject(ex2spec,ex2plant.Alphabet());
  
  // Run omega synthesis procedure
  Generator ex2super;
  OmegaSupConNB(ex2plant,ex2spec,ex2super);
  Generator ex2controller;
  OmegaConNB(ex2plant,ex2spec,ex2controller);

  // verify
  bool cntr2 = IsControllable(ex2plant,ex2controller);
  bool closed2 = IsRelativelyOmegaClosed(ex2plant,ex2controller);

  // Report result to console
  std::cout << "################################\n";
  std::cout << "# extended omega synthesis example 2 \n";
  ex2super.DWrite();
  ex2controller.DWrite();
  std::cout << "prefix controllability: " << cntr2 << std::endl;
  std::cout << "rel. top. closed: " << closed2 << std::endl;
  std::cout << "################################\n";

  // Prepare graphical aoutput for documentation
  ex2plant.Write("tmp_syn_2_wplant2.gen");
  ex2spec.Write("tmp_syn_2_wspec2.gen");
  ex2super.StateNamesEnabled(false);
  ex2super.Write("tmp_syn_2_wsuper2.gen");

  // Read example plant and spec
  System ex3plant("data/wplant3.gen");
  Generator ex3spec("data/wspec3.gen");

  // Run omega synthesis procedure
  Generator ex3super;
  OmegaSupConNormNB(ex3plant,ex3spec,ex3super);
  StateMin(ex3super, ex3super);
  Generator ex3controller;
  OmegaConNormNB(ex3plant,ex3spec,ex3controller);
  StateMin(ex3controller, ex3controller);

  // verify
  bool cntr3 = IsControllable(ex3plant,ex3controller);
  bool closed3 = IsRelativelyOmegaClosed(ex3plant,ex3controller);
  Generator ex3plant_loc = ex3plant;
  Generator ex3contr_loc = ex3controller;
  MarkAllStates(ex3plant_loc);
  MarkAllStates(ex3contr_loc);
  bool norm3 = IsNormal(ex3plant_loc,ex3plant.ObservableEvents(),ex3contr_loc);


  // Report result to console
  std::cout << "################################\n";
  std::cout << "# extended omega synthesis example 3 \n";
  ex3super.DWrite();
  ex3controller.DWrite();
  std::cout << "prefix controllability: " << cntr3 << std::endl;
  std::cout << "prefix normality : " << norm3 << std::endl;
  std::cout << "rel. top. closed: " << closed3 << std::endl;
  std::cout << "################################\n";

  // Prepare graphical aoutput for documentation
  ex3plant.Write("tmp_syn_2_wplant3.gen");
  ex3spec.Write("tmp_syn_2_wspec3.gen");
  ex3super.StateNamesEnabled(false);
  ex3super.Write("tmp_syn_2_wsuper3.gen");
  ex3controller.StateNamesEnabled(false);
  ex3controller.Write("tmp_syn_2_wcontr3.gen");

  FAUDES_TEST_DUMP("ex1super",ex1super);
  FAUDES_TEST_DUMP("ex1controller",ex1controller);
  FAUDES_TEST_DUMP("ex2super",ex2super);
  FAUDES_TEST_DUMP("ex2controller",ex2controller);
  FAUDES_TEST_DUMP("ex3super",ex3super);
  FAUDES_TEST_DUMP("ex3controller",ex3controller);

  return 0;
}

