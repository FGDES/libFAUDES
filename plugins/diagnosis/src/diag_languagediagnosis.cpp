/** @file diag_languagediagnosis.cpp
Functions to check a system's language-diagnosability and compute a language-diagnoser. 

(c) 2009, Tobias Barthel, Thomas Moor, Klaus Schmidt.
*/

#include "diag_languagediagnosis.h"

using namespace std;

namespace faudes {


///////////////////////////////////////////////////////////////////////////////
// Functions for specification framework
///////////////////////////////////////////////////////////////////////////////


// rti function interface
bool IsLanguageDiagnosable(const System& rGen, const System& rSpec) {
  std::string ignore;
  return IsLanguageDiagnosable(rGen,rSpec,ignore);
}


// ComputeGobs()
void ComputeGobs(const System& rGenMarkedNonSpecBehaviour, Diagnoser& rGobs) {
  EventSet gObservableEvents, gUnobservableEvents;
  StateSet newGobsStates;
  Idx currDState = 0;
  Idx nextDState = 0;
  Idx gStateEstimate = 0;
  Idx newState = 0;
  map<Idx,multimap<Idx,DiagLabelSet> > reachMap;   // maps executable events to all reachable states and occuring relative failure types
  map<Idx,multimap<Idx,DiagLabelSet> >::iterator it;
  multimap<Idx,DiagLabelSet>::iterator mmit, mmit2;
  pair<Idx,DiagLabelSet> reachMapPair;  
  TransSet transitions;
  DiagLabelSet oldLabel, newLabel, occFailureTypes;
  DiagLabelSet::Iterator labelIt;
  StateSet gObsStates;
  StateSet::Iterator stateIt, initStateIt;
  EventSet activeEvents;
  AttributeDiagnoserState newAttr;
  const TaIndexSet<DiagLabelSet>* pDiagState;
  
  FD_DD("ComputeGobs()");
  
  // clear Gobs
  rGobs.Clear();
  rGobs.ClearAttributes();
  
  // get observable events of original generator
  gObservableEvents = rGenMarkedNonSpecBehaviour.ObservableEvents();
  FD_DD( "Observable events: " << gObservableEvents.ToString() );
  
  // get unobservable events of original generator
  gUnobservableEvents = rGenMarkedNonSpecBehaviour.UnobservableEvents();
  FD_DD( "Unobservable events: " << gUnobservableEvents.ToString() );

  // parse through init states of rGenMarkedNonSpecBehaviour
  for (initStateIt = rGenMarkedNonSpecBehaviour.InitStatesBegin(); initStateIt != rGenMarkedNonSpecBehaviour.InitStatesEnd(); initStateIt++) {
    newGobsStates.Clear();
    newLabel.Clear();
    newAttr.Clear();
        
    // create newAttr with state index of current initial state of rGenMarkedNonSpecBehaviour and label "N"
    newLabel.Insert(DiagLabelSet::IndexOfLabelN());
    newAttr.AddStateLabelMap(*initStateIt,newLabel);

    // if newAttr equals any existing state attribute then use this state and make it an initial state
    gObsStates = rGobs.States();
    stateIt = gObsStates.Begin();
    while (stateIt != gObsStates.End()) {
      if (newAttr == rGobs.StateAttribute(*stateIt)) {
        FD_DD("Statelabel " << *initStateIt << "N already exists at state " << *stateIt << " --> make it init state.");
        rGobs.SetInitState(*stateIt);
        break;
      }
      stateIt++;
    }
    // if newAttribute is new to Gobs
    if (stateIt == gObsStates.End()) {
      // create new initial state of Gobs and its attribute with label "normal"
      currDState = rGobs.InsInitState();
      rGobs.StateAttribute(currDState, newAttr);
      newGobsStates.Insert(currDState);
    }
    
    // parse through new Gobs states
    while (!newGobsStates.Empty()) {
      // set current Gobs state
      currDState = *newGobsStates.Begin();
      
      // get unique state estimate and unique failure label from Gobs
      pDiagState = rGobs.StateAttribute(currDState).DiagnoserStateMapp();
      gStateEstimate = *(pDiagState->Begin());
      oldLabel = pDiagState->Attribute(*(pDiagState->Begin()));

      //  generate reachMap for current state estimate
      ComputeReachability(rGenMarkedNonSpecBehaviour, gUnobservableEvents, gStateEstimate, reachMap);


#ifdef FAUDES_DEBUG_DIAGNOSIS
      FD_DD("reachMap: ");
      for (it = reachMap.begin(); it != reachMap.end(); it++) {
        //print reachMap for current event
        FD_DD("_" << rGenMarkedNonSpecBehaviour.EventName(it->first) << " ("<< it->second.size() << " state estimates)");
        for (mmit = it->second.begin(); mmit != it->second.end(); mmit++) {
          FD_DD(mmit->first << " " << mmit->second.ToString());
        }
      }
#endif
      
      // parse through reachMap (eventwise)
      for (it = reachMap.begin(); it != reachMap.end(); it++) {
#ifdef FAUDES_DEBUG_DIAGNOSIS
        // print reachMap for current event
        FD_DD("Parsing reachMap: " << "_" << rGenMarkedNonSpecBehaviour.EventName(it->first));
        for (mmit = it->second.begin(); mmit != it->second.end(); mmit++) {
          FD_DD(mmit->first << " " << mmit->second.ToString());
        }
        #endif
        
        FD_DD("old label: " << oldLabel.ToString());
        
        newState = 0;
        // parse through state failure type map (for current event in reachMap)
        for (mmit = it->second.begin(); mmit != it->second.end(); mmit++) {
          newState = mmit->first;
          FD_DD("new state: " << newState);
          occFailureTypes = mmit->second;
          FD_DD("failure types occurred: " << occFailureTypes.ToString());
          LabelPropagation(oldLabel, occFailureTypes, newLabel);
          FD_DD("new label: " << newLabel.ToString());
          newAttr.Clear();
          newAttr.AddStateLabelMap(newState,newLabel);

          // if newAttr equals any existing state attribute then we create a transition to this very state
          gObsStates = rGobs.States();
          stateIt = gObsStates.Begin();
          while (stateIt != gObsStates.End()) {
            if (newAttr == rGobs.StateAttribute(*stateIt)) {
              FD_DD("realising as back- or self-loop to existing state " << *stateIt);
              if (!rGobs.ExistsTransition(currDState,it->first,*stateIt)) {
                if (!rGobs.ExistsEvent(it->first)) {
                  rGobs.InsEvent(it->first);
                }
                rGobs.SetTransition(currDState,it->first,*stateIt);
              }
              break;
            }
            stateIt++;
          }
          // if newAttribute is new to Gobs
          if (stateIt == gObsStates.End()) {
            // create new Gobs state and add it to new states
            nextDState = rGobs.InsState();
            FD_DD("Create new state " << nextDState << " and transition "  << currDState << " --" << rGenMarkedNonSpecBehaviour.EventName(it->first) << "--> " << nextDState);
            newGobsStates.Insert(nextDState);
            rGobs.InsEvent(it->first);
            rGobs.SetTransition(currDState,it->first,nextDState);
            
            //rGobs.InsStateLabelMap(nextDState,newState,newLabel);
            rGobs.StateAttribute(nextDState, newAttr);
          }
        }
      }
      
      activeEvents = rGobs.ActiveEventSet(currDState);
      transitions = rGobs.ActiveTransSet(currDState);

      // delete current Gobs state from new states
      newGobsStates.Erase(currDState);  
    }
  }
}

// ComputeReachability()
void ComputeReachability(const System& rGen, const EventSet& rUnobsEvents, Idx State, 
                map<Idx,multimap<Idx,DiagLabelSet> >& rReachabilityMap) {
                
  FD_DD("ComputeReachability() for state " << State << "...");
  StateSet doneStates;
  
  doneStates.Clear();
  rReachabilityMap.clear();
  ComputeReachabilityRecursive(rGen, rUnobsEvents, State, doneStates, rReachabilityMap);
  FD_DD("ComputeReachability for state " << State << ": done");
  
  #ifdef FAUDES_DEBUG_DIAGNOSIS
  map<Idx,multimap<Idx,DiagLabelSet> >::iterator it;
  multimap<Idx,DiagLabelSet>::iterator mmLabelIt;
  FD_DD("rReachabilityMap: ");
  for (it = rReachabilityMap.begin(); it != rReachabilityMap.end(); it++) {
    // print rReachabilityMap for current event
    FD_DD("_" << rGen.EventName(it->first) << " ("<< it->second.size() << " state estimates)");
    for (mmLabelIt = it->second.begin(); mmLabelIt != it->second.end(); mmLabelIt++) {
      FD_DD(mmLabelIt->first << " " << mmLabelIt->second.ToString());
    }
  }
  FD_DD("");
  #endif
}

// ComputeReachabilityRecursive()
void ComputeReachabilityRecursive(const System& rGen, const EventSet& rUnobsEvents, Idx State, StateSet done, 
                    map<Idx,multimap<Idx,DiagLabelSet> >& rReachabilityMap) {
  TransSet trans;
  TransSet::Iterator tIt;
  EventSet tmpFailureSet;
  EventSet::Iterator evIt;
  multimap<Idx,DiagLabelSet> stateFailureTypeMap; // maps generator states onto occurred failures types (=labels), part of rReachabilityMap
  multimap<Idx,DiagLabelSet>::iterator mmLabelIt;
  map<Idx,multimap<Idx,DiagLabelSet> >::iterator it;
  DiagLabelSet newFT;
  bool mappingExists;
  
  if (done.Exists(State)) {
    FD_DD( "State " << State << " has already been processed." );
    return;
  }
  trans = rGen.ActiveTransSet(State);
  
  FD_DD("ComputeReachabilityRecursive() for state " << State << ": Events in ActiveTransSet: ");  
  // parse through active transitions of current generator state
  for (tIt = trans.Begin(); tIt != trans.End(); tIt++) {
    FD_DD(tIt->X1 << "--" << rGen.EventName(tIt->Ev) << "-->" << tIt->X2);
    // if current event is unobservable
    if (rUnobsEvents.Exists(tIt->Ev)) {
      // call ComputeReachabilityRecursive for successor state
      done.Insert(tIt->X1);
      ComputeReachabilityRecursive(rGen, rUnobsEvents, tIt->X2, done, rReachabilityMap);
    }
    // if current event is observable add failure (spec violation) to rReachabilityMap
    else {
      FD_DD(rGen.EventName(tIt->Ev) << " is observable: add it to rReachabilityMap ");
      // get old entry of rReachabilityMap if it exists
      stateFailureTypeMap.clear();
      if (rReachabilityMap.find(tIt->Ev) != rReachabilityMap.end()) {
        stateFailureTypeMap = rReachabilityMap[tIt->Ev];
#ifdef FAUDES_DEBUG_DIAGNOSIS
        FD_DD("old entry of rReachabilityMap for " << rGen.EventName(tIt->Ev));
        for (mmLabelIt = stateFailureTypeMap.begin(); mmLabelIt != stateFailureTypeMap.end(); mmLabelIt++) {
          FD_DD(mmLabelIt->first << " " << mmLabelIt->second.ToString());
        }
#endif        
      }
      newFT.Clear();
      // if successor state is marked: add failure label
      if (rGen.ExistsMarkedState(tIt->X2)) {
        newFT.Insert(DiagLabelSet::IndexOfLabelSpecViolated());
      }
      // if no failure occurred add normal label
      if (newFT.Empty()) {
        newFT.Insert(DiagLabelSet::IndexOfLabelRelN());
      }
      // check if new mapping does already exist
      mappingExists = false;
      for (mmLabelIt = stateFailureTypeMap.lower_bound(tIt->X2); mmLabelIt != stateFailureTypeMap.upper_bound(tIt->X2); mmLabelIt++) {
        if (mmLabelIt->second == newFT) {
          mappingExists = true;
        }
      }
      // if new mapping does not yet exist: add it to rReachabilityMap
      if (!mappingExists) {
        stateFailureTypeMap.insert(pair<Idx,DiagLabelSet>(tIt->X2,newFT));
        rReachabilityMap[tIt->Ev] = stateFailureTypeMap;
      }
    }
  }
}

// ComputeLanguageDiagnoser()
void LanguageDiagnoser(const System& rGen, const System& rSpec, Diagnoser& rDiagGen) {
  System genMarkedNonSpecBehaviour, specComplement;
  Diagnoser genGobs;
  
  FD_DD("LanguageDiagnoser()");

  // copy const arguments
  System gen = rGen;
  System spec = rSpec;

  // prepare result
  rDiagGen.Clear();
  
  // mark all states in generator and specification
  StateSet::Iterator sit;
  for (sit = gen.StatesBegin(); sit != gen.StatesEnd(); sit++) 
    gen.SetMarkedState(*sit);
  for (sit = spec.StatesBegin(); sit != spec.StatesEnd(); sit++) 
    spec.SetMarkedState(*sit);
  
  // lift spec  to same alphabet (insert missing events)
  EventSet diffEventsGenAndSpec = gen.Alphabet() - spec.Alphabet();
  FD_DD("diffEventsGenAndSpec: " << diffEventsGenAndSpec.ToString() );
  spec.InsEvents(diffEventsGenAndSpec);  

  // figure failure behaviour Gtilde=  Closure(G) ^ Complement(Closure(Specification))
  // technical note: to obtain a diagnoser that refers to  G state indicees, we use the reverse
  // composition map to track the correspondance to the original state space.
  specComplement = spec;
  LanguageComplement(specComplement);  
  ProductCompositionMap rcmap;
  Parallel(gen, specComplement, rcmap, genMarkedNonSpecBehaviour);

  genMarkedNonSpecBehaviour.ClrObservable(gen.UnobservableEvents());
#ifdef FAUDES_DEBUG_DIAGNOSIS
  genMarkedNonSpecBehaviour.GraphWrite("tmp_wrtb_Gtilde_spec.png");
  genMarkedNonSpecBehaviour.Write("tmp_wrtb_Gtilde_spec.gen");
#endif
  
  // bail out trivial case "no failures"
  if (genMarkedNonSpecBehaviour.MarkedStatesEmpty()) {
    rDiagGen.InjectAlphabet(gen.ObservableEvents());
    Idx dstate = rDiagGen.InsInitState();
    // TODO: loop over all possible states
    rDiagGen.InsStateLabelMapping(dstate,rGen.InitState(),DiagLabelSet::IndexOfLabelN());
    SelfLoop(rDiagGen,gen.ObservableEvents());
    return;
  }

  // compute the observer, i.e. non-deterministic basis for diagnoser  
  ComputeGobs(genMarkedNonSpecBehaviour, genGobs);

#ifdef FAUDES_DEBUG_DIAGNOSIS
  genGobs.GraphWrite("tmp_wrtb_gobs.png");
#endif
 

  // post process
  // We construct a deterministic diagnoser from the non-deterministic GObs by
  // parsing the transition structure of GObs and merging possible diagnosis 
  // labels.

  // have all observable events
  rDiagGen.InjectAlphabet(gen.ObservableEvents());


  // create initial state of diagnoser and its attribute with label "normal"
#ifdef FAUDES_CHECKED
  if(genGobs.InitStatesSize() != 1) {    
    std::stringstream errstr;
    errstr << "Diagnosis: Internal Error: Go must have one initial state only" << std::endl;
    throw Exception("LanguageDiagnoser()", errstr.str(), 301);
  }
#endif
  Idx gInitState = genGobs.InitState();
  Idx dInitState = rDiagGen.InsInitState();
  rDiagGen.InsStateLabelMapping(dInitState,gInitState,DiagLabelSet::IndexOfLabelN());

  // track state correspondence (map diagnoser states to genObs states)
  std::multimap<Idx,Idx> dgmap; 
  dgmap.insert(std::pair<Idx,Idx>(dInitState,gInitState));

  // loop variables  
  EventSet activeEvents;
  AttributeDiagnoserState newDAttr;

  // parse through gObsStates states
  StateSet dStates;
  dStates.Insert(dInitState);
  while (!dStates.Empty()) {
    // set current diagnoser state 
    Idx dstate = *dStates.Begin(); 
    // prepare reachmap: ev -> gSuccessors
    std::multimap<Idx,Idx> gSuccessors;
    // iterate over corresponding genObs States  
    std::multimap<Idx,Idx>::iterator gsit= dgmap.lower_bound(dstate);    
    std::multimap<Idx,Idx>::iterator gsit_end= dgmap.upper_bound(dstate);
    for(;gsit!=gsit_end; gsit++) {    
      Idx gState = gsit->second;
      // iterate over all successors
      TransSet::Iterator tit= genGobs.TransRelBegin(gState);
      TransSet::Iterator tit_end= genGobs.TransRelEnd(gState);
      for(; tit!=tit_end; tit++)  
	gSuccessors.insert(std::pair<Idx,Idx>(tit->Ev,tit->X2)); 
    }
    // per event, accumulate estimates
    gsit= gSuccessors.begin();
    gsit_end= gSuccessors.end();
    std::multimap<Idx,Idx>::iterator gsit_beg= gsit;
    newDAttr.Clear();
    while(gsit!=gsit_end) {
      Idx ev= gsit->first;
      // TODO: need merge method for diagnoser estimates
      const AttributeDiagnoserState sestimate= genGobs.StateAttribute(gsit->second);
      StateSet::Iterator lit=sestimate.DiagnoserStateMap().Begin();
      StateSet::Iterator lit_end=sestimate.DiagnoserStateMap().End();
      for(; lit!=lit_end; lit++) {
        //newDAttr.AddStateLabelMap(*lit, sestimate.DiagnoserStateMap().Attribute(*lit)); 
        newDAttr.AddStateLabelMap(rcmap.Arg1State(*lit), sestimate.DiagnoserStateMap().Attribute(*lit)); 
      }
      // increment 
      gsit++;
      // sense new event to set label
      bool nextev=false;
      if(gsit== gsit_end) nextev=true;
      if(gsit!= gsit_end) if(gsit->first!=ev) nextev=true;
      if(!nextev) continue;
      // insert new diag state set attribute
      // case 1: attribute exists, so use the corresponding diagnoser state
      // note: rather hold a map rather than to iterate over all attributes
      StateSet::Iterator sit= rDiagGen.StatesBegin();
      StateSet::Iterator sit_end= rDiagGen.StatesEnd();
      for(; sit!=sit_end; sit++) {
        if(!(newDAttr == rDiagGen.StateAttribute(*sit))) continue; 
        FD_DD("LanguageDiagnoser(): insert diag transition " << rDiagGen.TStr(Transition(dstate,ev,*sit)));
        rDiagGen.SetTransition(dstate,ev,*sit); 
        break;
      }
      // case 2: attribute does not exist, so insert new diagnoser state
      if(sit==sit_end) {
        // insert new dstate
        Idx newDState=rDiagGen.InsState();
        dStates.Insert(newDState);
        rDiagGen.StateAttribute(newDState,newDAttr);
        FD_DD("LanguageDiagnoser(): insert state with attr " << newDAttr.Str());
        // track corresponding gstates
        std::multimap<Idx,Idx>::iterator gesit= gsit_beg;
        for(;gesit!=gsit;gesit++) {
          FD_DD("LanguageDiagnoser(): corresponding gstate " << gesit->second);
	  dgmap.insert(std::pair<Idx,Idx>(newDState,gesit->second));          
        }
        // insert transition
        FD_DD("LanguageDiagnoser(): insert diag transition " << rDiagGen.TStr(Transition(dstate,ev,newDState)));
        rDiagGen.SetTransition(dstate,ev,newDState);
      }
      // initialise per event variables in multimap loop
      gsit_beg=gsit;
      newDAttr.Clear();
    }
    // delete current state from todo list
    dStates.Erase(dstate);
  }

#ifdef FAUDES_DEBUG_DIAGNOSIS
  rDiagGen.GraphWrite("tmp_wrtb_diag.png");
#endif    
}


// IsLanguageDiagnosable
bool IsLanguageDiagnosable(const System& rGen, const System rSpec, std::string& rReportString){
	FD_DD("IsLanguageDiagnosable()");
	EventSet obsEvents = rGen.ObservableEvents();
#ifdef FAUDES_DEBUG_DIAGNOSIS
	obsEvents.DWrite();
#endif
	Generator verifier; // Graph for diagnosability verification
	Idx nullEvent = verifier.InsEvent("nullEvent"); // event that corresponds to the value 0
	Idx negEvent = verifier.InsEvent("negEvent"); // event that corresponds to the value -1 
	//Generator verifierTest; // verifier with eeent information
	//verifierTest.InjectAlphabet(rGen.Alphabet() ); // FIXME remove this. 
	//verifierTest.InsEvent(nullEvent);
	map<Idx, VerifierState> stateToVerifierMap;
	map<VerifierState, Idx> verifierToStateMap;
	stack<pair<Idx, VerifierState> > waitingStates;
	StateSet doneStates;
	// initialize the algorithm 
	EventSet fullAlphabet = rGen.Alphabet();
	EventSet::Iterator eIt, eEndIt;
	eEndIt = fullAlphabet.End();
	Idx newState = verifier.InsInitState();
	//verifierTest.InsInitState(newState);
	VerifierState newVerifierState = VerifierState(rSpec.InitState(), rSpec.InitState(), rGen.InitState(),NORMAL);
	stateToVerifierMap[newState] = newVerifierState;
	verifierToStateMap[newVerifierState ] = newState;
	waitingStates.push(make_pair(newState, newVerifierState) );
	// extend the verifier graph until no new nodes can be added
	pair<Idx, VerifierState> currentState;
	Idx blockingState; // = verifier.InsMarkedState();
	TransSet::Iterator tIt;
	map<VerifierState, Idx>::const_iterator vsIt;
	Idx X2;
	bool block = false; // set to true if the BLOCK state is reachable  
	FD_DD("Main loop");
	while(waitingStates.empty() == false){
		// take the first element from the stack
		currentState = waitingStates.top();
		waitingStates.pop();
		doneStates.Insert(currentState.first);
		FD_DD("currentState: " + ToStringInteger(currentState.first) + " VerifierState: (" + rSpec.StateName(currentState.second.mSpec1State) + "," + rSpec.StateName(currentState.second.mSpec2State) + "," + rGen.StateName(currentState.second.mPlantState) + "," + ToStringInteger(currentState.second.mLabel) + ")");
		// go over all possible events
		eIt = fullAlphabet.Begin();
		for(; eIt != eEndIt; eIt++){
			// if the event is not observable, the specification and the plant can evolve independently
			if(obsEvents.Exists(*eIt) == false){
				tIt = rSpec.TransRelBegin(currentState.second.mSpec1State,*eIt);
				// transition in Spec1 exists
				if(tIt != rSpec.TransRelEnd(currentState.second.mSpec1State,*eIt) ){
					if(currentState.second.mLabel == NORMAL ){ // transition in spec1 from normal state
						newVerifierState = VerifierState(tIt->X2, currentState.second.mSpec2State, currentState.second.mPlantState, NORMAL);
					}
					else // transition in spec1 from confused state
						newVerifierState = VerifierState(tIt->X2, currentState.second.mSpec2State, currentState.second.mPlantState, CONFUSED);
					// check if the new VerifierState already exist snd insert new transitions (rule 1 and 4)
					vsIt = verifierToStateMap.find(newVerifierState);
					// a new state is inserted into the verifier
					if(vsIt == verifierToStateMap.end() ){
						newState = verifier.InsState();
						//verifierTest.InsState(newState);
						verifier.SetTransition(currentState.first,nullEvent,newState);
						//verifierTest.SetTransition(currentState.first,*eIt,newState);
						verifierToStateMap[newVerifierState] = newState;
						stateToVerifierMap[newState] = newVerifierState;
						if(doneStates.Exists(newState) == false)
							waitingStates.push(make_pair(newState,newVerifierState) );
					}
					// a transition to the existing state is added
					else{
						//verifierTest.SetTransition(currentState.first,*eIt,vsIt->second);
						verifier.SetTransition(currentState.first,nullEvent,vsIt->second);
						
					}
				}
				tIt = rGen.TransRelBegin(currentState.second.mPlantState,*eIt);
				// transition in plant exists
				if(tIt != rGen.TransRelEnd(currentState.second.mPlantState,*eIt) ){
					X2 = tIt->X2;
					if(currentState.second.mLabel == CONFUSED)
						newVerifierState = VerifierState(currentState.second.mSpec1State, currentState.second.mSpec2State, X2, CONFUSED);
					else{ // current state is NORMAL
						tIt = rSpec.TransRelBegin(currentState.second.mSpec2State,*eIt); 
						if(tIt == rSpec.TransRelEnd(currentState.second.mSpec2State,*eIt) ){ // violation of the specification
							newVerifierState = VerifierState(currentState.second.mSpec1State, currentState.second.mSpec2State, X2, CONFUSED);
						}
						else{ // correct behavior
							newVerifierState = VerifierState(currentState.second.mSpec1State, tIt->X2, X2, NORMAL);
						}
					}
					// check if a new state has to be inserted into the verifier a
					vsIt = verifierToStateMap.find(newVerifierState);
					// a new state is inserted into the verifier
					if(vsIt == verifierToStateMap.end() ){
						newState = verifier.InsState();
						//verifierTest.InsState(newState);
						verifierToStateMap[newVerifierState] = newState;
						stateToVerifierMap[newState] = newVerifierState;
						if(doneStates.Exists(newState) == false)
							waitingStates.push(make_pair(newState,newVerifierState) );
					}
					else	
						newState = vsIt->second;
					// new transition in the verifier
					if(newVerifierState.mLabel == NORMAL || (currentState.second.mLabel == CONFUSED && newVerifierState.mPlantState == currentState.second.mPlantState) ){ // normal behavior or confused behavior extended in the specification (rule 3 or 4)
						//verifierTest.SetTransition(currentState.first,*eIt,newState);
						verifier.SetTransition(currentState.first,nullEvent,newState);
					}
					else{ // faulty behavior extended in the plant (rule 5)
						//verifierTest.SetTransition(currentState.first,*eIt,newState);
						verifier.SetTransition(currentState.first,negEvent,newState);
					}
				}
			}// (obsEvents.Exists(*eIt) == false)
			else{
				TransSet::Iterator plantIt, specIt;
				Idx eventIdx;
				tIt = rSpec.TransRelBegin(currentState.second.mSpec1State, *eIt);
				plantIt= rGen.TransRelBegin(currentState.second.mPlantState, *eIt);
				specIt = rSpec.TransRelBegin(currentState.second.mSpec2State, *eIt);
				if(tIt != rSpec.TransRelEnd(currentState.second.mSpec1State, *eIt) && plantIt != rGen.TransRelEnd(currentState.second.mPlantState, *eIt) ){ // event occurs in the potentially confused specification and in the plant
					if(currentState.second.mLabel == NORMAL && specIt != rSpec.TransRelEnd(currentState.second.mSpec2State, *eIt) ){ // no confusion (rule 6)
						newVerifierState = VerifierState(tIt->X2, specIt->X2, plantIt->X2, NORMAL);
						eventIdx = nullEvent;
					}
					else if(currentState.second.mLabel == NORMAL){// faulty behavior occurs (rule 7)
						newVerifierState = VerifierState(tIt->X2, currentState.second.mSpec2State, plantIt->X2, CONFUSED);
						eventIdx = negEvent;
					}
					else{ // there is already confusion (rule 8)
						newVerifierState = VerifierState(tIt->X2, currentState.second.mSpec2State, plantIt->X2, CONFUSED);
						eventIdx = negEvent;
					}
					// check if a new state has to be inserted into the verifier a
					vsIt = verifierToStateMap.find(newVerifierState);
					// a new state is inserted into the verifier
					if(vsIt == verifierToStateMap.end() ){
						newState = verifier.InsState();
						//verifierTest.InsState(newState);
						verifierToStateMap[newVerifierState] = newState;
						stateToVerifierMap[newState] = newVerifierState;
						if(doneStates.Exists(newState) == false)
							waitingStates.push(make_pair(newState,newVerifierState) );
					}
					else	
						newState = vsIt->second;		

					// update the verifier
					//verifierTest.SetTransition(currentState.first,*eIt,newState);
					verifier.SetTransition(currentState.first,eventIdx,newState);
				}
			}
		}
		// check if the Block state is reachable 
		if(rGen.TransRelBegin(currentState.second.mPlantState) == rGen.TransRelEnd(currentState.second.mPlantState) && currentState.second.mLabel == CONFUSED){
			blockingState = verifier.InsMarkedState();
			verifier.SetTransition(currentState.first,nullEvent,blockingState);
			//verifierTest.InsMarkedState(blockingState);
			//verifierTest.SetTransition(currentState.first,nullEvent,blockingState);
			FD_DD("Blocking State Reachable");
			block = true;
		}
	}
	#ifdef FAUDES_DEBUG_DIAGNOSIS
	verifier.GraphWrite("data/verifier.png");
	#endif 
	//verifierTest.GraphWrite("data/verifierTest.png");
	// Seach for cycles with "-1"-transitions (negEvent) in the verifier
	list<StateSet> sccList;
	StateSet rootSet;
	// compute the strongly connected components in the verifier
	ComputeScc(verifier,sccList,rootSet);
	// Check if there is a "-1"-transition in any of the SCCs
	list<StateSet>::const_iterator sccIt, sccEndIt;
	sccIt = sccList.begin();
	sccEndIt = sccList.end();
	StateSet::Iterator stIt, stEndIt;
	bool existsCycle = false;
	for( ; sccIt != sccEndIt; sccIt++){
#ifdef FAUDES_DEBUG_DIAGNOSIS
		sccIt->Write();
#endif
		stIt = sccIt->Begin();
		stEndIt = sccIt->End();
		for(; stIt != stEndIt; stIt++){// check all states in the SCC
			tIt = verifier.TransRelBegin(*stIt, negEvent);
			if(tIt != verifier.TransRelEnd(*stIt, negEvent) && sccIt->Exists(tIt->X2) ){ // there is a transition with negEvent within the SCC
				FD_DD("Confused Cycle Found");
				existsCycle = true;
				break;
			}
		}
		if(existsCycle == true)
			break;
	}
	if(block == true || existsCycle == true)
		return false;
	else
		return true;
}

//IsLoopPreservingObserver()
bool IsLoopPreservingObserver(const System& rGen, const EventSet& rHighAlph){
	System genCopy;
	TransSet::Iterator tit;
	string report;
	FD_DD("IsLoopPreservingObserver()");
	genCopy = rGen;
	genCopy.InjectMarkedStates(genCopy.States() );
	// Verify if the observer condition is fulfilled
	if(IsObs(genCopy,rHighAlph)  == false){ 
		FD_DD("Observer Condition violated");
		return false;
	}
	FD_DD("Observer Condition fulfilled");
	// Verify if there are loops with abstracted events
	// delete all transitions that do not belong to local high-alphabet
	for (tit = genCopy.TransRelBegin(); tit != genCopy.TransRelEnd();) {
		if (rHighAlph.Exists(tit->Ev)) 
			tit=genCopy.ClrTransition(tit);    
		else 
			++tit;
	}
	// search for cycles in remainder automaton
	std::list<StateSet> sccList;
	StateSet sccRoots;
	ComputeScc(genCopy,sccList,sccRoots);
	std::list<StateSet>::const_iterator sIt = sccList.begin();
	for( ; sIt != sccList.end(); sIt++){
		if(sIt->Size() > 1){
			#ifdef FAUDES_DEBUG_DIAGNOSIS
			cout << "Bad states that form a cycle with abstracted events: " << endl;
			StateSet::Iterator stIt = sIt->Begin();
			for(; stIt != sIt->End(); stIt++)
				cout << *stIt << " ";
			cout << endl;
			#endif
			return false;
		}
	}
	return true;
}


void LoopPreservingObserver(const System& rGen, const EventSet& rInitialHighAlph, EventSet& rHighAlph){
	// Verify if the projection with the given initial alphabet is already a loop-preserving observer
	rHighAlph = rInitialHighAlph;
        rHighAlph.Name("HiAlph");
	FD_DD("LoopPreservingObserver()");
	if(IsLoopPreservingObserver(rGen,rHighAlph) == true)
		return;
		
	// check all combinations of events from the difference set 
	EventSet diffSet = rGen.Alphabet() - rHighAlph;
	EventSet::Iterator eIt = diffSet.Begin();
	std::vector<Idx> diffVector;
	for( ; eIt != diffSet.End(); eIt++) // ordered list of events in the diffSet
		diffVector.push_back(*eIt);
		
	for(Idx numberEvents = 1; numberEvents <= diffVector.size(); numberEvents++){// number events that are chosen in this step
		FD_DD("numberEvents: " + ToStringInteger(numberEvents));
		Idx currentNumberEvents = 1; // number of events under investigation
		Idx currentLocation = 0; // start position for the search for new events
		EventSet chosenEvents;
		if(rec_ComputeLoopPreservingObserver(rGen,rInitialHighAlph,rHighAlph,diffVector,numberEvents,currentNumberEvents,currentLocation,chosenEvents) == true)
			break;
	}
        // fix name
        rHighAlph.Name("HiAlph");
}

// rec_ComputeLoopPreservingObserver(rGen,
bool rec_ComputeLoopPreservingObserver(const System& rGen, const EventSet& rInitialHighAlph, EventSet& rHighAlph, const std::vector<Idx>& rDiffVector, Idx numberEvents, Idx currentNumberEvents, Idx currentLocation, EventSet chosenEvents){
	FD_DD("rec_ComputeLoopPreservingObserver()");
	bool valid;
	for(Idx i = currentLocation; i < rDiffVector.size(); i++){
		FD_DD("currentNumberEvents: " + ToStringInteger(currentNumberEvents) + "currentLocation: " + ToStringInteger(i) + " event: " + SymbolTable::GlobalEventSymbolTablep()->Symbol(rDiffVector[i]));
		chosenEvents.Insert(rDiffVector[i]);
		rHighAlph = rInitialHighAlph + chosenEvents;
		if(currentNumberEvents == numberEvents){// enough events found
			valid = IsLoopPreservingObserver(rGen,rHighAlph);
			if(valid == true){
				return true;
			}
		}
		else if(rDiffVector.size() - 1 - i < numberEvents - currentNumberEvents){ // not enough events left to find numberEvents 
			return false;
		}
		else{// go to the next level to add events
			FD_DD("currentLevel: " + ToStringInteger(i));
			valid = rec_ComputeLoopPreservingObserver(rGen,rInitialHighAlph,rHighAlph,rDiffVector,numberEvents,currentNumberEvents + 1,i + 1, chosenEvents);
			if(valid == true){
				return true;
			}
		}
		chosenEvents.Erase(rDiffVector[i]);
	}
	return false;
}

} // namespace faudes
