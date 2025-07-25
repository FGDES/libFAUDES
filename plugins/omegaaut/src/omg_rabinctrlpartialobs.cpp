/** @file omg_rabinctrlpartialobs.cpp Rabin control synthesis under partial observation */

#include "omg_rabinctrlpartialobs.h"

namespace faudes {

// ============================================================================
// Helper Functions
// ============================================================================

// ExtractEventAttributes(rSys, rControllableEvents, rObservableEvents)
void ExtractEventAttributes(const System& rSys,
                           EventSet& rControllableEvents,
                           EventSet& rObservableEvents) {
    FD_DF("ExtractEventAttributes()");
    
    rControllableEvents.Clear();
    rObservableEvents.Clear();
    
    // Extract controllable events
    rControllableEvents = rSys.ControllableEvents();
    
    // Extract observable events
    rObservableEvents = rSys.ObservableEvents();
    
    FD_DF("ExtractEventAttributes: controllable events: " << rControllableEvents.ToString());
    FD_DF("ExtractEventAttributes: observable events: " << rObservableEvents.ToString());
}

// RabinCtrlPartialObsConsistencyCheck
void RabinCtrlPartialObsConsistencyCheck(const RabinAutomaton& rPlant,
                                        const RabinAutomaton& rSpec,
                                        const EventSet& rControllableEvents,
                                        const EventSet& rObservableEvents) {
    FD_DF("RabinCtrlPartialObsConsistencyCheck()");
    
    // Check that alphabets match
    if (rPlant.Alphabet() != rSpec.Alphabet()) {
        EventSet only_in_plant = rPlant.Alphabet() - rSpec.Alphabet();
        EventSet only_in_spec = rSpec.Alphabet() - rPlant.Alphabet();
        only_in_plant.Name("OnlyInPlant");
        only_in_spec.Name("OnlyInSpec");
        std::stringstream errstr;
        errstr << "Alphabets of plant and specification do not match.";
        if(!only_in_plant.Empty())
            errstr << " " << only_in_plant.ToString() << ".";
        if(!only_in_spec.Empty())
            errstr << " " << only_in_spec.ToString() << ".";
        throw Exception("RabinCtrlPartialObs", errstr.str(), 100);
    }
    
    // Check that controllable events are subset of alphabet
    if (!(rControllableEvents <= rPlant.Alphabet())) {
        EventSet not_in_alphabet = rControllableEvents - rPlant.Alphabet();
        std::stringstream errstr;
        errstr << "Controllable events not subset of alphabet: " 
               << not_in_alphabet.ToString();
        throw Exception("RabinCtrlPartialObs", errstr.str(), 305);
    }
    
    // Check that observable events are subset of alphabet
    if (!(rObservableEvents <= rPlant.Alphabet())) {
        EventSet not_in_alphabet = rObservableEvents - rPlant.Alphabet();
        std::stringstream errstr;
        errstr << "Observable events not subset of alphabet: " 
               << not_in_alphabet.ToString();
        throw Exception("RabinCtrlPartialObs", errstr.str(), 306);
    }
    
    // Check that plant and spec are deterministic
    bool plant_det = rPlant.IsDeterministic();
    bool spec_det = rSpec.IsDeterministic();
    
    if (!plant_det && !spec_det) {
        throw Exception("RabinCtrlPartialObs", 
                       "Both plant and specification must be deterministic", 204);
    } else if (!plant_det) {
        throw Exception("RabinCtrlPartialObs", 
                       "Plant must be deterministic", 201);
    } else if (!spec_det) {
        throw Exception("RabinCtrlPartialObs", 
                       "Specification must be deterministic", 203);
    }
    
    FD_DF("RabinCtrlPartialObsConsistencyCheck: passed all checks");
}

// ============================================================================
// Main API Functions
// ============================================================================

// RabinCtrlPartialObs (System interface - automatic event attribute extraction)
void RabinCtrlPartialObs(const System& rPlant, 
                        const RabinAutomaton& rSpec,           
                        RabinAutomaton& rSupervisor) {
    FD_DF("RabinCtrlPartialObs: System interface");
    
    // Extract controllable and observable events from System
    EventSet controllableEvents, observableEvents;
    ExtractEventAttributes(rPlant, controllableEvents, observableEvents);
    
    // Events extracted successfully
    
    // Call main implementation with the System cast to RabinAutomaton
    RabinCtrlPartialObs(static_cast<const RabinAutomaton&>(rPlant), 
                       controllableEvents, observableEvents, rSpec, rSupervisor);
}

// RabinCtrlPartialObs (explicit event sets)
void RabinCtrlPartialObs(const RabinAutomaton& rPlant, 
                        const EventSet& rControllableEvents,
                        const EventSet& rObservableEvents,
                        const RabinAutomaton& rSpec, 
                        RabinAutomaton& rSupervisor) {
    FD_DF("RabinCtrlPartialObs: explicit event sets");
    
    // CONSISTENCY CHECKS
    if (rControllableEvents.Empty()) {
        throw Exception("RabinCtrlPartialObs", "Empty controllable event set", 300);
    }
    
    if (rObservableEvents.Empty()) {
        throw Exception("RabinCtrlPartialObs", "Empty observable event set", 301);
    }
    
    if (rSpec.Empty()) {
        throw Exception("RabinCtrlPartialObs", "Empty specification language", 302);
    }
    
    RabinCtrlPartialObsConsistencyCheck(rPlant, rSpec, rControllableEvents, rObservableEvents);
    
    // PREPARE RESULT
    RabinAutomaton* pSupervisor = &rSupervisor;
    if (&rSupervisor == &rPlant || &rSupervisor == &rSpec) {
        pSupervisor = new RabinAutomaton();
    }
    pSupervisor->Clear();
    pSupervisor->Name("RabinCtrlPartialObs(" + rPlant.Name() + "," + rSpec.Name() + ")");
    
    try {
        FD_DF("RabinCtrlPartialObs: Starting synthesis algorithm");
        
        // STEP 1: Compute synchronous product of plant and specification
        FD_DF("RabinCtrlPartialObs: Step 1 - Computing product");
        RabinAutomaton Product;
        RabinBuechiProduct(rSpec, rPlant, Product);
        
        if (Product.Empty()) {
            throw Exception("RabinCtrlPartialObs", 
                           "Product of plant and specification is empty", 302);
        }
        
        FD_DF("RabinCtrlPartialObs: Product computed, states: " << Product.Size());
        
        // STEP 2: Apply epsilon observation directly on product (skip control pattern expansion)
        FD_DF("RabinCtrlPartialObs: Step 2 - Applying epsilon observation directly on product");
        RabinAutomaton epsObserved;
        
        Product.SetControllable(rControllableEvents);

        EventSet allProductEvents = Product.Alphabet();
        Product.ClrObservable(allProductEvents);
        Product.SetObservable(rObservableEvents);
        // Apply epsilon observation directly on the product
        EpsObservation(Product, epsObserved);
        
        if (epsObserved.Empty()) {
            throw Exception("RabinCtrlPartialObs", 
                           "Epsilon observation resulted in empty automaton", 303);
        }
        
        FD_DF("RabinCtrlPartialObs: Epsilon observation applied, states: " << epsObserved.Size());
        
        // STEP 3: Pseudo-determinize the result
        FD_DF("RabinCtrlPartialObs: Step 3 - Pseudo-determinization");
        PseudoDet(epsObserved, *pSupervisor);
        
        if (pSupervisor->Empty()) {
            throw Exception("RabinCtrlPartialObs", 
                           "Pseudo-determinization resulted in empty automaton", 303);
        }
        
        FD_DF("RabinCtrlPartialObs: Pseudo-determinization completed, states: " << pSupervisor->Size());
        
        // Keep the alphabet as computed by PseudoDet (includes epsilon events)
        // Do not reset to original plant alphabet since we need eps events
        
        FD_DF("RabinCtrlPartialObs: Synthesis completed successfully");
        
    } catch (const Exception& ex) {
        FD_DF("RabinCtrlPartialObs: Exception caught: " << ex.What());
        
        // Clean up and re-throw
        if (pSupervisor != &rSupervisor) {
            delete pSupervisor;
        }
        throw;
    }
    
    // Copy result if needed
    if (pSupervisor != &rSupervisor) {
        rSupervisor = *pSupervisor;
        delete pSupervisor;
    }
    
    FD_DF("RabinCtrlPartialObs: completed");
}

// ControlAut - Apply controller to filter transitions and create Buchi automaton
void ControlAut(const RabinAutomaton& rsDRA,
               const TaIndexSet<EventSet>& rController,
               Generator& rRes) {
    FD_DF("ControlAut()");
    
    // Clear result automaton
    Generator* pRes = &rRes;
    if (&rRes == &rsDRA) {
        pRes = new Generator();
    }
    pRes->Clear();
    pRes->Name("ControlAut(" + rsDRA.Name() + ")");
    
    // Copy alphabet from source automaton
    pRes->InjectAlphabet(rsDRA.Alphabet());
    
    // Copy only states that are in the controller
    StateSet::Iterator sit;
    for (sit = rsDRA.StatesBegin(); sit != rsDRA.StatesEnd(); ++sit) {
        if (rController.Exists(*sit)) {
            pRes->InsState(*sit);
        }
    }
    
    // Set initial states (only if they exist in the controller)
    StateSet::Iterator iit;
    for (iit = rsDRA.InitStatesBegin(); iit != rsDRA.InitStatesEnd(); ++iit) {
        if (rController.Exists(*iit)) {
            pRes->SetInitState(*iit);
        }
    }
    
    // Mark all states (create Buchi automaton)
    for (sit = pRes->StatesBegin(); sit != pRes->StatesEnd(); ++sit) {
        pRes->SetMarkedState(*sit);
    }
    
    // Add self-loops for unobservable events in each state (step 2)
    EventSet unobservableEvents = rsDRA.Alphabet() - rsDRA.ObservableEvents();
    for (sit = pRes->StatesBegin(); sit != pRes->StatesEnd(); ++sit) {
        EventSet::Iterator eit;
        for (eit = unobservableEvents.Begin(); eit != unobservableEvents.End(); ++eit) {
            pRes->SetTransition(*sit, *eit, *sit);
            FD_DF("ControlAut: Adding unobservable self-loop " << *sit << " --" << 
                  rsDRA.EventName(*eit) << "--> " << *sit);
        }
    }
    
    // Filter transitions based on controller
    TransSet::Iterator tit;
    for (tit = rsDRA.TransRelBegin(); tit != rsDRA.TransRelEnd(); ++tit) {
        Idx state = tit->X1;
        Idx event = tit->Ev;
        Idx nextState = tit->X2;
        
        // Check if this state has controller restrictions
        if (rController.Exists(state)) {
            const EventSet& allowedEvents = rController.Attribute(state);
            // Only add transition if event is allowed by controller
            if (allowedEvents.Exists(event)) {
                pRes->SetTransition(state, event, nextState);
                FD_DF("ControlAut: Adding transition " << state << " --" << 
                      rsDRA.EventName(event) << "--> " << nextState);
            } else {
                FD_DF("ControlAut: Filtering out transition " << state << " --" << 
                      rsDRA.EventName(event) << "--> " << nextState << " (not allowed by controller)");
            }
        } else {
            // Skip transitions from states not in controller (they are deleted)
            FD_DF("ControlAut: Skipping transition " << state << " --" << 
                  rsDRA.EventName(event) << "--> " << nextState << " (state not in controller)");
        }
    }
    
    // Copy result if needed
    if (pRes != &rRes) {
        rRes = *pRes;
        delete pRes;
    }
    
    
    FD_DF("ControlAut: Generated Buchi automaton with " << rRes.Size() << 
          " states and " << rRes.TransRelSize() << " transitions");
    FD_DF("ControlAut: All " << rRes.MarkedStatesSize() << " states are marked");
}

} // namespace faudes