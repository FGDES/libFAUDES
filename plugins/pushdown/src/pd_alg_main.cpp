/** @file pd_alg_main.cpp  Top-Level functions*/


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013/14 Ramon Barakat, Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/


#include "pd_alg_main.h"

namespace faudes {

//ConstructController(rSpec, rPlant, rResPd)
void PushdownConstructController(const PushdownGenerator& rSpec, const System& rPlant,
		PushdownGenerator& rResPd, bool debug) {
	FUNCTION(__FUNCTION__);	//scope log
	FD_DF("ConstructController(" << &rSpec << "," << &rPlant << ")");

	// warn if rPlant or rSpec isn't deterministic
	if (!rPlant.IsDeterministic())
		FD_WARN("The given system automaton (plant) is not deterministic ! "
				"May use Deterministic(const Generator&,Generator& rResGen) ");

	if (!rSpec.IsDeterministic())
		FD_WARN("The given pushdown automaton (spec) is not deterministic !");

	// prepare result
	PushdownGenerator* pResPd = &rResPd;
	if (&rResPd == &rSpec) {
		pResPd = rResPd.New();
	}

	DEBUG_PRINTF(debug, "Specification", rSpec);
	DEBUG_PRINTF(debug, "Plant", rPlant);

	//pre combine of specification and plant
	ConstructControllerPreCombine(rSpec, rPlant, *pResPd);
	DEBUG_PRINTF(debug, "PreComp", *pResPd);
	FD_DF("ConstructController: PreCombine done !");

	if(!pResPd->Empty()){
		//start algorithm loop
		//count loops for debugging
		int loopcounter =ConstructControllerLoop(*pResPd, rPlant, *pResPd, 0);
		DEBUG_PRINTF(debug, "Controller", *pResPd);
		FD_DF("ConstructController: Done after " + ToStringInteger(loopcounter) +" iterations !");
	}

	// copy result
	if (pResPd != &rResPd) {
		pResPd->Move(rResPd);
		delete pResPd;
	}
}

//ConstructControllerPreCombine(rSpec, rPlant, rResPd)
void ConstructControllerPreCombine(const PushdownGenerator& rSpec,
		const System& rPlant, PushdownGenerator& rResPd, bool debug) {
	FUNCTION(__FUNCTION__);	//scope log
	FD_DF("ConstructControllerPreCombine(" << &rSpec << "," << &rPlant << ")");

	//create product generator of plant and specification
	PushdownGenerator rPd = Times(rPlant, rSpec);
	DEBUG_PRINTF(debug, "PreTimes", rPd);
	FD_DF("ConstructControllerPreCombine: Times done !");

	//make product generator non-blocking
	PushdownBlockfree(rPd,rPd);
	DEBUG_PRINTF(debug, "PreBlockfree", rPd);
	FD_DF("ConstructControllerPreCombine: Blockfree done !");

	// return result
	rResPd = rPd;
}

//ConstructControllerLoop(rContr, rPlant, rResPd, loopcounter)
int ConstructControllerLoop(const PushdownGenerator& rContr,
		const System& rPlant, PushdownGenerator& rResPd, int loopcounter, bool debug) {
	FUNCTION(__FUNCTION__);		//scope log
	loopcounter++;				//count iterations

	//annotate iteration for debugging
	std::string iteration = ToStringInteger(loopcounter)+ "_Iteration";
	FD_DF("ConstructControllerLoop(" << &rContr << "," << &rPlant << ") " + iteration);

	DEBUG_PRINTF(debug, ("LoopControllerCandidate_" + iteration), rContr);

	//generator to return
	PushdownGenerator rPd;

	//create product generator
	rPd = Times(rPlant, rContr);
	DEBUG_PRINTF(debug, ("LoopTimes_" + iteration), rPd);

	//trim the generator
	PushdownTrim(rPd, FAUDES_PD_LOOKAHEAD);
	DEBUG_PRINTF(debug, ("LoopTimesTrim_" + iteration), rPd);

	//split states into heads and ears
	rPd = Split(rPd);
	DEBUG_PRINTF(debug, ("LoopSplit_" + iteration), rPd);
	FD_DF("ConstructControllerLoop: Split done !");

	//trim the generator
	PushdownTrim(rPd, FAUDES_PD_LOOKAHEAD);
	DEBUG_PRINTF(debug, ("LoopSplitTrim_" + iteration), rPd);

	//get accessible part
	PushdownAccessible(rPd, rPd);
	DEBUG_PRINTF(debug, ("LoopAC_" + iteration), rPd);

	if(rPd.Empty())
		rResPd = rPd;

	//save number of states
	uint numberOfStates = rPd.Size();

	//set lambda event controllable
	rPd.SetControllable(FAUDES_PD_LAMBDA);

	//remove non-controllable ears
	rPd = Rnce(rPd, rPlant);
	DEBUG_PRINTF(debug, ("LoopRnce_" + iteration), rPd);

	if (numberOfStates == rPd.Size()) {
		DEBUG_PRINTF(debug, ("LoopEnd_" + iteration), rContr);
		//if no states were removed
		// copy controller to result
		rResPd = PushdownGenerator(rContr);
		//return number of iterations
		return loopcounter;
	} else {
		DEBUG_PRINTF(debug, "numberOfStates != rPd.Size()", "");

		//make temporary controller non-blocking
		PushdownBlockfree(rPd,rPd);
		DEBUG_PRINTF(debug, "LoopBlockfree_" + iteration, rPd);

		//start next loop iteration
		return ConstructControllerLoop(rPd, rPlant, rResPd, loopcounter);
	}
}



/* *************************
 * PushdownBlockfree
 * *************************/
void PushdownBlockfree(const PushdownGenerator& pd, PushdownGenerator& resPd, bool debug) {
	FUNCTION(__FUNCTION__);	//scop log
	FD_DF("PushdownBlockfree(" << &pd << "," << &resPd << ")");

	//warning if pd isn't deterministic
	if (!pd.IsDeterministic())
		FD_WARN("PushdownBlockfree: The given pushdown automaton is not deterministic !");

	//take a copy of the old generator
	PushdownGenerator rPd(pd);
	DEBUG_PRINTF(debug,"BlockfreeInput",rPd);

	//trim input
	if(!PushdownTrim(rPd, FAUDES_PD_LOOKAHEAD)){
		//if no initial or no marked state exists, return empty generator
		resPd = rPd;
		return;
	}
	DEBUG_PRINTF(debug,"BlockfreeInputTrim",rPd);

	//create SDPDA
	rPd = SPDA(rPd);
	DEBUG_PRINTF(debug,"BlockfreeSDPDA",rPd);

	//trim SDPDA
	if(!PushdownTrim(rPd, FAUDES_PD_LOOKAHEAD)){
		//if no initial or no marked state exists, return empty generator
		resPd = rPd;
		return;
	}
	DEBUG_PRINTF(debug,"BlockfreeSDPDATrim",rPd);

	//remove double acceptance
	rPd = Nda(rPd);
	DEBUG_PRINTF(debug,"BlockfreeNda",rPd);

	//trim Nda
	if(!PushdownTrim(rPd, FAUDES_PD_LOOKAHEAD)){
		//if no initial or no marked state exists, return empty generator
		resPd = rPd;
		return;
	}
	DEBUG_PRINTF(debug,"BlockfreeNdaTrim",rPd);

	//convert to grammar
	Grammar gr = Sp2Lr(rPd,0);
	DEBUG_PRINTF(debug,"BlockfreeSp2Lr",gr.Str());

	//remove non-productive productions
	gr = Rnpp(gr);
	DEBUG_PRINTF(debug,"BlockfreeRnpp",gr.Str());

	//remove unreachable productions
	gr = Rup(gr);
	DEBUG_PRINTF(debug,"BlockfreeRup",gr.Str());

	//if no productions exists, return empty generator
	if(gr.GrammarProductions().empty()){
		resPd.Clear();
		return;
	}

	//augment grammar
	//create temporary automaton to create a new event for the augment terminal
	//and a new stack symbol for the augment nonterminal
	PushdownGenerator tempPd = rPd;

	std::string freshEventName = rPd.UniqueEventName("augSymbol");
	//create fresh terminal
	Terminal t(tempPd.InsEvent(freshEventName));

	std::vector<Idx> v;
	//no need for this stack symbol to be unique, because no nonterminal should contain 0
	Idx stackSymbolIdx = tempPd.InsStackSymbol(StackSymbol("augSymbol"));
	v.push_back(stackSymbolIdx);
	Nonterminal nt(0, v);

	//Augments the grammar such that a new grammar production  will be inserted
	Grammar augGr = Aug(gr, nt, t);
	DEBUG_PRINTF(debug,"BlockfreeAugGr",augGr.Str());

	//construct the goto generator for the augmented grammar
	GotoGenerator gotoGen = Lrm(augGr, 1);

	//construct a parser for the grammar
	Lr1Parser parser = Lrp(gr, augGr, gotoGen, 1, t);
	DEBUG_PRINTF(debug,"BlockfreeLrParser",parser.Str());

	//detach augment symbol
	DetachAugSymbol(parser);
	DEBUG_PRINTF(debug,"BlockfreeDetachAugSymbol",parser.Str());

	// Transform following actions
	// (q,a,p,a) 		=> ((q,lambda),a,(p,a),lambda) 		, ((q,a),lambda,(p,a),lambda)
	// (q,a,p,lambda) 	=> ((q,lambda),a,(p,lambda),lambda)	, ((q,a),lambda,(p,lambda),lambda)
	parser = TransformParserAction(parser);
	DEBUG_PRINTF(debug,"BlockfreeTransformedParserActions",parser.Str());

	//convert parser to pushdown generator
	rPd = LrParser2EPDA(parser);
	DEBUG_PRINTF(debug,"BlockfreeLrParser2EPDA",rPd);

	//trim / remove unreachable states
	if(!PushdownTrim(rPd, 0)){
		//if no initial or no marked state exists, return empty generator
		resPd = rPd;
		return;
	}
	DEBUG_PRINTF(debug,"BlockfreeLrParser2EPDATrim",rPd);

	//annotate stack symbols with "old"
	rPd = RenameStackSymbold("old",rPd);

	//merge adjacent transitions
	MergeAdjacentTransitions(rPd);
	DEBUG_PRINTF(debug,"BlockfreeMergeAdjTrans",rPd);

	//trim / remove unreachable states
	if(!PushdownTrim(rPd, 0)){
		//if no initial or no marked state exists, return empty generator
		resPd = rPd;
		return;
	}
	DEBUG_PRINTF(debug,"BlockfreeMergeAdjTransTrim",rPd);

	//removes transitions that pop lambda
	rPd = RemoveLambdaPop(rPd);
	DEBUG_PRINTF(debug,"BlockfreeRemoveLambdaPop",rPd);

	//split transitions popping more than one stack symbol into more transitions
	rPd = RemoveMultPop(rPd);
	DEBUG_PRINTF(debug,"BlockfreeRemoveMultPop",rPd);

	//removes transitions that pop lambda
	rPd = RemoveLambdaPop(rPd);
	DEBUG_PRINTF(debug,"BlockfreeRemoveLambdaPop",rPd);

	//trim
	if(!PushdownTrim(rPd, FAUDES_PD_LOOKAHEAD)){
		//if no initial or no marked state exists, return empty generator
		resPd = rPd;
		return;
	}

	//merge adjacent transitions
	MergeAdjacentTransitions(rPd);
	DEBUG_PRINTF(debug,"BlockfreeMergeAdjTransTrim",rPd);

	//trim
	if(!PushdownTrim(rPd, FAUDES_PD_LOOKAHEAD)){
		//if no initial or no marked state exists, return empty generator
		resPd = rPd;
		return;
	}

	//set correct flags for controllability and observability
	CorrectEvents(pd, rPd);

	//return resulting generator
	resPd = rPd;
	DEBUG_PRINTF(debug,"BlockfreeResult",rPd);
}




void PushdownAccessible(const PushdownGenerator& pd, PushdownGenerator& resPd,  bool coacc, bool debug) {
	FUNCTION(__FUNCTION__);	//scope log
	FD_DF("PushdownAccessible(" << &pd << "," << &resPd << ")");

	//warning if pd isn't deterministic
	if (!pd.IsDeterministic())
		FD_WARN("PushdownBlockfree: The given pushdown automaton is not deterministic !");

	// if no initial state exists,
	//their exists no accessible states or transitions
	if(pd.InitStates().Empty()){
		resPd = pd;
		resPd.ClearStates();
		resPd.ClearTransRel();
		return;
	}

	//take a copy of the old generator
	PushdownGenerator tmp = PushdownGenerator(pd);
	DEBUG_PRINTF(debug,"AccessibleInput",tmp);

	//remember changes from DPDA to SDPDA
	TransformationHistoryVec history;

	//create SDPDA
	tmp = SPDA(tmp, &history);
	DEBUG_PRINTF(debug,"AccessibleSdpda",tmp);

	if(coacc){
		//trim SDPDA
		PushdownTrim(tmp, FAUDES_PD_LOOKAHEAD);
		DEBUG_PRINTF(debug,"AccessibleSdpdaTrim",tmp);
	}

	//convert to grammar
	Grammar gr = Sp2Lr(tmp,0,true);
	DEBUG_PRINTF(debug,"AccessibleSp2Lr",gr.Str());

	if(coacc){
		//remove non-productive productions
		gr = Rnpp(gr);
		DEBUG_PRINTF(debug,"AccessibleRnpp",gr.Str());
	}

	//remove unreachable productions
	gr = Rup(gr);
	DEBUG_PRINTF(debug,"AccessibleRup",gr.Str());

	//get remaining generator
	tmp = RemainingPd(gr, tmp);
	DEBUG_PRINTF(debug,"AccessibleRemainingA",tmp);

	if(coacc){
		//remove not (co)accessible states
		PushdownTrim(tmp,0);
	} else {
		//remove inaccessible states
		tmp.vGenerator::Accessible();
	}

	DEBUG_PRINTF(debug,"AccessibleRemainingB",tmp);

	//recreate DPDA
	tmp = RebuildFromSPDA(tmp, history);
	DEBUG_PRINTF(debug,"AccessibleRebuildFromSPDA",tmp);

	//compare with given DPDA
	TransSet::Iterator transit;
	PopPushSet::const_iterator ppit;
	PushdownGenerator rPd(pd);
	for (transit = pd.TransRelBegin(); transit != pd.TransRelEnd(); transit++) {
		for (ppit = pd.PopPushBegin(*transit); ppit != pd.PopPushEnd(*transit);
				ppit++) {
			if (!tmp.ExistsTransition(*transit, ppit->first, ppit->second)) {
				rPd.ClrTransition(*transit, ppit->first, ppit->second);
			}
		}
	}

	if(coacc){
		//remove not (co)accessible states
		PushdownTrim(tmp,0);
	} else {
		//remove inaccessible states
		tmp.vGenerator::Accessible();
	}

	DEBUG_PRINTF(debug,"AccessibleEnd",rPd);

	//return resulting generator
	resPd = rPd;
}



/* *************************
 * Transient
 * *************************/
StateSet Transient(const PushdownGenerator& pd) {

	TransSet::Iterator transit;
	StateSet rStates;
	//iterate over all transitions
	for (transit = pd.TransRelBegin(); transit != pd.TransRelEnd(); transit++) {

		//add start state of lambda read transition to set
		if (pd.IsEventLambda(transit->Ev)) {
			rStates.Insert(transit->X1);
		}
	}

	return rStates;
}

/* *************************
 * Rnce
 * *************************/
PushdownGenerator Rnce(const PushdownGenerator& pd, const System& s) {
	FUNCTION(__FUNCTION__);	//scope log
	FD_DF("Rnce(" << &pd << "," << &s << ")");

	StateSet transientStates = Transient(pd);
	StateSet ucEars;
	EventSet uc = pd.UncontrollableEvents();
	StateSet::Iterator stateit;
	EventSet::Iterator eventit;
	Idx oldState;

	//iterate over all states
	for (stateit = pd.StatesBegin(); stateit != pd.StatesEnd(); stateit++) {

		//get the merge attribute, which must be either an ear or a heads
		const MergeStateSplit* mss =
				dynamic_cast<const MergeStateSplit*>(pd.Merge(*stateit));
		if (mss == NULL) {
			std::stringstream errstr;
			errstr
					<< "State in Rnce was neither head nor ear, check correctness of Merge attribute"
					<< std::endl;
			throw Exception("Rnce", errstr.str(), 1001);
		}

		//only look at ears
		if (mss->IsHead()) {
			continue;
		}

		//only look at transient ears
		if (transientStates.Exists(*stateit)) {
			continue;
		}

		//check the current state for each uncontrollable event
		for (eventit = uc.Begin(); eventit != uc.End(); eventit++) {

			//only look at states whose corresponding dfa state allows this
			//uncontrollable event
			oldState = pd.DfaState(*stateit);
			if (!s.ExistsTransition(oldState, *eventit)) {
				continue;
			}

			//only look at states that do not allow the uncontrollable event to happen
			if (pd.ExistsTransition(*stateit, *eventit)) {
				continue;
			}

			//all these states are uncontrollable ears
			ucEars.Insert(*stateit);
		}
	}
	PushdownGenerator rPd = pd;
	rPd.DelStates(ucEars);
	return rPd;
}

/* *************************
 * Times
 * *************************/
PushdownGenerator Times(const System& s, const PushdownGenerator& pd) {
	FUNCTION(__FUNCTION__);	//scope log
	FD_DF("Times(" << &s << "," << &pd << ")");

	//result generator
	PushdownGenerator rPd;
	rPd.Name(s.Name() + " | " + pd.Name());

	//map to remember which states get merged into which
	std::map<std::pair<Idx, Idx>, Idx> stateMap;

	StateSet::Iterator pdStateit, sStateit;
	Idx newState;
	std::vector<Idx> oldStatesVector;
	//construct states as cartesian prodcut of both generators states
	for (pdStateit = pd.StatesBegin(); pdStateit != pd.StatesEnd();
			pdStateit++) {
		for (sStateit = s.StatesBegin(); sStateit != s.StatesEnd();
				sStateit++) {

			//add new state for each pair of states and save it in the map
			newState = rPd.InsState();
			stateMap.insert(
					std::make_pair(std::make_pair(*sStateit, *pdStateit),
							newState));

			//add information about parent states
			oldStatesVector.clear();
			oldStatesVector.push_back(*sStateit);
			oldStatesVector.push_back(*pdStateit);
			MergeStates oldStates = MergeStates(oldStatesVector);
			rPd.SetMerge(newState, oldStates);
			rPd.SetDfaState(newState, *sStateit);

			//if both states are init states, the new state is an init state as well
			if (*pdStateit == pd.InitState() && *sStateit == s.InitState()) {
				rPd.SetInitState(newState);
			}

			//if both states are final states, the new state is a final state as well
			if (pd.ExistsMarkedState(*pdStateit)
					&& s.ExistsMarkedState(*sStateit)) {
				rPd.SetMarkedState(newState);
			}
		}
	}

	//stack symbols are the same as the pushdown generator's stack symbols
	rPd.InsStackSymbols(pd.StackSymbols());

	//stack bottom is the same as the pushdown generator's stack bottom
	rPd.SetStackBottom(pd.StackSymbolObj(pd.StackBottom()));

	//the new event set is the intersection of both old event sets
	IntersectEvents(s, pd, rPd);

	//add lambda event if there was a lambda event in the pushdown generator's event set
	if (pd.Alphabet().Exists(FAUDES_PD_LAMBDA)) {
		rPd.InsEvent(FAUDES_PD_LAMBDA);
	}

	Idx newX1, newX2;
	std::string sEvName, pdEvName;
	//add transitions to rPd. Iterate over all pushdown generator transitions
	TransSet::Iterator pdTransit, sTransit;
	PopPushSet::const_iterator ppsit;
	for (pdTransit = pd.TransRelBegin(); pdTransit != pd.TransRelEnd();
			pdTransit++) {

		//look at every pop push set of the transition
		for (ppsit = pd.PopPushBegin(*pdTransit);
				ppsit != pd.PopPushEnd(*pdTransit); ppsit++) {

			pdEvName = pd.EventName(pdTransit->Ev);
			//for every no-read transition (q,lambda,w,w',q') of pd and every state p of s,
			//add a transition ((p,q),lambda,w,w',(p,q'))
			if (pdEvName.compare(FAUDES_PD_LAMBDA) == 0) {
				for (sStateit = s.StatesBegin(); sStateit != s.StatesEnd();
						sStateit++) {

					//find new states corresponding to (p,q), and (p,q')
					newX1 = stateMap.find(
							std::make_pair(*sStateit, pdTransit->X1))->second;
					newX2 = stateMap.find(
							std::make_pair(*sStateit, pdTransit->X2))->second;

					//add transition
					rPd.SetTransition(newX1, rPd.EventIndex(FAUDES_PD_LAMBDA),
							newX2, ppsit->first, ppsit->second);
				}
			}
			//for every read transition (q,a,w,w',q') of pd and every transition (p,a,p')
			//of s, add a transition ((p,q),a,w,w',(p',q'))
			else {

				//iterate over all system transitions
				for (sTransit = s.TransRelBegin(); sTransit != s.TransRelEnd();
						sTransit++) {

					sEvName = s.EventName(sTransit->Ev);
					//compare the transition events
					if (pdEvName.compare(sEvName) == 0) {

						//find new states corresponding to (p,q), and (p',q')
						newX1 =
								stateMap.find(
										std::make_pair(sTransit->X1,
												pdTransit->X1))->second;
						newX2 =
								stateMap.find(
										std::make_pair(sTransit->X2,
												pdTransit->X2))->second;

						//add transition
						rPd.SetTransition(newX1, rPd.EventIndex(sEvName), newX2,
								ppsit->first, ppsit->second);
					}
				}
			}
		}
	}
	// set statenames
	if (s.StateNamesEnabled() && pd.StateNamesEnabled()
			&& rPd.StateNamesEnabled())
		SetComposedStateNames(s, pd, stateMap, rPd);
	else
		rPd.ClearStateNames();

	return rPd;
}

/* *************************
 * IntersectEvents
 * *************************/
void IntersectEvents(const System& s, const PushdownGenerator& pd,
		PushdownGenerator& rPd) {

	EventSet::Iterator pdEventit, sEventit;
	for (pdEventit = pd.AlphabetBegin(); pdEventit != pd.AlphabetEnd();
			pdEventit++) {
		for (sEventit = s.AlphabetBegin(); sEventit != s.AlphabetEnd();
				sEventit++) {
			if (*pdEventit == *sEventit) {
				//controllable and observable
				if (pd.Controllable(*pdEventit) && s.Controllable(*sEventit)
						&& pd.Observable(*pdEventit)
						&& s.Observable(*sEventit)) {
					rPd.InsControllableEvent(*pdEventit);
				}
				//controllable and not observable
				else if (pd.Controllable(*pdEventit)
						&& s.Controllable(*sEventit)
						&& !pd.Observable(*pdEventit)
						&& !s.Observable(*sEventit)) {
					rPd.InsControllableEvent(*pdEventit);
					rPd.ClrObservable(*pdEventit);
				}
				//not controllable, but observable
				else if (!pd.Controllable(*pdEventit)
						&& !s.Controllable(*sEventit)
						&& pd.Observable(*pdEventit)
						&& s.Observable(*sEventit)) {
					rPd.InsEvent(*pdEventit);
				}
				//neither controllable nor observable
				else if (!pd.Controllable(*pdEventit)
						&& !s.Controllable(*sEventit)
						&& !pd.Observable(*pdEventit)
						&& !s.Observable(*sEventit)) {
					rPd.InsUnobservableEvent(*pdEventit);
				}
			}
		}
	}
}

/* *************************
 * Split
 * *************************/
PushdownGenerator Split(const PushdownGenerator& pd) {
	FUNCTION(__FUNCTION__);	//scope log
	FD_DF("Split(" << &pd << ")");

	//copy the generator
	PushdownGenerator rPd = pd;

	//insert lambda event in case it has not already been added
	rPd.InsEvent(FAUDES_PD_LAMBDA);

	//clear transition relation and states, because they will be replaced
	rPd.ClearTransRel();
	StateSet::Iterator stateit = rPd.StatesBegin();
	while (stateit != rPd.StatesEnd()) {
		rPd.DelState(*(stateit++)); // fixed: 2013-12-17
	}

	//map ears and stack symbols to their heads for later reference
	std::multimap<Idx, std::pair<Idx, Idx> > headEarMap;
	//map ears and stack symbols to the states of the old generator for later reference.
	//headMap<headIdx,oldIdx>, earMap<oldIdx,headIdx>
	std::map<Idx, Idx> headMap, earMap;

	StackSymbolSet::Iterator ssit;
	Idx newHead, newEar;
	//insert new states as ears and heads
	for (stateit = pd.StatesBegin(); stateit != pd.StatesEnd(); stateit++) {

		//insert a head for each state in the old generator
		newHead = rPd.InsState();
		MergeStateSplit mss(*stateit);
		rPd.SetMerge(newHead, mss);
		rPd.SetDfaState(newHead, pd.DfaState(*stateit));

		//if the state was the initial state in the old generator, this head
		//is an initial state as well
		if (pd.ExistsInitState(*stateit)) {
			rPd.SetInitState(newHead);
		}

		//save head in map
		headMap.insert(std::make_pair(*stateit, newHead));

		//insert an ear for each state in the old generator and each stack symbol
		//except lambda
		for (ssit = rPd.StackSymbolsBegin(); ssit != rPd.StackSymbolsEnd();
				ssit++) {
			if (!rPd.IsStackSymbolLambda(*ssit)) {
				newEar = rPd.InsState();
				MergeStateSplit mss(*stateit, *ssit);
				rPd.SetMerge(newEar, mss);
				rPd.SetDfaState(newEar, pd.DfaState(*stateit));

				//if the state was a final state in the old generator, this ears
				//is a final state as well
				if (pd.ExistsMarkedState(*stateit)) {
					rPd.SetMarkedState(newEar);
				}

				//save ear in map
				earMap.insert(std::make_pair(newEar, *stateit));
				//save pair of head and ear in map
				headEarMap.insert(
						std::make_pair(newHead, std::make_pair(newEar, *ssit)));

			}
		}
	}

	TransSet::Iterator transit;
	PopPushSet::const_iterator popPushit;
	std::multimap<Idx, std::pair<Idx, Idx> >::iterator headEarit;
	std::vector<Idx> ssvector;
	Idx head, ear, startState, endState, event;
	Idx stackSymbol;
	//insert transitions
	//iterate over all ears
	for (headEarit = headEarMap.begin(); headEarit != headEarMap.end();
			headEarit++) {

		//insert transition from head to ear
		head = headEarit->first;
		ear = headEarit->second.first;
		stackSymbol = headEarit->second.second;

		ssvector.clear();
		ssvector.push_back(stackSymbol);

		rPd.SetTransition(head, rPd.EventIndex(FAUDES_PD_LAMBDA), ear, ssvector,
				ssvector);

		//connect ears to heads
		for (transit = pd.TransRelBegin(); transit != pd.TransRelEnd();
				transit++) {
			for (popPushit = pd.PopPushBegin(*transit);
					popPushit != pd.PopPushEnd(*transit); popPushit++) {

				//determine if this ear is associated with the transitions start state and
				//determine if the ear has the same stack symbol as the transition's
				//pop symbol (the pop vector is supposed to contain only one stack symbol).
				//if both are true:
				if (earMap.find(ear)->second == transit->X1
						&& stackSymbol == *popPushit->first.begin()) {

					//look up the head associated with the transition's end state
					endState = headMap.find(transit->X2)->second;
					startState = ear;
					event = rPd.EventIndex(pd.EventName(transit->Ev));

					//insert transition from ear to head
					rPd.SetTransition(startState, event, endState,
							popPushit->first, popPushit->second);
				}
			}
		}
	}
	return rPd;
}


/* *************************
 * CorrectEvents
 * *************************/
void CorrectEvents(const PushdownGenerator& correctPd, PushdownGenerator& pd) {
	EventSet::Iterator correctevit, evit;
	for (evit = pd.AlphabetBegin(); evit != pd.AlphabetEnd(); evit++) {
		for (correctevit = correctPd.AlphabetBegin();
				correctevit != correctPd.AlphabetEnd(); correctevit++) {
			if (*evit == *correctevit) {
				//controllable
				if (correctPd.Controllable(*correctevit)) {
					pd.SetControllable(*evit);
				}
				//uncontrollable
				else {
					pd.ClrControllable(*evit);
				}

				//observable
				if (correctPd.Observable(*correctevit)) {
					pd.SetObservable(*evit);
				}
				//uncontrollable
				else {
					pd.ClrObservable(*evit);
				}
			}
		}
	}
}








/* *************************
 * timeval_subtract
 * *************************/
int timeval_subtract(timeval* result, timeval* x, timeval* y) {
	//FUNCTION(__FUNCTION__);

	/* Perform the carry for the later subtraction by updating y. */
	if (x->tv_usec < y->tv_usec) {
		int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
		y->tv_usec -= 1000000 * nsec;
		y->tv_sec += nsec;
	}
	if (x->tv_usec - y->tv_usec > 1000000) {
		int nsec = (x->tv_usec - y->tv_usec) / 1000000;
		y->tv_usec += 1000000 * nsec;
		y->tv_sec -= nsec;
	}

	/* Compute the time remaining to wait.
	 tv_usec is certainly positive. */
	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_usec = x->tv_usec - y->tv_usec;

	/* Return 1 if result is negative. */
	return x->tv_sec < y->tv_sec;
}

} // namespace faudes

