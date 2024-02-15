// obsolete --- moved to corefaudes cfl_bisimulation* --- file kept for reference 

/** @file op_partition.cpp 

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

#include "op_partition.h"

using namespace std;

namespace faudes {

Partition::Partition()
{
    OP_DF("Partition::Partition: entering function");                      
    nonSplitting=false;
    OP_DF("Partition::Partition: leaving function");    
}

void Partition::writeInfoMap(Idx event) const
{
    OP_DF("Partition::writeInfoMap: entering function");
    cout << "Writing info-map for event " << event << endl;
    std::map<Idx,map<Idx,Idx> >::const_iterator pMap = infoMap.find(event);

    if(pMap!=infoMap.end())
    {
        std::map<Idx,Idx>::const_iterator mIt;
        std::map<Idx,Idx>::const_iterator mItBegin = pMap->second.begin();
        std::map<Idx,Idx>::const_iterator mItEnd = pMap->second.end();

        if(mItBegin == mItEnd)
            cout << "no entries for this event" << endl; 
        for(mIt=mItBegin; mIt != mItEnd; ++mIt)
            cout << "state: " << (*mIt).first << " : occurrences: " << (*mIt).second << endl;
        cout << endl;
    }

    else
        cout << "no info-map for this event" << endl;

    OP_DF("Partition::writeInfoMap: leaving function");
}

}
