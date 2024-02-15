/** @file obs_local_observation_consistency.cpp Supervisor Reduction */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Klaus Schmidt

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
   

#include "obs_local_observation_consistency.h"


namespace faudes {




/*
***************************************************************************************
***************************************************************************************
 Implementation
***************************************************************************************
***************************************************************************************
*/


// SupReduce(rPlantGen, rSupGen, rReducedSup)
bool LocalObservationConsistency(const System& rPlantGen, const System& rSpecGen, const EventSet& rHighAlph, const EventSet& rObsAlph) {
  FD_DF("LocalObservationConsistency...");

  // CONSISTENCY CHECK:

  // plant and spec must be deterministic
  bool plant_det = rPlantGen.IsDeterministic();
  bool sup_det = rSpecGen.IsDeterministic();

  if ((plant_det == false) && (sup_det == true)) {
    std::stringstream errstr;
    errstr << "Plant generator must be deterministic, " << "but is nondeterministic";
    throw Exception("LocalObservationConsistency", errstr.str(), 201);
  }
  else if ((plant_det == true) && (sup_det == false)) {
    std::stringstream errstr;
    errstr << "Specification generator must be deterministic, " << "but is nondeterministic";
    throw Exception("LocalObservationConsistency", errstr.str(), 203);
  }
  else if ((plant_det == false) && (sup_det == false)) {
    std::stringstream errstr;
    errstr << "Plant and specification generator must be deterministic, "
                    << "but both are nondeterministic";
    throw Exception("LocalObservationConsistency", errstr.str(), 204);
  }
  // Compute the parallel composition of abstraction and specification
  System parallelGen;
  aParallel(rPlantGen,rSpecGen,parallelGen);
  System highGen;
  if(!IsObs(parallelGen,rHighAlph) )
     return false;
  
  aProject(parallelGen,rHighAlph,highGen);
  //highGen.GraphWrite("data/highGen.png");
  aProjectNonDet(highGen,rHighAlph*rObsAlph);
  // make projected generator deterministic and record the state tuples
  std::vector<StateSet> powerStates;
  std::vector<Idx> detStates;
  System detGen;
  Deterministic(highGen,powerStates,detStates,detGen);
  //detGen.GraphWrite("data/detGen.png");
  // Go through all power states and check the feasible controllable events
  std::vector<StateSet>::const_iterator pIt, pEndIt;
  pIt = powerStates.begin();
  pEndIt = powerStates.end();
  for(; pIt != pEndIt; pIt++){// go through all power states
      StateSet::Iterator stIt;
      // Determine the controllable events that should be present in each state
      stIt = pIt->Begin();
      //std::cout << "State: " << *stIt;
      EventSet controllableEvents;
      TransSet::Iterator tIt = highGen.TransRelBegin(*stIt);
      for( ; tIt != highGen.TransRelEnd(*stIt); tIt++){
	  //std::cout << " event " << highGen.EventName(tIt->Ev);
	  if(highGen.Controllable(tIt->Ev) )
	     controllableEvents.Insert(tIt->Ev);
      }
      stIt++;
      EventSet otherControllableEvents;
      for(; stIt != pIt->End(); stIt++ ){// go through all remaining states and compare
	  //std::cout << "State: " << *stIt;
	  otherControllableEvents.Clear();
	  tIt = highGen.TransRelBegin(*stIt);
	  for( ; tIt != highGen.TransRelEnd(*stIt); tIt++){
	     //std::cout << " event " << highGen.EventName(tIt->Ev);
	    if(highGen.Controllable(tIt->Ev) )
	      otherControllableEvents.Insert(tIt->Ev);
	  }
// // 	  //std::cout << std::endl;
	  //controllableEvents.Write();
	  //otherControllableEvents.Write();
	  if(controllableEvents != otherControllableEvents)
	    return false;
      }
  }
 
  return true;
}

} // name space 
