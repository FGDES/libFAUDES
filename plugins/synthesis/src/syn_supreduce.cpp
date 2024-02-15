/** @file syn_supreduce.cpp Supervisor Reduction */

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
   

#include "syn_supreduce.h"

/* turn on debugging for this file */
//#undef FD_DF
//#define FD_DF(a) FD_WARN(a);


namespace faudes {




/*
***************************************************************************************
***************************************************************************************
 Implementation
***************************************************************************************
***************************************************************************************
*/


/** 
 * Data structure for identifying states in the same coset for supervisor reduction
 */
struct ReductionStateInfo{
    StateSet mPlantStates;
    EventSet mEnabledEvents;
    EventSet mDisabledEvents;
    bool mMarkedState;
    bool mPlantMarked;
};


/*
Reduction mergibility algorithm

This recursive algorithm determines if two supervisor states can be merged to the same coset. 
It is called by the main procedure SupReduce. 
-- stateI, stateJ: pair of states to be tested
-- rWaitList: list of waiting state pairs
-- cNode: record first state to be checked
-- rSupGen: const ref to supervisor 
-- rSupStateInfo: ref to state info
-- rState2Class: const ref to state->coset map
-- rClass2State: const ref to coset->states

return True if the classes of the two states can be merged
*/

bool TestMergibility(
  Idx stateI, Idx stateJ,
  std::vector<std::set<Idx> >& rWaitList,
  Idx cNode,
  const System& rSupGen,
  const std::map<Idx,ReductionStateInfo>& rSupStateInfo,
  const std::map<Idx,Idx>& rState2Class,
  const std::vector<StateSet>& rClass2States)
{
  FD_DF("TestMergebility: stateI " << stateI << " stateJ " << stateJ);
  // Loop through all state combinations in the current classes 
  //std::cout
  /* for(unsigned int i = 0; i < rWaitList.size(); i++)
    std::cout << *rWaitList[i].begin() << " " << *(++rWaitList[i].begin() ) << std::endl;*/
  StateSet::Iterator siIt, siEndIt, sjIt, sjEndIt;
  StateSet statesI, statesJ;
  statesI = rClass2States[rState2Class.at(stateI)]; // all states of the class of stateI
  statesJ = rClass2States[rState2Class.at(stateJ)]; // all states of the class of stateJ
  // add the states on the waitlist to statesI and statesJ
  for(unsigned int i = 0; i < rWaitList.size(); i++){
      // StateI: add classes for corresponding state on waitList   
      if(*rWaitList[i].begin() == stateI )
	statesI = statesI + rClass2States[rState2Class.find(*(++rWaitList[i].begin() ) )->second];
      if(*(++rWaitList[i].begin() ) == stateI )
	statesI = statesI + rClass2States[rState2Class.find(*rWaitList[i].begin() )->second];
      // StateJ: add classes for corresponding state on waitList
      if(*rWaitList[i].begin() == stateJ )
	statesJ = statesJ + rClass2States[rState2Class.find(*(++rWaitList[i].begin() ) )->second];
      if(*(++rWaitList[i].begin() ) == stateJ )
	statesJ = statesJ + rClass2States[rState2Class.find(*rWaitList[i].begin() )->second];
  }
  //FD_DF("Test: statesI " << statesI.ToString() << " stateJ " << statesJ.ToString());
  siIt = statesI.Begin(); // Iterators for states of class for stateI
  siEndIt = statesI.End();
  sjIt = statesJ.Begin(); // Iterators for states of class for stateJ
  sjEndIt = statesJ.End();  
  std::set<Idx> statePair;
  for( ; siIt != siEndIt; siIt++){// loop over states for stateI
    sjIt = statesJ.Begin();
    for(; sjIt != sjEndIt; sjIt++){ // loop over states for stateJ
      // only look at state pairs that are not already in the same class2ReducedStates
      if(rClass2States[rState2Class.find(*siIt)->second].Exists(*sjIt) )
	continue;
      if(*siIt == *sjIt)
	continue;

      statePair.clear();
      statePair.insert(*siIt);
      statePair.insert(*sjIt);
      //std::cout << "mergibility states: " << *siIt << " and " << *sjIt << std::endl;

      bool continueLoop = false;
      for(unsigned int i = 0; i < rWaitList.size(); i++)
	if(rWaitList[i] == statePair){
	  continueLoop = true;
	  break;
	}
      if(continueLoop == true)// the current state pair is already on the waiting list
	  continue;
      
      // tmoor: fix nonstandard std::map::at()
      const ReductionStateInfo& siinf=rSupStateInfo.find(*siIt)->second;
      const ReductionStateInfo& sjinf=rSupStateInfo.find(*sjIt)->second;

      // Test whether the state pair belongs to the control relation \mathcal{R}: 
      // E(*siIt) \cap D(*sjIt) = E(*sjIt) \cap D(*siIt) = \emptyset
      // and C(*siIt) = C(*sjIt) \Rightarrow M(*siIt) = M(*sjIt)
      //if( !(siinf.mEnabledEvents * sjinf.mDisabledEvents).Empty() ) return false;
      //if( !(sjinf.mEnabledEvents * siinf.mDisabledEvents).Empty() ) return false;
      if( !(siinf.mEnabledEvents.Disjoint(sjinf.mDisabledEvents)) ) return false;
      if( !(sjinf.mEnabledEvents.Disjoint(siinf.mDisabledEvents)) ) return false;
      
      // Test whether the marking of the states is consistent
      if( (siinf.mPlantMarked == sjinf.mPlantMarked) && (siinf.mMarkedState != sjinf.mMarkedState) )
	return false;



      /*
      // original (for reference)
  
      // Test if the state pair belongs to the control relation \mathcal{R}: 
      // E(*siIt) \cap D(*sjIt) = E(*sjIt) \cap D(*siIt) = \emptyset and C(*siIt) = C(*sjIt) \Rightarrow M(*siIt) = M(*sjIt)
      if( !(rSupStateInfo.at(*siIt).mEnabledEvents * rSupStateInfo.at(*sjIt).mDisabledEvents).Empty() || 
	!(rSupStateInfo.at(*sjIt).mEnabledEvents * rSupStateInfo.at(*siIt).mDisabledEvents).Empty() ) 
	return false;
      
      // Test if the marking of the states is consistent
      if( (rSupStateInfo.at(*siIt).mPlantMarked == rSupStateInfo.at(*sjIt).mPlantMarked) &&
          (rSupStateInfo.at(*siIt).mPlantMarked != rSupStateInfo.at(*sjIt).mPlantMarked) )
	return false;

      */     
      rWaitList.push_back(std::set<Idx>() );
      rWaitList.back().insert(*siIt);
      rWaitList.back().insert(*sjIt);

      EventSet sharedEvents = rSupGen.ActiveEventSet(*siIt) * rSupGen.ActiveEventSet(*sjIt);
      EventSet::Iterator eIt, eEndIt;
      eIt = sharedEvents.Begin();
      eEndIt = sharedEvents.End();
      Idx goalStateI, goalStateJ;
      for( ; eIt != eEndIt; eIt++){// go over all shared active events of the current states
	goalStateI = (rSupGen.TransRelBegin(*siIt,*eIt) )->X2;
	goalStateJ = (rSupGen.TransRelBegin(*sjIt,*eIt) )->X2;
	if(*rState2Class.find( goalStateI ) == *rState2Class.find( goalStateJ ) )// event leads to same class
	  continue;
	statePair.clear();
	statePair.insert(goalStateI);
	statePair.insert(goalStateJ);
	continueLoop = false;
	for(unsigned int i = 0; i < rWaitList.size(); i++){
	  if(rWaitList[i] == statePair)// the current goal state pair is already on the waiting list
	  continueLoop = true;
	  break;
	}
	if(continueLoop == true)// the current state pair is already on the waiting list
	  continue;
	  
	// find classes of goalStateI and goalStateJ and check if they are already merged
	if( *(rClass2States.at( rState2Class.find(goalStateI)->second ).Begin() ) < cNode)
	  return false;
	if( *(rClass2States.at(rState2Class.find(goalStateJ)->second ).Begin() ) < cNode)
	  return false;      
	bool flag = TestMergibility(goalStateI, goalStateJ, rWaitList, cNode, rSupGen, rSupStateInfo, rState2Class, rClass2States);
	if(flag == false){
	  return false;
	}
      }
    }
  }
  return true;
}




// SupReduce(rPlantGen, rSupGen, rReducedSup)
bool SupReduce(const System& rPlantGen, const System& rSupGen, System& rReducedSup) {
  FD_DF("SupReduce()");

  // CONSISTENCY CHECK:

  // alphabets must match
  if (rPlantGen.Alphabet() != rSupGen.Alphabet()) {
    EventSet only_in_plant = rPlantGen.Alphabet() - rSupGen.Alphabet();
    EventSet only_in_spec = rSupGen.Alphabet() - rPlantGen.Alphabet();
    std::stringstream errstr;
    errstr << "Alphabets of generators do not match. Only in plant: " 
      << only_in_plant.ToString() << ". Only in spec: "
      << only_in_spec.ToString() << ".";
    throw Exception("SupReduce", errstr.str(), 100);
  }

  // plant and spec must be deterministic
  bool plant_det = rPlantGen.IsDeterministic();
  bool sup_det = rSupGen.IsDeterministic();

  if ((plant_det == false) && (sup_det == true)) {
    std::stringstream errstr;
    errstr << "Plant generator must be deterministic, " << "but is nondeterministic";
    throw Exception("SupReduce", errstr.str(), 201);
  }
  else if ((plant_det == true) && (sup_det == false)) {
    std::stringstream errstr;
    errstr << "Supervisor generator must be deterministic, " << "but is nondeterministic";
    throw Exception("SupReduce", errstr.str(), 203);
  }
  else if ((plant_det == false) && (sup_det == false)) {
    std::stringstream errstr;
    errstr << "Plant and supervisor generator must be deterministic, "
                    << "but both are nondeterministic";
    throw Exception("SupReduce", errstr.str(), 204);
  }
  // Clear the result generator rReducedsup
  rReducedSup.Clear();

  // HELPERS:
  System previousSupReduced = rSupGen;
  std::vector<StateSet> class2States; // control equivalent states in supervisor
  std::map<Idx, Idx> state2Class; // map from state Idx to its control equivalent class
  std::vector<std::set<Idx> > waitList; // list of states for  classes to be merged
  EventSet alwaysEnabledEvents = rSupGen.Alphabet(); // set of events that are never disabled
  // Initialize the helpers that store information about the equivalence classes
  StateSet::Iterator sIt, sEndIt;
  sIt = rSupGen.States().Begin();
  sEndIt = rSupGen.States().End();
  for(; sIt != sEndIt; sIt++){ // Note: States are ordered by index
      class2States.push_back(StateSet() );
      class2States.back().Insert(*sIt); 
      state2Class[*sIt] = class2States.size() - 1;
  }
  // Evaluate the composition of plant and supervisor in order to classify corresponding states
  System tmp;
  std::map<std::pair<Idx,Idx>, Idx> reverseCompositionMap;
  std::map<std::pair<Idx,Idx>, Idx>::const_iterator rcIt, rcEndIt;
  Parallel(rPlantGen, rSupGen, reverseCompositionMap, tmp);
  rcIt = reverseCompositionMap.begin();
  rcEndIt = reverseCompositionMap.end();
  std::map<Idx,ReductionStateInfo> supStateInfo;
  std::map<Idx,ReductionStateInfo>::iterator rsIt;
  // Find the plant states that belong to each supervisor state
  for(; rcIt != rcEndIt; rcIt++){
      rsIt = supStateInfo.find(rcIt->first.second);
      if(rsIt == supStateInfo.end() )
	supStateInfo[rcIt->first.second] = ReductionStateInfo();

      supStateInfo[rcIt->first.second].mPlantStates.Insert(rcIt->first.first);
  }
  /*
  FD_DF("SupReduce(): states per supervisor state ");
  for(rsIt = supStateInfo.begin(); rsIt != supStateInfo.end(); rsIt++)
    FD_DF("sup state: " << rsIt->first << " plant states " << rsIt->second.mPlantStates.ToString());
  */
  // Determine the state properties for all supervisor stateset
  for(rsIt = supStateInfo.begin(); rsIt != supStateInfo.end(); rsIt++){
      rsIt->second.mEnabledEvents = rSupGen.ActiveEventSet(rsIt->first); // all events enabled at current state2Class
      sIt = rsIt->second.mPlantStates.Begin();
      sEndIt = rsIt->second.mPlantStates.End();
      rsIt->second.mPlantMarked = false;
      for(; sIt != sEndIt; sIt++){
	 rsIt->second.mDisabledEvents = rsIt->second.mDisabledEvents + rPlantGen.ActiveEventSet(*sIt); // compute active events in plant_det for state *sIt
	 rsIt->second.mPlantMarked = rsIt->second.mPlantMarked || rPlantGen.ExistsMarkedState(*sIt); // compute colors of corresponding plant states 
      }
	
      rsIt->second.mDisabledEvents = rsIt->second.mDisabledEvents - rsIt->second.mEnabledEvents; // compute disable events (events that are not enabled
      rsIt->second.mMarkedState = rSupGen.ExistsMarkedState(rsIt->first);
      alwaysEnabledEvents = alwaysEnabledEvents -  rsIt->second.mDisabledEvents; // subtract disabled events from always enabled events
  }
  FD_DF("SupReduce(): Always enabled events: " << alwaysEnabledEvents.ToString());
  // if no events are disabled, then the reduced supervisor has only one state without events
  if(rSupGen.Alphabet() == alwaysEnabledEvents){
      Idx state = rReducedSup.InsState();
      rReducedSup.SetMarkedState(state);
      rReducedSup.SetInitState(state);
      return true;
  }
  /*for(rsIt = supStateInfo.begin(); rsIt != supStateInfo.end(); rsIt++)
      std::cout << "state: " << rsIt->first << " enabled: " << rsIt->second.mEnabledEvents.ToString() << " disabled: " << rsIt->second.mDisabledEvents.ToString() << std::endl;*/ // REMOVE
  
  std::map<Idx,bool> usedEventsMap;
  EventSet::Iterator eIt = alwaysEnabledEvents.Begin();
  for( ; eIt != alwaysEnabledEvents.End(); eIt++)// map that indicates if always enabled event is relevant for supervisor (true) or not (false)
    usedEventsMap[*eIt] = false;
  // ==========================
  // Algorithm
  //===========================
  // go through all supervisor states
  std::map<Idx,Idx>::const_iterator mIt;
  std::map<Idx,Idx>::const_iterator mEndIt = state2Class.end();
  mEndIt--;
  std::map<Idx,Idx>::const_iterator mbIt, mbEndIt;
  mbEndIt = state2Class.end();
  
 /* std::cout << "Classes to states" << std::endl;
  for(unsigned int i = 0; i < class2States.size(); i ++)
     std::cout << "class: " << i << " state " << class2States[i].ToString() << std::endl;
  
  std::cout << "State to class" << std::endl;
  for(mIt = state2Class.begin(); mIt != state2Class.end(); mIt++)
    std::cout << "state: " << mIt->first << " class: " << mIt->second << std::endl; */ // REMOVE
  
  for(mIt = state2Class.begin(); mIt != mEndIt; mIt++){
    // Evaluate min{k \in I | x_k \in  [x_i]}; since StateSets are ordered by index, this simply means finding the first state in the class of x_i
    if( mIt->first > *class2States[mIt->second].Begin() ){// state is already in other equivalence class 
      continue;
    }
    mbIt = ++mIt;
    mIt--;
    for(; mbIt != mbEndIt; mbIt++) {// approach from back
	//if(j > min{k \in I  | x_k \in [x_j]}
	if(mbIt->first > *class2States[mbIt->second].Begin() ){
	  continue;
	}
        FD_DF("SupReduce(): loop state i " << mIt->first << "state j " << mbIt->first);
	
	// Start actual algorithm after filtering
	waitList.clear();
	//if((mIt->first == 1 && mbIt->first == 4) ){
	//waitList.push_back(std::set<Idx>() );
	//waitList.back().insert(mIt->first);
	//waitList.back().insert(mbIt->first); REMOVE
	//}
	bool flag = TestMergibility(mIt->first,mbIt->first,waitList,mIt->first, rSupGen, supStateInfo, state2Class, class2States);
	if(flag == true){// merge classes indicated by waitList
            FD_DF("SupReduce(): merging");
	    std::vector<std::set<Idx> >::const_iterator wlIt, wlEndIt;
	    //std::cout << "size of weitlist " << waitList.size() << std::endl;
	    wlIt = waitList.begin();
	    wlEndIt = waitList.end();
	    for(; wlIt != wlEndIt; wlIt++){// go through waiting list
		//std::cout << " states " << *wlIt->begin() << " " << *(++wlIt->begin() ) << std::endl;
		if(state2Class[*wlIt->begin() ] == state2Class[*(++(wlIt->begin() ) ) ])// no action is required if the states are already in the same class
		  continue;
		class2States[state2Class[*wlIt->begin() ] ] = class2States[state2Class[*wlIt->begin()] ] + class2States[state2Class[*(++(wlIt->begin() ) ) ] ]; // union of state sets of both classes
		Idx removeClass = state2Class[*(++(wlIt->begin() ) ) ];
		sIt = class2States[removeClass ].Begin();
		sEndIt = class2States[removeClass ].End();
		for(; sIt != sEndIt; sIt++)
		  state2Class[*sIt] = state2Class[*wlIt->begin() ]; // change class of all states that were merged
		  
		class2States[removeClass ].Clear(); // clear merged class 

	    /*std::cout << "Classes to states" << std::endl;
	    for(unsigned int i = 0; i < class2States.size(); i ++)
	      std::cout << "class: " << i << " state " << class2States[i].ToString() << std::endl;
	    
	    std::cout << "State to class" << std::endl;
	     std::map<Idx,Idx>::const_iterator cIt;
	    for(cIt = state2Class.begin(); cIt != state2Class.end(); cIt++)
	      std::cout << "state: " << cIt->first << " class: " << cIt->second << std::endl; */ // REMOVE
	    }
	}
    }
  }

  // ===============================
  // Construct the reduced superisor
  // ===============================
  // Every state corresponds to a class that we found and we try to avoid adding trnasitions with always enabled events
  FD_DF("SupReduce(): construct quitient");
  std::map<Idx,Idx> class2ReducedStates;
  Idx newStateIdx;
  rReducedSup.InjectAlphabet(rSupGen.Alphabet() );
  // First generate one state for each class in the reduced generator
  for(unsigned int i = 0; i < class2States.size(); i++){
    if(class2States[i].Empty() == true)// if the state set is empty, then the class is not used
	continue;
    else{// create new state in the reduced supervisor for the class
      newStateIdx = rReducedSup.InsState();
      class2ReducedStates[i ] = newStateIdx; // save state for the class
    }
  }// all states of the reduced generator are now generated and stored

  // Now add the transitions to the reduced generator
  TransSet::Iterator tIt, tEndIt;
  Idx newGoalState; // goal state for transition to be added
  for(unsigned int i = 0; i < class2States.size(); i++){
    if(class2States[i].Empty() == true)// if the state set is empty, then the class is not used
	continue;
    sIt = class2States[i].Begin();
    sEndIt = class2States[i].End();
    newStateIdx = class2ReducedStates[i]; 
    for(; sIt != sEndIt; sIt++){// go through all states of the current class
	if(rSupGen.ExistsInitState(*sIt) )// determine the initial state of the reduced supervisor
	  rReducedSup.InsInitState(newStateIdx);
	
	if(rSupGen.ExistsMarkedState(*sIt) )
	  rReducedSup.SetMarkedState(newStateIdx); // insert the supervisor colors per state
	
	tIt = rSupGen.TransRelBegin(*sIt); // transitions of state *sIt in supervisor
	tEndIt = rSupGen.TransRelEnd(*sIt); 
	for( ; tIt != tEndIt; tIt++){
	  newGoalState = class2ReducedStates[state2Class[tIt->X2] ]; // goal state of transition in the reduced supervisor
	  if(alwaysEnabledEvents.Exists(tIt->Ev) == true && newGoalState != newStateIdx )// always enabled event changes class and is thus relevant for supervisor
	    usedEventsMap[tIt->Ev] = true;
	  
	  rReducedSup.SetTransition(newStateIdx, tIt->Ev, newGoalState); 
	}
    }
  }
  if(previousSupReduced.Size() == rReducedSup.Size() ){
    std::map<Idx,bool>::const_iterator uIt = usedEventsMap.begin();
    for(; uIt != usedEventsMap.end(); uIt++){// delete the unused events from the reduced supervisor
      if(uIt->second == false){
	rReducedSup.DelEvent(uIt->first);
      }
    }
  }
  else{
      previousSupReduced.Clear();
      Deterministic(rReducedSup,previousSupReduced);
      SupReduce(rPlantGen,previousSupReduced,rReducedSup);
  }
  FD_DF("SupReduce(): done");
  return true;
}



} // name space 
