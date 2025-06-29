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
  * Tree node structure for labeled tree used in pseudo-determinization
  */
 struct TreeNode {
     StateSet stateLabel;     ///< S: state label
     std::set<Idx> aSet;      ///< A-set
     std::set<Idx> rSet;      ///< R-set
     std::vector<Idx> children; ///< child nodes
     
     /// Node color for Rabin acceptance condition tracking
     enum Color { WHITE, RED, GREEN } color;
     
     /// Default constructor
     TreeNode();
     
     /// Comparison operator for ordering
     bool operator<(const TreeNode& other) const;
     
     /// Equality operator
     bool operator==(const TreeNode& other) const;
     
     /// Debug string representation
     std::string ToString() const;
 };
 
 /**
  * Labeled tree class for pseudo-determinization algorithm
  *
  * @ingroup OmgPlugin
  */
 class FAUDES_API LabeledTree {
 public:
     std::map<Idx, TreeNode> nodes;  ///< Tree nodes
     Idx rootNode;                   ///< Root node ID
     Idx nextNodeId;                 ///< Next available node ID
     
     static const Idx INVALID_NODE = 0; ///< Invalid node constant
     
     /// Default constructor
     LabeledTree();
     
     /// Create a new node and return its ID
     Idx createNode();
     
     /// Delete a node and clean up references
     void deleteNode(Idx nodeId);
     
     /// Debug string representation
     std::string ToString() const;
     
     /// Comparison operator for ordering
     bool operator<(const LabeledTree& other) const;
     
     /// Equality operator
     bool operator==(const LabeledTree& other) const;
 };
 
 /**
  * Pseudo-determinization algorithm for Rabin automata
  *
  * This function converts a nondeterministic Rabin automaton to an equivalent
  * deterministic Rabin automaton using the pseudo-determinization algorithm.
  * The algorithm uses labeled trees to track the acceptance condition during
  * the determinization process.
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
  * Compute tree signature for state equivalence checking
  *
  * @param tree
  *   Input labeled tree
  * @return
  *   String signature representing the tree structure
  *
  * @ingroup OmgPlugin
  */
 FAUDES_API std::string ComputeTreeSignature(const LabeledTree& tree);
 
 } // namespace faudes
 
 #endif // FAUDES_OMG_PSEUDODET_H
