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
  System machineab1("data/omg_machineab1_alt.gen");
  System machineab2("data/omg_machineab2.gen");
  System machineab3("data/omg_machineab3.gen");
  Generator specab1("data/omg_specab1.gen");
  Generator specab2("data/omg_specab2.gen");
  Generator specab3("data/omg_specab3.gen");

  // select test case plant=1 spec=3
  const System& plant=machineab1;
  plant.Write("tmp_omg_4_plant1.gen");
  const Generator&  spec=specab3;  
  EventSet sigall = plant.Alphabet() + spec.Alphabet();
  EventSet sigctrl = plant.ControllableEvents();

  // set up cloed loop candidate
  RabinAutomaton cand=spec;
  cand.Write("tmp_omg_4_spec3_buechi.gen");
  cand.RabinAcceptance(cand.MarkedStates());
  cand.ClearMarkedStates();
  cand.Write("tmp_omg_4_spec3_rabin.gen");
  InvProject(cand,sigall);
  Automaton(cand);
  cand.Write("tmp_omg_4_spec3_rabinfull.gen");
  RabinAutomaton rbaut;                         // dox only
  RabinBuechiAutomaton(cand,plant,rbaut);       // dox only
  rbaut.Write("tmp_omg_4_rbaut13.gen");         // dox only
  RabinBuechiProduct(cand,plant,cand);
  cand.Write("tmp_omg_4_cand13.gen");
  std::cout << "====== first candidate" << std::endl;
  cand.Write();

  // turn on mu-nu protocol
  Verbosity(10);

  // compute controllability prefix
  StateSet ctrlpfx;
  RabinCtrlPfx(cand,sigctrl,ctrlpfx);
  std::cout << "====== controllability prefix" << std::endl;
  cand.WriteStateSet(ctrlpfx);
  std::cout << std::endl;
  RabinAutomaton cpxaut=cand;                   // dox only
  RabinPair rpair;                              // dox only
  cpxaut.RabinAcceptance().Append(rpair);       // dox only
  cpxaut.RabinAcceptance().Append(rpair);       // dox only
  rpair.RSet().InsertSet(ctrlpfx);              // dox only
  cpxaut.RabinAcceptance().Append(rpair);       // dox only
  cpxaut.Write("tmp_omg_4_ctrlpfx13.gen");      // dox only

  
  // compute controller
  TaIndexSet<EventSet> controller;
  RabinCtrlPfx(cand,sigctrl,controller);
  std::cout << "====== controller" << std::endl;
  cand.WriteStateSet(controller);
  std::cout << std::endl;

  // compute supremal controllable sublanguage
  cand.ClearMarkedStates();
  cand.InsMarkedStates(ctrlpfx);
  SupClosed(cand,cand);
  cand.ClearMarkedStates();
  cand.RestrictStates(cand.States());
  cand.Write("tmp_omg_4_supcon13.gen");

  // finalise result to compare with SupBuechiCon
  Generator test=cand;
  test.StateNamesEnabled(false);
  test.InjectMarkedStates(cand.RabinAcceptance().Begin()->RSet());
  StateMin(test,test);
  test.Write("tmp_omg_4_rabinctrl13.gen");
  
  return 0;
}

