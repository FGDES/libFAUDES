/** @file mtc_supcon.cpp

Supremal controllable sublanguage and controllablity

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


#include "mtc_supcon.h"
#include "syn_include.h"

namespace faudes {

/*
******************************
* SUPCON: USER FUNCTIONS     *
******************************
*/


// mtcSupConNB(rPlantGen, rSpecGen, rResGen)
void mtcSupConNB(const  MtcSystem& rPlantGen, const MtcSystem& rSpecGen, MtcSystem& rResGen) {
  // HELPERS:
  std::map< std::pair<Idx,Idx>, Idx> rcmap;
  // ALGORITHM:
  mtcSupConNB(rPlantGen, rSpecGen, rcmap, rResGen);
}


// mtcSupConNB(rPlantGen, rSpecGen, rReverseCompositionMap, rResGen)
void mtcSupConNB(const  MtcSystem& rPlantGen, const MtcSystem& rSpecGen,
  std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap,  MtcSystem& rResGen) {
  FD_DF("mtcSupConNB(" << &rPlantGen << "," << &rSpecGen << ")");

  // PREPARE RESULT:

  rResGen.Clear();
  rResGen.Name("mtcSupConNB(("+rPlantGen.Name()+"),("+rSpecGen.Name()+"))");
  rResGen.InjectAlphabet(rPlantGen.Alphabet());

  rResGen.SetControllable(rPlantGen.ControllableEvents());
  rResGen.SetForcible(rPlantGen.ForcibleEvents());
  rResGen.ClrObservable(rPlantGen.UnobservableEvents());

  // HELPERS:

  // controllable events
  const EventSet ualph = rPlantGen.UncontrollableEvents();
  FD_DF("mtcSupConNB: controllable events: " << rPlantGen.ControllableEvents().ToString());
  FD_DF("mtcSupConNB: uncontrollable events: " << ualph.ToString());

  // CONSISTENCY CHECK:

  // alphabets must match
  if (rPlantGen.Alphabet() != rSpecGen.Alphabet()) {
    EventSet only_in_plant = rPlantGen.Alphabet() - rSpecGen.Alphabet();
    EventSet only_in_spec = rSpecGen.Alphabet() - rPlantGen.Alphabet();
    std::stringstream errstr;
    errstr << "Alphabets of generators do not match. Only in plant: " << only_in_plant.ToString()
      << ". Only in spec: " << only_in_spec.ToString() << ".";
    throw Exception("mtcSupConNB", errstr.str(), 500);
  }

  // plant and spec must be deterministic
  bool plant_det = rPlantGen.IsDeterministic();
  bool spec_det = rSpecGen.IsDeterministic();

  if ((plant_det == false) && (spec_det == true)) {
    std::stringstream errstr;
    errstr << "Plant generator must be deterministic, "
                    << "but is nondeterministic";
    throw Exception("mtcSupConNB", errstr.str(), 501);
  }
  else if ((plant_det == true) && (spec_det == false)) {
    std::stringstream errstr;
    errstr << "Spec generator must be deterministic, "
                    << "but is nondeterministic";
    throw Exception("mtcSupConNB", errstr.str(), 503);
  }
  else if ((plant_det == false) && (spec_det == false)) {
    std::stringstream errstr;
    errstr << "Plant and spec generator must be deterministic, "
                    << "but both are nondeterministic";
    throw Exception("mtcSupConNB", errstr.str(), 504);
  }

  // ALGORITHM:

  mtcSupConParallel(rPlantGen, rSpecGen, ualph, rCompositionMap, rResGen);
  FD_DF("mtcSupConNB: mtcSupConParallel passed...");

  // make resulting generator trim until it's fully controllable
  while (1) {
    if(rResGen.Empty()) break;
    Idx state_num = rResGen.Size();
    mtcSupConUnchecked(rPlantGen, rPlantGen.ControllableEvents(), rResGen);
    rResGen.StronglyTrim();
    if(rResGen.Size() == state_num) break;
    FD_DF("mtcSupConNB: rResGen.Size() = " << ToStringInteger(rResGen.Size()) << ", state_num = " << ToStringInteger(state_num));
  }

  // restrict composition map
  std::map< std::pair<Idx,Idx>, Idx>::iterator rcmapit = rCompositionMap.begin();
  for(; rcmapit != rCompositionMap.end(); rcmapit++)
    if(!rResGen.ExistsState(rcmapit->second)) rCompositionMap.erase(rcmapit++);


}

// mtcSupConClosed(rPlantGen, rSpecGen, rResGen)
void mtcSupConClosed(const  MtcSystem& rPlantGen, const MtcSystem& rSpecGen, MtcSystem& rResGen) {
  // HELPERS:
  std::map< std::pair<Idx,Idx>, Idx> rcmap;

  // ALGORITHM:
  mtcSupConClosed(rPlantGen, rSpecGen, rcmap, rResGen);
}


// mtcSupConClosed(rPlantGen, rSpecGen, rCompositionMap, rResGen)
void mtcSupConClosed(const  MtcSystem& rPlantGen, const MtcSystem& rSpecGen,
  std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap,  MtcSystem& rResGen) {
  FD_DF("mtcSupConClosed(" << &rPlantGen << "," << &rSpecGen << ")");

  // PREPARE RESULT:
  rResGen.Clear();
  rResGen.Name("mtcSupConClosed(("+rPlantGen.Name()+"),("+rSpecGen.Name()+"))");

  rResGen.InjectAlphabet(rPlantGen.Alphabet());

  rResGen.SetControllable(rPlantGen.ControllableEvents());
  rResGen.SetForcible(rPlantGen.ForcibleEvents());
  rResGen.ClrObservable(rPlantGen.UnobservableEvents());

  // HELPERS:

  // controllable events
  const EventSet ualph = rPlantGen.UncontrollableEvents();
  FD_DF("mtcSupConClosed: controllable events: " << rPlantGen.ControllableEvents().ToString());
  FD_DF("mtcSupConClosed: uncontrollable events: " << ualph.ToString());

  // CONSISTENCY CHECK:

  // alphabets must match
  if (rPlantGen.Alphabet() != rSpecGen.Alphabet()) {
    EventSet only_in_plant = rPlantGen.Alphabet() - rSpecGen.Alphabet();
    EventSet only_in_spec = rSpecGen.Alphabet() - rPlantGen.Alphabet();
    std::stringstream errstr;
    errstr << "Alphabets of generators do not match. Only in plant: " 
      << only_in_plant.ToString() << ". Only in spec: "
      << only_in_spec.ToString() << ".";
    throw Exception("mtcSupConClosed", errstr.str(), 500);
  }

  // plant and spec must be deterministic
  bool plant_det = rPlantGen.IsDeterministic();
  bool spec_det = rSpecGen.IsDeterministic();

  if ((plant_det == false) && (spec_det == true)) {
    std::stringstream errstr;
    errstr << "Plant generator must be deterministic, " << "but is nondeterministic";
    throw Exception("mtcSupConClosed", errstr.str(), 501);
  }
  else if ((plant_det == true) && (spec_det == false)) {
    std::stringstream errstr;
    errstr << "Spec generator must be deterministic, " << "but is nondeterministic";
    throw Exception("mtcSupConClosed", errstr.str(), 503);
  }
  else if ((plant_det == false) && (spec_det == false)) {
    std::stringstream errstr;
    errstr << "Plant and spec generator must be deterministic, "
                    << "but both are nondeterministic";
    throw Exception("mtcSupCon", errstr.str(), 504);
  }

  // ALGORITHM:

  // parallel composition
  mtcSupConParallel(rPlantGen, rSpecGen, ualph, rCompositionMap, rResGen);
  // make resulting generator controllable
  mtcSupConUnchecked(rPlantGen, rPlantGen.ControllableEvents(), rResGen);

  // restrict composition map
  std::map< std::pair<Idx,Idx>, Idx>::iterator rcmapit = rCompositionMap.begin();
  for(; rcmapit != rCompositionMap.end(); rcmapit++)
    if(!rResGen.ExistsState(rcmapit->second)) rCompositionMap.erase(rcmapit++);


}



/*
********************************************
* Fast parallel composition for mtcSupCon     *
********************************************
*/


// mtcSupConParallel(rPlantGen, rSpecGen, rUAlph, rReverseCompositionMap, rResGen)
void mtcSupConParallel(const  MtcSystem& rPlantGen, const MtcSystem& rSpecGen, const EventSet& rUAlph,
  std::map< std::pair<Idx,Idx>, Idx>& rReverseCompositionMap, MtcSystem& rResGen) {
  FD_DF("mtcSupConParallel(" << &rPlantGen << "," << &rSpecGen << ")");

  // HELPERS:

  // todo stack
  std::stack< std::pair<Idx,Idx> > todo;
  // set of forbidden states
  StateSet forbidden;
  // actual pair, new pair
  std::pair<Idx,Idx> currentstates, newstates;
  // other stuff
  Idx tmpstate;
  std::map< std::pair<Idx,Idx>, Idx>::iterator rcmapit;
  StateSet::Iterator lit1, lit2;
  TransSet::Iterator titg, titg_end, tith, tith_end;

  // Prepare
  rResGen.ClearStates();

  // ALGORITHM:
  if (rPlantGen.InitStatesEmpty()) {
    FD_DF("SupconParallel: plant got no initial states. "
        << "parallel composition contains empty language.");
    return;
  }

  if (rSpecGen.InitStatesEmpty()) {
    FD_DF("mtcSupConParallel: spec got no initial states. "
        << "parallel composition contains empty language.");
    return;
  }
  ColorSet plantColors = rPlantGen.Colors();
  ColorSet specColors = rSpecGen.Colors();
  // create initial state
  currentstates = std::make_pair(*rPlantGen.InitStatesBegin(), *rSpecGen.InitStatesBegin());
  todo.push(currentstates);
  ColorSet ComposedSet;
  ComposedColorSet(rPlantGen, currentstates.first, plantColors, rSpecGen, currentstates.second,
      specColors, ComposedSet);
    if (! ComposedSet.Empty() ) {
      Idx StateIndex = rResGen.InsInitState();
      rReverseCompositionMap[currentstates] = StateIndex;
      rResGen.InsColors(StateIndex, ComposedSet);
      FD_DF("mtcSupConParallel: NEW IMSTATE: (" << rPlantGen.SStr(currentstates.first)
          << "|" << rSpecGen.SStr(currentstates.second) << ") -> "	<< rReverseCompositionMap[currentstates]);
    }
    else {
      rReverseCompositionMap[currentstates] = rResGen.InsInitState();
      FD_DF("mtcSupConParallel: NEW ISTATE: (" << rPlantGen.SStr(currentstates.first) 
        << "|" << rSpecGen.SStr(currentstates.second) << ") -> " << rReverseCompositionMap[currentstates]);
    }

    // first do parallel composition of allowed states
    // by deleting forbidden states on the fly.
    // this creates an accessible new generator
    FD_DF("mtcSupConParallel: *** processing reachable states ***");
    while (! todo.empty()) {
      // get next reachable pair of states from todo stack
      currentstates = todo.top();
      todo.pop();
      FD_DF("mtcSupConParallel: todo pop: (" << rPlantGen.SStr(currentstates.first) 
        << "|" << rSpecGen.SStr(currentstates.second) << ") -> " << rReverseCompositionMap[currentstates]);

      titg = rPlantGen.TransRelBegin(currentstates.first);
      titg_end = rPlantGen.TransRelEnd(currentstates.first);
      tith = rSpecGen.TransRelBegin(currentstates.second);
      tith_end = rSpecGen.TransRelEnd(currentstates.second);

#ifdef FAUDES_DEBUG_FUNCTION
      // print all transitions of current states
      FD_DF("mtcSupConParallel: transitions from current states:");
      for (;titg != titg_end; ++titg) { 
        FD_DF("mtcSupConParallel: g: " << rPlantGen.SStr(titg->X1) << "-" << rPlantGen.EStr(titg->Ev) 
                              << "-" << rPlantGen.SStr(titg->X2));
      }
      for (;tith != tith_end; ++tith) {
        FD_DF("mtcSupConParallel: h: " << rSpecGen.SStr(tith->X1) << "-" << rSpecGen.EStr(tith->Ev) 
                              << "-" << rSpecGen.SStr(tith->X2));
      }
      titg = rPlantGen.TransRelBegin(currentstates.first);
              tith = rSpecGen.TransRelBegin(currentstates.second);
#endif
      // process all h transitions while there could be matching g transitions
      while ((tith != tith_end) && (titg != titg_end)) {
        FD_DF("mtcSupConParallel: actual g-transition: " << rPlantGen.SStr(titg->X1) 
                        << "-" << rPlantGen.EStr(titg->Ev) << "-" << rPlantGen.SStr(titg->X2));
        FD_DF("mtcSupConParallel: actual h-transition: " << rSpecGen.SStr(tith->X1) 
                        << "-" << rSpecGen.EStr(tith->Ev) << "-" << rSpecGen.SStr(tith->X2));
        // execute common events
        if (titg->Ev == tith->Ev) {
          FD_DF("mtcSupConParallel: executing common event " 
                          << rPlantGen.EStr(titg->Ev));
          newstates = std::make_pair(titg->X2, tith->X2);
          rcmapit = rReverseCompositionMap.find(newstates);
          // add to todo list if state is new
          if (rcmapit == rReverseCompositionMap.end()) {
            todo.push(newstates);
            // if colored state
            ComposedColorSet(rPlantGen, newstates.first, plantColors, rSpecGen, newstates.second,
               specColors, ComposedSet);
            if(!ComposedSet.Empty() ) {
              tmpstate = rResGen.InsState();
              rResGen.InsColors(tmpstate, ComposedSet);
              FD_DF("mtcSupConParallel: NEW MSTATE: (" 
                << rPlantGen.SStr(newstates.first) << "|"
                << rSpecGen.SStr(newstates.second) << ") -> " << tmpstate);
            }
            // if "normal" state
            else {
              tmpstate = rResGen.InsState();
              FD_DF("mtcSupConParallel: NEW STATE: (" 
                << rPlantGen.SStr(newstates.first) << "|" 
                << rSpecGen.SStr(newstates.second) << ") -> " << tmpstate);
            }
            rReverseCompositionMap[newstates] = tmpstate;
            FD_DF("mtcSupConParallel: todo push: (" << rPlantGen.SStr(newstates.first)
              << "|" << rSpecGen.SStr(newstates.second) << ") -> " << tmpstate);
          }
          // if state already exists
          else {
            tmpstate = rcmapit->second;
          }
          // if successor state not in forbidden set add transition
          if (! forbidden.Exists(tmpstate)) {
            FD_DF("mtcSupConParallel: ADDING TRANSITION " 
              << rPlantGen.SStr(rReverseCompositionMap[currentstates]) << "-" << rPlantGen.EStr(titg->Ev) 
              << "-" << rPlantGen.SStr(tmpstate));
            rResGen.SetTransition(rReverseCompositionMap[currentstates], titg->Ev, tmpstate);
            FD_DF("mtcSupConParallel: incrementing g transrel");
            ++titg;
            FD_DF("mtcSupConParallel: incrementing h transrel");
            ++tith;
          }
          // if successor state in forbidden and event uncontrollable 
          // delete state
          else if (rUAlph.Exists(titg->Ev)) {
            FD_DF("mtcSupConParallel: successor " << rSpecGen.SStr(tmpstate) 
              << "in forbidden and common event " << rSpecGen.EStr(titg->Ev) 
              << " uncontrollable:");
            FD_DF("mtcSupConParallel: forbidden insert" << rPlantGen.SStr(tmpstate));
            forbidden.Insert(tmpstate);
#ifdef FAUDES_CHECKED
            // do not end while loops here for detecting all h transitions 
            // not in g
            FD_DF("mtcSupConParallel: incrementing g transrel (FAUDES_CHECKED)");
            ++titg;
            FD_DF("mtcSupConParallel: incrementing h transrel (FAUDES_CHECKED)");
            ++tith;
#else
            // exit all loops
            titg = titg_end;
            tith = tith_end;
#endif
          }
          // else if successor state in forbidden and event controllable
          else {
            FD_DF("mtcSupConParallel: incrementing g transrel");
            ++titg;
            FD_DF("mtcSupConParallel: incrementing h transrel");
            ++tith;
          }
        }
        // if g got some more transitions try to resync events
        else if (titg->Ev < tith->Ev) {
          FD_DF("mtcSupConParallel: asynchronous execution of event " 
            << rPlantGen.EStr(titg->Ev) << " in g while " << rSpecGen.EStr(tith->Ev)
          << " in h");
          // if uncontrollable transition leaves specification
          // delete state from res and put into forbiddenset
          if (rUAlph.Exists(titg->Ev)) {
            FD_DF("mtcSupConParallel: asynchronous event " << rPlantGen.EStr(titg->Ev) 
                            << " in g is uncontrollable");
            tmpstate = rReverseCompositionMap[currentstates];	
            FD_DF("mtcSupConParallel: forbidden insert" << rPlantGen.SStr(tmpstate));
            forbidden.Insert(tmpstate);
            // exit all loops
            titg = titg_end;
            tith = tith_end;
            break;
          }
          FD_DF("mtcSupConParallel: incrementing g transrel");
          ++titg;
        } // if specification leaves plant model emit warning
        else {
#ifdef FAUDES_CHECKED
//          FD_WARN("mtcSupConParallel:  transition " << rSpecGen.SStr(tith->X1) 
//            << "-" << rSpecGen.EStr(tith->Ev) << "-" << rSpecGen.SStr(tith->X2) 
//            << " in specification h not found in g");
#endif
          FD_DF("mtcSupConParallel: incrementing h transrel");
          ++tith;
        }
      }
      if (rResGen.InitStates().Empty()) {
	FD_DF("mtcSupConParallel: rResGen has no initial states... (2)");
      }
      
      // if g got some more transitions not in h
      while (titg != titg_end) {
        FD_DF("mtcSupConParallel: asynchronous execution of event " 
          << rPlantGen.EStr(titg->Ev) << " in g at end of h");
        FD_DF("mtcSupConParallel: actual g-transition: " << rPlantGen.SStr(titg->X1) 
          << "-" << rPlantGen.EStr(titg->Ev) << "-" << rPlantGen.SStr(titg->X2));
        FD_DF("mtcSupConParallel: actual h-transition: end");
        // if uncontrollable transition leaves specification
        if (rUAlph.Exists(titg->Ev)) {
          tmpstate = rReverseCompositionMap[currentstates];
          FD_DF("mtcSupConParallel: asynchron executed uncontrollable end "
            << "event " << rPlantGen.EStr(titg->Ev) << " leaves specification:");
          FD_DF("mtcSupConParallel: forbidden insert" << rPlantGen.SStr(tmpstate));
          forbidden.Insert(tmpstate);
          // exit this loop
          // if FAUDES_CHECKED not defined this means exiting all loops
          break;
        }
        FD_DF("mtcSupConParallel: incrementing g transrel");
        ++titg;
      }
#ifdef FAUDES_CHECKED
      // if h got some more transitions not in g
//      while (tith != tith_end) {
//  FD_WARN("mtcSupConParallel:  transition " << rSpecGen.SStr(tith->X1) << "-" 
//    << rSpecGen.EStr(tith->Ev) << "-" << rSpecGen.SStr(tith->X2) 
//    << "in specification h not found in g");
//  FD_DF("mtcSupConParallel: incrementing h transrel");
//    ++tith;
// }
#endif
    }
    FD_DF("mtcSupConParallel: deleting forbidden states...");
    // remove forbidden states from stateset
    rResGen.DelStates(forbidden);
}

/*
*****************************************
* mtcSupConUnchecked: REAL SUPCON FUNCTION *
*****************************************
*/


// mtcSupConUnchecked(rPlantGen, rSpecGen, rCAlph)
void mtcSupConUnchecked(const  MtcSystem& rPlantGen, const EventSet& rCAlph, MtcSystem& rSpecGen) {

  FD_DF("mtcSupConUnchecked(" << &rSpecGen << "," << &rPlantGen << ")");

  // HELPERS:

  // forbiden states
  StateSet forbidden;
  // todo stack
  std::stack<Idx> todog, todoh;
  // set of already discovered states
  StateSet discovered;
  // reverse sorted transition relation build on the fly
  TransSetX2EvX1 rtransrel;


  // ALGORITHM:

  // bail out if there is no initial state
  // if (rPlantGen.InitStatesEmpty() || rSpecGen.InitStatesEmpty());

  // push combined initial state on todo stack
  todog.push(*rPlantGen.InitStatesBegin());
  todoh.push(*rSpecGen.InitStatesBegin());
  FD_DF("mtcSupCon: todo push: (" << rPlantGen.SStr(*rPlantGen.InitStatesBegin()) << "|"
    << rSpecGen.SStr(*rSpecGen.InitStatesBegin()) << ")");

  // process todo stack
  while (! todog.empty()) {
    // get top elements from todo stack
    Idx currentg = todog.top();
    Idx currenth = todoh.top();
    todog.pop();
    todoh.pop();
    FD_DF("mtcSupCon: todo pop: (" << rPlantGen.SStr(currentg) << "|" 
      << rSpecGen.SStr(currenth) << ")");

#ifdef FAUDES_DEBUG_FUNCTION
    TransSet::Iterator _titg, _tith;
    // print all transitions of current states
    FD_DF("mtcSupCon: transitions from current states:");
    for (_titg = rPlantGen.TransRelBegin(currentg); _titg != rPlantGen.TransRelEnd(currentg); ++_titg) 
    FD_DF("mtcSupCon: g: " << rPlantGen.SStr(_titg->X1) << "-" 
      << rPlantGen.EStr(_titg->Ev) << "-" << rPlantGen.SStr(_titg->X2));
    for (_tith = rSpecGen.TransRelBegin(currenth); _tith != rSpecGen.TransRelEnd(currenth); ++_tith) 
    FD_DF("mtcSupCon: h: " << rSpecGen.SStr(_tith->X1) << "-" 
      << rSpecGen.EStr(_tith->Ev) << "-" << rSpecGen.SStr(_tith->X2));
#endif 

    // process all h transitions while there could be matching g transitions
    TransSet::Iterator titg = rPlantGen.TransRelBegin(currentg);
    TransSet::Iterator titg_end = rPlantGen.TransRelEnd(currentg);
    TransSet::Iterator tith = rSpecGen.TransRelBegin(currenth);
    TransSet::Iterator tith_end = rSpecGen.TransRelEnd(currenth);
    while ((tith != tith_end) && (titg != titg_end)) {
      FD_DF("mtcSupCon: actual g-transition: " << rPlantGen.SStr(titg->X1) 
        << "-" << rPlantGen.EStr(titg->Ev) << "-" << rPlantGen.SStr(titg->X2));
      FD_DF("mtcSupCon: actual h-transition: " << rSpecGen.SStr(tith->X1) 
        << "-" << rSpecGen.EStr(tith->Ev) << "-" << rSpecGen.SStr(tith->X2));
      // execute common events
      if (titg->Ev == tith->Ev) {
        FD_DF("mtcSupCon: executing common event " << rPlantGen.EStr(titg->Ev));
        // add to todo list if state is undiscovered
        if (! discovered.Exists(currenth)) {
          todog.push(titg->X2);
          todoh.push(tith->X2);
          FD_DF("mtcSupCon: todo push: (" << rPlantGen.SStr(titg->X2) << "|"
            << rSpecGen.SStr(tith->X2) << ")");
        }
        // if h successor state not in forbidden set add transition to rtransrel
        if (! forbidden.Exists(tith->X2)) {
          rtransrel.Insert(*tith);
          FD_DF("mtcSupCon: incrementing g transrel");
          ++titg;
          FD_DF("mtcSupCon: incrementing h transrel");
          ++tith;
        }
        // if successor state is forbidden and event uncontrollable 
        else if (!rCAlph.Exists(titg->Ev)) {
          FD_DF("mtcSupCon: successor state " << rSpecGen.SStr(tith->X2) << 
            " forbidden and event " << rPlantGen.EStr(titg->Ev) << " uncontrollable:");
          FD_DF("mtcSupCon: TraverseUncontrollableBackwards(" << rSpecGen.SStr(currenth) << ")");
          TraverseUncontrollableBackwards(rCAlph, rtransrel, forbidden, currenth);
#ifdef FAUDES_CHECKED
          // just increment transrel iterators to find all h transitions not in g
          FD_DF("mtcSupCon: incrementing g transrel (FAUDES_CHECKED)");
          ++titg;
          FD_DF("mtcSupCon: incrementing h transrel (FAUDES_CHECKED)");
          ++tith;
#else
          // exit all loops
          titg = titg_end;
          tith = tith_end;
#endif
          break;
        }
        // else if successor state in forbidden and event controllable
        else {
          FD_DF("mtcSupCon: incrementing g transrel");
          ++titg;
          FD_DF("mtcSupCon: incrementing h transrel");
          ++tith;
        }
      }
      // if g got some more transitions try to resync events
      else if (titg->Ev < tith->Ev) {
        FD_DF("mtcSupCon: asynchronous execution of event " 
          << rPlantGen.EStr(titg->Ev) << " in g while " << rSpecGen.EStr(tith->Ev)
          << " in h");
        // if uncontrollable transition leaves specification
        // delete state from rResGen and put into forbiddenset
        if (!rCAlph.Exists(titg->Ev)) {
        FD_DF("mtcSupCon: asynchronous event " << rPlantGen.EStr(titg->Ev) 
          << " in g is uncontrollable");
        FD_DF("mtcSupCon: TraverseUncontrollableBackwards(" << rSpecGen.SStr(currenth) << ")");
        TraverseUncontrollableBackwards(rCAlph, rtransrel, forbidden, currenth);
        // exit all loops over g transrel
          titg = titg_end;
          break;
        }
        FD_DF("mtcSupCon: incrementing g transrel");
        ++titg;
      }
      // if specification leaves plant model emit warning
      else {
#ifdef FAUDES_CHECKED
        FD_WARN("mtcSupCon: transition " << rSpecGen.SStr(tith->X1) << "-" 
          << rSpecGen.EStr(tith->Ev) << "-" << rSpecGen.SStr(tith->X2)
          << "in specification h not found in g");
#endif
        FD_DF("mtcSupCon: incrementing h transrel");
        ++tith;
      }
    }
    // process other transitions not in h
    while (titg != titg_end) {
      FD_DF("mtcSupCon: asynchronous execution of event " 
        << rPlantGen.EStr(titg->Ev) << " in g at end of h");
      FD_DF("mtcSupCon: actual g-transition: " << rPlantGen.SStr(titg->X1) 
        << "-" << rPlantGen.EStr(titg->Ev) << "-" << rPlantGen.SStr(titg->X2));
      FD_DF("mtcSupCon: actual h-transition: end");
      // if uncontrollable transition leaves specification
      if (!rCAlph.Exists(titg->Ev)) {
        FD_DF("mtcSupCon: asynchronous execution of uncontrollable event "
          << rPlantGen.EStr(titg->Ev) << " in g");
        FD_DF("mtcSupCon: TraverseUncontrollableBackwards(" << rPlantGen.SStr(currenth) << ")");
        TraverseUncontrollableBackwards(rCAlph, rtransrel, forbidden, currenth);
        // exit this loop
        break;
      }
      FD_DF("mtcSupCon: incrementing g transrel");
      ++titg;
    }
#ifdef FAUDES_CHECKED
    // process other transitions not in g
    while (tith != tith_end) {
      FD_WARN("mtcSupCon: transition " << rSpecGen.SStr(tith->X1) << "-" 
        << rSpecGen.EStr(tith->Ev) << "-" << rSpecGen.SStr(tith->X2)
        << "in specification h not found in g");
      FD_DF("mtcSupCon: incrementing h transrel");
      ++tith;
    }
#endif
    discovered.Insert(currenth);
  }

  // compute complete set of forbidden states
  forbidden = rSpecGen.States() - ( discovered - forbidden );

  // remove forbidden states from stateset
  rSpecGen.DelStates(forbidden);
}




} // namespace faudes
