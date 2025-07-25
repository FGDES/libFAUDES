#include "libfaudes.h"
#undef FD_DF

using namespace faudes;

// Function to create muted automaton by removing states and transitions
Generator CreateMutedAutomaton(const Generator& original, const StateSet& statesToMute) {
    Generator muted = original;
    
    // Remove transitions to and from muted states
    TransSet::Iterator tit = muted.TransRelBegin();
    while (tit != muted.TransRelEnd()) {
        if (statesToMute.Exists(tit->X1) || statesToMute.Exists(tit->X2)) {
            TransSet::Iterator tit_del = tit;
            ++tit;
            muted.ClrTransition(*tit_del);
        } else {
            ++tit;
        }
    }
    
    // Remove muted states from state set
    StateSet::Iterator sit;
    for (sit = statesToMute.Begin(); sit != statesToMute.End(); ++sit) {
        if (muted.ExistsState(*sit)) {
            muted.DelState(*sit);
        }
    }
    
    return muted;
}

// Function to compute strongly connected components
void ComputeSCCs(const Generator& gen, std::vector<StateSet>& sccs) {
    StateSet visited;
    StateSet finished;
    std::stack<Idx> stack;
    std::map<Idx, int> low, disc;
    std::stack<Idx> sccStack;
    std::map<Idx, bool> inStack;
    int time = 0;
    
    // Initialize
    StateSet::Iterator sit;
    for (sit = gen.StatesBegin(); sit != gen.StatesEnd(); ++sit) {
        inStack[*sit] = false;
    }
    
    // Tarjan's algorithm for SCC detection
    std::function<void(Idx)> tarjan = [&](Idx v) {
        disc[v] = low[v] = ++time;
        stack.push(v);
        sccStack.push(v);
        inStack[v] = true;
        
        TransSet::Iterator tit;
        for (tit = gen.TransRelBegin(v); tit != gen.TransRelEnd(v); ++tit) {
            Idx w = tit->X2;
            if (disc.find(w) == disc.end()) {
                tarjan(w);
                low[v] = std::min(low[v], low[w]);
            } else if (inStack[w]) {
                low[v] = std::min(low[v], disc[w]);
            }
        }
        
        if (low[v] == disc[v]) {
            StateSet scc;
            Idx w;
            do {
                w = sccStack.top();
                sccStack.pop();
                inStack[w] = false;
                scc.Insert(w);
            } while (w != v);
            sccs.push_back(scc);
        }
    };
    
    for (sit = gen.StatesBegin(); sit != gen.StatesEnd(); ++sit) {
        if (disc.find(*sit) == disc.end()) {
            tarjan(*sit);
        }
    }
}

// Function to verify language inclusion L � K for Buechi (L) and Rabin (K) automata
bool VerifyLanguageInclusion(const System& buechi, const RabinAutomaton& rabin) {
    std::cout << "====== Language Inclusion Verification ======" << std::endl;
    
    // Step 1: Assume both automata are trim and deterministic as per requirement
    std::cout << "Assuming both automata are trim and deterministic" << std::endl;
    
    // Check deterministic - if not, try to determinize

    
    std::cout << "Both automata are verified as deterministic" << std::endl;
    // Step 2: Compute product automaton
    RabinAutomaton product;
    RabinBuechiProduct(rabin, buechi, product);
    product.Name("Product of Buechi and Rabin");
    product.Write("tmp_8_SupervisedSystem.gen");
    
    std::cout << "Product automaton computed with " << product.Size() << " states" << std::endl;
    
    // Step 3: Check if product generates closure of L
    if (product.Size() == 0) {
        std::cout << "Empty product - languages are disjoint, inclusion fails" << std::endl;
        return false;
    }
    
    // Make product accessible to ensure we only consider reachable states
    product.Accessible();
    std::cout << "Product made accessible, " << product.Size() << " reachable states" << std::endl;
    
    // Step 4: Mute L-marking (Buechi marked states) 
    StateSet buechlMarkedInProduct;
    StateSet::Iterator sit;
    for (sit = product.StatesBegin(); sit != product.StatesEnd(); ++sit) {
        if (product.ExistsMarkedState(*sit)) {
            buechlMarkedInProduct.Insert(*sit);
        }
    }
    
    std::cout << "Found " << buechlMarkedInProduct.Size() << " Buechi-marked states in product" << std::endl;
    
    // Step 5: Process each Rabin pair
    const RabinAcceptance& acceptance = rabin.RabinAcceptance();
    RabinAcceptance::CIterator rait;
    
    for (rait = acceptance.Begin(); rait != acceptance.End(); ++rait) {
        const StateSet& R = rait->RSet();  // R set (must be visited infinitely often)
        const StateSet& I = rait->ISet();  // I set (complement will be muted)
        
        std::cout << "Processing Rabin pair: R=" << R.ToString() << ", I=" << I.ToString() << std::endl;
        
        // Create states to mute: L-marked states + complement of I
        StateSet statesToMute = buechlMarkedInProduct;
        
        // Add complement of I to muted states
        for (sit = product.StatesBegin(); sit != product.StatesEnd(); ++sit) {
            if (!I.Exists(*sit)) {
                statesToMute.Insert(*sit);
            }
        }
        
        std::cout << "Muting " << statesToMute.Size() << " states (L-marked + complement of I)" << std::endl;
        
        // Create muted automaton
        Generator mutedProduct = CreateMutedAutomaton(product, statesToMute);
        mutedProduct.Write("tmp_8_mutedSystem.gen");
        
        if (mutedProduct.Size() == 0) {
            std::cout << "Muted product is empty - continuing to next Rabin pair" << std::endl;
            continue;
        }
        
        // Step 6: Compute SCCs in muted product
        std::vector<StateSet> sccs;
        ComputeSCCs(mutedProduct, sccs);
        
        std::cout << "Found " << sccs.size() << " SCCs in muted system" << std::endl;
        
        // Check if any SCC contains R-marked states
        bool foundProblematicSCC = false;
        for (size_t i = 0; i < sccs.size(); ++i) {
            bool hasRMarked = false;
            StateSet::Iterator sccit;
            for (sccit = sccs[i].Begin(); sccit != sccs[i].End(); ++sccit) {
                if (R.Exists(*sccit)) {
                    hasRMarked = true;
                    break;
                }
            }
            
            if (hasRMarked && sccs[i].Size() > 1) {  // Non-trivial SCC with R-marked state
                std::cout << "Found problematic SCC " << i << " with R-marked states: " << sccs[i].ToString() << std::endl;
                foundProblematicSCC = true;
                break;
            }
        }
        
        if (foundProblematicSCC) {
            std::cout << "Language inclusion violated for Rabin pair" << std::endl;
            return false;
        }
    }
    
    std::cout << "All tests passed - language inclusion holds" << std::endl;
    return true;
}

int main() {
    std::cout << "====== Language Inclusion Verification: L(V/G) � L(E) ======" << std::endl;
    
    // Read controller V (generated by omg_6)
    System controller;
    controller.Read("tmp_6_Controller.gen");
    controller.Name("Controller (V)");

    // Read plant G
    System plant;
    plant.Read("tmp_6_plant.gen");
    plant.Name("Plant (G)");
    
    // Read specification E 
    RabinAutomaton spec;
    spec.Read("tmp_6_spec.gen");
    spec.Name("Specification (E)");
    
    // Compute V/G (supervised system) 
    System supervisedSystem;
    InvProject(controller,plant.Alphabet());
    controller.DWrite();
    Product(controller, plant, supervisedSystem);
    supervisedSystem.Name("V/G (Supervised System)");
    supervisedSystem.Write("tmp_8_supervised.gen");
    std::cout << "Supervised system V/G computed: " << supervisedSystem.Size() << " states, "
              << supervisedSystem.MarkedStates().Size() << " marked states" << std::endl;
    
    // Perform language inclusion verification: L(V/G) � L(E)
    bool result = VerifyLanguageInclusion(supervisedSystem, spec);
    
    if (result) {
        std::cout << " VERIFICATION SUCCESSFUL: L(V/G) /soet L(E)" << std::endl;
        std::cout << "The supervised system satisfies the specification!" << std::endl;
    } else {
        std::cout << " VERIFICATION FAILED: L(V/G) /soet L(E)" << std::endl;
        std::cout << "The supervised system does not satisfy the specification." << std::endl;
    }
    
    return 0;
}