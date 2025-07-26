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


// Rabin-Buechi product (lifting individual acceptence conditions, generated languages not affected
// if arguments are full))
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
  // std product (this will make res->Alphabet the union of the arguments)
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
  // be attribute aware
  bool careattr=rRAut.Alphabet().EqualAttributes(rBAut.Alphabet());
  if(careattr) {
    pRes->EventAttributes(rRAut.Alphabet());
  }
  // copy result
  if(pRes != &rRes) {
    pRes->Move(rRes);
    delete pRes;
  }
}


// helper class for omega compositions
class RPState {
public:
  // minimal interface
  RPState() {};
  RPState(const Idx& rq1, const Idx& rq2, const bool& rf) :
    q1(rq1), q2(rq2), m1required(rf), mresolved(false) {};
  std::string Str(void) { return ToStringInteger(q1)+"|"+
      ToStringInteger(q2)+"|"+ToStringInteger(m1required); };
  // order
  bool operator < (const RPState& other) const {
    if (q1 < other.q1) return(true);
    if (q1 > other.q1) return(false);
    if (q2 < other.q2) return(true);
    if (q2 > other.q2) return(false);
    if (!m1required && other.m1required)  return(true);
    if (m1required &&  !other.m1required) return(false);
    if (!mresolved &&  other.mresolved)  return(true);
    return(false);
  }
  // member variables
  Idx q1;
  Idx q2;
  bool m1required;
  bool mresolved;
};


// product of rabin and buechi automata, langugae intersection (only one rabin pair) 
void RabinBuechiProduct(const RabinAutomaton& rRAut, const Generator& rBAut, RabinAutomaton& rRes) {
  // we can only handle one Rabin pair
  if(rRAut.RabinAcceptance().Size()!=1){
    std::stringstream errstr;
    errstr << "the current implementation requires exactly one Rabin pair";
    throw Exception("RabinBuechiProduct", errstr.str(), 80);
  }

  // prepare result
  RabinAutomaton* pRes = &rRes;
  if(&rRes== &rRAut || &rRes== &rBAut) {
    pRes= rRes.New();
  }
  pRes->Clear();
  pRes->Name(CollapsString(rRAut.Name()+".x."+rBAut.Name()));

  // create res alphabet
  pRes->InsEvents(rRAut.Alphabet());
  pRes->InsEvents(rBAut.Alphabet());

  // pick the one Rabin pair we care and prepare result
  RabinPair rpair= *(rRAut.RabinAcceptance().Begin());
  rpair.RSet().RestrictSet(rpair.ISet());
  pRes->RabinAcceptance().Size(1);
  RabinPair& resrpair= *(pRes->RabinAcceptance().Begin());
  resrpair.Clear();
  
  // reverse composition map
  std::map< RPState, Idx> reverseCompositionMap;
  // todo stack
  std::stack< RPState > todo;
  // current pair, new pair
  RPState currentstates, newstates;
  // state
  Idx tmpstate;  
  StateSet::Iterator lit1, lit2;
  TransSet::Iterator tit1, tit1_end, tit2, tit2_end;
  std::map< RPState, Idx>::iterator rcit;
  // push all combinations of initial states on todo stack
  FD_DF("RabinBuechiProduct: push initial states to todo:");
  for(lit1 = rRAut.InitStatesBegin(); lit1 != rRAut.InitStatesEnd(); ++lit1) {
    for(lit2 = rBAut.InitStatesBegin(); lit2 != rBAut.InitStatesEnd(); ++lit2) {
      currentstates = RPState(*lit1, *lit2, true);
      todo.push(currentstates);
      Idx tmpstate=pRes->InsInitState();
      reverseCompositionMap[currentstates] = tmpstate;
      FD_DF("RabinRabinBuechiProduct:   " << currentstates.Str() << " -> " << tmpstate);
      // figure, whether this state should be in the invariant
      if(rpair.ISet().Exists(currentstates.q1))
        resrpair.ISet().Insert(tmpstate);
      // copy buechi marking
      if(rBAut.ExistsMarkedState(currentstates.q2))
	 pRes->SetMarkedState(tmpstate);
    }
  }

  // start algorithm
  FD_DF("RabinBuechiProduct: processing reachable states:");
  while (! todo.empty()) {
    // allow for user interrupt
    LoopCallback();
    // get next reachable state from todo stack
    currentstates = todo.top();
    todo.pop();
    FD_DF("RabinBuechiProduct: processing (" << currentstates.Str() << " -> " << reverseCompositionMap[currentstates]);
    // iterate over all rRAut transitions  
    tit1 = rRAut.TransRelBegin(currentstates.q1);
    tit1_end = rRAut.TransRelEnd(currentstates.q1);
    for(; tit1 != tit1_end; ++tit1) {
      // find transition in rBAut
      tit2 = rBAut.TransRelBegin(currentstates.q2, tit1->Ev);
      tit2_end = rBAut.TransRelEnd(currentstates.q2, tit1->Ev);
      for (; tit2 != tit2_end; ++tit2) {
        newstates = RPState(tit1->X2, tit2->X2,currentstates.m1required);
        // figure whether marking was resolved
        if(currentstates.m1required) {
  	  if(rpair.RSet().Exists(currentstates.q1))
	    newstates.m1required=false;
        } else {
  	  if(rpair.ISet().Exists(currentstates.q1))
  	  if(rBAut.ExistsMarkedState(currentstates.q2))
	    newstates.m1required=true;
        }
        // add to result and todo list if composition state is new
        rcit = reverseCompositionMap.find(newstates);
        if(rcit == reverseCompositionMap.end()) {
          todo.push(newstates);
          tmpstate = pRes->InsState();
	  // figure, whether this state should be recurrent
          if(!newstates.m1required)
	    if(rBAut.ExistsMarkedState(newstates.q2))
              resrpair.RSet().Insert(tmpstate);
	  // figure, whether this state should be invariant
	  if(rpair.ISet().Exists(newstates.q1))
              resrpair.ISet().Insert(tmpstate);
	  // copy buechi marking
  	  if(rBAut.ExistsMarkedState(newstates.q2))
	    pRes->SetMarkedState(tmpstate);
	  // record new state
          reverseCompositionMap[newstates] = tmpstate;
          FD_DF("RabinBuechiProduct:   todo push: (" << newstates.Str() << ") -> " << reverseCompositionMap[newstates]);
        }
        else {
          tmpstate = rcit->second;
        }
        pRes->SetTransition(reverseCompositionMap[currentstates], 
            tit1->Ev, tmpstate);
        FD_DF("RabinBuechiProduct:  add transition to new generator: " << 
	      pRes->TStr(Transition(reverseCompositionMap[currentstates], tit1->Ev, tmpstate)));
      }
    }
  }

  FD_DF("RabinBuechiProduct: recurrent states: "  << pRes->StatesToString(resrpair.RSet()));


  // fix statenames ...
  if(rRAut.StateNamesEnabled() && rBAut.StateNamesEnabled() && pRes->StateNamesEnabled()) 
  for(rcit=reverseCompositionMap.begin(); rcit!=reverseCompositionMap.end(); rcit++) {
    Idx x1=rcit->first.q1;
    Idx x2=rcit->first.q2;
    bool m1requ=rcit->first.m1required;
    Idx x12=rcit->second;
    if(!pRes->ExistsState(x12)) continue;
    std::string name1= rRAut.StateName(x1);
    if(name1=="") name1=ToStringInteger(x1);
    std::string name2= rBAut.StateName(x2);
    if(name2=="") name1=ToStringInteger(x2);
    std::string name12 = name1 + "|" + name2;
    if(m1requ) name12 += "|r1m";
    else name12 +="|r2m";
    name12=pRes->UniqueStateName(name12);
    pRes->StateName(x12,name12);
  }

  // .. or clear them (?)
  if(!(rRAut.StateNamesEnabled() && rBAut.StateNamesEnabled() && pRes->StateNamesEnabled())) 
    pRes->StateNamesEnabled(false);

  // be attribute aware
  bool careattr=rRAut.Alphabet().EqualAttributes(rBAut.Alphabet());
  if(careattr) {
    pRes->EventAttributes(rRAut.Alphabet());
  }

  // copy result
  if(pRes != &rRes) {
    pRes->Move(rRes);
    delete pRes;
  }
}



} // namespace faudes

