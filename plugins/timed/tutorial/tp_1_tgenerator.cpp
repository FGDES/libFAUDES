/** @file tp_1_tgenerator.cpp  

Tutorial, class faudes::TimedGenerator. This tutorial demonstrates
basic maintenance of TimedGenerator objects

@ingroup Tutorials 

@include tp_1_tgenerator.cpp

*/

#include "libfaudes.h"


// make the faudes namespace available to our program
using namespace faudes;



/////////////////
// main program
/////////////////

int main() {

  ////////////////////////////////////////////////////
  // constructor and file io
  // EventSets
  ////////////////////////////////////////////////////

  // Create an empty TimedGenerator object
  TimedGenerator g1;

  // Create a TimedGenerator by reading a TimedGenerator file
  TimedGenerator g2("data/tsimplemachine.gen");
 
  // Create a TimedGenerator by reading a Generator file (no timing, no control)
  TimedGenerator g3("data/simplemachine.gen");

  // Read a TimedGenerator from a System file (no timing)
  g1.Read("data/csimplemachine.gen");

  // Copy a Generator to a TimedGenerator (no timing, no control)
  TimedGenerator g4;
  Generator g5("data/simplemachine.gen");
  g4.Assign(g5);

  // Write the TimedGenerator to a files 
  g1.Write("tmp_tsimplemachine1.gen");
  g2.Write("tmp_tsimplemachine2.gen");
  g3.Write("tmp_tsimplemachine3.gen");
  g4.Write("tmp_tsimplemachine4.gen");

  // See result on console
  std::cout << "######################################\n";
  std::cout << "# tc simple machine (\n";
  g2.DWrite();
  std::cout << "######################################\n";

  // Test protocol
  FAUDES_TEST_DUMP("construct g1",g1);
  FAUDES_TEST_DUMP("construct g2",g2);
  FAUDES_TEST_DUMP("construct g3",g3);
  FAUDES_TEST_DUMP("construct g4",g4);


  ////////////////////////////////////////////////////
  // access to controllability status of events
  //
  // (using System methods)
  ////////////////////////////////////////////////////

  // Retrieve an EventSet containing all the controllabe events 

  EventSet cevents = g2.ControllableEvents();

  // See result on console
  std::cout << "######################################\n";
  std::cout << "# controllabel events\n";
  cevents.DWrite();
  std::cout << "######################################\n";

  // Test protocol
  FAUDES_TEST_DUMP("contr attrib",cevents);


  ////////////////////////////////////////////////////
  //   access/edit timing 
  ////////////////////////////////////////////////////
  
  // Insert a clock to tcsimplemachine
  g2.InsClock("cRepair");

  // Read clocks from tcsimplemachine and set them for csimplemachine 
  ClockSet clocks = g2.Clocks();
 
  // Report
  std::cout << "######################################\n";
  std::cout << "# tcsimple machine clocks \n";
  clocks.DWrite();
  std::cout << "######################################\n";


  // Add constraints to invariant 
  TimeConstraint inv;
  inv.Insert("cRepair",ElemConstraint::LessEqual,20);
  g2.InsInvariant("down", inv);

  // Add constraints to guard and a reset
  TransSet::Iterator tit;
  tit=g2.FindTransition("down","lambda","idle");
  if(tit==g2.TransRelEnd()) {}; // not found
  TimeConstraint guard;
  guard.Insert("cRepair",ElemConstraint::GreaterEqual,10);
  g2.InsGuard(*tit, guard);
  tit=g2.FindTransition("busy","mue","down");
  if(tit==g2.TransRelEnd()) {}; // not found
  ClockSet resets;
  resets.Insert("cRepair");
  g2.InsResets(*tit, resets);

  // Fix name
  g2.Name("tc simple machine with timed repair");

  // Report
  std::cout << "######################################\n";
  std::cout << "# tcsimple machine with timed repair \n";
  g2.DWrite();
  g2.ActiveClocks().DWrite();
  std::cout << "######################################\n";

  // Copy to g1 and remove a clock
  g1=g2;
  g1.DelClock("cBusy");

  // Report
  std::cout << "######################################\n";
  std::cout << "# tcsimple machine with untimed busy \n";
  g1.DWrite();
  g1.ActiveClocks().DWrite();
  std::cout << "######################################\n";


  // Test protocol
  FAUDES_TEST_DUMP("edit timing 1",g1);
  FAUDES_TEST_DUMP("edit timing 2",g2);


  // Read clocks from tcsimplemachine and set them for csimplemachine 
  std::cout << "######################################\n";
  std::cout << "# csimple machine (active clocks) \n";
  g3.InsClocks(g2.ActiveClocks());
  std::cout << g3.Clocks().ToString() << "\n";

  // Read invriants from tcsimplemachine and set them for csimplemachine 
  std::cout << "######################################\n";
  std::cout << "# csimple machine (invariants) \n";
  StateSet::Iterator sit;
  for(sit=g2.StatesBegin(); sit!=g2.StatesEnd(); sit++) {
    TimeConstraint invariant = g2.Invariant(*sit);
    std::string state = g2.StateName(*sit);
    std::cout << state << " inv " << invariant.ToString() << "\n";
    g3.Invariant(state, invariant);
  }

  // Read guards and resets from tcsimplemachine and set them for csimplemachine 
  std::cout << "######################################\n";
  std::cout << "# csimple machine (guards and resets) \n";
  TransSet::Iterator stit, dtit;
  for(stit=g2.TransRelBegin(); stit!= g2.TransRelEnd(); stit++) {
    std::cout << "src transition: " << g2.TStr(*stit) << "\n";
    std::cout << "guard:  " << g2.Guard(*stit).ToString() << "\n";
    std::cout << "resets: " << g2.Resets(*stit).ToString() << "\n";
    dtit=g3.FindTransition(g2.StateName(stit->X1),g2.EventName(stit->Ev),g2.StateName(stit->X2));
    g3.Guard(*dtit,g2.Guard(*stit));
    g3.Resets(*dtit,g2.Resets(*stit));
  }

  // Report
  std::cout << "######################################\n";
  std::cout << "# csimple machine with timing \n";
  g3.DWrite();
  std::cout << "######################################\n";


  // Check validiy (internal names and symboltables)
  if(g3.Valid())
    std::cout << "############# valid #################\n";
  
  // Test protocol
  FAUDES_TEST_DUMP("edit timing 3",g3);

  return 0;
}



