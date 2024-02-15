/** @file cfl_graphfncts.cpp Operations on (directed) graphs. */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor, Klaus Schmidt, Sebastian Perk
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


#include "cfl_graphfncts.h"


namespace faudes {

/*
**********************************************************************
**********************************************************************
**********************************************************************

Search for strongly connected ycless components (SCC)

**********************************************************************
**********************************************************************
**********************************************************************
*/

// Filter class for SCC search: statics
const StateSet SccFilter::msEmptyStates=StateSet();
const EventSet SccFilter::msEmptyEvents=EventSet();

// Filter class for SCC search: destructor
SccFilter::~SccFilter(void) {
  if(mpStatesAvoid) delete mpStatesAvoid;
  if(mpStatesRequire) delete mpStatesRequire;
  if(mpEventsAvoid) delete mpEventsAvoid;
}

// Filter class for SCC search: constructors
SccFilter::SccFilter(void) :
  mMode(0x00),
  pStatesAvoid(&msEmptyStates),
  pStatesRequire(&msEmptyStates),
  pEventsAvoid(&msEmptyEvents),
  mpStatesAvoid(0),
  mpStatesRequire(0),
  mpEventsAvoid(0)
{}


// Filter class for SCC search: constructors
SccFilter::SccFilter(const SccFilter& rSrc) :
  mMode(rSrc.mMode),
  pStatesAvoid(rSrc.pStatesAvoid),
  pStatesRequire(rSrc.pStatesRequire),
  pEventsAvoid(rSrc.pEventsAvoid),
  mpStatesAvoid(0),
  mpStatesRequire(0),
  mpEventsAvoid(0)
{}


// Filter class for SCC search: constructors
SccFilter::SccFilter(int mode, const EventSet& rEventsAvoid) :
  mMode(mode),
  pStatesAvoid(&msEmptyStates),
  pStatesRequire(&msEmptyStates),
  pEventsAvoid(&msEmptyEvents),
  mpStatesAvoid(0),
  mpStatesRequire(0),
  mpEventsAvoid(0)
{
  // no special states
  mMode &= ~(FmStatesRequire | FmStatesAvoid);
  // use events for avoid
  mMode |= FmEventsAvoid;
  // record
  pEventsAvoid=&rEventsAvoid;
}


// Filter class for SCC search: constructors
SccFilter::SccFilter(int mode, const StateSet& rStatesAvoidRequire) :
  mMode(mode),
  pStatesAvoid(&msEmptyStates),
  pStatesRequire(&msEmptyStates),
  pEventsAvoid(&msEmptyEvents),
  mpStatesAvoid(0),
  mpStatesRequire(0),
  mpEventsAvoid(0)
{
  // no special events
  mMode &= ~FmEventsAvoid;
  // use states for require
  if(mMode & FmStatesRequire)
    pStatesRequire=&rStatesAvoidRequire;
  // use states for avoid
  if(mMode & FmStatesAvoid)
    pStatesAvoid=&rStatesAvoidRequire;
}

// Filter class for SCC search: constructors
SccFilter::SccFilter(int mode, const StateSet& rStatesAvoid, const StateSet& rStatesRequire): 
  mMode(mode),
  pStatesAvoid(&msEmptyStates),
  pStatesRequire(&msEmptyStates),
  pEventsAvoid(&msEmptyEvents),
  mpStatesAvoid(0),
  mpStatesRequire(0),
  mpEventsAvoid(0)
{
  // no special events
  mMode &= ~FmEventsAvoid;
  // but special states
  mMode |= FmStatesRequire;
  mMode |= FmStatesAvoid;
  // set internal pointers
  pStatesAvoid=&rStatesAvoid;
  pStatesRequire=&rStatesRequire;
}


// Filter class for SCC search: constructors
SccFilter::SccFilter(int mode, const Generator& rGen) :
  mMode(mode),
  pStatesAvoid(&msEmptyStates),
  pStatesRequire(&msEmptyStates),
  pEventsAvoid(&msEmptyEvents),
  mpStatesAvoid(0),
  mpStatesRequire(0),
  mpEventsAvoid(0)
{
  // sets all empty by default
  mMode &= ~(FmStatesAvoid | FmStatesRequire | FmEventsAvoid );
  // ignore livelocks ==> require at least one marked state
  if(mode & FmIgnoreLiveLocks) {
    pStatesRequire=&rGen.MarkedStates();
    mMode |= FmStatesRequire;
  }
  // livelocks only ==> avoid marked states
  if(mode & FmLiveLocksOnly) {
    pStatesAvoid=&rGen.MarkedStates();
    mMode |= FmStatesAvoid;
  }
  // accessible only ==> deal with this in api wrappers
  if(mode & FmIgnoreUnaccessible) {
    (void) mode;
  }  
}


// Filter class for SCC search: clear
void SccFilter::Clear(void) {
  if(mpStatesAvoid) delete mpStatesAvoid;
  if(mpStatesRequire) delete mpStatesRequire;
  if(mpEventsAvoid) delete mpEventsAvoid;
  mpStatesAvoid=0;
  mpStatesRequire=0;
  mpEventsAvoid=0;
  mMode=0x00;
  pStatesAvoid=&msEmptyStates;
  pStatesRequire=&msEmptyStates;
  pEventsAvoid=&msEmptyEvents;
}

// Filter class for SCC search: edit
void SccFilter::StatesAvoid(const StateSet& rStatesAvoid) {
  if(mpStatesAvoid) delete mpStatesAvoid;
  mpStatesAvoid=0;
  mMode |= FmStatesAvoid;
  pStatesAvoid=&rStatesAvoid;
}

// Filter class for SCC search: edit
void SccFilter::StatesRequire(const StateSet& rStatesRequire) {
  if(mpStatesRequire) delete mpStatesRequire;
  mpStatesRequire=0;
  mMode |= FmStatesRequire;
  pStatesRequire=&rStatesRequire;
}

// Filter class for SCC search: edit
void SccFilter::MergeStatesAvoid(const StateSet& rStatesAvoid) {
  if(mpStatesAvoid==0) mpStatesAvoid= new IndexSet(*pStatesAvoid);
  mpStatesAvoid->InsertSet(rStatesAvoid);
  mpStatesAvoid->Name("StatesAvoid");
  pStatesAvoid=mpStatesAvoid;
} 

// Filter class for SCC search: edit
void SccFilter::EventsAvoid(const EventSet& rEventsAvoid) {
  if(mpEventsAvoid) delete mpEventsAvoid;
  mpEventsAvoid=0;
  mMode |= FmEventsAvoid;
  pEventsAvoid=&rEventsAvoid;
}

// Filter class for SCC search: edit
void SccFilter::IgnoreTrivial(bool flag) {
  mMode &= ~FmIgnoreTrivial;
  if(flag) mMode |= FmIgnoreTrivial;
}

// Filter class for SCC search: edit
void SccFilter::FindFirst(bool flag) {
  mMode &= ~FmFindFirst;
  if(flag) mMode |= FmFindFirst;
}




// Recursive function to search for SCCs.
// The algorithm is a variation of the one prsented in
//  -- Aho, Hopcroft, Ullman: The Design and Analysis of Computer Algorithms
void SearchScc(
  const Idx vState,            // Current state under investigation
  int& vRcount,                // Depth count of recursion
  const Generator& rGen,       // Graph to inspect
  const SccFilter& rFilter,    // Filter to ignore transitions with specified events
  StateSet&  rTodo,            // States not dealt with so far
  std::stack<Idx>& rStack,     // Stack of currently considered states
  StateSet& rStackStates,      // Stack contents as set
  std::map<const Idx, int>& rDfn, // Map assigning to each state its depth-first count
  std::map<const Idx, int>& rLowLnk, // Map assigning to each state its LOWLINK Number
  std::list<StateSet>& rSccList,  // Record result: list of SCCs 
  StateSet& rRoots)             // Record result: one state per SCC
{
  FD_DF("SerchScc: -- recursive search from state "<< vState << " at level " << vRcount << " --");
  // local variables (expensive for recursion)
  TransSet::Iterator tit;
  Idx ls;
  bool fl;
  // Take current state from todo list
  rTodo.Erase(vState);
  // Ignore filter: avoid states
  if(rFilter.mMode & SccFilter::FmStatesAvoid)
  if(rFilter.pStatesAvoid->Exists(vState)) return;
  // Ignore filter: break recursion on find first
  if(rFilter.mMode & SccFilter::FmFindFirst)
  if(!rSccList.empty()) return;
  // Record depth-first level for current state and initialise low-link level
  rDfn[vState]=vRcount;
  rLowLnk[vState]=vRcount;
  // Increment depth-first level
  vRcount++;
  // Push state on stack;
  rStack.push(vState);
  rStackStates.Insert(vState);
  /*
  std::list<Idx>::iterator vsit = --rStack.end();
  */
  // Investigate successor states  "L[state]" 
  tit=rGen.TransRelBegin(vState);
  for(; tit != rGen.TransRelEnd(); ++tit) {
    if(tit->X1!=vState) break;
    // Sucessor to investigate
    ls=tit->X2;
    // Ignore avoid: avoid states
    if(rFilter.mMode & SccFilter::FmStatesAvoid)
    if(rFilter.pStatesAvoid->Exists(ls)) continue;
    // Ignore filter: avoid events
    if(rFilter.mMode & SccFilter::FmEventsAvoid)
    if(rFilter.pEventsAvoid->Exists(tit->Ev)) continue;
    // Successors that are on the todo list get ... 
    if(rTodo.Exists(ls)) {        
       // ... searched recursively, and ...
       SearchScc(ls, vRcount, rGen, rFilter, rTodo, rStack, rStackStates, rDfn, rLowLnk, rSccList, rRoots);
       // ... the current low-link gets updated to the new minimum.
       if(rLowLnk[ls]<rLowLnk[vState]) rLowLnk[vState]=rLowLnk[ls];
    }
    // Successors that are not on the todo list ...
    else {
      // ... if they have a lower deph-first level ...
      if(rDfn[ls]<rDfn[vState])  
      // ... and if they are on the stack at all ...
      if(rStackStates.Exists(ls)) 
      // ... set the current low-link to the new minimum 
      if(rDfn[ls]<rLowLnk[vState]) rLowLnk[vState]=rDfn[ls];
    }
  }//end for: iterate successors

  // If the current state is the root of a SCC, record the result
  if(rLowLnk[vState]==rDfn[vState]) {
    // Retrieve SCC from stack
    FD_DF("SearchScc: retrieving SCC from stack, root " << vState);        
    rSccList.push_back(StateSet());
    rRoots.Insert(vState);
    do {
      // Pop top of stack
      ls=rStack.top();
      rStackStates.Erase(ls);
      rStack.pop();
      // Record
      rSccList.back().Insert(ls);
    } while(ls!=vState);
    // Invalidate for missed requirements
    fl=false;
    // .. required states.
    if(rFilter.mMode & SccFilter::FmStatesRequire)
      if(((*rFilter.pStatesRequire) * rSccList.back()).Empty()) 
        fl=true;
    // .. ignore trivial (singleton without relevant selfloop)
    if(rFilter.mMode & SccFilter::FmIgnoreTrivial) {
      if((!fl) && (rSccList.back().Size()==1)) {
	fl=true;
        tit = rGen.TransRelBegin(vState);
        for(; tit != rGen.TransRelEnd(); ++tit) {
          if(tit->X1!=vState) break;
          // no self loop
          if(tit->X2!=vState) continue;
          // avoid event self loop does not qualify
          if(rFilter.mMode & SccFilter::FmEventsAvoid) 
          if(rFilter.pEventsAvoid->Exists(tit->Ev)) 
            continue;
          // found relevant selfloop
          fl=false;
          break;
        }
      }
    }
    // Invalidate result
    if(fl) {
#ifdef FAUDES_DEBUG_FUNCTION
      FD_DF("SearchScc: invalidate scc for filter condition");
#endif
      rSccList.pop_back();
      rRoots.Erase(vState);
    }
  }
}


// ComputeScc(Generator, SccList, Roots)
bool ComputeScc(
  const Generator& rGen,
  const SccFilter& rFilter,
  std::list<StateSet>& rSccList,
  StateSet& rRoots)
{
  FD_DF("CompteScc(" << rGen.Name() << ")");

  // inititalize results:
  rRoots.Clear();
  rSccList.clear();

  // initialize counter for depthfirstnumbers(DFN):
  int count=1;

  // provide local variables 
  std::stack<Idx> stack;
  StateSet stackstates;
  StateSet todostates;
  std::map<const Idx, int> dfn;
  std::map<const Idx, int> lowlnk;

  // initialise todo list
  if(rFilter.Mode() & SccFilter::FmIgnoreUnaccessible)
    todostates=rGen.AccessibleSet();
  else
    todostates=rGen.States();
  if(rFilter.Mode() & SccFilter::FmStatesAvoid)
    todostates = todostates - rFilter.StatesAvoid();

  // each recursion level keeps transition iterators
  rGen.TransRel().Lock();

  // start recursive depth-first search for Scc's:
  while(!todostates.Empty()) {
    SearchScc(*todostates.Begin(), count, rGen, rFilter, todostates, stack, stackstates, 
      dfn, lowlnk,rSccList, rRoots);
  }

  // done
  return !rSccList.empty();
}

// ComputeScc(Generator, SccList, Roots)
bool ComputeScc(
  const Generator& rGen,
  std::list<StateSet>& rSccList,
  StateSet& rRoots)
{
  FD_DF("CompteScc(" << rGen.Name() << ") [std]");

  // dummy
  const static SccFilter msFilter = SccFilter();

  // doit
  return ComputeScc(rGen,msFilter,rSccList,rRoots);
}


// ComputeScc(Generator, Filter, q0, Scc)
bool  ComputeScc(
  const Generator& rGen,
  const SccFilter& rFilter,
  Idx q0, 
  StateSet& rScc)
{
  FD_DF("ComputeScc(" << rGen.Name() << ")");

  // inititalize result
  rScc.Clear();

  // initialize counter for depthfirstnumbers:
  int count=1;

  // provide local variables 
  std::stack<Idx> stack;
  StateSet stackstates;
  StateSet todostates;
  std::map<const Idx, int> dfn;
  std::map<const Idx, int> lowlnk;
  std::list<StateSet> scclist;
  StateSet roots;

  // initialise todo list
  if(rFilter.Mode() & SccFilter::FmIgnoreUnaccessible)
    todostates=rGen.AccessibleSet();
  else
    todostates=rGen.States();
  if(rFilter.Mode() & SccFilter::FmStatesAvoid)
    todostates = todostates - rFilter.StatesAvoid();

  // copy and edit the filter
  SccFilter filter(rFilter);
  filter.FindFirst(true);
  StateSet reqstate;
  reqstate.Insert(q0);
  filter.StatesRequire(reqstate);

  // each recursion level keeps transition iterators
  rGen.TransRel().Lock();

  // start recursive depth-first search for Scc's:
  while(!todostates.Empty()) {
    SearchScc(*todostates.Begin(), count, rGen, filter, todostates, stack, stackstates, 
      dfn, lowlnk, scclist, roots);
  }

  // copy (!) result
  if(!scclist.empty()) rScc=*scclist.begin();

  // done
  return !rScc.Empty();
}


// ComputeScc(Generator, Filter, Scc)
bool ComputeScc(
  const Generator& rGen,
  const SccFilter& rFilter,
  StateSet& rScc)
{
  FD_DF("ComputeScc(" << rGen.Name() << ")");

  // inititalize result
  rScc.Clear();

  // initialize counter for depthfirstnumbers:
  int count=1;

  // provide local variables 
  std::stack<Idx> stack;
  StateSet stackstates;
  StateSet todostates;
  std::map<const Idx, int> dfn;
  std::map<const Idx, int> lowlnk;
  std::list<StateSet> scclist;
  StateSet roots;

  // initialise todo list
  if(rFilter.Mode() & SccFilter::FmIgnoreUnaccessible)
    todostates=rGen.AccessibleSet();
  else
    todostates=rGen.States();
  if(rFilter.Mode() & SccFilter::FmStatesAvoid)
    todostates = todostates - rFilter.StatesAvoid();

  // copy and edit the filter
  SccFilter filter(rFilter);
  filter.FindFirst(true);

  // each recursion level keeps transition iterators
  rGen.TransRel().Lock();

  // start recursive depth-first search for Scc's:
  while(!todostates.Empty()) {
    SearchScc(*todostates.Begin(), count, rGen, filter, todostates, stack, stackstates, 
      dfn, lowlnk, scclist, roots);
  }

  // copy (!) result
  if(!scclist.empty()) rScc=*scclist.begin();

  // done
  return !rScc.Empty();
}

// HasScc(Generator, Filter)
bool HasScc(
  const Generator& rGen,
  const SccFilter& rFilter)
{
  FD_DF("HasScc(" << rGen.Name() << ") [boolean only]");

  // initialize counter for depthfirstnumbers:
  int count=1;

  // provide local variables 
  std::stack<Idx> stack;
  StateSet stackstates;
  StateSet todostates;
  std::map<const Idx, int> dfn;
  std::map<const Idx, int> lowlnk;
  std::list<StateSet> scclist;
  StateSet roots;

  // initialise todo list
  if(rFilter.Mode() & SccFilter::FmIgnoreUnaccessible)
    todostates=rGen.AccessibleSet();
  else
    todostates=rGen.States();
  if(rFilter.Mode() & SccFilter::FmStatesAvoid)
    todostates = todostates - rFilter.StatesAvoid();

  // copy and edit the filter
  SccFilter filter(rFilter);
  filter.FindFirst(true);

  // each recursion level keeps transition iterators
  rGen.TransRel().Lock();

  // start recursive depth-first search for Scc's:
  while(!todostates.Empty()) {
    SearchScc(*todostates.Begin(), count, rGen, filter, todostates, stack, stackstates, 
      dfn, lowlnk, scclist, roots);
  }

  // done
  return !scclist.empty();

}


// RTI wrapper
bool ComputeNextScc(
  const Generator& rGen,
  SccFilter& rFilter,
  StateSet& rScc
) {
  // find first
  rFilter.FindFirst(true);
  bool res = ComputeScc(rGen,rFilter,rScc);
  // fix filter
  if(res) rFilter.MergeStatesAvoid(rScc);
  rFilter.FindFirst(false);
  // done
  return res;
}



/*

// WriteStateSets(rStateSets)
// Write set of StateSet's to console.
void WriteStateSets(
  const std::set<StateSet>& rStateSets)
{
  FD_DF("WriteStateSets()");
  std::cout<<std::endl;
  if(rStateSets.empty()) {
    std::cout<<"WriteStateSets: Set of state sets is empty."<<std::endl;
    FD_DF("WriteStateSets: Set of state sets is empty.");
    return;
  }
  std::cout<<">WriteStateSets: Set of state sets begin:"<< std::endl;
  std::set<StateSet>::iterator StateSetit = rStateSets.begin();
  std::set<StateSet>::iterator StateSetit_end = rStateSets.end();
  for (; StateSetit != StateSetit_end; ++StateSetit) {
    std::cout<<std::endl<<"  >> State set begin:"<< std::endl;
    StateSet::Iterator sit = StateSetit->Begin();
    StateSet::Iterator sit_end = StateSetit->End();
    for (; sit != sit_end; ++sit) {
      std::cout<<"  >> "<<*sit<<std::endl;
    }
    std::cout<<"  >> State set end."<< std::endl;
  }  
  std::cout<<std::endl<<">WriteStateSets: Set of state sets end."<<std::endl;
}

// WriteStateSets(rGen,rStateSets)
// Write set of StateSet's to console.
// Use rGen for symbolic state names
void WriteStateSets(
  const Generator& rGen,
  const std::set<StateSet>& rStateSets)
{
  FD_DF("WriteStateSets()");
  std::cout<<std::endl;
  if(rStateSets.empty()) {
    std::cout<<"WriteStateSets: Set of state sets is empty."<<std::endl;
    FD_DF("WriteStateSets: Set of state sets is empty.");
    return;
  }
  std::cout<<">WriteStateSets: Set of state sets begin:"<< std::endl;
  std::set<StateSet>::iterator StateSetit = rStateSets.begin();
  std::set<StateSet>::iterator StateSetit_end = rStateSets.end();
  for (; StateSetit != StateSetit_end; ++StateSetit) {
    std::cout<<std::endl<<"  >> State set begin:"<< std::endl;
    std::cout<<"  "<<rGen.StateSetToString(*StateSetit)<<std::endl;
    std::cout<<"  >> State set end."<< std::endl;
  }  
  std::cout<<std::endl<<">WriteStateSets: Set of state sets end."<<std::endl;
}

*/

}//namespace faudes

