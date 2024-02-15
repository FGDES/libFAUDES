/** @file  cfl_localgen.cpp Helper functions for projected generators */

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

#include "cfl_localgen.h"

namespace faudes {

// LowExitStates(rHighAlph, rEntryStatesMap, rLowRevTransRel, highState)
StateSet LowExitStates(const Generator& rLowGen, const EventSet& rHighAlph, 
		const std::map<Idx,StateSet>& rEntryStatesMap, const TransSetX2EvX1& rLowRevTransRel, 
		Idx highState) {
	// prepare result:
        StateSet lo_exitstates;
	// algorithm:
	LowExitStates(rHighAlph, rEntryStatesMap, rLowRevTransRel, highState, lo_exitstates);
	// return result:
	return lo_exitstates;
}


// LowExitStates(rHighAlph, rEntryStatesMap, rLowRevTransRel, highState, rLowExitStates)
void LowExitStates(const EventSet& rHighAlph, const std::map<Idx,StateSet>& rEntryStatesMap, 
		const TransSetX2EvX1& rLowRevTransRel, Idx highState, StateSet& rLowExitStates) {
	FD_DF("LowExitStates: computing low level exit states for high level"
			<< " state " << rLowExitStates.Str(highState));
	// helpers:
	std::map<Idx,StateSet>::const_iterator esmap_it;
	StateSet::Iterator lit;
	TransSetX2EvX1::Iterator rtit;
	TransSetX2EvX1::Iterator rtit_end;
	// algorithm:
	esmap_it = rEntryStatesMap.find(highState);
#ifdef FAUDES_CHECKED
	if (esmap_it == rEntryStatesMap.end()) {
		std::stringstream errstr;
		errstr << "Hi level state " << rHighAlph.Str(highState)
				<< " not found in entry states map";
		throw Exception("LowExitStates", errstr.str(), 502);
	}
#endif 
	// find predecessor states of low_level entry states
	for (lit = esmap_it->second.Begin(); lit != esmap_it->second.End(); ++lit) {
		FD_DF("LowExitStates: current low level entry state "
				<< rLowExitStates.Str(*lit));
		rtit = rLowRevTransRel.BeginByX2(*lit);
		rtit_end = rLowRevTransRel.EndByX2(*lit);
		for (; rtit != rtit_end; ++rtit) {
			if (rHighAlph.Exists(rtit->Ev)) {
				FD_DF("LowExitStates: found low level exit state "
						 << rLowExitStates.Str(rtit->X1));
				rLowExitStates.Insert(rtit->X1);
			}
		}
	}
}

// ReachableEvents(lo, rHighAlph, lowState)
EventSet ReachableEvents(const Generator& rLowGen, const EventSet& rHighAlph, 
		Idx lowState) {
	// prepare result:
	EventSet reachable_events = rLowGen.NewEventSet();
	// algorithm:
	ReachableEvents(rLowGen, rHighAlph, lowState, reachable_events);
	// return result:
	return reachable_events;
}


// ReachableEvents(lo, rHighAlph, lowState, rReachableEvents)
void ReachableEvents(const Generator& rLowGen, const EventSet& rHighAlph, 
		Idx lowState, EventSet& rReachableEvents) {
	// helpers:
	// iterators
	TransSet::Iterator tit;
	TransSet::Iterator tit_end;
	// todo list
	std::stack<Idx> todo;
	// done set
	StateSet done;

	// prepare result:
	rReachableEvents.Clear();

	// algorithm:
	todo.push(lowState);
	done.Insert(lowState);
	while (! todo.empty()) {
		const Idx current = todo.top();
		todo.pop();
		tit = rLowGen.TransRelBegin(current);
		tit_end = rLowGen.TransRelEnd(current);
		for (; tit != tit_end; ++tit) {
			if (rHighAlph.Exists(tit->Ev)) {
				rReachableEvents.Insert(tit->Ev);
				// if high level event 
				if (rReachableEvents.Size() == rHighAlph.Size()) {
					return;
				}
			}
			// if low level event and not already in done set
			else if (! done.Exists(tit->X2)) {
				todo.push(tit->X2);
				done.Insert(tit->X2);
			}
		}
	}
}


// LocalCoaccessibleReach(rRevTransRel, rHighAlph, lowState, rCoaccessibleReach)
void LocalCoaccessibleReach(const TransSetX2EvX1& rRevTransRel, 
		const EventSet& rHighAlph, Idx lowState, StateSet& rCoaccessibleReach) {
	// helpers:
	// iterators
	TransSetX2EvX1::Iterator tit;
	TransSetX2EvX1::Iterator tit_end;
	// todo list
	std::stack<Idx> todo;

	// algorithm:
	todo.push(lowState);
	rCoaccessibleReach.Insert(lowState);
	while (! todo.empty()) {
		const Idx current = todo.top();
		todo.pop();
		tit = rRevTransRel.BeginByX2(current);
		tit_end = rRevTransRel.EndByX2(current);
		for (; tit != tit_end; ++tit) {
			// if high level event skip transition
			if (rHighAlph.Exists(tit->Ev)) {
				continue;
			}
			// if predecessor notin rCoaccessibleReach set
			else if (! rCoaccessibleReach.Exists(tit->X1)) {
				todo.push(tit->X1);
				rCoaccessibleReach.Insert(tit->X1);
			}
		}
	}
}

// LocalAccessibleReach(rLowGen, rHighAlph, lowState, rAccessibleReach
void LocalAccessibleReach(const Generator& rLowGen, const EventSet& rHighAlph, 
		Idx lowState, StateSet& rAccessibleReach) {
	// helpers:
	// iterators
	TransSet::Iterator tit;
	TransSet::Iterator tit_end;
	// todo list
	std::stack<Idx> todo;

	// algorithm:
	todo.push(lowState);
	rAccessibleReach.Insert(lowState);
	while (! todo.empty()) {
		const Idx current = todo.top();
		todo.pop();
		tit = rLowGen.TransRelBegin(current);
		tit_end = rLowGen.TransRelEnd(current);
		for (; tit != tit_end; ++tit) {
			// if high level event skip transition
			if (rHighAlph.Exists(tit->Ev)) {
				continue;
			}
			// if successor not in rAccessibleReach set
			if (! rAccessibleReach.Exists(tit->X2)) {
				todo.push(tit->X2);
				rAccessibleReach.Insert(tit->X2);
			}
		}
	}
}

} // namespace faudes
