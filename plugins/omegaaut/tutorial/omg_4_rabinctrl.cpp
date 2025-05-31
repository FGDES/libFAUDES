/** @file omg_4_rabinctrl.cpp

Supervision of omega-languages w.r.t. Rabin acceptance

@ingroup Tutorials 

@include  omg_4_rabinctrl.cpp

*/

#include "libfaudes.h"


// WORK IN PROGRESS

// we make the faudes namespace available to our program
using namespace faudes;


/////////////////
// main program
/////////////////


int main() {


  /////////////////////////////////////////////
  // Control w.r.t Rabin specifications
  //
  // 
  // The A-B-Machine is a machine that can run process A (event a) or process B (event b). 
  // Per operation, the machine may succeed (event c) of fail (event d). However, it is 
  // guaranteed to eventually suceed. We have three variations:
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
  System machineab1("data/omg_machineab1.gen");
  System machineab2("data/omg_machineab2.gen");
  System machineab3("data/omg_machineab3.gen");
  Generator specab1("data/omg_specab1.gen");
  Generator specab2("data/omg_specab2.gen");
  Generator specab3("data/omg_specab3.gen");

  // compute a controllability prefix
  const System& plant=machineab1;
  const Generator&  spec=specab3;  
  EventSet sigall = plant.Alphabet() + spec.Alphabet();
  EventSet sigctrl = plant.ControllableEvents();
  RabinAutomaton cand=spec;
  cand.RabinAcceptance(cand.MarkedStates());
  cand.InsEvents(sigall);
  Automaton(cand);
  cand.Write();
  RabinBuechiAutomaton(cand,plant,cand);
  cand.Write();
  StateSet ctrlpfx;
  RabinCtrlPfx(cand,sigctrl,ctrlpfx);
  cand.WriteStateSet(ctrlpfx);
  cand.RestrictStates(ctrlpfx);
  cand.GraphWrite("tmp_omg_rabinctrl13.png");
  
  

  // Report statistics
  /*
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
  */

  // Record test case
  /*
  FAUDES_TEST_DUMP("OSup1_1",msupab11);
  FAUDES_TEST_DUMP("OSup2_1",msupab21);
  FAUDES_TEST_DUMP("OSup3_1",msupab31);
  FAUDES_TEST_DUMP("OSup1_2",msupab12);
  FAUDES_TEST_DUMP("OSup2_2",msupab22);
  FAUDES_TEST_DUMP("OSup3_2",msupab32);
  FAUDES_TEST_DUMP("OSup1_3",msupab13);
  FAUDES_TEST_DUMP("OSup2_3",msupab23);
  FAUDES_TEST_DUMP("OSup3_3",msupab33);
  */


  return 0;
}

