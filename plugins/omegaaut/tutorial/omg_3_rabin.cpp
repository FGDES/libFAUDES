/** @file omg_3_rabin.cpp 

Tutorial on  Rabin automata

@ingroup Tutorials 

@include omg_3_rabin.cpp

*/

#include "libfaudes.h"

using namespace faudes;


int main(void) {

  ////////////////////////////////////////////////////
  // Have some state sets to play with
  ////////////////////////////////////////////////////

  StateSet statesa, statesb, statesc;
  statesa.FromString("<S> 1 2 3 </S>");
  statesb.FromString("<S> 4 5 6 </S>");
  statesc.FromString("<S> 7 8 9 </S>");

  ////////////////////////////////////////////////////
  // RabinPair
  ////////////////////////////////////////////////////

  // say hello
  std::cout << "======== a Rabin pair" << std::endl;

  // set up a Rabin pair programatically
  RabinPair rpair;
  rpair.RSet().InsertSet(statesa);
  rpair.ISet().InsertSet(statesa + statesb);

  // further inspect/manipulate as any state set; e.g.
  rpair.ISet().Erase(5);
  
  // report result
  rpair.Write();
  std::cout << std::endl;

  ////////////////////////////////////////////////////
  // RabinAceptance
  ////////////////////////////////////////////////////

  // say hello
  std::cout << "======== a Rabin acceptance condition" << std::endl;

  // set up a Rabin acceptance condition programatically
  RabinAcceptance raccept;
  raccept.Insert(rpair);
  rpair.Clear();
  rpair.RSet().InsertSet(statesc);
  rpair.ISet().InsertSet(statesc + statesb);
  raccept.Insert(rpair);
  
  // report
  raccept.Write();
  std::cout << std::endl;
  raccept.SWrite();
  std::cout << std::endl;

  // test file io
  std::cout << "======== serialisation " << std::endl;
  raccept.Write("tmp_raccept.txt");
  RabinAcceptance rareadback;
  rareadback.Read("tmp_raccept.txt");
  bool eq1= rareadback==raccept;
  if(eq1) {
    std::cout << "== readback ok" << std::endl;
  } else {
    std::cout << "== readback test case FAIL" << std::endl;
  } 
  std::cout << std::endl;

  // record test case
  FAUDES_TEST_DUMP("readback from file (expecy true)", eq1);
      

  // manipulate/inspect with the BaseVector interface, e.g. iterate over Rabin pairs
  RabinAcceptance::Iterator rit;
  rit=raccept.Begin();
  for(;rit!=raccept.End();++rit) {
    rpair=*rit;
    std::cout << "found a Robin pair with RSet " << rpair.RSet().ToString() << std::endl;   
  }
  std::cout << std::endl;

  // manipulate/inspect with the BaseVector interface, e.g. edit Rabin pair bhy iterator
  rit=raccept.Begin();
  rit->ISet().Erase(6); // this is why we use a vector and not a set ...
 
  // check euality operator // ... with sorting this would make much more sense
  bool eq2= rareadback==raccept;
  if(eq2) {
    std::cout << "== still equal? FAIL" << std::endl;
  } else {
    std::cout << "== sensed mismatch: ok" << std::endl;
  } 
  std::cout << std::endl;

  // record test case
  FAUDES_TEST_DUMP("readback from file (expecy true)", eq2);

  // report as 
  raccept.Write();
  std::cout << std::endl;

  
  ////////////////////////////////////////////////////
  // Testing strict XML and file IO
  ////////////////////////////////////////////////////

  // show
  std::cout << "======== strict XML serialisation" << std::endl;
  raccept.XWrite();
  std::cout << std::endl;

  // record test case
  FAUDES_TEST_DUMP("raccept stats",raccept);


  ////////////////////////////////////////////////////
  // Rabin automata basics
  ////////////////////////////////////////////////////

  // raed from file
  std::cout << "======== Rabin automaton from file" << std::endl;  
  RabinAutomaton ar;
  ar.Read("data/omg_rabinaut.gen");

  // show
  std::cout << "=== automaton" << std::endl;
  ar.XWrite();
  std::cout << "=== acceptance condition only" << std::endl;
  ar.RabinAcceptance().XWrite();
  std::cout << std::endl;
  std::cout << "=== edit in automaton" << std::endl;
  ar.RabinAcceptance().Begin()->RSet().Insert(11);
  ar.RabinAcceptance().XWrite();
  std::cout << "=== statistics" << std::endl;
  ar.SWrite();
  std::cout << std::endl;
  

  ////////////////////////////////////////////////////
  // Rabin automaton trim
  ////////////////////////////////////////////////////

  // read from file
  ar.Read("data/omg_rnottrim.gen");

  // copy for docs
  ar.Write("tmp_omg_rnottrim.gen");
  try {
    ar.GraphWrite("tmp_omg_rnottrim.png");
  } catch(faudes::Exception& exception) {
    std::cout << "omg_3_rabin: cannot execute graphviz' dot. " << std::endl;
  } 

  // show live states per Rabin pair
  raccept=ar.RabinAcceptance();  
  rit=raccept.Begin();
  for(;rit!=raccept.End();++rit) {
    StateSet inv;
    RabinLiveStates(ar,*rit,inv);    
    std::cout << "=== live states for Rabin pair " << rit->Name() << std::endl;
    ar.WriteStateSet(inv);
  }
  std::cout << std::endl;
  
  std::cout << "=== trim Rabin automaton" << std::endl;
  RabinTrim(ar);
  ar.Write();
  std::cout << std::endl;

  // copy for docs
  ar.Write("tmp_omg_rtrim.gen");
  try {
    ar.GraphWrite("tmp_omg_rtrim.png");
  } catch(faudes::Exception& exception) {
    std::cout << "omg_3_rabin: cannot execute graphviz' dot. " << std::endl;
  } 

}  

