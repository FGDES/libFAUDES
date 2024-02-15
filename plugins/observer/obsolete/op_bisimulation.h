// obsolete --- moved to corefaudes cfl_bisimulation* --- file kept for reference 


/** @file op_bisimulation.h 

Methods to compute bisimulations on dynamic systems (represented
by a finite automaton). 
The relevant algorithms are described in
J.-C. Fernandez, "An implementation of an efficient algorithm for
bisimulation equivalence", Science of Computer Programming, vol. 13,
pp. 219-236, 1990.
The class bisimulation supports these methods.
*/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include "corefaudes.h"
#include "op_partition.h"
#include "op_debug.h"
#include <vector>
#include <map>
#include <string>
#include <sstream>

#ifndef FAUDES_OP_BISIMULATION_H
#define FAUDES_OP_BISIMULATION_H



namespace faudes {

/**
* Computation of a bisimulation over a given generator. 
* This funcion reates an instance of the class Bisimulation and starts the computation
* of the coarsest quasi-congruence on the given generator by calling the function Bisimulation::partition. 
* A generator representing the result of the computation is generated.
*
* @param rGenOrig
*   Original generator
* @param rMapStateToPartition
*   Maps each state to its equivalence class
* @param rGenPart
*   Quotient automaton representing the result of the computation. Each state corresponds to an 
*   equivalence class
* @param rNewPartitions
*   Holds the indices of all equivalence classes
*
* @ingroup ObserverPlugin
*/
 void calcBisimulation(Generator& rGenOrig, std::map<Idx,Idx>& rMapStateToPartition, Generator& rGenPart, std::vector<Idx>& rNewPartitions);

/**
* Computation of a bisimulation over a given generator. 
* This funcion reates an instance of the class Bisimulation and starts the computation
* of the coarsest quasi-congruence on the given generator by calling the function Bisimulation::partition. 
* See J.-C. Fernandez, “An implementation of an efficient algorithm for bisimulation equivalence,” Science of Computer Programming, vol. 13,
* pp. 219-236, 1990 for further details.
*
* @param rGenOrig
*   Original generator
* @param rMapStateToPartition
*   Maps each state to its equivalence class
* @param rNewPartitions
*   Holds the indices of all equivalence classes
*
* @ingroup ObserverPlugin
*/
void calcBisimulation(Generator& rGenOrig, std::map<Idx,Idx>& rMapStateToPartition, std::vector<Idx>& rNewPartitions);

/**
* This class implements the algorithms needed for the computation of 
* the coarsest quasi-congruence (=Bisimulation) of a given generator.
*/

class Bisimulation {

    public:

        /**
        * Contructor
        *
        * @param g
        *   Original generator
        */
        Bisimulation(Generator& g);


        /**
        * Write W-tree to console
        */
        void writeW(void);

        /**
        * Administration of the various steps of the computation of the coarsest quasi-congruence. An output 
        * generator that represents the resulting quotient automaton is also provided. 
        *
        * @param rMapStateToPartition
        *   Maps each state to its equivalence class
        * @param rGenPart
        *   Generator representing the result of the computation. Each state corresponds to an
        *   euivalence class
        * @param rNewPartitions
        *   Holds the indices of all equivalence classes
        */
        void partition(std::map<Idx,Idx>& rMapStateToPartition, Generator& rGenPart, std::vector<Idx>& rNewPartitions);


        /**
        * Administration of the various steps of the computation of the coarsest quasi-congruence
        *
        * @param rMapStateToPartition
        *   Maps each state to its equivalence class
        * @param rNewPartitions
        *   Holds the indices of all equivalence classes
        */
        void partition(std::map<Idx,Idx>& rMapStateToPartition, std::vector<Idx>& rNewPartitions);

        /**
        * Write the current set of equivalence classes to console
        */
        void writeRo(void);

    private:

        /**
        *  Original Automaton
        */
        Generator* gen;

        /**
        * Compute info-maps for two cosets P1 and P2. The current partition is 
        * stable with respect to their parent coset P.
        *
        * @param node
        *   Coset whose states are examined
        * @param pSmallerPart
        *   Child coset P1 of P with smaller number of states
        * @param pLargerPart
        *   Child coset P2 of P with larger number of states
        * @param eIt
        *   Event
        */
        void computeInfoMaps(Partition& node, Partition* pSmallerPart, Partition* pLargerPart, EventSet::Iterator eIt);

        /**
        * Compute info-maps for coset B
        *
        * @param B
        *   Coset for which the info-map is computed
        * @param Bstates
        *   Coset whose states are examined
        * @param eIt
        *   iterator to an event in a EventSet
        * @param tb
        *   StateSet containing all the states that have a *eIt-transition into the coset B
        */
        void computeInfoMap(Partition& B, Partition& Bstates, EventSet::Iterator eIt, StateSet& tb);

        /**
        * Check if a state has a eIt-transition into a coset denoted as node
        *
        * @param state
        *   State to be examined
        * @param node
        *   Coset
        * @param eIt
        *   Event
        */
        bool stateLeadsToPartition(Idx state, Partition& node, EventSet::Iterator eIt);

        /**
        * W-tree. Contains all cosets ever created
        */
        std::map<Idx, Partition> W;
        
        /**
        * Counter to assign unique indices to the cosets
        */
        Idx index;

        /**
        * Contains the cosets of the current partition
        */
        std::vector<Partition*> ro;

        /**
        * TransSet of original generator sorted by EvX2X1
        */
        TransSetEvX2X1 tset_evx2x1;


        /**
        * Refine current partition with respect to partition B
        *
        * @param B
        *   Coset
        */
        void partitionClass(Partition& B);

        /**
        * Refine current partition with respect to partition B and make use of the fact that
        * the current partition is stable with respect to the parent coset of B.
        *
        * @param B
        *   Coset
        */
        void partitionSplitter (Partition& B);


        /**
        * Function needed for recursively plotting the W-tree to console. For debugging purpose
        *
        * @param node
        *   Coset
        */
        void writeNode(Partition& node);

        /** 
        * Holds the cosets that can possibly split cosets in ro
        */
        std::set<Partition*> roDividers;
};

}

#endif

