/** @file 6_algorithm.cpp

Tutorial, implementing generator algorithms. 

The example function provides an
alternative implementation of the Accessible() method of the Generator
class, that removes all states from the Generator which are not accessible
through a transition path from an initial state.

@ingroup Tutorials 

@include 6_algorithm.cpp

*/



#include "corefaudes.h"


// for simplicity we make the faudes namespace available to our program

using namespace faudes;


/////////////////
// Example algorithm
/////////////////

void AlternativeAccessible(Generator& rGen) {

  // create a todo stack for state indices

  std::stack<Idx> todo;

  // create a empty StateSet for the set of accessible state

  StateSet accessible_states;

  // iterator for a StateSet

  StateSet::Iterator sit;

  // initialize the algorithm by pushing all initial states on the todo stack

  for(sit = rGen.InitStatesBegin(); sit != rGen.InitStatesEnd(); ++sit) {
    todo.push(*sit);
  }

  // process the todo stack until it's empty

  while(!todo.empty()) {

    // get the next state index from the todo stack

    const Idx current = todo.top();

    // delete the top element

    todo.pop();

    // insert the current state in the set of accessible states

    accessible_states.Insert(current);

    // create transition iterator for the states of the current state

    TransSet::Iterator tit = rGen.TransRelBegin(current);
    TransSet::Iterator tit_end = rGen.TransRelEnd(current);

    while (tit != tit_end) {

      // push successor states ton todo stack if not already discovered
      if(!accessible_states.Exists(tit->X2)) {
        todo.push(tit->X2);
      }

      // increment the transition iterator
      ++tit;
    }
  }

  // delete the states and transitions which are not accessible

  rGen.DelStates(rGen.States() - accessible_states);
}


/////////////////
// main program
/////////////////

int main() {

  // create a Generator

  Generator g1;

  // do some random "user interaction" stuff with the Generator g1

  g1.InsState("s1");
  g1.InsState("s2");
  g1.InsState("s3");
  g1.InsState("s4");
  g1.InsEvent("a");
  g1.InsEvent("b");
  g1.SetTransition("s1", "a", "s2");
  g1.SetTransition("s2", "a", "s3");
  g1.SetTransition("s3", "b", "s1");
  g1.SetInitState("s1");
  g1.SetMarkedState("s2");
  g1.SetMarkedState("s3");

  // write Generator to console in debugging mode

  g1.DWrite();

  // we call our example function "AlternativeAccessible", that makes the Generator's
  // set of states accessible

  AlternativeAccessible(g1);

  // write Generator to console in debugging mode

  g1.DWrite();


  return 0;
}


