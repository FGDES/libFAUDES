/** @file mtc_observercomputation.cpp

Methods to compute natural projections that exhibit the obsrver property.
The observer algorithm is elaborated in
K. C. Wong and W. M. Wonham, "On the Computation of Observers in Discrete Event
Systems," Discrete Event Dynamic Systems, vol. 14, no. 1, pp. 55-107, 2004. 
In addition, methods to compute natural projections that exhibit
output control consistency (OCC) and local control consistency (LCC) are provided. See for example
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

#include "mtc_observercomputation.h"
#include "cfl_localgen.h"

using  namespace std;

namespace faudes{


// calcNaturalObserver(rGen,rHighAlph)
Idx calcNaturalObserver(const MtcSystem& rGen, EventSet& rHighAlph){
    // helpers
    Generator dynGen;
    map<Idx,Idx> mapStateToPartition;
    EventSet origAlph;
    Generator genPart;
    while(origAlph != rHighAlph){
        origAlph  = rHighAlph;
        // compute the dynamic system for the given generator and high-level alphabet
        calculateDynamicSystemObs(rGen, rHighAlph, dynGen);
        // compute the quasi conqruence 
        ComputeBisimulation(dynGen, mapStateToPartition, genPart);
        // Extend the high-level alphabet according to the algorithm of Lei
        ExtendHighAlphabet(rGen, rHighAlph, mapStateToPartition);
    }
    return genPart.Size();
}

// calcAbstAlphObs(rGenObs, rHighAlph, rNewHighAlph, rMapRelabeledEvents)
void calcAbstAlphObs(MtcSystem& rGenObs, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Idx,set<Idx > > &  rMapRelabeledEvents) {
	OP_DF("calcAbstAlphObs(" << rGenObs.Name() << "," << "rHighAlph, rNewHighAlph, rMapRelabeledEvents)");
	// The controllable events are separated from the System. All functions that are successively 
	// called, are defined for Generators
	EventSet cntrevs = rGenObs.ControllableEvents();
	calcAbstAlphObs(rGenObs, cntrevs , rHighAlph, rNewHighAlph, rMapRelabeledEvents);
	// the controllable events that have been changed by the called function are set in the System cGenObs
	rGenObs.SetControllable(cntrevs);
}

// calcAbstAlphObs(rGenObs, rControllableEvents, rHighAlph, rNewHighAlph, rMapRelabeledEvents)
void calcAbstAlphObs(MtcSystem& rGenObs, EventSet& rControllableEvents, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Idx,set<Idx > > &  rMapRelabeledEvents) {
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
void calcAbstAlphObs(MtcSystem& rGenObs, EventSet& rControllableEvents, EventSet& rHighAlph, EventSet& rNewHighAlph, map<Transition,Idx>& rMapChangedTrans)
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
		// compute dynamic system for Lm-observer on rGenObs
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

// calculateDynamicSystemObs(rGen, rHighAlph, rGenDyn)
void calculateDynamicSystemObs(const MtcSystem& rGen, EventSet& rHighAlph, Generator& rGenDyn)
{
	OP_DF("calculateDynamicSystemObs(" << rGen.Name() << "," << rHighAlph.Name() << "," << rGenDyn.Name() << ")");
	// transition relation sorted in reverse order for backwards reachability
	TransSetX2EvX1 tset_X2EvX1;
	rGen.TransRel(tset_X2EvX1);
	
	// prepare generator rGenDyn
	rGenDyn.ClearTransRel();
	rGenDyn.InjectAlphabet(rHighAlph); // all high-level events are contained in the alphabet of rGenDyn
	
	// helpers
	EventSet::Iterator eIt = rGenDyn.AlphabetBegin();
	EventSet::Iterator eItEnd = rGenDyn.AlphabetEnd();
	
	TransSetX2EvX1::Iterator tItByX2Ev;
	TransSetX2EvX1::Iterator tItByX2EvEnd;
	StateSet reach;
	StateSet::Iterator sIt;
	StateSet::Iterator sItEnd;
	TransSet::Iterator tIt;
	TransSet::Iterator tItEnd;
	map<Idx,StateSet> entryStateToLocalReach;
	set<Idx>::iterator esIt, esItEnd;
	// map that maps a state (first Idx) to a reachable entry state (second Idx) via a high-level event (third Idx)
	map<Idx,vector<pair<Idx,Idx> > > stateToEntryState;
	map<Idx,vector<pair<Idx,Idx> > >::iterator stesIt;
	map<Idx,vector<pair<Idx,Idx> > >::iterator stesItEnd;
	
	// generate the color labels for the dynamic system
	ColorSet allColors = rGen.Colors();
	ColorSet::Iterator csIt, csEndIt;
	csIt = allColors.Begin();
	csEndIt = allColors.End();
	std::string colorLabel;
	// maps a unique color index to the index of the associated transition label in teh dynamic system
	map<Idx,Idx> colorEventMap; 
	for(; csIt != csEndIt; csIt++){
		colorLabel = ( rGenDyn.EventSymbolTablep())->UniqueSymbol("colorLabel" + rGen.ColorName(*csIt) + "_1");
		colorEventMap[*csIt] = (rGenDyn.EventSymbolTablep())->InsEntry(colorLabel);	
	}
	
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
		
		// check if current state (*stateSetIt) is an entry-state (a backward transition with
		// a high-level event exists. If yes, the locally reachable states (reach) are stored in
		// the entryStateToLocalReach map
		tItByX2Ev=tset_X2EvX1.BeginByX2(*stateSetIt); 
		tItByX2EvEnd=tset_X2EvX1.EndByX2(*stateSetIt); 
		for(; tItByX2Ev != tItByX2EvEnd; ++tItByX2Ev)
		{
			OP_DF("calculateDynamicSystemObs: checking transition : " << rGen.TStr(*tItByX2Ev));
			if(rHighAlph.Exists(tItByX2Ev->Ev)){
			    OP_DF("calculateDynamicSystemObs: current state is an entry-state");
    			// map entry-state to its locally reachable states
    			entryStateToLocalReach[*stateSetIt]=reach;
    			break;
			}
		}
		vector<pair<Idx,Idx> > emptyVector;
		stateToEntryState[*stateSetIt]=emptyVector;
		// loop over all states in the local reach of current state to determine which marked states
		// are locally reachable and to determine which high-level events can occur after local strings
		sIt=reach.Begin();
		sItEnd=reach.End();
		Idx colorEvent;
		for( ; sIt != sItEnd; ++sIt) 
		{
			// check if state sIt is marked; if yes, create an m-transition (mLabel) between current state and sIt
			const ColorSet& stateColors = rGen.Colors(*sIt);
			csIt = stateColors.Begin();
			csEndIt = stateColors.End();
			for( ; csIt != csEndIt; csIt++){
			    OP_DF("calculateDynamicSystemObs: marked state " << rGen.SStr(*sIt) 
    			<<  " is locally reachable from current state");
			colorEvent = colorEventMap[*csIt];
    			rGenDyn.InsEvent(colorEvent ); // ??
    			rGenDyn.InsState(*stateSetIt); // ??
    			rGenDyn.InsState(*sIt); //??
    			rGenDyn.SetTransition(*stateSetIt, colorEvent, *sIt);
			}
			// loop over all transitions of current state sIt to determine if high-level events are possible
			tIt    = rGen.TransRelBegin(*sIt);
			tItEnd = rGen.TransRelEnd(*sIt);
			for(; tIt != tItEnd; ++tIt)
			{
			    OP_DF("calculateDynamicSystemObs: Loop over all states in reach; checking transition: " 
    				<< rGen.TStr(*tIt));
    			if(rHighAlph.Exists(tIt->Ev))
    			{
    				OP_DF("calculateDynamicSystemObs: state " << rGen.SStr(tIt->X2) << 
    				" is an entry state and can be reached from current state" << rGen.SStr(*stateSetIt) << " by event " << rGen.EventName(tIt->Ev));
    				pair<Idx,Idx> newPair;
    				newPair.first=tIt->X2;
    				newPair.second=tIt->Ev;
    				// store the reachable entry state and the corresponding high-level event
    				stateToEntryState[*stateSetIt].push_back(newPair);
    			}
			}
		}
	}
	
	// create the transition structure of the dynamic system
	stesIt    = stateToEntryState.begin();
	stesItEnd = stateToEntryState.end();
	vector<pair <Idx,Idx> >* pPairs;
	vector<pair <Idx,Idx> >::iterator vIt;
	vector<pair <Idx,Idx> >::iterator vItEnd;
	// To construct the dynamic system, each local state has to be connected to all states in
	// the local accessible reach of entry states that can be reached via a high-level event.
	// This information is contained in the stateToEntryState map combined with the entryStateToLocalReach map.
	// iteration over all entries (X1) of map stateToEntryState
	for(; stesIt != stesItEnd; ++stesIt)
	{
    	pPairs=&(stesIt->second);
    	vIt    = (*pPairs).begin();
    	vItEnd = (*pPairs).end();
    	// loop over all pairs (Ev,X2) of current entry
    	for( ; vIt != vItEnd; ++vIt)
    	{
    		// check if transition already exists
    		if(!((rGenDyn.TransRel()).Exists(stesIt->first,vIt->second,vIt->first))) 
    		{
        		// find local reach of entry state X2 
        		StateSet* pLocalReach = &((entryStateToLocalReach.find(vIt->first))->second);
        	
        		// Add a Ev-transition from X1 to every state in the local reach of X2     
        		StateSet::Iterator lrsIt    = pLocalReach->Begin();
        		StateSet::Iterator lrsItEnd = pLocalReach->End();
        		for(; lrsIt != lrsItEnd; ++lrsIt)
        		{
        			rGenDyn.InsEvent(vIt->second); // ??
        			rGenDyn.InsState(stesIt->first); // ??
        			rGenDyn.InsState(*lrsIt); //??
        			rGenDyn.SetTransition(stesIt->first,vIt->second,*lrsIt);
        			OP_DF("calculateDynamicSystemObs: Transition added to resulting generator: " << 
        				rGenDyn.TStr(Transition(stesIt->first,vIt->second,*lrsIt))); 
        		}
    		}
    	}
	}
	OP_DF("calculateDynamicSystemObs: leaving function");
}
 
}
