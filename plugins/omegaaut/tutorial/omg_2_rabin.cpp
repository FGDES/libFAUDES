/** @file omg_2_rabin.cpp 

Tutorial ond  Rabin automata

@ingroup Tutorials 

@include omg_2_rabin.cpp

*/

#include "libfaudes.h"

using namespace faudes;

int main() {

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
  if(rareadback==raccept) {
    std::cout << "== readback ok" << std::endl;
  } else {
    std::cout << "== readback test case FAIL" << std::endl;
  } 
  std::cout << std::endl;
      

  // manipulate/inspect with the BaseSet interface, e.g. iterate over Rabin pairs
  RabinAcceptance::Iterator rit;
  rit=raccept.Begin();
  for(;rit!=raccept.End();++rit) {
    rpair=*rit;
    std::cout << "found a Robin pair with RSet " << rpair.RSet().ToString() << std::endl;   
  }
  std::cout << std::endl;

  // manipulate/inspect with the BaseSet interface, e.g. edit Rabin pair bhy iterator
  rit=raccept.Begin();
  // rit->ISet().Erase(6); // cant do this as it would mess up the ordered set
  RabinPair rpedit=*raccept.Erase(rit); // take a Rabin Pair
  rpedit.ISet().Erase(6);               // edit that Rabin pair
  raccept.Insert(rpedit);               // ort it back in		       

  // check euality operator 
  if(rareadback==raccept) {
    std::cout << "== still wqual? FAIL" << std::endl;
  } else {
    std::cout << "== sensed mismatch: ok" << std::endl;
  } 
  std::cout << std::endl;


  // report as 
  raccept.Write();
  std::cout << std::endl;

  
  ////////////////////////////////////////////////////
  // Testing strict XML and file IO
  ////////////////////////////////////////////////////

  // show
  std::cout << "======== strict XML serialisation" << std::endl;
  raccept.XWrite();

  // record test case
  FAUDES_TEST_DUMP("raccept stats",raccept);


  ////////////////////////////////////////////////////
  // Rabin automaton
  ////////////////////////////////////////////////////

  
  RabinAutomaton ar;
  ar.Read("data/omg_rabinaut.gen");
  ar.Write();
  

}  

