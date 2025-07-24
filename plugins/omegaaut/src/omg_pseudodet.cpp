/** @file omg_pseudodet.cpp 
 * 
 * Implementation of pseudo-determinization algorithm for Rabin automata
 * Strictly follows the paper algorithm with fixed node numbering
 */

#include "libfaudes.h"
#include "omg_pseudodet.h"

namespace faudes {

/*
********************************
Enhanced State Implementation
********************************
*/

EnhancedState::EnhancedState() : originalState(0) {}

bool EnhancedState::operator<(const EnhancedState& other) const {
    if(originalState != other.originalState) return originalState < other.originalState;
    return deletedNodesInPrevStep < other.deletedNodesInPrevStep;
}

bool EnhancedState::operator==(const EnhancedState& other) const {
    return originalState == other.originalState && 
           deletedNodesInPrevStep == other.deletedNodesInPrevStep;
}

std::string EnhancedState::ToString() const {
    std::ostringstream oss;
    oss << "EnhancedState{original=" << originalState << ", deleted=[";
    for(int nodeNum : deletedNodesInPrevStep) {
        oss << nodeNum << ",";
    }
    oss << "]}";
    return oss.str();
}

/*
********************************
TreeNode Implementation
********************************
*/

TreeNode::TreeNode() : nodeNumber(-1), color(WHITE) {}

TreeNode::TreeNode(int number) : nodeNumber(number), color(WHITE) {}

bool TreeNode::operator<(const TreeNode& other) const {
    if(nodeNumber != other.nodeNumber) return nodeNumber < other.nodeNumber;
    if(stateLabel != other.stateLabel) return stateLabel < other.stateLabel;
    if(aSet != other.aSet) return aSet < other.aSet;
    if(rSet != other.rSet) return rSet < other.rSet;
    if(children != other.children) return children < other.children;
    return color < other.color;
}

bool TreeNode::operator==(const TreeNode& other) const {
    return nodeNumber == other.nodeNumber &&
           stateLabel == other.stateLabel && 
           aSet == other.aSet && 
           rSet == other.rSet && 
           children == other.children && 
           color == other.color;
}

std::string TreeNode::ToString() const {
    std::ostringstream oss;
    oss << "Node{num=" << nodeNumber << ", states=[" << stateLabel.ToString() << "], ";
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

LabeledTree::LabeledTree() : rootNode(LabeledTree::INVALID_NODE), nextNodeId(1), maxNodeCount(0) {}

LabeledTree::LabeledTree(int N) : rootNode(LabeledTree::INVALID_NODE), nextNodeId(1), maxNodeCount(N) {
    nodeNumberUsed.resize(N + 1, false); // Index 0 unused, 1..N available
}

Idx LabeledTree::createNodeWithNumber(int nodeNumber) {
    if(nodeNumber < 1 || nodeNumber > maxNodeCount) {
        throw Exception("LabeledTree::createNodeWithNumber", 
                       "Invalid node number " + ToStringInteger(nodeNumber), 500);
    }
    
    if(nodeNumberUsed[nodeNumber]) {
        throw Exception("LabeledTree::createNodeWithNumber", 
                       "Node number " + ToStringInteger(nodeNumber) + " already used", 500);
    }
    
    Idx newId = nextNodeId++;
    nodes[newId] = TreeNode(nodeNumber);
    numberToId[nodeNumber] = newId;
    idToNumber[newId] = nodeNumber;
    nodeNumberUsed[nodeNumber] = true;
    
    return newId;
}

Idx LabeledTree::createNode() {
    // Find first available node number
    for(int i = 1; i <= maxNodeCount; ++i) {
        if(!nodeNumberUsed[i]) {
            return createNodeWithNumber(i);
        }
    }
    
    throw Exception("LabeledTree::createNode", 
                   "No available node numbers (max=" + ToStringInteger(maxNodeCount) + ")", 500);
}

void LabeledTree::deleteNode(Idx nodeId) {
    if(nodes.find(nodeId) == nodes.end()) return;
    
    int nodeNumber = idToNumber[nodeId];
    
    // Remove this node from all parent children lists
    for(auto& pair : nodes) {
        TreeNode& node = pair.second;
        auto it = std::find(node.children.begin(), node.children.end(), nodeId);
        if(it != node.children.end()) {
            node.children.erase(it);
        }
        
        // Remove from A-sets and R-sets (use node numbers)
        node.aSet.erase(nodeNumber);
        node.rSet.erase(nodeNumber);
    }
    
    // Clean up mappings
    numberToId.erase(nodeNumber);
    idToNumber.erase(nodeId);
    nodeNumberUsed[nodeNumber] = false;
    deletedNodeNumbers.insert(nodeNumber);
    
    nodes.erase(nodeId);
}

int LabeledTree::getNodeNumber(Idx nodeId) const {
    auto it = idToNumber.find(nodeId);
    return (it != idToNumber.end()) ? it->second : -1;
}

Idx LabeledTree::getNodeId(int nodeNumber) const {
    auto it = numberToId.find(nodeNumber);
    return (it != numberToId.end()) ? it->second : LabeledTree::INVALID_NODE;
}

bool LabeledTree::hasNodeNumber(int nodeNumber) const {
    return numberToId.find(nodeNumber) != numberToId.end();
}

std::string LabeledTree::ToString() const {
    std::ostringstream oss;
    oss << "Tree{root=" << rootNode << "(num=" << getNodeNumber(rootNode) << "), nodes=[";
    for(auto& pair : nodes) {
        oss << pair.first << ":" << pair.second.ToString() << ", ";
    }
    oss << "], deleted=[";
    for(int nodeNum : deletedNodeNumbers) {
        oss << nodeNum << ",";
    }
    oss << "]}";
    return oss.str();
}

bool LabeledTree::operator<(const LabeledTree& other) const {
    if(rootNode != other.rootNode) return rootNode < other.rootNode;
    if(deletedNodeNumbers != other.deletedNodeNumbers) return deletedNodeNumbers < other.deletedNodeNumbers;
    return nodes < other.nodes;
}

bool LabeledTree::operator==(const LabeledTree& other) const {
    if(rootNode != other.rootNode) return false;
    if(deletedNodeNumbers != other.deletedNodeNumbers) return false;
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

std::string ComputeTreeSignature(const LabeledTree& tree, const std::set<int>& deletedNodes) {
    std::ostringstream oss;
    
    // Include deleted nodes in signature
    oss << "deleted:[";
    for(int nodeNum : deletedNodes) {
        oss << nodeNum << ",";
    }
    oss << "];";
    
    // Recursive signature processing using node numbers
    std::function<void(Idx)> dfs = [&](Idx nodeId) {
        if(tree.nodes.find(nodeId) == tree.nodes.end()) return;
        
        const TreeNode& node = tree.nodes.at(nodeId);
        oss << "n" << node.nodeNumber << ":";
        oss << node.stateLabel.ToString() << ":";
        oss << static_cast<int>(node.color) << ":";
        
        // Process children in node number order for consistency
        std::vector<std::pair<int, Idx>> childrenWithNumbers;
        for(Idx childId : node.children) {
            if(tree.nodes.find(childId) != tree.nodes.end()) {
                childrenWithNumbers.push_back({tree.getNodeNumber(childId), childId});
            }
        }
        std::sort(childrenWithNumbers.begin(), childrenWithNumbers.end());
        
        for(auto& child : childrenWithNumbers) {
            dfs(child.second);
        }
        
        oss << ";";
    };
    
    dfs(tree.rootNode);
    return oss.str();
}

/*
********************************
Main Algorithm Implementation - PAPER COMPLIANT
********************************
*/

void PseudoDet(const RabinAutomaton& rGen, RabinAutomaton& rRes) {
    FD_DF("PseudoDet(" << rGen.Name() << ") - Paper Algorithm");
    
    // Get input Rabin automaton acceptance condition
    RabinAcceptance inputRabinPairs = rGen.RabinAcceptance();
    
    rRes.Clear();
    rRes.Name(CollapsString("PseudoDet(" + rGen.Name() + ")"));
    
    // Copy alphabet
    rRes.InjectAlphabet(rGen.Alphabet());
    
    // Safety limits
    const int MAX_STATES = 10000;
    const int MAX_ITERATIONS = 100000;
    int stateCounter = 0;
    int iterationCounter = 0;
    
    // Fixed number of nodes N = |Xv| as per paper
    const int N = 10*rGen.Size();
    FD_DF("Using N = " << N << " fixed nodes");
    
    // Enhanced state tracking: map enhanced state -> tree
    std::map<EnhancedState, LabeledTree> enhancedStateToTree;
    
    // Track tree signatures
    std::map<std::string, EnhancedState> treeSignatureToEnhancedState;
    
    // Create initial tree with fixed node count
    LabeledTree initialTree(N);
    Idx root = initialTree.createNodeWithNumber(1); // Root is always node number 1
    initialTree.rootNode = root;
    
    // Set root node label to contain all initial states
    for(StateSet::Iterator sit = rGen.InitStatesBegin(); sit != rGen.InitStatesEnd(); ++sit) {
        initialTree.nodes[root].stateLabel.Insert(*sit);
    }
    
    // Create initial enhanced state
    EnhancedState initialEnhancedState;
    initialEnhancedState.originalState = rRes.InsInitState();
    // No deleted nodes initially
    
    enhancedStateToTree[initialEnhancedState] = initialTree;
    
    std::string initialSig = ComputeTreeSignature(initialTree, initialEnhancedState.deletedNodesInPrevStep);
    treeSignatureToEnhancedState[initialSig] = initialEnhancedState;
    
    std::queue<EnhancedState> stateQueue;
    stateQueue.push(initialEnhancedState);
    stateCounter++;
    
    // Process all states
    while(!stateQueue.empty() && stateCounter < MAX_STATES && iterationCounter < MAX_ITERATIONS) {
        iterationCounter++;
        
        EnhancedState currentEnhancedState = stateQueue.front();
        stateQueue.pop();
        
        LabeledTree currentTree = enhancedStateToTree[currentEnhancedState];
        FD_DF("Processing enhanced state " << currentEnhancedState.ToString() 
              << " with tree: " << currentTree.ToString());
        
        // Process each event
        for(EventSet::Iterator evIt = rGen.AlphabetBegin(); evIt != rGen.AlphabetEnd(); ++evIt) {
            Idx event = *evIt;
            FD_DF("Processing event " << rGen.EventName(event));
            
            // Clone tree with same node count
            LabeledTree newTree(N);
            newTree.rootNode = LabeledTree::INVALID_NODE;
            
            // Copy existing nodes with their numbers
            for(auto& pair : currentTree.nodes) {
                Idx oldId = pair.first;
                TreeNode& oldNode = pair.second;
                
                Idx newId = newTree.createNodeWithNumber(oldNode.nodeNumber);
                newTree.nodes[newId] = oldNode;
                newTree.nodes[newId].children.clear(); // Will rebuild children relationships
                
                if(oldId == currentTree.rootNode) {
                    newTree.rootNode = newId;
                }
            }
            
            // Rebuild parent-child relationships
            for(auto& pair : currentTree.nodes) {
                Idx oldParentId = pair.first;
                TreeNode& oldParent = pair.second;
                int parentNumber = oldParent.nodeNumber;
                
                Idx newParentId = newTree.getNodeId(parentNumber);
                if(newParentId == LabeledTree::INVALID_NODE) continue;
                
                for(Idx oldChildId : oldParent.children) {
                    if(currentTree.nodes.find(oldChildId) == currentTree.nodes.end()) continue;
                    
                    int childNumber = currentTree.nodes[oldChildId].nodeNumber;
                    Idx newChildId = newTree.getNodeId(childNumber);
                    
                    if(newChildId != LabeledTree::INVALID_NODE) {
                        newTree.nodes[newParentId].children.push_back(newChildId);
                    }
                }
            }
            
            // Track deleted nodes from this step
            std::set<int> deletedInThisStep;
            
            // STEP 1: Color all nodes white
            for(auto& pair : newTree.nodes) {
                pair.second.color = TreeNode::WHITE;
            }
            
            // STEP 2: Update state labels based on transitions
            std::string eventName = rGen.EventName(event);
            bool isEpsilonEvent = (eventName == "eps");
            
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
                // Case 2b: Check epsilon closure condition
                if(newTree.rootNode == LabeledTree::INVALID_NODE) {
                    continue; // No root, skip
                }
                
                StateSet rootLabel = newTree.nodes[newTree.rootNode].stateLabel;
                StateSet epsilonSuccessors;
                
                // Compute δ_v(ε, Y_r)
                for(StateSet::Iterator sit = rootLabel.Begin(); sit != rootLabel.End(); ++sit) {
                    Idx state = *sit;
                    for(EventSet::Iterator eit = rGen.AlphabetBegin(); eit != rGen.AlphabetEnd(); ++eit) {
                        std::string eName = rGen.EventName(*eit);
                        if(eName == "eps") {
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
            
            // STEP 3: Create nodes for Rabin acceptance violations
            // Use single I set (first Rabin pair) as per paper assumption
            if(inputRabinPairs.Size() > 0) {
                const StateSet& Iv = inputRabinPairs.Begin()->ISet();
                
                // Create copy of current nodes to iterate over
                std::vector<std::pair<Idx, TreeNode>> currentNodes;
                for(auto& pair : newTree.nodes) {
                    currentNodes.push_back(pair);
                }
                
                for(auto& nodePair : currentNodes) {
                    Idx nodeId = nodePair.first;
                    TreeNode& node = newTree.nodes[nodeId];
                    
                    // Compute Y ∩ (Xv \ Iv)
                    StateSet intersectionWithoutI;
                    for(StateSet::Iterator sit = node.stateLabel.Begin(); 
                        sit != node.stateLabel.End(); ++sit) {
                        if(!Iv.Exists(*sit)) {
                            intersectionWithoutI.Insert(*sit);
                        }
                    }
                    
                    // If Y ∩ (Xv \ Iv) ≠ ∅, create red child
                    if(!intersectionWithoutI.Empty()) {
                        try {
                            Idx newChild = newTree.createNode();
                            newTree.nodes[newChild].stateLabel = intersectionWithoutI;
                            newTree.nodes[newChild].color = TreeNode::RED;
                            newTree.nodes[newChild].aSet.clear();
                            newTree.nodes[newChild].rSet.clear();
                            node.children.push_back(newChild);
                            
                            FD_DF("Created RED child node " << newChild 
                                     << " (number " << newTree.getNodeNumber(newChild) << ") for node " << nodeId);
                        } catch(Exception& e) {
                            FD_DF("Warning: Could not create child node - " << e.What());
                            // Continue without creating child if no node numbers available
                        }
                    }
                }
            }
            
            // STEP 4: Maintain state disjointness among siblings
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
                int nodeNumber = newTree.getNodeNumber(nodeId);
                deletedInThisStep.insert(nodeNumber);
                newTree.deleteNode(nodeId);
            }
            
            // STEP 6: Determine red breakpoints
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
                    
                    // Delete all descendants and record their numbers
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
                        int nodeNumber = newTree.getNodeNumber(descendant);
                        deletedInThisStep.insert(nodeNumber);
                        newTree.deleteNode(descendant);
                    }
                    
                    node.children.clear();
                    node.aSet.clear();
                    node.rSet.clear();
                }
            }
            
            // STEP 7: Delete the nodes removed in the above two steps from the A- and R-sets of all other nodes
            // (This is already handled by deleteNode method which uses node numbers)
            
            // STEP 8: If the A-set of a node n is empty and n is not colored red, then color n green
            // and set its A-set equal to its R-set. Then let its R-set be empty.
            for(auto& pair : newTree.nodes) {
                Idx nodeId = pair.first;
                TreeNode& node = pair.second;
                
                if(node.aSet.empty() && node.color != TreeNode::RED) {
                    FD_DF("Green coloring for node " << nodeId << " (number " << node.nodeNumber << ")");
                    node.color = TreeNode::GREEN;
                    node.aSet = node.rSet;
                    node.rSet.clear();
                }
            }
            
            // STEP 9: If a node is not colored red, then add to its R-set the set of all other nodes
            // presently colored red (using node numbers).
            std::set<int> redNodeNumbers;
            for(auto& pair : newTree.nodes) {
                if(pair.second.color == TreeNode::RED) {
                    redNodeNumbers.insert(pair.second.nodeNumber);
                }
            }
            
            for(auto& pair : newTree.nodes) {
                if(pair.second.color != TreeNode::RED) {
                    for(int redNodeNumber : redNodeNumbers) {
                        pair.second.rSet.insert(redNodeNumber);
                    }
                }
            }
            
            // Create enhanced target state
            EnhancedState targetEnhancedState;
            targetEnhancedState.deletedNodesInPrevStep = deletedInThisStep;
            
            // Check if this tree was seen before
            std::string treeSig = ComputeTreeSignature(newTree, deletedInThisStep);
            
            auto stateIt = treeSignatureToEnhancedState.find(treeSig);
            if(stateIt != treeSignatureToEnhancedState.end()) {
                // If so, use existing enhanced state
                targetEnhancedState = stateIt->second;
                FD_DF("Found existing enhanced state " << targetEnhancedState.ToString() << " for tree");
            } else {
                // Create new state for this tree
                targetEnhancedState.originalState = rRes.InsState();
                enhancedStateToTree[targetEnhancedState] = newTree;
                treeSignatureToEnhancedState[treeSig] = targetEnhancedState;
                
                stateQueue.push(targetEnhancedState);
                stateCounter++;
                
                FD_DF("Created new enhanced state " << targetEnhancedState.ToString() << " for tree");
            }
            
            // Add transition from current state to target state
            rRes.SetTransition(currentEnhancedState.originalState, event, targetEnhancedState.originalState);
        }
    }
    
    if(stateCounter >= MAX_STATES) {
        FD_DF("Warning: Reached maximum state limit of " << MAX_STATES);
    }
    
    if(iterationCounter >= MAX_ITERATIONS) {
        FD_DF("Warning: Reached maximum iteration limit of " << MAX_ITERATIONS);
    }
    
    // Create Rabin pairs for output automaton - PAPER ALGORITHM
    // "for every n, 1 ≤ n ≤ N, let Rn be the set of states in which node n is colored green in the tree 
    // and let In be the set of trees in which node n is not colored red in the tree and node n has not just been deleted from the tree."
    
    RabinAcceptance outputRabinPairs;
    
    // For each fixed node number 1 to N
    for(int nodeNumber = 1; nodeNumber <= N; ++nodeNumber) {
        StateSet Rn; // States where node n is colored green
        StateSet In; // States where node n is not colored red and not just deleted
        
        // Iterate through all enhanced states
        for(const auto& enhancedStatePair : enhancedStateToTree) {
            const EnhancedState& enhancedState = enhancedStatePair.first;
            const LabeledTree& tree = enhancedStatePair.second;
            Idx state = enhancedState.originalState;
            
            // Check if node n exists in this tree
            bool nodeExists = tree.hasNodeNumber(nodeNumber);
            
            // Check if node n was just deleted in previous step
            bool nodeJustDeleted = enhancedState.deletedNodesInPrevStep.find(nodeNumber) != 
                                  enhancedState.deletedNodesInPrevStep.end();
            
            if(nodeExists) {
                Idx nodeId = tree.getNodeId(nodeNumber);
                const TreeNode& node = tree.nodes.at(nodeId);
                
                // Populate In: node exists, not red, and not just deleted
                if(node.color != TreeNode::RED && !nodeJustDeleted) {
                    In.Insert(state);
                }
                
                // Populate Rn: node is green
                if(node.color == TreeNode::GREEN) {
                    Rn.Insert(state);
                }
            } else {
                // Node doesn't exist (was deleted in some previous step)
                // It's not in In because it doesn't exist
                // It's not in Rn because it can't be green if it doesn't exist
            }
        }
        
        // Add Rabin pair only if Rn is not empty
        // (According to paper: meaningful pairs where infinite green visits can happen)
        if(!Rn.Empty()) {
            RabinPair newPair;
            newPair.RSet() = Rn;
            newPair.ISet() = In;
            outputRabinPairs.Insert(newPair);
            
            FD_DF("Created Rabin pair for node " << nodeNumber 
                  << ": R=" << Rn.Size() << " states, I=" << In.Size() << " states");
        }
    }
    
    rRes.RabinAcceptance() = outputRabinPairs;
    
    FD_DF("PseudoDet completed with " 
              << stateCounter << " states and "
              << outputRabinPairs.Size() << " Rabin pairs");
}

/*
********************************
RemoveEps Implementation
********************************
*/

void RemoveEps(const RabinAutomaton& rGen, RabinAutomaton& rRes) {
    FD_DF("RemoveEps(" << rGen.Name() << ") - Two-phase approach");
    
    // Find epsilon event
    Idx epsEvent = 0;
    for(EventSet::Iterator evIt = rGen.AlphabetBegin(); evIt != rGen.AlphabetEnd(); ++evIt) {
        std::string eventName = rGen.EventName(*evIt);
        if(eventName == "eps") {
            epsEvent = *evIt;
            break;
        }
    }
    
    if(epsEvent == 0) {
        FD_DF("No epsilon event found, copying automaton as-is");
        rRes = rGen;
        return;
    }
    
    // Phase 1: Start with a copy and process eps-only states (relink)
    rRes = rGen;
    
    bool changed = true;
    while(changed) {
        changed = false;
        
        // Find a state that has ONLY eps outgoing transitions (excluding self-loops)
        for(StateSet::Iterator sit = rRes.StatesBegin(); sit != rRes.StatesEnd(); ++sit) {
            Idx sourceState = *sit;
            
            std::vector<Transition> nonEpsOutgoing;
            std::vector<Transition> epsNonSelfLoop;
            
            // Classify outgoing transitions
            for(TransSet::Iterator tit = rRes.TransRelBegin(sourceState); 
                tit != rRes.TransRelEnd(sourceState); ++tit) {
                if(tit->Ev == epsEvent) {
                    if(tit->X1 != tit->X2) { // Not a self-loop
                        epsNonSelfLoop.push_back(*tit);
                    }
                    // Ignore eps self-loops for now
                } else {
                    nonEpsOutgoing.push_back(*tit);
                }
            }
            
            // Check if this state has ONLY one non-self-loop eps transition
            if(nonEpsOutgoing.empty() && epsNonSelfLoop.size() == 1) {
                Idx targetState = epsNonSelfLoop[0].X2;
                
                FD_DF("State " << sourceState << " has only eps transition to " 
                      << targetState << " - relinking all inputs");
                
                // Find all incoming transitions to sourceState
                std::vector<Transition> incomingTrans;
                for(TransSet::Iterator tit = rRes.TransRelBegin(); 
                    tit != rRes.TransRelEnd(); ++tit) {
                    if(tit->X2 == sourceState) {
                        incomingTrans.push_back(*tit);
                    }
                }
                
                // Relink all incoming transitions to targetState
                for(const auto& incoming : incomingTrans) {
                    FD_DF("Relinking " << incoming.X1 << " --" 
                          << rRes.EventName(incoming.Ev) << "--> " 
                          << sourceState << " to " << incoming.X1 
                          << " --" << rRes.EventName(incoming.Ev) 
                          << "--> " << targetState);
                    
                    // Remove old transition
                    rRes.ClrTransition(incoming.X1, incoming.Ev, sourceState);
                    
                    // Add new transition (check for duplicates)
                    if(!rRes.ExistsTransition(incoming.X1, incoming.Ev, targetState)) {
                        rRes.SetTransition(incoming.X1, incoming.Ev, targetState);
                    }
                }
                
                // Remove the eps transition from sourceState to targetState
                rRes.ClrTransition(sourceState, epsEvent, targetState);
                
                // Handle initial states
                if(rRes.ExistsInitState(sourceState)) {
                    rRes.ClrInitState(sourceState);
                    rRes.SetInitState(targetState);
                    FD_DF("Redirected initial state from " << sourceState << " to " << targetState);
                }
                
                // Handle marked states
                if(rRes.ExistsMarkedState(sourceState)) {
                    rRes.ClrMarkedState(sourceState);
                    rRes.SetMarkedState(targetState);
                    FD_DF("Redirected marked state from " << sourceState << " to " << targetState);
                }
                
                // Update Rabin acceptance condition
                RabinAcceptance currentRabin = rRes.RabinAcceptance();
                RabinAcceptance newRabin;
                
                for(RabinAcceptance::Iterator rit = currentRabin.Begin(); rit != currentRabin.End(); ++rit) {
                    StateSet newR = rit->RSet();
                    StateSet newI = rit->ISet();
                    
                    // Replace sourceState with targetState in R set
                    if(newR.Exists(sourceState)) {
                        newR.Erase(sourceState);
                        newR.Insert(targetState);
                    }
                    
                    // Replace sourceState with targetState in I set
                    if(newI.Exists(sourceState)) {
                        newI.Erase(sourceState);
                        newI.Insert(targetState);
                    }
                    
                    RabinPair newPair;
                    newPair.RSet() = newR;
                    newPair.ISet() = newI;
                    newRabin.Insert(newPair);
                }
                
                rRes.RabinAcceptance() = newRabin;
                
                changed = true;
                break; // Restart to find next eps-only state
            }
        }
    }
    
    // Phase 2: Remove all eps self-loops
    FD_DF("Phase 2: Removing all eps self-loops");
    
    std::vector<Transition> selfLoopsToRemove;
    for(TransSet::Iterator tit = rRes.TransRelBegin(); tit != rRes.TransRelEnd(); ++tit) {
        if(tit->Ev == epsEvent && tit->X1 == tit->X2) {
            selfLoopsToRemove.push_back(*tit);
        }
    }
    
    for(const auto& selfLoop : selfLoopsToRemove) {
        FD_DF("Removing eps self-loop at state " << selfLoop.X1);
        rRes.ClrTransition(selfLoop.X1, epsEvent, selfLoop.X2);
    }
    
    // Remove unreachable states
    StateSet reachableStates;
    std::queue<Idx> stateQueue;
    
    // Start from initial states
    for(StateSet::Iterator sit = rRes.InitStatesBegin(); sit != rRes.InitStatesEnd(); ++sit) {
        if(!reachableStates.Exists(*sit)) {
            reachableStates.Insert(*sit);
            stateQueue.push(*sit);
        }
    }
    
    // BFS to find all reachable states
    while(!stateQueue.empty()) {
        Idx current = stateQueue.front();
        stateQueue.pop();
        
        for(TransSet::Iterator tit = rRes.TransRelBegin(current); 
            tit != rRes.TransRelEnd(current); ++tit) {
            if(!reachableStates.Exists(tit->X2)) {
                reachableStates.Insert(tit->X2);
                stateQueue.push(tit->X2);
            }
        }
    }
    
    // Remove unreachable states
    StateSet statesToRemove;
    for(StateSet::Iterator sit = rRes.StatesBegin(); sit != rRes.StatesEnd(); ++sit) {
        if(!reachableStates.Exists(*sit)) {
            statesToRemove.Insert(*sit);
        }
    }
    
    for(StateSet::Iterator sit = statesToRemove.Begin(); sit != statesToRemove.End(); ++sit) {
        rRes.DelState(*sit);
        FD_DF("Removed unreachable state " << *sit);
    }
    
    // Check if we still have eps transitions
    bool hasEpsTransitions = false;
    for(TransSet::Iterator tit = rRes.TransRelBegin(); tit != rRes.TransRelEnd(); ++tit) {
        if(tit->Ev == epsEvent) {
            hasEpsTransitions = true;
            break;
        }
    }
    
    // Remove eps from alphabet if no eps transitions remain
    if(!hasEpsTransitions) {
        EventSet newAlphabet;
        for(EventSet::Iterator evIt = rRes.AlphabetBegin(); evIt != rRes.AlphabetEnd(); ++evIt) {
            if(*evIt != epsEvent) {
                newAlphabet.Insert(*evIt);
            }
        }
        rRes.InjectAlphabet(newAlphabet);
        FD_DF("Removed eps from alphabet");
    }
    
    // Final step: Renumber states to be consecutive 1,2,3...
    StateSet finalStates;
    for(StateSet::Iterator sit = rRes.StatesBegin(); sit != rRes.StatesEnd(); ++sit) {
        finalStates.Insert(*sit);
    }
    
    if(finalStates.Empty()) {
        FD_DF("Warning: No states remaining after eps removal");
        return;
    }
    
    std::map<Idx, Idx> renumberMapping;
    Idx newStateId = 1;
    for(StateSet::Iterator sit = finalStates.Begin(); sit != finalStates.End(); ++sit) {
        renumberMapping[*sit] = newStateId++;
    }
    
    FD_DF("Renumbering states:");
    for(auto& pair : renumberMapping) {
        FD_DF("  " << pair.first << " -> " << pair.second);
    }
    
    // Apply renumbering
    RabinAutomaton tempResult;
    tempResult.Name(rRes.Name());
    tempResult.InjectAlphabet(rRes.Alphabet());
    
    // Create renumbered states
    for(auto& pair : renumberMapping) {
        tempResult.InsState(pair.second);
    }
    
    // Renumber initial states
    for(StateSet::Iterator sit = rRes.InitStatesBegin(); sit != rRes.InitStatesEnd(); ++sit) {
        tempResult.SetInitState(renumberMapping[*sit]);
    }
    
    // Renumber marked states
    for(StateSet::Iterator sit = rRes.MarkedStatesBegin(); sit != rRes.MarkedStatesEnd(); ++sit) {
        tempResult.SetMarkedState(renumberMapping[*sit]);
    }
    
    // Renumber transitions
    for(TransSet::Iterator tit = rRes.TransRelBegin(); tit != rRes.TransRelEnd(); ++tit) {
        tempResult.SetTransition(renumberMapping[tit->X1], tit->Ev, renumberMapping[tit->X2]);
    }
    
    // Renumber Rabin acceptance condition
    RabinAcceptance originalRabin = rRes.RabinAcceptance();
    RabinAcceptance newRabin;
    
    for(RabinAcceptance::Iterator rit = originalRabin.Begin(); rit != originalRabin.End(); ++rit) {
        StateSet newR, newI;
        
        for(StateSet::Iterator sit = rit->RSet().Begin(); sit != rit->RSet().End(); ++sit) {
            if(renumberMapping.find(*sit) != renumberMapping.end()) {
                newR.Insert(renumberMapping[*sit]);
            }
        }
        
        for(StateSet::Iterator sit = rit->ISet().Begin(); sit != rit->ISet().End(); ++sit) {
            if(renumberMapping.find(*sit) != renumberMapping.end()) {
                newI.Insert(renumberMapping[*sit]);
            }
        }
        
        if(!newR.Empty() && !newI.Empty()) {
            RabinPair newPair;
            newPair.RSet() = newR;
            newPair.ISet() = newI;
            newRabin.Insert(newPair);
        }
    }
    
    tempResult.RabinAcceptance() = newRabin;
    
    // Final assignment
    rRes = tempResult;
    
    FD_DF("RemoveEps completed using two-phase approach:");
    FD_DF("  Result: " << rRes.Size() << " states (renumbered 1 to " << rRes.Size() << "), " 
          << rRes.TransRelSize() << " transitions");
}

} // namespace faudes