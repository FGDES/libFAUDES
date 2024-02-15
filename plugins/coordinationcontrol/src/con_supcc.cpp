/** @file con_controllability.cpp Conditionalcontrollability */

/*
 * Implementation of the conditionally controllable algorithm
 *
 * Copyright (C) 2012  Tomas Masopust
 *
 */


#include "con_supcc.h"
#include "con_include.h"
#include "op_include.h"
#include <vector>

namespace faudes {

bool SupConditionalControllable(
	const Generator& gen, 
	const GeneratorVector& genVector, 
	const EventSet& ACntrl,
	const EventSet& InitEk,
	GeneratorVector& supVector,
	Generator& Coord) {

	/* Steps of the algorithm
	 * 1. compute Ek so that it contains all shared events
	 * 2. extend Ek so that K = L(gen) is CD
	 * 3. extend Ek so that Pk is Li-observer, i=1,2,...,n
	 * 4. compute the coordinator Gk and supCk, supCi+k
	 * 5. if supCk subseteq Pk(supCi+k), return supCi+k and the supervised coordinator (supCk)
	 * 6. extend Ek so that P^{i+k}_k is (P^{i+k}_k)^{-1}(Li)-observer and OCC for that language
	 * 7. recompute supCi+k a return them and the supervised coordinator
	 */
	
	Idx i;

	// the generators must be deterministic and prefix-closed
	if (gen.IsDeterministic() == false || IsPrefixClosed(gen) == false) {
	  std::stringstream errstr;
	  errstr << "Generators must be deterministic and prefix-closed";
	  throw Exception("ConditionalControllability", errstr.str(), 201);
	}	
	for (i = 0; i < genVector.Size(); i++) {
	  if (genVector.At(i).IsDeterministic() == false || IsPrefixClosed(genVector.At(i)) == false) {
	    std::stringstream errstr;
	    errstr << "Generators of genVector must be deterministic and prefix-closed";
	    throw Exception("ConditionalControllability", errstr.str(), 201);
	  }
	}

/* 1. compute Ek so that it contains all shared events */
	EventSet unionset;			// contains union of Ei
	EventSet shared;			// contains union of intersections

	// Compute unionset
	for (i = 0; i < genVector.Size(); i++) {
		SetUnion(unionset,genVector.At(i).Alphabet(),unionset);
	}

	// Compute the set of shared events
	for (i = 0; i < genVector.Size(); i++) {
	  for (Idx j = 0; j < genVector.Size(); j++) {
	    if (j != i) {
		EventSet eHelpInt;
		SetIntersection(genVector.At(i).Alphabet(),genVector.At(j).Alphabet(),eHelpInt);
		SetUnion(shared,eHelpInt,shared);
	    }
	  }
	}

	EventSet ek = shared + InitEk;
	std::cerr << "Initial Ek = { ";
	EventSet::Iterator eit;
	for (eit=ek.Begin(); eit != ek.End(); ++eit) {
	  std::cerr << ek.SymbolicName(*eit) << " ";
	}	
	std::cerr << "}" << std::endl;

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

/* 2. extend Ek so that K = L(gen) is CD */
	std::cerr << "Extension of Ek for Conditional Decomposability..." << std::endl;
	EventSetVector ee;
	for (i = 0; i < genVector.Size(); i++) {
		ee.Append(genVector.At(i).Alphabet());
	}	
	ConDecExtension(gen,ee,ek);

/* 3. extend Ek so that Pk is Li-observer, i=1,2,...,n */
	bool repeat = true;
	std::cerr << "Extension of Ek for Li-observers..." << std::endl;
	while (repeat) {
	  repeat = false;
	  for (i = 0; i < genVector.Size(); i++) {
	    if (IsObs(genVector.At(i),ek*genVector.At(i).Alphabet()) == false ) {
	      repeat = true;
	      EventSet ekHelp = ek * genVector.At(i).Alphabet();
	      calcNaturalObserver(genVector.At(i),ekHelp);
	      SetUnion(ek,ekHelp,ek);
	    }
	  }	
	}
	std::cerr << "Extended Ek = { ";
	for (eit=ek.Begin(); eit != ek.End(); ++eit) {
	  std::cerr << ek.SymbolicName(*eit) << " ";
	}	
	std::cerr << "}" << std::endl;

/* 4. compute the coordinator Gk and supCk, supCi+k */
	// coordinator Gk = || P_k(G_i)
	Generator gk;
	FullLanguage(ek,gk);
	for (i = 0; i < genVector.Size(); i++) {
	  Generator pomGen;
	  Project(genVector.At(i),ek,pomGen);
	  Parallel(gk,pomGen,gk);
	}

	// compute P_k(K) and the vector of P_{i+k}(K)
	Generator pk;
	Project(gen,ek,pk);
	GeneratorVector specVect;
	for (i = 0; i < genVector.Size(); i++) {
	  Generator pomGen;
	  Project(gen,ek+genVector.At(i).Alphabet(),pomGen);
	  specVect.Append(pomGen);
	}

	// the set of controllable events E_{k,c}	
	EventSet ekc;
	SetIntersection(ek,ACntrl,ekc);
	// supCk supervisor
	Generator supCk;
	// vector of supC_{i+k} supervisors
	GeneratorVector supCkVector;

	// compute supremal controllable sublanguage of P_k(K) wrt L(G_k) and E_{k,c}
	// if (IsControllable(gk,ekc,pk)) --  DO NOT USE, if K not subset of L, then the results are wrong!!!
	SupConClosed(gk,ekc,pk,supCk);

	// compute supremal controllable sublanguage of P_{i+k}(K) wrt L(G_i)||supCk and E_{1+k,c}
	for (i = 0; i < genVector.Size(); i++) {
	  Generator helpPlant;
	  Parallel(genVector.At(i),supCk,helpPlant);
	  Generator supCik;
	  SupConClosed(helpPlant,helpPlant.Alphabet()*ACntrl,specVect.At(i),supCik);
	  supCkVector.Append(supCik); // supC_{i+k} = supCik
	}

/* 5. if supCk subseteq Pk(supCi+k), return supCi+k and the supervised coordinator*/
	bool incl = true;
	// TODO -- implement this test nodeterministically!!!
	for (i = 0; i < supCkVector.Size(); i++) {
	  Generator PkHelp;
	  Project(supCkVector.At(i),ek,PkHelp);
	  incl = incl && LanguageInclusion(supCk,PkHelp);
	}
	// incl = true ==> ok
	if (incl) {
	  Coord = supCk;
	  supVector = supCkVector;
	  std::cout << "Finished: supCk is a subset of all Pk(supCi+k)." << std::endl;
	  return true;
	}

/* 6. extend Ek so that P^{i+k}_k is (P^{i+k}_i)^{-1}(Li)-observer and OCC for that language */
// Here LCC is used instead of OCC
	// compute a vector of (P^{i+k}_k)^{-1}(Li)
	GeneratorVector invLiVect;
	for (i = 0; i < genVector.Size(); i++) {
	  Generator invLi;
	  aInvProject(genVector.At(i),ek+genVector.At(i).Alphabet(),invLi);
	  invLiVect.Append(invLi);
	}
	repeat = true;
	while (repeat) {
	  repeat = false;
	  for (i = 0; i < invLiVect.Size(); i++) {
	    if (IsObs(invLiVect.At(i),ek) && IsLCC(invLiVect.At(i),ek) == false ) {
	      repeat = true;
	      calcNaturalObserverLCC(genVector.At(i),ekc,ek);
	    }
	  }	
	}

/* 7. recompute supCi+k a return them and the supervised coordinator*/
	// recompute the set of controllable events E_{k,c}	
	ekc = ek * ACntrl;
	supCk.Clear();
	supCkVector.Clear();

	// coordinator Gk = || P_k(G_i)
	gk.Clear();
	FullLanguage(ek,gk);
	for (i = 0; i < genVector.Size(); i++) {
	  Generator pomGen;
	  Project(genVector.At(i),ek,pomGen);
	  Parallel(gk,pomGen,gk);
	}

	// compute P_k(K) and the vector of P_{i+k}(K)
	pk.Clear();
	Project(gen,ek,pk);
	specVect.Clear();
	for (i = 0; i < genVector.Size(); i++) {
	  Generator pomGen;
	  Project(gen,ek+genVector.At(i).Alphabet(),pomGen);
	  specVect.Append(pomGen);
	}

	// compute supremal controllable sublanguage of P_k(K) wrt L(G_k) and E_{k,c}
	SupConClosed(gk,ekc,pk,supCk);

	// compute supremal controllable sublanguages of P_{i+k}(K) wrt L(G_i)||supCk and E_{1+k,c}
	for (i = 0; i < genVector.Size(); i++) {
	  Generator helpPlant;
	  Parallel(genVector.At(i),supCk,helpPlant);
	  Generator supCik;
	  SupConClosed(helpPlant,helpPlant.Alphabet()*ACntrl,specVect.At(i),supCik);
	  supCkVector.Append(supCik); // supC_{i+k} = supCik
	}
	
	// the resulting supervisors
	supVector = supCkVector;
	Coord = supCk;
	std::cout << "Finished: supCk was NOT a subset of all Pk(supCi+k)." << std::endl;
	return true;

}


} // name space 



