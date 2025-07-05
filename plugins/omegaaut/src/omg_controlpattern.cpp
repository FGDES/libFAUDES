#include "libfaudes.h"
#include "omg_controlpattern.h"

namespace faudes {

/*
********************************
Implementation AugmentedEvent
********************************
*/

// faudes Type std
FAUDES_TYPE_IMPLEMENTATION(Void, AugmentedEvent, AttributeVoid)

// Default constructor
AugmentedEvent::AugmentedEvent(void) : AttributeVoid() {
    mEvent = 0;
    mControlPattern.Clear();
}

// Constructor with event and pattern
AugmentedEvent::AugmentedEvent(Idx event, const EventSet& pattern) : AttributeVoid() {
    mEvent = event;
    mControlPattern = pattern;
}

// Copy constructor
AugmentedEvent::AugmentedEvent(const AugmentedEvent& other) : AttributeVoid() {
    DoAssign(other);
}

// Assignment method
void AugmentedEvent::DoAssign(const AugmentedEvent& rSrc) {
    mEvent = rSrc.mEvent;
    mControlPattern = rSrc.mControlPattern;
}

// Test equality
bool AugmentedEvent::DoEqual(const AugmentedEvent& rOther) const {
    return (mEvent == rOther.mEvent) && (mControlPattern == rOther.mControlPattern);
}

// Write to TokenWriter
void AugmentedEvent::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
    std::string label = rLabel.empty() ? "AugmentedEvent" : rLabel;
    
    Token btag;
    btag.SetBegin(label);
    btag.InsAttributeInteger("event", mEvent);
    
    rTw.Write(btag);
    mControlPattern.Write(rTw, "ControlPattern", pContext);
    rTw.WriteEnd(label);
}

// Read from TokenReader
void AugmentedEvent::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
    std::string label = rLabel.empty() ? "AugmentedEvent" : rLabel;
    
    Token btag;
    rTr.ReadBegin(label, btag);
    
    if (btag.ExistsAttributeInteger("event")) {
        mEvent = btag.AttributeIntegerValue("event");
    }
    
    mControlPattern.Read(rTr, "ControlPattern", pContext);
    rTr.ReadEnd(label);
}

/*
********************************
Implementation AugmentedAlphabet
********************************
*/

// faudes Type std
FAUDES_TYPE_IMPLEMENTATION(Void, AugmentedAlphabet, TBaseSet<AugmentedEvent>)

// Default constructor
AugmentedAlphabet::AugmentedAlphabet(void) : TBaseSet<AugmentedEvent>() {
    Name("AugmentedAlphabet");
}

// Copy constructor
AugmentedAlphabet::AugmentedAlphabet(const AugmentedAlphabet& other) : TBaseSet<AugmentedEvent>() {
    Name("AugmentedAlphabet");
    DoAssign(other);
}

// Assignment method
void AugmentedAlphabet::DoAssign(const AugmentedAlphabet& rSrc) {
    TBaseSet<AugmentedEvent>::DoAssign(rSrc);
}

// Insert with event and pattern (wrapper)
bool AugmentedAlphabet::InsertAugmentedEvent(Idx event, const EventSet& pattern) {
    AugmentedEvent augEvent(event, pattern);
    return TBaseSet<AugmentedEvent>::Insert(augEvent);
}

// Check if event-pattern combination exists
bool AugmentedAlphabet::ExistsAugmentedEvent(Idx event, const EventSet& pattern) const {
    AugmentedEvent augEvent(event, pattern);
    return TBaseSet<AugmentedEvent>::Exists(augEvent);
}



// Find by pattern
AugmentedAlphabet AugmentedAlphabet::FindByPattern(const EventSet& pattern) const {
    AugmentedAlphabet rRes;
    
    for (Iterator it = Begin(); it != End(); ++it) {
        if (it->ControlPattern() == pattern) {
            rRes.Insert(*it);
        }
    }
    
    return rRes;
}

// Generate subsets recursively

// Get all events
EventSet AugmentedAlphabet::Events(void) const {
    EventSet rRes;
    
    for (Iterator it = Begin(); it != End(); ++it) {
        rRes.Insert(it->Event());
    }
    
    return rRes;
}

// Get all control patterns
std::vector<EventSet> AugmentedAlphabet::ControlPatterns(void) const {
    std::vector<EventSet> rRes;
    
    for (Iterator it = Begin(); it != End(); ++it) {
        const EventSet& pattern = it->ControlPattern();
        
        // Check if pattern already exists
        bool found = false;
        for (const auto& existing : rRes) {
            if (existing == pattern) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            rRes.push_back(pattern);
        }
    }
    
    return rRes;
}

/*
********************************
Implementation ControlPatternGenerator
********************************
*/

// Generate all valid control patterns
std::vector<EventSet> ControlPatternGenerator::GenerateControlPatterns(
    const EventSet& alphabet, 
    const EventSet& controllableEvents) {
    
    std::vector<EventSet> rRes;
    
    // Get uncontrollable events (must always be included)
    EventSet uncontrollableEvents = alphabet - controllableEvents;
    
    // Convert controllable events to vector for easier manipulation
    std::vector<Idx> controllableVec;
    for (EventSet::Iterator it = controllableEvents.Begin(); 
         it != controllableEvents.End(); ++it) {
        controllableVec.push_back(*it);
    }
    
    // Generate all subsets of controllable events
    EventSet current = uncontrollableEvents; // Start with uncontrollable events
    GenerateSubsets(controllableVec, uncontrollableEvents, rRes, current, 0);
    
    // Assign names to the control patterns
    for (size_t i = 0; i < rRes.size(); ++i) {
        std::string patternName = "G" + std::to_string(i + 1);
        rRes[i].Name(patternName);
    }
    
    return rRes;
}

// Generate augmented alphabet
AugmentedAlphabet ControlPatternGenerator::GenerateAugmentedAlphabet(
    const EventSet& alphabet,
    const EventSet& controllableEvents) {
    
    AugmentedAlphabet rRes;
    
    // Generate all control patterns
    std::vector<EventSet> patterns = GenerateControlPatterns(alphabet, controllableEvents);
    
    // For each event and each pattern, create augmented event if event is in pattern
    for (EventSet::Iterator eventIt = alphabet.Begin(); 
         eventIt != alphabet.End(); ++eventIt) {
        
        for (const auto& pattern : patterns) {
            if (pattern.Exists(*eventIt)) {
                rRes.InsertAugmentedEvent(*eventIt, pattern);
            }
        }
    }
    
    return rRes;
}



// Expand Rabin automaton to control patterns
RabinAutomaton ControlPatternGenerator::ExpandToControlPatterns(
    const RabinAutomaton& rGen,
    const EventSet& controllableEvents) {
    
    RabinAutomaton rRes;
    
    // Get original alphabet and controllable events
    EventSet originalAlphabet = rGen.Alphabet();
    EventSet actualControllableEvents = controllableEvents * originalAlphabet; // intersection
    
    // Generate control patterns
    std::vector<EventSet> controlPatterns = GenerateControlPatterns(originalAlphabet, actualControllableEvents);
    
    FD_DF("Generated " << controlPatterns.size() << " control patterns:");
    for (size_t i = 0; i < controlPatterns.size(); ++i) {
        FD_DF(controlPatterns[i].Name() << " = " << controlPatterns[i].ToString());
    }
    
    // Create mapping from original events to expanded events
    std::map<Idx, std::vector<Idx>> eventMapping;
    
    // Build expanded alphabet using original event names
    EventSet::Iterator eventIt;
    for (eventIt = originalAlphabet.Begin(); eventIt != originalAlphabet.End(); ++eventIt) {
        Idx originalEvent = *eventIt;
        
        // Get original event name from the source automaton
        std::string originalEventName;
        if (rGen.ExistsEvent(originalEvent)) {
            
            // Try to get symbolic name if available
                originalEventName = rGen.EventName(originalEvent);
        }
        
        // For each control pattern that contains this event
        for (size_t i = 0; i < controlPatterns.size(); ++i) {
            const EventSet& pattern = controlPatterns[i];
            
            if (pattern.Exists(originalEvent)) {
                // Create expanded event name: originalEventName_PatternName
                std::string expandedEventName = originalEventName + "_" + pattern.Name();
                
                // Insert into rRes alphabet
                Idx expandedEvent = rRes.InsEvent(expandedEventName);
                
                // Copy controllability attributes
                if (actualControllableEvents.Exists(originalEvent)) {
                    rRes.SetControllable(expandedEvent);
                } else {
                    rRes.ClrControllable(expandedEvent);
                }
                
                // Copy observability from original event
                if (rGen.Observable(originalEvent)) {
                    rRes.SetObservable(expandedEvent);
                } else {
                    rRes.ClrObservable(expandedEvent);
                }
                
                // Add to mapping
                eventMapping[originalEvent].push_back(expandedEvent);
            }
        }
    }
    
    // Copy states exactly as they are in the original automaton
    StateSet::Iterator stateIt;
    for (stateIt = rGen.StatesBegin(); stateIt != rGen.StatesEnd(); ++stateIt) {
        Idx state = *stateIt;
        
        // Get original state name from the source automaton
        std::string stateName;
        try {
            stateName = rGen.StateName(state);
        } catch (...) {
            // Fallback to index-based name
            stateName = "state_" + std::to_string(state);
        }
        
        // Insert state with same index and name
        rRes.InsState(state);
        rRes.StateName(state, stateName);
        
        // Copy initial and marked state properties
        if (rGen.ExistsInitState(state)) {
            rRes.SetInitState(state);
        }
        if (rGen.ExistsMarkedState(state)) {
            rRes.SetMarkedState(state);
        }
    }
    
    // Expand transitions
    TransSet::Iterator transIt;
    for (transIt = rGen.TransRelBegin(); transIt != rGen.TransRelEnd(); ++transIt) {
        Idx srcState = transIt->X1;
        Idx originalEvent = transIt->Ev;
        Idx dstState = transIt->X2;
        
        // For each expanded event corresponding to the original event
        if (eventMapping.find(originalEvent) != eventMapping.end()) {
            const std::vector<Idx>& expandedEvents = eventMapping[originalEvent];
            
            for (Idx expandedEvent : expandedEvents) {
                rRes.SetTransition(srcState, expandedEvent, dstState);
            }
        }
    }
    
    // Copy and adapt Rabin acceptance condition
    RabinAcceptance originalAcceptance = rGen.RabinAcceptance();
    RabinAcceptance newAcceptance;
    
    RabinAcceptance::Iterator accIt;
    for (accIt = originalAcceptance.Begin(); accIt != originalAcceptance.End(); ++accIt) {
        RabinPair newPair;
        newPair.Name(accIt->Name()); // Keep original name
        
        // Copy R and I sets (state sets remain the same)
        newPair.RSet() = accIt->RSet();
        newPair.ISet() = accIt->ISet();
        
        newAcceptance.Insert(newPair);
    }
    
    rRes.RabinAcceptance() = newAcceptance;
    
    // Set rRes name
    rRes.Name(rGen.Name() + "_Expanded");
    
    return rRes;
}
void ControlPatternGenerator::GenerateSubsets(
    const std::vector<Idx>& controllableEvents,
    const EventSet& uncontrollableEvents,
    std::vector<EventSet>& rRes,
    EventSet& current,
    size_t index) {
    
    if (index == controllableEvents.size()) {
        // Base case: add current subset to rRes
        rRes.push_back(current);
        return;
    }
    
    // Recursive case: try without current controllable event
    GenerateSubsets(controllableEvents, uncontrollableEvents, rRes, current, index + 1);
    
    // Try with current controllable event
    current.Insert(controllableEvents[index]);
    GenerateSubsets(controllableEvents, uncontrollableEvents, rRes, current, index + 1);
    current.Erase(controllableEvents[index]); // backtrack
}

void RabinProduct(const RabinAutomaton& rGen1, const RabinAutomaton& rGen2, RabinAutomaton& rRes) {
    rRes.Clear();
    
    // 1. Construct alphabet intersection
    EventSet intersectAlphabet = rGen1.Alphabet() * rGen2.Alphabet();
    
    // 2. Create alphabet with attributes - inherit ALL event properties
    EventSet::Iterator evit;
    for(evit = intersectAlphabet.Begin(); evit != intersectAlphabet.End(); ++evit) {
        Idx event = *evit;
        
        // Get event name first
        std::string eventName;
        if(rGen1.ExistsEvent(event)) {
            eventName = rGen1.EventName(event);
        } else if(rGen2.ExistsEvent(event)) {
            eventName = rGen2.EventName(event);
        }
        
        // Insert event with name
        rRes.InsEvent(event);
        if(!eventName.empty()) {
            rRes.EventName(event, eventName);
        }
        
        // Inherit controllability (from the automaton that has this event)
        bool isControllable = false;
        if(rGen1.ExistsEvent(event) && rGen1.Controllable(event)) {
            isControllable = true;
        }
        if(rGen2.ExistsEvent(event) && rGen2.Controllable(event)) {
            isControllable = true;
        }
        
        if(isControllable) {
            rRes.SetControllable(event);
        } else {
            rRes.ClrControllable(event);
        }
        
        // Inherit observability (from the automaton that has this event)
        bool isObservable = true;  // Default to observable
        if(rGen1.ExistsEvent(event)) {
            isObservable = rGen1.Observable(event);
        } else if(rGen2.ExistsEvent(event)) {
            isObservable = rGen2.Observable(event);
        }
        
        if(isObservable) {
            rRes.SetObservable(event);
        } else {
            rRes.ClrObservable(event);
        }
        
        // Inherit forcibility (from the automaton that has this event)
        bool isForcible = false;  // Default to not forcible
        if(rGen1.ExistsEvent(event)) {
            isForcible = rGen1.Forcible(event);
        } else if(rGen2.ExistsEvent(event)) {
            isForcible = rGen2.Forcible(event);
        }
        
        if(isForcible) {
            rRes.SetForcible(event);
        } else {
            rRes.ClrForcible(event);
        }
    }
    
    // 3-5. Construct states, transitions etc. (unchanged)
    std::map<std::pair<Idx, Idx>, Idx> stateMap;
    StateSet::Iterator sit1, sit2;
    for(sit1 = rGen1.StatesBegin(); sit1 != rGen1.StatesEnd(); ++sit1) {
        for(sit2 = rGen2.StatesBegin(); sit2 != rGen2.StatesEnd(); ++sit2) {
            std::string stateName = rGen1.StateName(*sit1) + "|" + rGen2.StateName(*sit2);
            Idx newState = rRes.InsState(stateName);
            stateMap[std::make_pair(*sit1, *sit2)] = newState;
        }
    }
    
    StateSet::Iterator init1, init2;
    for(init1 = rGen1.InitStatesBegin(); init1 != rGen1.InitStatesEnd(); ++init1) {
        for(init2 = rGen2.InitStatesBegin(); init2 != rGen2.InitStatesEnd(); ++init2) {
            rRes.SetInitState(stateMap[std::make_pair(*init1, *init2)]);
        }
    }
    
    StateSet::Iterator marked1, marked2;
    for(marked1 = rGen1.MarkedStatesBegin(); marked1 != rGen1.MarkedStatesEnd(); ++marked1) {
        for(marked2 = rGen2.MarkedStatesBegin(); marked2 != rGen2.MarkedStatesEnd(); ++marked2) {
            rRes.SetMarkedState(stateMap[std::make_pair(*marked1, *marked2)]);
        }
    }
    
    TransSet::Iterator tit1, tit2;
    for(tit1 = rGen1.TransRelBegin(); tit1 != rGen1.TransRelEnd(); ++tit1) {
        for(tit2 = rGen2.TransRelBegin(); tit2 != rGen2.TransRelEnd(); ++tit2) {
            if(tit1->Ev == tit2->Ev && intersectAlphabet.Exists(tit1->Ev)) {
                Idx srcState = stateMap[std::make_pair(tit1->X1, tit2->X1)];
                Idx dstState = stateMap[std::make_pair(tit1->X2, tit2->X2)];
                rRes.SetTransition(srcState, tit1->Ev, dstState);
            }
        }
    }
    
    // 6. Construct Rabin acceptance condition - ensure at least one empty pair to start the loop
    RabinAcceptance productAcc;
    RabinAcceptance acc1 = rGen1.RabinAcceptance();
    RabinAcceptance acc2 = rGen2.RabinAcceptance();
    
    // If any acceptance condition is empty, add an empty RabinPair
    if(acc1.Size() == 0) {
        RabinPair emptyPair;
        emptyPair.Name("empty1");
        acc1.Insert(emptyPair);
    }
    if(acc2.Size() == 0) {
        RabinPair emptyPair;
        emptyPair.Name("empty2");
        acc2.Insert(emptyPair);
    }
    
    // Now use unified logic to handle all cases
    RabinAcceptance::CIterator rit1, rit2;
    for(rit1 = acc1.Begin(); rit1 != acc1.End(); ++rit1) {
        for(rit2 = acc2.Begin(); rit2 != acc2.End(); ++rit2) {
            
            RabinPair newPair;
            
            // R1 × X2 (if R1 is empty, no states will be added here)
            StateSet::Iterator r1_it, x2_it;
            for(r1_it = rit1->RSet().Begin(); r1_it != rit1->RSet().End(); ++r1_it) {
                for(x2_it = rGen2.StatesBegin(); x2_it != rGen2.StatesEnd(); ++x2_it) {
                    Idx combinedState = stateMap[std::make_pair(*r1_it, *x2_it)];
                    newPair.RSet().Insert(combinedState);
                }
            }
            
            // X1 × R2 (if R2 is empty, no states will be added here)
            StateSet::Iterator x1_it, r2_it;
            for(x1_it = rGen1.StatesBegin(); x1_it != rGen1.StatesEnd(); ++x1_it) {
                for(r2_it = rit2->RSet().Begin(); r2_it != rit2->RSet().End(); ++r2_it) {
                    Idx combinedState = stateMap[std::make_pair(*x1_it, *r2_it)];
                    newPair.RSet().Insert(combinedState);
                }
            }
            
            // I1 × X2 (if I1 is empty, no states will be added here)
            StateSet::Iterator i1_it;
            for(i1_it = rit1->ISet().Begin(); i1_it != rit1->ISet().End(); ++i1_it) {
                for(x2_it = rGen2.StatesBegin(); x2_it != rGen2.StatesEnd(); ++x2_it) {
                    Idx combinedState = stateMap[std::make_pair(*i1_it, *x2_it)];
                    newPair.ISet().Insert(combinedState);
                }
            }
            
            // X1 × I2 (if I2 is empty, no states will be added here)
            StateSet::Iterator i2_it;
            for(x1_it = rGen1.StatesBegin(); x1_it != rGen1.StatesEnd(); ++x1_it) {
                for(i2_it = rit2->ISet().Begin(); i2_it != rit2->ISet().End(); ++i2_it) {
                    Idx combinedState = stateMap[std::make_pair(*x1_it, *i2_it)];
                    newPair.ISet().Insert(combinedState);
                }
            }
            
            newPair.Name(rit1->Name() + "_x_" + rit2->Name());
            productAcc.Insert(newPair);
        }
    }
    
    rRes.RabinAcceptance() = productAcc;

    rRes.Alphabet().DWrite();
}

void EpsObservation(const RabinAutomaton& rGen, RabinAutomaton& rRes) {
    rRes = rGen;  // Copy original automaton
    
    // Get unobservable events
    EventSet unobservableEvents = rRes.UnobservableEvents();
    
    // If no unobservable events, return original automaton
    if(unobservableEvents.Size() == 0) {
        return;
    }
    
    // Step 1: Create new epsilon events for each control pattern
    std::set<std::string> patterns;  // Store unique patterns
    std::map<Idx, Idx> eventMapping;  // Map from old unobservable event to new epsilon event
    
    // First pass: collect all unique patterns from unobservable events
    EventSet::Iterator uit;
    for(uit = unobservableEvents.Begin(); uit != unobservableEvents.End(); ++uit) {
        std::string eventName = rRes.EventName(*uit);
        
        // Extract pattern from event name (e.g., "beta_1_G3" -> "G3")
        size_t pos = eventName.find_last_of('_');
        if(pos != std::string::npos && pos < eventName.length() - 1) {
            std::string pattern = eventName.substr(pos + 1);
            patterns.insert(pattern);
        }
    }
    
    // Step 2: Create epsilon events for each pattern
    std::map<std::string, Idx> patternToEpsilon;
    for(const std::string& pattern : patterns) {
        std::string epsEventName = "eps";
        Idx epsEvent = rRes.InsEvent(epsEventName);
        
        // Set epsilon event as uncontrollable and unobservable
        rRes.ClrControllable(epsEvent);
        rRes.ClrObservable(epsEvent);
        rRes.ClrForcible(epsEvent);
        
        patternToEpsilon[pattern] = epsEvent;
    }
    
    // Step 3: Create mapping from unobservable events to epsilon events
    for(uit = unobservableEvents.Begin(); uit != unobservableEvents.End(); ++uit) {
        std::string eventName = rRes.EventName(*uit);
        
        // Extract pattern from event name
        size_t pos = eventName.find_last_of('_');
        if(pos != std::string::npos && pos < eventName.length() - 1) {
            std::string pattern = eventName.substr(pos + 1);
            if(patternToEpsilon.find(pattern) != patternToEpsilon.end()) {
                eventMapping[*uit] = patternToEpsilon[pattern];
            }
        }
    }
    
    // Step 4: Replace transitions
    TransSet originalTransitions = rRes.TransRel();  // Get copy of original transition relation
    
    TransSet::Iterator tit;
    for(tit = originalTransitions.Begin(); tit != originalTransitions.End(); ++tit) {
        if(eventMapping.find(tit->Ev) != eventMapping.end()) {
            // This transition uses an unobservable event that should be replaced
            Idx newEvent = eventMapping[tit->Ev];
            
            // Remove original transition
            rRes.ClrTransition(tit->X1, tit->Ev, tit->X2);
            
            // Add new transition using epsilon
            rRes.SetTransition(tit->X1, newEvent, tit->X2);
            
        }
    }
    
    // Step 5: Remove old unobservable events from alphabet
    for(uit = unobservableEvents.Begin(); uit != unobservableEvents.End(); ++uit) {
        std::string removedEventName = rRes.EventName(*uit);
        rRes.DelEvent(*uit);
    }
}

} // namespace faudes