/** @file op_observercomputation.cpp

Methods to compute natural projections that exhibit the obsrver property.
The observer algorithm is elaborated in
K. C. Wong and W. M. Wonham, "On the Computation of Observers in Discrete Event
Systems," Discrete Event Dynamic Systems, vol. 14, no. 1, pp. 55-107, 2004. 
In addition, methods to compute natural projections that exhibit
output control consistency (OCC) and local control consistency (LCC) are provided. See for example
K. Schmidt and C. Breindl, "On Maximal Permissiveness of Hierarchical and Modular Supervisory
Control Approaches for Discrete Event Systems," Workshop on Discrete Event Systems, 2008. 
Furthermore, an algorithm for computing natural observers without changing event labels as 
presented in 
Lei Feng; Wonham, W.M., "On the Computation of Natural Observers in Discrete-Event Systems," 
Decision and Control, 2006 45th IEEE Conference on , vol., no., pp.428-433, 13-15 Dec. 2006
is implemented.
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

#include "op_observercomputation.h"
#include "cfl_localgen.h"

using namespace std;
  
namespace faudes{


// calculateDynamicSystemClosedObs(rGen, rHighAlph, rGenDyn)
void calculateDynamicSystemClosedObs(const Generator& rGen, EventSet& rHighAlph, Generator& rGenDyn)
{
	OP_DF("calculateDynamicSystemClosedObs(" << rGen.Name() << "," << rHighAlph.Name() << "," << rGenDyn.Name() << ")");
	// transition relation sorted in reverse order for backwards reachability
	TransSetX2EvX1 tset_X2EvX1;
	rGen.TransRel(tset_X2EvX1);
	
	// prepare generator rGenDyn
	rGenDyn.ClearTransRel();
	rGenDyn.InjectAlphabet(rHighAlph); // all high-level events are contained in the alphabet of rGenDyn
	rGenDyn.InjectStates(rGen.States() ); // the dynamic system has all states of rGen
	
	// helpers
	EventSet::Iterator eIt = rGenDyn.AlphabetBegin();
	EventSet::Iterator eItEnd = rGenDyn.AlphabetEnd();
	
	TransSetX2EvX1::Iterator tItByX2Ev;
	TransSetX2EvX1::Iterator tItByX2EvEnd;
	StateSet reach;
	// map from exit states (Idx) to high-level event (Idx) and reachable entry state (Idx)
	map<Idx,vector<pair<Idx,Idx> > > exitStateToEventState;
	StateSet::Iterator sIt;
	StateSet::Iterator sItEnd;
	TransSet::Iterator tIt;
	TransSet::Iterator tItEnd;
	map<Idx,StateSet> entryStateToLocalReach;
	map<Idx,vector<pair<Idx,Idx> > >::iterator esIt;
	map<Idx,vector<pair<Idx,Idx> > >::iterator esItEnd;	
	// algorithm for computing the dynamic system
	// loop over all states of original generator
	StateSet::Iterator stateSetIt = rGen.StatesBegin();
	StateSet::Iterator stateSetItEnd = rGen.StatesEnd();
	// flag that indicates if the current state is an entry state
	bool isEntryState = false;
	for( ; stateSetIt != stateSetItEnd; ++stateSetIt) {
		OP_DF("calculateDynamicSystemClosedObs: loop over all states; current state: " << rGen.StateName(*stateSetIt) 
		<< " [" << *stateSetIt << "]");
		// check if current state (*stateSetIt) is an entry-state (a backward transition with
		// a high-level event exists. If yes, the locally reachable states (reach) are stored in
		// the entryStateToLocalReach map
		isEntryState = false;
		tItByX2Ev=tset_X2EvX1.BeginByX2(*stateSetIt); 
		tItByX2EvEnd=tset_X2EvX1.EndByX2(*stateSetIt); 
		for(; tItByX2Ev != tItByX2EvEnd; ++tItByX2Ev)
		{
			OP_DF("calculateDynamicSystemClosedObs: checking transition : " << rGen.TStr(*tItByX2Ev));
			if(rHighAlph.Exists(tItByX2Ev->Ev)){
                isEntryState = true;
			    OP_DF("calculateDynamicSystemClosedObs: current state is an entry-state");
			    // insert the exit state and the high-level event with the reachable entry state into the exitStates set
			    esIt = exitStateToEventState.find(tItByX2Ev->X1);
			    if(esIt == exitStateToEventState.end() ){
			        exitStateToEventState[tItByX2Ev->X1] = vector<pair<Idx,Idx> >();
			    }
			    exitStateToEventState[tItByX2Ev->X1].push_back(make_pair(tItByX2Ev->Ev,tItByX2Ev->X2) );
			}
		}
		if(isEntryState == true){
            // compute locally reachable states for current entry state and insert it into entryStateToLocalReach
            reach.Clear();
            LocalAccessibleReach(rGen, rHighAlph, *stateSetIt, reach);
            OP_DF("calculateDynamicSystemClosedObs: states in local reach: \n " << reach.ToString() );
            entryStateToLocalReach[*stateSetIt]=reach;
		}
	}
	// after this loop we have the set of exit states in exitStates and a map from entry states to their 
	// locally reachable state set entryStateToLocalReach
	
	// create the transition structure of the dynamic system
	esIt    = exitStateToEventState.begin();
	esItEnd = exitStateToEventState.end();
	vector<pair <Idx,Idx> >::iterator vIt;
	vector<pair <Idx,Idx> >::iterator vItEnd;
	// To construct the dynamic system, each local state has to be connected to all states in
	// the local accessible reach of entry states that can be reached via a high-level event.
	// We compute the local backward reach of each exit state and find the corresponding entry states in 
	// the map exitStateToEventState
	StateSet exitStateBackwardReach;
	StateSet::Iterator brIt, brItEnd;
	map<Idx,StateSet>::const_iterator enIt;
	for(; esIt != esItEnd; ++esIt)
	{
	    // compute the locally backwards reachable states
	    exitStateBackwardReach.Clear();
	    LocalCoaccessibleReach(tset_X2EvX1, rHighAlph, esIt->first, exitStateBackwardReach);

    	vIt    = esIt->second.begin();
    	vItEnd = esIt->second.end();
    	brItEnd = exitStateBackwardReach.End();
    	// loop over all pairs (Ev,X2) of current exit state and insert transitions from all states in the
    	// backwards reach
    	for( ; vIt != vItEnd; ++vIt)
    	{
    	     brIt = exitStateBackwardReach.Begin();
    	     // go over all backwards reachable states
    	     for( ; brIt != brItEnd; brIt++){
                  // find the set of reachable states from exit state
                  enIt = entryStateToLocalReach.find(vIt->second);
                  sIt = enIt->second.Begin();
                  sItEnd = enIt->second.End();
                  // go over all reachable states from the entry states
                  for( ; sIt != sItEnd; sIt++){
                      rGenDyn.SetTransition(*brIt,vIt->first,*sIt);
                      OP_DF("calculateDynamicSystemClosedObs: Transition added to resulting generator: " << 
        				rGenDyn.TStr(Transition(*brIt,vIt->first,*sIt))); 
                  }
    	     }
        }
	}
	OP_DF("calculateDynamicSystemClosedObs: leaving function");
}

// calculateDynamicSystemObs(rGen, rHighAlph, rGenDyn)
void calculateDynamicSystemObs(const Generator& rGen, EventSet& rHighAlph, Generator& rGenDyn)
{
	OP_DF("calculateDynamicSystemObs(" << rGen.Name() << "," << rHighAlph.Name() << "," << rGenDyn.Name() << ")");	
	// prepare generator rGenDyn
	// locally reachable states
	StateSet reach;
	StateSet::Iterator stIt, stEndIt;
	// label for transitions to marked states in the dynamic system
	std::string eventname = ( rGenDyn.EventSymbolTablep())->UniqueSymbol("obsLabel");
	Idx obsLabel = (rGenDyn.EventSymbolTablep())->InsEntry(eventname);	
	rGenDyn.InsEvent(obsLabel);
	// algorithm for computing the dynamic system
	// loop over all states of original generator
	StateSet::Iterator stateSetIt = rGen.StatesBegin();
	StateSet::Iterator stateSetItEnd = rGen.StatesEnd();
	for( ; stateSetIt != stateSetItEnd; ++stateSetIt) {
		OP_DF("calculateDynamicSystemObs: loop over all states; current state: " << rGen.StateName(*stateSetIt) 
		<< " [" << *stateSetIt << "]");
		// compute locally reachable states for current state
		reach.Clear();
		LocalAccessibleReach(rGen, rHighAlph, *stateSetIt, reach);
		OP_DF("calculateDynamicSystemObs: states in local reach: \n " << reach.ToString() );
		stIt = reach.Begin();
		stEndIt = reach.End();
		for( ; stIt != stEndIt; stIt++){
		    if(rGen.ExistsMarkedState(*stIt) ){
		          rGenDyn.SetTransition(*stateSetIt,obsLabel,*stIt);
		          OP_DF("calculateDynamicSystemObs: Transition added to resulting generator: " << 
        				rGenDyn.TStr(Transition(*stateSetIt,obsLabel,*stIt))); 
		    }
		}
	}
	OP_DF("calculateDynamicSystemObs: leaving function");
}

// calculateDynamicSystemMSA(rGen, rHighAlph, rGenDyn)
void calculateDynamicSystemMSA(const Generator& rGen, EventSet& rHighAlph, Generator& rGenDyn)
{
	OP_DF("calculateDynamicSystemMSA(" << rGen.Name() << "," << rHighAlph.Name() << "," << rGenDyn.Name() << ")");
	// transition relation sorted in reverse order for backwards reachability
	TransSetX2EvX1 tset_X2EvX1;
	rGen.TransRel(tset_X2EvX1);
	
	// prepare generator rGenDyn
	// set of states already investigated
	StateSet doneStates, goodStates;
	TransSet::Iterator tIt;
	TransSet::Iterator tEndIt;
	// label for transitions for msa-accepting states in the dynamic system
	std::string eventname = ( rGenDyn.EventSymbolTablep())->UniqueSymbol("msaLabel");
	Idx msaLabel = (rGenDyn.EventSymbolTablep())->InsEntry(eventname);	
	rGenDyn.InsEvent(msaLabel);
	// algorithm for computing the dynamic system
	// loop over all states of original generator
	StateSet::Iterator stateSetIt = rGen.StatesBegin();
	StateSet::Iterator stateSetItEnd = rGen.StatesEnd();
	// flag that indicates if the current state is an entry state
	for( ; stateSetIt != stateSetItEnd; ++stateSetIt) {
	    // if the state is known to be good (msa holds), it does not have to be considered again
	    if(goodStates.Exists(*stateSetIt) )
	         continue;
	         
		OP_DF("calculateDynamicSystemMSA: loop over all states; current state: " << rGen.StateName(*stateSetIt) 
		<< " [" << *stateSetIt << "]");
		doneStates.Clear();
		bool msaHolds = recursiveCheckMSAForward(rGen, rHighAlph, *stateSetIt, doneStates);
		// if all forward transitions lead to marked states, all states that have been reached are states where msa holds
		if(msaHolds == true){
		    goodStates.InsertSet(doneStates);
		    continue;
		}
		doneStates.Clear();
		msaHolds = recursiveCheckMSABackward(rGen, tset_X2EvX1, rHighAlph, *stateSetIt, doneStates);
		// if all backward transitions lead to marked states, all states that have been reached are states where msa holds
		if(msaHolds == true){
		    goodStates.InsertSet(doneStates);
		    continue;
		}
		// otherwise, msa is violated for the current state. Then, msa-transitions are introduced to all states 
		// reachable via a high-level event
		else{
		    tIt = rGenDyn.TransRelBegin(*stateSetIt);
		    tEndIt = rGenDyn.TransRelEnd(*stateSetIt);
		    for( ; tIt != tEndIt; tIt++){
                rGenDyn.SetTransition(*stateSetIt,msaLabel,tIt->X2);
                OP_DF("calculateDynamicSystemObs: Transition added to resulting generator: " << 
        				rGenDyn.TStr(Transition(*stateSetIt,msaLabel,tIt->X2))); 		        
		    }
		}
	}
	OP_DF("calculateDynamicSystemMSA: leaving function");
}

bool recursiveCheckMSAForward(const Generator& rGen, const EventSet& rHighAlph, Idx currentState, StateSet& rDoneStates){
    // if the current state is marked, no violation of MSA occurs
    rDoneStates.Insert(currentState);
    if(rGen.ExistsMarkedState(currentState) ){
        return true;
    }
    TransSet::Iterator tIt, tEndIt;
    tIt = rGen.TransRelBegin(currentState);
    tEndIt = rGen.TransRelEnd(currentState);
    for( ; tIt != tEndIt; tIt++){
        if(rHighAlph.Exists( tIt->Ev) ){
            return false;            
        }
        else if(rDoneStates.Exists( tIt->X2) )
            continue;
        else{
            bool msaHolds = recursiveCheckMSAForward(rGen, rHighAlph, tIt->X2, rDoneStates);
            if(msaHolds == false)
                return false;
        }
    }
    return true;

}


bool recursiveCheckMSABackward(const Generator& rGen, const TransSetX2EvX1& rRevTransSet, const EventSet& rHighAlph, Idx currentState, StateSet& rDoneStates){
    // if the current state is marked, no violation of MSA occurs
    rDoneStates.Insert(currentState);
    if(rGen.ExistsMarkedState(currentState) ){
        return true;
    }
    TransSetX2EvX1::Iterator tIt, tEndIt;
    tIt = rRevTransSet.BeginByX2(currentState);
    tEndIt = rRevTransSet.EndByX2(currentState);
    for( ; tIt != tEndIt; tIt++){
        if(rHighAlph.Exists( tIt->Ev) ){
            return false;            
        }
        else if(rDoneStates.Exists( tIt->X1) )
            continue;
        else{
            bool msaHolds = recursiveCheckMSABackward(rGen, rRevTransSet, rHighAlph, tIt->X1, rDoneStates);
            if(msaHolds == false)
                return false;
        }
    }
    return true;
}

//calculateDynamicSystemLCC(rGen, rControllableEvents, rHighAlph, rGenDyn)
void calculateDynamicSystemLCC(const Generator& rGen, const EventSet& rControllableEvents, const EventSet& rHighAlph, Generator& rGenDyn){
	OP_DF("calculateDynamicSystemLCC(" << rGen.Name() << "," << rControllableEvents.Name() << "," << rHighAlph.Name() << "," << rGenDyn.Name() << ")");
	// transition relation sorted in reverse order for backwards reachability
	TransSetX2EvX1 tset_X2EvX1;
	rGen.TransRel(tset_X2EvX1);
	
	// prepare generator rGenDyn
	TransSet::Iterator tIt;
	TransSet::Iterator tEndIt;
	// labels for transitions for states with uncontrollable successor strings in the dynamic system
	// eventLCCLabel maps the original event to the LCCLabel in the dynamic system
	EventSet::Iterator eIt, eEndIt;
	map<Idx, Idx> eventLCCLabel;
	eIt = rHighAlph.Begin();
	eEndIt = rHighAlph.End();
	for( ; eIt != eEndIt; eIt++){
	    if(!rControllableEvents.Exists(*eIt) ){
          std::string eventname = ( rGenDyn.EventSymbolTablep())->UniqueSymbol(SymbolTable::GlobalEventSymbolTablep()->Symbol(*eIt) + "LCCLabel");
          eventLCCLabel[*eIt] = (rGenDyn.EventSymbolTablep())->InsEntry(eventname);	
          rGenDyn.InsEvent(eventLCCLabel[*eIt] );	    
	    }
	}
	// map from exit states (Idx) to uncontrollable high-level events (Idx)
	map<Idx,vector<Idx> > exitStateToEvents;	
	map<Idx,vector<Idx> >::const_iterator esIt, esEndIt;
	// algorithm for computing the dynamic system
	// loop over all states of original generator
	StateSet::Iterator sIt = rGen.StatesBegin();
	StateSet::Iterator sEndIt = rGen.StatesEnd();
	for( ; sIt != sEndIt; ++sIt) {
		OP_DF("calculateDynamicSystemLCC: loop over all states; current state: " << rGen.StateName(*sIt) 
		<< " [" << *sIt << "]");
		// check if current state (*stateSetIt) is an exit-state (a forward transition with
		// a high-level event exists. If yes, the outgoing high-level events are stored
		tIt = rGen.TransRelBegin(*sIt);
		tEndIt = rGen.TransRelEnd(*sIt);
		for(; tIt != tEndIt; ++tIt)
		{
			OP_DF("calculateDynamicSystemLCC: checking transition : " << rGen.TStr(*tIt));
			// if the event is an uncontrollable high-level event, it is inserted for the current exit state
			if(rHighAlph.Exists(tIt->Ev) && !rControllableEvents.Exists(tIt->Ev) ){
			    OP_DF("calculateDynamicSystemLCC: current state is an exit-state");
			    // insert the exit state and the high-level event into the exitStates set
			    esIt = exitStateToEvents.find(*sIt);
			    if(esIt == exitStateToEvents.end() ){
			        exitStateToEvents[*sIt] = vector<Idx>();
			    }
			    exitStateToEvents[*sIt].push_back(tIt->Ev);
			}
		}
	}
	// after this loop, we have the set of exit states with their outgoing uncontrollable high-level events. 
	// Now we compute the backwards reachable state via uncontrollable strings to determine all states where lcc holds
	esIt = exitStateToEvents.begin();
	esEndIt = exitStateToEvents.end();
	vector<Idx>::const_iterator vIt, vEndIt;
	// set of states already investigated
	StateSet doneStates;
 	for( ; esIt != esEndIt; esIt++){
        doneStates.Clear();
        recursiveCheckLCC(tset_X2EvX1, rControllableEvents, rHighAlph, esIt->first, doneStates);
        // add the uncontrollable transition labels to all states found in the backward reachability
        // transitions are introduced to all states that are reachable via strings including one high-level event
        vIt = esIt->second.begin();
        vEndIt = esIt->second.end();
        // loop over all uncontrollable high-level events in the current exit state
        for( ; vIt != vEndIt; vIt++){
            sIt = doneStates.Begin();
            sEndIt = doneStates.End();
            // loop over all backward reachable states
            for( ; sIt != sEndIt; sIt++){
                TransSet::Iterator dsIt, dsEndIt;
                dsIt = rGenDyn.TransRelBegin(*sIt);
                dsEndIt = rGenDyn.TransRelEnd(*sIt);
                // loop over all states reachable via strings with one high-level event (these states are already encoded
                // in rGenDyn if calculateDynamicSystemClosedObs is called before calculateDynamicSystemLCC)
                for( ; dsIt != dsEndIt; dsIt++){
                    rGenDyn.SetTransition(*sIt, eventLCCLabel[*vIt], dsIt->X2);
                }
            }
        }
 	}
}

void recursiveCheckLCC(const TransSetX2EvX1& rRevTransSet, const EventSet& rControllableEvents, const EventSet& rHighAlph, Idx currentState, StateSet& rDoneStates){
    // insert the current state into the backward reachable states via uncontrollable strings
    rDoneStates.Insert(currentState);
    // helpers
    TransSetX2EvX1::Iterator tIt, tEndIt;
    tIt = rRevTransSet.BeginByX2(currentState);
    tEndIt = rRevTransSet.EndByX2(currentState);
    // loop over all outgoing transitions in the current state
    for( ; tIt != tEndIt; tIt++){
        // if the transition is a high-level transition, we stop the backward reachability
        if(rHighAlph.Exists(tIt->Ev) ){
            // if the transition is uncontrollable, an uncontrollable string has been found -> LCC is fulfilled
            continue;
        }
        // if the event is an uncontrollable low-level event, we go backward along uncontrollable transitions
        else{
            if(!rControllableEvents.Exists(tIt->Ev) && !rDoneStates.Exists(tIt->X1) ){
                recursiveCheckLCC(rRevTransSet,rControllableEvents,rHighAlph,tIt->X1,rDoneStates);
            }
        }
    }
    // the end of the loop is reached, if no more uncontrollable event has been found from the current state
}


// calcClosedObserver(rGen,rHighAlph)
Idx calcClosedObserver(const Generator& rGen, EventSet& rHighAlph){
    // helpers
    Generator dynGen;
    map<Idx,Idx> mapStateToPartition;
    EventSet origAlph;
    Generator genPart;
    while(origAlph != rHighAlph){
        origAlph  = rHighAlph;
        dynGen.Clear();
        // compute the dynamic system for the given generator and high-level alphabet
        calculateDynamicSystemClosedObs(rGen, rHighAlph, dynGen);
        // compute the quasi conqruence 
        ComputeBisimulation(dynGen, mapStateToPartition, genPart);
        // Extend the high-level alphabet according to the algorithm of Lei
        ExtendHighAlphabet(rGen, rHighAlph, mapStateToPartition);
    }
    return genPart.Size();
}

// calcNaturalObserver(rGen,rHighAlph)
Int calcNaturalObserver(const Generator& rGen, EventSet& rHighAlph){
    // helpers
    Generator dynGen;
    map<Idx,Idx> mapStateToPartition;
    EventSet origAlph;
    Generator genPart;
    while(origAlph != rHighAlph){
        origAlph  = rHighAlph;
        dynGen.Clear();
        // compute the dynamic system for the given generator and high-level alphabet
        calculateDynamicSystemClosedObs(rGen, rHighAlph, dynGen);	
        calculateDynamicSystemObs(rGen, rHighAlph, dynGen);
        // compute the quasi conqruence 
        ComputeBisimulation(dynGen, mapStateToPartition, genPart);
        // Extend the high-level alphabet according to the algorithm of Lei
        ExtendHighAlphabet(rGen, rHighAlph, mapStateToPartition);
    }
    return genPart.Size();
}

// calcNaturalObserverLCC(rGen,rHighAlph)
Int calcNaturalObserverLCC(const Generator& rGen, const EventSet& rControllableEvents, EventSet& rHighAlph){
    // helpers
    Generator dynGen;
    map<Idx,Idx> mapStateToPartition;
    EventSet origAlph;
    Generator genPart;
    while(origAlph != rHighAlph){
        origAlph  = rHighAlph;
        dynGen.Clear();
        // compute the dynamic system for the given generator and high-level alphabet
        calculateDynamicSystemClosedObs(rGen, rHighAlph, dynGen);
        calculateDynamicSystemLCC(rGen, rControllableEvents, rHighAlph, dynGen);
        calculateDynamicSystemObs(rGen, rHighAlph, dynGen);
        // compute the quasi conqruence 
        ComputeBisimulation(dynGen, mapStateToPartition, genPart);
        // Extend the high-level alphabet according to the algorithm of Lei
        ExtendHighAlphabet(rGen, rHighAlph, mapStateToPartition);
    }
    return genPart.Size();
}

// calcMSAObserver(rGen,rHighAlph)
Int calcMSAObserver(const Generator& rGen, EventSet& rHighAlph){
    // helpers
    Generator dynGen;
    map<Idx,Idx> mapStateToPartition;
    EventSet origAlph;
    Generator genPart;
    while(origAlph != rHighAlph){
        origAlph  = rHighAlph;
        dynGen.Clear();
        // compute the dynamic system for the given generator and high-level alphabet
        calculateDynamicSystemClosedObs(rGen, rHighAlph, dynGen);
        calculateDynamicSystemMSA(rGen, rHighAlph, dynGen);
        // compute the quasi conqruence 
        ComputeBisimulation(dynGen, mapStateToPartition, genPart);
        // Extend the high-level alphabet according to the algorithm of Lei
        ExtendHighAlphabet(rGen, rHighAlph, mapStateToPartition);
    }
    return genPart.Size();
}   

// calcMSAObserverLCC(rGen,rHighAlph)
Int calcMSAObserverLCC(const Generator& rGen, const EventSet& rControllableEvents, EventSet& rHighAlph){
    // helpers
    Generator dynGen;
    map<Idx,Idx> mapStateToPartition;
    EventSet origAlph;
    Generator genPart;
    while(origAlph != rHighAlph){
        origAlph  = rHighAlph;
        dynGen.Clear();
        // compute the dynamic system for the given generator and high-level alphabet
        calculateDynamicSystemClosedObs(rGen, rHighAlph, dynGen);
        calculateDynamicSystemLCC(rGen, rControllableEvents, rHighAlph, dynGen);
        calculateDynamicSystemMSA(rGen, rHighAlph, dynGen);
        // compute the quasi conqruence 
        ComputeBisimulation(dynGen, mapStateToPartition, genPart);
        // Extend the high-level alphabet according to the algorithm of Lei
        ExtendHighAlphabet(rGen, rHighAlph, mapStateToPartition);
    }
    return genPart.Size();
}


// ExtendHighAlphabet(rGen,rHighAlph,rMapStateToPartition)
void ExtendHighAlphabet(const Generator& rGen, EventSet& rHighAlph,  map<Idx,Idx>& rMapStateToPartition){

  OP_DF("relabel(" << rGen.Name() << "," << rHighAlph.Name() << ", rMapStateToPartition)");

	// helpers
    map<Idx,Idx>::const_iterator spIt, spEndIt;
    spIt = rMapStateToPartition.begin();
    spEndIt = rMapStateToPartition.end();
    // Determine map from Partition to states from the rMapStateToPartition
    map<Idx,StateSet> partitionToStateMap; // (coset,states)
    map<Idx,StateSet>::iterator fIt, fEndIt;
    for( ; spIt != spEndIt; spIt++){
        fIt = partitionToStateMap.find(spIt->second);
        if(fIt == partitionToStateMap.end() ){
            partitionToStateMap[spIt->second] = StateSet();
        }
        partitionToStateMap[spIt->second].Insert(spIt->first);
        
    }
	// local events that lead to a different coset
	EventSet localViolatingEvents;
	// local events that appear inside the cosets
	EventSet localEvents;
	// vector for the nondeterministic exit states for each coset
	map<Idx,vector<pair<StateSet, Idx> > > nondeterministicStates; // coset, nondet states, event)
    StateSet::Iterator stIt, stEndIt;
    // Go through all cosets and identify local violating events and all local events
    fIt = partitionToStateMap.begin();
    fEndIt = partitionToStateMap.end();
    for( ; fIt != fEndIt; fIt++){
        // Current Coset
        Idx currentCoset = fIt->first;
        // Examine all states of the current coset
        stIt = fIt->second.Begin();
        stEndIt = fIt->second.End();
        // map from event to exit states and goal cosets
        map<Idx,map<Idx,pair<StateSet,IndexSet> > > eventStatesMap; // (coset, event, exit states, goal cosets)

        for( ; stIt != stEndIt; stIt++){
		    TransSet::Iterator tIt, tEndIt;
            tIt = rGen.TransRelBegin(*stIt);
            tEndIt = rGen.TransRelEnd(*stIt);
            for( ; tIt != tEndIt; tIt++){
                // local event
                if(!rHighAlph.Exists(tIt->Ev) ){
                    // unobservable transition to other coset
                    if(rMapStateToPartition[tIt->X2] != currentCoset){
                        localViolatingEvents.Insert(tIt->Ev);
                        localEvents.Erase(tIt->Ev);
                    }
                    else{
                        if(!localViolatingEvents.Exists(tIt->Ev) )
                            localEvents.Insert(tIt->Ev);
                    }
                }
                // high-level event
                else{
                    eventStatesMap[fIt->first][tIt->Ev].first.Insert(*stIt);
                    eventStatesMap[fIt->first][tIt->Ev].second.Insert(rMapStateToPartition[tIt->X2]);
                }
            } 
        }
		map<Idx,pair<StateSet,IndexSet> >::const_iterator seIt, seEndIt;
		seIt = eventStatesMap[fIt->first].begin();
		seEndIt = eventStatesMap[fIt->first].end();
		for( ; seIt  != seEndIt; seIt  ++){
			// if there is more than one goal coset, the current event introduces nondeterminism in the current coset
			if(seIt->second.second.Size() > 1){
				nondeterministicStates[fIt->first].push_back(make_pair(StateSet(),0) );
				nondeterministicStates[fIt->first].back().first = seIt->second.first;
				nondeterministicStates[fIt->first].back().second = seIt->first;
			}
		}
    }
   // All outgoing transitions for any coset are determined. Now, the localViolatingEvents are added
	// to the high-level alphabet, and the remaining nondeterministic transitions are found
	rHighAlph = rHighAlph + localViolatingEvents;
	// Only if there were no local violating events added to the high-level alphabet, the nondeterministic transitions are checked (Modification with respect to Lei Feng's algorithm)
	if(localViolatingEvents.Empty() == true){
		// Go through all cosets and identify local violating events and all local events
		rHighAlph = rHighAlph + localEvents;
		EventSet::Iterator eIt, eEndIt;
		eIt = localEvents.Begin();
		eEndIt = localEvents.End();
		// check which local events can be removed from the high-level alphabet
		for( ; eIt != eEndIt; eIt++){
			rHighAlph.Erase(*eIt);
			// go through all states of all cosets and check if rHighAlph splits all cosets
			fIt = partitionToStateMap.begin();
			fEndIt = partitionToStateMap.end();
			for( ; fIt != fEndIt; fIt++){
				if(nondeterministicStates[fIt->first].empty() == true) // no need to split if there are no nondeterministic states
					continue;
				stIt = fIt->second.Begin();
				stEndIt = fIt->second.End();
				bool erase = true;
				// Now, all combinations with nondeterministic exit states have to be checked 
				for( ; stIt != stEndIt; stIt++){
					// if the rHighAlph does not split the coset, the current event cannot be removed
					if(!CheckSplit(rGen, rHighAlph, nondeterministicStates[fIt->first], *stIt) ){
						rHighAlph.Insert(*eIt);
						erase = false;
						break;
					}
				}
				if(erase == false)
					break;
			}
		}
	}
}


// CheckSplit(rGen,rSplitAlphabet,rNondeterministicStates,entryState)
bool CheckSplit(const Generator& rGen, const EventSet& rSplitAlphabet, const vector<pair<StateSet, Idx> >& rNondeterministicStates, Idx entryState){
    StateSet accessibleReach;
    // compute the local accessible reach for the current entry state
    LocalAccessibleReach(rGen, rSplitAlphabet, entryState, accessibleReach);
    // check if for any of the nondeterministic state sets, more than one state appears in the local accessible reach
    vector<pair<StateSet, Idx> >::const_iterator vsIt, vsEndIt;
    vsIt = rNondeterministicStates.begin();
    vsEndIt = rNondeterministicStates.end();
    bool split = true;
    for( ; vsIt != vsEndIt; vsIt++){
        StateSet  nondetExit = vsIt->first * accessibleReach;
        StateSet::Iterator stIt, stEndIt;
        stIt = nondetExit.Begin();
        stEndIt = nondetExit.End();
        StateSet nondetReach;
        // collect the goal states of the nondeterministic exit states
        for( ; stIt != stEndIt; stIt++){
            nondetReach.Insert(rGen.TransRelBegin(*stIt, vsIt->second)->X2);
        }
        if( nondetReach.Size() > 1){
            split = false;
            break;
        }
    }
    return split;
}

// ==================================================================================
// Functions for the computation of the high-level alphabet with relabeling
// ==================================================================================


// calcAbstAlphClosed(rGenObs, rHighAlph, rNewHighAlph, rMapRelabeledEvents)
void calcAbstAlphClosed(System& rGenObs, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Idx,set<Idx > > &  rMapRelabeledEvents) {
	OP_DF("calcAbstAlphClosed(" << rGenObs.Name() << "," << "rHighAlph, rNewHighAlph, rMapRelabeledEvents)");
	// The controllable events are separated from the System. All functions that are successively 
	// called, are defined for Generators
	EventSet cntrevs = rGenObs.ControllableEvents();
	calcAbstAlphClosed(rGenObs, cntrevs , rHighAlph, rNewHighAlph, rMapRelabeledEvents);
	// the controllable events that have been changed by the called function are set in the System cGenObs
	rGenObs.SetControllable(cntrevs);
}

// calcAbstAlphClosed(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, rMapRelabeledEvents)
void calcAbstAlphClosed(Generator& rGenObs, EventSet& rControllableEvents, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Idx,set<Idx > > &  rMapRelabeledEvents) {
	OP_DF("calcAbstAlphClosed(" << rGenObs.Name() << "," << "rControllableEvents, rHighAlph, rNewHighAlph, rMapRelabeledEvents)");
	// The function called next returns a relabeled generator and a map of relabeled transitions
	map<Transition,Idx>  changedtrans;
	calcAbstAlphClosed(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, changedtrans);
	// for later use, the relabeled transitions are converted into relabeled events 
	// note that this function is accumulative, i.e., rMapRelabeledEvents need not be empty
	map<Transition,Idx>::iterator rtEndIt = changedtrans.end();
	map<Transition,Idx>::iterator rtIt =  changedtrans.begin();
	for(; rtIt != rtEndIt; rtIt++){
		if(rMapRelabeledEvents.find(rtIt->first.Ev) == rMapRelabeledEvents.end() ){// if the event does not exist, yet, a nex element in the map is created
			rMapRelabeledEvents[rtIt->first.Ev] = set<Idx>();
			if(rGenObs.Alphabet().Exists(rtIt->second) )// FIXME: there seem to be relabeled transitions that do not exist 
				rMapRelabeledEvents[rtIt->first.Ev].insert(rtIt->second);
		} 
		else {  // a new label is inserted into the map
			if(rGenObs.Alphabet().Exists(rtIt->second) )// FIXME: there seem to be relabeled transitions that do not exist    
				rMapRelabeledEvents[rtIt->first.Ev].insert(rtIt->second);
		}
	}
}

// calcAbstAlphClosed(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, rMapChangedTrans)
void calcAbstAlphClosed(Generator& rGenObs, EventSet& rControllableEvents, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Transition,Idx>& rMapChangedTrans)
{	
	OP_DF("calcAbstAlphClosed(" << rGenObs.Name() << ", rControllableEvents, rHighAlph, rNewHighAlph, rMapChangedTRans)");
	// Initialization of variables
	rNewHighAlph = rHighAlph;
	rMapChangedTrans.clear();
	Generator genDyn(rGenObs);
	map<Transition,Transition> mapChangedTransReverse;
	map<Idx,Idx> mapStateToPartition;
	map<Idx, EventSet> mapRelabeledEvents;
	bool done=false;
	#ifdef DF_PLOT
		Idx iterationCount=1;
		string name;
	#endif
	// observer algorithm: In each step, a dynamic system is computed that fulfills the one-step observer condition. 
	// Iterative application of the bisimulation algorithm and the relabeling procedure yields an automaton rGenObs
	// that, together with the new high-level alphabet rNewHighAlph fulfills the observer condition. 
	while(done==false)
	{
        	// compute the dynamic system for the given generator and high-level alphabet
        	calculateDynamicSystemClosedObs(rGenObs, rNewHighAlph, genDyn);
		#ifdef DF_PLOT
		name = ("./Automata/Plots/" + rGenObs.Name() + "DynamicSystem_" + ToStringInteger(iterationCount));
		genDyn.DotWrite(name);
		#endif
	
	
		// compute coarsest quasi-congruence on the dynamic system
		Generator genPart;
		ComputeBisimulation(genDyn, mapStateToPartition, genPart);
		#ifdef DF_PLOT
		name = ("./Automata/Plots/" + rGenObs.Name() + "Bisimulation_" + ToStringInteger(iterationCount));
		genPart.DotWrite(name);
		++iterationCount;
		#endif
	
		// check if quotient automaton is deterministic and free of unobservable events
		// and relabel transitions in rGenObs if necessary. The high-level alphabet is modified accordingly
		done=relabel(rGenObs, rControllableEvents, rNewHighAlph, mapStateToPartition, mapChangedTransReverse,  rMapChangedTrans, mapRelabeledEvents);
	}
}


// calcAbstAlphObs(rGenObs, rHighAlph, rNewHighAlph, rMapRelabeledEvents)
void calcAbstAlphObs(System& rGenObs, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Idx,set<Idx > > &  rMapRelabeledEvents) {
	OP_DF("calcAbstAlphObs(" << rGenObs.Name() << "," << "rHighAlph, rNewHighAlph, rMapRelabeledEvents)");
	// The controllable events are separated from the System. All functions that are successively 
	// called, are defined for Generators
	EventSet cntrevs = rGenObs.ControllableEvents();
	calcAbstAlphObs(rGenObs, cntrevs , rHighAlph, rNewHighAlph, rMapRelabeledEvents);
	// the controllable events that have been changed by the called function are set in the System cGenObs
	rGenObs.SetControllable(cntrevs);
}

// calcAbstAlphObs(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, rMapRelabeledEvents)
void calcAbstAlphObs(Generator& rGenObs, EventSet& rControllableEvents, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Idx,set<Idx > > &  rMapRelabeledEvents) {
	OP_DF("calcAbstAlphObs(" << rGenObs.Name() << "," << "rControllableEvents, rHighAlph, rNewHighAlph, rMapRelabeledEvents)");
	// The function called next returns a relabeled generator and a map of relabeled transitions
	map<Transition,Idx>  changedtrans;
	calcAbstAlphObs(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, changedtrans);
	// for later use, the relabeled transitions are converted into relabeled events 
	// note that this function is accumulative, i.e., rMapRelabeledEvents need not be empty
	map<Transition,Idx>::iterator rtEndIt = changedtrans.end();
	map<Transition,Idx>::iterator rtIt =  changedtrans.begin();
	for(; rtIt != rtEndIt; rtIt++){
		if(rMapRelabeledEvents.find(rtIt->first.Ev) == rMapRelabeledEvents.end() ){// if the event does not exist, yet, a nex element in the map is created
			rMapRelabeledEvents[rtIt->first.Ev] = set<Idx>();
			if(rGenObs.Alphabet().Exists(rtIt->second) )// FIXME: there seem to be relabeled transitions that do not exist 
				rMapRelabeledEvents[rtIt->first.Ev].insert(rtIt->second);
		} 
		else {  // a new label is inserted into the map
			if(rGenObs.Alphabet().Exists(rtIt->second) )// FIXME: there seem to be relabeled transitions that do not exist    
				rMapRelabeledEvents[rtIt->first.Ev].insert(rtIt->second);
		}
	}
}

// calcAbstAlphObs(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, rMapChangedTrans)
void calcAbstAlphObs(Generator& rGenObs, EventSet& rControllableEvents, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Transition,Idx>& rMapChangedTrans)
{	
	OP_DF("calcAbstAlphObs(" << rGenObs.Name() << ", rControllableEvents, rHighAlph, rNewHighAlph, rMapChangedTRans)");
	// Initialization of variables
	rNewHighAlph = rHighAlph;
	rMapChangedTrans.clear();
	Generator genDyn(rGenObs);
	map<Transition,Transition> mapChangedTransReverse;
	map<Idx,Idx> mapStateToPartition;
	map<Idx, EventSet> mapRelabeledEvents;
	bool done=false;
	#ifdef DF_PLOT
		Idx iterationCount=1;
		string name;
	#endif
	// observer algorithm: In each step, a dynamic system is computed that fulfills the one-step observer condition. 
	// Iterative application of the bisimulation algorithm and the relabeling procedure yields an automaton rGenObs
	// that, together with the new high-level alphabet rNewHighAlph fulfills the observer condition. 
	while(done==false)
	{
        	// compute the dynamic system for the given generator and high-level alphabet
        	calculateDynamicSystemClosedObs(rGenObs, rNewHighAlph, genDyn);
        	calculateDynamicSystemObs(rGenObs, rNewHighAlph, genDyn);
		#ifdef DF_PLOT
		name = ("./Automata/Plots/" + rGenObs.Name() + "DynamicSystem_" + ToStringInteger(iterationCount));
		genDyn.DotWrite(name);
		#endif
	
		// compute coarsest quasi-congruence on the dynamic system
		Generator genPart;
		ComputeBisimulation(genDyn, mapStateToPartition, genPart);
		#ifdef DF_PLOT
		name = ("./Automata/Plots/" + rGenObs.Name() + "Bisimulation_" + ToStringInteger(iterationCount));
		genPart.DotWrite(name);
		++iterationCount;
		#endif
	
		// check if quotient automaton is deterministic and free of unobservable events
		// and relabel transitions in rGenObs if necessary. The high-level alphabet is modified accordingly
		done=relabel(rGenObs, rControllableEvents, rNewHighAlph, mapStateToPartition, mapChangedTransReverse,  rMapChangedTrans, mapRelabeledEvents);
	}
}


// calcAbstAlphMSA(rGenObs, rHighAlph, rNewHighAlph, rMapRelabeledEvents)
void calcAbstAlphMSA(System& rGenObs, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Idx,set<Idx > > &  rMapRelabeledEvents) {
	OP_DF("calcAbstAlphMSA(" << rGenObs.Name() << "," << "rHighAlph, rNewHighAlph, rMapRelabeledEvents)");
	// The controllable events are separated from the System. All functions that are successively 
	// called, are defined for Generators
	EventSet cntrevs = rGenObs.ControllableEvents();
	calcAbstAlphMSA(rGenObs, cntrevs , rHighAlph, rNewHighAlph, rMapRelabeledEvents);
	// the controllable events that have been changed by the called function are set in the System cGenObs
	rGenObs.SetControllable(cntrevs);
}

// calcAbstAlphMSA(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, rMapRelabeledEvents)
void calcAbstAlphMSA(Generator& rGenObs, EventSet& rControllableEvents, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Idx,set<Idx > > &  rMapRelabeledEvents) {
	OP_DF("calcAbstAlphMSA(" << rGenObs.Name() << "," << "rControllableEvents, rHighAlph, rNewHighAlph, rMapRelabeledEvents)");
	// The function called next returns a relabeled generator and a map of relabeled transitions
	map<Transition,Idx>  changedtrans;
	calcAbstAlphMSA(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, changedtrans);
	// for later use, the relabeled transitions are converted into relabeled events 
	// note that this function is accumulative, i.e., rMapRelabeledEvents need not be empty
	map<Transition,Idx>::iterator rtEndIt = changedtrans.end();
	map<Transition,Idx>::iterator rtIt =  changedtrans.begin();
	for(; rtIt != rtEndIt; rtIt++){
		if(rMapRelabeledEvents.find(rtIt->first.Ev) == rMapRelabeledEvents.end() ){// if the event does not exist, yet, a nex element in the map is created
			rMapRelabeledEvents[rtIt->first.Ev] = set<Idx>();
			if(rGenObs.Alphabet().Exists(rtIt->second) )// FIXME: there seem to be relabeled transitions that do not exist 
				rMapRelabeledEvents[rtIt->first.Ev].insert(rtIt->second);
		} 
		else {  // a new label is inserted into the map
			if(rGenObs.Alphabet().Exists(rtIt->second) )// FIXME: there seem to be relabeled transitions that do not exist    
				rMapRelabeledEvents[rtIt->first.Ev].insert(rtIt->second);
		}
	}
}

// calcAbstAlphMSA(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, rMapChangedTrans)
void calcAbstAlphMSA(Generator& rGenObs, EventSet& rControllableEvents, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Transition,Idx>& rMapChangedTrans)
{	
	OP_DF("calcAbstAlphMSA(" << rGenObs.Name() << ", rControllableEvents, rHighAlph, rNewHighAlph, rMapChangedTRans)");
	// Initialization of variables
	rNewHighAlph = rHighAlph;
	rMapChangedTrans.clear();
	Generator genDyn(rGenObs);
	map<Transition,Transition> mapChangedTransReverse;
	map<Idx,Idx> mapStateToPartition;
	map<Idx, EventSet> mapRelabeledEvents;
	bool done=false;
	#ifdef DF_PLOT
		Idx iterationCount=1;
		string name;
	#endif
	// observer algorithm: In each step, a dynamic system is computed that fulfills the one-step observer condition. 
	// Iterative application of the bisimulation algorithm and the relabeling procedure yields an automaton rGenObs
	// that, together with the new high-level alphabet rNewHighAlph fulfills the observer condition. 
	while(done==false)
	{
        	// compute the dynamic system for the given generator and high-level alphabet
        	calculateDynamicSystemClosedObs(rGenObs, rNewHighAlph, genDyn);
        	calculateDynamicSystemMSA(rGenObs, rNewHighAlph, genDyn);
		#ifdef DF_PLOT
		name = ("./Automata/Plots/" + rGenObs.Name() + "DynamicSystem_" + ToStringInteger(iterationCount));
		genDyn.DotWrite(name);
		#endif
	

		// compute coarsest quasi-congruence on the dynamic system
		Generator genPart;
		ComputeBisimulation(genDyn, mapStateToPartition, genPart);
		#ifdef DF_PLOT
		name = ("./Automata/Plots/" + rGenObs.Name() + "Bisimulation_" + ToStringInteger(iterationCount));
		genPart.DotWrite(name);
		++iterationCount;
		#endif
	
		// check if quotient automaton is deterministic and free of unobservable events
		// and relabel transitions in rGenObs if necessary. The high-level alphabet is modified accordingly
		done=relabel(rGenObs, rControllableEvents, rNewHighAlph, mapStateToPartition, mapChangedTransReverse,  rMapChangedTrans, mapRelabeledEvents);
	}
}


// // calcAbstAlphObsOCC(rGenObs, rHighAlph, rNewHighAlph, rMapRelabeledEvents)
// void calcAbstAlphObsOCC(System& rGenObs, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Idx,set<Idx > > &  rMapRelabeledEvents){
// 	OP_DF("calcAbstAlphObsOCC(" << rGenObs.Name() << "," << "rHighAlph, rNewHighAlph, rMapRelabeledEvents)");
// 	// The controllable events are separated from the System. All functions that are successively 
// 	// called, are defined for Generators
//    map<Transition,Idx>  changedtrans;
//    EventSet rControllableEvents = rGenObs.ControllableEvents();
//    // The function called next returns a relabeled generator and a map of relabeled transitions
//    calcAbstAlphObsOCC(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, changedtrans);
//    // for later use, the relabeled transitions are converted into relabeled events 
// 	// note that this function is accumulative, i.e., rMapRelabeledEvents need not be empty
//    map<Transition,Idx>::iterator rtEndIt = changedtrans.end();
//    map<Transition,Idx>::iterator rtIt =  changedtrans.begin();
//    for(; rtIt != rtEndIt; rtIt++){
//        if(rMapRelabeledEvents.find(rtIt->first.Ev) == rMapRelabeledEvents.end() ){
//           rMapRelabeledEvents[rtIt->first.Ev] = set<Idx>();
//           if(rGenObs.Alphabet().Exists(rtIt->second) )// FIXME: there seem to be relabeled transitions that do not exist 
//               rMapRelabeledEvents[rtIt->first.Ev].insert(rtIt->second);
//        } else {  
//           if(rGenObs.Alphabet().Exists(rtIt->second) )// FIXME: there seem to be relabeled transitions that do not exist    
//               rMapRelabeledEvents[rtIt->first.Ev].insert(rtIt->second);
//        }
//    } 
//    // the controllable events that have been changed by the called function are set in the System cGenObs
//    rGenObs.SetControllable(rControllableEvents);
// }

// // calcAbstAlphObsOCC(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, rMapChangedTrans)
// void calcAbstAlphObsOCC(Generator& rGenObs, EventSet& rControllableEvents, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Transition,Idx>& rMapChangedTrans)
// {
//    OP_DF("calcAbstAlphObsOCC(" << rGenObs.Name() << ", rControllableEvents, rHighAlph, rNewHighAlph, rMapChangedTRans)");
// 	// Initialization of variables
//    rNewHighAlph = rHighAlph;
//    rMapChangedTrans.clear();
//    Generator genDyn(rGenObs);
//    map<Transition,Transition> mapChangedTransReverse;
//    vector<Idx> newPartitions;
//    map<Idx,Idx> mapStateToPartition;
//    map<Idx, EventSet> mapRelabeledEvents;
//    bool done=false;
//    #ifdef DF_PLOT
//        Idx iterationCount=1;
//        string name;
//    #endif   
//    // observer algorithm: In each step, a dynamic system is computed that fulfills the one-step observer condition
//    // and output control consistency (OCC). 
// 	// Iterative application of the bisimulation algorithm and the relabeling procedure yields an automaton rGenObs
// 	// that, together with the new high-level alphabet rNewHighAlph fulfills the observer condition and OCC.
//    while(done==false)
//    {
//        // compute dynamic system for Lm-observer and OCC on rGenObs
//        calculateDynamicSystemObsOCC(rGenObs, rControllableEvents, rNewHighAlph, genDyn);
//        #ifdef DF_PLOT
//            name = ("./Automata/Plots/" + rGenObs.Name() + "DynamicSystem_" + ToStringInteger(iterationCount));
//            genDyn.DotWrite(name);
//        #endif
// 
//        Generator genPart;
//        mapStateToPartition.clear();
//        newPartitions.clear();
//        // compute coarsest quasi-congruence on the dynamic system
//        ComputeBisimulation(genDyn, mapStateToPartition, genPart, newPartitions);
//        #ifdef DF_PLOT
//            name = ("./Automata/Plots/" + rGenObs.Name() + "Bisimulation_" + ToStringInteger(iterationCount));
//            genPart.DotWrite(name);
//            ++iterationCount;
//        #endif
//    
//        // check if quotient automaton is deterministic and free of unobservable events
// 		// and relabel transitions in rGenObs if necessary. The high-level alphabet is modified accordingly
//        done=relabel(rGenObs, rControllableEvents, rNewHighAlph, newPartitions, mapStateToPartition, mapChangedTransReverse,  rMapChangedTrans, mapRelabeledEvents);
//    }
// }
/*
// calculateDynamicSystemObsOCC(rGen, rControllableEvents, rHighAlph, rGenDyn)
void calculateDynamicSystemObsOCC(const Generator& rGen, EventSet& rControllableEvents, EventSet& rHighAlph, Generator& rGenDyn){
   OP_DF("calculateDynamicSystemObsOCC(" << rGen.Name() << "," << rControllableEvents.Name() << "," << rHighAlph.Name() << "," << rGenDyn.Name() << ")");
	// transition relation sorted in reverse order for backwards reachability
   TransSetX2EvX1 tset_X2EvX1;
   rGen.TransRel(tset_X2EvX1);

	// prepare generator rGenDyn
	rGenDyn.ClearTransRel();
	rGenDyn.InjectAlphabet(rHighAlph); // all high-level events are contained in the alphabet of rGenDyn
	// labels for transitions to marked states and for controllable paths
	std::string eventname = ( rGenDyn.EventSymbolTablep())->UniqueSymbol("cLabel_1");
	Idx cLabel = (rGenDyn.EventSymbolTablep())->InsEntry(eventname);
	eventname = ( rGenDyn.EventSymbolTablep())->UniqueSymbol("mLabel_1");
	Idx mLabel = (rGenDyn.EventSymbolTablep())->InsEntry(eventname);
	rGenDyn.InsEvent(cLabel);
	rGenDyn.InsEvent(mLabel);
	rGenDyn.InjectInitStates(rGen.InitStates() );
	rGenDyn.InjectStates(rGen.States() );
	rGenDyn.InjectMarkedStates(rGen.MarkedStates() );

	// maps for the construction of the dynamic system
	map<Idx, map<Idx, bool> > exitLocalStatesMap; // map from each exit state to locally backward reachable states and a boolean that is false if there exists an uncontrollable path to the exit state
	map<Idx, StateSet> entryLocalStatesMap; // map from entry states to locally forward reachable states
	StateSet::Iterator stIt, stEndIt;
	stIt = rGen.StatesBegin();
	stEndIt = rGen.StatesEnd();
	TransSet::Iterator tsIt, tsEndIt;
	bool isExitState;
	// go through all states of the original generator
	for(; stIt != stEndIt; stIt++){
	    OP_DF("calculateDynamicSystemObsOCC: loop over all states; current state: " << rGen.StateName(*stIt) 
		<< " [" << *stIt << "]");
		// determine the marked states that are locally reachable from the current state and insert 
		// transitions labeled with mLabel in rGenDyn
		forwardReachabilityObs(rGen, rHighAlph, *stIt, mLabel, rGenDyn);
		// if the current state is an exit state, carry out the backward reachability to determine 
		// which states can be reached on a controllable/uncontrollable path -> store in exitLocalStatesMap
		// in this case, also determine the corresponding entry states and compute their locally reachable states
		// for the entryLocalStatesMap
		tsIt = rGen.TransRelBegin(*stIt);
		tsEndIt = rGen.TransRelEnd(*stIt);
		isExitState = false;
		for( ; tsIt != tsEndIt; tsIt++){
			if(rHighAlph.Exists(tsIt->Ev) ){
              		 OP_DF("calculateDynamicSystemObsOCC: current state is an exit-state");
				isExitState = true;
				// if the local reach for the connected entry state has not been computed, yet, insert it in the 
				// entryLocalStatesMap
				if( entryLocalStatesMap.find(tsIt->X2) == entryLocalStatesMap.end() ){
					entryLocalStatesMap[tsIt->X2] = StateSet();
					LocalAccessibleReach(rGen,rHighAlph, tsIt->X2, entryLocalStatesMap[tsIt->X2]);
				}
			}
		}
		// if the current state is an exit state, compute the backward local reach with the controllability properties of the 
		// paths to locally backward reachable states
		if(isExitState == true){
			StateSet doneStates;
			exitLocalStatesMap[*stIt][*stIt] = false; // the exit state is reachable from the exit state via an uncontrollable path
			doneStates.Insert(*stIt);
			backwardReachabilityObsOCC(tset_X2EvX1, rControllableEvents, rHighAlph, *stIt, *stIt, false, exitLocalStatesMap, doneStates);
			
		}
		
	}
	// the generator rGenDyn is constructed by connecting all exit and entry states with their local state sets
	map<Idx, map<Idx, bool> >::const_iterator elIt, elEndIt;
	elIt = exitLocalStatesMap.begin();
	elEndIt = exitLocalStatesMap.end();
	map<Idx,bool>::const_iterator lcIt, lcEndIt;
	StateSet::Iterator exIt, exEndIt;
	map<Idx,StateSet>::const_iterator enIt;

	for( ; elIt != elEndIt; elIt++){
		lcEndIt = elIt->second.end();
		// go over all entry states reachable from the current exit state (via all feasible high-level events)
		tsIt = rGen.TransRel().Begin(elIt->first);
		tsEndIt = rGen.TransRel().End(elIt->first);
		for( ; tsIt != tsEndIt; tsIt++){
		    OP_DF("calculateDynamicSystemObsOCC: insert transitions for the high-level event" << rGen.EventName(tsIt->Ev) << "[" << tsIt->Ev << "]");
			if(rHighAlph.Exists(tsIt->Ev) ){
				bool controllable = rControllableEvents.Exists(tsIt->Ev);
				enIt = entryLocalStatesMap.find(tsIt->X2);
				stEndIt = enIt->second.End();
				// iterate over all locally backward reachable states from current exit state
				for(lcIt = elIt->second.begin(); lcIt != lcEndIt; lcIt ++){
					// iterate over all locally forward reachable states from current entry state
					for( stIt = enIt->second.Begin(); stIt != stEndIt; stIt++){
					    OP_DF("calculateDynamicSystemObsOCC: Transition added to resulting generator: " << 
       				rGenDyn.TStr(Transition(lcIt->first,tsIt->Ev,*stIt))); 
						
                       rGenDyn.SetTransition(lcIt->first,tsIt->Ev,*stIt); // insert a transition for each local state combination
                       if( controllable || lcIt->second ){ // insert an clabel transition if the local path is controllable or the high-level event is controllable
                           OP_DF("calculateDynamicSystemObsOCC: cLabel-Transition added to resulting generator: " << 
                           rGenDyn.TStr(Transition(lcIt->first,cLabel,*stIt))); 
                           
                           rGenDyn.SetTransition(lcIt->first,cLabel,*stIt);
                       }
					}
				}
			}
		}		
	}
}

// forwardReachabilityObs(rGen, rHighAlph, lowState, mLabel, rGenDyn)
void forwardReachabilityObs(const Generator& rGen, const EventSet& rHighAlph, Idx lowState, Idx mLabel, Generator& rGenDyn) {
	OP_DF("forwardReachabilityObs(" << rGen.Name() << "," << rHighAlph.Name() << "," << lowState << "," << rGenDyn.EventName(mLabel) << "," << rGenDyn.Name() << ")");
   // helpers:
	// iterators
	TransSet::Iterator tIt;
	TransSet::Iterator tEndIt;
	// todo list
	std::stack<Idx> todo;

	// algorithm: the locally reachable states from lowState are evaluated. If a reachable state is marked,
	// a transition with mLabel is inserted from lowState to that state is inserted in rGenDyn. 
	todo.push(lowState);
	StateSet doneStates;
	// if lowState is marked itself, the dynamic system contains a selfloop with the mlabel
	if(rGen.MarkedStates().Exists(lowState) ){
	    OP_DF("forwardReachabilityObs: Transition with mLabel added to resulting generator: " << 
       				rGenDyn.TStr(Transition(lowState,mLabel,lowState)));
		rGenDyn.SetTransition(lowState, mLabel, lowState);
	}

	doneStates.Insert(lowState);
	// the local reachability is evaluated until no new state is found
	while (! todo.empty()) {
		const Idx current = todo.top();
		todo.pop();
		tIt = rGen.TransRelBegin(current);
		tEndIt = rGen.TransRelEnd(current);
		for (; tIt != tEndIt; ++tIt) {
			// if the current transition is labeled with a high level event, it is skipped
			if (rHighAlph.Exists(tIt->Ev)) {
				continue;
			}
			// if the successor state has not been found, yst (not in doneStates)
			else if (! doneStates.Exists(tIt->X2)) {
				todo.push(tIt->X2);
				if(rGen.MarkedStates().Exists(tIt->X2) ){
				    OP_DF("forwardReachabilityObs: Transition with mLabel added to resulting generator: " << 
       				rGenDyn.TStr(Transition(lowState,mLabel,tIt->X2)));
					rGenDyn.SetTransition(lowState, mLabel, tIt->X2);
				}
				doneStates.Insert(tIt->X2);
			}
		}
	}
}*/

// backwardReachabilityObsOCC(rTransSetX2EvX1, rControllableEvents, rHighAlph, exitState, currentState, controllablePath, rExitLocalStatesMap, rDoneStates) 
void backwardReachabilityObsOCC(const TransSetX2EvX1& rTransSetX2EvX1, const EventSet& rControllableEvents, const EventSet& rHighAlph, Idx exitState, Idx currentState, bool controllablePath, map<Idx, map<Idx, bool> >& rExitLocalStatesMap, StateSet& rDoneStates){
	OP_DF("backwardReachabilityObsOCC(rTransSetX2EvX1," << rControllableEvents.Name() << "," << rHighAlph.Name() << "," << exitState << "," << currentState << "," << controllablePath << ",rExitLocalStatesMap, rDoneStates)");
   // go along all backward transitions. Discard the goal state if it is reached via a high-level event or if it is in the rDoneStates and 
   // the controllability properties of the state do not change on the current path
   
   // helpers
	TransSetX2EvX1::Iterator tsIt, tsEndIt;
   tsIt = rTransSetX2EvX1.BeginByX2(currentState);
   tsEndIt = rTransSetX2EvX1.EndByX2(currentState);
	bool currentControllablePath;
	// we iterate over all backward transitions of the currentState to establish backward reachability
	for( ;tsIt != tsEndIt; tsIt++){
	    // states reachable via a high-level event are not in the local backward reach and the controllability property of the current exitState does not change
		if( !rHighAlph.Exists(tsIt->Ev) && tsIt->X1 != exitState){
			// if the state has not been visited, yet, the controllability of the current path are set in the rExitLocalStatesMap
           		if( !rDoneStates.Exists(tsIt->X1) ){
				rDoneStates.Insert(tsIt->X1);
				// the path is controllable if the current transition has a controllable event or the path was already controllable
				currentControllablePath = rControllableEvents.Exists(tsIt->Ev) || controllablePath;
				rExitLocalStatesMap[exitState][tsIt->X1] = currentControllablePath;
				// as the state has not been visited, yet, it is subject to a new backward reachability
				backwardReachabilityObsOCC(rTransSetX2EvX1, rControllableEvents, rHighAlph, exitState, tsIt->X1, currentControllablePath, rExitLocalStatesMap, rDoneStates);
			}
			else{ // for an existing state, the controllability value can change from uncontrollable to controllable (if 
				// a new controllable path has been found). It is important to note, that the OCC condition implies that
				// if there is one controllable path, then the the state is flagged controllable except for the case of the 
				// given exitState that is always uncontrollable
				currentControllablePath = rControllableEvents.Exists(tsIt->Ev) || controllablePath;
				if(rExitLocalStatesMap[exitState][tsIt->X1] != currentControllablePath && currentControllablePath == true){
					rExitLocalStatesMap[exitState][tsIt->X1] = true;
					// as the controllabiity attribute of the current state changed it is subject to a new backward reachability
					backwardReachabilityObsOCC(rTransSetX2EvX1, rControllableEvents, rHighAlph, exitState, tsIt->X1, true, rExitLocalStatesMap, rDoneStates);
				}
			}
		}
	}
}

// calcAbstAlphObsLCC(rGenObs, rHighAlph, rNewHighAlph, rMapRelabeledEvents)
void calcAbstAlphObsLCC(System& rGenObs, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Idx,set<Idx > > &  rMapRelabeledEvents){
   OP_DF("calcAbstAlphObsLCC(" << rGenObs.Name() << "," << "rHighAlph, rNewHighAlph, rMapRelabeledEvents)");
   // The controllable events are separated from the System. All functions that are successively 
   // called, are defined for Generators
   map<Transition,Idx>  changedtrans;
   EventSet rControllableEvents = rGenObs.ControllableEvents();
   // The function called next returns a relabeled generator and a map of relabeled transitions
   calcAbstAlphObsLCC(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, changedtrans);
   // for later use, the relabeled transitions are converted into relabeled events 
	// note that this function is accumulative, i.e., rMapRelabeledEvents need not be empty
   map<Transition,Idx>::iterator rtEndIt = changedtrans.end();
   map<Transition,Idx>::iterator rtIt =  changedtrans.begin();
   for(; rtIt != rtEndIt; rtIt++){
   if(rMapRelabeledEvents.find(rtIt->first.Ev) == rMapRelabeledEvents.end() ){
      rMapRelabeledEvents[rtIt->first.Ev] = set<Idx>();
      if(rGenObs.Alphabet().Exists(rtIt->second) )
          rMapRelabeledEvents[rtIt->first.Ev].insert(rtIt->second);
   } else {  
      if(rGenObs.Alphabet().Exists(rtIt->second) )
          rMapRelabeledEvents[rtIt->first.Ev].insert(rtIt->second);
   }
     } 
     // the controllable events that have been changed by the called function are set in the System cGenObs
     rGenObs.SetControllable(rControllableEvents);
}

// calcAbstAlphObsLCC(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, rMapChangedTrans)
void calcAbstAlphObsLCC(Generator& rGenObs, EventSet& rControllableEvents, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Transition,Idx>& rMapChangedTrans)
{
   OP_DF("calcAbstAlphObsLCC(" << rGenObs.Name() << ", rControllableEvents, rHighAlph, rNewHighAlph, rMapChangedTRans)");
	// Initialization of variables
   rNewHighAlph = rHighAlph;
   rMapChangedTrans.clear();
   Generator genDyn(rGenObs);
   map<Transition,Transition> mapChangedTransReverse;
   map<Idx,Idx> mapStateToPartition;
   map<Idx, EventSet> mapRelabeledEvents;
   bool done=false;
   #ifdef DF_PLOT
       Idx iterationCount=1;
       string name;
   #endif   
   // observer algorithm: In each step, a dynamic system is computed that fulfills the one-step observer condition
   // and local control consistency (LCC). 
	// Iterative application of the bisimulation algorithm and the relabeling procedure yields an automaton rGenObs
	// that, together with the new high-level alphabet rNewHighAlph fulfills the observer condition and LCC.
   while(done==false)
   {
        // compute the dynamic system for the given generator and high-level alphabet
        calculateDynamicSystemClosedObs(rGenObs, rNewHighAlph, genDyn);
        calculateDynamicSystemLCC(rGenObs, rControllableEvents, rNewHighAlph, genDyn);
        calculateDynamicSystemObs(rGenObs, rNewHighAlph, genDyn);
       #ifdef DF_PLOT
           name = ("./Automata/Plots/" + rGenObs.Name() + "DynamicSystem_" + ToStringInteger(iterationCount));
           genDyn.DotWrite(name);
       #endif

       // compute coarsest quasi-congruence on the dynamic system
       Generator genPart;
       ComputeBisimulation(genDyn, mapStateToPartition, genPart);
       #ifdef DF_PLOT
           name = ("./Automata/Plots/" + rGenObs.Name() + "Bisimulation_" + ToStringInteger(iterationCount));
           genPart.DotWrite(name);
           ++iterationCount;
       #endif
   
       // check if quotient automaton is deterministic and free of unobservable events
		// and relabel transitions in rGenObs if necessary. The high-level alphabet is modified accordingly
       done=relabel(rGenObs, rControllableEvents, rNewHighAlph, mapStateToPartition, mapChangedTransReverse,  rMapChangedTrans, mapRelabeledEvents);
   }
}


// calcAbstAlphMSALCC(rGenObs, rHighAlph, rNewHighAlph, rMapRelabeledEvents)
void calcAbstAlphMSALCC(System& rGenObs, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Idx,set<Idx > > &  rMapRelabeledEvents){
   OP_DF("calcAbstAlphMSALCC(" << rGenObs.Name() << "," << "rHighAlph, rNewHighAlph, rMapRelabeledEvents)");
   // The controllable events are separated from the System. All functions that are successively 
   // called, are defined for Generators
   map<Transition,Idx>  changedtrans;
   EventSet rControllableEvents = rGenObs.ControllableEvents();
   // The function called next returns a relabeled generator and a map of relabeled transitions
   calcAbstAlphMSALCC(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, changedtrans);
   // for later use, the relabeled transitions are converted into relabeled events 
	// note that this function is accumulative, i.e., rMapRelabeledEvents need not be empty
   map<Transition,Idx>::iterator rtEndIt = changedtrans.end();
   map<Transition,Idx>::iterator rtIt =  changedtrans.begin();
   for(; rtIt != rtEndIt; rtIt++){
   if(rMapRelabeledEvents.find(rtIt->first.Ev) == rMapRelabeledEvents.end() ){
      rMapRelabeledEvents[rtIt->first.Ev] = set<Idx>();
      if(rGenObs.Alphabet().Exists(rtIt->second) )
          rMapRelabeledEvents[rtIt->first.Ev].insert(rtIt->second);
   } else {  
      if(rGenObs.Alphabet().Exists(rtIt->second) )
          rMapRelabeledEvents[rtIt->first.Ev].insert(rtIt->second);
   }
     } 
     // the controllable events that have been changed by the called function are set in the System cGenObs
     rGenObs.SetControllable(rControllableEvents);
}

// calcAbstAlphMSALCC(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, rMapChangedTrans)
void calcAbstAlphMSALCC(Generator& rGenObs, EventSet& rControllableEvents, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Transition,Idx>& rMapChangedTrans)
{
   OP_DF("calcAbstAlphMSALCC(" << rGenObs.Name() << ", rControllableEvents, rHighAlph, rNewHighAlph, rMapChangedTRans)");
	// Initialization of variables
   rNewHighAlph = rHighAlph;
   rMapChangedTrans.clear();
   Generator genDyn(rGenObs);
   map<Transition,Transition> mapChangedTransReverse;
   map<Idx,Idx> mapStateToPartition;
   map<Idx, EventSet> mapRelabeledEvents;
   bool done=false;
   #ifdef DF_PLOT
       Idx iterationCount=1;
       string name;
   #endif   
   // observer algorithm: In each step, a dynamic system is computed that fulfills the one-step observer condition
   // and local control consistency (LCC). 
	// Iterative application of the bisimulation algorithm and the relabeling procedure yields an automaton rGenObs
	// that, together with the new high-level alphabet rNewHighAlph fulfills the observer condition and LCC.
   while(done==false)
   {
        // compute the dynamic system for the given generator and high-level alphabet
        calculateDynamicSystemClosedObs(rGenObs, rNewHighAlph, genDyn);
        calculateDynamicSystemLCC(rGenObs, rControllableEvents, rNewHighAlph, genDyn);
        calculateDynamicSystemMSA(rGenObs, rNewHighAlph, genDyn);
       #ifdef DF_PLOT
           name = ("./Automata/Plots/" + rGenObs.Name() + "DynamicSystem_" + ToStringInteger(iterationCount));
           genDyn.DotWrite(name);
       #endif

       // compute coarsest quasi-congruence on the dynamic system
       Generator genPart;
       ComputeBisimulation(genDyn, mapStateToPartition, genPart);
       #ifdef DF_PLOT
           name = ("./Automata/Plots/" + rGenObs.Name() + "Bisimulation_" + ToStringInteger(iterationCount));
           genPart.DotWrite(name);
           ++iterationCount;
       #endif
   
       // check if quotient automaton is deterministic and free of unobservable events
		// and relabel transitions in rGenObs if necessary. The high-level alphabet is modified accordingly
       done=relabel(rGenObs, rControllableEvents, rNewHighAlph, mapStateToPartition, mapChangedTransReverse,  rMapChangedTrans, mapRelabeledEvents);
   }
}

/*
// calculateDynamicSystemObsLCC(rGen, rControllableEvents, rHighAlph, rGenDyn)
void calculateDynamicSystemObsLCC(const Generator& rGen, EventSet& rControllableEvents, EventSet& rHighAlph, Generator& rGenDyn){
   OP_DF("calculateDynamicSystemObsLCC(" << rGen.Name() << "," << rControllableEvents.Name() << "," << rHighAlph.Name() << "," << rGenDyn.Name() << ")");
	// transition relation sorted in reverse order for backwards reachability
   TransSetX2EvX1 tset_X2EvX1;
  	rGen.TransRel(tset_X2EvX1);

  	// prepare generator rGenDyn
  	rGenDyn.ClearTransRel();
  	rGenDyn.InjectAlphabet(rHighAlph);
	std::string eventname = ( rGenDyn.EventSymbolTablep())->UniqueSymbol("ucLabel_1");
	Idx ucLabel = (rGenDyn.EventSymbolTablep())->InsEntry(eventname);
	eventname = ( rGenDyn.EventSymbolTablep())->UniqueSymbol("mLabel_1");
	Idx mLabel = (rGenDyn.EventSymbolTablep())->InsEntry(eventname);
	rGenDyn.InsEvent(ucLabel);
	rGenDyn.InsEvent(mLabel);
	rGenDyn.InjectInitStates(rGen.InitStates() );
	rGenDyn.InjectStates(rGen.States() );
	rGenDyn.InjectMarkedStates(rGen.MarkedStates() );

	// maps for the construction of the dynamic system
	map<Idx, map<Idx, bool> > exitLocalStatesMap; // map from each exit state to locally backward reachable states and a boolean that is false if there exists an uncontrollable path to the exit state
	map<Idx, StateSet> entryLocalStatesMap; // map from entry states to locally forward reachable states
	StateSet::Iterator stIt, stEndIt;
	stIt = rGen.StatesBegin();
	stEndIt = rGen.StatesEnd();
	TransSet::Iterator tsIt, tsEndIt;
	bool isExitState;
	// go through all states of the original generator
	for(; stIt != stEndIt; stIt++){
		OP_DF("calculateDynamicSystemObsLCC: loop over all states; current state: " << rGen.StateName(*stIt) 
		<< " [" << *stIt << "]");
		// determine the marked states that are locally reachable from the current state and insert 
		// transitions labeled with mLabel in rGenDyn
		forwardReachabilityObs(rGen, rHighAlph, *stIt, mLabel, rGenDyn);
		// if the current state is an exit state, carry out the backward reachability to determine 
		// which states can be reached on a controllable/uncontrollable path -> store in exitLocalStatesMap
		// in this case, also determine the corresponding entry states and compute their locally reachable states
		// for the entryLocalStatesMap
		tsIt = rGen.TransRelBegin(*stIt);
		tsEndIt = rGen.TransRelEnd(*stIt);
		isExitState = false;
		for( ; tsIt != tsEndIt; tsIt++){
			if(rHighAlph.Exists(tsIt->Ev) ){
			    OP_DF("calculateDynamicSystemObsLCC: current state is an exit-state");
				isExitState = true;
				// if the local reach for the connected entry state has not been computed, yet, insert it in the 
				// entryLocalStatesMap
				if( entryLocalStatesMap.find(tsIt->X2) == entryLocalStatesMap.end() ){
					entryLocalStatesMap[tsIt->X2] = StateSet();
					LocalAccessibleReach(rGen,rHighAlph, tsIt->X2, entryLocalStatesMap[tsIt->X2]);
				}
			}
		}
		// if the current state is an exit state, compute the backward local reach with the controllability properties of the 
		// paths to locally backward reachable states
		if(isExitState == true){
			StateSet doneStates;
			exitLocalStatesMap[*stIt][*stIt] = false; // the exit state is reachable from the exit state via an uncontrollable path
			doneStates.Insert(*stIt);
			backwardReachabilityObsLCC(tset_X2EvX1, rControllableEvents, rHighAlph, *stIt, *stIt, false, exitLocalStatesMap, doneStates);
			
		}
		
	}
	// the generator rGenDyn is constructed by connecting all exit and entry states with their local state sets
	map<Idx, map<Idx, bool> >::const_iterator elIt, elEndIt;
	elIt = exitLocalStatesMap.begin();
	elEndIt = exitLocalStatesMap.end();
	map<Idx,bool>::const_iterator lcIt, lcEndIt;
	StateSet::Iterator exIt, exEndIt;
	map<Idx,StateSet>::const_iterator enIt;

	for( ; elIt != elEndIt; elIt++){
		lcEndIt = elIt->second.end();
		// go over all entry states reachable from the current exit state (via all feasible high-level events)
		tsIt = rGen.TransRel().Begin(elIt->first);
		tsEndIt = rGen.TransRel().End(elIt->first);
		for( ; tsIt != tsEndIt; tsIt++){
		    OP_DF("calculateDynamicSystemObsLCC: insert transitions for the high-level event" << rGen.EventName(tsIt->Ev) << "[" << tsIt->Ev << "]");
			if(rHighAlph.Exists(tsIt->Ev) ){
				bool controllable = rControllableEvents.Exists(tsIt->Ev);
				enIt = entryLocalStatesMap.find(tsIt->X2);
				stEndIt = enIt->second.End();
				// iterate over all locally backward reachable states from current exit state
				for(lcIt = elIt->second.begin(); lcIt != lcEndIt; lcIt ++){
					// iterate over all locally forward reachable states from current entry state
					for( stIt = enIt->second.Begin(); stIt != stEndIt; stIt++){
						OP_DF("calculateDynamicSystemObsLCC: Transition added to resulting generator: " << 
       					rGenDyn.TStr(Transition(lcIt->first,tsIt->Ev,*stIt))); 
       				
                       			rGenDyn.SetTransition(lcIt->first,tsIt->Ev,*stIt); // insert a transition for each local state combination
						if( !(controllable || lcIt->second) ){ // insert an uclabel transition if the local path is uncontrollable
							OP_DF("calculateDynamicSystemObsLCC: cLabel-Transition added to resulting generator: " << 
                           				rGenDyn.TStr(Transition(lcIt->first,ucLabel,*stIt))); 

                           				rGenDyn.SetTransition(lcIt->first,ucLabel,*stIt);
						}
					}
				}
			}		
		}
	}
}

// backwardReachabilityObsLCC(crTransSetX2EvX1, rControllableEvents, rHighAlph, exitState, currentState, controllablePath, rExitLocalStatesMap, rDoneStates)
void backwardReachabilityObsLCC(const TransSetX2EvX1& rTransSetX2EvX1, const EventSet& rControllableEvents, const EventSet& rHighAlph, Idx exitState, Idx currentState, bool controllablePath, map<Idx, map<Idx, bool> >& rExitLocalStatesMap, StateSet& rDoneStates){
	OP_DF("backwardReachabilityObsOCC(rTransSetX2EvX1," << rControllableEvents.Name() << "," << rHighAlph.Name() << "," << exitState << "," << currentState << "," << controllablePath << ",rExitLocalStatesMap, rDoneStates)");
   // go along all backward transitions. Discard the goal state if it is reached via a high-level event or if it is in the rDoneStates and 
   // the controllability properties of the state do not change on the current path
   
   // helpers
	TransSetX2EvX1::Iterator tsIt, tsEndIt;
   tsIt = rTransSetX2EvX1.BeginByX2(currentState);
   tsEndIt = rTransSetX2EvX1.EndByX2(currentState);
	bool currentControllablePath;
	// we iterate over all backward transitions of the currentState to establish backward reachability
	for( ;tsIt != tsEndIt; tsIt++){
	    // states reachable via a high-level event are not in the local backward reach and the controllability property of the current exitState does not change
		if( !rHighAlph.Exists(tsIt->Ev) && tsIt->X1 != exitState){
			// if the state has not been visited, yet, the controllability of the current path are set in the rExitLocalStatesMap
          if( !rDoneStates.Exists(tsIt->X1) ){
				rDoneStates.Insert(tsIt->X1);
				// the path is uncontrollable if the current transition has an uncontrollable event or the path was already uncontrollable
				currentControllablePath = rControllableEvents.Exists(tsIt->Ev) || controllablePath;
				rExitLocalStatesMap[exitState][tsIt->X1] = currentControllablePath;
				// as the state has not been visited, yet, it is subject to a new backward reachability
               backwardReachabilityObsLCC(rTransSetX2EvX1, rControllableEvents, rHighAlph, exitState, tsIt->X1, currentControllablePath, rExitLocalStatesMap, rDoneStates);
			}
			else{ // for an existing state, the controllability value can change from controllable to uncontrollable (if 
				// a new uncontrollable path has been found). It is important to note, that the LCC condition implies that
				// if there is one uncontrollable path, then the state is flagged uncontrollable except for the case of the 
				// given exitState that is always uncontrollable
				currentControllablePath = rControllableEvents.Exists(tsIt->Ev) || controllablePath;
				if(rExitLocalStatesMap[exitState][tsIt->X1] != currentControllablePath && currentControllablePath == false){
					rExitLocalStatesMap[exitState][tsIt->X1] = currentControllablePath;
					// as the controllabiity attribute of the current state changed it is subject to a new backward reachability
                   backwardReachabilityObsLCC(rTransSetX2EvX1, rControllableEvents, rHighAlph, exitState, tsIt->X1, false, rExitLocalStatesMap, rDoneStates);
				}
			}
		}
	}
}*/

// relabel(rGenRelabel, rControllableEvents, rHighAlph, rMapStateToPartition, rMapChangedTransReverse, rMapChangedTrans, rMapRelabeledEvents)
bool relabel(Generator& rGenRelabel, EventSet& rControllableEvents, EventSet& rHighAlph, map<Idx,Idx>& rMapStateToPartition, map<Transition,Transition>& rMapChangedTransReverse, map<Transition,Idx>& rMapChangedTrans, map<Idx, EventSet>& rMapRelabeledEvents)
{

    OP_DF("relabel(" << rGenRelabel.Name() << "," << rControllableEvents.Name() << "," << rHighAlph.Name() << ", rMapStateToPartition, rMapChangedTransReverse, rMapChangedTrans, rMapRelabeledEvents)");

	// helpers

	// keep track of how transitions between cosets have been relabeled
	// first Idx: coset from where transition starts, second Idx: high-level event,
	// third Idx: other coset, where transition ends, forth Idx: new high-level event
	map<Idx, map<Idx, map<Idx, Idx> > > mapRelabel;
	
	// set of low-level events, that have not been relabeled
	set<Idx> notRelabeledLowEvents;
	// transition relation of the original version of rGenRelabel
	const TransSet& transSet = rGenRelabel.TransRel();  
	TransSet::Iterator tIt = transSet.Begin();
	TransSet::Iterator tItEnd = transSet.End();
	// vector that contains pairs of original and changed transitions
	vector<pair<TransSet::Iterator,Transition> > editTransSet;
	// prepare data structure for keeping track of how transitions were relabeled
	map<Idx,Idx>::iterator newPartIt = rMapStateToPartition.begin();
	map<Idx,Idx>::iterator newPartItEnd = rMapStateToPartition.end();
	for(; newPartIt != newPartItEnd; ++newPartIt)
		mapRelabel[newPartIt->second]=map<Idx, map <Idx, Idx> >();
	
	// algorithm: The relabeling according to the state partition as computed by the bisimulation algorithm is
	// performed. A high-level transitions is relebaled with a new event, if a transition with the same event leads
	// from the same coset to a different coset (non-determinism in the projected automaton). A low-level transition
	// is relabeled if it connects different cosets (corresponds to unobservable transition betweeen cosets). The 
	// also has build-in procedures to take care that as few new event labels as possible are introduced. 
	
	// iteration over all transitions of rGenRelabel
	for(; tIt != tItEnd; ++tIt)
	{
		OP_DF("relabel: current transition: " << rGenRelabel.TStr(*tIt) );
		Idx indexX1Partition=rMapStateToPartition[tIt->X1];
		OP_DF("relabel: X1 belongs to coset with index " << indexX1Partition);
		Idx indexX2Partition=rMapStateToPartition[tIt->X2];
		OP_DF("relabel: X2 belongs to coset with index " << indexX2Partition);
	
		//check if current transition is labeled with a high-level-event
		if(rHighAlph.Exists(tIt->Ev))
		{
			OP_DF("relabel: Event is high-level event");
			// In the first case, there exists no entry for this transition in mapRelabel. Hence, this is the
			// first occurence of tIt->Ev leaving the coset of tIt->X1 found in the iteration.
			// Thus, there is no need to relabel this event. Store transition in mapRelabel
			if(mapRelabel[indexX1Partition].find(tIt->Ev) == mapRelabel[indexX1Partition].end())
			{
				mapRelabel[indexX1Partition][tIt->Ev][indexX2Partition]=tIt->Ev;
				OP_DF("relabel: First occurence of the current event leaving the current X1-coset");
			}
			// Otherwise, there exists an entry for the current event leaving the current X1-coset. It has to be
			// verified if the transition goes to the same coset as the previously found transitions or not.
			else 
			{
				// check if a transition with the same event has already been found that enters the same X2-coset. If
				// yes, the same label as for this transition is chosen. If not, a new label is introduced. 
				if(mapRelabel[indexX1Partition][tIt->Ev].find(indexX2Partition) != mapRelabel[indexX1Partition][tIt->Ev].end())
				{
					if(mapRelabel[indexX1Partition][tIt->Ev][indexX2Partition] != tIt->Ev)
					{
						OP_DF("relabel: the same event leading to the same X2-coset has already been relabeld before. The current transition is relabeld correspondingly");
						pair<TransSet::Iterator,Transition> newPair;
						newPair.first=tIt;
						newPair.second = Transition(tIt->X1,mapRelabel[indexX1Partition][tIt->Ev][indexX2Partition],tIt->X2);
						editTransSet.push_back(newPair);
					}
					else //nothing to be done
						OP_DF("relabel: There already exists a high-level transition from the current X1-coset to the same X2-coset and the current transition ist labeled with the same event. No relabeling necessary");
				}
				// there are entries of tIt->Ev, but no one leads to the same X2-coset. 
				// Check if new labels created before can be reused. If not, create a new one 
				// and eliminate the non-determinism by relebeling the current transition tIt with this event
				else 
				{
					bool createNewLabel = false;
					// check if the event already has been relabeled for some other transition
					if(rMapRelabeledEvents.find(tIt->Ev) != rMapRelabeledEvents.end())
					{
						EventSet::Iterator lsIt    = rMapRelabeledEvents[tIt->Ev].Begin();
						EventSet::Iterator lsItEnd = rMapRelabeledEvents[tIt->Ev].End();
						for(; lsIt != lsItEnd; ++lsIt)
						{
							createNewLabel = false;
							map<Idx,Idx>::iterator mapIt    = mapRelabel[indexX1Partition][tIt->Ev].begin();
							map<Idx,Idx>::iterator mapItEnd = mapRelabel[indexX1Partition][tIt->Ev].end();
							for(; mapIt != mapItEnd; ++mapIt)
							{	
								// if the currently investigated label has already been used in 
								// the current coset for a transition to another coset, relabeling is necessary
								if(mapIt->second == *lsIt)
								{
									createNewLabel = true;
									break;
								}
							}
							// use an old label if possible
							if(createNewLabel == false)
							{
								pair<TransSet::Iterator,Transition> newPair;
								newPair.first=tIt;
								newPair.second = Transition(tIt->X1,*lsIt,tIt->X2);
								editTransSet.push_back(newPair);
								OP_DF("relabel: An event was found that can be reused: " << rGenRelabel.EventName(*lsIt));
								break;
							}
						}
					}
					// if no useable labels are available, a new label has to be introduced
					else
					{
						createNewLabel = true;
					}
					// no label could be found that can be reused
					if(createNewLabel == true)
					{
						// create a new label and relabel the current transition with it
						std::string eventName = ( rGenRelabel.EventSymbolTablep())->UniqueSymbol(rGenRelabel.EventSymbolTablep()->Symbol(tIt->Ev) + "newHLevent_1");
						Idx newLabel = (rGenRelabel.EventSymbolTablep())->InsEntry(eventName);
				
						rGenRelabel.InsEvent(eventName);
						// if the original is controllable, the attribute of the new event is also controllable
						if(rControllableEvents.Exists(tIt->Ev)){ 
							rControllableEvents.Insert(newLabel);
						}
						OP_DF("relabel: No event that can be reused could be found. The new event " << rGenRelabel.EventName(newLabel) << " was created");
						rHighAlph.Insert(newLabel);
						//create the new transition and remember that the old one has to be replaced with the new one
						pair<TransSet::Iterator,Transition> newPair;
						newPair.first=tIt;
						newPair.second = Transition(tIt->X1,newLabel,tIt->X2);
						editTransSet.push_back(newPair);
						rMapRelabeledEvents[tIt->Ev].Insert(newLabel);
					}
				}
			}
		}
		// the current transition is labeled with a low-level event
		else
		{
			OP_DF("relabel: Event is low-level event");
			// potential relabeling is only necessary if the low-level transition leaves its coset
			if(indexX1Partition != indexX2Partition)
			{
				// transition connects two cosets and it is the first occurrence of the event. 
				// Create a new event and relabel the transition with it                                
				if(rMapRelabeledEvents.find(tIt->Ev) == rMapRelabeledEvents.end())
				{
					std::string eventName = ( rGenRelabel.EventSymbolTablep())->UniqueSymbol(rGenRelabel.EventSymbolTablep()->Symbol(tIt->Ev) + "newHLevent_1");
					Idx newLabel = (rGenRelabel.EventSymbolTablep())->InsEntry(eventName);
					rGenRelabel.InsEvent(eventName);
					// insert the new event in the high-level alphabet and set its controllability property
					rHighAlph.Insert(newLabel);
					if(rControllableEvents.Exists(tIt->Ev)){ 
					rControllableEvents.Insert(newLabel);
					}
					OP_DF("relabel: First occurence of current low-level event " << rGenRelabel.EventName(tIt->Ev) 
					<< " between cosets. The new event " << rGenRelabel.EventName(newLabel) << " was created");
					mapRelabel[indexX1Partition][tIt->Ev][indexX2Partition]=newLabel;
					rMapRelabeledEvents[tIt->Ev].Insert(newLabel);
					pair<TransSet::Iterator,Transition> newPair;
					newPair.first=tIt;
					newPair.second  = Transition(tIt->X1,newLabel,tIt->X2);
					editTransSet.push_back(newPair);
				}
				// there are entries of tIt->Ev, but no one leads to the same X2-coset. 
				// Check if new labels created before can be reused. If not, create a new one 
				// and eliminate the non-determinism by relebeling the current transition tIt with this event
				else 
				{
					EventSet::Iterator lsIt    = rMapRelabeledEvents[tIt->Ev].Begin();
					EventSet::Iterator lsItEnd = rMapRelabeledEvents[tIt->Ev].End();
					bool createNewLabel = true;
					for(; lsIt != lsItEnd; ++lsIt)
					{
						bool labelFound = false;
						// check if the event already has been relabeled for some other transition
						if(mapRelabel.find(indexX1Partition) == mapRelabel.end())
						{
							// if the currently investigated label has already been used in 
							// the current coset for a transition to another coset, relabeling is necessary
							labelFound = true;
						}
						else
						{
							// label lsIt can be reused as this is the first low-level transition leaving the X1-coset
							if(mapRelabel[indexX1Partition].find(tIt->Ev) == mapRelabel[indexX1Partition].end())
							{
								labelFound = true;
							}
							else
							{
								// a transition with the same original event tIt->Ev leaving the X1-coset has been found before
								// and it entered a different X2-coset. Check if one of them was relabeled with lsIt. If yes, then lsIt can
								// not be reused and the next event has to be examined. If no, lsIt can be reused 
								if(mapRelabel[indexX1Partition][tIt->Ev].find(indexX2Partition) == mapRelabel[indexX1Partition][tIt->Ev].end())
								{
									map<Idx,Idx>::iterator mapIt=mapRelabel[indexX1Partition][tIt->Ev].begin();
									map<Idx,Idx>::iterator mapItEnd=mapRelabel[indexX1Partition][tIt->Ev].end();
									labelFound = true;
									for(; mapIt != mapItEnd; ++ mapIt)
									{
										if(mapIt->second== *lsIt)
										{
											labelFound = false;
											break;
										}
									}
								} 
								// a transition with the same original event tIt-Ev leaving the X1-coset has been found before
								// and it entered the same X2-coset. Reuse the corresponding label
								else
								{
									pair<TransSet::Iterator,Transition> newPair;
									newPair.first=tIt;
									newPair.second = Transition(tIt->X1,mapRelabel[indexX1Partition][tIt->Ev][indexX2Partition],tIt->X2);
									editTransSet.push_back(newPair);
									OP_DF("relabel: A transition to same X2-coset has already been found and relabeled before; the current transition is also labeled with " << rGenRelabel.EventName(mapRelabel[indexX1Partition][tIt->Ev][indexX2Partition]));
									createNewLabel = false;
									break;
								}
							}
						}
						// reuse existing event
						if(labelFound == true)
						{
							pair<TransSet::Iterator,Transition> newPair;
							newPair.first=tIt;
							newPair.second = Transition(tIt->X1,*lsIt,tIt->X2);
							editTransSet.push_back(newPair);
							OP_DF("relabele: Low level event "  << rGenRelabel.EventName(tIt->Ev) << " is relabeled with " << rGenRelabel.EventName(*lsIt) << " which can be reused");
							mapRelabel[indexX1Partition][tIt->Ev][indexX2Partition]=*lsIt;
							createNewLabel = false;
							break;
						}
					} //end for
					// a new label has to be created and the corresponding transition is inserted
					if(createNewLabel == true)
					{
						std::string eventName = ( rGenRelabel.EventSymbolTablep())->UniqueSymbol(rGenRelabel.EventSymbolTablep()->Symbol(tIt->Ev) + "newHLevent_1"); //??
						Idx newLabel = (rGenRelabel.EventSymbolTablep())->InsEntry(eventName);
						rGenRelabel.InsEvent(eventName);
						// insert the new event in the high-level alphabet and set its controllability property
						rHighAlph.Insert(newLabel);
						if(rControllableEvents.Exists(tIt->Ev) ){ 
							rControllableEvents.Insert(newLabel);
						}
						OP_DF("relabel: No label could be reused, and the new event " << rGenRelabel.EventName(newLabel) << " was created");
						mapRelabel[indexX1Partition][tIt->Ev][indexX2Partition]=newLabel;
						pair<TransSet::Iterator,Transition> newPair;
						newPair.first=tIt;
						newPair.second = Transition(tIt->X1,newLabel,tIt->X2);
						editTransSet.push_back(newPair);
						rMapRelabeledEvents[tIt->Ev].Insert(newLabel);
					}
				}
			}
			// the low-level event does not have to be relabeled 
			else //indexX1Partition==indexX2Partition
			{
				notRelabeledLowEvents.insert(tIt->Ev);
			}
		}
	}
		
		
	// It is possible that events need not be relabeled. For example, if all occurrences of a low-level event have to 
	// be relabeled and added to the high-level alphabet, it is possible to just add the original low-level event
	// to the high-level alphabet without any relabeling. Note that this information is only available after all 
	// transitions have been evaluated.
	vector<pair<TransSet::Iterator,Transition> >::iterator etSetIt    = editTransSet.begin();
	vector<pair<TransSet::Iterator,Transition> >::iterator etSetItEnd = editTransSet.end();
	set<Idx> insertHighEvents;
	set<Idx> deleteHighEvents;
	bool quotAuotTrue=true;
	
	OP_DF("relabel: Trying to avoid unnecessariy new labels");
	// All relabeled transitions are checked
	for(etSetIt = editTransSet.begin(); etSetIt != editTransSet.end(); ++etSetIt)
	{
		Idx oldEvent = etSetIt->first->Ev;
		Idx newEvent = etSetIt->second.Ev;
		OP_DF("relabel: Checking transition X1="  << rGenRelabel.StateName(etSetIt->first->X1)<< " ["<<etSetIt->first->X1
		<< "] Ev=" << rGenRelabel.EventName(oldEvent) << " X2="  << rGenRelabel.StateName(etSetIt->first->X2)
		<< " [" << etSetIt->first->X2 << "] which shall be relabeled with event " << rGenRelabel.EventName(newEvent));
		// check if the original event is a low-level event and if there is an event left that has not been relabeled. If all events are relabeled, then at least one label can be replaced by the original event label.
		if(notRelabeledLowEvents.find(oldEvent) == notRelabeledLowEvents.end() && !rHighAlph.Exists(oldEvent))
		{
			OP_DF("relabel: There is no low-level event " << rGenRelabel.EventName(oldEvent) << " left");
			insertHighEvents.insert(oldEvent);
			// if a low-level event has been relabeled, the automaton under investigation is not the desired quotient automaton, yet.
			quotAuotTrue=false;
			// if newEvent is the first new event created for relabeling oldEvent, the relabeling is discarded
			if(rMapRelabeledEvents[oldEvent].Find(newEvent) == rMapRelabeledEvents[oldEvent].Begin())
			{
				OP_DF("relabel: Transition will not be relabeled");
				// if newEvent is the only event created for relabeling oldEvent, delete newEvent from rGenRelabel
				if(rMapRelabeledEvents[oldEvent].Find(newEvent)==(--(rMapRelabeledEvents[oldEvent].End())))
				{
					OP_DF("relabel: newEvent is the first and last event in rMapRelabeledEvents[oldEvent]");
					deleteHighEvents.insert(newEvent);
				}
				// delete pair<original Transition, new Transition>
				vector<pair<TransSet::Iterator,Transition> >::iterator helpIt = etSetIt;
				helpIt--;
				editTransSet.erase(etSetIt);
				etSetIt = helpIt;
			}
			else
			{
				// find predecessor of newEvent in map rMapRelabeledEvents[oldEvent] and use that event for relabeling
				Idx newLabel = *(--(rMapRelabeledEvents[oldEvent].Find(newEvent)));
				etSetIt->second.Ev = newLabel;
				// delete newEvent from rGenRelabel if it is no longer needed
				if(rMapRelabeledEvents[oldEvent].Find(newEvent)==(--(rMapRelabeledEvents[oldEvent].End())))
				{
					deleteHighEvents.insert(newEvent);
				}
			}
		}
	}
	// update the rMapRelabeledEvents with the changes
	// as the relabeling of oldEvent is discarded, it is removed from the map of relebeled events and
	// reinserted into the map of not relabeled events

	// mofidy alphabet of rGenRelabel and the abstraction alphabet by inserting the new high-level events and deleting the not needed new event labels.
	set<Idx>::iterator setIt    = insertHighEvents.begin();
	set<Idx>::iterator setItEnd = insertHighEvents.end();
	for(; setIt!= setItEnd; ++ setIt)
	{
		rHighAlph.Insert(*setIt);
	}
	setIt = deleteHighEvents.begin();
	setItEnd = deleteHighEvents.end();
	for(; setIt != setItEnd; ++setIt)
	{
		rGenRelabel.DelEvent(*setIt);
		rHighAlph.Erase(*setIt);
		rControllableEvents.Erase(*setIt); // controllable event is erased if it does not exist anymore. Schmidt 10/07
	}
		
	rControllableEvents = rControllableEvents * rGenRelabel.Alphabet(); // Schmidt 10/07
	// check if quotient automaton is deterministic and free of unobservable transitions, i.e., no transitions are relabeled and no low-level events are declared as high-level events
	if(editTransSet.empty()&& quotAuotTrue== true)
	{
		OP_DF("relabel: Leaving function relabel with true");
		return true; 
	}
		
	// delete original trnasitions and create relabeled transitions in rGenRelabel
	etSetIt = editTransSet.begin();
	etSetItEnd = editTransSet.end();
	for(; etSetIt != etSetItEnd; ++etSetIt)
	{
		map<Transition,Transition>::iterator mrIt;
		mrIt=rMapChangedTransReverse.find(*(etSetIt->first));
		// if the current relabeled transition has already been relabeled in a previous step of the observer algorithm, the original transition is found in the rMapChangedTransReverse map.
		if(mrIt!=rMapChangedTransReverse.end())
		{
			Transition originalTrans = mrIt->second;
			(rMapChangedTrans.find(originalTrans))->second = (etSetIt->second).Ev;
			rMapChangedTransReverse.erase(mrIt);
			rMapChangedTransReverse[etSetIt->second]=originalTrans;
			OP_DF("relabel: The transition X1= " << rGenRelabel.SStr((etSetIt->first)->X1) << " Ev=" << rGenRelabel.EStr((etSetIt->first)->Ev) << " X2= " << rGenRelabel.SStr((etSetIt->first)->X2) << " has already been relabeled in a former iteration step. The original transition was " << rGenRelabel.SStr(originalTrans.X1) << " Ev=" << rGenRelabel.EStr(originalTrans.Ev) << " X2= " << rGenRelabel.SStr(originalTrans.X2) << "; the new label is " << rGenRelabel.EStr((etSetIt->second).Ev));
		}
		// the current relabeled transition is simply inserted in the rMapChangedTrans map with its original transition if it is the first relabeling.
		else
		{
			rMapChangedTransReverse[etSetIt->second]=*(etSetIt->first);
			rMapChangedTrans[*(etSetIt->first)]=(etSetIt->second).Ev;
			OP_DF("relabel: First relabeling of transition X1=" << rGenRelabel.SStr((etSetIt->first)->X1) 
			<< " Ev=" << rGenRelabel.EStr((etSetIt->first)->Ev) << " X2= " << rGenRelabel.SStr((etSetIt->first)->X2) << " new label is " << rGenRelabel.EStr((etSetIt->second).Ev));
		}
		// The old transition is removed and the new transition is inserted into rGenRelabel
		rGenRelabel.ClrTransition(etSetIt->first);
		rGenRelabel.SetTransition(etSetIt->second);
	}
	
	OP_DF("relabel: leaving function with false");
	return false;
}

// insertRelabeledEvents(rGenPlant, rMapRelabeledEvents, rNewEvents) 
void insertRelabeledEvents(System& rGenPlant, const map<Idx,set<Idx> >&  rMapRelabeledEvents, Alphabet& rNewEvents) {
	map<Idx,set<Idx> >::const_iterator reEndIt = rMapRelabeledEvents.end();
	TransSet::Iterator tsEndIt = rGenPlant.TransRelEnd();
	TransSet::Iterator tsIt = rGenPlant.TransRelBegin();
	// check all transitions of rGenPlant
	for(; tsIt != tsEndIt; tsIt++){
		map<Idx,set<Idx> >::const_iterator reIt = rMapRelabeledEvents.find(tsIt->Ev);
		if(reIt == reEndIt) continue;
		AttributeCFlags attr = rGenPlant.EventAttribute(tsIt->Ev); 
		set<Idx>::const_iterator rsEndIt = reIt->second.end();
		set<Idx>::const_iterator rsIt = reIt->second.begin();
		// if a transition with an event that gets new labels has been found, parallel transitions 
                // with the new events are added to rGenPlant
		for(; rsIt != rsEndIt; rsIt++){
			rGenPlant.InsEvent(*rsIt,attr);
			rNewEvents.Insert(*rsIt,attr); 
			rGenPlant.SetTransition(tsIt->X1,*rsIt,tsIt->X2);
		}
	}
}

// insertRelabeledEvents(rGenPlant, rMapRelabeledEvents, rNewEvents) 
// tm: alternative, smart on attributes
void insertRelabeledEvents(Generator& rGenPlant, const map<Idx,set<Idx> >&  rMapRelabeledEvents, EventSet& rNewEvents) {
	map<Idx,set<Idx> >::const_iterator reEndIt = rMapRelabeledEvents.end();
	TransSet::Iterator tsEndIt = rGenPlant.TransRelEnd();
	TransSet::Iterator tsIt = rGenPlant.TransRelBegin();
	// check all transitions of rGenPlant
	for(; tsIt != tsEndIt; tsIt++){
		map<Idx,set<Idx> >::const_iterator reIt = rMapRelabeledEvents.find(tsIt->Ev);
		if(reIt == reEndIt) continue;
		AttributeVoid* attrp = rGenPlant.EventAttribute(tsIt->Ev).Copy(); 
		set<Idx>::const_iterator rsEndIt = reIt->second.end();
		set<Idx>::const_iterator rsIt = reIt->second.begin();
		// if a transition with an event that gets new labels has been found, parallel transitions 
                // with the new events are added to rGenPlant
		for(; rsIt != rsEndIt; rsIt++){
		        rGenPlant.InsEvent(*rsIt);
		        rGenPlant.EventAttribute(*rsIt,*attrp);
			rNewEvents.Insert(*rsIt); 
			rNewEvents.Attribute(*rsIt,*attrp); 
			rGenPlant.SetTransition(tsIt->X1,*rsIt,tsIt->X2);
                        // should also copy transition attribute
		}
                delete attrp;
	}
}

// insertRelabeledEvents(rGenPlant, rMapRelabeledEvents)
void insertRelabeledEvents(System& rGenPlant, const map<Idx,set<Idx> >&  rMapRelabeledEvents) {
  Alphabet dummy;
  insertRelabeledEvents(rGenPlant,rMapRelabeledEvents,dummy);
}

// insertRelabeledEvents(rGenPlant, rMapRelabeledEvents)
void insertRelabeledEvents(Generator& rGenPlant, const map<Idx,set<Idx> >&  rMapRelabeledEvents) {
  EventSet dummy;
  insertRelabeledEvents(rGenPlant,rMapRelabeledEvents,dummy);
}




/** 
 * Rti convenience wrapper
 */   
void calcAbstAlphObs(
  System& rGenObs, 
  EventSet& rHighAlph, 
  EventSet& rNewHighAlph, 
  EventRelabelMap&  rMapRelabeledEvents) 
{
  calcAbstAlphObs(rGenObs, rHighAlph, rNewHighAlph, rMapRelabeledEvents.StlMap());
}


/** 
 * Rti convenience wrapper
 */   
void insertRelabeledEvents(Generator& rGenPlant, const EventRelabelMap& rMapRelabeledEvents, EventSet& rNewEvents) {
  insertRelabeledEvents(rGenPlant, rMapRelabeledEvents.StlMap(), rNewEvents); 
}

/** 
 * Rti convenience wrapper
 */   
void insertRelabeledEvents(Generator& rGenPlant, const EventRelabelMap& rMapRelabeledEvents) {
  insertRelabeledEvents(rGenPlant, rMapRelabeledEvents.StlMap()); 
}

/**
 * Rti wrapper class implementation
 */

// std faudes type
FAUDES_TYPE_IMPLEMENTATION(EventRelabelMap,EventRelabelMap,Type)

// construct/destruct
EventRelabelMap::EventRelabelMap(void) : Type() {}
EventRelabelMap::EventRelabelMap(const EventRelabelMap& rOther) : Type() {mMap=rOther.mMap; }
EventRelabelMap::~EventRelabelMap(void) {}

// clear
void EventRelabelMap::Clear(void) { mMap.clear(); }

// assignment/equality
void EventRelabelMap::DoAssign(const EventRelabelMap& rSrc) {mMap=rSrc.mMap;}
bool EventRelabelMap::DoEqual(const EventRelabelMap& rOther) const { return mMap==rOther.mMap;}

// access
const std::map<Idx, std::set<Idx> >& EventRelabelMap::StlMap(void) const { return mMap;}
std::map<Idx, std::set<Idx> >& EventRelabelMap::StlMap(void) { return mMap;}
void EventRelabelMap::StlMap(const std::map<Idx, std::set<Idx> >& rMap) { mMap=rMap;}



}// namespace

