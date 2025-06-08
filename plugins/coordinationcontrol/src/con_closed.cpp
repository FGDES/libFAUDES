/** @file con_closed.cpp Conditionalclosedness */

/*
 * Implementation of the conditionally clossed algorithm
 *
 * Copyright (C) 2011  Tomas Masopust
 *
 */


#include "con_closed.h"
#include <vector>

namespace faudes {

bool IsConditionalClosed(const GeneratorVector& specVect, const Generator& pk, const GeneratorVector& genVect, const Generator& gk) {
	FD_DF("Conditionalclosedness checking...");
	
	Idx i;

	// there must be the same number of parameters
	if (specVect.Size() != genVect.Size()) {
	  std::stringstream errstr;
	  errstr << "The sizes of specVect and genVect are different.";
	  throw Exception("ConditionalClosedness", errstr.str(), 201);
	}

	// the generators must be deterministic
	for (i = 0; i < specVect.Size(); i++) {
	  if (specVect.At(i).IsDeterministic() == false) {
	    std::stringstream errstr;
	    errstr << "Generators of specVect must be deterministic, but there is a nondeterministic one";
	    throw Exception("ConditionalClosednes", errstr.str(), 201);
	  }
	}
	
	if (pk.IsDeterministic() == false) {
	  std::stringstream errstr;
	  errstr << "Generator pk must be deterministic, but is nondeterministic";
	  throw Exception("ConditionalClosedness", errstr.str(), 201);
	}
	
	for (i = 0; i < genVect.Size(); i++) {
	  if (genVect.At(i).IsDeterministic() == false) {
	    std::stringstream errstr;
	    errstr << "Generators of genVect must be deterministic, but there is a nondeterministic one";
	    throw Exception("ConditionalClosedness", errstr.str(), 201);
	  }
	}
	
	if (gk.IsDeterministic() == false) {
	  std::stringstream errstr;
	  errstr << "Generator gk must be deterministic, but is nondeterministic";
	  throw Exception("ConditionalClosedness", errstr.str(), 201);
	}

	// check if P_k(K) is Lm(G_k)-closed
	if (IsRelativelyClosed(gk,pk) == false) {
	  return false;
	}

	// check if P_{i+k}(K) is Lm(G_i)||P_k(K)-closed
	for (i = 0; i < specVect.Size(); i++) {
	  Generator helpPlant;
	  Parallel(genVect.At(i),pk,helpPlant);
	  if (IsRelativelyClosed(helpPlant,specVect.At(i)) == false) {
	    return false;
	  }
	}
	
	return true;
}


} // name space 



