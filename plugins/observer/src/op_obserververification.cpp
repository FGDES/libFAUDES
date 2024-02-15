/** @file op_obserververification.cpp 

Methods to verify the obsrver condition for natural projections.
The observer condition is, e.g., defined in 
K. C. Wong and W. M. Wonham, “Hierarchical control of discrete-event
systems,” Discrete Event Dynamic Systems: Theory and Applications, 1996.
In addition, methods to verify output control consistency (OCC) and 
local control consistency (LCC) are provided. See for example
K. Schmidt and C. Breindl, "On Maximal Permissiveness of Hierarchical and Modular Supervisory
Control Approaches for Discrete Event Systems," Workshop on Discrete Event Systems, 2008. 
*/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
   Exclusive copyright is granted to Klaus Schmidt

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */
#include "op_obserververification.h"


using namespace std;

namespace faudes {

bool IsObs(const Generator& rLowGen, const EventSet& rHighAlph){
	OP_DF("IsObs(" << rLowGen.Name() << "," << rHighAlph.Name() << ")");
	// Initialization of variables
	EventSet newHighAlph = rHighAlph;
	EventSet controllableEvents;
	map<Transition,Idx> mapChangedTrans;
	Generator genDyn(rLowGen);
	map<Transition,Transition> mapChangedTransReverse;
	map<Idx,Idx> mapStateToPartition;
	map<Idx, EventSet> mapRelabeledEvents;
	// One step of the observer algorithm: A dynamic system is computed that fulfills the one-step observer condition. 
	// if the result is equal to the original generator, then the natural projection on the high-level alphabet fulfills the observer property
	calculateDynamicSystemClosedObs(rLowGen, newHighAlph, genDyn);
	calculateDynamicSystemObs(rLowGen, newHighAlph, genDyn);
	Generator genPart;
	// compute coarsest quasi-congruence on the dynamic system
	ComputeBisimulation(genDyn, mapStateToPartition, genPart);
	// Extend the high-level alphabet according to the algorithm of Lei
        ExtendHighAlphabet(rLowGen, newHighAlph, mapStateToPartition);
	// return the result of the event extension
	return newHighAlph == rHighAlph;

}

bool IsMSA(const Generator& rLowGen, const EventSet& rHighAlph){
	OP_DF("IsMSA(" << rLowGen.Name() << "," << rHighAlph.Name() << ")");
	// Initialization of variables
	EventSet newHighAlph = rHighAlph;
	EventSet controllableEvents;
	map<Transition,Idx> mapChangedTrans;
	Generator genDyn(rLowGen);
	map<Transition,Transition> mapChangedTransReverse;
	map<Idx,Idx> mapStateToPartition;
	map<Idx, EventSet> mapRelabeledEvents;
	// One step of the observer algorithm: A dynamic system is computed that fulfills the one-step observer condition. 
	// if the result is equal to the original generator, then the natural projection on the high-level alphabet fulfills the observer property
	calculateDynamicSystemClosedObs(rLowGen, newHighAlph, genDyn);
        calculateDynamicSystemMSA(rLowGen, newHighAlph, genDyn);
 	Generator genPart;
	// compute coarsest quasi-congruence on the dynamic system
	ComputeBisimulation(genDyn, mapStateToPartition, genPart);
	// Extend the high-level alphabet according to the algorithm of Lei
        ExtendHighAlphabet(rLowGen, newHighAlph, mapStateToPartition);
	// return the result of the event extension
	return newHighAlph == rHighAlph;

}

bool IsOCC(const System& rLowGen, const EventSet& rHighAlph){
	OP_DF("IsOCC(" << rLowGen.Name() << "," << rHighAlph.Name() << ")");
	EventSet controllableEvents = rLowGen.ControllableEvents();
	// call the function that implements the algorithm
	return IsOCC(rLowGen, controllableEvents, rHighAlph);
}

bool IsOCC(const Generator& rLowGen, const EventSet& rControllableEvents, const EventSet& rHighAlph){
	OP_DF("IsOCC(" << rLowGen.Name() << "," << rHighAlph.Name() << ")");
	//helpers:
	StateSet::Iterator stIt, stEndIt;
	stIt = rLowGen.StatesBegin();
	stEndIt = rLowGen.StatesEnd();
	TransSet::Iterator tIt, tEndIt;
	// iteration over all states of rLowGen. If there is an uncontrollable feasible high-level event, backward reachability is conducted to determine if OCC holds. 
	for( ; stIt != stEndIt; stIt++){
		tIt = rLowGen.TransRelBegin(*stIt);
		tEndIt = rLowGen.TransRelEnd(*stIt);
		for( ; tIt != tEndIt; tIt++){
			if(rHighAlph.Exists(tIt->Ev) && !rControllableEvents.Exists(tIt->Ev) ){
				// check if all local backward paths are uncontrollable
				bool uncontrollable = backwardVerificationOCC(rLowGen, rControllableEvents, rHighAlph, *stIt);
				// if not all paths are uncontrollable, OCC is violated
				if(uncontrollable == false)
					return false;
				// otherwise, go to the next state
				else
					break;
			}
		}
	}
	return true;
}

bool backwardVerificationOCC(const Generator& rLowGen, const EventSet& rControllableEvents, const EventSet& rHighAlph, Idx currentState){
	OP_DF("backwardVerificationOCC(" << rLowGen.Name() << "," << rControllableEvents.Name() << "," << rHighAlph.Name() << "," << currentState << ")");
	// reverse transition relation
	TransSetX2EvX1 tset_X2EvX1;
	rLowGen.TransRel(tset_X2EvX1);
	TransSetX2EvX1::Iterator tsIt, tsEndIt;
	// todo list
	std::stack<Idx> todo;
	// algorithm: the locally backwards reachable states from current staet are 
	// evaluated. If a controllable event is found, OCC is violated.
	StateSet doneStates;
	doneStates.Insert(currentState);
	todo.push(currentState);
	// the local reachability is evaluated until no new state is found
	while( !todo.empty() ){
		const Idx current = todo.top();
		todo.pop();
		tsIt = tset_X2EvX1.BeginByX2(current);
		tsEndIt = tset_X2EvX1.EndByX2(current);
		for(; tsIt != tsEndIt; tsIt++){
			// if the current transition is labeled with a high-level evnet
			// it is skipped
			if(rHighAlph.Exists(tsIt->Ev) )
				continue;
			// if the current transition is controllable, OCC is violated
			else if( rControllableEvents.Exists(tsIt->Ev) )
				return false;
			else if( !doneStates.Exists(tsIt->X1) ){
				todo.push(tsIt->X1);
				doneStates.Insert(tsIt->X1);
			}
		}
	}
	return true;
}


bool IsLCC(const System& rLowGen, const EventSet& rHighAlph){
	OP_DF("IsLCC(" << rLowGen.Name() << "," << rHighAlph.Name() << ")");
	EventSet controllableEvents = rLowGen.ControllableEvents();
	// call the function that implements the algorithm
	return IsLCC(rLowGen, controllableEvents, rHighAlph);
}


/*

In my opinion, the below algorithm to test LCC is incorrect. 
It gives false negatives; e.g. stackfeeder example from 
LRT "Uebung 12", data given below. I've
temporarily replaced IsLCC by a naive algorithm
which should be correct. This topic needs further 
attentio .. tmoor, 07/20012


<Generator> 
"Generator"    
<Alphabet>
"wpar"          "mv"            +C+           "wplv"          "r"             "nr"           
"stp"           +C+          
</Alphabet>

<States>
<Consecutive>
1              8             
</Consecutive>
</States>

<TransRel>
1              "wpar"          2             
2              "mv"            3             
3              "nr"            4             
4              "wplv"          5             
5              "wpar"          8             
5              "r"             6             
6              "wpar"          7             
6              "stp"           1             
7              "stp"           2             
8              "r"             7             
</TransRel>

<InitStates>   1 </InitStates>
<MarkedStates> 1 </MarkedStates>
</Generator>

with high-level alphabet wpar and mv
*/

/*
bool IsLCC(const Generator& rLowGen, const EventSet& rControllableEvents, const EventSet& rHighAlph){
  OP_DF("IsLCC(" << rLowGen.Name() << "," << rHighAlph.Name() << ")");
  // reverse transition relation
  TransSetX2EvX1 tset_X2EvX1;
  rLowGen.TransRel(tset_X2EvX1);
  //helpers:
  StateSet::Iterator stIt, stEndIt;
  stIt = rLowGen.StatesBegin();
  stEndIt = rLowGen.StatesEnd();
  TransSet::Iterator tIt, tEndIt;
  StateSet doneStates;
  map<Idx, bool> localStatesMap;
  map<Idx, bool>::const_iterator lsIt, lsEndIt;
  // iteration over all states of rLowGen. If there is an uncontrollable feasible high-level event, 
  // backward reachability is conducted to determine if LCC holds. 
  for( ; stIt != stEndIt; stIt++){
    tIt = rLowGen.TransRelBegin(*stIt);
    tEndIt = rLowGen.TransRelEnd(*stIt);
    for( ; tIt != tEndIt; tIt++){
      if(rHighAlph.Exists(tIt->Ev) && !rControllableEvents.Exists(tIt->Ev) ){
        doneStates.Clear();
        localStatesMap.clear();
        localStatesMap[*stIt] = false;
        doneStates.Insert(*stIt);
        // check if for all backward reachable states, a local uncontrollable backward paths exists
        FD_WARN("IsLCC(): analyse state " << rLowGen.SStr(*stIt));
        backwardVerificationLCC(tset_X2EvX1, rControllableEvents, rHighAlph, *stIt, *stIt, false, localStatesMap, doneStates);
        // if for some state, all paths are controllable, LCC is violated
        lsIt = localStatesMap.begin();
        lsEndIt = localStatesMap.end();
        for( ; lsIt != lsEndIt; lsIt++){
          // if there is a state with only controllable paths, LCC is violated
          if(lsIt->second == true) {
            FD_WARN("IsLCC(): fail for state " << lsIt->first);
            return false; 
	  }
        }
        // the evaluation for the current state is finished
        break;
      }
    }
  }
  FD_WARN("IsLCC(): pass");
  return true;
}


void backwardVerificationLCC(
  const TransSetX2EvX1& rTransSetX2EvX1, 
  const EventSet& rControllableEvents, 
  const EventSet& rHighAlph, 
  Idx exitState, 
  Idx currentState, 
  bool controllablePath, 
  map<Idx, bool>& rLocalStatesMap, 
  StateSet& rDoneStates)
{
  OP_DF("backwardVerificationLCC(rTransSetX2EvX1," << 
      rControllableEvents.Name() << "," << rHighAlph.Name() << "," << exitState << "," 
    << currentState << "," << controllablePath << ",rExitLocalStatesMap, rDoneStates)");
  FD_WARN("IsLCC(): b-reach at state " << currentState);
  // go along all backward transitions. Discard the goal state if it is reached via a high-level event or if it is in the rDoneStates and 
  // the controllability properties of the state do not change on the current path
    
  // helpers
  TransSetX2EvX1::Iterator tsIt, tsEndIt;
  tsIt = rTransSetX2EvX1.BeginByX2(currentState);
  tsEndIt = rTransSetX2EvX1.EndByX2(currentState);
  bool currentControllablePath;
  // we iterate over all backward transitions of the currentState to establish backward reachability
  for( ;tsIt != tsEndIt; tsIt++){
    // states reachable via a high-level event 
    // are not in the local backward reach and the controllability property of the current exitState does not change
    if( !rHighAlph.Exists(tsIt->Ev) && tsIt->X1 != exitState){
      // if the state has not been visited, yet, the controllability of the current path are set in the rExitLocalStatesMap
      if( !rDoneStates.Exists(tsIt->X1) ){
	rDoneStates.Insert(tsIt->X1);
	// the path is uncontrollable if the current transition has an uncontrollable event or the path was already uncontrollable
	currentControllablePath = rControllableEvents.Exists(tsIt->Ev) || controllablePath;
        FD_WARN("IsLCC(): record new state " << tsIt->X1 << " cntr=" << currentControllablePath);
	rLocalStatesMap[tsIt->X1] = currentControllablePath;
	// as the state has not been visited, yet, it is subject to a new backward reachability
	backwardVerificationLCC(rTransSetX2EvX1, rControllableEvents, rHighAlph, exitState, tsIt->X1, currentControllablePath, rLocalStatesMap, rDoneStates);
      }
      else{ // for an existing state, the controllability value can change from controllable to uncontrollable (if 
	// a new uncontrollable path has been found). It is important to note, that the LCC condition implies that
	// if there is one uncontrollable path, then the state is flagged uncontrollable except for the case of the 
	// given exitState that is always uncontrollable
	currentControllablePath = rControllableEvents.Exists(tsIt->Ev) || controllablePath;
	if(rLocalStatesMap[tsIt->X1] != currentControllablePath && currentControllablePath == false){
	  rLocalStatesMap[tsIt->X1] = false;
	  // as the controllabiity attribute of the current state changed it is subject to a new backward reachability
	  backwardVerificationLCC(rTransSetX2EvX1, rControllableEvents, rHighAlph, exitState, tsIt->X1, false, rLocalStatesMap, rDoneStates);
	}
      }
    }
  }
}

*/

 
// Naive replacement to test LCC with std. forward search for each state
// (complexity is quadratic in the number of states and 
// linear in the number of events; actual performance is suboptimal, 
// but I don't see how to get lower complexity)

bool IsLCC(const Generator& rLowGen, const EventSet& rControllableEvents, const EventSet& rHighAlph){
  OP_DF("IsLCC(" << rLowGen.Name() << "," << rHighAlph.Name() << ")");
  // iteration over all states of rLowGen. 
  StateSet::Iterator sit=rLowGen.StatesBegin();
  for( ; sit != rLowGen.StatesEnd(); sit++){
    // forward reachablity to figure all next high-level events
    StateSet freach;
    StateSet todo;
    EventSet hievs;
    todo.Insert(*sit);
    while(!todo.Empty()) {
      // take from todo stack
      Idx cs = *todo.Begin();
      todo.Erase(todo.Begin());
      freach.Insert(cs);
      // figure next states
      TransSet::Iterator tit = rLowGen.TransRelBegin(cs);
      TransSet::Iterator tit_end=rLowGen.TransRelEnd(cs);
      for( ; tit != tit_end; tit++){
        if(rHighAlph.Exists(tit->Ev)) {
          hievs.Insert(tit->Ev);
	  continue;
        }
        if(!freach.Exists(tit->X2)) {
	  todo.Insert(tit->X2);
	}
      }
    }
    // forward reachablity to figure next high-level events by uncontr. paths
    EventSet hievs_ucpath;
    todo.Clear();
    freach.Clear();
    todo.Insert(*sit);
    while(!todo.Empty()) {
      // take from todo stack
      Idx cs = *todo.Begin();
      todo.Erase(todo.Begin());
      freach.Insert(cs);
      // figure next states
      TransSet::Iterator tit = rLowGen.TransRelBegin(cs);
      TransSet::Iterator tit_end=rLowGen.TransRelEnd(cs);
      for( ; tit != tit_end; tit++){
        if(rControllableEvents.Exists(tit->Ev)) {
	  continue;
	}
        if(rHighAlph.Exists(tit->Ev)) {
          hievs_ucpath.Insert(tit->Ev);
	  continue;
        }
        if(!freach.Exists(tit->X2)) {
	  todo.Insert(tit->X2);
	}
      }
    }
    // test lcc
    hievs = hievs - rControllableEvents;
    hievs_ucpath = hievs_ucpath - rControllableEvents;
    if(hievs_ucpath != hievs) {
      OP_DF("IsLCC: failed at state " << rLowGen.SStr(*sit))
      return false;
    }
  }
  // no issues found
  OP_DF(y"IsLCC: passed");
  return true;
}


}// namespace faudes
