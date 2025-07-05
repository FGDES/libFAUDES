/** @file omg_pseudodet.cpp 
 * 
 * Implementation of pseudo-determinization algorithm for Rabin automata
 * Fixed to strictly follow the paper algorithm
 */

#include "libfaudes.h"
#include "omg_pseudodet.h"

namespace faudes {

/*
********************************
TreeNode Implementation
********************************
*/

TreeNode::TreeNode() : color(WHITE) {}

bool TreeNode::operator<(const TreeNode& other) const {
    if(stateLabel != other.stateLabel) return stateLabel < other.stateLabel;
    if(aSet != other.aSet) return aSet < other.aSet;
    if(rSet != other.rSet) return rSet < other.rSet;
    if(children != other.children) return children < other.children;
    return color < other.color;
}

bool TreeNode::operator==(const TreeNode& other) const {
    return stateLabel == other.stateLabel && 
           aSet == other.aSet && 
           rSet == other.rSet && 
           children == other.children && 
           color == other.color;
}

std::string TreeNode::ToString() const {
    std::ostringstream oss;
    oss << "Node{states=[" << stateLabel.ToString() << "], ";
    oss << "color=" << (color == WHITE ? "WHITE" : (color == RED ? "RED" : "GREEN")) << ", ";
    oss << "children=" << children.size() << ", ";
    oss << "aSet=" << aSet.size() << ", ";
    oss << "rSet=" << rSet.size() << "}";
    return oss.str();
}

/*
********************************
LabeledTree Implementation
********************************
*/

LabeledTree::LabeledTree() : rootNode(INVALID_NODE), nextNodeId(1) {}

Idx LabeledTree::createNode() {
    Idx newId = nextNodeId++;
    nodes[newId] = TreeNode();
    return newId;
}

void LabeledTree::deleteNode(Idx nodeId) {
    if(nodes.find(nodeId) == nodes.end()) return;
    
    // Remove this node from all parent children lists
    for(auto& pair : nodes) {
        TreeNode& node = pair.second;
        auto it = std::find(node.children.begin(), node.children.end(), nodeId);
        if(it != node.children.end()) {
            node.children.erase(it);
        }
        
        // Remove from A-sets and R-sets
        node.aSet.erase(nodeId);
        node.rSet.erase(nodeId);
    }
    
    nodes.erase(nodeId);
}

std::string LabeledTree::ToString() const {
    std::ostringstream oss;
    oss << "Tree{root=" << rootNode << ", nodes=[";
    for(auto& pair : nodes) {
        oss << pair.first << ":" << pair.second.ToString() << ", ";
    }
    oss << "]}";
    return oss.str();
}

bool LabeledTree::operator<(const LabeledTree& other) const {
    if(rootNode != other.rootNode) return rootNode < other.rootNode;
    return nodes < other.nodes;
}

bool LabeledTree::operator==(const LabeledTree& other) const {
    if(rootNode != other.rootNode) return false;
    if(nodes.size() != other.nodes.size()) return false;
    
    for(auto it1 = nodes.begin(), it2 = other.nodes.begin();
        it1 != nodes.end() && it2 != other.nodes.end(); ++it1, ++it2) {
        if(it1->first != it2->first) return false;
        if(!(it1->second == it2->second)) return false;
    }
    
    return true;
}

/*
********************************
Helper Functions
********************************
*/

std::string ComputeTreeSignature(const LabeledTree& tree) {
    std::ostringstream oss;
    
    // Recursive signature processing
    std::function<void(Idx)> dfs = [&](Idx nodeId) {
        if(tree.nodes.find(nodeId) == tree.nodes.end()) return;
        
        const TreeNode& node = tree.nodes.at(nodeId);
        oss << nodeId << ":";
        oss << node.stateLabel.ToString() << ":";
        oss << static_cast<int>(node.color) << ":";
        
        // Process children
        for(Idx childId : node.children) {
            dfs(childId);
        }
        
        oss << ";";
    };
    
    dfs(tree.rootNode);
    return oss.str();
}

/*
********************************
Main Algorithm Implementation - FIXED
********************************
*/

void PseudoDet(const RabinAutomaton& rGen, RabinAutomaton& rRes) {
    FD_DF("PseudoDet(" << rGen.Name() << ")");
    
    // Get input Rabin automaton acceptance condition
    RabinAcceptance inputRabinPairs = rGen.RabinAcceptance();
    
    rRes.Clear();
    rRes.Name(CollapsString("PseudoDet(" + rGen.Name() + ")"));
    
    
    // Copy alphabet
    rRes.InjectAlphabet(rGen.Alphabet());
    
    // Safety limits (increased but still reasonable)
    const int MAX_STATES = 10000;
    const int MAX_ITERATIONS = 100000;
    int stateCounter = 0;
    int iterationCounter = 0;
    
    // Store tree for each state
    std::map<Idx, LabeledTree> stateToTree;
    
    // Track tree signatures
    std::map<std::string, Idx> treeSignatureToState;
    
    // Create initial tree
    LabeledTree initialTree;
    Idx root = initialTree.createNode();
    initialTree.rootNode = root;
    
    // Set root node label to contain all initial states
    for(StateSet::Iterator sit = rGen.InitStatesBegin(); sit != rGen.InitStatesEnd(); ++sit) {
        initialTree.nodes[root].stateLabel.Insert(*sit);
    }
    
    // Create initial state in output automaton
    Idx initialState = rRes.InsInitState();
    stateToTree[initialState] = initialTree;
    
    std::string initialSig = ComputeTreeSignature(initialTree);
    treeSignatureToState[initialSig] = initialState;
    
    std::queue<Idx> stateQueue;
    stateQueue.push(initialState);
    stateCounter++;
    
    // Process all states
    while(!stateQueue.empty() && stateCounter < MAX_STATES && iterationCounter < MAX_ITERATIONS) {
        iterationCounter++;
        
        Idx currentState = stateQueue.front();
        stateQueue.pop();
        
        LabeledTree currentTree = stateToTree[currentState];
        FD_DF("Processing state " << currentState << " with tree: " 
                  << currentTree.ToString());
        
        // Process each event
        for(EventSet::Iterator evIt = rGen.AlphabetBegin(); evIt != rGen.AlphabetEnd(); ++evIt) {
            Idx event = *evIt;
            FD_DF("Processing event " << rGen.EventName(event));
            
            // Clone tree
            LabeledTree newTree = currentTree;
            
            // STEP 1: Color all nodes white
            for(auto& pair : newTree.nodes) {
                pair.second.color = TreeNode::WHITE;
            }
            
            // STEP 2: Update state labels based on transitions
            // Check if sigma_c is epsilon
            std::string eventName = rGen.EventName(event);
            bool isEpsilonEvent = (eventName == "epsilon" || eventName.find("eps") != std::string::npos);
            
            if (isEpsilonEvent) {
                // Case 2a: If σ_c = ε, replace every state label Y with δ_v(ε, Y) ∪ Y
                for(auto& pair : newTree.nodes) {
                    TreeNode& node = pair.second;
                    StateSet newLabel = node.stateLabel; // Start with Y
                    
                    // Add δ_v(ε, Y)
                    for(StateSet::Iterator sit = node.stateLabel.Begin(); sit != node.stateLabel.End(); ++sit) {
                        Idx state = *sit;
                        for(TransSet::Iterator tit = rGen.TransRelBegin(state, event); 
                            tit != rGen.TransRelEnd(state, event); ++tit) {
                            newLabel.Insert(tit->X2);
                        }
                    }
                    node.stateLabel = newLabel;
                }
            } else {
                // Case 2b: If σ_c ∈ Σ_o, check condition Y_r ⊇ δ_v(ε, Y_r)
                StateSet rootLabel = newTree.nodes[newTree.rootNode].stateLabel;
                StateSet epsilonSuccessors;
                
                // Compute δ_v(ε, Y_r) - find epsilon transitions from root states
                for(StateSet::Iterator sit = rootLabel.Begin(); sit != rootLabel.End(); ++sit) {
                    Idx state = *sit;
                    // Look for epsilon transitions
                    for(EventSet::Iterator eit = rGen.AlphabetBegin(); eit != rGen.AlphabetEnd(); ++eit) {
                        std::string eName = rGen.EventName(*eit);
                        if(eName == "epsilon" || eName.find("eps") != std::string::npos) {
                            for(TransSet::Iterator tit = rGen.TransRelBegin(state, *eit); 
                                tit != rGen.TransRelEnd(state, *eit); ++tit) {
                                epsilonSuccessors.Insert(tit->X2);
                            }
                        }
                    }
                }
                
                // Check if Y_r ⊇ δ_v(ε, Y_r)
                bool conditionSatisfied = true;
                for(StateSet::Iterator sit = epsilonSuccessors.Begin(); sit != epsilonSuccessors.End(); ++sit) {
                    if(!rootLabel.Exists(*sit)) {
                        conditionSatisfied = false;
                        break;
                    }
                }
                
                if(conditionSatisfied) {
                    // Replace every state label Y with δ_v(σ_c, Y)
                    for(auto& pair : newTree.nodes) {
                        TreeNode& node = pair.second;
                        StateSet newLabel;
                        
                        for(StateSet::Iterator sit = node.stateLabel.Begin(); sit != node.stateLabel.End(); ++sit) {
                            Idx state = *sit;
                            for(TransSet::Iterator tit = rGen.TransRelBegin(state, event); 
                                tit != rGen.TransRelEnd(state, event); ++tit) {
                                newLabel.Insert(tit->X2);
                            }
                        }
                        node.stateLabel = newLabel;
                    }
                } else {
                    // Transition function is undefined, skip this event
                    continue;
                }
            }
            
            // Check if there exists a state in root that has no transitions for ALL observable events and epsilon
            // According to algorithm step 2: skip event only if such a state exists
            StateSet rootLabel = newTree.nodes[newTree.rootNode].stateLabel;
            bool shouldSkipEvent = false;
            
            for(StateSet::Iterator sit = rootLabel.Begin(); sit != rootLabel.End(); ++sit) {
                Idx state = *sit;
                bool hasAnyTransition = false;
                
                // Check if this state has transitions for any observable event or epsilon
                for(EventSet::Iterator evIt = rGen.AlphabetBegin(); evIt != rGen.AlphabetEnd(); ++evIt) {
                    Idx checkEvent = *evIt;
                    std::string eventName = rGen.EventName(checkEvent);
                    
                    // Check observable events and epsilon
                    if(rGen.Observable(checkEvent) || eventName == "epsilon" || eventName.find("eps") != std::string::npos) {
                        for(TransSet::Iterator tit = rGen.TransRelBegin(state, checkEvent); 
                            tit != rGen.TransRelEnd(state, checkEvent); ++tit) {
                            hasAnyTransition = true;
                            break;
                        }
                        if(hasAnyTransition) break;
                    }
                }
                
                // If this state has no transitions for all observable events and epsilon, skip current event
                if(!hasAnyTransition) {
                    shouldSkipEvent = true;
                    break;
                }
            }
            
            if(shouldSkipEvent) {
                continue; // Skip this event according to algorithm step 2
            }
            
            // STEP 3: Create nodes for Rabin acceptance violations
            // Paper: "For every node n with state label Y ⊆ Xv such that Y ∩ (Xv \ Iv) ≠ ∅, 
            // create a new node n' which becomes the newest child of n. Color n' red, 
            // and let its state label be Y ∩ (Xv \ Iv) and its A-set and R-set be the empty set."
            if(inputRabinPairs.Size() > 0) {
                // Create a copy of current nodes to iterate over (avoid modification during iteration)
                std::vector<std::pair<Idx, TreeNode>> currentNodes;
                for(auto& pair : newTree.nodes) {
                    currentNodes.push_back(pair);
                }
                
                for(auto& nodePair : currentNodes) {
                    Idx nodeId = nodePair.first;
                    TreeNode& node = newTree.nodes[nodeId]; // Get current reference
                    
                    // For each Rabin pair (R, I)
                    for(RabinAcceptance::CIterator rit = inputRabinPairs.Begin(); 
                        rit != inputRabinPairs.End(); ++rit) {
                        
                        const StateSet& I = rit->ISet();
                        
                        // Check if Y ∩ (Xv \ I) ≠ ∅
                        StateSet intersectionWithoutI;
                        for(StateSet::Iterator sit = node.stateLabel.Begin(); 
                            sit != node.stateLabel.End(); ++sit) {
                            if(!I.Exists(*sit)) {
                                intersectionWithoutI.Insert(*sit);
                            }
                        }
                        
                        // If intersection is non-empty, create child
                        if(!intersectionWithoutI.Empty()) {
                            Idx newChild = newTree.createNode();
                            newTree.nodes[newChild].stateLabel = intersectionWithoutI;
                            newTree.nodes[newChild].color = TreeNode::RED;
                            newTree.nodes[newChild].aSet.clear();
                            newTree.nodes[newChild].rSet.clear();
                            node.children.push_back(newChild);
                            
                            FD_DF("Created RED child node " << newChild 
                                     << " for node " << nodeId << " with " 
                                     << intersectionWithoutI.Size() << " states");
                        }
                    }
                }
            }
            
            // STEP 4: Maintain state disjointness among siblings
            // Paper: "For every node n with state label Y and state x ∈ Y that also belongs to 
            // the state label of an older sibling of n, remove x from the state labels of n 
            // and all of its descendants."
            for(auto& pair : newTree.nodes) {
                Idx parentId = pair.first;
                TreeNode& parent = pair.second;
                
                // For each child, remove states that appear in older siblings
                for(size_t i = 1; i < parent.children.size(); ++i) {
                    Idx youngerId = parent.children[i];
                    
                    if(newTree.nodes.find(youngerId) == newTree.nodes.end()) continue;
                    
                    // Check all older siblings
                    for(size_t j = 0; j < i; ++j) {
                        Idx olderId = parent.children[j];
                        
                        if(newTree.nodes.find(olderId) == newTree.nodes.end()) continue;
                        
                        // Remove states from younger sibling and ALL its descendants
                        std::vector<Idx> toProcess;
                        toProcess.push_back(youngerId);
                        
                        while(!toProcess.empty()) {
                            Idx currentId = toProcess.back();
                            toProcess.pop_back();
                            
                            if(newTree.nodes.find(currentId) == newTree.nodes.end()) continue;
                            
                            TreeNode& currentNode = newTree.nodes[currentId];
                            
                            // Remove states that appear in older sibling
                            for(StateSet::Iterator sit = newTree.nodes[olderId].stateLabel.Begin(); 
                                sit != newTree.nodes[olderId].stateLabel.End(); ++sit) {
                                currentNode.stateLabel.Erase(*sit);
                            }
                            
                            // Add children to processing queue
                            for(Idx childId : currentNode.children) {
                                toProcess.push_back(childId);
                            }
                        }
                    }
                }
            }
            
            // STEP 5: Remove all nodes with empty state labels
            std::vector<Idx> nodesToRemove;
            for(auto& pair : newTree.nodes) {
                if(pair.second.stateLabel.Empty()) {
                    nodesToRemove.push_back(pair.first);
                }
            }
            
            for(Idx nodeId : nodesToRemove) {
                newTree.deleteNode(nodeId);
            }
            
            // STEP 6: Determine red breakpoints
            // Paper: "If the state label of any node n is equal to the union of the state labels of the
            // children of n, remove all descendants of n and color n red. Let the A- and R-sets of n be the empty set."
            for(auto& pair : newTree.nodes) {
                Idx nodeId = pair.first;
                TreeNode& node = pair.second;
                
                // Compute union of children's state labels
                StateSet unionOfChildren;
                for(Idx childId : node.children) {
                    if(newTree.nodes.find(childId) == newTree.nodes.end()) continue;
                    
                    for(StateSet::Iterator sit = newTree.nodes[childId].stateLabel.Begin(); 
                        sit != newTree.nodes[childId].stateLabel.End(); ++sit) {
                        unionOfChildren.Insert(*sit);
                    }
                }
                
                if(node.stateLabel == unionOfChildren && !unionOfChildren.Empty()) {
                    FD_DF("Red breakpoint at node " << nodeId);
                    node.color = TreeNode::RED;
                    
                    // Delete all descendants
                    std::vector<Idx> descendants;
                    std::queue<Idx> bfs;
                    for(Idx child : node.children) {
                        if(newTree.nodes.find(child) != newTree.nodes.end()) {
                            bfs.push(child);
                        }
                    }
                    
                    while(!bfs.empty()) {
                        Idx current = bfs.front();
                        bfs.pop();
                        descendants.push_back(current);
                        
                        if(newTree.nodes.find(current) == newTree.nodes.end()) continue;
                        
                        for(Idx child : newTree.nodes[current].children) {
                            if(newTree.nodes.find(child) != newTree.nodes.end()) {
                                bfs.push(child);
                            }
                        }
                    }
                    
                    for(Idx descendant : descendants) {
                        newTree.deleteNode(descendant);
                    }
                    
                    node.children.clear();
                    node.aSet.clear();
                    node.rSet.clear();
                }
            }
            
            // STEP 7: Delete the nodes removed in the above two steps from the A- and R-sets of all other nodes
            // (This is already handled by deleteNode method)
            
            // STEP 8: If the A-set of a node n is empty and n is not colored red, then color n green
            // and set its A-set equal to its R-set. Then let its R-set be empty.
            for(auto& pair : newTree.nodes) {
                Idx nodeId = pair.first;
                TreeNode& node = pair.second;
                
                if(node.aSet.empty() && node.color != TreeNode::RED) {
                    FD_DF("Green coloring for node " << nodeId);
                    node.color = TreeNode::GREEN;
                    node.aSet = node.rSet;
                    node.rSet.clear();
                }
            }
            
            // STEP 9: If a node is not colored red, then add to its R-set the set of all other nodes
            // presently colored red.
            std::set<Idx> redNodes;
            for(auto& pair : newTree.nodes) {
                if(pair.second.color == TreeNode::RED) {
                    redNodes.insert(pair.first);
                }
            }
            
            for(auto& pair : newTree.nodes) {
                if(pair.second.color != TreeNode::RED) {
                    for(Idx redNode : redNodes) {
                        if(newTree.nodes.find(redNode) != newTree.nodes.end()) {
                            pair.second.rSet.insert(redNode);
                        }
                    }
                }
            }
            
            // Check if this tree was seen before
            std::string treeSig = ComputeTreeSignature(newTree);
            Idx targetState;
            
            auto stateIt = treeSignatureToState.find(treeSig);
            if(stateIt != treeSignatureToState.end()) {
                // If so, use existing state
                targetState = stateIt->second;
                FD_DF("Found existing state " << targetState << " for tree");
            } else {
                // Create new state for this tree
                targetState = rRes.InsState();
                stateToTree[targetState] = newTree;
                treeSignatureToState[treeSig] = targetState;
                
                stateQueue.push(targetState);
                stateCounter++;
                
                FD_DF("Created new state " << targetState << " for tree");
                
                // Set marking based on Rabin acceptance condition
                bool shouldMark = false;
                
                // Mark states that contain green nodes but no red nodes
                bool hasGreen = false;
                bool hasRed = false;
                
                for(auto& nodePair : newTree.nodes) {
                    if(nodePair.second.color == TreeNode::GREEN) hasGreen = true;
                    if(nodePair.second.color == TreeNode::RED) hasRed = true;
                }
                
                if(hasGreen && !hasRed) {
                    shouldMark = true;
                }
                
                if(shouldMark) {
                    rRes.SetMarkedState(targetState);
                    FD_DF("Marking state " << targetState);
                }
            }
            
            // Add transition from current state to target state
            rRes.SetTransition(currentState, event, targetState);
        }
    }
    
    if(stateCounter >= MAX_STATES) {
        FD_DF("Warning: Reached maximum state limit of " << MAX_STATES);
    }
    
    if(iterationCounter >= MAX_ITERATIONS) {
        FD_DF("Warning: Reached maximum iteration limit of " << MAX_ITERATIONS);
    }
    
    // Create Rabin pairs for output automaton
    RabinAcceptance outputRabinPairs;

    // First, collect all unique node IDs that have ever been created.
    std::set<Idx> allNodeIds;
    for (const auto& statePair : stateToTree) {
        const LabeledTree& tree = statePair.second;
        for(const auto& nodePair : tree.nodes) {
            allNodeIds.insert(nodePair.first);
        }
    }

    // Now, create a Rabin pair for each unique node ID.
    for (Idx nodeId : allNodeIds) {
        if (nodeId == LabeledTree::INVALID_NODE) continue;

        StateSet Rn; // States where this nodeId is GREEN
        StateSet In; // States where this nodeId is NOT RED and NOT DELETED

        // Iterate through all generated states of the result automaton.
        for (const auto& statePair : stateToTree) {
            Idx state = statePair.first;
            const LabeledTree& tree = statePair.second;

            // Check if 'nodeId' exists in this tree's nodes.
            // Its existence means it is "not deleted" for this tree state.
            auto nodeIt = tree.nodes.find(nodeId);
            if (nodeIt != tree.nodes.end()) {
                const TreeNode& node = nodeIt->second;

                // Populate In: node must exist and not be RED.
                if (node.color != TreeNode::RED) {
                    In.Insert(state);
                }

                // Populate Rn: node must be GREEN.
                if (node.color == TreeNode::GREEN) {
                    Rn.Insert(state);
                }
            }
        }

        // According to the paper's logic, a pair is meaningful if the condition
        // of infinite visits (green coloring) can actually happen.
        // So, we add the pair only if the set Rn is not empty.
        if (!Rn.Empty()) {
            RabinPair newPair;
            // RSet is the set of states to be visited infinitely often.
            newPair.RSet() = Rn; 
            // ISet is the set of states to be eventually stayed within.
            newPair.ISet() = In; 
            outputRabinPairs.Insert(newPair);
        }
    }

    rRes.RabinAcceptance() = outputRabinPairs;
    
    FD_DF("PseudoDet completed with " 
              << stateCounter << " states and "
              << outputRabinPairs.Size() << " RabinPairs");
              
    // FD_DF("GlobalR has " << globalR.Size() << " states");
    // FD_DF("GlobalI has " << globalI.Size() << " states");
}

} // namespace faudes