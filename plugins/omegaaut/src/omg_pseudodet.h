/** @file omg_pseudodet.h Pseudo-determinization algorithm for Rabin automata
 *   
 * @ingroup OmegaautPlugin
 */

/*
  
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2025 Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


   This module implements the pseudo-determinization algorithm that converts
   nondeterministic Rabin automata to deterministic Rabin automata.
  
   Copyrigth Changming Yang 2025
   Non-exclusive copyright is granted to Thomas Moor

*/


 #ifndef FAUDES_OMG_PSEUDODET_H
 #define FAUDES_OMG_PSEUDODET_H
 
 #include "libfaudes.h"
 #include "omg_rabinacc.h"
 #include <cmath>
 #include <vector>
 #include <sstream>
 #include <map>
 #include <queue>
 #include <functional>
 #include <stack>
 #include <set>
 
 namespace faudes {

 /**
  * Enhanced state structure that includes deleted node information
  */
 class FAUDES_API EnhancedState {
 public:
     Idx originalState;                    ///< Original state ID
     std::set<int> deletedNodesInPrevStep; ///< Node numbers deleted in previous step
     
     /// Default constructor
     EnhancedState();
     
     /// Comparison operator
     bool operator<(const EnhancedState& other) const;
     
     /// Equality operator
     bool operator==(const EnhancedState& other) const;
     
     /// Debug string representation
     std::string ToString() const;
 };
 
 /**
  * Tree node structure for labeled tree used in pseudo-determinization
  */
 class FAUDES_API TreeNode {
 public:
     StateSet stateLabel;     ///< S: state label
     std::set<int> aSet;      ///< A-set (using node numbers 1..N)
     std::set<int> rSet;      ///< R-set (using node numbers 1..N)
     std::vector<Idx> children; ///< child nodes
     int nodeNumber;          ///< Fixed node number (1 to N)
     
     /// Node color for Rabin acceptance condition tracking
     enum Color { WHITE, RED, GREEN } color;
     
     /// Default constructor
     TreeNode();
     
     /// Constructor with node number
     TreeNode(int number);
     
     /// Comparison operator for ordering
     bool operator<(const TreeNode& other) const;
     
     /// Equality operator
     bool operator==(const TreeNode& other) const;
     
     /// Debug string representation
     std::string ToString() const;
 };
 
 /**
  * Labeled tree class for pseudo-determinization algorithm (Fixed N nodes)
  *
  * @ingroup OmgPlugin
  */
 class FAUDES_API LabeledTree {
 public:
     std::map<Idx, TreeNode> nodes;       ///< Tree nodes (ID -> node)
     std::map<int, Idx> numberToId;       ///< Node number -> ID mapping
     std::map<Idx, int> idToNumber;       ///< ID -> node number mapping
     std::set<int> deletedNodeNumbers;    ///< Node numbers deleted in this step
     Idx rootNode;                        ///< Root node ID
     Idx nextNodeId;                      ///< Next available node ID
     int maxNodeCount;                    ///< Maximum number of nodes (N)
     std::vector<bool> nodeNumberUsed;    ///< Track which node numbers are used
     
     static const Idx INVALID_NODE = 0;   ///< Invalid node constant
     
     /// Default constructor
     LabeledTree();
     
     /// Constructor with fixed node count
     LabeledTree(int N);
     
     /// Create a new node with specific number, return its ID
     Idx createNodeWithNumber(int nodeNumber);
     
     /// Create a new node with auto-assigned number, return its ID
     Idx createNode();
     
     /// Delete a node and clean up references
     void deleteNode(Idx nodeId);
     
     /// Get node number from ID
     int getNodeNumber(Idx nodeId) const;
     
     /// Get node ID from number
     Idx getNodeId(int nodeNumber) const;
     
     /// Check if node number exists in tree
     bool hasNodeNumber(int nodeNumber) const;
     
     /// Debug string representation
     std::string ToString() const;
     
     /// Comparison operator for ordering
     bool operator<(const LabeledTree& other) const;
     
     /// Equality operator
     bool operator==(const LabeledTree& other) const;
 };
 
 /**
  * Pseudo-determinization algorithm for Rabin automata (Paper Algorithm)
  *
  * This function converts a nondeterministic Rabin automaton to an equivalent
  * deterministic Rabin automaton using the pseudo-determinization algorithm
  * from the paper, with fixed node numbering and enhanced state tracking.
  *
  * @param rGen
  *   Input nondeterministic Rabin automaton
  * @return
  *   Equivalent deterministic Rabin automaton
  *
  * @exception Exception
  *   - Input automaton has no initial states (id 201)
  *   - Algorithm complexity limits exceeded (id 202)
  *
  * @ingroup OmgPlugin
  */
 FAUDES_API void PseudoDet(const RabinAutomaton& rGen, RabinAutomaton& rRes);
 
 /**
  * Compute tree signature for state equivalence checking (Enhanced)
  *
  * @param tree
  *   Input labeled tree
  * @param deletedNodes
  *   Set of deleted node numbers
  * @return
  *   String signature representing the tree structure and deleted nodes
  *
  * @ingroup OmgPlugin
  */
 FAUDES_API std::string ComputeTreeSignature(const LabeledTree& tree, const std::set<int>& deletedNodes);

/**
 * Remove epsilon transitions from a Rabin automaton
 *
 * This function removes epsilon transitions from a Rabin automaton by:
 * 1. Removing epsilon self-loops
 * 2. Merging epsilon transitions with other transitions
 *
 * @param rGen
 *   Input Rabin automaton with epsilon transitions
 * @param rRes
 *   Output Rabin automaton without epsilon transitions
 *
 * @ingroup OmgPlugin
 */
FAUDES_API void RemoveEps(const RabinAutomaton& rGen, RabinAutomaton& rRes);
 
 } // namespace faudes
 
 #endif // FAUDES_OMG_PSEUDODET_H