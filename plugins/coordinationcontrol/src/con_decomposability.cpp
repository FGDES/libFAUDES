/** @file con_decomposability.cpp Conditionaldecomposability */

/*
 * Implementation of the Conditionally decomposable algorithm
 *
 * Copyright (C) 2011, 2015  Tomas Masopust
 *
 */


#include "con_decomposability.h"
#include <vector>

namespace faudes {

bool IsConditionalDecomposable(const Generator& gen, const EventSetVector& ee, const EventSet& ek, Generator& proof) {
	FD_DF("Conditionaldecomposability checking...");
	
  // the generator must be deterministic
	if (gen.IsDeterministic() == false) {
	  std::stringstream errstr;
	  errstr << "Generator must be deterministic, but is nondeterministic";
	  throw Exception("ConditionalDecomposability", errstr.str(), 201);
	}

  Idx n = ee.Size();  // the number of components
	if (n < 2) {
	  std::stringstream errstr;
	  errstr << "At least two alphabets must be included in the EventSetVector";
	  throw Exception("ConditionalDecomposability", errstr.str(), 201);
	}

	// Compute the union of all Ei
	EventSet unionset;
  for (Idx i = 0; i < ee.Size(); ++i) {
		unionset = unionset + ee.At(i);
	}

	// Compute the set of shared events
	EventSet shared;
	for (Idx i = 0; i < ee.Size(); i++) {
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
	    throw Exception("ConditionalDecomposability", errstr.str(), 100);
  	}

  // Alphabet of the generator must contain Ek
	ok = SetInclusion(ek,gen.Alphabet());
  	if (ok == false) {
	    std::stringstream errstr;
	    errstr << "Generator alphabet does not include the alphabet ek";
	    throw Exception("ConditionalDecomposability", errstr.str(), 100);
  	}

  // Ek must contain all shared events
	ok = SetInclusion(shared,ek);
  	if (ok == false) {
	    std::stringstream errstr;
	    errstr << "Ek does not include all shared events";
	    throw Exception("ConditionalDecomposability", errstr.str(), 100);
  	}

  //call IsCD for each pair (Ei, \cup_{j<>i} Ej)
  for (Idx i = 0; i < n; ++i) {
    EventSetVector eev;
    eev.Append(ee.At(i));
    //union of Ej for j<>i
	  EventSet uset;
    for (Idx j = 0; j < n; ++j) {
      if ( i == j ) continue;
		  uset = uset + ee.At(j);
	  }
    eev.Append(uset);
    Generator p;
    proof = p;
    if ( IsCD(gen,eev,ek,unionset,proof) == false ) return false;
  }

  return true;
}

//here we have only two alphabets
bool IsCD(
  const Generator& gen, 
  const EventSetVector& ee, 
  const EventSet& ek, 
  const EventSet& unionset,
  Generator& proof) 
{

	Idx i;

	// Make copies of the generator
	GeneratorVector copies;
	EventSet unionsetNew = unionset;		// unionset with new events
	EventSet::Iterator eit;

	for (i = 0; i < ee.Size(); ++i) {
		EventSet notInEiEk;
		notInEiEk = unionset - ( ee.At(i) + ek );
		Generator copy = gen;
		for (eit = notInEiEk.Begin(); eit != notInEiEk.End(); ++eit) {
			copy.EventRename(*eit,copy.UniqueEventName(""));
		}
		copies.Append(copy);
		unionsetNew = unionsetNew + copy.Alphabet();
	}
	
	// Compute tilde G
	Generator tildeG = copies.At(0);
	for (i = 1; i < copies.Size(); ++i) {
		Parallel(tildeG,copies.At(i),tildeG);
	}

	// Compute the inverse image of G
	Generator invGen;
	aInvProject(gen,unionsetNew,invGen);
	Trim(tildeG);
	Trim(invGen);
	if (LanguageInclusion(tildeG,invGen)) {
		return true;
	}
	
	// Save proof automaton showing that it is not CD
	LanguageComplement(invGen);
	Generator proofGen;
	LanguageIntersection(invGen,tildeG,proofGen);
	Trim(proofGen);
	proofGen.ClearStateNames();
	proof = proofGen;
	
	return false;
}


} // name space 



