/** @file mtc_project.cpp

Methods for computing the natural projection of multitasking generators

*/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008  Matthias Singer
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


#include "mtc_project.h"
#include "cfl_localgen.h"

namespace faudes {

// mtcUniqueInit(rGen&)
void mtcUniqueInit(MtcSystem& rGen) {
  Idx inituni;
  StateSet::Iterator lit;
  TransSet::Iterator tit;
  // check number of initial states
  if (rGen.InitStatesSize() <= 1) return;
  // introduce new initial state
  if (rGen.StateNamesEnabled()) {
    std::string initname=rGen.UniqueStateName("InitUni");
    inituni = rGen.InsState(initname); 
  }
  else {
    inituni = rGen.InsState();
  }
  FD_DF("mtcUniqueInit: introducing new initial state: " << inituni);
  // introduce outgoing transitions from initial state
  FD_DF("mtcUniqueInit: introduce outgoing transitions: ");
  for (lit = rGen.InitStatesBegin(); lit != rGen.InitStatesEnd(); ++lit) {
    for (tit = rGen.TransRelBegin(*lit); tit != rGen.TransRelEnd(*lit); ++tit) {
      rGen.SetTransition(inituni, tit->Ev, tit->X2);
      FD_DF("mtcUniqueInit:   " << inituni << "-" << tit->Ev << "-" << tit->X2);
    }
    // colored states
    const ColorSet& colors = rGen.Colors(*lit);
    rGen.InsColors(inituni, colors);
    // marked states
    if (!rGen.MarkedStatesEmpty()) {
      rGen.SetMarkedState(inituni);
      FD_DF("mtcUniqueInit: set marked state: " << inituni);
    }
  }
  // delete old istates
  rGen.ClearInitStates();
  // set inituni as new initial state	
  rGen.SetInitState(inituni);
}


// mtcDeterministic(rGen&, rResGen&)
void mtcDeterministic(const MtcSystem& rGen, MtcSystem& rResGen) {
  // temporary vectors
  std::vector<StateSet> power_states;
  std::vector<Idx> det_states;
  mtcDeterministic(rGen, power_states, det_states, rResGen);
}


// mtcDeterministic(rGen&, rEntryStatesMap&, rResGen&)
void mtcDeterministic(const MtcSystem& rGen, std::map<Idx,StateSet>& rEntryStatesMap,
    MtcSystem& rResGen) {
  // prepare result:
  rEntryStatesMap.clear();
  // helpers:
  std::vector<StateSet> power_states;
  std::vector<Idx> det_states;
  // call Deterministic function
  mtcDeterministic(rGen, power_states, det_states, rResGen);
  // build entry states map
  std::vector<StateSet>::size_type i;
  for (i = 0; i < power_states.size(); ++i) {
    rEntryStatesMap.insert(std::pair<Idx,StateSet>(det_states[i], power_states[i]));
  }
}


void mtcDeterministic(const MtcSystem& rGen, std::vector<StateSet>& rPowerStates,
    std::vector<Idx>& rDetStates, MtcSystem& rResGen) {
  // set the name
  rResGen.Name("Det(" + rGen.Name() + ")");

  // prepare result
  rPowerStates.clear();
  rDetStates.clear();
  rResGen.Clear();

  // copy alphabet
  rResGen.InjectAlphabet(rGen.Alphabet());

  // helpers
  TransSetEvX1X2 trel_evx1x2;
  rGen.TransRel(trel_evx1x2);
  typedef std::multimap< Idx,std::vector<StateSet>::size_type > T_HASHMAP;
  T_HASHMAP hashmap;
  std::vector<StateSet>::size_type current_vecindex;
  std::pair< std::map<StateSet,Idx>::iterator,bool > result;
  TransSet::Iterator transrel_end = rGen.TransRelEnd();
  StateSet newset;
  StateSet::Iterator lit;
  const Idx max_idx = std::numeric_limits<Idx>::max();
  if (rGen.InitStatesEmpty()) {
    return;
  }
//   std::map<Idx,ColorSet> CopyMap = rGen.StateColorMap();
  Idx newstate = rResGen.InsInitState();
  // initialize rPowerStates with subset of initial states
  for (lit = rGen.InitStatesBegin(); lit != rGen.InitStatesEnd(); ++lit) {
    // clear set and insert single state
    newset.Insert(*lit);
    const ColorSet& colors = rGen.Colors(*lit);
    // set colored state if a member state is colored 
    rResGen.InsColors(newstate,colors);
  }
  FD_DF("mtcDeterministic: created subset of initial states {"
      << newset.ToString() << "} with deterministic state index " << rGen.SStr(newstate));
  // insert newset in rPowerStates
  rPowerStates.push_back(newset);
  rDetStates.push_back(newstate);
  hashmap.insert(std::make_pair(newset.Signature(), (Idx)rPowerStates.size() - 1));

  // iteration over all states
  for (current_vecindex = 0; current_vecindex < rPowerStates.size(); ++current_vecindex) {
    FD_DF("mtcDeterministic: current power set: {" << rPowerStates[current_vecindex].ToString() <<
        "} -> " << rDetStates[current_vecindex]);

    std::vector<StateSet> newset_vec;
    std::vector<Idx> event_vec;

    // multiway merge begin
    FD_DF("mtcDeterministic: starting multiway merge...");
    std::list<TransSet::Iterator> merge_iterators;
    std::vector<Transition> trans_vec;

    // add transset iterator at begin of each state's transitions
    TransSet::Iterator tit;
    for (lit = rPowerStates[current_vecindex].Begin();
    lit != rPowerStates[current_vecindex].End(); ++lit) {
      tit = rGen.TransRelBegin(*lit);
      if (tit != rGen.TransRelEnd(*lit)) {
        merge_iterators.push_back(tit);
        FD_DF("mtcDeterministic: added merge iterator: " << rGen.SStr(tit->X1) 
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

      // merge all transitions with currentevent at each iterator in a row
      // this is a modification of multiway merge as after projection the
      // automaton most likely holds states with many transitions that share
      // the same event. only merging the lowest transition and continue with
      // search for the lowest event again would be to slow here (because
      // of too much iterator dereferencing).
      Idx currentstate;
      while (currentit != merge_iterators.end()) {
        currentstate = (*currentit)->X1;
        TransSet::Iterator& j = *currentit;
        while (1) {
          // remove iterator if it reaches the end of the transition set
          if (j == transrel_end) {
            std::list<TransSet::Iterator>::iterator tmpit = currentit;
            ++currentit;
            merge_iterators.erase(tmpit);
            break;
          }
          // if current iterator is in its original state
          else if (j->X1 == currentstate) {
            // if the event is still the same add the transition
            if (j->Ev == currentevent) {
              trans_vec.push_back(*j);
              FD_DF("Determine: adding transition to list: " << rGen.SStr(j->X1)
                  << "-" << rGen.EStr(j->Ev) << "-" << rGen.SStr(j->X2));
            }
            // else go to next iterator
            else {
              ++currentit;
              break;
            }
          }
          // if the iterator is beyond its original state remove it
          else {
            std::list<TransSet::Iterator>::iterator tmpit = currentit;
            ++currentit;
            merge_iterators.erase(tmpit);
            break;
          }
          ++j;
        }
      }
    }

    // partition transition vector by events. optimizable? 
    FD_DF("mtcDeterministic: partitioning the transition vector...");
    std::vector<Transition>::iterator tv_it;
    StateSet newset;
    Idx lastevent = 0;
    for (tv_it = trans_vec.begin(); tv_it != trans_vec.end(); ++tv_it) {
      if ((tv_it->Ev == lastevent) || (lastevent == 0)) {
        newset.Insert(tv_it->X2);
        lastevent = tv_it->Ev;
      }
      else {
        FD_DF("mtcDeterministic: partition: {" << newset.ToString() 
            << "} with event " << rGen.EStr(lastevent));
        newset_vec.push_back(newset);
        event_vec.push_back(lastevent);
        newset.Clear();
        newset.Insert(tv_it->X2);
        lastevent = tv_it->Ev;
      }
    }
    if (! newset.Empty()) {
      FD_DF("mtcDeterministic: partition: {" << newset.ToString() 
          << "} with event " << rGen.EStr(lastevent));
      newset_vec.push_back(newset);
      event_vec.push_back(lastevent);
    }
    FD_DF("mtcDeterministic: partitioning the transition vector finished");
    FD_DF("mtcDeterministic: multiway merge finished");
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
        tmp_x2 = rResGen.InsState();
        // insert newset in rPowerStates and get iterator,bool pair
        rPowerStates.push_back(currentset);
        rDetStates.push_back(tmp_x2);
        hashmap.insert(std::make_pair(sig, (Idx)rPowerStates.size() - 1));
        FD_DF("mtcDeterministic: added new state " << rGen.SStr(tmp_x2) 
            << " for new subset {" <<	currentset.ToString() << "}");
        // set colored if one of the states in current set is colored
        for (lit = currentset.Begin(); lit != currentset.End(); ++lit) {
          const ColorSet& colors = rGen.Colors(*lit);
          rResGen.InsColors(tmp_x2,colors);
          FD_DF("mtcDeterministic: setting as colored: " << rGen.SStr(tmp_x2));
        }
      }
      // introduce transition
      rResGen.SetTransition(rDetStates[current_vecindex], currentevent, tmp_x2);
    }
  }
  // fix names
  if (rGen.StateNamesEnabled() && rResGen.StateNamesEnabled()) {
    FD_DF("mtcDeterministic: fixing names...");
    // rPowerStates / rDetStates index "iterator"
    std::vector<StateSet>::size_type i;
    // deterministic states iterator
    std::vector<Idx>::const_iterator dit;
    for (i = 0; i < rPowerStates.size(); ++i) {
      // temporary state name
      std::string name = "{";
      for (lit = rPowerStates[i].Begin(); lit != rPowerStates[i].End(); ++lit) {
        if (rResGen.StateName(*lit) != "") {
          name = name + rResGen.StateName(*lit) + ",";
        }
        else {
          name = name + ToStringInteger(*lit) + ",";
        }
      }
      name.erase(name.length() - 1);
      name = name + "}";
      rResGen.StateName(rDetStates[i], name);
      FD_DF("mtcDeterministic: setting state name \"" << name << "\" for index " << rDetStates[i]);
    }
  }
  // Delete unnecessary events and set event attributes
  rResGen.DelEvents(rResGen.UnusedEvents());
  EventSet usedEvents = rResGen.UsedEvents();
  rResGen.SetControllable(rGen.ControllableEvents()*usedEvents);
  rResGen.SetForcible(rGen.ForcibleEvents()*usedEvents);
  rResGen.ClrObservable(rGen.UnobservableEvents()*usedEvents);
}

// mtcProjectNonDet(rGen&, rProjectAlphabet&)
void mtcProjectNonDet(MtcSystem& rGen, const EventSet& rProjectAlphabet) {

  // HELPERS:
  StateSet reach; // StateSet for reachable states
  std::stack<Idx> todo; // todo stack
  StateSet done; // done set
  Idx currentstate; // the currently processed state
  StateSet::Iterator lit; 
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;

  // ALGORITHM:
  // initialize algorithm by pushing init states on todo stack
  for (lit = rGen.InitStatesBegin(); lit != rGen.InitStatesEnd(); ++lit) {
    FD_DF("mtcProjectNonDet: todo add: " << rGen.SStr(*lit));
    todo.push(*lit);
  }

  // process todo stack
  while (! todo.empty()) {
    currentstate = todo.top();
    todo.pop();
    done.Insert(currentstate); // mark as done
    FD_DF("mtcProjectNonDet: current state: " << rGen.SStr(currentstate));

    // comp accessible reach
    reach.Clear();
    LocalAccessibleReach(rGen, rProjectAlphabet, currentstate, reach);
    FD_DF("mtcProjectNonDet: local reach: " << reach.ToString());

    // remove all transitions that leave current state 
    // with an invisible event
    tit = rGen.TransRelBegin(currentstate);
    tit_end = rGen.TransRelEnd(currentstate);
    while(tit != tit_end) {
      FD_DF("mtcProjectNonDet: current transition: " << rGen.SStr(tit->X1)
          << "-" << rGen.EStr(tit->Ev) << "-" << rGen.SStr(tit->X2));
      if (! rProjectAlphabet.Exists(tit->Ev)) {
        FD_DF("mtcProjectNonDet: deleting current transition");
        TransSet::Iterator tit_tmp = tit;
        ++tit;
        rGen.ClrTransition(tit_tmp);
      }
      else {
              ++tit;
      }
    }

    // relink outgoing transitions
    FD_DF("mtcProjectNonDet: relinking outgoing transitions...");
    for (lit = reach.Begin(); lit != reach.End(); ++lit) {
      tit = rGen.TransRelBegin(*lit);
      tit_end = rGen.TransRelEnd(*lit);
      for (; tit != tit_end; ++tit) {
        if (rProjectAlphabet.Exists(tit->Ev)) {
          FD_DF("mtcProjectNonDet: relinking transition: " << rGen.TStr(*tit) << " to " << rGen.SStr(currentstate) << "--(" << rGen.EStr(tit->Ev) << ")-->" << rGen.SStr(tit->X2));
          rGen.SetTransition(currentstate, tit->Ev, tit->X2);
          if (! done.Exists(tit->X2)) {
            FD_DF("mtcProjectNonDet: todo push: " << rGen.SStr(tit->X2));
            todo.push(tit->X2);
          }
        }
      }
      // colored status test
      const ColorSet& colors = rGen.Colors(*lit);
      if (!colors.Empty()) {
        FD_DF("mtcProjectNonDet: setting colored state " << rGen.SStr(currentstate));
        rGen.InsColors(currentstate, colors);
      }
    }
  }

  // inject projection alphabet
  rGen.InjectAlphabet(rProjectAlphabet);

  EventSet unused = rGen.UnusedEvents();
  EventSet::Iterator eit;
  for(eit = unused.Begin(); eit != unused.End(); ++eit){
    rGen.ClrEventAttribute(*eit);
    rGen.EventSymbolTablep()->ClrEntry(*eit);
    rGen.DelEvent(*eit);
  }
  // set name
  rGen.Name("MtcPro(" + rGen.Name() + ")" );
}

// wrapper
void mtcProjectNonDet(const MtcSystem& rGen, const EventSet& rProjectAlphabet, MtcSystem& rResGen) {
  rResGen=rGen;
  mtcProjectNonDet(rResGen,rProjectAlphabet);
}

// mtcProject(rGen, rProjectAlphabet, rResGen&)
void mtcProject(const MtcSystem& rGen, const EventSet& rProjectAlphabet, MtcSystem& rResGen) {
  // algorithm:
  // temporary copy of rGen
  MtcSystem tmp;
  // temporarily assign rGen to rResGen
  MtcSystem copyGen = MtcSystem(rGen);
  // project rResGen with respect to rProjectAlphabet
  mtcProjectNonDet(copyGen, rProjectAlphabet);
#ifdef FAUDES_DEBUG_FUNCTION
  FD_WARN("mtcProject: debug out")
  copyGen.Write("tmp_project_nd.gen");
#endif
  // put deterministic result into tmp
  mtcDeterministic(copyGen, tmp);
#ifdef FAUDES_DEBUG_FUNCTION
  FD_WARN("mtcProject: debug out")
  tmp.Write("tmp_project_d.gen");
#endif
  // minimize states and rewrite result to rResGen
  mtcStateMin(tmp, rResGen);
#ifdef FAUDES_DEBUG_FUNCTION
  FD_WARN("mtcProject: debug out")
  rResGen.Write("tmp_project_m.gen");
#endif
  // set controllability status TODO: other event attributes
  rResGen.SetControllable(rGen.ControllableEvents()*rProjectAlphabet);
  // set name
  rResGen.Name(rResGen.Name()+": mtcProject");
}


// mtcProject(rGen, rProjectAlphabet, rEntryStatesMap&, rResGen&)
void mtcProject(const MtcSystem& rGen, const EventSet& rProjectAlphabet,
    std::map<Idx,StateSet>& rEntryStatesMap, MtcSystem& rResGen) {
  // temporary copy of rGen
  MtcSystem tmp;
  MtcSystem copyGen = MtcSystem(rGen);
  // temporary entry state map
  std::map<Idx,StateSet> tmp_entrystatemap;
  // temporarily assign rGen to rResGen
  // std::cout << " copied low-level generator " << std::endl;
  // copyGen.DWrite();
  // project tmp with respect to palphabet
  mtcProjectNonDet(copyGen, rProjectAlphabet); 

  // put deterministic result into tmp
  mtcDeterministic(copyGen, tmp_entrystatemap, tmp);
  // write entry state map for minimized generator
  std::vector<StateSet> subsets;
  std::vector<Idx> newindices;
  // minimize states and rewrite result to rResGen
  mtcStateMin(tmp, rResGen, subsets, newindices);
  // rResGen.DWrite();
  // build entry state map
  std::vector<StateSet>::size_type i;
  std::map<Idx,StateSet>::iterator esmit;
  StateSet::Iterator sit;
  for (i = 0; i < subsets.size(); ++i) {	
    StateSet tmpstates;
    for (sit = subsets[i].Begin(); sit != subsets[i].End(); ++sit) {
      esmit = tmp_entrystatemap.find(*sit);
#ifdef FD_CHECKED
      if (esmit == tmp_entrystatemap.end()) {
        FD_WARN("mtcproject internal error");
        abort();
      }
#endif
      // insert entry states in temporary StateSet
      tmpstates.InsertSet(esmit->second);
    }

    rEntryStatesMap.insert(std::make_pair(newindices[i], tmpstates));
  }
}


// mtcInvProject(rGen&, rProjectAlphabet)
void mtcInvProject(MtcSystem& rGen, const EventSet& rProjectAlphabet) {
  // test if the alphabet of the generator is included in the given alphabet
  if(! (rProjectAlphabet >= rGen.Alphabet() ) ){
    std::stringstream errstr;
    errstr << "Input alphabet has to contain alphabet of generator \"" << rGen.Name() << "\"";
    throw Exception("InvProject(Generator,EventSet)", errstr.str(), 506);
  }
  EventSet newevents = rProjectAlphabet - rGen.Alphabet();
  // insert events into generator
  rGen.InsEvents(newevents);
  FD_DF("mtcInvProject: adding events \"" << newevents.ToString() << "\" at every state");
  StateSet::Iterator lit;
  EventSet::Iterator eit;
  for (lit = rGen.StatesBegin(); lit != rGen.StatesEnd(); ++lit) {
    for (eit = newevents.Begin(); eit != newevents.End(); ++eit)
      rGen.SetTransition(*lit, *eit, *lit);
  }
}

// RTI wrapper
void mtcInvProject(const MtcSystem& rGen, const EventSet& rProjectAlphabet, MtcSystem& rResGen) {
  rResGen=rGen;
  mtcInvProject(rResGen,rProjectAlphabet);
}

} // namespace faudes
