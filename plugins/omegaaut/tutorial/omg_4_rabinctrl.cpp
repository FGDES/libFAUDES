/** @file omg_4_rabinctrl.cpp

Supervision of omega-languages w.r.t. Rabin acceptance

@ingroup Tutorials 

@include  omg_4_rabinctrl.cpp

*/

#include "libfaudes.h"


// we make the faudes namespace available to our program
using namespace faudes;


int main() {

  // Early stage of development ... see whether we can recover
  // the test cases from Buechi supervisory control. 

  /////////////////////////////////////////////
  // Control w.r.t Rabin specifications
  //
  // The A-B-Machine is a machine that can run process A (event a) or process B (event b). 
  // Per operation, the machine may succeed (event c) of fail (event d). However, it is 
  // guaranteed to eventually suceed when running the same process over and over again.
  // We have three variations:
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
  // 3. Run optionally process A, eventually turn to B until success, repeat
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
  InvProject(cand,sigall);
  Automaton(cand);
  RabinBuechiProduct(cand,plant,cand);
  //RabinBuechiAutomaton(cand,plant,cand);
  cand.Write();
  StateSet ctrlpfx;
  RabinCtrlPfx(cand,sigctrl,ctrlpfx);
  cand.WriteStateSet(ctrlpfx);
  cand.RestrictStates(ctrlpfx);
  // SupClosed(cand)
  cand.RabinAcceptanceWrite();
  cand.GraphWrite("tmp_omg_rabinctrl13.png");
  Generator test=cand;
  test.StateNamesEnabled(false);
  test.InjectMarkedStates(cand.RabinAcceptance().Begin()->RSet());
  StateMin(test,test);
  test.GraphWrite("tmp_omg_rabinctrl13_test.png");
  
  



  return 0;
}

