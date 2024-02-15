/** @file con_controllability.cpp Conditionalcontrollability */

/*
 * Implementation of the conditionally controllable algorithm
 *
 * Copyright (C) 2011  Tomas Masopust
 *
 */


#include "con_controllability.h"
#include <vector>

namespace faudes {

bool IsConditionalControllable(const GeneratorVector& specVect, const Generator& pk, const GeneratorVector& genVect, const Generator& gk, const EventSet& ACntrl) {
	FD_DF("Conditionalcontrollability checking...");
	
	Idx i;

	// there must be the same number of parameters
	if (specVect.Size() != genVect.Size()) {
	  std::stringstream errstr;
	  errstr << "The sizes of specVect and genVect are different.";
	  throw Exception("ConditionalControllability", errstr.str(), 201);
	}

	// the generators must be deterministic
	for (i = 0; i < specVect.Size(); i++) {
	  if (specVect.At(i).IsDeterministic() == false) {
	    std::stringstream errstr;
	    errstr << "Generators of specVect must be deterministic, but there is a nondeterministic one";
	    throw Exception("ConditionalControllability", errstr.str(), 201);
	  }
	}
	
	if (pk.IsDeterministic() == false) {
	  std::stringstream errstr;
	  errstr << "Generator pk must be deterministic, but is nondeterministic";
	  throw Exception("ConditionalControllability", errstr.str(), 201);
	}
	
	for (i = 0; i < genVect.Size(); i++) {
	  if (genVect.At(i).IsDeterministic() == false) {
	    std::stringstream errstr;
	    errstr << "Generators of genVect must be deterministic, but there is a nondeterministic one";
	    throw Exception("ConditionalControllability", errstr.str(), 201);
	  }
	}
	
	if (gk.IsDeterministic() == false) {
	  std::stringstream errstr;
	  errstr << "Generator gk must be deterministic, but is nondeterministic";
	  throw Exception("ConditionalControllability", errstr.str(), 201);
	}

	EventSet ekc;	// the set of controllable events E_{k,c}
	SetIntersection(gk.Alphabet(),ACntrl,ekc);
	
	// check whether the generators are nonblocking
	if (IsNonblocking(pk) && IsNonblocking(gk) == false) {
	  std::stringstream errstr;
	  errstr << "Generators pk and gk must be nonblocking";
	  throw Exception("ConditionalControllability", errstr.str(), 201);
	}

	// check if P_k(K) is controllable wrt L(G_k) and E_{k,c}
	if (IsControllable(gk,ekc,pk) == false) {
	  return false;
	}

	// check if P_{i+k}(K) is controllable wrt L(G_i)||\overline{P_k(K)} and E_{1+k,c}
	Generator pkClosure = pk;
	PrefixClosure(pkClosure);
	for (i = 0; i < specVect.Size(); i++) {
	  Generator helpPlant;
	  Parallel(genVect.At(i),pkClosure,helpPlant);
	  EventSet uSet, Eikc;
	  SetUnion(genVect.At(i).Alphabet(),pk.Alphabet(),uSet);
	  SetIntersection(uSet,ACntrl,Eikc);
	  if (IsControllable(helpPlant,Eikc,specVect.At(i)) == false) {
	    return false;
	  }
	  if (IsNonblocking(genVect.At(i)) && IsNonblocking(specVect.At(i)) == false) {
	    std::stringstream errstr;
	    errstr << i << "th generator or specification projection is blocking";
	    throw Exception("ConditionalControllability", errstr.str(), 201);
	  }
	}
	
	return true;
}


} // name space 



