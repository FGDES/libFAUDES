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

// ============================================================================
// Epsilon Observation Functions
// ============================================================================

void EpsObservation(const RabinAutomaton& rGen, RabinAutomaton& rRes) {
    rRes = rGen;  // Copy original automaton
    
    // Get unobservable events
    EventSet unobservableEvents = rRes.UnobservableEvents();
    
    // If no unobservable events, return original automaton
    if(unobservableEvents.Size() == 0) {
        return;
    }
    
    // Create single epsilon event
    std::string epsEventName = "eps";
    Idx epsEvent = rRes.InsEvent(epsEventName);
    
    // Set epsilon event as uncontrollable and unobservable
    rRes.ClrControllable(epsEvent);
    rRes.ClrObservable(epsEvent);
    rRes.ClrForcible(epsEvent);
    
    // Step 1: Replace transitions
    TransSet originalTransitions = rRes.TransRel();  // Get copy of original transition relation
    
    TransSet::Iterator tit;
    for(tit = originalTransitions.Begin(); tit != originalTransitions.End(); ++tit) {
        if(unobservableEvents.Exists(tit->Ev)) {
            // This transition uses an unobservable event that should be replaced
            
            // Remove original transition
            rRes.ClrTransition(tit->X1, tit->Ev, tit->X2);
            
            // Add new transition using epsilon
            rRes.SetTransition(tit->X1, epsEvent, tit->X2);
        }
    }
    
    // Step 2: Remove old unobservable events from alphabet
    EventSet::Iterator uit;
    for(uit = unobservableEvents.Begin(); uit != unobservableEvents.End(); ++uit) {
        rRes.DelEvent(*uit);
    }
}

void EpsObservation(const System& rGen, System& rRes) {
    rRes = rGen;  // Copy original generator
    
    // Get unobservable events
    EventSet unobservableEvents = rRes.UnobservableEvents();
    
    // If no unobservable events, return original generator
    if(unobservableEvents.Size() == 0) {
        return;
    }
    
    // Create single epsilon event
    std::string epsEventName = "eps";
    Idx epsEvent = rRes.InsEvent(epsEventName);
    
    // Set epsilon event as uncontrollable and unobservable
    rRes.ClrControllable(epsEvent);
    rRes.ClrObservable(epsEvent);
    rRes.ClrForcible(epsEvent);
    
    // Step 1: Replace transitions
    TransSet originalTransitions = rRes.TransRel();  // Get copy of original transition relation
    
    TransSet::Iterator tit;
    for(tit = originalTransitions.Begin(); tit != originalTransitions.End(); ++tit) {
        if(unobservableEvents.Exists(tit->Ev)) {
            // This transition uses an unobservable event that should be replaced
            
            // Remove original transition
            rRes.ClrTransition(tit->X1, tit->Ev, tit->X2);
            
            // Add new transition using epsilon
            rRes.SetTransition(tit->X1, epsEvent, tit->X2);
        }
    }
    
    // Step 2: Remove old unobservable events from alphabet
    EventSet::Iterator uit;
    for(uit = unobservableEvents.Begin(); uit != unobservableEvents.End(); ++uit) {
        rRes.DelEvent(*uit);
    }
}

// ============================================================================
// Language Verification Functions
// ============================================================================

Generator CreateMutedAutomaton(const Generator& rOriginal, const StateSet& rStatesToMute) {
    Generator muted = rOriginal;
    
    // Remove transitions to and from muted states
    TransSet::Iterator tit = muted.TransRelBegin();
    while (tit != muted.TransRelEnd()) {
        if (rStatesToMute.Exists(tit->X1) || rStatesToMute.Exists(tit->X2)) {
            TransSet::Iterator tit_del = tit;
            ++tit;
            muted.ClrTransition(*tit_del);
        } else {
            ++tit;
        }
    }
    
    // Remove muted states from state set
    StateSet::Iterator sit;
    for (sit = rStatesToMute.Begin(); sit != rStatesToMute.End(); ++sit) {
        if (muted.ExistsState(*sit)) {
            muted.DelState(*sit);
        }
    }
    
    return muted;
}

bool RabinLanguageInclusion(const System& rGenL, const RabinAutomaton& rRabK) {
    FD_DF("RabinLanguageInclusion(): Starting language inclusion verification");
    
    // Step 1: Create copies of genL and rabK
    Generator genL_copy = rGenL;  // Buechi automaton to accept L
    RabinAutomaton rabK_copy = rRabK;  // Rabin automaton to accept K
    
    // Step 2: Apply BuechiTrim and RabinTrim respectively
    bool genL_trim_result = IsBuechiTrim(genL_copy);
    bool rabK_trim_result = IsRabinTrim(rabK_copy);
    
    if (!genL_trim_result) {
        FD_DF("RabinLanguageInclusion(): genL trim failed");
        return false;
    }
    
    if (!rabK_trim_result) {
        FD_DF("RabinLanguageInclusion(): rabK trim failed");
        return false; 
    }
    
    // Step 3: Mark all states to generate closure(L) and closure(K)
    genL_copy.InjectMarkedStates(genL_copy.States());
    rabK_copy.InjectMarkedStates(rabK_copy.States());

    // Step 4: Test ordinary *-language inclusion: closure(L) ⊆ closure(K)
    bool closure_inclusion = LanguageInclusion(genL_copy, rabK_copy);
    
    if (closure_inclusion) {
        std::cout << "✓ closure(SupervisedSystem) ⊆ closure(specification)" << std::endl;
        return true;
    }
    else {
        std::cout << "RabinLanguageInclusion(): Closure inclusion failed - bailing out" << std::endl;
        return false;
    }

    // Step 5: Use Automaton() on original genL and rabK to make them full
    Generator genL_full = rGenL;
    RabinAutomaton rabK_full = rRabK;
    
    // Make automata complete (full)
    genL_full.Complete();
    rabK_full.Complete();
    
    FD_DF("RabinLanguageInclusion(): Step 5: Made automata complete - genL: " << genL_full.Size() 
          << " states, rabK: " << rabK_full.Size() << " states");
    
    // Step 6: Use RabinBuechiAutomaton to construct product system
    RabinAutomaton rgProduct;
    RabinBuechiAutomaton(rabK_full, genL_full, rgProduct);
    rgProduct.Name("Product System (rabK x genL)");
    
    if (rgProduct.Size() == 0) {
        FD_DF("RabinLanguageInclusion(): Empty product system - languages are disjoint");
        return false;
    }
    
    // Make product accessible
    rgProduct.Accessible();

    // Step 7: Continue with SCC analysis on rgProduct

    // Identify L-marked states in product (corresponding to genL marked states)
    StateSet lMarkedInProduct;
    StateSet::Iterator sit;
    for (sit = rgProduct.StatesBegin(); sit != rgProduct.StatesEnd(); ++sit) {
        if (rgProduct.ExistsMarkedState(*sit)) {
            lMarkedInProduct.Insert(*sit);
        }
    }
    
    FD_DF("RabinLanguageInclusion(): Found " << lMarkedInProduct.Size() << " L-marked states in product");
    
    // Process each Rabin pair from original specification
    const RabinAcceptance& acceptance = rRabK.RabinAcceptance();
    RabinAcceptance::CIterator rait;
    
    for (rait = acceptance.Begin(); rait != acceptance.End(); ++rait) {
        const StateSet& R = rait->RSet();  // R set (must be visited infinitely often)
        const StateSet& I = rait->ISet();  // I set (complement will be muted)
        
        FD_DF("RabinLanguageInclusion(): Processing Rabin pair: R=" << R.ToString() << ", I=" << I.ToString());
        
        // Create states to mute: L-marked states + complement of I
        StateSet statesToMute = lMarkedInProduct;
        
        // Add complement of I to muted states
        for (sit = rgProduct.StatesBegin(); sit != rgProduct.StatesEnd(); ++sit) {
            if (!I.Exists(*sit)) {
                statesToMute.Insert(*sit);
            }
        }
        
        FD_DF("RabinLanguageInclusion(): Muting " << statesToMute.Size() << " states (L-marked + complement of I)");
        
        // Create muted automaton
        Generator mutedProduct = CreateMutedAutomaton(rgProduct, statesToMute);
        
        if (mutedProduct.Size() == 0) {
            FD_DF("RabinLanguageInclusion(): Muted product is empty - continuing to next Rabin pair");
            continue;
        }
        
        // Compute SCCs in muted product using libFAUDES function
        std::list<StateSet> sccList;
        StateSet sccRoots;
        ComputeScc(mutedProduct, sccList, sccRoots);
        
        FD_DF("RabinLanguageInclusion(): Found " << sccList.size() << " SCCs in muted system");
        
        // Check if any SCC contains R-marked states
        bool foundProblematicSCC = false;
        std::list<StateSet>::iterator sccIt;
        int sccIndex = 0;
        for (sccIt = sccList.begin(); sccIt != sccList.end(); ++sccIt, ++sccIndex) {
            bool hasRMarked = false;
            StateSet::Iterator sccit;
            for (sccit = sccIt->Begin(); sccit != sccIt->End(); ++sccit) {
                if (R.Exists(*sccit)) {
                    hasRMarked = true;
                    break;
                }
            }
            
            if (hasRMarked && sccIt->Size() > 1) {  // Non-trivial SCC with R-marked state
                FD_DF("RabinLanguageInclusion(): Found problematic SCC " << sccIndex << " with R-marked states");
                foundProblematicSCC = true;
                break;
            }
        }
        
        if (foundProblematicSCC) {
            FD_DF("RabinLanguageInclusion(): Language inclusion violated for Rabin pair");
            return false;
        }
    }
    
    FD_DF("RabinLanguageInclusion(): All tests passed - language inclusion L ⊆ K holds");
    return true;
}

} // namespace faudes