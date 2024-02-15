/** @file diag_decentralizeddiagnosis.cpp Functions to test decentralized diagnosability and compute diagnosers.
*/

/*

Copyright Tobias Barthel, Klaus Schmidt, Thomas Moor

*/

#include "diag_decentralizeddiagnosis.h"


using namespace std;

namespace faudes {

///////////////////////////////////////////////////////////////////////////////
// Functions for decentralized diagnosability (co-diagnosability)
///////////////////////////////////////////////////////////////////////////////



// IsCoDiagnosable()
bool IsCoDiagnosable(const System& rGen, const Generator& rSpec, const vector<const EventSet*>& rAlphabets, std::string& rReportString) {
  
	FD_DD("IsCoDiagnosable()");

	// clear report
	rReportString.clear();
	EventSet obsEvents = rGen.ObservableEvents();
#ifdef FAUDES_DEBUG_DIAGNOSIS
	obsEvents.DWrite();
#endif
	Generator verifier; // Graph for co-diagnosability verification
	Idx nullEvent = verifier.InsEvent("nullEvent"); // event that corresponds to the value 0
	Idx negEvent = verifier.InsEvent("negEvent"); // event that corresponds to the value -1 
	Generator verifierTest; // verifier with eeent information
	verifierTest.InjectAlphabet(rGen.Alphabet() ); // FIXME remove this. 
	verifierTest.InsEvent(nullEvent);
	map<Idx, CoVerifierState> stateToVerifierMap;
	map<CoVerifierState, Idx> verifierToStateMap;
	stack<pair<Idx, CoVerifierState> > waitingStates;
	StateSet doneStates;
	// initialize the algorithm 
	EventSet fullAlphabet = rGen.Alphabet();
	EventSet::Iterator eIt, eEndIt;
	eEndIt = fullAlphabet.End();
	Idx newState = verifier.InsInitState();
	verifierTest.InsInitState(newState);
	CoVerifierState newVerifierState = CoVerifierState(rAlphabets.size(), rSpec.InitState(), rGen.InitState(),NORMAL);
	for(Idx i = 0; i < rAlphabets.size(); i++) // all decentralized versions of the specification are initialized
		newVerifierState.mSpec1State[i] = rSpec.InitState();
		
	stateToVerifierMap[newState] = newVerifierState;
	verifierToStateMap[newVerifierState ] = newState;
	waitingStates.push(make_pair(newState, newVerifierState) );
	// extend the verifier graph until no new nodes can be added
	pair<Idx, CoVerifierState> currentState;
	Idx blockingState; // = verifier.InsMarkedState();
	TransSet::Iterator tIt;
	map<CoVerifierState, Idx>::const_iterator vsIt;
	Idx X2;
	bool block = false; // set to true if the BLOCK state is reachable  
	FD_DD("Main loop");
	while(waitingStates.empty() == false){
		// take the first element from the stack
		currentState = waitingStates.top();
		waitingStates.pop();
		doneStates.Insert(currentState.first);
		#ifdef FAUDES_DEBUG_DIAGNOSIS
		std::cout << "currentState: " << ToStringInteger(currentState.first) <<  " VerifierState: (";
		for(unsigned k = 0; k < currentState.second.mSpec1State.size(); k++)
			cout << rSpec.StateName(currentState.second.mSpec1State.at(k)) << " ";
		cout << rSpec.StateName(currentState.second.mSpec2State) << " " << rGen.StateName(currentState.second.mPlantState) << " " << currentState.second.mLabel << ")" << endl;
		#endif
		// go over all possible events
		eIt = fullAlphabet.Begin();
		for(; eIt != eEndIt; eIt++){
			// if the event is not observable, the specifications and the plant can evolve independently
			if(obsEvents.Exists(*eIt) == false){
				for(Idx i = 0; i < rAlphabets.size(); i++){
					tIt = rSpec.TransRelBegin(currentState.second.mSpec1State.at(i),*eIt);
					// transition in decentralized version of Spec for component i exists
					if(tIt != rSpec.TransRelEnd(currentState.second.mSpec1State.at(i),*eIt) ){
						newVerifierState = CoVerifierState(rAlphabets.size(), currentState.second.mSpec2State, currentState.second.mPlantState, NORMAL);
						for(Idx j = 0; j < rAlphabets.size(); j++){ // determine new verifier state
							if(j == i)
								newVerifierState.mSpec1State[j] = tIt->X2;
							else
								newVerifierState.mSpec1State[j] = currentState.second.mSpec1State.at(j);
						}
						if(currentState.second.mLabel == CONFUSED ) // transition in spec1 from normal state// transition in spec1 from confused state
							newVerifierState.mLabel = CONFUSED;
						// check if the new VerifierState already exist snd insert new transitions (rule 1 and 4)

						vsIt = verifierToStateMap.find(newVerifierState);
						// a new state is inserted into the verifier
						if(vsIt == verifierToStateMap.end() ){
			
							newState = verifier.InsState();
							verifierTest.InsState(newState);
							verifier.SetTransition(currentState.first,nullEvent,newState);
							verifierTest.SetTransition(currentState.first,*eIt,newState);
							verifierToStateMap[newVerifierState] = newState;
							stateToVerifierMap[newState] = newVerifierState;
							if(doneStates.Exists(newState) == false)
								waitingStates.push(make_pair(newState,newVerifierState) );
						}
						// a transition to the existing state is added
						else{
							verifierTest.SetTransition(currentState.first,*eIt,vsIt->second);
							verifier.SetTransition(currentState.first,nullEvent,vsIt->second);
							
						}
					}
				}// for(Idx i = 0...
				tIt = rGen.TransRelBegin(currentState.second.mPlantState,*eIt);
				// transition in plant exists
				if(tIt != rGen.TransRelEnd(currentState.second.mPlantState,*eIt) ){
					X2 = tIt->X2;
					newVerifierState = CoVerifierState(rAlphabets.size(), currentState.second.mSpec2State, X2, NORMAL); // prepare the new verifier state
					newVerifierState.mSpec1State = currentState.second.mSpec1State;
					if(currentState.second.mLabel == CONFUSED)
						newVerifierState.mLabel = CONFUSED;
					else{ // current state is NORMAL
						tIt = rSpec.TransRelBegin(currentState.second.mSpec2State,*eIt); 
						if(tIt == rSpec.TransRelEnd(currentState.second.mSpec2State,*eIt) ){ // violation of the specification
							newVerifierState.mLabel = CONFUSED;
						}
						else{ // correct behavior
							newVerifierState.mSpec2State = tIt->X2;
						}
					}
					// check if a new state has to be inserted into the verifier
					vsIt = verifierToStateMap.find(newVerifierState);
					// a new state is inserted into the verifier
					if(vsIt == verifierToStateMap.end() ){
						newState = verifier.InsState();
						verifierTest.InsState(newState);
						verifierToStateMap[newVerifierState] = newState;
						stateToVerifierMap[newState] = newVerifierState;
						if(doneStates.Exists(newState) == false)
							waitingStates.push(make_pair(newState,newVerifierState) );
					}
					else	
						newState = vsIt->second;
					// new transition in the verifier
					if(newVerifierState.mLabel == NORMAL || (currentState.second.mLabel == CONFUSED && newVerifierState.mPlantState == currentState.second.mPlantState) ){ // normal behavior or confused behavior extended in the specification (rule 3 or 4)
						verifierTest.SetTransition(currentState.first,*eIt,newState);
						verifier.SetTransition(currentState.first,nullEvent,newState);
					}
					else{ // faulty behavior extended in the plant (rule 5)
						verifierTest.SetTransition(currentState.first,*eIt,newState);
						verifier.SetTransition(currentState.first,negEvent,newState);
					}
				}
			}// (obsEvents.Exists(*eIt) == false)
			else{
				CoVerifierState unobsVerifierState; // new verifier state in case of unobservable events
				bool allSpecsParticipate = true; // indicates if the plant can proceed together with all specs that have the current event
				TransSet::Iterator plantIt, specIt;
				plantIt= rGen.TransRelBegin(currentState.second.mPlantState, *eIt);
				specIt = rSpec.TransRelBegin(currentState.second.mSpec2State, *eIt);
				if(plantIt != rGen.TransRelEnd(currentState.second.mPlantState, *eIt) ){// there plant has a transition with *eIt
					// the new state is confused 
					if(specIt == rSpec.TransRelEnd(currentState.second.mSpec2State, *eIt) || currentState.second.mLabel == CONFUSED){
						newVerifierState = CoVerifierState(rAlphabets.size(), currentState.second.mSpec2State, plantIt->X2, CONFUSED);
					}
					else{// the new state is normal (spec follows plant)
						newVerifierState = CoVerifierState(rAlphabets.size(), specIt->X2, plantIt->X2, NORMAL);
					}
					// the state only exists if all specifications that observe *eIt can follow
					for(unsigned int i = 0; i < rAlphabets.size(); i++){
						if(rAlphabets.at(i)->Exists(*eIt) == true){ // check if transition exists
							tIt = rSpec.TransRelBegin(currentState.second.mSpec1State.at(i), *eIt);
							if(tIt == rSpec.TransRelEnd(currentState.second.mSpec1State.at(i), *eIt) ){
								allSpecsParticipate = false; // this subsystem can detect that there is a deviationfrom the specification
								break;
							}
							else{ // execute the transition in the respective specification
								newVerifierState.mSpec1State[i] = tIt->X2;
							}
						}
						else
							newVerifierState.mSpec1State[i] = currentState.second.mSpec1State.at(i);
					}
					if(allSpecsParticipate == true){ // a new state has to be inserted in the verifier
						// check if a new state has to be inserted into the verifier
						vsIt = verifierToStateMap.find(newVerifierState);
						// a new state is inserted into the verifier
						if(vsIt == verifierToStateMap.end() ){
							newState = verifier.InsState();
							verifierTest.InsState(newState);
							verifierToStateMap[newVerifierState] = newState;
							stateToVerifierMap[newState] = newVerifierState;
							if(doneStates.Exists(newState) == false)
								waitingStates.push(make_pair(newState,newVerifierState) );
						}
						else	
							newState = vsIt->second;
						// new transition in the verifier
						if(newVerifierState.mLabel == NORMAL){ // normal behavior 
							verifierTest.SetTransition(currentState.first,*eIt,newState);
							verifier.SetTransition(currentState.first,nullEvent,newState);
						}
						else{ // faulty behavior extended in the plant and unnoticed by the specification
							verifierTest.SetTransition(currentState.first,*eIt,newState);
							verifier.SetTransition(currentState.first,negEvent,newState);
						}						
					}
				}
				// go through all specifications and execute locally unobservable transitions
				for(unsigned int i = 0; i < rAlphabets.size(); i++){
					if(rAlphabets.at(i)->Exists(*eIt) == false){
						tIt = rSpec.TransRelBegin(currentState.second.mSpec1State.at(i), *eIt);
						if(tIt != rSpec.TransRelEnd(currentState.second.mSpec1State.at(i), *eIt) ){
							newVerifierState = currentState.second;
							newVerifierState.mSpec1State[i] = tIt->X2;
							// check if a new state has to be inserted into the verifier
							vsIt = verifierToStateMap.find(newVerifierState);
							// a new state is inserted into the verifier
							if(vsIt == verifierToStateMap.end() ){
								newState = verifier.InsState();
								verifierTest.InsState(newState);
								verifierToStateMap[newVerifierState] = newState;
								stateToVerifierMap[newState] = newVerifierState;
								if(doneStates.Exists(newState) == false)
									waitingStates.push(make_pair(newState,newVerifierState) );
							}
							else	
								newState = vsIt->second;

							verifierTest.SetTransition(currentState.first,*eIt,newState);
							verifier.SetTransition(currentState.first,nullEvent,newState);
						}
					}
				}
			}
		}// for(; *eIt ...)
		// check if the Block state is reachable 
		if(rGen.TransRelBegin(currentState.second.mPlantState) == rGen.TransRelEnd(currentState.second.mPlantState) && currentState.second.mLabel == CONFUSED){
			blockingState = verifier.InsMarkedState();
			verifier.SetTransition(currentState.first,nullEvent,blockingState);
			verifierTest.InsMarkedState(blockingState);
			verifierTest.SetTransition(currentState.first,nullEvent,blockingState);
			FD_DD("Blocking State Reachable");
			block = true;
		}
	} // while(waiting...)
	#ifdef FAUDES_DEBUG_DIAGNOSIS
	if(verifier.Size() < 200){
		verifier.GraphWrite("data/verifier.png");
		verifierTest.GraphWrite("data/verifierTest.png");
	}
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


// DecentralizedDiagnoser(rGsubs, rKsubs, rDiagsubs, rReportString)
bool DecentralizedDiagnoser(const System& rGen, const Generator& rSpec, const std::vector<const EventSet*>& rAlphabets, std::vector<Diagnoser*>& rDiags, std::string& rReportString){
  
  FD_DD("DecentralizedDiagnoser()");
  System copyGen = rGen;
  rDiags.clear();
  Diagnoser *newGen;
  // clear report
  rReportString.clear();
  // Verify Codiagnosability
  bool diagnosable = IsCoDiagnosable(rGen,rSpec,rAlphabets,rReportString);
  // Compute diagnoser for each local site
  for(unsigned int i = 0; i < rAlphabets.size(); i++){
		// modify observable events of copyGen according to the local observation
		copyGen.ClrObservable(copyGen.Alphabet() );
		copyGen.SetObservable(*rAlphabets.at(i) );
		newGen = new Diagnoser;
		rDiags.push_back(newGen );
		LanguageDiagnoser(copyGen,rSpec,*rDiags[i]);
  }

  return diagnosable;
}

// DecentralizedDiagnoser(rGsubs, rKsubs, rDiagsubs, rReportString)
void DecentralizedModularDiagnoser(const std::vector<const System*>& rGens, const Generator& rSpec, std::vector<Diagnoser*>& rDiags, std::string& rReportString){
	Generator projSpec;
	Diagnoser *newGen;
  // clear report
  rReportString.clear();
  // Compute diagnoser for each local site
  for(unsigned int i = 0; i < rGens.size(); i++){
		// generate local versioin of the specification
		Project(rSpec,rGens.at(i)->Alphabet(),projSpec);
		newGen = new Diagnoser;
		rDiags.push_back(newGen);
		LanguageDiagnoser(*rGens.at(i),projSpec,*rDiags[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////
// RTI wrapper
///////////////////////////////////////////////////////////////////////////////

// IsCoDiagnosable()
bool IsCoDiagnosable(const System& rGen, const Generator& rSpec, const EventSetVector& rAlphabets){
  std::string ignore;
  // reorganize as std vector
  std::vector<const EventSet*> alphabets;
  Idx i; 

  for(i = 0; i < rAlphabets.Size(); ++i) 
    alphabets.push_back(&rAlphabets.At(i));
  return IsCoDiagnosable(rGen, rSpec, alphabets, ignore);
}

// DecentralizedDiagnoser()
bool DecentralizedDiagnoser(const System& rGen, const Generator& rSpec, const EventSetVector& rAlphabets, GeneratorVector& rDiags){
  std::string ignore;
  // reorganize as std vector
  std::vector<const EventSet*> alphabets;
  std::vector<Diagnoser*> diags;
  Idx i; 

  for(i = 0; i < rAlphabets.Size(); ++i) 
    alphabets.push_back(&rAlphabets.At(i));

  bool ok = DecentralizedDiagnoser(rGen,rSpec,alphabets,diags,ignore);
  for(i = 0; i < rAlphabets.Size(); ++i)
	rDiags.Append(diags.at(i) );
  rDiags.TakeOwnership();
  
  return ok;
}

void DecentralizedModularDiagnoser(const SystemVector& rGens, const Generator& rSpec, GeneratorVector& rDiags){
  std::string ignore;
  // reorganize as std vector
  std::vector<const System*> generators;
  std::vector<Diagnoser*> diags;
  Idx i; 

  for(i = 0; i < rGens.Size(); ++i) 
    generators.push_back(&rGens.At(i));

  DecentralizedModularDiagnoser(generators,rSpec,diags,ignore);
  for(i = 0; i < rGens.Size(); ++i)
	rDiags.Append(diags.at(i) );
  rDiags.TakeOwnership();
}



} // namespace faudes
