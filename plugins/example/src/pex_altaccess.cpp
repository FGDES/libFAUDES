/** @file pex_altaccess.cpp Example plugin */

#include "pex_altaccess.h"
#include <stack>

namespace faudes {


// we use the alternative accessibility algorithm from tutorial 6 
// for our example plugin. 

void AlternativeAccessible(Generator& rGen) {
  // create a todo stack for state indices
  std::stack<Idx> todo;
  // create an empty StateSet for the set of accessible state
  StateSet accessible_states;
  // iterator for a StateSet
  StateSet::Iterator sit;
  // initialize the algorithm by pushing all initial states on the todo stack
  for (sit = rGen.InitStatesBegin(); sit != rGen.InitStatesEnd(); ++sit) {
    todo.push(*sit);
  }
  // process the todo stack until it's empty
  while (not todo.empty()) {
    // get and delete the next state index from the todo stack
    const Idx current = todo.top();
    todo.pop();
    // insert the current state in the set of accessible states
    accessible_states.Insert(current);
    // create transition iterator for the states of the current state
    TransSet::Iterator tit = rGen.TransRelBegin(current);
    TransSet::Iterator tit_end = rGen.TransRelEnd(current);
    // push successor states ton todo stack if not already discovered
    while (tit != tit_end) {
      if (not accessible_states.Exists(tit->X2)) {
        todo.push(tit->X2);
      }
      ++tit;
    }
  }
  // delete the states and transitions which are not accessible
  rGen.DelStates(rGen.States() - accessible_states);
}


} // namespace faudes
