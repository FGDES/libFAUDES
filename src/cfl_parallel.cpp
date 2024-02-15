/** @file cfl_parallel.cpp parallel composition */

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


#include "cfl_parallel.h"

/* turn on debugging for this file */
//#undef FD_DF
//#define FD_DF(a) FD_WARN(a);

namespace faudes {

// Parallel(rGen1, rGen2, res)
void Parallel(const Generator& rGen1, const Generator& rGen2, Generator& rResGen) {
  // helpers:
  std::map< std::pair<Idx,Idx>, Idx> cmap;
  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen1 || &rResGen== &rGen2) {
    pResGen= rResGen.New();
  }
  // doit
  Parallel(rGen1, rGen2, cmap, *pResGen);
  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}
 

// Parallel for Generators, transparent for event attributes.
void aParallel(
  const Generator& rGen1,
  const Generator& rGen2,
  Generator& rResGen) 
{
  FD_DF("aParallel(...)");

  // inputs have to agree on attributes of shared events:
  bool careattr=rGen1.Alphabet().EqualAttributes(rGen2.Alphabet());

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen1 || &rResGen== &rGen2) {
    pResGen= rResGen.New();
  }

  // make product composition of inputs
  Parallel(rGen1,rGen2,*pResGen);

  // copy all attributes of input alphabets
  if(careattr) {
    pResGen->EventAttributes(rGen1.Alphabet());
    pResGen->EventAttributes(rGen2.Alphabet());
  }

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

  FD_DF("aParallel(...): done");
}


// Parallel for multiple Generators, transparent for event attributes.
void aParallel(
  const GeneratorVector& rGenVec,
  Generator& rResGen) 
{

  // inputs have to agree on attributes of pairwise shared events:
  bool careattr=true;
  for(GeneratorVector::Position i=0; i<rGenVec.Size(); i++)
    for(GeneratorVector::Position j=0; j<i; j++) 
      if(!rGenVec.At(i).Alphabet().EqualAttributes(rGenVec.At(j).Alphabet()))
        careattr=false;

  // ignore empty
  if(rGenVec.Size()==0) {
    return;
  }

  // copy one 
  if(rGenVec.Size()==1) {
    rResGen=rGenVec.At(0);
    return;
  }

  // run parallel 
  Parallel(rGenVec.At(0),rGenVec.At(1),rResGen);
  for(GeneratorVector::Position i=2; i<rGenVec.Size(); i++) 
    Parallel(rGenVec.At(i),rResGen,rResGen);

  // fix alphabet
  if(careattr) {
    for(GeneratorVector::Position i=0; i<rGenVec.Size(); i++) 
      rResGen.EventAttributes(rGenVec.At(i).Alphabet());
  }

}


// aParallel(rGen1, rGen2, rCompositionMap, res)
void aParallel(
  const Generator& rGen1, 
  const Generator& rGen2, 
  ProductCompositionMap& rCompositionMap, 
  Generator& rResGen) 
{
  
  // inputs have to agree on attributes of shared events:
  bool careattr=rGen1.Alphabet().EqualAttributes(rGen2.Alphabet());

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen1 || &rResGen== &rGen2) {
    pResGen= rResGen.New();
  }

  // make product composition of inputs
  Parallel(rGen1,rGen2,rCompositionMap.StlMap(),*pResGen);

  // copy all attributes of input alphabets
  if(careattr) {
    pResGen->EventAttributes(rGen1.Alphabet());
    pResGen->EventAttributes(rGen2.Alphabet());
  }

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

}

// Parallel(rGen1, rGen2, rCompositionMap, res)
void Parallel(
  const Generator& rGen1, 
  const Generator& rGen2, 
  ProductCompositionMap& rCompositionMap, 
  Generator& rResGen) 
{
  // make product composition of inputs
  Parallel(rGen1,rGen2,rCompositionMap.StlMap(),rResGen);
}

// Parallel(rGen1, rGen2, rCompositionMap, mark1, mark2, res)
void Parallel(
  const Generator& rGen1, const Generator& rGen2, 
  ProductCompositionMap& rCompositionMap, 
  StateSet& rMark1,
  StateSet& rMark2,
  Generator& rResGen)
{

  // do the composition
  Parallel(rGen1,rGen2,rCompositionMap,rResGen);

  // clear marking
  rMark1.Clear();
  rMark2.Clear();

  /*
  see tmoor 20110208

  // catch special cases: a
  if(rGen1.AlphabetSize()==0) {
    rMark2=rGen2.MarkedStates();
    return;
  }

  // catch special cases: b
  if(rGen2.AlphabetSize()==0) {
    rMark1=rGen1.MarkedStates();
    return;
  }
  */

  // retrieve marking from reverse composition map
  StateSet::Iterator sit;
  for(sit=rResGen.StatesBegin(); sit!=rResGen.StatesEnd(); ++sit) {
    Idx s1=rCompositionMap.Arg1State(*sit);
    Idx s2=rCompositionMap.Arg2State(*sit);
    if(rGen1.ExistsMarkedState(s1)) rMark1.Insert(*sit);
    if(rGen2.ExistsMarkedState(s2)) rMark1.Insert(*sit);
  }
}


// Parallel(rGen1, rGen2, rCompositionMap, res)
void Parallel(
  const Generator& rGen1, const Generator& rGen2, 
  std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap, 
  Generator& rResGen)
{
  FD_DF("Parallel(" << &rGen1 << "," << &rGen2 << ")");

  /*
  re-consider the special cases:

  if Sigma_1=0, we have either 
    -- L_res=L_2  (if L_1!=0) 
    -- L_res=0    (if L_1==0)
 
  the below special cases do not handle this correct,
  nor do they setup the composition map; thus, we drop
  the special cases;   tmoor 20110208
  */

  /*
  // special case: empty alphabet
  if(rGen1.AlphabetSize()==0) {
    rResGen=rGen2;
    rResGen.Name(rGen2.Name());
    return;
  }

  // special case: empty alphabet
  if(rGen2.AlphabetSize()==0) {
    rResGen=rGen1;
    rResGen.Name(rGen1.Name());
    return;
  }
  */

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen1 || &rResGen== &rGen2) {
    pResGen= rResGen.New();
  }
  pResGen->Clear();
  pResGen->Name(CollapsString(rGen1.Name()+"||"+rGen2.Name()));
  rCompositionMap.clear();

  // create res alphabet
  EventSet::Iterator eit;
  for (eit = rGen1.AlphabetBegin(); eit != rGen1.AlphabetEnd(); ++eit) {
    pResGen->InsEvent(*eit);
  }
  for (eit = rGen2.AlphabetBegin(); eit != rGen2.AlphabetEnd(); ++eit) {
    pResGen->InsEvent(*eit);
  }
  FD_DF("Parallel: inserted indices in rResGen.alphabet( "
      << pResGen->AlphabetToString() << ")");

  // shared events
  EventSet sharedalphabet = rGen1.Alphabet() * rGen2.Alphabet();
  FD_DF("Parallel: shared events: " << sharedalphabet.ToString());

  // todo stack
  std::stack< std::pair<Idx,Idx> > todo;
  // current pair, new pair
  std::pair<Idx,Idx> currentstates, newstates;
  // state
  Idx tmpstate;
  StateSet::Iterator lit1,lit2;
  TransSet::Iterator tit1, tit1_end, tit2, tit2_end;
  std::map< std::pair<Idx,Idx>, Idx>::iterator rcit;

  // push all combinations of initial states on todo stack
  FD_DF("Parallel: adding all combinations of initial states to todo:");
  for (lit1 = rGen1.InitStatesBegin(); lit1 != rGen1.InitStatesEnd(); ++lit1) {
    for (lit2 = rGen2.InitStatesBegin(); lit2 != rGen2.InitStatesEnd(); ++lit2) {
      currentstates = std::make_pair(*lit1, *lit2);
      todo.push(currentstates);
      tmpstate = pResGen->InsInitState();
      rCompositionMap[currentstates] = tmpstate;
      FD_DF("Parallel:   (" << *lit1 << "|" << *lit2 << ") -> " 
          << rCompositionMap[currentstates]);
    }
  }

  // start algorithm
  FD_DF("Parallel: processing reachable states:");
  while (! todo.empty()) {
    // allow for user interrupt
    // LoopCallback();
    // allow for user interrupt, incl progress report
    FD_WPC(rCompositionMap.size(),rCompositionMap.size()+todo.size(),"Parallel(): processing"); 
    // get next reachable state from todo stack
    currentstates = todo.top();
    todo.pop();
    FD_DF("Parallel: processing (" << currentstates.first << "|" 
        << currentstates.second << ") -> " 
        << rCompositionMap[currentstates]);
    // iterate over all rGen1 transitions 
    // (includes execution of shared events)
    tit1 = rGen1.TransRelBegin(currentstates.first);
    tit1_end = rGen1.TransRelEnd(currentstates.first);
    for (; tit1 != tit1_end; ++tit1) {
      // if event not shared
      if (! sharedalphabet.Exists(tit1->Ev)) {
        FD_DF("Parallel:   exists only in rGen1");
        newstates = std::make_pair(tit1->X2, currentstates.second);
        // add to todo list if composition state is new
        rcit = rCompositionMap.find(newstates);
        if (rcit == rCompositionMap.end()) {
          todo.push(newstates);
          tmpstate = pResGen->InsState();
          rCompositionMap[newstates] = tmpstate;
          FD_DF("Parallel:   todo push: (" << newstates.first << "|" 
              << newstates.second << ") -> " 
              << rCompositionMap[newstates]);
        }
        else {
          tmpstate = rcit->second;
        }
        pResGen->SetTransition(rCompositionMap[currentstates], tit1->Ev, tmpstate);
        FD_DF("Parallel:   add transition to new generator: " 
            << rCompositionMap[currentstates] << "-" << tit1->Ev << "-" 
            << tmpstate);
      }
      // if shared event
      else {
        FD_DF("Parallel:   common event");
        // find shared transitions
        tit2 = rGen2.TransRelBegin(currentstates.second, tit1->Ev);
        tit2_end = rGen2.TransRelEnd(currentstates.second, tit1->Ev);
        for (; tit2 != tit2_end; ++tit2) {
          newstates = std::make_pair(tit1->X2, tit2->X2);
          // add to todo list if composition state is new
          rcit = rCompositionMap.find(newstates);
          if (rcit == rCompositionMap.end()) {
            todo.push(newstates);
            tmpstate = pResGen->InsState();
            rCompositionMap[newstates] = tmpstate;
            FD_DF("Parallel:   todo push: (" << newstates.first << "|" 
                << newstates.second << ") -> " 
                << rCompositionMap[newstates]);
          }
          else {
            tmpstate = rcit->second;
          }
          pResGen->SetTransition(rCompositionMap[currentstates], 
              tit1->Ev, tmpstate);
          FD_DF("Parallel:   add transition to new generator: " 
              << rCompositionMap[currentstates] << "-" 
              << tit1->Ev << "-" << tmpstate);
        }
      }
    }
    // iterate over all rGen2 transitions 
    // (without execution of shared events)
    tit2 = rGen2.TransRelBegin(currentstates.second);
    tit2_end = rGen2.TransRelEnd(currentstates.second);
    for (; tit2 != tit2_end; ++tit2) {
      if (! sharedalphabet.Exists(tit2->Ev)) {
        FD_DF("Parallel:   exists only in rGen2");
        newstates = std::make_pair(currentstates.first, tit2->X2);
        // add to todo list if composition state is new
        rcit = rCompositionMap.find(newstates);
        if (rcit == rCompositionMap.end()) {
          todo.push(newstates);
          tmpstate = pResGen->InsState();
          rCompositionMap[newstates] = tmpstate;
          FD_DF("Parallel:   todo push: (" << newstates.first << "|" 
              << newstates.second << ") -> " 
              << rCompositionMap[newstates]);
        }
        else {
          tmpstate = rcit->second;
        }
        pResGen->SetTransition(rCompositionMap[currentstates], 
            tit2->Ev, tmpstate);
        FD_DF("Parallel:   add transition to new generator: " 
            << rCompositionMap[currentstates] << "-" 
            << tit2->Ev << "-" << tmpstate);
      }
    }
  }

  // set marked states
  rcit=rCompositionMap.begin();
  while(rcit!=rCompositionMap.end()) {    
    if(rGen1.ExistsMarkedState(rcit->first.first))
      if(rGen2.ExistsMarkedState(rcit->first.second))
        pResGen->SetMarkedState(rcit->second);
    ++rcit;
  }
  FD_DF("Parallel: marked states: " << pResGen->MarkedStatesToString());

  // copy result
  if(pResGen != &rResGen) {
    rResGen = *pResGen;
    delete pResGen;
  }
  // set statenames
  if(rGen1.StateNamesEnabled() && rGen2.StateNamesEnabled() && rResGen.StateNamesEnabled()) 
    SetComposedStateNames(rGen1, rGen2, rCompositionMap, rResGen); 
  else
    rResGen.StateNamesEnabled(false);
}


// Product(rGen1, rGen2, res)
void Product(const Generator& rGen1, const Generator& rGen2, Generator& rResGen) {
  std::map< std::pair<Idx,Idx>, Idx> cmap;
  // doit
  Product(rGen1, rGen2, cmap, rResGen);
}


// Product for Generators, transparent for event attributes.
void aProduct(
  const Generator& rGen1,
  const Generator& rGen2,
  Generator& rResGen) 
{

  // inputs have to agree on attributes of shared events:
  bool careattr=rGen1.Alphabet().EqualAttributes(rGen2.Alphabet());

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen1 || &rResGen== &rGen2) {
    pResGen= rResGen.New();
  }

  // make product composition of inputs
  Product(rGen1,rGen2,*pResGen);

  // copy all attributes of input alphabets
  if(careattr) {
    pResGen->EventAttributes(rGen1.Alphabet());
    pResGen->EventAttributes(rGen2.Alphabet());
  }

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

}


// aProduct(rGen1, rGen2, rCompositionMap, res)
void aProduct(
  const Generator& rGen1, 
  const Generator& rGen2, 
  ProductCompositionMap& rCompositionMap, 
  Generator& rResGen) 
{
  // inputs have to agree on attributes of shared events:
  bool careattr=rGen1.Alphabet().EqualAttributes(rGen2.Alphabet());

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen1 || &rResGen== &rGen2) {
    pResGen= rResGen.New();
  }

  // make product composition of inputs
  Product(rGen1,rGen2,rCompositionMap.StlMap(),*pResGen);

  // copy all attributes of input alphabets
  if(careattr) {
    pResGen->EventAttributes(rGen1.Alphabet());
    pResGen->EventAttributes(rGen2.Alphabet());
  }

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

}

// Product(rGen1, rGen2, rCompositionMap, mark1, mark2, res)
void Product(
  const Generator& rGen1, const Generator& rGen2, 
  std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap, 
  StateSet& rMark1,
  StateSet& rMark2,
  Generator& rResGen)
{

  // do the composition
  Product(rGen1,rGen2,rCompositionMap,rResGen);

  // clear marking
  rMark1.Clear();
  rMark2.Clear();

  // retrieve marking from reverse composition map
  std::map< std::pair<Idx,Idx>, Idx>::iterator rit;
  for(rit=rCompositionMap.begin(); rit!=rCompositionMap.end(); ++rit){
    if(rGen1.ExistsMarkedState(rit->first.first)) rMark1.Insert(rit->second);
    if(rGen2.ExistsMarkedState(rit->first.second)) rMark2.Insert(rit->second);
  }
}


// Product(rGen1, rGen2, rCompositionMap, res)
void Product(
  const Generator& rGen1, const Generator& rGen2, 
  std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap, 
  Generator& rResGen)
{
  FD_DF("Product(" << rGen1.Name() << "," << rGen2.Name() << ")");
  FD_DF("Product(): state counts " << rGen1.Size() << "/" << rGen2.Size());

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen1 || &rResGen== &rGen2) {
    pResGen= rResGen.New();
  }
  pResGen->Clear();
  rCompositionMap.clear();

  // shared alphabet
  pResGen->InjectAlphabet(rGen1.Alphabet() * rGen2.Alphabet());
  FD_DF("Product: shared alphabet: "
      << (rGen1.Alphabet() * rGen2.Alphabet()).ToString());

  // todo stack
  std::stack< std::pair<Idx,Idx> > todo;
  // current pair, new pair
  std::pair<Idx,Idx> currentstates, newstates;
  // state
  Idx tmpstate;
  
  StateSet::Iterator lit1, lit2;
  TransSet::Iterator tit1, tit1_end, tit2, tit2_end, tit2_begin;
  std::map< std::pair<Idx,Idx>, Idx>::iterator rcit;

  // push all combinations of initial states on todo stack
  FD_DF("Product: adding all combinations of initial states to todo:");
  for (lit1 = rGen1.InitStatesBegin(); 
      lit1 != rGen1.InitStatesEnd(); ++lit1) {
    for (lit2 = rGen2.InitStatesBegin(); 
        lit2 != rGen2.InitStatesEnd(); ++lit2) {
      currentstates = std::make_pair(*lit1, *lit2);
      todo.push(currentstates);
      rCompositionMap[currentstates] = pResGen->InsInitState();
      FD_DF("Product:   (" << *lit1 << "|" << *lit2 << ") -> " 
          << rCompositionMap[currentstates]);
    }
  }

  // start algorithm
  FD_DF("Product: processing reachable states:");
  while (! todo.empty()) {
    // allow for user interrupt
    // LoopCallback();
    // allow for user interrupt, incl progress report
    FD_WPC(rCompositionMap.size(),rCompositionMap.size()+todo.size(),"Product(): processing"); 
    // get next reachable state from todo stack
    currentstates = todo.top();
    todo.pop();
    FD_DF("Product: processing (" << currentstates.first << "|" 
        << currentstates.second << ") -> " << rCompositionMap[currentstates]);
    // iterate over all rGen1 and rGen2 transitions
    tit1 = rGen1.TransRelBegin(currentstates.first);
    tit1_end = rGen1.TransRelEnd(currentstates.first);
    tit2 = rGen2.TransRelBegin(currentstates.second);
    tit2_end = rGen2.TransRelEnd(currentstates.second);
    while((tit1 != tit1_end) && (tit2 != tit2_end)) {
      // sync event by tit1
      if(tit1->Ev < tit2->Ev) {
        ++tit1;
        continue;
      }
      // sync event by tit2
      if(tit1->Ev > tit2->Ev) {
        ++tit2;
        continue;
      }
      // shared event: iterate tit2
      tit2_begin = tit2;
      while(tit2 != tit2_end) {
        // break iteration
        if(tit1->Ev != tit2->Ev) break;
        // successor composition state
        newstates = std::make_pair(tit1->X2, tit2->X2);
        // add to todo list if composition state is new
        rcit = rCompositionMap.find(newstates);
        if(rcit == rCompositionMap.end()) {
          todo.push(newstates);
          tmpstate = pResGen->InsState();
          rCompositionMap[newstates] = tmpstate;
	  //if(tmpstate%1000==0)
          FD_DF("Product: todo push: (" << newstates.first << "|" 
		  << newstates.second << ") -> " << rCompositionMap[newstates] << " todo #" << todo.size());
        } else {
          tmpstate = rcit->second;
        }
        // set transition in result
        pResGen->SetTransition(rCompositionMap[currentstates], tit1->Ev, tmpstate);
        FD_DF("Product: add transition to new generator: " 
            << rCompositionMap[currentstates] << "-" << tit1->Ev << "-" << tmpstate);
        ++tit2;
      }
      // increment tit1 
      ++tit1;
      // reset tit2 (needed for non-deterministic case)
      if(tit1 != tit1_end)
        if(tit1->Ev == tit2_begin->Ev) 
          tit2=tit2_begin;
    }
  } // todo


  // set marked states (tmoor 2024: reorganised for performance)
  rcit=rCompositionMap.begin();
  while(rcit!=rCompositionMap.end()) {    
    if(rGen1.ExistsMarkedState(rcit->first.first))
      if(rGen2.ExistsMarkedState(rcit->first.second))
        pResGen->SetMarkedState(rcit->second);
    ++rcit;
  }
  FD_DF("Parallel: marked states: " << pResGen->MarkedStatesToString());

  // copy result (TODO: use move)
  if(pResGen != &rResGen) {
    rResGen = *pResGen;
    delete pResGen;
  }
  // set statenames
  if(rGen1.StateNamesEnabled() && rGen2.StateNamesEnabled() && rResGen.StateNamesEnabled()) 
    SetComposedStateNames(rGen1, rGen2, rCompositionMap, rResGen); 
  else
    rResGen.ClearStateNames();

  FD_DF("Product(...): done");
}



// SetParallelStateNames
void SetComposedStateNames(
  const Generator& rGen1, const Generator& rGen2, 
  const std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap, 
  Generator& rGen12)
{
  std::map< std::pair<Idx,Idx>, Idx>::const_iterator rcit;
  for(rcit=rCompositionMap.begin(); rcit!=rCompositionMap.end(); rcit++) {
    Idx x1=rcit->first.first;
    Idx x2=rcit->first.second;
    Idx x12=rcit->second;
    if(!rGen12.ExistsState(x12)) continue;
    std::string name1= rGen1.StateName(x1);
    if(name1=="") name1=ToStringInteger(x1);
    std::string name2= rGen2.StateName(x2);
    if(name2=="") name2=ToStringInteger(x2);
    std::string name12= name1 + "|" + name2;
    name12=rGen12.UniqueStateName(name12);
    rGen12.StateName(x12,name12);
  }
}


// CompositionMap1
void CompositionMap1(
  const std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap, 
  std::map<Idx,Idx>& rCompositionMap1)
{
  rCompositionMap1.clear();
  std::map< std::pair<Idx,Idx>, Idx>::const_iterator rcit;
  for(rcit=rCompositionMap.begin(); rcit!=rCompositionMap.end(); rcit++) 
    rCompositionMap1.insert(std::pair<Idx,Idx>(rcit->second,rcit->first.first));
}


// CompositionMap2
void CompositionMap2(
  const std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap, 
  std::map<Idx,Idx>& rCompositionMap2)
{
  rCompositionMap2.clear();
  std::map< std::pair<Idx,Idx>, Idx>::const_iterator rcit;
  for(rcit=rCompositionMap.begin(); rcit!=rCompositionMap.end(); rcit++) 
    rCompositionMap2.insert(std::pair<Idx,Idx>(rcit->second,rcit->first.second));
}


/**
 * Rti wrapper class implementation
 */

// std faudes type
FAUDES_TYPE_IMPLEMENTATION(ProductCompositionMap,ProductCompositionMap,Type)

// construct
ProductCompositionMap::ProductCompositionMap(void) : Type() { 
  mCompiled=true;
}

// construct
ProductCompositionMap::ProductCompositionMap(const ProductCompositionMap& rOther) : Type() {
  DoAssign(rOther);
}

// destruct
ProductCompositionMap::~ProductCompositionMap(void) {
}

// clear
void ProductCompositionMap::Clear(void) { 
  mCompositionMap.clear(); 
  mCompiled=true;
  mArg1Map.clear(); 
  mArg2Map.clear(); 
}

// assignment
void ProductCompositionMap::DoAssign(const ProductCompositionMap& rSrc) {
  mCompositionMap=rSrc.mCompositionMap;
  mCompiled=rSrc.mCompiled;
  mArg1Map=rSrc.mArg1Map; 
  mArg2Map=rSrc.mArg2Map;  
}

// equality
bool ProductCompositionMap::DoEqual(const ProductCompositionMap& rOther) const { 
  return mCompositionMap==rOther.mCompositionMap;
}

// C++/STL access
const std::map< std::pair<Idx,Idx> , Idx >& ProductCompositionMap::StlMap(void) const { 
  return mCompositionMap;
}

// C++/STL access
std::map< std::pair<Idx,Idx> , Idx >& ProductCompositionMap::StlMap(void) { 
  mCompiled=false;
  return mCompositionMap;
}

// C++/STL access
void ProductCompositionMap::StlMap(const std::map< std::pair<Idx,Idx> , Idx >& rMap) { 
  mCompositionMap=rMap;
  mCompiled=false;
}

// access
Idx ProductCompositionMap::CompState(Idx x1, Idx x2) const {
  std::pair<Idx,Idx> x12(x1,x2);
  std::map< std::pair<Idx,Idx> , Idx >::const_iterator x12it=mCompositionMap.find(x12);
  if(x12it==mCompositionMap.end()) return 0;
  return x12it->second;
}

// access
Idx ProductCompositionMap::Arg1State(Idx x1) const {
  if(!mCompiled) {
    mArg1Map.clear();
    mArg2Map.clear();
    std::map< std::pair<Idx,Idx>, Idx>::const_iterator rcit;
    for(rcit=mCompositionMap.begin(); rcit!=mCompositionMap.end(); rcit++) {
      mArg1Map.insert(std::pair<Idx,Idx>(rcit->second,rcit->first.first));
      mArg2Map.insert(std::pair<Idx,Idx>(rcit->second,rcit->first.second));
    }
    mCompiled=true;
  }
  std::map< Idx , Idx >::const_iterator x1it=mArg1Map.find(x1);
  if(x1it==mArg1Map.end()) return 0;
  return x1it->second;
}

// access
Idx ProductCompositionMap::Arg2State(Idx x2) const {
  if(!mCompiled) {
    mArg1Map.clear();
    mArg2Map.clear();
    std::map< std::pair<Idx,Idx>, Idx>::const_iterator rcit;
    for(rcit=mCompositionMap.begin(); rcit!=mCompositionMap.end(); rcit++) {
      mArg1Map.insert(std::pair<Idx,Idx>(rcit->second,rcit->first.first));
      mArg2Map.insert(std::pair<Idx,Idx>(rcit->second,rcit->first.second));
    }
    mCompiled=true;
  }
  std::map< Idx , Idx >::const_iterator x2it=mArg2Map.find(x2);
  if(x2it==mArg2Map.end()) return 0;
  return x2it->second;
}



} // name space
