/** @file diag_eventdiagnosis.cpp
Functions to check a system's event-diagnosability and computation of an event-diagnoser. Covers diagnosability with respect to failure events (diagnosability, I-diagnosability).
*/

#include "diag_eventdiagnosis.h"

using namespace std;

namespace faudes {


// IsEventDiagnosable()
bool IsEventDiagnosable(const System& rGen, const AttributeFailureTypeMap& rFailureTypeMap, string& rReportString) {
  Diagnoser genGobs;
  System genGd;
  map<pair<Idx,Idx>,Idx> reverseCompositionMap;
  map<pair<Idx,Idx>,Idx>::iterator rcmIt;
  StateSet::Iterator stateIt;
  TaNameSet<AttributeFailureEvents>::Iterator ftIt;
  EventSet::Iterator evIt;

  // reset report  
  rReportString.clear();
  
  FD_DD("IsEventDiagnosable()");
  // TODO: throw exception, dont report
  // check for indicator events (which should not exist)
  for (ftIt = rFailureTypeMap.mFailureTypeMap.Begin(); ftIt != rFailureTypeMap.mFailureTypeMap.End(); ftIt++) {
    if (!rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).mIndicatorEvents.Empty()) {
      FD_DD("IsEventDiagnosable(): Warning: Existing indicator events are ignored! If you want to check for I-diagnosability use IsIdiagnosable() instead.");
      rReportString.append("IsEventDiagnosable(): Warning: Existing indicator events are ignored! If you want to check for I-diagnosability use IsIdiagnosable() instead.\n");
      break;
    }
  }
  
  // check if assumptions are met
  // TODO: convention: "Check" rather than "Meet"
  if (!MeetsDiagnosabilityAssumptions(rGen, rFailureTypeMap, rReportString)) {
    return false;
  }    
  
  // Implementation of Remark 2: Applying Algorithm 1 for each failure type on its own
  for (ftIt = rFailureTypeMap.mFailureTypeMap.Begin(); ftIt != rFailureTypeMap.mFailureTypeMap.End(); ftIt++) {  
    FD_DD("Testing for failure type " + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) \
       + " with failures " + rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).mFailureEvents.ToString());
    
    // 3 Steps of Algorithm 1
    // Step 1: Generate G_o
    FD_DD("__Step 1__");
    ComputeGobs(rGen, rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt), rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).mFailureEvents, genGobs);
    //genGobs.Write("tmp_Gobs_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".gen");
    //genGobs.GraphWrite("tmp_Gobs_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".png");
    
    // Step 2: Generate G_d = G_o || G_o
    // State names/attributes are not copied to new generator G_d
    // but can be obtained via reverse composition map and the corresponding elements of G_o
    FD_DD("__Step 2__");
    ComputeGd(genGobs, reverseCompositionMap, genGd);
    //genGd.Write("tmp_Gd_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".gen");
    //genGd.GraphWrite("tmp_Gd_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".png");
        
    // Step 3: Check for cycles in G_d which contain states with different failure labels
    FD_DD("__Step 3__");
    if(ExistsViolatingCyclesInGd(genGd, genGobs, reverseCompositionMap, rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt), rReportString)) {
      //genGd.Write("tmp_Gd_pruned_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".gen");    
      //genGd.GraphWrite("tmp_Gd_pruned_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".png");    
      return false;
    } else {
      //genGd.Write("tmp_Gd_pruned_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".gen");    
      //genGd.GraphWrite("tmp_Gd_pruned_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".png");
    }
    
  }
  return true;
}

// rti function interface
bool IsEventDiagnosable(const System& rGen, const AttributeFailureTypeMap& rFailureTypeMap) {
  std::string ignore;
  return IsEventDiagnosable(rGen,rFailureTypeMap,ignore);
}


// IsIdiagnosable()
bool IsIndicatorEventDiagnosable(const System& rGen, const AttributeFailureTypeMap& rFailureTypeMap, string& rReportString) {
  Diagnoser genGobs;
  System genGd;
  map<pair<Idx,Idx>,Idx> reverseCompositionMap;
  map<pair<Idx,Idx>,Idx>::iterator rcmIt;
  StateSet::Iterator stateIt;
  TaNameSet<AttributeFailureEvents>::Iterator ftIt;
  EventSet::Iterator evIt;
  rReportString.clear();
  
  FD_DD("IsIndicatorEventDiagnosable()");
  // check for indicator events (which should exist)
  for (ftIt = rFailureTypeMap.mFailureTypeMap.Begin(); ftIt != rFailureTypeMap.mFailureTypeMap.End(); ftIt++) {
    if (rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).mIndicatorEvents.Empty()) {
      FD_DD("IsIndicatorEventDiagnosable(): Warning: There are no indicator events for failure type " << rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) << "!");
      rReportString.append("IsIndicatorEventDiagnosable(): Warning: There are no indicator events for failure type " + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + "!\n");
    }
  }
  
  // check if assumptions are met
  if (!MeetsDiagnosabilityAssumptions(rGen, rFailureTypeMap, rReportString)) {
    return false;
  }    
  
  // Implementation of Remark 2: Applying Algorithm 1 for each failure type on its own
  for (ftIt = rFailureTypeMap.mFailureTypeMap.Begin(); ftIt != rFailureTypeMap.mFailureTypeMap.End(); ftIt++) {  
    FD_DD("Testing for failure type " + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) \
       + " with failures/indicators " + rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).ToString());
    
    // 3 Steps of Algorithm 1
    // Step 1: Generate G_o
    FD_DD("__Step 1__");
    ComputeGobs(rGen, rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt), rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).mFailureEvents, genGobs);
    //genGobs.Write("tmp_I_Gobs_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".gen");
    //genGobs.GraphWrite("tmp_I_Gobs_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".png");
    
    // Step 2: Generate G_d = G_o || G_o
    // State names/attributes are not copied to new generator G_d
    // but can be obtained via reverse composition map and the corresponding elements of G_o
    FD_DD("__Step 2__");
    ComputeGd(genGobs, reverseCompositionMap, genGd);
    //genGd.Write("tmp_I_Gd_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".gen");
    //genGd.GraphWrite("tmp_I_Gd_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".png");
    
    // Additionally for I-diagnosability: Remove all traces which do not contain a failure event followed by an indicator event
    FD_DD("Removing all traces not containing an indicator event " + rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).mIndicatorEvents.ToString());
    TrimNonIndicatorTracesOfGd(genGd, genGobs, *ftIt, rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).mIndicatorEvents, reverseCompositionMap);
    //genGd.Write("tmp_I_Gd_iTraces_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".gen");
    //genGd.GraphWrite("tmp_I_Gd_iTraces_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".png");
    
    // Step 3: Check for cycles in G_d which contain states with different failure labels
    FD_DD("__Step 3__");
    if(ExistsViolatingCyclesInGd(genGd, genGobs, reverseCompositionMap, rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt), rReportString)) {
      //genGd.Write("tmp_I_Gd_pruned_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".gen");    
      //genGd.GraphWrite("tmp_I_Gd_pruned_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".png");    
      return false;
    } else {
      //genGd.Write("tmp_I_Gd_pruned_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".gen");    
      //genGd.GraphWrite("tmp_I_Gd_pruned_" + rFailureTypeMap.mFailureTypeMap.SymbolicName(*ftIt) + ".png");
    }
    
  }
  return true;
}



// rti function interface
bool IsIndicatorEventDiagnosable(const System& rGen, const AttributeFailureTypeMap& rFailureTypeMap) {
  std::string ignore;
  return IsEventDiagnosable(rGen,rFailureTypeMap,ignore);
}


// MeetsDiagnosabilityAssumptions()
bool MeetsDiagnosabilityAssumptions(const System& rGen, const AttributeFailureTypeMap& rFailureTypeMap, string& rReportString) {
  TaNameSet<AttributeFailureEvents>::Iterator ftIt;
  EventSet::Iterator evIt;
  
  // check if failure and indicator events are part of the generators alphabet
  for (ftIt = rFailureTypeMap.mFailureTypeMap.Begin(); ftIt != rFailureTypeMap.mFailureTypeMap.End(); ftIt++) {
    // check if all failures are valid events of generator
    for (evIt = rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).mFailureEvents.Begin(); evIt != rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).mFailureEvents.End(); evIt++) {
      if (!rGen.Alphabet().Exists(*evIt)) {
        stringstream errstr;
        errstr << "Failure " << rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).mFailureEvents.SymbolicName(*evIt) << " is not in alphabet of generator!" << endl;
        throw Exception("MeetsDiagnosabilityAssumptions()", errstr.str(), 302);
      }
    }
    // check if all indicator events are valid events of generator
    for (evIt = rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).mIndicatorEvents.Begin(); evIt != rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).mIndicatorEvents.End(); evIt++) {
      if (!rGen.Alphabet().Exists(*evIt)) {
        stringstream errstr;
        errstr << "Indicator " << rFailureTypeMap.mFailureTypeMap.Attribute(*ftIt).mIndicatorEvents.SymbolicName(*evIt) << " is not in alphabet of generator!" << endl;
        throw Exception("MeetsDiagnosabilityAssumptions()", errstr.str(), 303);
      }
    }
  }

  // Assumption A1: Liveness
  if (!IsLive(rGen, rReportString)) {
    FD_DD("MeetsDiagnosabilityAssumptions(): Generator is not live!");
    rReportString.append("Generator is not live!\n");
    return false;
  }

  // Assumption A3: All failure events are unobservable
  if (!FailuresUnobservable(rGen, rFailureTypeMap, rReportString)) {
    FD_DD("MeetsDiagnosabilityAssumptions(): Not all failure events are unobservable!");
    rReportString.append("Not all failure events are unobservable!\n");
    return false;
  }
  
  // Assumption A2: No cycles of unobservable events
  if (CycleOfUnobsEvents(rGen, rReportString)) {
    FD_DD("MeetsDiagnosabilityAssumptions(): Generator contains cycle of unobservable events!");
    rReportString.append("Generator contains cycle of unobservable events!\n");
    return false;
  }
  
  // otherwise
  return true;
}

// ConvertParallelCompositionMap()
void ConvertParallelCompositionMap(  const map<pair<Idx,Idx>,Idx>& rReverseCompositionMap, 
                        map<Idx,pair<Idx,Idx> >& rCompositionMap) {
  // invert rReverseCompositionMap (possible, as map is expected to be from parallel composition
  // and thus must be bijective)
  map<pair<Idx,Idx>,Idx>::const_iterator iter;
  
  FD_DD("ConvertParallelCompositionMap()");
  rCompositionMap.clear();
  for(iter = rReverseCompositionMap.begin(); iter != rReverseCompositionMap.end(); iter++) {
    rCompositionMap.insert(make_pair(iter->second,iter->first));
  }
}

// IsLive()
// TODO: inefficient set of active vents; have method in Generator
bool IsLive(const System& rGen, string& rReport) {
  StateSet states;
  StateSet::Iterator it;
  FD_DD("IsLive()");
  states = rGen.States();
  for (it = states.Begin(); it != states.End(); it++) {
    if (rGen.ActiveEventSet(*it).Empty()) {
      rReport.append("Missing transition at state " + ToStringInteger(*it) + " --> ");
      return false;
    }
  }
  return true;
}

// CycleOfUnobsEvents()
// TODO: inefficient copy; have method in Generator
bool CycleOfUnobsEvents(const System& rGen, string& rReport) {
  TransSet transitionsToDelete;
  TransSet::Iterator it;
  System genCopy;
  
  FD_DD("CycleOfUnobsEvents()");
  transitionsToDelete.Clear();
  // make a copy of generator
  genCopy = rGen;
  // parse through all its transitions and delete the observable ones
  for (it = genCopy.TransRelBegin(); it != genCopy.TransRelEnd(); it++) {
    if (genCopy.Observable(it->Ev)) {
      transitionsToDelete.Insert(*it);
    }
  }
  for (it = transitionsToDelete.Begin(); it != transitionsToDelete.End(); it++) {
    FD_DD("delete " << it->X1 << " --" << genCopy.EventName(it->Ev) << "--> " << it->X2);
    genCopy.ClrTransition(*it);
  }
  // check for cycles within the remaining unobservable events
  return ExistsCycle(genCopy, rReport);
}

// FailuresUnobservable()
bool FailuresUnobservable(const System& rGen, const AttributeFailureTypeMap& rFailureTypeMap, string& rReport) {
  EventSet failures, unobsEvents;
  EventSet::Iterator evIt;
  
  FD_DD("FailuresUnobservable()");
  failures = rFailureTypeMap.AllFailureEvents();
  unobsEvents = rGen.UnobservableEvents();
  for (evIt = failures.Begin(); evIt != failures.End(); evIt++) {
    if (!unobsEvents.Exists(*evIt)) {
      FD_DD("FailuresUnobservable(): Failure event \"" << failures.SymbolicName(*evIt) << "\" is not unobservable in generator!");
      rReport.append("Failure event \"" + failures.SymbolicName(*evIt) + "\" is observable in generator --> ");
      return false;
    }
  }
  return true;
}

// ExistsCycle()
bool ExistsCycle(const System& rGen, string& rReport) {
  StateSet todo, path;
  StateSet::Iterator stateIt;
  
  FD_DD("ExistsCycle()");
  todo = rGen.States();
  path.Clear();
  for (stateIt = rGen.InitStatesBegin(); stateIt != rGen.InitStatesEnd(); ++stateIt) {
    FD_DD("Start cycle search at initial state");
    if (ExistsCycleSearch(rGen, todo, *stateIt, path, rReport)) {
      return true;
    }
  }
  while (!todo.Empty()) {
    FD_DD("Start cycle search at some state..");
    if (ExistsCycleSearch(rGen, todo, *todo.Begin(), path, rReport)) {
      return true;
    }
  }
  return false;
}

// ExistsCycleSearch()
bool ExistsCycleSearch(const System& rGen, StateSet& rTodo, Idx currState, StateSet statesOnPath, string& rReport) {
  StateSet successors, newStatesOnPath;
  StateSet::Iterator stateIt;
  
  FD_DD("ExistsCycleSearch() for State " << currState);
  rTodo.Erase(currState);
  statesOnPath.Insert(currState);
  
  successors = rGen.SuccessorStates(currState);
  // parse through active state set of currState
  for (stateIt = successors.Begin(); stateIt != successors.End(); stateIt++) {
    if (statesOnPath.Exists(*stateIt)) {
      FD_DD("Cycle found at state " << *stateIt);
      rReport.append("Cycle found at state " + ToStringInteger(*stateIt) + " --> ");
      return true;
    }
    // call ExistsCycleSearch() for next successor state and with updated state set newStatesOnPath
    newStatesOnPath.Clear();
    newStatesOnPath.InsertSet(statesOnPath);
    newStatesOnPath.Insert(*stateIt);
    if (ExistsCycleSearch(rGen, rTodo, *stateIt, newStatesOnPath, rReport)) {
      return true;
    }
  }
  return false;
}

// CycleStartStates()
void CycleStartStates(const System& rGen, StateSet& rCycleOrigins) {
  StateSet todo, path;
  
  FD_DD("ExistsCycle()");
  rCycleOrigins.Clear();
  todo = rGen.States();
  if (!rGen.InitStatesEmpty()) {
    FD_DD("Start cycle search at initial state");
    path.Clear();
    CycleStartStatesSearch(rGen, todo, rGen.InitState(), path, rCycleOrigins);
  }
  while (!todo.Empty()) {
    FD_DD("Start cycle search at some state..");
    path.Clear();
    CycleStartStatesSearch(rGen, todo, *todo.Begin(), path, rCycleOrigins);    
  }
  return;
}

// CycleStartStatesSearch()
void CycleStartStatesSearch(const System& rGen, StateSet& rTodo, Idx currState, StateSet statesOnPath, StateSet& rCycleOriginStates) {
  StateSet successors, newStatesOnPath;
  StateSet::Iterator stateIt;
  
  FD_DD("CycleStartStatesSearch() for State " << currState);
  rTodo.Erase(currState);
  statesOnPath.Insert(currState);
  
  successors = rGen.SuccessorStates(currState);
  // parse through active state set of currState
  for (stateIt = successors.Begin(); stateIt != successors.End(); stateIt++) {
    if (statesOnPath.Exists(*stateIt)) {
      FD_DD("Cycle found at state " << *stateIt);
      rCycleOriginStates.Insert(*stateIt);
      return;
    }
    // call ExistsCycleSearch() for next successor state and with updated state set newStatesOnPath
    newStatesOnPath.Clear();
    newStatesOnPath.InsertSet(statesOnPath);
    newStatesOnPath.Insert(*stateIt);
    CycleStartStatesSearch(rGen, rTodo, *stateIt, newStatesOnPath, rCycleOriginStates);
  }
  return;
}

// ExistsViolatingCyclesInGd()
bool ExistsViolatingCyclesInGd(System& rGd, const Diagnoser& rGobs, map<pair<Idx,Idx>,Idx>& rReverseCompositionMap, const string& rFailureType, string& rReportString) {
  map<pair<Idx,Idx>,Idx>::iterator rcmIt;
  const TaIndexSet<DiagLabelSet>* fLabel1;
  const TaIndexSet<DiagLabelSet>* fLabel2;
  TaIndexSet<DiagLabelSet>::Iterator fL1Begin;
  TaIndexSet<DiagLabelSet>::Iterator fL2Begin;

  FD_DD("ExistsViolatingCyclesInGd()");
  // Therefore parse through reverse composition map
  for (rcmIt = rReverseCompositionMap.begin(); rcmIt != rReverseCompositionMap.end();) {
    FD_DD("state " << rcmIt->second << " (" << rcmIt->first.first << "," << rcmIt->first.second << ")");
    // if both states in G_o are equal or just contain the same failure label: delete corresponding state in G_d
    if (rcmIt->first.first == rcmIt->first.second) {
      FD_DD(" --> delete (same G_o state)");
      rGd.DelState(rcmIt->second);
      rReverseCompositionMap.erase(rcmIt++);
    } else {
      fLabel1 = rGobs.StateAttribute(rcmIt->first.first).DiagnoserStateMapp();
      fLabel2 = rGobs.StateAttribute(rcmIt->first.second).DiagnoserStateMapp();
      fL1Begin = fLabel1->Begin();
      fL2Begin = fLabel2->Begin();
      if (fLabel1->Attribute(*fL1Begin) == fLabel2->Attribute(*fL2Begin)) {
        FD_DD(" --> delete (same failure label)");
        rGd.DelState(rcmIt->second);
        rReverseCompositionMap.erase(rcmIt++);
      } else {
        ++rcmIt;
      }
    }
    FD_DD("");
  }
  // if there exists a cycle in the remainder graph the system rGen is not diagnosable
  if (ExistsCycle(rGd,rReportString)) {
    FD_DD("Detected cycle in G_d");
    rReportString.append("While checking diagnosability for failure type " + rFailureType + ": " + \
                  "G_d contains a cycle of states with unequal failure labels, i.e. there exists an " + \
                  rFailureType + "-indeterminate cycle in the diagnoser.\n");
    return true;
  }
  return false;
}

// ComputeGobs()
void ComputeGobs(const System& rOrigGen, const string& rFailureType, const EventSet& rFailureEvents, Diagnoser& rGobs) {
  AttributeFailureTypeMap singleFailureTypeMap;
  singleFailureTypeMap.Clear();
  singleFailureTypeMap.AddFailureTypeMapping(rFailureType, rFailureEvents);
  ComputeGobs(rOrigGen, singleFailureTypeMap, rGobs);
}

// ComputeGobs()
void ComputeGobs(const System& rOrigGen, const AttributeFailureTypeMap& rAttrFTMap, Diagnoser& rGobs) {
  EventSet failureEvents;
  EventSet gObservableEvents, gUnobservableEvents;
  StateSet newGobsStates;
  Idx currDState = 0;
  Idx nextDState = 0;
    
  FD_DD("ComputeGobs()");
  // check if FailureTypeMap is empty
  if (rAttrFTMap.Empty()) {
    FD_DD("WARNING - ComputeGobs(): failure type map is empty!");
  }
  
  // clear Gobs
  rGobs.Clear();
  rGobs.ClearAttributes();
  
  // copy attribute failure type map to Gobs
  rGobs.GlobalAttribute(rAttrFTMap);
  
  // get observable events of original generator
  gObservableEvents = rOrigGen.ObservableEvents();
  FD_DD("Observable events: " << gObservableEvents.ToString());
  
  // get unobservable events of original generator
  gUnobservableEvents = rOrigGen.UnobservableEvents();
  FD_DD("Unobservable events: " <<  gUnobservableEvents.ToString());

  // copy all failure events into one single EventSet
  failureEvents = rGobs.GetAllFailureEvents();
  FD_DD("Failure events: " << failureEvents.ToString());
  
  // create initial state of Gobs and its attribute with label "normal"
  #ifdef FAUDES_CHECKED
  if(rOrigGen.InitStatesSize() != 1) {    
    std::stringstream errstr;
    errstr << "original generator has no unique initial state" << std::endl;
    throw Exception("ComputeGobs()", errstr.str(), 301);
  }
  #endif
  Idx gInitState = rOrigGen.InitState();
  currDState = rGobs.InsInitState();
  newGobsStates.Insert(currDState);
  rGobs.InsStateLabelMapping(currDState,gInitState,DiagLabelSet::IndexOfLabelN());
  
  Idx gStateEstimate = 0;
  Idx newState = 0;
  
  map<Idx,multimap<Idx,DiagLabelSet> > reachMap; // maps executable events to all reachable states and occuring relative failure types
  map<Idx,multimap<Idx,DiagLabelSet> >::iterator it;
  
  multimap<Idx,DiagLabelSet>::iterator mmit, mmit2;
  
  pair<Idx,DiagLabelSet> reachMapPair;  

  TransSet transitions;
  DiagLabelSet oldLabel, newLabel, occFailureTypes;
  DiagLabelSet::Iterator labelIt;
  StateSet gObsStates;
  StateSet::Iterator stateIt;
  EventSet activeEvents;
  AttributeDiagnoserState newAttr;
  AttributeDiagnoserState currDStateAttr;
  TaIndexSet<DiagLabelSet> currDStateMap; 
  TaIndexSet<DiagLabelSet>::Iterator currDStateMapIt;
  
  // parse through new Gobs states
  while (!newGobsStates.Empty()) {
    // set current Gobs state
    currDState = *newGobsStates.Begin();
    
    currDStateAttr = rGobs.StateAttribute(currDState);
    currDStateMap = currDStateAttr.DiagnoserStateMap();
    
    // parse through state estimates of current Gobs state
    for(currDStateMapIt = currDStateMap.Begin(); currDStateMapIt != currDStateMap.End(); ++ currDStateMapIt){
      gStateEstimate = *currDStateMapIt;
      
      //  generate reachMap for current state estimate
      ComputeReachability(rOrigGen, gUnobservableEvents, failureEvents, gStateEstimate, rAttrFTMap, reachMap);

      #ifdef FAUDES_DEBUG_DIAGNOSIS
      FD_DD(endl << "reachMap: ");
      for (it = reachMap.begin(); it != reachMap.end(); it++) {
        //print reachMap for current event
        FD_DD("_" << rOrigGen.EventName(it->first) << " ("<< it->second.size() << " state estimates)");
        for (mmit = it->second.begin(); mmit != it->second.end(); mmit++) {
          FD_DD(mmit->first << " " << mmit->second.ToString());
        }
      }
      FD_DD("");
      #endif
      
      // parse through reachMap (eventwise)
      for (it = reachMap.begin(); it != reachMap.end(); it++) {
        //print reachMap for current event
        #ifdef FAUDES_DEBUG_DIAGNOSIS
        FD_DD(endl << "_" << rOrigGen.EventName(it->first) << " ("<< it->second.size() << " state estimates)");
        for (mmit = it->second.begin(); mmit != it->second.end(); mmit++) {
          FD_DD(mmit->first << " " << mmit->second.ToString());
        }
        #endif
        
        // get label set of current state estimate
        oldLabel = currDStateMap.Attribute(*currDStateMapIt);
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

          // if newAttr equals any existing state attribute than we create a transition to this very state
          gObsStates = rGobs.States();        
          stateIt = gObsStates.Begin();
          while (stateIt != gObsStates.End()) {
            if (newAttr == rGobs.StateAttribute(*stateIt)) {
              FD_DD("realising as back- or self-loop to existing state " << *stateIt);
              rGobs.InsEvent(it->first);
              rGobs.SetTransition(currDState,it->first,*stateIt);
              break;
            }
            stateIt++;
          }
          // if newAttribute is new to Gobs
          if (stateIt == gObsStates.End()) {
            // create new Gobs state and add it to new states
            nextDState = rGobs.InsState();
            FD_DD("Create new state " << nextDState << " and transition "  << currDState << " --" << rOrigGen.EventName(it->first) << "--> " << nextDState);
            newGobsStates.Insert(nextDState);
            rGobs.InsEvent(it->first);
            rGobs.SetTransition(currDState,it->first,nextDState);
            rGobs.StateAttribute(nextDState, newAttr);
            
            FD_DD("Print Gobs label of state " << nextDState);
            FD_DD(rGobs.StateAttributep(nextDState)->ToString());
          }
        }
      }
    }
    
    activeEvents = rGobs.ActiveEventSet(currDState);
    transitions = rGobs.ActiveTransSet(currDState);

    // delete current Gobs state from new states
    newGobsStates.Erase(currDState);  
  }
}

// ComputeGd()
void ComputeGd(const Diagnoser& rGobs, map<pair<Idx,Idx>,Idx>& rReverseCompositionMap, System& rGd) {
  string stateName;
  map<pair<Idx,Idx>,Idx>::iterator rcmIt;
  rReverseCompositionMap.clear();
  
  FD_DD("ComputeGd()");
  FD_DD("Performing parallel compostion of G_o with itself ...");
  Parallel(rGobs, rGobs, rReverseCompositionMap, rGd);
  FD_DD("Writing G_d state names ...");
  for (rcmIt = rReverseCompositionMap.begin(); rcmIt != rReverseCompositionMap.end(); rcmIt++) {
    stateName = ToStringInteger(rcmIt->second) + "{" + \
      rGobs.SAStr(rcmIt->first.first) + "'" + \
      rGobs.SAStr(rcmIt->first.second) +"}";
    rGd.StateName(rcmIt->second, stateName);
  }
}

// TrimNonIndicatorTracesOfGd()
void TrimNonIndicatorTracesOfGd(System& rGd, const Diagnoser& rGobs, const Idx rFailureType,
                const EventSet& rIndicatorEvents, const map<pair<Idx,Idx>,Idx>& rReverseCompositionMap) {
  StateSet statesDone;
  map<Idx,pair<Idx,Idx> > CompositionMap;

  FD_DD("TrimNonIndicatorTracesOfGd()");
  ConvertParallelCompositionMap(rReverseCompositionMap, CompositionMap);
  statesDone.Clear();
  TrimNonIndicatorTracesOfGdRecursive(rGd, rGobs, rFailureType, rIndicatorEvents, CompositionMap, rGd.InitState(), statesDone);
}

// TrimNonIndicatorTracesOfGdRecursive()
void TrimNonIndicatorTracesOfGdRecursive(System& rGd, const Diagnoser& rGobs, const Idx rFailureType, 
                            const EventSet& rIndicatorEvents, map<Idx,pair<Idx,Idx> >& rCompositionMap,
                            Idx state, StateSet& rStatesDone) {
  TransSet trans, backTrans;
  TransSet::Iterator it;
  Idx nextState = 0;
  const TaIndexSet<DiagLabelSet>* pDiagState1;
  const TaIndexSet<DiagLabelSet>* pDiagState2;
  map<Idx,pair<Idx,Idx> >::iterator compMapIt;
  bool failureHasAlreadyOccurred = false;
  
  FD_DD("TrimNonIndicatorTracesOfGdRecursive() for state " + ToStringInteger(state));
  
  // return if this state has already been processed
  if (rStatesDone.Exists(state)) {
    return;
  }
  rStatesDone.Insert(state);
  trans = rGd.ActiveTransSet(state);
  
  // if state has no successors than delete it
  if (trans.Empty()) {
    rGd.DelState(state);
    FD_DD("removing state " << state);
    return;
  }
  
  // If there exists a self-loop of an indicator event (after the occurrence of a failure event), return.
  // This needs to be checked because otherwise the following for-loop could cut parts of the future traces before noticing the self-loop.
  for (it = trans.Begin(); it != trans.End(); it++) {
    if (it->X1 == it->X2) {
      if (rIndicatorEvents.Exists(it->Ev)) {
        compMapIt = rCompositionMap.find(it->X2);
        pDiagState1 = rGobs.StateAttribute(compMapIt->second.first).DiagnoserStateMapp();
        pDiagState2 = rGobs.StateAttribute(compMapIt->second.second).DiagnoserStateMapp();
        if (*(pDiagState1->Attribute(*(pDiagState1->Begin())).mDiagLabels.Begin()) == rFailureType) {
          return;
        } 
        if (*(pDiagState2->Attribute(*(pDiagState2->Begin())).mDiagLabels.Begin()) == rFailureType) {
          return;
        }
      }
    }
  }
  
  // parse through transitions of active transition set
  for (it = trans.Begin(); it != trans.End(); it++) {
    nextState = it->X2;
    failureHasAlreadyOccurred = false;
    
    // if event is an indicator: check if corresponding failure type has already occurred
    // by checking if there exists a corresponding failure in the _next_ failure label in G_d
    // (we use the _next_ label (and not the last one) to make sure not to miss out failures that occur immediately before the indicator event)
    if (rIndicatorEvents.Exists(it->Ev)) {    
      compMapIt = rCompositionMap.find(nextState);
      pDiagState1 = rGobs.StateAttribute(compMapIt->second.first).DiagnoserStateMapp();
      pDiagState2 = rGobs.StateAttribute(compMapIt->second.second).DiagnoserStateMapp();
      if (*(pDiagState1->Attribute(*(pDiagState1->Begin())).mDiagLabels.Begin()) == rFailureType) {
        failureHasAlreadyOccurred = true;
      } 
      if (*(pDiagState2->Attribute(*(pDiagState2->Begin())).mDiagLabels.Begin()) == rFailureType) {
        failureHasAlreadyOccurred = true;
      }
    }
    
    // if transition event is not an indicator event or there did not occur a failure _before_ the indicator
    if (!rIndicatorEvents.Exists(it->Ev) || !failureHasAlreadyOccurred) {
      // remove transition
      rGd.ClrTransition(*it);
      FD_DD("removing transition " << it->X1 << "--" << rGd.EventName(it->Ev) << "-->" << it->X2 );
      // remove state if it does not have any transitions left
      if (rGd.ActiveTransSet(state).Empty()) {
        rGd.DelState(state);
        FD_DD("removing state " << state );
      }
      // if there do not exist any further transitions form other states into the next state: continue trimming at next state
      backTrans = ActiveBackwardTransSet(rGd, nextState);
      if (backTrans.Empty() || ((backTrans.Size() == 1) && (backTrans.Begin()->X2 == nextState))) {
        TrimNonIndicatorTracesOfGdRecursive(rGd, rGobs, rFailureType, rIndicatorEvents, rCompositionMap, nextState, rStatesDone);
      }      
    }
  }  
}

// ComputeReachability()
void ComputeReachability(const System& rGen, const EventSet& rUnobsEvents, const EventSet& rFailures, Idx State, 
                const AttributeFailureTypeMap& rAttrFTMap, map<Idx,multimap<Idx,DiagLabelSet> >& rReachabilityMap) {
  DiagLabelSet FTonPath;
  
  FD_DD("ComputeReachability() for state " << State << "...");
  rReachabilityMap.clear();
  FTonPath.Clear();
  ComputeReachabilityRecursive(rGen, rUnobsEvents, rFailures, State, rAttrFTMap, rReachabilityMap, FTonPath);
  
  multimap<Idx,DiagLabelSet>::iterator mmLabelIt;
  map<Idx,multimap<Idx,DiagLabelSet> >::iterator it;
  
  #ifdef FAUDES_DEBUG_DIAGNOSIS
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
void ComputeReachabilityRecursive(const System& rGen, const EventSet& rUnobsEvents,
                      const EventSet& rFailures, Idx State, const AttributeFailureTypeMap& rAttrFTMap,
                      map<Idx,multimap<Idx,DiagLabelSet> >& rReachabilityMap, const DiagLabelSet FToccurred) {
  TransSet trans;
  TransSet::Iterator tIt;
  EventSet tmpFailureSet;
  EventSet::Iterator evIt;
  multimap<Idx,DiagLabelSet> stateFailureTypeMap; // maps generator states onto occurred failure types (=labels), part of rReachabilityMap
  multimap<Idx,DiagLabelSet>::iterator mmLabelIt;
  map<Idx,multimap<Idx,DiagLabelSet> >::iterator it;
  Idx failureType;
  DiagLabelSet newFT;
  bool mappingExists;
  
  trans = rGen.ActiveTransSet(State);
  
  FD_DD("ComputeReachabilityRecursive() for state " << State);
  // parse through active transitions of current generator state
  for (tIt = trans.Begin(); tIt != trans.End(); tIt++) {
    FD_DD(tIt->X1 << "--" << rGen.EventName(tIt->Ev) << "-->" << tIt->X2 << " for " << FToccurred.ToString());
    // if current event is unobservable
    if (rUnobsEvents.Exists(tIt->Ev)) {
      // if it is a failure as well add its failure type
      if (rFailures.Exists(tIt->Ev)) {
        FD_DD(rGen.EventName(tIt->Ev) << " is a failure");
        newFT = FToccurred;
        newFT.Erase(DiagLabelSet::IndexOfLabelN());
        failureType = rAttrFTMap.FailureType(tIt->Ev);
        newFT.Insert(failureType);
        FD_DD("new failure path: " << newFT.ToString());
      } else {
        FD_DD(rGen.EventName(tIt->Ev) << " is unobservable but no failure");
        newFT = FToccurred;
      }
      // call ComputeReachabilityRecursive for successor state
      ComputeReachabilityRecursive(rGen, rUnobsEvents, rFailures, tIt->X2, rAttrFTMap, rReachabilityMap, newFT);
    }
    // if current event is observable add failure type path to rReachabilityMap
    else {
      FD_DD(rGen.EventName(tIt->Ev) << " is observable: add it to rReachabilityMap " << FToccurred.ToString());
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
      // if no failure occurred add normal label
      newFT = FToccurred;
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

// ActiveBackwardTransSet()
TransSet ActiveBackwardTransSet(const System& rGen, Idx state) {
  TransSet result;
  TransSetX2EvX1 transByX2;
  TransSetX2EvX1::Iterator it;
  
  rGen.TransRel(transByX2);
  for (it = transByX2.BeginByX2(state); it != transByX2.EndByX2(state); it++) {
    result.Insert(*it);
  }
  return result;
}


// EventDiagnoser()
void EventDiagnoser(const System& rOrigGen, const map<string,EventSet>& rFailureTypeMap, Diagnoser& rDiagGen) {
  FD_DD("EventDiagnoser()");
  AttributeFailureTypeMap attrFT;
  attrFT.AddFailureTypeMap(rFailureTypeMap);
  EventDiagnoser(rOrigGen, attrFT, rDiagGen);
}

// EventDiagnoser()
void EventDiagnoser(const System& rOrigGen, const AttributeFailureTypeMap& rAttrFTMap, Diagnoser& rDiagGen) {
  EventSet failureEvents;
  EventSet gObservableEvents, gUnobservableEvents;
  StateSet newDiagStates;
  Idx currDState = 0;
  Idx nextDState = 0;
  string reportString;
  
  FD_DD("EventDiagnoser()");
  // check if FailureTypeMap is empty
  if (rAttrFTMap.Empty()) {
    FD_DD( endl << "WARNING - EventDiagnoser(): failure type map is empty!" << endl);
  }
  
  // Necessary assumption: No cycles of unobservable events
  reportString.clear();
  if (CycleOfUnobsEvents(rOrigGen,reportString)) {
    FD_DD( "EventDiagnoser(): Generator contains cycle of unobservable events! Aborting..");
    FD_DD(reportString);
    return;
  }
    
  // clear diagnoser
  rDiagGen.Clear();
  rDiagGen.ClearAttributes();
  
  // copy attribute failure type map to diagnoser
  rDiagGen.GlobalAttribute(rAttrFTMap);
  
  // get observable events of original generator
  gObservableEvents = rOrigGen.ObservableEvents();
  FD_DD("Observable events: " << gObservableEvents.ToString());
  
  // get unobservable events of original generator
  gUnobservableEvents = rOrigGen.UnobservableEvents();
  FD_DD("Unobservable events: " << gUnobservableEvents.ToString());

  // copy all failure events into one single EventSet
  failureEvents = rDiagGen.GetAllFailureEvents();
  FD_DD("Failure events: " << failureEvents.ToString());
  
  // create initial state of diagnoser and its attribute with label "normal"
  #ifdef FAUDES_CHECKED
  if(rOrigGen.InitStatesSize() != 1) {    
    std::stringstream errstr;
    errstr << "original generator has no unique initial state" << std::endl;
    throw Exception("EventDiagnoser()", errstr.str(), 301);
  }
  #endif
  Idx gInitState = rOrigGen.InitState();
  currDState = rDiagGen.InsInitState();
  newDiagStates.Insert(currDState);
  rDiagGen.InsStateLabelMapping(currDState,gInitState,DiagLabelSet::IndexOfLabelN());
  
  Idx gStateEstimate = 0;
  Idx newState = 0;
  
  map<Idx,multimap<Idx,DiagLabelSet> > reachMap; // maps executable events to all reachable states and occuring relative failure types
  map<Idx,multimap<Idx,DiagLabelSet> > reachMapWholeState;  // map for whole diagnoser state, contains propagated absolute failure type labels
  map<Idx,multimap<Idx,DiagLabelSet> >::iterator it;
  
  multimap<Idx,DiagLabelSet> tmpPropagatedLabels;
  multimap<Idx,DiagLabelSet> bufferPropLabels;
  multimap<Idx,DiagLabelSet>::iterator mmit, mmit2;
  
  //pair<Idx,DiagLabelSet> stateLabelPair;
  pair<Idx,DiagLabelSet> reachMapPair;  

  TransSet transitions;
  TransSet::Iterator transIt;
  DiagLabelSet oldLabel, newLabel, occFailureTypes;
  DiagLabelSet::Iterator labelIt;
  StateSet diagStates;
  StateSet::Iterator stateIt;
  EventSet activeEvents;
  AttributeDiagnoserState newAttr;
  bool stateLabelExists = false;
  AttributeDiagnoserState currDStateAttr;
  TaIndexSet<DiagLabelSet> currDStateMap; 
  TaIndexSet<DiagLabelSet>::Iterator currDStateMapIt;
  
  // parse through new diagnoser states
  while (!newDiagStates.Empty()) {
    // set current diagnoser state
    currDState = *newDiagStates.Begin();
    
    reachMapWholeState.clear();
    
    currDStateAttr = rDiagGen.StateAttribute(currDState);
    currDStateMap = currDStateAttr.DiagnoserStateMap();
    
    // parse through state estimates of current diagnoser state
    for(currDStateMapIt = currDStateMap.Begin(); currDStateMapIt != currDStateMap.End(); ++ currDStateMapIt){
      gStateEstimate = *currDStateMapIt;
      //  generate reachMap for current state estimate
      ComputeReachability(rOrigGen, gUnobservableEvents, failureEvents, gStateEstimate, rAttrFTMap, reachMap);
      // parse through reachMap (eventwise), propagate label and copy it to reachMapWholeState
      for (it = reachMap.begin(); it != reachMap.end(); it++) {
        // get label set of current state estimate
        oldLabel = currDStateMap.Attribute(*currDStateMapIt);
        newState = 0;
        tmpPropagatedLabels.clear();
        // parse through state failure type mappings of state failure type map (for current event in reachMap)
        for (mmit = it->second.begin(); mmit != it->second.end(); mmit++) {
          newState = mmit->first;
          occFailureTypes = mmit->second;
          LabelPropagation(oldLabel, occFailureTypes, newLabel);
          // check if this combination of state and label does already exist 
          stateLabelExists = false;
          for (mmit2 = tmpPropagatedLabels.lower_bound(newState); mmit2 != tmpPropagatedLabels.upper_bound(newState); mmit2++) {
            if (mmit2->second == newLabel) {
              stateLabelExists = true;
            }
          }
          // insert new state-label-pair if it does not exist yet
          if (!stateLabelExists) {
            tmpPropagatedLabels.insert(pair<Idx,DiagLabelSet>(newState,newLabel));
          }
        }
        
        // if event is already mapped: add new Labels to multimap and insert it afterwards
        if (reachMapWholeState.find(it->first) != reachMapWholeState.end()) {
          bufferPropLabels = reachMapWholeState[it->first];
          
          // parse throught tmpPropagatedLabels and check for every combination of state and label
          // if it does already exist in bufferPropLabels
          for (mmit = tmpPropagatedLabels.begin(); mmit != tmpPropagatedLabels.end(); mmit++) {
            stateLabelExists = false;
            for (mmit2 = bufferPropLabels.lower_bound(mmit->first); mmit2 != bufferPropLabels.upper_bound(mmit->first); mmit2++) {
              if (mmit2->second == mmit->second) {
                stateLabelExists = true;
              }
            }
            // insert new state-label-pair if it does not exist yet
            if (!stateLabelExists) {
              bufferPropLabels.insert(pair<Idx,DiagLabelSet>(mmit->first,mmit->second));
            }
          }
          reachMapWholeState[it->first] = bufferPropLabels;
        } 
        // if not just insert the new labels
        else {
          reachMapWholeState[it->first] = tmpPropagatedLabels;
        }
      }
    }
    activeEvents = rDiagGen.ActiveEventSet(currDState);
    transitions = rDiagGen.ActiveTransSet(currDState);
    
    for (it = reachMapWholeState.begin(); it != reachMapWholeState.end(); it++) {
      LabelCorrection(it->second,newAttr);
      // if newAttr equals any existing state attribute than create a transition to this very state
      diagStates = rDiagGen.States();        
      stateIt = diagStates.Begin();
      while (stateIt != diagStates.End()) {
        if (newAttr == rDiagGen.StateAttribute(*stateIt)) {
          // realising as back- or self-loop to existing state *stateIt
          rDiagGen.InsEvent(it->first);
          rDiagGen.SetTransition(currDState,it->first,*stateIt);
          break;
        }
        stateIt++;
      }
      // if newAttr is new to diagnoser
      if (stateIt == diagStates.End()) {
        // if current event is executable from current diagnoser state
        if (activeEvents.Exists(it->first)) {
          // this event is executable from current diagnoser state
          // find successor state nextDState
          transIt = transitions.Begin();
          while (transIt != transitions.End()) {
            if (transIt->Ev == it->first) {
              nextDState = transIt->X2;
              break;
            }
            transIt++;
          }
        }
        // if event is not executable from current diagnoser state
        else {
          // this event is not yet executable from current diagnoser state: create new diagnoser state
          // creat new diagnoser state and add it to new states
          nextDState = rDiagGen.InsState();
          newDiagStates.Insert(nextDState);
          rDiagGen.InsEvent(it->first);
          rDiagGen.SetTransition(currDState,it->first,nextDState);
        }
        rDiagGen.StateAttribute(nextDState, newAttr);
      }
    }  
    // delete current diagnoser state from new states
    newDiagStates.Erase(currDState);  
  }  
}

// LabelPropagation()
void LabelPropagation(const DiagLabelSet& lastLabel, const DiagLabelSet& failureTypes, DiagLabelSet& newLabel) {
  FD_DD("LabelPropagation()");
  newLabel.Clear();

  // if no failure occurred
  if (failureTypes.Size() == 1 && failureTypes.Exists(DiagLabelSet::IndexOfLabelRelN())) {
    // if label = {"N"}
    if (lastLabel.Size() == 1 && lastLabel.Exists(DiagLabelSet::IndexOfLabelN())) {
      newLabel.Insert(DiagLabelSet::IndexOfLabelN());
      return;
    }
    // if label = {"A"}
    if (lastLabel.Size() == 1 && lastLabel.Exists(DiagLabelSet::IndexOfLabelA())) {  
      newLabel.Insert(DiagLabelSet::IndexOfLabelA());
      return;
    }
  }
  // otherwise:
  newLabel.InsertSet(lastLabel);
  newLabel.InsertSet(failureTypes);
  newLabel.Erase(DiagLabelSet::IndexOfLabelN());
  newLabel.Erase(DiagLabelSet::IndexOfLabelA());
  newLabel.Erase(DiagLabelSet::IndexOfLabelRelN());
  return;
}

// LabelCorrection()
void LabelCorrection(const multimap<Idx,DiagLabelSet>& mm, AttributeDiagnoserState& attr) {
  multimap<Idx,DiagLabelSet>::const_iterator mmit;
  multimap<Idx,DiagLabelSet>::const_iterator mmit_ub;
  DiagLabelSet label;
  Idx state;
  
  FD_DD("LabelCorrection()");
  attr.Clear();
  mmit = mm.begin();
  // parse through propagated labels
  while (mmit != mm.end()) {
    // if there is only one label for a particular state: no correction is needed and the label is copied to diagnoser state attribute
    if (mm.count(mmit->first) == 1) {
      attr.AddStateLabelMap(mmit->first,mmit->second);
      mmit++;
    }
    // if there are several labels: correct label before adding it to the diagnoser state attribute
    else {
      mmit_ub = mm.upper_bound(mmit->first);
      state = mmit->first;
      
      label = mmit->second;
      mmit++;
      for ( ; mmit != mmit_ub; mmit++) {
        label = label * mmit->second;        
      }
      label.Insert(DiagLabelSet::IndexOfLabelA());
      attr.AddStateLabelMap(state,label);
    }
  }
}


} // namespace faudes
