/** @file syn_supcon.cpp Supremal controllable sublanguage */

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
   

#include "syn_supcon.h"


namespace faudes {




/*
***************************************************************************************
***************************************************************************************
 Implementation
***************************************************************************************
***************************************************************************************
*/


/*
Revision fro libFAUDES 2.20b:

SupCon and IsControllable used to share the same algorithm to identify critical
states, implemented as IsControllableUnchecked().

The implementation, however, was only correct under the assumption that the state set of the 
supervisor/candidate H would also distinguish plant states. The later assumption is
true in the context of SupCon, where the candidate is build as the parallel composition. 
It is not true in general.

Thus, in libFAUDES 2.20b the original algorithm was moved from  IsControllableUnchecked to
SupConClosedUnchecked and IsControllableUnchecked was reimplemented.
*/


// SupConClosedUnchecked(rPlantGen, rCAlph, rSupCandGen)
void SupConClosedUnchecked(
  const Generator& rPlantGen,  // aka G
  const EventSet& rCAlph,  
  Generator& rSupCandGen       // aka H
) 
{
  FD_DF("SupConClosedUnchecked(" << &rSupCandGen << "," << &rPlantGen << ")");

  // bail out if plant or candidate contain no initial states
  if(rPlantGen.InitStatesEmpty() || rSupCandGen.InitStatesEmpty()) 
    return;
   
  // todo stack (pairs of states)
  std::stack<Idx> todog, todoh;
  // set of already processed states of H (this is where we need H-states to also distinguish G-states) 
  StateSet processed;
  // reverse sorted transition relation of H (to be built on the fly)
  TransSetX2EvX1 rtransrel;
  // critical states
  StateSet critical;

  // push combined initial state on todo stack
  todog.push(*rPlantGen.InitStatesBegin());
  todoh.push(*rSupCandGen.InitStatesBegin());
  FD_DF("SupCon: todo push: (" << rPlantGen.SStr(*rPlantGen.InitStatesBegin()) << "|"
     << rSupCandGen.SStr(*rSupCandGen.InitStatesBegin()) << ")");

  // process todo stack
  while (! todog.empty()) {
    // allow for user interrupt
    // LoopCallback();
    // allow for user interrupt, incl progress report
    FD_WPC(1,2,"Controllability(): iterating states"); 
    // get top element from todo stack
    Idx currentg = todog.top();
    Idx currenth = todoh.top();
    todog.pop();
    todoh.pop();
    FD_DF("SupCon: todo pop: (" << rPlantGen.SStr(currentg) << "|" 
       << rSupCandGen.SStr(currenth) << ")");

    // break on doublets (tmoor 201104)
    if(processed.Exists(currenth)) continue;

#ifdef FAUDES_DEBUG_FUNCTION
    TransSet::Iterator _titg, _tith;
    // print all transitions of current states
    FD_DF("SupCon: transitions from current states:");
    for (_titg = rPlantGen.TransRelBegin(currentg); _titg != rPlantGen.TransRelEnd(currentg); ++_titg) 
       FD_DF("SupCon: g: " << rPlantGen.SStr(_titg->X1) << "-" 
		 << rPlantGen.EStr(_titg->Ev) << "-" << rPlantGen.SStr(_titg->X2));
    for (_tith = rSupCandGen.TransRelBegin(currenth); _tith != rSupCandGen.TransRelEnd(currenth); ++_tith) 
       FD_DF("SupCon: h: " << rSupCandGen.SStr(_tith->X1) << "-" 
		 << rSupCandGen.EStr(_tith->Ev) << "-" << rSupCandGen.SStr(_tith->X2));
#endif 

    // process all h transitions while there could be matching g transitions
    TransSet::Iterator titg = rPlantGen.TransRelBegin(currentg);
    TransSet::Iterator titg_end = rPlantGen.TransRelEnd(currentg);
    TransSet::Iterator tith = rSupCandGen.TransRelBegin(currenth);
    TransSet::Iterator tith_end = rSupCandGen.TransRelEnd(currenth);
    while ((tith != tith_end) && (titg != titg_end)) {
      FD_DF("SupCon: processing g-transition: " << rPlantGen.SStr(titg->X1) 
		  << "-" << rPlantGen.EStr(titg->Ev) << "-" << rPlantGen.SStr(titg->X2));
      FD_DF("SupCon: processing h-transition: " << rSupCandGen.SStr(tith->X1) 
	          << "-" << rSupCandGen.EStr(tith->Ev) << "-" << rSupCandGen.SStr(tith->X2));
      // increment tith and titg case A: process common events
      if (titg->Ev == tith->Ev) {
	FD_DF("SupCon: processing common event " << rPlantGen.EStr(titg->Ev));
	// add to todo list if state not processed (tmoor 201104: record next H state on stack)
	if(!processed.Exists(tith->X2)) {
	  todog.push(titg->X2);
	  todoh.push(tith->X2);
	  FD_DF("SupCon: todo push: (" << rPlantGen.SStr(titg->X2) << "|"
	    << rSupCandGen.SStr(tith->X2) << ")");
	}
	// if h successor state is not critical add transition to rtransrel
	if (!critical.Exists(tith->X2)) {
	  rtransrel.Insert(*tith);
	  FD_DF("SupCon: incrementing g and h transrel");
	  ++titg;
	  ++tith;
	}
	// if successor state is critical and event uncontrollable 
	else if (!rCAlph.Exists(titg->Ev)) {
	  FD_DF("SupCon: successor state " << rSupCandGen.SStr(tith->X2) << 
	    " critical and event " << rPlantGen.EStr(titg->Ev) << " uncontrollable:");
	  FD_DF("SupCon: TraverseUncontrollableBackwards(" << rSupCandGen.SStr(currenth) << ")");
	    TraverseUncontrollableBackwards(rCAlph, rtransrel, critical, currenth);
#ifdef FAUDES_CHECKED
	    // just increment transrel iterators to find all h transitions not in g
	    FD_DF("IsControllable: incrementing g an h transrel (FAUDES_CHECKED)");
	    ++titg;
	    ++tith;
	    break;
#else
            // exit all loops
	    titg = titg_end;
	    tith = tith_end;
	    break;
#endif
	}
        // if successor state is critical and event controllable
        else {
          FD_DF("SupCon: incrementing g and h transrel");
          ++titg;
          ++tith;
        }		
      }
      // increment tith and titg case B: process g event that is not enabled in h
      else if (titg->Ev < tith->Ev) {
	FD_DF("SupCon: asynchronous execution of event " 
	         << rPlantGen.EStr(titg->Ev) << " in g while " << rSupCandGen.EStr(tith->Ev)
		 << " in h");
        // if uncontrollable transition violates specification
        // delete state from rResGen and put into criticalset
        if (!rCAlph.Exists(titg->Ev)) {
	  FD_DF("SupCon: asynchronous event " << rPlantGen.EStr(titg->Ev) 
		     << " in g is uncontrollable");
	  FD_DF("SupCon: TraverseUncontrollableBackwards(" << rSupCandGen.SStr(currenth) << ")");
	  TraverseUncontrollableBackwards(rCAlph, rtransrel, critical, currenth);
	  // exit all loops over g transrel
	  titg = titg_end;
	  break;
        }
	FD_DF("SupCon: incrementing g transrel");
	++titg;
      }
      // increment tith and titg case C: process h event that is not enabled in g
      else {
       // #ifdef FAUDES_CHECKED
       // FD_WARN("SupCon: transition " << rSupCandGen.TStr(*tith) 
       //  << " in specification h not found in g");
       // #endif
       FD_DF("SupCon: incrementing h transrel");
       ++tith;
      }
    } // end while tith and titg
    // increment tith and titg case D: process leftover g events 
    while (titg != titg_end) {
      FD_DF("SupCon: asynchronous execution of event " 
	        << rPlantGen.EStr(titg->Ev) << " in g at end of h");
      FD_DF("SupCon: actual g-transition: " << rPlantGen.SStr(titg->X1) 
		<< "-" << rPlantGen.EStr(titg->Ev) << "-" << rPlantGen.SStr(titg->X2));
      FD_DF("SupCon: actual h-transition: end");
      // if uncontrollable transition violates specification
      if (!rCAlph.Exists(titg->Ev)) {
	FD_DF("SupCon: asynchronous execution of uncontrollable event "
		  << rPlantGen.EStr(titg->Ev) << " in g");
	FD_DF("SupCon: TraverseUncontrollableBackwards(" << rPlantGen.SStr(currenth) << ")");
        TraverseUncontrollableBackwards(rCAlph, rtransrel, critical, currenth);
	// exit this loop
	break;
      }
      FD_DF("SupCon: incrementing g transrel");
      ++titg;
    }
    //#ifdef FAUDES_CHECKED
    // increment tith and titg case E: process leftover h events 
    //while (tith != tith_end) {
    //  FD_WARN("SupCon: transition " << rSupCandGen.TStr(*tith) << " in specification h not found in g");
    //  FD_DF("SupCon: incrementing h transrel");
    //  ++tith;
    //}
    //#endif
    processed.Insert(currenth);
  } //end while todog

  // remove all states that have been identified as critical or that have
  // been not processed
  critical = rSupCandGen.States() - ( processed - critical );
  rSupCandGen.DelStates(critical);
}



/*
Revision for libFAUDES 2.20b:

The re-implemented version will only report states as critical 
if they disable an uncontrollable plant event. No reverse
search is performed.
*/


// IsControllableUnchecked(rPlantGen, rCAlph, rSupCandGen, rCriticalStates)
bool IsControllableUnchecked(
  const Generator& rPlantGen,
  const EventSet& rCAlph,  
  const Generator& rSupCandGen, 
  StateSet& rCriticalStates) 
{
  FD_DF("IsControllableUnchecked(" << &rSupCandGen << "," << &rPlantGen << ")");

  // todo stack
  std::stack< std::pair<Idx,Idx> > todo;
  // set of already processed states
  std::set< std::pair<Idx,Idx> > processed;

  // PREPARE RESULT:
  rCriticalStates.Clear();
  rCriticalStates.Name("CriticalStates");

  // return true (controllable) if there is no initial state
  if(rPlantGen.InitStatesEmpty() || rSupCandGen.InitStatesEmpty()) 
    return true;

  // push combined initial state on todo stack
  todo.push( std::make_pair(*rPlantGen.InitStatesBegin(),*rSupCandGen.InitStatesBegin()) );
  FD_DF("IsControllable: todo push: (" << rPlantGen.SStr(*rPlantGen.InitStatesBegin()) << "|"
     << rSupCandGen.SStr(*rSupCandGen.InitStatesBegin()) << ")");

  // process todo stack
  while (! todo.empty()) {
    // allow for user interrupt
    // LoopCallback();
    // allow for user interrupt, incl progress report
    FD_WPC(1,2,"Controllability(): iterating states"); 
    // get top element from todo stack
    std::pair<Idx,Idx> current = todo.top(); todo.pop();
    processed.insert(current);
    Idx currentg = current.first;
    Idx currenth = current.second;
    FD_DF("IsControllable: todo pop: (" << rPlantGen.SStr(currentg) << "|" 
       << rSupCandGen.SStr(currenth) << ")");

#ifdef FAUDES_DEBUG_FUNCTION
    TransSet::Iterator _titg, _tith;
    // print all transitions of current states
    FD_DF("IsControllable: transitions from current states:");
    for (_titg = rPlantGen.TransRelBegin(currentg); _titg != rPlantGen.TransRelEnd(currentg); ++_titg) 
       FD_DF("IsControllable: g: " << rPlantGen.SStr(_titg->X1) << "-" 
		 << rPlantGen.EStr(_titg->Ev) << "-" << rPlantGen.SStr(_titg->X2));
    for (_tith = rSupCandGen.TransRelBegin(currenth); _tith != rSupCandGen.TransRelEnd(currenth); ++_tith) 
       FD_DF("IsControllable: h: " << rSupCandGen.SStr(_tith->X1) << "-" 
		 << rSupCandGen.EStr(_tith->Ev) << "-" << rSupCandGen.SStr(_tith->X2));
#endif 

    // process all h transitions while there could be matching g transitions
    TransSet::Iterator titg = rPlantGen.TransRelBegin(currentg);
    TransSet::Iterator titg_end = rPlantGen.TransRelEnd(currentg);
    TransSet::Iterator tith = rSupCandGen.TransRelBegin(currenth);
    TransSet::Iterator tith_end = rSupCandGen.TransRelEnd(currenth);
    while ((tith != tith_end) && (titg != titg_end)) {
      FD_DF("IsControllable: processing g-transition: " << rPlantGen.SStr(titg->X1) 
		  << "-" << rPlantGen.EStr(titg->Ev) << "-" << rPlantGen.SStr(titg->X2));
      FD_DF("IsControllable: processing h-transition: " << rSupCandGen.SStr(tith->X1) 
	          << "-" << rSupCandGen.EStr(tith->Ev) << "-" << rSupCandGen.SStr(tith->X2));
      // increment tith and titg case A: process common events
      if (titg->Ev == tith->Ev) {
	FD_DF("IsControllable: processing common event " << rPlantGen.EStr(titg->Ev));
        // form next state and add to stack
        std::pair<Idx,Idx> next = std::make_pair(titg->X2,tith->X2);       
	// add to todo list if not processed so far 
     	if(processed.find(next)==processed.end()) {
 	  FD_DF("IsControllable: todo push: (" << rPlantGen.SStr(titg->X2) << "|"
	    << rSupCandGen.SStr(tith->X2) << ")");
          todo.push(next);
	}
        // proceed
	++titg;
	++tith;
      }
      // increment tith and titg case B: process g event that is not enabled in h
      else if (titg->Ev < tith->Ev) {
	FD_DF("IsControllable: asynchronous execution of event " 
	         << rPlantGen.EStr(titg->Ev) << " in g while " << rSupCandGen.EStr(tith->Ev)
		 << " in h");
        // if uncontrollable transition violates specification
        // record that state as critical
        if (!rCAlph.Exists(titg->Ev)) {
	  FD_DF("IsControllable: asynchronous event " << rPlantGen.EStr(titg->Ev) 
		     << " in g is uncontrollable");
	  FD_DF("IsControllable: TraverseUncontrollableBackwards(" << rSupCandGen.SStr(currenth) << ")");
	  rCriticalStates.Insert(currenth);
	  // exit all loops over g transrel
	  titg = titg_end;
	  break;
        }
	FD_DF("IsControllable: incrementing g transrel");
	++titg;
      }
      // increment tith and titg case C: process h event that is not enabled in g
      else {
       // #ifdef FAUDES_CHECKED
       // FD_WARN("IsControllable: transition " << rSupCandGen.TStr(*tith) 
       //  << " in specification h not found in g");
       // #endif
       FD_DF("IsControllable: incrementing h transrel");
       ++tith;
      }
    } // end while tith and titg
    // increment tith and titg case D: process leftover g events 
    while (titg != titg_end) {
      FD_DF("IsControllable: asynchronous execution of event " 
	        << rPlantGen.EStr(titg->Ev) << " in g at end of h");
      FD_DF("IsControllable: actual g-transition: " << rPlantGen.SStr(titg->X1) 
		<< "-" << rPlantGen.EStr(titg->Ev) << "-" << rPlantGen.SStr(titg->X2));
      FD_DF("IsControllable: actual h-transition: end");
      // if uncontrollable transition violates specification
      if (!rCAlph.Exists(titg->Ev)) {
	FD_DF("IsControllable: asynchronous execution of uncontrollable event "
		  << rPlantGen.EStr(titg->Ev) << " in g");
	FD_DF("IsControllable: TraverseUncontrollableBackwards(" << rPlantGen.SStr(currenth) << ")");
	rCriticalStates.Insert(currenth);
	// exit this loop
	break;
      }
      FD_DF("IsControllable: incrementing g transrel");
      ++titg;
    }
    //#ifdef FAUDES_CHECKED
    // increment tith and titg case E: process leftover h events 
    //while (tith != tith_end) {
    //  FD_WARN("IsControllable: transition " << rSupCandGen.TStr(*tith) << " in specification h not found in g");
    //  FD_DF("IsControllable: incrementing h transrel");
    //  ++tith;
    //}
    //#endif
  } //end while todo


  // return identified critical states
  return rCriticalStates.Empty();
}



// SupConProduct(rPlantGen, rCAlph, rSpecGen, rCompositionMap, rResGen)
void SupConProduct(
  const Generator& rPlantGen, 
  const EventSet& rCAlph,
  const Generator& rSpecGen,
  std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap, 
  Generator& rResGen) 
{
  FD_DF("SupConProduct(" << &rPlantGen << "," << &rSpecGen << ")");
	
  // HELPERS:

  // todo stack
  std::stack< std::pair<Idx,Idx> > todo;
  // set of critical states
  StateSet critical;
  // current state, next state, as pair
  std::pair<Idx,Idx> currentp, nextp;
  // current state, next state, at result
  Idx currentt, nextt;
  std::map< std::pair<Idx,Idx>, Idx>::iterator rcmapit;
  StateSet::Iterator lit1, lit2;
  TransSet::Iterator titg, titg_end, tith, tith_end;

  // prepare
  rResGen.ClearStates();
  rCompositionMap.clear();

  // ALGORITHM:  
  if (rPlantGen.InitStatesEmpty()) {
    FD_DF("SupConProduct: plant got no initial states. "
            << "parallel composition contains empty language.");
    return;
  }
  if (rSpecGen.InitStatesEmpty()) {
    FD_DF("SupConProduct: spec got no initial states. "
              << "parallel composition contains empty language.");
    return;
  }

  // create initial state
  currentp = std::make_pair(*rPlantGen.InitStatesBegin(), *rSpecGen.InitStatesBegin());
  todo.push(currentp);
  rCompositionMap[currentp] = rResGen.InsInitState();
  FD_DF("SupConProduct: insert initial state: (" 
     << rPlantGen.SStr(currentp.first) 
     << "|" << rSpecGen.SStr(currentp.second) << ") as idx " 
     << rCompositionMap[currentp]);
  if(rPlantGen.ExistsMarkedState(*rPlantGen.InitStatesBegin()) 
                && rSpecGen.ExistsMarkedState(*rSpecGen.InitStatesBegin())) {
    rResGen.SetMarkedState(rCompositionMap[currentp]);
     FD_DF("SupConProduct: initial state is marked");
  }

  // do parallel composition of reachable states while avoiding critical states
  // this creates an accessible generator

  FD_DF("SupConProduct: *** processing reachable states ***");
  while (! todo.empty()) {
    // allow for user interrupt
    // LoopCallback();
    // allow for user interrupt, incl progress report
    FD_WPC(rCompositionMap.size(),rCompositionMap.size()+todo.size(),"SupConProduct(): processing"); 
    // get next reachable pair of states from todo stack
    currentp = todo.top();
    todo.pop();
    currentt = rCompositionMap[currentp];
    FD_DF("SupConProduct: todo pop: (" 
        << rPlantGen.SStr(currentp.first) 
	<< "|" << rSpecGen.SStr(currentp.second) << ") as idx " 
        << currentt);
    // might have been indicated to be critical
    if(critical.Exists(currentt)) continue;    
    // set up transition relation iterators	
    titg = rPlantGen.TransRelBegin(currentp.first);
    titg_end = rPlantGen.TransRelEnd(currentp.first);
    tith = rSpecGen.TransRelBegin(currentp.second);
    tith_end = rSpecGen.TransRelEnd(currentp.second);

#ifdef FAUDES_DEBUG_FUNCTION
    // print all transitions of current states
    FD_DF("SupConParallel: transitions from current state:");
    for (;titg != titg_end; ++titg) 
      FD_DF("SupConParallel: g: " << rPlantGen.TStr(*titg));
    for (;tith != tith_end; ++tith) {
      FD_DF("SupConParallel: h: " << rSpecGen.TStr(*tith));
    }
    titg = rPlantGen.TransRelBegin(currentp.first);
    tith = rSpecGen.TransRelBegin(currentp.second);
#endif                                                                      

    // process all h transitions while there could be matching g transitions
    while((tith != tith_end) && (titg != titg_end)) {
      FD_DF("SupConProduct: current g-transition: " << rPlantGen.TStr(*titg) );
      FD_DF("SupConProduct: current h-transition: " << rSpecGen.TStr(*tith));
 
      // case A: execute common events
      if(titg->Ev == tith->Ev) {
	FD_DF("SupConProduct: executing common event "  << rPlantGen.EStr(titg->Ev));
	nextp = std::make_pair(titg->X2, tith->X2);
	rcmapit = rCompositionMap.find(nextp);
	// if state is new ...
	if(rcmapit == rCompositionMap.end()) {
          // ... add on todo stack
	  todo.push(nextp);
          // ... insert in result
    	  nextt = rResGen.InsState();
	  rCompositionMap[nextp] = nextt;
	  FD_DF("SupConProduct: insert state: (" << 
             rPlantGen.SStr(nextp.first) << "|" 
	     << rSpecGen.SStr(nextp.second) << ") as idx " 
             << nextt);
	  // .. mark on the fly
	  if(rPlantGen.ExistsMarkedState(nextp.first) && 
		rSpecGen.ExistsMarkedState(nextp.second)) {
	    rResGen.SetMarkedState(nextt);
	    FD_DF("SupConProduct: marked nes state")
	  }
	}
	// if state already exists
	else {
	  nextt = rcmapit->second;
	}
	// if successor state is not critical add transition and proceed
	if(!critical.Exists(nextt)) {
	  FD_DF("SupConProduct: set transition " << 
		rResGen.TStr(Transition(currentt,titg->Ev,nextt)));
	  rResGen.SetTransition(currentt, titg->Ev, nextt);
	  FD_DF("SupConProduct: incrementing g and h transrel");
	  ++titg;
	  ++tith;
	}
	// if successor state is critical and event is uncontrollable then this state becomes critical, too
	else if (!rCAlph.Exists(titg->Ev)) {
	  FD_DF("SupConProduct: successor in critical with uncntr. shared event " << rSpecGen.EStr(titg->Ev)) 
	  FD_DF("SupConProduct: critical insert, exit loop");
	  critical.Insert(currentt);
	  // exit all loops
	  titg = titg_end;
	  tith = tith_end;
        }
        // else if successor state is critical and event controllable we may proceed
        else {
	  FD_DF("SupConProduct: incrementing g and h transrel");
          ++titg;
          ++tith;
        }
      } // end: shared event (case A) 

      // case B: process g event that is not enabled for h
      else if (titg->Ev < tith->Ev) {
	FD_DF("SupConProduct: event only enabled in g: " << rPlantGen.EStr(titg->Ev));
	// when uncontrollable, current state is critical
	if(!rCAlph.Exists(titg->Ev)) {
	  FD_DF("SupConProduct: asynchronous event is uncontrollable, critical insert, exit loop");
	  critical.Insert(currentt);
	  // exit all loops
	  titg = titg_end;
	  tith = tith_end;
	  break;
	}
	FD_DF("SupConProduct: incrementing g transrel");
	++titg;
      } 

      // case C: process h event that is not enabled for g
      else {
	FD_DF("SupConProduct: incrementing h transrel");
	++tith;
      }
    } // end while incrementing both tith and titg 

    // case D: process leftover events of g
    while (titg != titg_end) {
      FD_DF("SupConProduct: event only enabled in g: " << rPlantGen.EStr(titg->Ev));
      // if uncontrollable transition leaves specification
      if (!rCAlph.Exists(titg->Ev)) {
	FD_DF("SupConProduct: asynchronous event is uncontrollable, critical insert, exit loop");
	critical.Insert(currentt);
	// exit all loops
	break;
      }
      FD_DF("SupConProduct: incrementing g transrel");
      ++titg;
    }
    //#ifdef FAUDES_CHECKED
    // increment titg and titg, case E: process leftover events of h
    //while (txoith != tith_end) {
    //  FD_WARN("SupConProduct:  transition " << rSpecGen.TStr(*tith)  << " in specification h not found in g");
    //  FD_DF("SupConProduct: incrementing h transrel");
    //  ++tith;
    //}
    //#endif
  } // while todo
  FD_DF("SupConProduct: deleting critical states...");
  rResGen.DelStates(critical);
}



// SupConUnchecked(rPlantGen, rCAlph, rSpecGen, rCompositionMap, rResGen)
void SupConUnchecked(
  const Generator& rPlantGen,
  const EventSet& rCAlph,  
  const Generator& rSpecGen,
  std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap, 
  Generator& rResGen) 
{
  FD_DF("SupCon(" << &rPlantGen << "," << &rSpecGen << ")");

  // PREPARE RESULT:	
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  pResGen->Clear();
  pResGen->Name(CollapsString("SupCon(("+rPlantGen.Name()+"),("+rSpecGen.Name()+"))"));
  pResGen->InjectAlphabet(rPlantGen.Alphabet());

  // controllable events
  FD_DF("SupCon: controllable events: "   << rCAlph.ToString());

  // ALGORITHM:
  SupConProduct(rPlantGen, rCAlph, rSpecGen, rCompositionMap, *pResGen);

  // make resulting generator trim until it's fully controllable
  while(true) {
    if(pResGen->Empty()) break;
    Idx state_num = pResGen->Size();
    SupConClosedUnchecked(rPlantGen, rCAlph, *pResGen);
    pResGen->Trim();
    if(pResGen->Size() == state_num) break;
  }

  // convenience state names
  if(rPlantGen.StateNamesEnabled() && rSpecGen.StateNamesEnabled() && rResGen.StateNamesEnabled()) 
    SetComposedStateNames(rPlantGen, rSpecGen, rCompositionMap, *pResGen);
  else
    pResGen->StateNamesEnabled(false);

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

}

// ControlProblemConsistencyCheck(rPlantGen, rCAlph, rSpecGen)
void ControlProblemConsistencyCheck(
  const Generator& rPlantGen,
  const EventSet& rCAlph, 
  const Generator& rSpecGen) 
{

  // alphabets must match
  if ( rPlantGen.Alphabet() != rSpecGen.Alphabet()) {
    EventSet only_in_plant = rPlantGen.Alphabet() - rSpecGen.Alphabet();
    EventSet only_in_spec = rSpecGen.Alphabet() - rPlantGen.Alphabet();
    only_in_plant.Name("Only_In_Plant");
    only_in_spec.Name("Only_In_Specification");
    std::stringstream errstr;
    errstr << "Alphabets of generators do not match.";
    if(!only_in_plant.Empty())
      errstr << " " << only_in_plant.ToString() << ".";
    if(!only_in_spec.Empty())
      errstr << " " << only_in_spec.ToString() << ".";
    throw Exception("SupCon/SupCon", errstr.str(), 100);
  }
	
  // controllable events have to be subset of Sigma
  if(!(rCAlph<=rPlantGen.Alphabet())) {
    EventSet only_in_CAlph = rCAlph - rPlantGen.Alphabet();
    std::stringstream errstr;
    errstr << "Not all controllable events are contained in Sigma: " 
       << only_in_CAlph.ToString() << ".";
    throw Exception("ControllProblemConsistencyCheck", errstr.str(), 100);
  } 

  // TODO: check symboltables to match

  // plant and spec must be deterministic
  bool plant_det = rPlantGen.IsDeterministic();
  bool spec_det = rSpecGen.IsDeterministic();

  if ((plant_det == false) && (spec_det == true)) {
    std::stringstream errstr;
    errstr << "Plant generator must be deterministic, "
      << "but is nondeterministic";
    throw Exception("ControllableConsistencyCheck", errstr.str(), 201);
  }
  else if ((plant_det == true) && (spec_det == false)) {
    std::stringstream errstr;
    errstr << "Spec generator must be deterministic, "
       << "but is nondeterministic";
    throw Exception("ControllableConsistencyCheck", errstr.str(), 203);
  }
  else if ((plant_det == false) && (spec_det == false)) {
    std::stringstream errstr;
    errstr << "Plant and spec generator must be deterministic, "
       << "but both are nondeterministic";
    throw Exception("ControllableConsistencyCheck", errstr.str(), 204);
  }
}

// ControlProblemConsistencyCheck(rPlantGen, rCAlph, rSpecGen)
void ControlProblemConsistencyCheck(
  const Generator& rPlantGen,
  const EventSet& rCAlph, 
  const EventSet& rOAlph, 
  const Generator& rSpecGen) 
{
  // std test
  ControlProblemConsistencyCheck(rPlantGen,rCAlph,rSpecGen);
  // observable events have to be subset of Sigma
  if(!(rOAlph<=rPlantGen.Alphabet())) {
    EventSet only_in_OAlph = rOAlph - rPlantGen.Alphabet();
    std::stringstream errstr;
    errstr << "Not all observable events are contained in Sigma: " 
       << only_in_OAlph.ToString() << ".";
    throw Exception("ControllProblemConsistencyCheck", errstr.str(), 100);
  } 
}


/*
****************************************
* SUPCON: WRAPPER / USER FUNCTIONS     *
****************************************
*/


// IsControllable(rPlantGen, rCAlph, rSupCandGen)
bool IsControllable(
  const Generator& rPlantGen, 
  const EventSet& rCAlph, 
  const Generator& rSupCandGen) 
{
  FD_DF("IsControllable(" << &rSupCandGen << "," << &rPlantGen << ")");

  // HELPERS:
  FD_DF("IsControllable: controllable events: " << rCAlph.ToString());

  // critical set
  StateSet critical;

  // CONSISTENCY CHECKS:
  ControlProblemConsistencyCheck(rPlantGen, rCAlph, rSupCandGen);

  // ALGORITHM:
  return IsControllableUnchecked(rPlantGen, rCAlph, rSupCandGen, critical);
}


// IsControllable(rPlantGen, rCAlph, rSupCandGen, critical)
bool IsControllable(
  const Generator& rPlantGen, 
  const EventSet& rCAlph, 
  const Generator& rSupCandGen, 
  StateSet& rCriticalStates) 
{
  FD_DF("IsControllable(" << &rSupCandGen << "," << &rPlantGen << ")");

  // CONSISTENCY CHECKS:
  ControlProblemConsistencyCheck(rPlantGen, rCAlph, rSupCandGen);

  // ALGORITHM:
  return IsControllableUnchecked(rPlantGen, rCAlph, rSupCandGen, rCriticalStates);
}


// SupCon(rPlantGen, rCAlph, rSpecGen, rResGen)
void SupCon(
  const Generator& rPlantGen, 
  const EventSet& rCAlph, 
  const Generator& rSpecGen, 
  Generator& rResGen) 
{

  // CONSISTENCY CHECK:
  ControlProblemConsistencyCheck(rPlantGen, rCAlph, rSpecGen);

  // HELPERS:
  std::map< std::pair<Idx,Idx>, Idx> rcmap;

  // ALGORITHM:
  SupConUnchecked(rPlantGen, rCAlph, rSpecGen, rcmap, rResGen);
}




// SupConClosed(rPlantGen, rCAlph, rSpecGen, rResGen)
void SupConClosed(
  const Generator& rPlantGen, 
  const EventSet& rCAlph,
  const Generator& rSpecGen, 
  Generator& rResGen) 
{
  // HELPERS:
  std::map< std::pair<Idx,Idx>, Idx> rcmap;

  // ALGORITHM:
  SupConClosed(rPlantGen, rCAlph, rSpecGen, rcmap, rResGen);
}


// SupConClosed(rPlantGen, rCAlph, rSpecGen, rCompositionMap, rResGen)
void SupConClosed(
  const Generator& rPlantGen, 
  const EventSet& rCAlph,
  const Generator& rSpecGen,
  std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap, 
  Generator& rResGen) 
{
  FD_DF("SupConClosed(" << &rPlantGen << "," << &rSpecGen << ")");

  // PREPARE RESULT:
	
  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  pResGen->Clear();
  pResGen->Name(CollapsString("SupConClosed(("+rPlantGen.Name()+"),("+rSpecGen.Name()+"))"));
  pResGen->InjectAlphabet(rPlantGen.Alphabet());
	
  // HELPERS:
  FD_DF("SupCon: controllable events: " << rCAlph.ToString());

  // CONSISTENCY CHECK:
  ControlProblemConsistencyCheck(rPlantGen, rCAlph, rSpecGen);

  // ALGORITHM:

  // product composition
  SupConProduct(rPlantGen, rCAlph, rSpecGen, rCompositionMap, *pResGen);
  // make resulting generator controllable
  SupConClosedUnchecked(rPlantGen, rCAlph, *pResGen);

  // restrict composition map
  std::map< std::pair<Idx,Idx>, Idx>::iterator rcmapit = rCompositionMap.begin();
  while(rcmapit != rCompositionMap.end()) 
    if(!rResGen.ExistsState(rcmapit->second)) rCompositionMap.erase(rcmapit++);
    else rcmapit++;

  // convenience state names
  if (rPlantGen.StateNamesEnabled() && rSpecGen.StateNamesEnabled() && rResGen.StateNamesEnabled()) 
    SetComposedStateNames(rPlantGen, rSpecGen, rCompositionMap, *pResGen);
  else
    pResGen->StateNamesEnabled(false);

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}


// TraverseUncontrollableBackwards(rCAlph, rtransrel, critical, current)
void TraverseUncontrollableBackwards(
  const EventSet& rCAlph, 
  TransSetX2EvX1& rtransrel, 
  StateSet& rCriticalStates, 
  Idx current) {
  FD_DF("TraverseUncontrollableBackwards: " << rCriticalStates.Str(current));

  // HELPERS:
  std::stack<Idx> todo;
  TransSetX2EvX1::Iterator rtit = rtransrel.BeginByX2(current);
  TransSetX2EvX1::Iterator rtit_end = rtransrel.EndByX2(current);

  // ALGORITHM:
  rCriticalStates.Insert(current);
  todo.push(current);
  FD_DF("TraverseUncontrollableBackwards: current rCriticalStates set: " 
     << rCriticalStates.ToString());
  // process todo stack
  while(! todo.empty()) {
    // get top state from todo stack
    current = todo.top(); todo.pop();
    // iteration over all transitions with x2 == current
    rtit_end = rtransrel.EndByX2(current);
    rtit = rtransrel.BeginByX2(current);
    for(; rtit != rtit_end; ++rtit) {
      // if uncontrollable event and predecessor state not already critical
      if((!rCAlph.Exists(rtit->Ev)) && (! rCriticalStates.Exists(rtit->X1))) {
        FD_DF("TraverseUncontrollableBackwards: todo push " << rCriticalStates.Str(rtit->X1));
	todo.push(rtit->X1);
	FD_DF("TraverseUncontrollableBackwards: critical insert: " << rCriticalStates.Str(rtit->X1));
	rCriticalStates.Insert(rtit->X1);
      }
    }
  } // end while todo
}


// controllability test for System plant
bool IsControllable(
  const System& rPlantGen, 
  const Generator& rSupCandGen)
{
  return IsControllable(rPlantGen, rPlantGen.ControllableEvents(), rSupCandGen);
}


// supcon for Systems:
// uses and maintains controllablity from plant 
void SupCon(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) {

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }

  // execute 
  SupCon(rPlantGen, rPlantGen.ControllableEvents(),rSpecGen,*pResGen);

  // copy all attributes of input alphabet
  pResGen->EventAttributes(rPlantGen.Alphabet());

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

}

// supcon for Systems
// uses and maintains controllablity from plant 
void SupConClosed(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) {

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }

  // execute
  SupConClosed(rPlantGen, rPlantGen.ControllableEvents(),rSpecGen,*pResGen);

  // copy all attributes of input alphabet
  pResGen->EventAttributes(rPlantGen.Alphabet());

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

}

// legacy interface (pre v2.33d, will be discontinued) 
void SupConNB(
  const Generator& rPlantGen, 
  const EventSet&  rCAlph,
  const Generator& rSpecGen, 
  Generator& rResGen) {
  SupCon(rPlantGen,rCAlph,rSpecGen,rResGen);
}
void SupConNB(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) {
  SupCon(rPlantGen,rSpecGen,rResGen);
}


} // name space 
