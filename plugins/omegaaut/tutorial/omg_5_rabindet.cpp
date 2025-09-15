/** @file omg_5_rabindet.cpp
 *
 * Example usage of pseudo-determinization algorithm for Rabin automata
 *
 * This example demonstrates how to use the PseudoDet function to convert
 * nondeterministic Rabin automata to deterministic ones.
 *
 * @ingroup Tutorials
 * @include omg_5_rabindet.cpp
 */

 #include "libfaudes.h"
 #include <chrono>
 #include <ctime>
 #include <cstdlib>
 
 using namespace faudes;

/**
 * Generate a random nondeterministic Rabin automaton
 * @param numStates Number of states (3-100)
 * @param numEvents Number of events (3-7) 
 * @return Generated NRA
 */
RabinAutomaton GenerateRandomNRA(int numStates, int numEvents) {
    RabinAutomaton nra;
    
    // Create states (1, 2, 3, ...)
    for(int i = 1; i <= numStates; ++i) {
        nra.InsState(i);
    }
    
    // Set initial state (always state 1)
    nra.SetInitState(1);
    
    // Set some marked states (randomly select 1-2 states)
    unsigned int numMarked = 1 + (rand() % std::min(2, numStates));
    std::set<int> markedStates;
    while(markedStates.size() < numMarked) {
        int state = 1 + (rand() % numStates);
        markedStates.insert(state);
        nra.SetMarkedState(state);
    }
    
    // Create events (a, b, c, ...)
    std::vector<Idx> events;
    for(int i = 0; i < numEvents; ++i) {
        char eventName = 'a' + i;
        Idx event = nra.InsEvent(std::string(1, eventName));
        events.push_back(event);
    }
    
    // Generate exactly numStates * numEvents transitions
    int targetTransitions = numStates * numEvents;
    int transitionsAdded = 0;
    
    // First ensure each state has at least one outgoing transition
    for(int state = 1; state <= numStates; ++state) {
        Idx event = events[rand() % events.size()];
        Idx target = 1 + (rand() % numStates);
        
        if(!nra.ExistsTransition(state, event, target)) {
            nra.SetTransition(state, event, target);
            transitionsAdded++;
        }
    }
    
    // Add remaining transitions randomly
    while(transitionsAdded < targetTransitions) {
        Idx source = 1 + (rand() % numStates);
        Idx event = events[rand() % events.size()];
        Idx target = 1 + (rand() % numStates);
        
        if(!nra.ExistsTransition(source, event, target)) {
            nra.SetTransition(source, event, target);
            transitionsAdded++;
        }
    }
    
    // Create simple Rabin acceptance condition (1 pair)
    RabinAcceptance rabinPairs;
    RabinPair pair;
    
    // R-set: select 1-2 random states
    unsigned int rSize = 1 + (rand() % std::min(2, numStates));
    std::set<int> rStates;
    while(rStates.size() < rSize) {
        int state = 1 + (rand() % numStates);
        rStates.insert(state);
        pair.RSet().Insert(state);
    }
    
    // I-set: select 2-4 random states (include all states by default)
    for(int state = 1; state <= numStates; ++state) {
        pair.ISet().Insert(state);
    }
    
    rabinPairs.Insert(pair);
    nra.RabinAcceptance() = rabinPairs;
    
    return nra;
}

/**
 * Validation function to test PseudoDet algorithm with random NRAs
 */
void validate() {
    std::cout << "======== PseudoDet Algorithm Validation ========" << std::endl;
    
    // Initialize random seed
    srand(time(nullptr));
    
    // Test parameters
    std::vector<int> stateCounts = {3, 5, 10, 15, 20, 30, 50, 75, 100};
    std::vector<int> eventCounts = {3, 4, 5, 7};
    int testsPerConfig = 3;
    
    std::cout << "State_Count\tEvent_Count\tNRA_States\tNRA_Trans\tDRA_States\tDRA_Trans\tTime_ms\tStatus" << std::endl;
    std::cout << "==========================================================================" << std::endl;
    
    int totalTests = 0;
    int successfulTests = 0;
    
    for(int numStates : stateCounts) {
        for(int numEvents : eventCounts) {
            for(int test = 0; test < testsPerConfig; ++test) {
                totalTests++;
                
                try {
                    // Generate random NRA
                    RabinAutomaton nra = GenerateRandomNRA(numStates, numEvents);
                    
                    // Record NRA statistics
                    int nraStates = nra.Size();
                    int nraTransitions = nra.TransRelSize();
                    
                    // Apply PseudoDet algorithm
                    RabinAutomaton dra;
                    auto startTime = std::chrono::high_resolution_clock::now();
                    
                    PseudoDet(nra, dra);
                    
                    auto endTime = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
                    
                    // Record DRA statistics
                    int draStates = dra.Size();
                    int draTransitions = dra.TransRelSize();
                    
                    std::cout << numStates << "\t\t" << numEvents << "\t\t" 
                             << nraStates << "\t\t" << nraTransitions << "\t\t"
                             << draStates << "\t\t" << draTransitions << "\t\t"
                             << duration.count() << "\t\tSUCCESS" << std::endl;
                    
                    successfulTests++;
                    
                } catch(const std::exception& e) {
                    std::cout << numStates << "\t\t" << numEvents << "\t\t"
                             << "N/A\t\tN/A\t\tN/A\t\tN/A\t\tN/A\t\tFAILED: " << e.what() << std::endl;
                } catch(...) {
                    std::cout << numStates << "\t\t" << numEvents << "\t\t"
                             << "N/A\t\tN/A\t\tN/A\t\tN/A\t\tN/A\t\tFAILED: Unknown error" << std::endl;
                }
            }
        }
    }
    
}

 
 int main(void) {
     
     std::cout << "======== Pseudo-Determinization Example ========" << std::endl;

         // Read nondeterministic Rabin automaton from file
         RabinAutomaton NRA;
         NRA.Read("pseudotest/test4.gen");
         
         // Apply pseudo-determinization algorithm
         RabinAutomaton DRA;
         PseudoDet(NRA, DRA);

	 // record test case
         FAUDES_TEST_DUMP("pseudo det result", DRA);

	 // compare with our records
         FAUDES_TEST_DIFF();

         //std::cout << "\n======== Running Validation Tests ========" << std::endl;
         //validate();

         return 0;
     }


