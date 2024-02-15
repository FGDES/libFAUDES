/** @file cfl_determin.cpp powerset construction */

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


#include "cfl_determin.h"

namespace faudes {

// UniqueInit(rGen&)
void UniqueInit(Generator& rGen) {
  Idx inituni;
  StateSet::Iterator lit;
  TransSet::Iterator tit;
  // check number of initial states
  if(rGen.InitStatesSize() == 1) return;
  // introduce new initial state
  if(rGen.StateNamesEnabled()) {
    std::string initname=rGen.UniqueStateName("InitUni");
    inituni = rGen.InsState(initname); 
  } else {
    inituni = rGen.InsState();
  }
  FD_DF("UniqueInit: introducing new initial state: " << inituni);
  // introduce outgoing transitions from initial state
  FD_DF("UniqueInit: introduce outgoing transitions: ");
  for (lit = rGen.InitStatesBegin(); lit != rGen.InitStatesEnd(); ++lit) {
    for (tit = rGen.TransRelBegin(*lit); tit != rGen.TransRelEnd(*lit); ++tit) {
      rGen.SetTransition(inituni, tit->Ev, tit->X2);
      FD_DF("UniqueInit:   " << inituni << "-" << tit->Ev << "-" << tit->X2);
    }
  }
  // mark the new init state if there exists an originally marked init state (tm 20101206)
  if(!(rGen.InitStates() *  rGen.MarkedStates()).Empty()){
    rGen.SetMarkedState(inituni);
    FD_DF("UniqueInit: set marked state: " << inituni);
  }
  // delete old istates
  rGen.ClearInitStates();
  // set inituni as new initial state	
  rGen.SetInitState(inituni);
}

// UniqueInit(rGen&,rResGen&)
void UniqueInit(const Generator& rGen, Generator& rResGen) {
  rResGen.Assign(rGen);
  UniqueInit(rResGen);
}

// Deterministic(rGen&, rResGen&)
void Deterministic(const Generator& rGen, Generator& rResGen) {
  // temporary vectors
  std::vector<StateSet> power_states;
  std::vector<Idx> det_states;
  Deterministic(rGen, power_states, det_states, rResGen);
}


// aDeterministic(rGen&, rResGen&)
void aDeterministic(const Generator& rGen, Generator& rResGen) {
  // prepare result to keep original alphabet
  Generator* pResGen = &rResGen;
  if(&rResGen==&rGen) {
    pResGen= rResGen.New();
  }
  // perform op
  Deterministic(rGen,*pResGen);
  // set old attributes
  pResGen->EventAttributes(rGen.Alphabet());
  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}


// Deterministic(rGen&, rEntryStatesMap&, rResGen&)
void Deterministic(const Generator& rGen, std::map<Idx,StateSet>& rEntryStatesMap,
		   Generator& rResGen) {
  // prepare result:
  rEntryStatesMap.clear();
  // helpers:
  std::vector<StateSet> power_states;
  std::vector<Idx> det_states;
  // call Deterministic function
  Deterministic(rGen, power_states, det_states, rResGen);
  // build entry states map
  std::vector<StateSet>::size_type i;
  for (i = 0; i < power_states.size(); ++i) {
    rEntryStatesMap.insert(std::pair<Idx,StateSet>(det_states[i], power_states[i]));
  }
}


void Deterministic(const Generator& rGen, std::vector<StateSet>& rPowerStates,
		   std::vector<Idx>& rDetStates, Generator& rResGen) {

  // note: there is a demonstrative description of the multiway merge
  // algorithm in the master thesis. (Bernd Opitz)

  FD_DF("Deterministic(): core function #" << rGen.Size());

  // use pointer pResGen to result rResGen
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen) {
    pResGen= rResGen.New();
  }

  // prepare result
  pResGen->Clear();  
  rPowerStates.clear();
  rDetStates.clear();
  // set the name
  pResGen->Name(CollapsString("Det(" + rGen.Name() + ")"));
  // copy alphabet
  FD_DF("Deterministic A " << rGen.Alphabet().ToString());
  pResGen->InjectAlphabet(rGen.Alphabet());
  FD_DF("Deterministic B");

  // bail out on empty input 
  if(rGen.InitStatesEmpty()) {
    if(pResGen != &rResGen) {
      pResGen->Move(rResGen);
      delete pResGen;
    }
    FD_DF("Deterministic(): done (empty)"); 
    return;
  }

  // helpers
  typedef std::multimap< Idx,std::vector<StateSet>::size_type > T_HASHMAP;
  T_HASHMAP hashmap;
  std::vector<StateSet>::size_type current_vecindex;
  std::pair< std::map<StateSet,Idx>::iterator,bool > result;
  TransSet::Iterator transrel_end = rGen.TransRelEnd();
  StateSet newset;
  StateSet::Iterator lit;
  const Idx max_idx = std::numeric_limits<Idx>::max();

  // lock transrel to prevent iterator tracking (tmoor 201403)
  rGen.TransRel().Lock();
  
  // initialize rPowerStates with subset of initial states
  Idx newstate = pResGen->InsInitState();
  for (lit = rGen.InitStatesBegin(); lit != rGen.InitStatesEnd(); ++lit) {
    // clear set and insert single state
    newset.Insert(*lit);
    // if marked state set in res generator
    if (rGen.ExistsMarkedState(*lit)) {
      pResGen->SetMarkedState(newstate);
      FD_DF("Deterministic: setting as mstate: " << rGen.SStr(newstate));
    }
  }
  FD_DF("Deterministic: created subset of initial states {"
	<< newset.ToString() << "} with deterministic state index "
	<< rGen.SStr(newstate));
  // internally record newset 
  rPowerStates.push_back(newset);
  rDetStates.push_back(newstate);
  hashmap.insert(std::make_pair(newset.Signature(), (Idx)rPowerStates.size() - 1));

  // iteration over all states
  for (current_vecindex = 0; current_vecindex < rPowerStates.size(); 
       ++current_vecindex) {
    FD_WPC(current_vecindex,rPowerStates.size(), "Deterministic(): current/size: "<<  current_vecindex << " / " << rPowerStates.size());
    FD_DF("Deterministic: current power set: {" 
	  << rPowerStates[current_vecindex].ToString() << "} -> " 
	  << rDetStates[current_vecindex]);

    std::vector<StateSet> newset_vec;
    std::vector<Idx> event_vec;

    // multiway merge begin
    FD_DF("Deterministic: starting multiway merge...");
    std::list<TransSet::Iterator> merge_iterators;
    std::vector<Transition> trans_vec;

    // add transset iterator at begin of each state's transitions
    TransSet::Iterator tit;
    for (lit = rPowerStates[current_vecindex].Begin(); 
	 lit != rPowerStates[current_vecindex].End(); ++lit) {
      tit = rGen.TransRelBegin(*lit);
      if (tit != rGen.TransRelEnd(*lit)) {
	merge_iterators.push_back(tit);
	FD_DF("Deterministic: added merge iterator: " << rGen.SStr(tit->X1) 
	      << "-" << rGen.EStr(tit->Ev) << "-" << rGen.SStr(tit->X2));
      }
    }

    // find first iterator with lowest event
    while (! merge_iterators.empty()) {
      Idx currentevent = max_idx;
      std::list<TransSet::Iterator>::iterator i; 
      std::list<TransSet::Iterator>::iterator currentit = merge_iterators.end();
      for (i = merge_iterators.begin(); i != merge_iterators.end(); ++i) {
	if ((*i)->Ev < currentevent) {
	  currentevent = (*i)->Ev;
	  currentit = i;
	}
      }
      // currentit now holds the iterator
      // currentevent holds the lowest event (lowest Idx)

      // merge all transitions with currentevent at each iterator in a row;
      // this is a modification of multiway merge as after projection the
      // automaton most likely holds states with many transitions that share 
      // the same event; only merging the lowest transition and continue with
      // search for the lowest event again would be to slow here (because
      // of too much iterator dereferencing).
      Idx currentstate;
      while (currentit != merge_iterators.end()) {
	currentstate = (*currentit)->X1;
	TransSet::Iterator& j = *currentit;
	while (1) {
	  // remove iterator if it reaches the end of the transition set
	  if (j == transrel_end) {
	    merge_iterators.erase(currentit++);
	    break;
	  }
	  // if current iterator is in its original state
	  else if (j->X1 == currentstate) {
	    // if the event is still the same add the transition
	    if (j->Ev == currentevent) {
	      trans_vec.push_back(*j); 
	      FD_DF("Deterine: adding transition to list: " 
		    << rGen.SStr(j->X1) << "-" << rGen.EStr(j->Ev) << "-"
		    << rGen.SStr(j->X2));
	    }
	    // else go to next iterator
	    else {
	      ++currentit;
	      break;
	    }
	  }
	  // if the iterator is beyond its original state remove it
	  else {
	    merge_iterators.erase(currentit++);
	    break;
	  }
	  ++j;
	}
      }
    }

    // partition transition vector by events. optimizable? 
    FD_DF("Deterministic: partitioning the transition vector...");
    std::vector<Transition>::iterator tv_it;
    StateSet newset;
    Idx lastevent = 0;
    for (tv_it = trans_vec.begin(); tv_it != trans_vec.end(); ++tv_it) {
      if ((tv_it->Ev == lastevent) || (lastevent == 0)) {
	newset.Insert(tv_it->X2);
	lastevent = tv_it->Ev;
      }
      else {
	FD_DF("Deterministic: partition: {" << newset.ToString() 
	      << "} with event " << rGen.EStr(lastevent));
	newset_vec.push_back(newset);
	event_vec.push_back(lastevent);
	newset.Clear();
	newset.Insert(tv_it->X2);
	lastevent = tv_it->Ev;
      }
    }
    if (! newset.Empty()) {
      FD_DF("Deterministic: partition: {" << newset.ToString() 
	    << "} with event " << rGen.EStr(lastevent));
      newset_vec.push_back(newset);
      event_vec.push_back(lastevent);
    }
    FD_DF("Deterministic: partitioning the transition vector finished");
    FD_DF("Deterministic: multiway merge finished");
    // multiway merge end


    std::vector<StateSet>::size_type nsv_index;
    for (nsv_index = 0; nsv_index < newset_vec.size(); ++nsv_index) {
      StateSet& currentset = newset_vec[nsv_index];
      Idx currentevent = event_vec[nsv_index];
      Idx tmp_x2 = 0;
      Idx sig = currentset.Signature();
      // test if newset signature is already known
      std::pair<T_HASHMAP::iterator,T_HASHMAP::iterator> phit
	= hashmap.equal_range(sig);
      T_HASHMAP::iterator hit = phit.first;
      for (hit = phit.first; hit != phit.second; ++hit) {
	// test set of every matching signature for equality
	if (currentset == rPowerStates[hit->second]) {
	  tmp_x2 = rDetStates[hit->second];
	  break;
	}
      }

      // if new set is unique within the existing power sets
      if (tmp_x2 == 0) {
	// create new state in res generator
	tmp_x2 = pResGen->InsState();
	// insert newset in rPowerStates and get iterator,bool pair
	rPowerStates.push_back(currentset);
	rDetStates.push_back(tmp_x2);
	hashmap.insert(std::make_pair(sig, (Idx)rPowerStates.size() - 1));
	FD_DF("Deterministic: added new state " 
	      << rGen.SStr(tmp_x2) 
	      << " for new subset {" <<	currentset.ToString() << "}");
	// set marked if one of the states in current set is marked
	for (lit = currentset.Begin(); lit != currentset.End(); ++lit) {
	  if (rGen.ExistsMarkedState(*lit)) {
	    pResGen->SetMarkedState(tmp_x2);
	    break;
	  }
	}
      }
      // introduce transition
      pResGen->SetTransition(rDetStates[current_vecindex], currentevent, tmp_x2);
    }
  }


  // fix names
  if (rGen.StateNamesEnabled() && pResGen->StateNamesEnabled()) {
    FD_DF("Deterministic: fixing names...");
    // rPowerStates / rDetStates index "iterator"
    std::vector<StateSet>::size_type i;
    // deterministic states iterator
    std::vector<Idx>::const_iterator dit;
    for (i = 0; i < rPowerStates.size(); ++i) {
      // temporary state name
      std::string name = "{";
      for (lit = rPowerStates[i].Begin();  lit != rPowerStates[i].End(); ++lit) {
	if (rGen.StateName(*lit) != "") name = name + rGen.StateName(*lit) + ",";
	else name = name + ToStringInteger(*lit) + ",";
      }
      name.erase(name.length() - 1);
      name = name + "}";
      FD_DF("Deterministic: setting state name \"" << name << "\" for index "
	    << rDetStates[i]);
      pResGen->StateName(rDetStates[i], name);
    }
  }
  
  // move pResGen to rResGen
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

  FD_DF("Deterministic(): core function: done");
    
}



} // namespace faudes
