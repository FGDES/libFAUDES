/** @file pev_sparallel.h Sshaped parallel composition */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2023 Yiheng Tang
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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef FAUDES_PEV_OPERATIONS
#define FAUDES_PEV_OPERATIONS


#include "corefaudes.h"
#include "pev_pgenerator.h"

namespace faudes {

// generate merged event name
std::string MergeEventNames (const std::set<std::string>& rNames);

// perform SFC Parallel composition
void SUParallel(
        const pGenerator& rPGen1, const pGenerator& rPGen2,
        std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap,
        EventSet& rMerge,
        const EventSet& rPrivate,
        EventPriorities& rPrios,
        pGenerator& rPRes);

// perform SFC Parallel composition
void SUParallel(
        const pGenerator& rPGen1, const pGenerator& rPGen2,
        EventSet& rMerge,
        const EventSet& rPrivate,
        EventPriorities& rPrios,
        pGenerator& rPRes);

// the special fairness merge for SFC
 void UParallel_MergeFairness(const pGenerator& rPGen1, const pGenerator& rPGen2, const Generator& rGen12, const EventSet& rMerge, FairnessConstraints& rFairRes);

 
}// namespace
#endif //.H
