/** @file pev_1_priorities.cpp 

Tutorial, events stes and generators with priorities

@ingroup Tutorials 

@include pev_1_priorities.cpp

*/

#include "libfaudes.h"


using namespace faudes;



int main() {

  ////////////////////////////////////////////////////
  // TaEventSet with Attribute AttributePriority
  ////////////////////////////////////////////////////

  // set up some alphabet with prioritised events
  TaEventSet<AttributePriority> prios1;
  Idx alpha= prios1.Insert("alpha");
  Idx beta=  prios1.Insert("beta");
  Idx gamma= prios1.Insert("gamma");
  AttributePriority prio;
  prio.Priority(10);
  prios1.Attribute(alpha,prio);
  prio.Priority(20);
  prios1.Attribute(beta,prio);
  prio.Priority(30);
  prios1.Attribute(gamma,prio);
  // report
  std::cout << "alphabet of priorisied events (using vanilla attributed event set)" << std::endl;
  prios1.XWrite();


  ////////////////////////////////////////////////////
  // Convenienceclass PriositisedAlphabet (fully registered faudes type)
  ////////////////////////////////////////////////////

  // set up some alphabet with prioritised events
  EventPriorities prios2;
  alpha= prios2.Insert("alpha");
  beta=  prios2.Insert("beta");
  gamma= prios2.Insert("gamma");
  prios2.Priority(alpha,13);    // access by index
  prios2.Priority("beta",23);   // access by symbolic name (convenience, performance penalty)
  prios2.Priority("gamma",23);  // access by symbolic name (convenience, performance penalty)  
  // report
  std::cout << "alphabet of priorisied events (using convenience class)" << std::endl;
  prios2.Write();

  // test copy/casting/equality
  EventSet events(prios2);
  bool ok= events == prios2;
  if(ok)
    std::cout << "copy to plain alphabet: events match (PASS)" << std::endl;
  else	      
    std::cout << "copy to plain alphabet: events mismatch (FAIL)" << std::endl;
  EventPriorities prios3(prios2);
  ok= prios3 == prios2;
  if(ok)
    std::cout << "copy to prio alphabet: events match (PASS)" << std::endl;
  else	      
    std::cout << "copy to prio alphabet: events mismatch (FAIL)" << std::endl;
  ok= prios2.EqualAttributes(prios3);
  if(ok)
    std::cout << "copy to prio alphabet: priorities match (PASS)" << std::endl;
  else	      
    std::cout << "copy to prio alphabet: priorities mismatch (FAIL)" << std::endl;
  prios3.Priority("alpha",50);
  prios3.Write();
  ok=prios2.EqualAttributes(prios3);
  if(ok)
    std::cout << "manipulate priorities: still match (FAIL)" << std::endl;
  else	      
    std::cout << "manipulate priorities: mismatch (PASS)" << std::endl;




  ////////////////////////////////////////////////////
  // pGenerator
  ////////////////////////////////////////////////////
  pGenerator pgen;
  pgen.FromString(R"(
    <Generator>
      <T>
        1 alpha 2
        2 beta  3
        3 gamma 1
      </T>
    </Generator>
  )");
  pgen.Priorities(prios3);
  pgen.XWrite();
  


}
