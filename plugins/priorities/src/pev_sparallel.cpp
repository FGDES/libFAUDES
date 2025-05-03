/** @file pev_sparallel.cpp Sshaped parallel composition */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2023 Yiheng Tang
   Copyright (C) 2025 Thomas Moor
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

#include "pev_sparallel.h"

namespace faudes {

// generate merged event name
std::string MergeEventNames (const std::set<std::string>& rNames){
  std::set<std::string>::const_iterator nameit = rNames.begin();
  if (nameit==rNames.end()){
    throw Exception("MergeEventNames()::","Empty set of events",0);
  }
  std::string result;
  for(;nameit!=rNames.end();nameit++){
    if (nameit!=rNames.begin()) result += "-";
    result += *nameit;
  }
  return result;
}


// perform SFC Parallel composition
void SUParallel(
		const pGenerator& rPGen1, const pGenerator& rPGen2,
		std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap,
		EventSet& rMerge,
		const EventSet& rPrivate,
		EventPriorities& rPrios,
		pGenerator& rPRes)
{
  FD_DF("Parallel(" << &rPGen1 << "," << &rPGen2 << ")");

  // prepare result
  Generator* pResGen = &rPRes;
  if(&rPRes== &rPGen1 || &rPRes== &rPGen2) {
    pResGen= rPRes.New();
  }
  pResGen->Clear();
  pResGen->Name(CollapsString(rPGen1.Name()+"||"+rPGen2.Name()));
  rCompositionMap.clear();

  // check if merging events are disjunct
  if(!(rPGen1.Alphabet()*rPGen2.Alphabet()*rMerge).Empty()){
    throw Exception("Parallel()","invalid input automata (they share some mergning events)",100);
  }
  // check if any merged event name is already utilised (consistent with the naming of merged event)
  EventSet::Iterator eit = rPGen1.AlphabetBegin();
  for (;eit!=rPGen1.AlphabetEnd();++eit){
    if (!rMerge.Exists(*eit)) continue;
    EventSet::Iterator eit2 = rPGen2.AlphabetBegin();
    for(;eit2!=rPGen2.AlphabetEnd();eit2++){
      if(!rMerge.Exists(*eit2)) continue;
      std::set<std::string> evs;
      evs.insert(rPGen1.EventName(*eit));
      evs.insert(rPGen2.EventName(*eit2));
      std::string newevname = MergeEventNames(evs);
      if (rPGen1.FindEvent(newevname)!=rPGen1.AlphabetEnd()
	  ||
	  rPGen2.FindEvent(newevname)!=rPGen2.AlphabetEnd()){
	throw Exception("Parallel()","invalid input automata (event name '"+newevname+"' is in the form of a merged event)",101);
      }
    }
  }

  // create res alphabet (no merging)
  pResGen->InsEvents(rPGen1.Alphabet());
  pResGen->InsEvents(rPGen2.Alphabet());
  Idx lowest = rPGen1.LowestPriority();

  // shared events
  EventSet sharedalphabet = rPGen1.Alphabet() * rPGen2.Alphabet();
  EventSet mergingalphabet = (rPGen1.Alphabet()+rPGen2.Alphabet())*rMerge;
  FD_DF("Parallel: shared events: " << sharedalphabet.ToString());

  // todo stack
  std::stack< std::pair<Idx,Idx> > todo;
  // current pair, new pair
  std::pair<Idx,Idx> currentstates, newstates;
  // state
  Idx tmpstate;
  StateSet::Iterator lit1, lit2;
  TransSet::Iterator tit1, tit1_end, tit2, tit2_end;
  std::map< std::pair<Idx,Idx>, Idx>::iterator rcit;

  // push all combinations of initial states on todo stack
  FD_DF("Parallel: adding all combinations of initial states to todo:");
  for (lit1 = rPGen1.InitStatesBegin(); lit1 != rPGen1.InitStatesEnd(); ++lit1) {
    for (lit2 = rPGen2.InitStatesBegin(); lit2 != rPGen2.InitStatesEnd(); ++lit2) {
      currentstates = std::make_pair(*lit1, *lit2);
      todo.push(currentstates);
      tmpstate = pResGen->InsInitState();
      rCompositionMap[currentstates] = tmpstate;
      FD_DF("Parallel:   (" << *lit1 << "|" << *lit2 << ") -> "
	    << rCompositionMap[currentstates]);
    }
  }

  // start algorithm
  EventSet newMerge; // buffer a set for newly emerging merging events
  FD_DF("Parallel: processing reachable states:");
  while (! todo.empty()) {

    // allow for user interrupt
    // LoopCallback();
    // allow for user interrupt, incl progress report
    FD_WPC(rCompositionMap.size(),rCompositionMap.size()+todo.size(),"Parallel(): processing");
    // get next reachable state from todo stack
    currentstates = todo.top();
    todo.pop();

    // get current highest priority
    Idx highest = lowest;
    EventSet active1 = rPGen1.ActiveEventSet(currentstates.first);
    EventSet active2 = rPGen2.ActiveEventSet(currentstates.second);
    EventSet active = (active1 * active2) + (active1-rPGen2.Alphabet()); // iterate rPGen1 first
    EventSet::Iterator eit = active.Begin();
    for(;eit!=active.End();eit++){
      if(!rPrivate.Exists(*eit)) continue;
      if(rPGen1.Priority(*eit)>highest) highest = rPGen1.Priority(*eit);
    }
    active = active2-rPGen1.Alphabet();
    eit = active.Begin();
    for(;eit!=active.End();eit++){
      if(!rPrivate.Exists(*eit)) continue;
      if(rPGen2.Priority(*eit)>highest) highest = rPGen2.Priority(*eit);
    }


    FD_DF("Parallel: processing (" << currentstates.first << "|"
	  << currentstates.second << ") -> "
	  << rCompositionMap[currentstates]);
    // iterate over all rGen1 transitions
    // (includes execution of shared events)
    tit1 = rPGen1.TransRelBegin(currentstates.first);
    tit1_end = rPGen1.TransRelEnd(currentstates.first);
    for (; tit1 != tit1_end; ++tit1) {
      // skip if priority lower than highest
      if (rPGen1.Priority(tit1->Ev)<highest) continue;
      // if asynch
      if ((!sharedalphabet.Exists(tit1->Ev) && !mergingalphabet.Exists(tit1->Ev))
	  ||
	  (mergingalphabet.Exists(tit1->Ev) &&  (rPGen2.ActiveEventSet(currentstates.second) * rMerge).Empty())) {
	FD_DF("Parallel:   asynch step (private non-merging or merging with cstate.second cannot merge)");
	newstates = std::make_pair(tit1->X2, currentstates.second);
	// add to todo list if composition state is new
	rcit = rCompositionMap.find(newstates);
	if (rcit == rCompositionMap.end()) {
	  todo.push(newstates);
	  tmpstate = pResGen->InsState();
	  rCompositionMap[newstates] = tmpstate;
	  FD_DF("Parallel:   todo push: (" << newstates.first << "|"
		<< newstates.second << ") -> "
		<< rCompositionMap[newstates]);
	}
	else {
	  tmpstate = rcit->second;
	}
	pResGen->SetTransition(rCompositionMap[currentstates], tit1->Ev, tmpstate);
	FD_DF("Parallel:   add transition to new generator: "
	      << rCompositionMap[currentstates] << "-" << tit1->Ev << "-"
	      << tmpstate);
      }
      // if synch
      else {
	FD_DF("Parallel: synch (common event or synch through merging)");
	// find shared transitions
	if (sharedalphabet.Exists(tit1->Ev)){
	  FD_DF("Parallel: synch through common event");
	  tit2 = rPGen2.TransRelBegin(currentstates.second, tit1->Ev);
	  tit2_end = rPGen2.TransRelEnd(currentstates.second, tit1->Ev);
	  for (; tit2 != tit2_end; ++tit2) {
	    newstates = std::make_pair(tit1->X2, tit2->X2);
	    // add to todo list if composition state is new
	    rcit = rCompositionMap.find(newstates);
	    if (rcit == rCompositionMap.end()) {
	      todo.push(newstates);
	      tmpstate = pResGen->InsState();
	      rCompositionMap[newstates] = tmpstate;
	      FD_DF("Parallel:   todo push: (" << newstates.first << "|"
		    << newstates.second << ") -> "
		    << rCompositionMap[newstates]);
	    }
	    else {
	      tmpstate = rcit->second;
	    }
	    pResGen->SetTransition(rCompositionMap[currentstates],
				   tit1->Ev, tmpstate);
	    FD_DF("Parallel:   add transition to new generator: "
		  << rCompositionMap[currentstates] << "-"
		  << tit1->Ev << "-" << tmpstate);
	  }
	}
	else{
	  FD_DF("Parallel: synch through merging");
	  tit2 = rPGen2.TransRelBegin(currentstates.second);
	  tit2_end = rPGen2.TransRelEnd(currentstates.second);
	  for (; tit2 != tit2_end; ++tit2) {
	    if (!rMerge.Exists(tit2->Ev)) continue; // only interested in merging events
	    std::set<std::string> evs;
	    evs.insert(rPGen1.EventName(tit1->Ev));
	    evs.insert(rPGen2.EventName(tit2->Ev));
	    std::string newevname = MergeEventNames(evs); // event name of merged event
	    Idx newev;
	    if (!rPRes.ExistsEvent(newevname)){
	      newev = rPRes.InsEvent(newevname);
	      newMerge.Insert(newevname);
	      rPrios.InsPriority(newevname,1); //TMPRIO ??
	    }
	    else{
	      newev = *rPRes.FindEvent(newevname);
	    }
	    newstates = std::make_pair(tit1->X2, tit2->X2);
	    // add to todo list if composition state is new
	    rcit = rCompositionMap.find(newstates);
	    if (rcit == rCompositionMap.end()) {
	      todo.push(newstates);
	      tmpstate = pResGen->InsState();
	      rCompositionMap[newstates] = tmpstate;
	      FD_DF("Parallel: todo push: (" << newstates.first << "|"
		    << newstates.second << ") -> "
		    << rCompositionMap[newstates]);
	    }
	    else {
	      tmpstate = rcit->second;
	    }
	    pResGen->SetTransition(rCompositionMap[currentstates],
				   newev, tmpstate);
	    FD_DF("Parallel:   add transition to new generator: "
		  << rCompositionMap[currentstates] << "-"
		  << tit1->Ev << "-" << tmpstate);
	  }

	}
      }
    }
    // iterate over all rGen2 transitions
    // (without execution of synch step)
    tit2 = rPGen2.TransRelBegin(currentstates.second);
    tit2_end = rPGen2.TransRelEnd(currentstates.second);
    for (; tit2 != tit2_end; ++tit2) {
      if (rPGen2.Priority(tit2->Ev)<highest) continue;
      if ((!sharedalphabet.Exists(tit2->Ev) && !mergingalphabet.Exists(tit2->Ev))
	  ||
	  (mergingalphabet.Exists(tit2->Ev) &&  (rPGen1.ActiveEventSet(currentstates.first) * rMerge).Empty())) {
	FD_DF("Parallel:   exists only in rGen2");
	newstates = std::make_pair(currentstates.first, tit2->X2);
	// add to todo list if composition state is new
	rcit = rCompositionMap.find(newstates);
	if (rcit == rCompositionMap.end()) {
	  todo.push(newstates);
	  tmpstate = pResGen->InsState();
	  rCompositionMap[newstates] = tmpstate;
	  FD_DF("Parallel:   todo push: (" << newstates.first << "|"
		<< newstates.second << ") -> "
		<< rCompositionMap[newstates]);
	}
	else {
	  tmpstate = rcit->second;
	}
	pResGen->SetTransition(rCompositionMap[currentstates],
			       tit2->Ev, tmpstate);
	FD_DF("Parallel:   add transition to new generator: "
	      << rCompositionMap[currentstates] << "-"
	      << tit2->Ev << "-" << tmpstate);
      }
    }
  }

  // set marked states
  for (lit1 = rPGen1.MarkedStatesBegin();
       lit1 != rPGen1.MarkedStatesEnd(); ++lit1) {
    for (lit2 = rPGen2.MarkedStatesBegin();
	 lit2 != rPGen2.MarkedStatesEnd(); ++lit2) {
      currentstates = std::make_pair(*lit1, *lit2);
      rcit = rCompositionMap.find(currentstates);
      if (rcit != rCompositionMap.end()) {
	pResGen->SetMarkedState(rcit->second);
      }
    }
  }
  FD_DF("Parallel: marked states: "
	<< pResGen->MarkedStatesToString());
  // copy result
  if(pResGen != &rPRes) {
    rPRes = *pResGen;
    delete pResGen;
  }
  // set statenames
  if(rPGen1.StateNamesEnabled() && rPGen2.StateNamesEnabled() && rPRes.StateNamesEnabled())
    SetComposedStateNames(rPGen1, rPGen2, rCompositionMap, rPRes);
  else
    rPRes.StateNamesEnabled(false);

  // push new merging events
  rMerge.InsertSet(newMerge);
}

void SUParallel(
		const pGenerator& rPGen1, const pGenerator& rPGen2,
		EventSet& rMerge,
		const EventSet& rPrivate,
		EventPriorities& rPrios,
		pGenerator& rPRes){
  std::map< std::pair<Idx,Idx>, Idx> dummy;
  SUParallel(rPGen1,rPGen2,dummy,rMerge,rPrivate,rPrios,rPRes);
}

// the special fairness merge for SFC
// does not do consistency check (should have been done before in SFC_Parallel)
// rGen12 is the parallel composition of rPGen1 and rPGen2, which should have been computed before
void UParallel_MergeFairness(const pGenerator& rPGen1, const pGenerator& rPGen2, const Generator& rGen12, const EventSet& rMerge, FairnessConstraints& rFairRes){
  rFairRes.Clear();
  // iterate over fairness of rPGen1
  FairnessConstraints::Position fpos = 0;
  for(;fpos<rPGen1.GlobalAttribute().Fairness().Size();++fpos){
    EventSet fair = rPGen1.GlobalAttribute().Fairness().At(fpos);
    EventSet fairm1 = fair * rMerge; // merging events in the current fairness (of rPGen1)
    EventSet::Iterator eit1 = fairm1.Begin();
    for(;eit1!=fairm1.End();eit1++){
      EventSet m2 = rPGen2.Alphabet() * rMerge; // merging events in rPgen2
      EventSet::Iterator eit2 = m2.Begin();
      for(;eit2!=m2.End();eit2++){
	std::set<std::string> evs;
	evs.insert(rPGen1.EventName(*eit1));
	evs.insert(rPGen2.EventName(*eit2));
	std::string newevname = MergeEventNames(evs);
	if (!rGen12.Alphabet().Exists(newevname)) continue; // skip if this event is actually not used in the parallel composition
	fair.Insert(newevname);
      }
    }
    rFairRes.Append(fair);
  }
  // iterate over fairness of rPGen2
  fpos = 0;
  for(;fpos<rPGen2.GlobalAttribute().Fairness().Size();++fpos){
    EventSet fair = rPGen2.GlobalAttribute().Fairness().At(fpos);
    EventSet fairm2 = fair * rMerge; // merging events in the current fairness (of rPGen2)
    EventSet::Iterator eit2 = fairm2.Begin();
    for(;eit2!=fairm2.End();eit2++){
      EventSet m1 = rPGen1.Alphabet() * rMerge; // merging events in rPgen1
      EventSet::Iterator eit1 = m1.Begin();
      for(;eit1!=m1.End();eit1++){
	std::set<std::string> evs;
	evs.insert(rPGen1.EventName(*eit1));
	evs.insert(rPGen2.EventName(*eit2));
	std::string newevname = MergeEventNames(evs);
	if (!rGen12.Alphabet().Exists(newevname)) continue; // skip if this event is actually not used in the parallel composition
	fair.Insert(newevname);
      }
    }
    rFairRes.Append(fair);
  }
}

}// namspace  
