/** @file con_decomposability_extension.cpp Conditionaldecomposability */

/*
 * Implementation of the Conditionally decomposable extension algorithm
 *
 * Copyright (C) 2012, 2015  Tomas Masopust
 *
 */


#include "con_decomposability_extension.h"
#include <vector>

namespace faudes {

void ConDecExtension(const Generator& gen, const EventSetVector& rAlphabets, EventSet& ek) {
	
	// the generator must be deterministic
	if (gen.IsDeterministic() == false) {
	  std::stringstream errstr;
	  errstr << "Generator must be deterministic, but is nondeterministic";
	  throw Exception("ConditionalDecomposabilityExtension", errstr.str(), 201);
	}

	// the generator must be trimed
	if (gen.IsTrim() == false) {
	  std::stringstream errstr;
	  errstr << "Generator must be trim, but is blocking";
	  throw Exception("ConditionalDecomposabilityExtension", errstr.str(), 201);
	}

	// at least two alphabets in the vector
	if (rAlphabets.Size() < 2) {
	  std::stringstream errstr;
	  errstr << "At least two alphabets must be included in the EventSetVector";
	  throw Exception("ConditionalDecomposabilityExtension", errstr.str(), 201);
	}

	Idx i;
  EventSetVector ee = rAlphabets;		// Vector of input alphabets
	EventSet unionset;			          // contains union of Ei
	EventSet shared;			            // contains union of intersections

	// Compute unionset
	for (i = 0; i < ee.Size(); i++) {
		unionset = unionset + ee.At(i);
	}

	// Compute the set of shared events
	for (i = 0; i < ee.Size(); i++) {
	  for (Idx j = 0; j < ee.Size(); j++) {
	    if (j != i) {
		    shared = shared + ( ee.At(i) * ee.At(j) );
	    }
	  }
	}

  // Alphabet of the generator must be under union Ei 
	bool ok = SetInclusion(gen.Alphabet(),unionset);
  	if (ok == false) {
	    std::stringstream errstr;
	    errstr << "Generator alphabet is not included in union of the alphabets";
	    throw Exception("ConditionalDecomposabilityExtension", errstr.str(), 100);
  	}

  // Alphabet of the generator must contain Ek
	ok = SetInclusion(ek,gen.Alphabet());
  	if (ok == false) {
	    std::stringstream errstr;
	    errstr << "Generator alphabet does not include the alphabet ek";
	    throw Exception("ConditionalDecomposabilityExtension", errstr.str(), 100);
  	}

  // Ek must contain all shared events
	ok = SetInclusion(shared,ek);
  	if (ok == false) {
	    std::stringstream errstr;
	    errstr << "Ek does not include all shared events";
	    throw Exception("ConditionalDecomposabilityExtension", errstr.str(), 100);
  	}

  //call CDExtension for each pair (Ei, \cup_{j<>i} Ej)
  for (Idx i = 0; i < ee.Size(); ++i) {
    EventSetVector eev;
    eev.Append(ee.At(i));
    //union of Ej for j<>i
	  EventSet uset;
    for (Idx j = 0; j < ee.Size(); ++j) {
      if ( i == j ) continue;
		  uset = uset + ee.At(j);
	  }
    eev.Append(uset);
    CDExt(gen,eev,unionset,ek);
  }

}


void CDExt(const Generator& gen, const EventSetVector& ee, const EventSet& unionset, EventSet& ek) {
	// Compute tilde G
	Generator tildeG;
	tildeG = ComputeTildeG(unionset,ee,ek,gen);
//  tildeG.GraphWrite("tildeG.png");

	// if Ek changed, set b = true and recompute tilde G
	while ( isExtendedEk(tildeG,gen,ek) ) {
	  tildeG = ComputeTildeG(unionset,ee,ek,gen);
	}
}


Generator ComputeTildeG(const EventSet& unionset, const EventSetVector& ee, const EventSet& ek, const Generator& gen) {
	// Make copies of the generator
	GeneratorVector copies;
	EventSet unionsetNew = unionset;	// unionset with new events
	EventSet::Iterator eit;

	for (Idx i = 0; i < ee.Size(); i++) {
		EventSet notInEiEk;
		notInEiEk = unionset - ( ee.At(i) + ek );
    Generator copy = gen;
		for (eit = notInEiEk.Begin(); eit != notInEiEk.End(); ++eit) {
			copy.EventRename(*eit,copy.UniqueEventName(""));
		}
		copies.Append(copy);
		unionsetNew = unionsetNew + copy.Alphabet();
	}

	Generator g = copies.At(0);
  for (Idx i = 1; i < copies.Size(); i++) {
    Parallel(g,copies.At(i),g);
  }
  Trim(g);
  return g;
}

// go through tilde G and check if all corresponding transitions are possible in G
bool isExtendedEk(const Generator& tildeGen, const Generator& rGen, EventSet& ek) {
  // shared events
  EventSet sharedalphabet = tildeGen.Alphabet() * rGen.Alphabet();
  // todo stack
	std::stack< std::pair<Idx,Idx> > todo;
	// tested transitions are stored here
  std::map< std::pair<Idx,Idx>, Idx> testedTrans;
  testedTrans.clear();
	// current pair, new pair
	std::pair<Idx,Idx> currentstates, newstates;
	// state
	Idx tmpstate = 1;
  Idx lastNonEk = -1;    //!! Remembers the last symbol from E-Ek that has been read.
	StateSet::Iterator lit1, lit2;
	TransSet::Iterator tit1, tit1_end, tit2, tit2_end;
	std::map< std::pair<Idx,Idx>, Idx>::iterator rcit;

  // push all combinations of initial states on todo stack
  for (lit1 = tildeGen.InitStatesBegin(); lit1 != tildeGen.InitStatesEnd(); lit1++) {
    for (lit2 = rGen.InitStatesBegin(); lit2 != rGen.InitStatesEnd(); lit2++) {
      currentstates = std::make_pair(*lit1, *lit2);
      todo.push(currentstates);
      testedTrans[currentstates] = tmpstate++;
//    std::cout << "TODO list: " 
//    << "(" << tildeGen.StateName(*lit1) << "," << *lit2 << ") " 
//    << std::endl;
    }
  }



	// start algorithm
  	while (! todo.empty()) {
    	  // get next reachable state from todo stack
    	  currentstates = todo.top();
    	  todo.pop();
    	  // iterate over all tildeGen transitions (includes execution of shared events)
    	  tit1 = tildeGen.TransRelBegin(currentstates.first);
    	  tit1_end = tildeGen.TransRelEnd(currentstates.first);
    	  for (; tit1 != tit1_end; ++tit1) {
      	  // if the event not shared
      	  if (! sharedalphabet.Exists(tit1->Ev)) {
            newstates = std::make_pair(tit1->X2, currentstates.second);
//    std::cout << "Non-shared: " 
//    << "(" << tildeGen.StateName(currentstates.first) << "," << currentstates.second << ") -- " 
//    << tildeGen.EventName(tit1->Ev) << " -- > (" 
//    << tildeGen.StateName(newstates.first) << "," << newstates.second <<")" 
//    << std::endl;
            // add to todo list if new
            rcit = testedTrans.find(newstates);
            if (rcit == testedTrans.end()) {
              todo.push(newstates);
              testedTrans[newstates] = tmpstate++;
            }
            else {
              tmpstate = rcit->second;
            }
          }
      	  // shared event
      	  else {
            // find shared transitions
            tit2 = rGen.TransRelBegin(currentstates.second, tit1->Ev);
            tit2_end = rGen.TransRelEnd(currentstates.second, tit1->Ev);
//    std::cout << "TEST: " 
//    << "(" << tit2->X1 << "," << tit2->X2 << ") under " 
//    << rGen.EventName(tit2->Ev) << std::endl;
	          // problematic event add it to Ek and leave the procedure
	          if (tit2 == tit2_end) {
		            ek.Insert(lastNonEk);
		            std::cerr << "ConDecExtension: Event " << rGen.EventName(lastNonEk) 
			                    << " has been added to Ek." << std::endl;
		            return true;
	          }
	          for (; tit2 != tit2_end; ++tit2) {
              newstates = std::make_pair(tit1->X2, tit2->X2);
              if (! ek.Exists(tit1->Ev)) {
                lastNonEk = tit1->Ev;
//                std::cout << "lastNonEk = " << rGen.EventName(tit1->Ev) << std::endl;
              }
//   std::cout << "Shared: " 
//    << "(" << tildeGen.StateName(currentstates.first) << "," << currentstates.second << ") -- " 
//    << tildeGen.EventName(tit1->Ev) << " -- > (" 
//    << tildeGen.StateName(tit1->X2) << "," << tit2->X2 <<")" << std::endl;
              // add to todo list if composition state is new
              rcit = testedTrans.find(newstates);
              if (rcit == testedTrans.end()) {
                todo.push(newstates);
                testedTrans[newstates] = tmpstate++;
              }
              else {
                tmpstate = rcit->second;
              }
       	    }
          }
        }
      }
      return false;
}

} // name space 



