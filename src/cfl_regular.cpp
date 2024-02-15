/** @file cfl_regular.cpp

Operations on regular languages.
See [Cassandras and Lafortune. Introduction to Discrete Event Systems] for an
introduction to regular language operations.
Operations are always performed on language(s) marked by the passed generator(s),
resulting in the language(s) marked by the resulting generator(s).
Only if mentioned extra, the same is done for the involved generated (prefix-closed)
languages.

*/

/* FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2006  Bernd Opitz
Copyright (C) 2008  Sebstian Perk
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

  
#include "cfl_regular.h"
#include "cfl_determin.h"


/* turn on debugging for this file */
//#undef FD_DF
//#define FD_DF(a) FD_WARN(a);

namespace faudes {

// LanguageUnionNonDet(rGen1, rGen2, rResGen)
void LanguageUnionNonDet(const Generator& rGen1, const Generator& rGen2,
			 Generator& rResGen) {
            
  FD_DF("LanguageUnionNonDet("<< rGen1.Name() 
	<< "," << rGen2.Name() << ")");
    
  // are state names enabled?
  bool stateNamesEnabled=rResGen.StateNamesEnabled();
    
  // use pointer pResGen to result rResGen; if rResGen is identical to
  // one of the parameters, allocate temporary object and copy back later
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen1 || &rResGen== &rGen2) {
    pResGen= rResGen.New();
  }

  // prepare result
  pResGen->Clear();

  // union of alphabets
  pResGen->InjectAlphabet(rGen1.Alphabet()+rGen2.Alphabet());
    
  // Maps from states of rGen1 and rGen2 to states of ResGen
  std::map<Idx,Idx> Gen1StatesMap;
  std::map<Idx,Idx> Gen2StatesMap;
    
  // "union" of states: insert states representing the states of rGen1 and rGen2
  StateSet::Iterator sit;
  for (sit = rGen1.StatesBegin(); sit != rGen1.StatesEnd(); ++sit) {
    if (stateNamesEnabled) {
      Gen1StatesMap[*sit] = pResGen->InsState(pResGen->UniqueStateName(rGen1.StateName(*sit)+"(1)"));
    }
    else {
      Gen1StatesMap[*sit] = pResGen->InsState();
    }
  }
  for (sit = rGen2.StatesBegin(); sit != rGen2.StatesEnd(); ++sit) {
    if (stateNamesEnabled) {
      Gen2StatesMap[*sit] = pResGen->InsState(pResGen->UniqueStateName(rGen2.StateName(*sit)+"(2)"));
    }
    else {
      Gen2StatesMap[*sit] = pResGen->InsState();
    }
  }
    
  // "union" of transition relations
  TransSet::Iterator tit;
  for (tit = rGen1.TransRelBegin(); tit != rGen1.TransRelEnd(); ++tit) {
    pResGen->SetTransition(Gen1StatesMap[tit->X1], tit->Ev, Gen1StatesMap[tit->X2]);
  }
  for (tit = rGen2.TransRelBegin(); tit != rGen2.TransRelEnd(); ++tit) {
    pResGen->SetTransition(Gen2StatesMap[tit->X1], tit->Ev, Gen2StatesMap[tit->X2]);
  }
    
  // "union" of init states
  for (sit = rGen1.InitStatesBegin(); sit != rGen1.InitStatesEnd(); ++sit) {
    pResGen->SetInitState(Gen1StatesMap[*sit]);
  }
  for (sit = rGen2.InitStatesBegin(); sit != rGen2.InitStatesEnd(); ++sit) {
    pResGen->SetInitState(Gen2StatesMap[*sit]);
  }
    
  // "union" of marked states
  for (sit = rGen1.MarkedStatesBegin(); sit != rGen1.MarkedStatesEnd(); ++sit) {
    pResGen->SetMarkedState(Gen1StatesMap[*sit]);
  }
  for (sit = rGen2.MarkedStatesBegin(); sit != rGen2.MarkedStatesEnd(); ++sit) {
    pResGen->SetMarkedState(Gen2StatesMap[*sit]);
  }
    
  // set name of result
  pResGen->Name(CollapsString("UnionNonDet("+rGen1.Name()+","+rGen2.Name()+")"));
    
  // if necessary, move pResGen to rResGen
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
    
}

// LanguageUnion(rGen1, rGen2, rResGen)
void LanguageUnion(const Generator& rGen1, const Generator& rGen2, 
		   Generator& rResGen) {

  FD_DF("LanguageUnion("<< rGen1.Name() 
	<< "," << rGen2.Name() << ")");
   
  // fix name
  std::string name1 = rGen1.Name();
  std::string name2 = rGen2.Name();

  // avoid copy
  Generator* pTempGen = rResGen.New();
    
  // perform nondeterministic language union
  LanguageUnionNonDet(rGen1, rGen2, *pTempGen);
    
  // make deterministic
  Deterministic(*pTempGen, rResGen);

  // dispose temp
  delete pTempGen;

  // set name of result
  rResGen.Name(CollapsString("Union("+name1+","+name2+")"));

}

// LanguageUnion(rGenVec, rResGen)
void LanguageUnion(const GeneratorVector& rGenVec, Generator& rResGen) {

  // ignore empty
  if(rGenVec.Size()==0) {
    return;
  }

  // copy one
  if(rGenVec.Size()==1) {
    rResGen=rGenVec.At(0);
    return;
  }

  // avoid final copy
  Generator* pTempGen = rResGen.New();

  // run union on others
  LanguageUnionNonDet(rGenVec.At(0),rGenVec.At(1),*pTempGen);
  for(GeneratorVector::Position i=2; i<rGenVec.Size(); i++) 
    LanguageUnionNonDet(rGenVec.At(i),*pTempGen,*pTempGen);

  // make deterministic
  Deterministic(*pTempGen, rResGen);

  // dispose temp
  delete pTempGen;

  // set name of result
  rResGen.Name(CollapsString("Union(...)"));
}


// LanguageIntersection(rGen1, rGen2, rResGen)
void LanguageIntersection(const Generator& rGen1, const Generator& rGen2, 
			  Generator& rResGen) {
  FD_DF("LanguageIntersection("<< rGen1.Name()  << "," << rGen2.Name() << ")");
    
  // fix name
  std::string name1 = rGen1.Name();
  std::string name2 = rGen2.Name();

  // the product of rGen1 and rGen2 implements the language intersection
  Product(rGen1, rGen2, rResGen);
  rResGen.Name(CollapsString("Intersection("+name1+","+name2+")"));
  FD_DF("LanguageIntersection("<< rGen1.Name()  << "," << rGen2.Name() << "): done");
    
}


// LanguageIntersection(rGenVec, rResGen)
void LanguageIntersection(const GeneratorVector& rGenVec, Generator& rResGen) 
{

  // ignore empty
  if(rGenVec.Size()==0) {
    return;
  }

  // copy one
  if(rGenVec.Size()==1) {
    rResGen=rGenVec.At(0);
    return;
  }

  // run product on others
  LanguageIntersection(rGenVec.At(0),rGenVec.At(1),rResGen);
  for(GeneratorVector::Position i=2; i<rGenVec.Size(); i++) 
    LanguageIntersection(rGenVec.At(i),rResGen,rResGen);
}


// EmptyLanguageIntersection(rGen1, rGen2)
bool EmptyLanguageIntersection(const Generator& rGen1, const Generator& rGen2) {
  FD_DF("EmptyLanguageIntersection("<< rGen1.Name() 
	<< "," << rGen2.Name() << ")");

  // not tested for non-det automata
  bool g1_det = rGen1.IsDeterministic();
  bool g2_det = rGen2.IsDeterministic();
  if( (!g1_det) || (!g2_det)) {
    std::stringstream errstr;
    errstr << "EmptyLanguageIntersection has not been tested for nondeterministic generators";
    throw Exception("EmptyLanguageIntersection", errstr.str(), 201);
  }

  // Perform product and break when a marking is reached simultaneously)

  // todo stack
  std::stack< std::pair<Idx,Idx> > todo;
  std::set< std::pair<Idx,Idx> > done;
  // iterate variables
  std::pair<Idx,Idx> currentstates, newstates;
  StateSet::Iterator lit1, lit2;
  TransSet::Iterator tit1, tit1_end, tit2, tit2_end;
  // convenience
  const StateSet& mark1 = rGen1.MarkedStates();
  const StateSet& mark2 = rGen2.MarkedStates();
  // restrict search to coaccessible states
  StateSet coac1 = rGen1.CoaccessibleSet();
  StateSet coac2 = rGen2.CoaccessibleSet();

  // push all combinations of coaccessible initial states on todo stack
  FD_DF("EmptyLanguageIntersection: push all combinations of initial states to todo:");
  StateSet init1 = coac1 * rGen1.InitStates();
  StateSet init2 = coac2 * rGen2.InitStates();
  for (lit1 = init1.Begin(); lit1 != init1.End(); ++lit1) {
    for (lit2 = init2.Begin(); lit2 != init2.End(); ++lit2) {
      currentstates = std::make_pair(*lit1, *lit2);
      todo.push(currentstates);
    }
  }

  // process todo stack
  bool empty = true;
  FD_DF("EmptyLanguageIntersection: processing reachable states:");
  while (!todo.empty()) {
    // allow for user interrupt
    // LoopCallback();
    // allow for user interrupt, incl progress report
    FD_WPC(done.size(),done.size()+todo.size(),"Product(): processing"); 
    // get next reachable state from todo stack
    currentstates = todo.top();
    todo.pop();
    done.insert(currentstates);
    //FD_DF("EmptyLanguageIntersection: processing " << currentstates.first << "|" << currentstates.second);
    // test for sync acceptance (do this here to include initial states)
    if(mark1.Exists(currentstates.first) && mark2.Exists(currentstates.second)) {
       empty=false; 
       break;
    }
    // iterate over all rGen1/rGen2 transitions
    tit1 = rGen1.TransRelBegin(currentstates.first);
    tit1_end = rGen1.TransRelEnd(currentstates.first);
    tit2 = rGen2.TransRelBegin(currentstates.second);
    tit2_end = rGen2.TransRelEnd(currentstates.second);
    while((tit1 != tit1_end) && (tit2 != tit2_end)) {
      // shared event
      if(tit1->Ev == tit2->Ev) {
        // push new state
        newstates = std::make_pair(tit1->X2, tit2->X2);
        if(done.find(newstates)==done.end()) 
	  if(coac1.Exists(newstates.first))
  	    if(coac2.Exists(newstates.second))
              todo.push(newstates);
        // increment transition
        ++tit1;
        ++tit2;
      }
      // try resync tit1
      else if (tit1->Ev < tit2->Ev) {
        ++tit1;
      }
      // try resync tit2
      else if (tit1->Ev > tit2->Ev) {
        ++tit2;
      }
    }
  }

#ifdef FAUDES_CODE
  // Alternative test for validation
  Generator ProductGen;
  ProductGen.StateNamesEnabled(false);
  Product(rGen1, rGen2, ProductGen);
  bool altempty = IsEmptyLanguage(ProductGen);
  if(empty != altempty) {
    rGen1.Write();
    rGen2.Write();
    FD_ERR("EmptyLanguageIntersection(): ERROR  -- ref result: " << altempty);
  }
#endif 

  // done
  return empty;
}

// LanguageDisjoint(rGen1, rGen2)
bool LanguageDisjoint(const Generator& rGen1, const Generator& rGen2) {
  FD_DF("LanguageDisjoint("<< rGen1.Name() 
	<< "," << rGen2.Name() << ")");
  return EmptyLanguageIntersection(rGen1,rGen2);
}

// Automaton(rGen)
void Automaton(Generator& rGen, const EventSet& rAlphabet) {
  FD_DF("Automaton("<< rGen.Name() << "," << rAlphabet.Name() << ")");
    
  TransSet::Iterator tit;
  EventSet::Iterator eit;
  StateSet::Iterator sit;

  // for correct result, rGen has to be deterministic!
#ifdef FAUDES_CHECKED
  if ( !(rGen.IsDeterministic()) ) {
    FD_WARN("Automaton(): nondeterministic parameter " << rGen.Name() <<".");
  }
#endif 

  // prepare result
  rGen.Name(CollapsString("Automaton(" + rGen.Name() + "," + rAlphabet.Name() + ")"));
    
  // extend rGen.Alphabet() by rAlphabet
  rGen.InjectAlphabet(rGen.Alphabet()+rAlphabet);
    
  // trim (this used to be coaccessible only until 2024)
  rGen.Trim();
    
  // introduce a dump state (unmarked)
  Idx dump;
  if (rGen.StateNamesEnabled()) {
    std::string dumpstr=rGen.UniqueStateName("dump");
    dump = rGen.InsState(dumpstr);
  } else {
    dump = rGen.InsState();
  }
  for(eit=rGen.AlphabetBegin();eit!=rGen.AlphabetEnd();++eit) 
    rGen.SetTransition(dump,*eit,dump);
  bool dumpReached=false; // record, whether dump state is indeed used
	
  // if there is no initial state, the dump state becomes the initial state
  if(rGen.InitStates().Empty()){
    rGen.SetInitState(dump);
    dumpReached=true;
  }
    
  // introduce transitions to dumpstate (reference implementation)
  /*
  for (sit = rGen.StatesBegin(); sit != rGen.StatesEnd(); ++sit) {        
    for (eit = rGen.Alphabet().Begin(); eit != rGen.Alphabet().End(); ++eit) {
      // If no transition is defined for this state and event, insert respective
      // transition to dump state (note that dump state itself is also treated here
      // and thus provided with selfloops)
      if (rGen.TransRelBegin(*sit, *eit) == rGen.TransRelEnd(*sit, *eit)) {
	rGen.SetTransition(*sit, *eit, dump);
	// indicate that dumstate was reached
	if(*sit!=dump) dumpReached=true;
      }
    }        
  }
  */
  

  // we can do faster ... but is this worth it?
  Idx cs=0, ce=0;
  tit=rGen.TransRelBegin();
  sit=rGen.StatesBegin();
  for(;sit!=rGen.StatesEnd();++sit) {
    FD_DF("Automaton: processing state " << *sit);
    cs=*sit;
    for(;tit!=rGen.TransRelEnd();++tit) 
      if(tit->X1 >= cs) break;
    bool fina= (tit!=rGen.TransRelEnd());
    bool finb= false;
    if(fina) finb = (tit->X1 == cs);
    if(!finb) {
      FD_DF("Automaton: completing state");
      for(eit=rGen.AlphabetBegin();eit!=rGen.AlphabetEnd();++eit) 
        rGen.SetTransition(cs,*eit,dump);
      dumpReached=true;
      continue;
    }  
    eit=rGen.AlphabetBegin();
    while(tit!=rGen.TransRelEnd()) {    
      ce=*eit;
      FD_DF("Automaton: processing " << tit->Str() << " awaiting " << ce);
      while(ce<tit->Ev) {
        FD_DF("Automaton: add " << cs << "-(" << ce << ")->");
        rGen.SetTransition(cs,ce,dump);
        dumpReached=true;
        ++eit;
        ce=*eit;
      }
      bool fin1=false;
      while(tit!=rGen.TransRelEnd()) {
        ++tit;
        if(tit->X1!=cs) {fin1=true; break;}
        if(tit->Ev!=ce) break;
        FD_DF("Automaton: skip " << tit->Str());
      }
      bool fin2= (tit==rGen.TransRelEnd());
      ++eit;
      if(fin1 || fin2) {
        while(eit != rGen.AlphabetEnd()) {
          FD_DF("Automaton: fin " << cs << "-(" << *eit  << ")->");
          rGen.SetTransition(cs,*eit,dump);
          dumpReached=true;
          ++eit;
        }
      }
      if(fin1||fin2) break;
    }
  }
  
  // if no transition was introduced (except for selfloops), remove dump state
  if(!dumpReached) 
    rGen.DelState(dump);
}

// Automaton(rGen)
void Automaton(Generator& rGen) {
  FD_DF("Automaton("<< rGen.Name() << ")");
  std::string name=rGen.Name();
  Automaton(rGen,rGen.Alphabet());    
  rGen.Name(CollapsString("Automaton(" + name + ")"));
}

// LanguageComplement(rGen,rAlphabet)
void LanguageComplement(Generator& rGen, const EventSet& rAlphabet) {
  FD_DF("LanguageComplement("<< rGen.Name() << "," << rAlphabet.Name() << ")");

  // fix name
  std::string name = rGen.Name();  
  
  // convert to automaton (avoiding statename "dump")
  bool stateNamesEnabled=rGen.StateNamesEnabled(); 
  rGen.StateNamesEnabled(false); 
  Automaton(rGen,rAlphabet);
  rGen.StateNamesEnabled(stateNamesEnabled); 
    
  // invert marking
  rGen.InjectMarkedStates(rGen.States() - rGen.MarkedStates());

  // set name    
  rGen.Name(CollapsString("Complement(" + name + "," + rAlphabet.Name() + ")"));
}

// LanguageComplement(rGen)
void LanguageComplement(Generator& rGen) {
  FD_DF("LanguageComplement("<< rGen.Name() << ")");
  std::string name=rGen.Name();
  LanguageComplement(rGen,rGen.Alphabet());
  rGen.Name(CollapsString("Complement(" + name + ")"));
  return;    
}

// language complement, uniform api
void LanguageComplement(const Generator& rGen, Generator& rRes) {
  rRes=rGen;
  LanguageComplement(rRes);
}


// language complement, uniform api
void LanguageComplement(const Generator& rGen, const EventSet& rSigma, Generator& rRes) {
  rRes=rGen;
  LanguageComplement(rRes,rSigma);
}




//LanguageDifference(rGen1, rGen2, rResGen)
void LanguageDifference(
  const Generator& rGen1, 
  const Generator& rGen2,
  Generator& rResGen) {
         
  FD_DF("LanguageDifference("<< rGen1.Name() << "," << rGen2.Name() << ")");

  // incl. all-empty case
  if(IsEmptyLanguage(rGen2)) {
    rResGen.Assign(rGen1);
    rResGen.Name(CollapsString("LanguageDifference(" + rGen1.Name() + "," + rGen2.Name() + ")"));
    return;
  }
     
  // use pointer pResGen to result rResGen
  Generator* pResGen = &rResGen;
  if(&rResGen == &rGen1 || &rResGen== &rGen2) {
    pResGen = rResGen.New();
  }    
    
  // due to the use of LanguageComplement(), rGen2 has to be deterministic
  #ifdef FAUDES_CHECKED
  if(!(rGen2.IsDeterministic())){
    std::stringstream errstr;
    errstr << "Nondeterministic parameter " << rGen2.Name() << ".";
    throw Exception("LanguageDifference()", errstr.str(), 101);
  }
  #endif
          
  // prepare result
  pResGen->Clear();
     
  // calculate "Lm1-Lm2" by building the intersection of Lm1 with the complement of Lm2
  // for correct result, complement has to be computed wrt the alphabet of Lm1 (!)
        
  *pResGen=rGen2;
  LanguageComplement(*pResGen,rGen1.Alphabet());
  LanguageIntersection(rGen1, *pResGen, *pResGen);
        
  FD_DF("LanguageDifference(...): stage 2");

  // if necessary, move pResGen to rResGen
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  } 
    
  FD_DF("LanguageDifference(...): done");
  return;
}

// LanguageConcatenateNonDet(rGen1, rGen2, rResGen)
void LanguageConcatenateNonDet(const Generator& rGen1, const Generator& rGen2, 
			       Generator& rResGen) {
  FD_DF("LanguageConcatenateNonDet(" << rGen1.Name() << "," << rGen2.Name() << ")");
            
  // are state names enabled in result?
  bool stateNamesEnabled=rResGen.StateNamesEnabled();
    
  // use pointer pResGen to result rResGen
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen1 || &rResGen== &rGen2) {
    pResGen= rResGen.New();
  }

  // prepare result
  pResGen->Clear();
    
  // union of alphabets
  pResGen->InjectAlphabet(rGen1.Alphabet() + rGen2.Alphabet());

  // Maps from states of rGen1 and rGen2 to states of ResGen
  std::map<Idx,Idx> Gen1StatesMap;
  std::map<Idx,Idx> Gen2StatesMap;

  // helpers
  StateSet::Iterator sit;
  TransSet::Iterator tit;
    
  // "union" of states: insert states representing the states of rGen1 and rGen2
  for (sit = rGen1.StatesBegin(); sit != rGen1.StatesEnd(); ++sit) {
    if (stateNamesEnabled) {
      Gen1StatesMap[*sit] = pResGen->InsState(pResGen->UniqueStateName(rGen1.StateName(*sit)+"(1)"));
    } else {
      Gen1StatesMap[*sit] = pResGen->InsState();
    }
  }
  for (sit = rGen2.StatesBegin(); sit != rGen2.StatesEnd(); ++sit) {
    if (stateNamesEnabled) {
      Gen2StatesMap[*sit] = pResGen->InsState(pResGen->UniqueStateName(rGen2.StateName(*sit)+"(2)"));
    } else {
      Gen2StatesMap[*sit] = pResGen->InsState();
    }
  }

  // "union" transitions: insert all rGen1 transitions
  for (tit = rGen1.TransRelBegin(); tit != rGen1.TransRelEnd(); ++tit) 
    pResGen->SetTransition(Gen1StatesMap[tit->X1], tit->Ev, Gen1StatesMap[tit->X2]);
    
  // "union" transitions: insert all rGen2 transitions
  for (tit = rGen2.TransRelBegin(); tit != rGen2.TransRelEnd(); ++tit) 
    pResGen->SetTransition(Gen2StatesMap[tit->X1], tit->Ev, Gen2StatesMap[tit->X2]);
    

  // initial state bug (detected by Tomas Masopust, fix by Klaus Schmidt)
  // 1) copy all transitions to the result, clear initial/marked status
  // 2) all initial states of G1 become initial states in the result
  // 3) if L1 contains the empty string, also all initial states of G2 become initial states
  //    in the result
  // 4) transition leading to a marked state in G1 also become transitions to all
  //    initial states of G2
  // 5) marked states of G2 become marked in the result


  // test whether L1 includes the empty string
  bool concatenateEpsilon1=false;
  for(sit = rGen1.InitStatesBegin(); sit != rGen1.InitStatesEnd(); ++sit) {
    if(rGen1.ExistsMarkedState(*sit)) {
      concatenateEpsilon1=true;
      break;
    }
  }

  // initial states of G1 become initial states in the result
  for (sit = rGen1.InitStatesBegin(); sit != rGen1.InitStatesEnd(); ++sit) 
    pResGen->SetInitState(Gen1StatesMap[*sit]);
    
  // if L1 contains the emtystring, G2 initial states become initial states in the result
  if(concatenateEpsilon1)
    for (sit = rGen2.InitStatesBegin(); sit != rGen2.InitStatesEnd(); ++sit) 
      pResGen->SetInitState(Gen2StatesMap[*sit]);
    
  // any G1 transition to a marked state must also lead to all initial states of G2
  for(tit = rGen1.TransRelBegin(); tit != rGen1.TransRelEnd(); ++tit) 
    if(rGen1.ExistsMarkedState(tit->X2)) 
      for(sit = rGen2.InitStatesBegin(); sit != rGen2.InitStatesEnd(); ++sit) 
        pResGen->SetTransition(Gen1StatesMap[tit->X1], tit->Ev, Gen2StatesMap[*sit]);

  // set pResGen marked states corresponding to rGen2 marked states using Gen2StatesMap
  for(sit = rGen2.MarkedStatesBegin(); sit != rGen2.MarkedStatesEnd(); ++sit) {
    pResGen->SetMarkedState(Gen2StatesMap[*sit]);
  }

  // remove blocking states as they provide no useful meaning.
  pResGen->Trim();
  pResGen->Name("ConcatenateNonDet("+rGen1.Name()+","+rGen2.Name()+")");
    
  // if necessary, move pResGen to rResGen
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
    
}

// LanguageConcatenate(rGen1, rGen2, rResGen)
void LanguageConcatenate(const Generator& rGen1, const Generator& rGen2, 
			 Generator& rResGen) {

  FD_DF("LanguageConcatenate("<< rGen1.Name() 
	<< "," << rGen2.Name() << ")");
    
  // perform nondeterministic language concatenation
  LanguageConcatenateNonDet(rGen1, rGen2, rResGen);
    
  // make deterministic if necessary
  if(!(rResGen.IsDeterministic())){
    Deterministic(rResGen, rResGen);
  }

  // set name of result
  rResGen.Name("Concatenate("+rGen1.Name()+","+rGen2.Name()+")");
    
  return;    
}

// FullLanguage(rAlphabet, rResGen)
void FullLanguage(const EventSet& rAlphabet, Generator& rResGen) {
  FD_DF("FullLanguage("<< rAlphabet.Name() 
	<< "," << rResGen.Name() << ")");
	
  // prepare result
  rResGen.Clear();
	
  // helpers
  Idx state;
  EventSet::Iterator evit;
	
  // alphabet
  rResGen.InjectAlphabet(rAlphabet);
	
  // insert marked initial state
  if(rResGen.StateNamesEnabled()){
    state = rResGen.InsInitState("1");
  } else{
    state = rResGen.InsInitState();
  }
  rResGen.SetMarkedState(state);
	
  // create selfloop for each event
  for (evit = rAlphabet.Begin(); evit != rAlphabet.End(); ++evit) {
    rResGen.SetTransition(state, *evit, state);
  }
	
  // set name of result
  rResGen.Name("FullLanguage("+rAlphabet.Name()+")");
    
  return;
}

// AlphabetLanguage(rAlphabet, rResGen)
void AlphabetLanguage(const EventSet& rAlphabet, Generator& rResGen) {
  FD_DF("AlphabetLanguage("<< rAlphabet.Name() 
	<< "," << rResGen.Name() << ")");
	
  // prepare result
  rResGen.Clear();
    	
  // set name of result
  rResGen.Name("AlphabetLanguage("+rAlphabet.Name()+")");
    
  // if alphabet is empty, leave generator empty
  if(rAlphabet.Empty()){
    FD_WARN("AlphabetLanguage: empty alphabet.");
    return;
  }
    
  // helpers
  Idx istate, mstate;
  EventSet::Iterator evit;
	
  // alphabet
  rResGen.InjectAlphabet(rAlphabet);
	
  // insert one initial state and one marked state
  if(rResGen.StateNamesEnabled()){
    istate = rResGen.InsInitState("1");
    mstate = rResGen.InsMarkedState("2");
  }
  else{
    istate = rResGen.InsInitState();
    mstate = rResGen.InsMarkedState();
  }
	
  // for each event from rAlphabet, inserted transition leading from init state to marked state
  for (evit = rAlphabet.Begin(); evit != rAlphabet.End(); ++evit) {
    rResGen.SetTransition(istate, *evit, mstate);
  }
    
  return;
}

// EmptyStringLanguage(rAlphabet, rResGen)
void EmptyStringLanguage(const EventSet& rAlphabet, Generator& rResGen) {
  FD_DF("EmptyStringLanguage("<< rAlphabet.Name() 
	<< "," << rResGen.Name() << ")");
	
  // prepare result
  rResGen.Clear();
	
  // helpers
  Idx state;
	
  // alphabet
  rResGen.InjectAlphabet(rAlphabet);
	
  // insert marked initial state
  if(rResGen.StateNamesEnabled()){
    state = rResGen.InsInitState("1");
  }
  else{
    state = rResGen.InsInitState();
  }
  rResGen.SetMarkedState(state);
	
  // set name of result
  rResGen.Name("EmptyStringLanguage("+rAlphabet.Name()+")");
    
  return;
}

// EmptyLanguage(rAlphabet, rResGen)
void EmptyLanguage(const EventSet& rAlphabet, Generator& rResGen) {
  FD_DF("EmptyStringLanguage("<< rAlphabet.Name() 
	<< "," << rResGen.Name() << ")");
	
  // prepare result
  rResGen.Clear();
	
  // set alphabet
  rResGen.InjectAlphabet(rAlphabet);

  // set name of result
  rResGen.Name("EmptyLanguage("+rAlphabet.Name()+")");
    
  return;
}

// EmptyLanguage(rGen)
bool IsEmptyLanguage(const Generator& rGen) {
  // case a) check if set of marked or initial states is empty
  if(rGen.MarkedStatesSize()==0) return true;
  if(rGen.InitStatesSize()==0) return true;
  // case b) check if no marked state is accessible (reachable)
  return (rGen.AccessibleSet()*rGen.MarkedStates()).Empty();
}

// LanguageInclusion(rGen1, rGen2)
bool LanguageInclusion(const Generator& rGen1, const Generator& rGen2) {
    
  FD_DF("LanguageInclusion("<< rGen1.Name() << "," << rGen2.Name() << ")"); 
    
  // check if there is no string in Lm1 that is not in Lm2, which means Lm1<=Lm2
  Generator NotrGen2=rGen2;
  NotrGen2.StateNamesEnabled(false);
  // note: complement w.r.t. union of alphabets to ensure that elementwise 
  // inclusion of Lm1 in Lm2 is tested
  LanguageComplement(NotrGen2 , rGen1.Alphabet()+rGen2.Alphabet());
  return EmptyLanguageIntersection(rGen1,NotrGen2);        
}

// LanguageEquality(rGen1, rGen2)
bool LanguageEquality(const Generator& rGen1, const Generator& rGen2) {
    
  FD_DF("LanguageEquality("<< rGen1.Name() << "," << rGen2.Name() << ")");

  // Check for equality by testing mutual inclusion
  return LanguageInclusion(rGen1,rGen2) && LanguageInclusion(rGen2,rGen1);
}

// KleeneClosure(rGen)
void KleeneClosure(Generator& rGen) {
   
  FD_DF("KleeneClosure("<< rGen.Name() << ")");

  // fix name 
  std::string name=CollapsString("KleeneClosure(" + rGen.Name() + ")");

  // The Kleene Closure of the empty set is the empty set
  if(IsEmptyLanguage(rGen)){
    rGen.Clear();
    rGen.Name(name);
    return;
  }

  // run nondet version    
  KleeneClosureNonDet(rGen);
  Deterministic(rGen, rGen);

  // set name
  rGen.Name(name);
}

// KleeneClosure(rGen, rResGen)
void KleeneClosure(const Generator& rGen, Generator& rResGen) {
   
  FD_DF("KleeneClosure("<< rGen.Name() << ", ... )");

  // if arg and result match, call respective version
  if(&rGen==&rResGen) {
    KleeneClosure(rResGen);
    return;
  }

  // fix name 
  std::string name=CollapsString("KleeneClosure(" + rGen.Name() + ")");

  // The Kleene Closure of the empty set is the empty set
  if(IsEmptyLanguage(rGen)){
    rResGen.Clear();
    rResGen.Name(name);
    return;
  }

  // run nondet version with intermediate result
  Generator* pgen=rGen.Copy();   
  KleeneClosureNonDet(*pgen);
  Deterministic(*pgen, rResGen);
  delete pgen;

  // set name
  rResGen.Name(name);
}

// KleeneClosureNonDet(rGen)
void KleeneClosureNonDet(Generator& rGen) {
    
  FD_DF("KleeneClosureNonDet("<< rGen.Name()  << ")");
    
  // set name 
  rGen.Name(CollapsString("KleeneClosureNonDet("+ rGen.Name() + ")"));

  // make accessible (relevant)
  rGen.Accessible();

  // test for empty language
  if(rGen.MarkedStatesSize()==0) {
    rGen.Clear();
    return;
  }
    
  // helpers
  TransSet::Iterator tit;
  TransSet TransToInsert;

  // initial state bug (detected by Tomas Masopust, fixes proposed by Klaus Schmidt and Florian Frohn)
  // 1. prepare the generator to have a unique initial state
  // 2. if the initial state fails to be marked, introduce an alternative marked initial state.
  // 3. equip the markded initial state with the same transitions as the original initial state
  UniqueInit(rGen);
  Idx istate = *rGen.InitStatesBegin();
  Idx imstate = istate;
  if(!rGen.ExistsMarkedState(imstate)) {
    imstate=rGen.InsInitState();
    rGen.SetMarkedState(imstate);
    for(tit = rGen.TransRelBegin(istate); tit != rGen.TransRelEnd(istate); ++tit) {	
      TransToInsert.Insert(imstate, tit->Ev, tit->X2);
    }
    for (tit = TransToInsert.Begin(); tit != TransToInsert.End(); ++tit) {
      rGen.SetTransition(*tit);
    }
    TransToInsert.Clear();
    rGen.ClrInitState(istate);
  }
  rGen.Accessible(); // cosmetic

      
  // for all transitions leading from a state x1 to a marked state: insert a transition
  // with the same event that leads to the unique marked initial state. 
  for(tit = rGen.TransRelBegin(); tit != rGen.TransRelEnd(); ++tit) {	
    if(rGen.ExistsMarkedState(tit->X2)) {
      if(!(rGen.ExistsTransition(tit->X1, tit->Ev, imstate)) ){
	TransToInsert.Insert(tit->X1, tit->Ev, imstate);
      }
    }        
  }
  for (tit = TransToInsert.Begin(); tit != TransToInsert.End(); ++tit) {
    rGen.SetTransition(*tit);
  }
    
}

// PrefixClosure(rGen)
void PrefixClosure(Generator& rGen) {
    
  FD_DF("PrefixClosure("<< rGen.Name() << ")");

  // fix name
  std::string name=CollapsString("PrefixClosure("+ rGen.Name() + ")");
    
  // remove all states that do net represent prefixes of marked strings
  rGen.Coaccessible();
    
  // mark all remaining states
  rGen.InjectMarkedStates(rGen.States());
   
  // set name 
  rGen.Name(name);
    
}

// IsPrefixClosed
bool IsPrefixClosed(const Generator& rGen) {
  
  // figure relevant states
  StateSet relevant = rGen.AccessibleSet() * rGen.CoaccessibleSet();

  // test
  return relevant <= rGen.MarkedStates();

}

// IsNonblocking
bool IsNonblocking(const Generator& rGen) {
  
  // test
  return rGen.AccessibleSet() <= rGen.CoaccessibleSet();

}

// IsNonblocking 
bool IsNonblocking(const Generator& rGen1, const Generator& rGen2) {
  
  // build composition
  Generator parallel;
  parallel.StateNamesEnabled(false);
  Parallel(rGen1,rGen2,parallel);

  // test (parallel returns an accessible generator).
  return parallel.States() <= parallel.CoaccessibleSet();

}



// SelfLoop(rGen,rAlphabet)
void SelfLoop(Generator& rGen,const EventSet& rAlphabet) {
    
  FD_DF("SelfLoop(" << rGen.Name() << "," << rAlphabet.Name() << ")");

  // fix name
  std::string name = CollapsString("SelfLoop(" + rGen.Name() + "," + rAlphabet.Name() + ")");
  // extend alphabet of rGen
  rGen.InjectAlphabet(rGen.Alphabet()+rAlphabet);
    
  //helpers
  EventSet::Iterator evit,evbegin,evend;
  evbegin = rAlphabet.Begin();
  evend = rAlphabet.End();  
  StateSet::Iterator sit;
    
  // iterate over all states and insert selfloop for each event of rAlphabet
  for (sit = rGen.StatesBegin(); sit != rGen.StatesEnd(); ++sit) {
    for(evit = evbegin; evit != evend; ++evit){
      rGen.SetTransition(*sit, *evit, *sit);
    }
  }
    
  // set name
  rGen.Name(name);
}

// SelfLoopMarkedStates(rGen,rAlphabet)
void SelfLoopMarkedStates(Generator& rGen,const EventSet& rAlphabet) {
    
  FD_DF("SelfLoopMarkedStates(" << rGen.Name() << "," << rAlphabet.Name() << ")");

  // fix name
  std::string name = CollapsString("SelfLoopMarkedStates(" + rGen.Name() 
     + "," + rAlphabet.Name() + ")");

  // extend alphabet of rGen
  rGen.InjectAlphabet(rGen.Alphabet()+rAlphabet);
    
  //helpers
  EventSet::Iterator evit,evbegin,evend;
  evbegin = rAlphabet.Begin();
  evend = rAlphabet.End();  
  StateSet::Iterator sit;
    
  // iterate over all marked states and insert selfloop for each event of rAlphabet
  for (sit = rGen.MarkedStatesBegin(); sit != rGen.MarkedStatesEnd(); ++sit) {
    for(evit = evbegin; evit != evend; ++evit){
      rGen.SetTransition(*sit, *evit, *sit);
    }
  }
    
  // set name
  rGen.Name(name);
}

// SelfLoop(rGen,rAlphabet,rStates)
void SelfLoop(Generator& rGen,const EventSet& rAlphabet,const StateSet& rStates) {
    
  FD_DF("SelfLoop(" << rGen.Name() << "," << rAlphabet.Name() << "," << rStates.Name() << ")");

  // fix name
  std::string name = CollapsString("SelfLoop(" + rGen.Name() 
     + "," + rAlphabet.Name() + "," + rStates.Name() + ")");

  // exception: rStates must be states of rGen
#ifdef FAUDES_CHECKED
  if( !(rStates <= rGen.States()) ){
    std::stringstream errstr;
    errstr << "State set " << rStates.Name() << 
      " has to be included in state set of "<< rGen.Name() << ".";
    throw Exception("SelfLoop()", errstr.str(), 100);
  }
#endif
    
  // extend alphabet of rGen
  rGen.InjectAlphabet(rGen.Alphabet()+rAlphabet);
    
  //helpers
  EventSet::Iterator evit,evbegin,evend;
  evbegin = rAlphabet.Begin();
  evend = rAlphabet.End();    
  StateSet::Iterator sit;
    
  // iterate over all marked states and insert selfloop for each event of rAlphabet
  for (sit = rStates.Begin(); sit != rStates.End(); ++sit) {
    for(evit = evbegin; evit != evend; ++evit){
      rGen.SetTransition(*sit, *evit, *sit);
    }
  }
    
  // set name
  rGen.Name(name);
}


} // namespace faudes

#undef Product //see define above for comment
