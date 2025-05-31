/** @file omg_rabinfnct.cpp

Operations regarding omega languages accepted by Rabin automata

*/

/* FAU Discrete Event Systems Library (libFAUDES)

Copyright (C) 2025 Thomas Moor

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

  

#include "omg_rabinfnct.h"


namespace faudes {

// RabinLiveStates 
// compute states from a rabin pair that are not livelocks/deadlocks 
void RabinLiveStates(
  const TransSet& rTransRel,
  const TransSetX2EvX1& rRevTransRel,
  const RabinPair& rRPair,
  StateSet& rInv)
{
  // convenience accessors
  const StateSet& iset=rRPair.ISet();
  const StateSet& rset=rRPair.RSet();
  // initialise optimistic candidate of life states
  rInv=iset;
  // iterate for overall fixpoint
  bool fix=false;
  while(!fix) {
    // record size to detect change
    std::size_t invsz=rInv.Size();
    // nu-iteration to restrict rInv to an existential forward invariant
    bool nufix=false;
    StateSet nudel;
    while(!nufix) {
      nudel.Clear();
      StateSet::Iterator sit=rInv.Begin();
      StateSet::Iterator sit_end=rInv.End();
      while(sit!=sit_end) {
        if((rTransRel.SuccessorStates(*sit) * rInv).Empty())
          nudel.Insert(*sit);
        ++sit;
      }
      rInv.EraseSet(nudel);
      nufix=nudel.Empty();
    }
    // mu-iteration to obtain existential backward reach from rset
    StateSet breach=rInv*rset;
    StateSet todo=breach;
    bool mufix=false;
    StateSet muins;
    while(!mufix) {
      muins.Clear();
      StateSet::Iterator sit=todo.Begin();
      StateSet::Iterator sit_end=todo.End();
      while(sit!=sit_end) {
        muins.InsertSet(rRevTransRel.PredecessorStates(*sit));
        ++sit;
      }
      muins.RestrictSet(rInv);
      muins.EraseSet(breach);
      breach.InsertSet(muins);
      todo=muins;
      mufix=muins.Empty();	  
    }
    // restrict rInv to breach
    rInv=rInv*breach;
    // sense change
    if(invsz == rInv.Size()) fix=true;    
  }
  // one more mu-iteration to obtain existential backward reach from inv
  bool mufix=false;
  StateSet todo=rInv;
  StateSet muins;
  while(!mufix) {
    muins.Clear();
    StateSet::Iterator sit=todo.Begin();
    StateSet::Iterator sit_end=todo.End();
    while(sit!=sit_end) {
      muins.InsertSet(rRevTransRel.PredecessorStates(*sit));
      ++sit;
    }
    muins.EraseSet(rInv);
    rInv.InsertSet(muins);
    todo=muins;
    mufix=muins.Empty();	  
  }
  // done
}


// RabinLiveStates API wrapper
void RabinLiveStates(
  const vGenerator& rRAut,
  const RabinPair& rRPair,
  StateSet& rInv)
{
  // convenience accessor
  const TransSet& transrel=rRAut.TransRel();
  TransSetX2EvX1 revtransrel(transrel);
  // run algorithm
  RabinLiveStates(transrel,revtransrel,rRPair,rInv);
}


// RabinLiveStates API wrapper
void RabinLiveStates(const RabinAutomaton& rRAut, StateSet& rInv) {
  // convenience accessor
  const RabinAcceptance& raccept=rRAut.RabinAcceptance();
  const TransSet& transrel=rRAut.TransRel();
  TransSetX2EvX1 revtransrel(transrel);
  // pessimistic candidate for the trim set
  rInv.Clear();
  // iterate over Rabin pairs 
  StateSet inv;
  RabinAcceptance::CIterator rit=raccept.Begin();
  for(;rit!=raccept.End();++rit) {
    RabinLiveStates(transrel,revtransrel,*rit,inv);    
    rInv.InsertSet(inv);
  }  
}


// RabinTrimSet
// (returns the sets that conbtribute to the Rabin accepted omega-language)
void RabinTrimSet(const RabinAutomaton& rRAut, StateSet& rTrim) {
  // get live states
  RabinLiveStates(rRAut,rTrim);
  // only reachable states contribute
  rTrim.RestrictSet(rRAut.AccessibleSet());
}

// RabinTrim
// (return  True if result contains at least one initial state and at least one non-trivial Rabin pair)
bool RabinTrim(RabinAutomaton& rRAut) {
  // make the automaton accessible first
  rRAut.Accessible();
  // convenience accessor
  RabinAcceptance& raccept=rRAut.RabinAcceptance();
  const TransSet& transrel=rRAut.TransRel();
  const TransSetX2EvX1 revtransrel(transrel);
  // trim each Rabin pair to its  live states
  StateSet alive;
  StateSet plive;
  RabinAcceptance::Iterator rit=raccept.Begin();
  for(;rit!=raccept.End();++rit) {
    RabinLiveStates(transrel,revtransrel,*rit,plive);
    rit->RestrictStates(plive);
    alive.InsertSet(plive);
  }
  // remove obviously redundant (could do better here)
  raccept.EraseDoublets();
  // trim automaton to live states
  rRAut.RestrictStates(alive);
  // figure result
  bool res=true;
  if(!rRAut.InitStates().Empty()) res =false;
  return res;
}

// rti wrapper
bool RabinTrim(const RabinAutomaton& rRAut, RabinAutomaton& rRes) {
  rRes=rRAut;
  return RabinTrim(rRes);
}  

// Rabin-Buechi product
void RabinBuechiAutomaton(const RabinAutomaton& rRAut, const Generator& rBAut,  RabinAutomaton& rRes) {
  // prepare result
  RabinAutomaton* pRes = &rRes;
  if(&rRes== &rRAut || &rRes== &rBAut) {
    pRes= rRes.New();
  }
  // utils
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  std::map< std::pair<Idx,Idx>, Idx>::iterator cit;
  std::map< std::pair<Idx,Idx>, Idx> cmap;
  // transition product
  Product(rRAut,rBAut,cmap,*pRes);
  // set conversion rRAut-state to set of new states
  std::map< Idx , StateSet > rmap;
  cit=cmap.begin();;
  for(;cit!=cmap.end();++cit)
    rmap[cit->first.first].Insert(cit->second);
  // copy and fix Rabin acceptance
  pRes->RabinAcceptance().Clear();
  RabinAcceptance::CIterator rit=rRAut.RabinAcceptance().Begin();;
  RabinAcceptance::CIterator rit_end=rRAut.RabinAcceptance().End();;
  for(;rit!=rit_end;++rit) {
    RabinPair rpair;
    sit=rit->RSet().Begin();
    sit_end=rit->RSet().End();
    for(;sit!=sit_end;++sit) 
      rpair.RSet().InsertSet(rmap[*sit]);          
    sit=rit->ISet().Begin();
    sit_end=rit->ISet().End();
    for(;sit!=sit_end;++sit) 
      rpair.ISet().InsertSet(rmap[*sit]);
    pRes->RabinAcceptance().Insert(rpair);
  }
  // set conversion rBAut-state to set of new states
  std::map< Idx , StateSet > bmap;
  cit=cmap.begin();;
  for(;cit!=cmap.end();++cit)
    bmap[cit->first.second].Insert(cit->second);
  // set buechi marking
  sit=rBAut.MarkedStatesBegin();
  sit_end=rBAut.MarkedStatesEnd();
  for(;sit!=sit_end;++sit) 
    pRes->InsMarkedStates(bmap[*sit]);  
  // copy result
  if(pRes != &rRes) {
    pRes->Move(rRes);
    delete pRes;
  }
}
  
} // namespace faudes

