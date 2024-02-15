// obsolete --- moved to corefaudes cfl_bisimulation* --- file kept for reference 

/** @file op_partition.h 

Data structure that implements an equivalence class in the bisimulation algorithm.
The algorithm is presented in 
J.-C. Fernandez, “An implementation of an efficient algorithm for
bisimulation equivalence,” Science of Computer Programming, vol. 13,
pp. 219-236, 1990.
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
#include "op_debug.h"
#include <vector>
#include <map>

#ifndef FAUDES_OP_PARTITION_H
#define FAUDES_OP_PARTITION_H


namespace faudes {
          
/**
* This struct implements a coset (=equivalence class) as needed for the computation of the coarsest quasi-congruence on an automaton.
*/          
struct Partition {

        /** Constructor */
        Partition();

        /** Unique index */
        Idx index;

        /** Pointer to the associated StateSet */
        StateSet states;

        /** Index of the parent coset */
        Partition* pFather;

        /** Index of the brother coset */
        Partition* pBrother;

        /** Index of first child coset */
        Partition* pFirstChild;

        /** Index of second child coset */
        Partition* pSecondChild;

        /** indicates if the current partition is stable with respect to this coset */
        bool nonSplitting;

        /** Info-map. First Idx: event, second Idx: state, third Idx: number of occurences */
        std::map<Idx,std::map<Idx,Idx> > infoMap;

        /**
        * Write info-map to console
        *
        * @param event
        *    event for which the info-map shall be plotted
        */
        void writeInfoMap(Idx event) const;

        /** number of states in this coset */
        Idx numberOfStates;
};

} // name space

#endif
