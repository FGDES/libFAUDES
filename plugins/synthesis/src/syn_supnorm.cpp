/** @file syn_supnorm.cpp Supremal normal sublanguage */

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
   

#include "syn_supnorm.h"
#include "syn_supcon.h"

/* turn on debugging for this file */
//#undef FD_DF
//#define FD_DF(a) FD_WARN(a);

namespace faudes {




/*
***************************************************************************************
***************************************************************************************
 Implementation
***************************************************************************************
***************************************************************************************
*/


//void NormalityConsistencyCheck(rL,rOAlph,rK) 
void NormalityConsistencyCheck(
  const Generator& rL,
  const EventSet& rOAlph,
  const Generator& rK) {
  
  FD_DF("NormalityConsistencyCheck(...)");
  
  if(!(rK.IsDeterministic())) {
    std::stringstream errstr;
    errstr << "Nondeterministic parameter rK.";
    if(!(rL.IsDeterministic())) errstr << "Nondeterministic parameter rL.";
    throw Exception("NormalityConsistencyCheck", errstr.str(), 101);
  }
  if(!(rL.IsDeterministic())) {
    std::stringstream errstr;
    errstr << "Nondeterministic parameter rL.";
    if(!(rL.IsDeterministic())) errstr << "Nondeterministic parameter rL.";
    throw Exception("NormalityConsistencyCheck", errstr.str(), 101);
  }
  
  EventSet Kevents,Sigma;
  Sigma=rL.Alphabet();
  Kevents=rK.Alphabet();
  
  // observable events have to be subset of Sigma
  if(!(rOAlph<=Sigma)) {
    EventSet only_in_OAlph = rOAlph - Sigma;
    std::stringstream errstr;
    errstr << "Not all observable events are contained in Sigma: " 
       << only_in_OAlph.ToString() << ".";
    throw Exception("NormalityConsistencyCheck", errstr.str(), 100);
  } 
	 
  // alphabets must match  
  if(Sigma != Kevents) {
    EventSet only_in_L = Sigma - Kevents;
    EventSet only_in_K = Kevents - Sigma;
    only_in_L.Name("Only_In_L");
    only_in_L.Name("Only_In_K");
    std::stringstream errstr;
    errstr << "Alphabets of generators do not match.";
    if(!only_in_L.Empty())
      errstr <<  " " << only_in_L.ToString() << ".";
    if(!only_in_K.Empty())
      errstr <<  " " << only_in_K.ToString() << ".";
    throw Exception("NormalityConsistencyCheck", errstr.str(), 100);
  }
  
  // K must be subset of L
  if(!LanguageInclusion(rK,rL)) {
    std::stringstream errstr;
    errstr << "K is not subset of L.";
    throw Exception("NormalityConsistencyCheck", errstr.str(), 0);
  }

  FD_DF("NormalityConsistencyCheck(...): passed");
}

// IsNormal(rK,rL,rOAlph)
bool IsNormal(
  const Generator& rL,
  const EventSet& rOAlph,
  const Generator& rK) 
{
  FD_DF("IsNormal(...)");

  // bail out on empty K
  // note: this is required to survive the
  // determinism test when rK has no states at all
  if(IsEmptyLanguage(rK)) return true;
  
  // determinism required
  NormalityConsistencyCheck(rL,rOAlph,rK);
  
  //extract overall alphabet:
  EventSet Sigma;
  Sigma=rL.Alphabet();
  
  //extract alphabet of rK:
  EventSet Kevents;
  Kevents=rK.Alphabet();
  
  //calculate p(K)
  Generator Ktest1, Ktest2;
  Ktest1.StateNamesEnabled(false);
  Ktest2.StateNamesEnabled(false);
  Project(rK,rOAlph,Ktest1);
  
  //calculate pinv(p(K))
  InvProject(Ktest1, Sigma);
  
  //check normality: pinv(p(K)) intersect L = K?
  LanguageIntersection(Ktest1,rL,Ktest2);
  return LanguageInclusion(Ktest2,rK);
	
  // Remark: testing for LanguageEquality is not required, as inclusion
  // in the reverse direction is always met (assuming K\subseteq L)
}



// IsNormal rti wrapper
bool IsNormal(const System& rPlantGen, const Generator& rSupCandGen) {
  return IsNormal(rPlantGen, rPlantGen.ObservableEvents(),rSupCandGen);
}


// ConcatenateFullLanguage(rGen)
void ConcatenateFullLanguage(Generator& rGen) {
  FD_DF("ConcatenateFullLanguage(" << rGen.Name() << ")");
  
  // prepare result
  rGen.Name("ConcatenateFullLanguage(" + rGen.Name() + ")");
  
  // treat trivial empty result in case of empty marked language
  if(rGen.MarkedStatesSize()==0) {
    rGen.Clear();
    return;
  }

  // treat trivial case if marked initial state
  if( ! (rGen.InitStates() * rGen.MarkedStates()).Empty() ) {
    FD_DF("ConcatenateFullLanguage(...): marked initial state");
    // reduce to empty string language
    rGen.ClearStates();
    rGen.ClearTransRel();
    Idx state=rGen.InsInitState();
    rGen.SetMarkedState(state);
    // now concatenate Sigma* by selflooping marked state with all events
    EventSet::Iterator eit;
    for(eit = rGen.AlphabetBegin(); eit != rGen.AlphabetEnd(); ++eit) 
      rGen.SetTransition(state,*eit,state);
    return;
  }
  
  // helpers
  EventSet alph=rGen.Alphabet();
  StateSet StatesToClear;
  TransSet TransToClear,TransToSet;
  StateSet::Iterator sit;
  TransSet::Iterator tit, tit_end;;
  EventSet::Iterator eit;
  
  
  // all marked states become eqivalent -> switch transitions leading to marked states to one remaining marked state
  // and delete all the others
  sit = rGen.MarkedStatesBegin();
  Idx marked = *sit;
  // clear all transitions the one  marked states
  tit = rGen.TransRelBegin(marked);
  tit_end = rGen.TransRelEnd(marked);
  while(tit != tit_end)
    rGen.ClrTransition(tit++);
  rGen.Accessible();
  FD_DF("ConcatenateFullLanguage(...): cleared transitions");

  
  // relink transitions to all other marked states if there are any
  if(rGen.MarkedStatesSize()>1) {
    // extract transitions sorted by target state X2
    TransSetX2EvX1 trel;
    rGen.TransRel(trel);
    TransSetX2EvX1::Iterator tit2, tit2_end;	  
    // switch transitions to all other marked states to the one remaining marked state
    ++sit;
    for(; sit != rGen.MarkedStatesEnd(); ++sit) {
       tit2 =     trel.BeginByX2(*sit);
       tit2_end = trel.EndByX2(*sit);
       for(; tit2 != tit2_end; ++tit2) 
	 rGen.SetTransition(tit2->X1,tit2->Ev,marked);
    }		  
    FD_DF("ConcatenateFullLanguage(...): relinked transitions");
    // delete all but the remaining marked state (note: by doing so, also corresp. transitions are cleared.)
    sit = rGen.MarkedStatesBegin();
    ++sit;
    for(; sit != rGen.MarkedStatesEnd(); ++sit) 
       StatesToClear.Insert(*sit);
    rGen.DelStates(StatesToClear);		  
    FD_DF("ConcatenateFullLanguage(...): removed #" << StatesToClear.Size() << " marked states");
  }
  // now concatenate Sigma* by selflooping marked state with all events
  for(eit = rGen.AlphabetBegin(); eit != rGen.AlphabetEnd(); ++eit) 
    rGen.SetTransition(marked,*eit,marked);
  
  FD_DF("ConcatenateFullLanguage(...): done");
}  


//  SupNorm(rL,rOAlph,rK,rResult)
bool SupNorm(
  const Generator& rL,
  const EventSet& rOAlph,
  const Generator& rK, 
  Generator& rResult)
{
  FD_DF("SupNorm(" << rL.Name() << "," << rK.Name() << "," << rOAlph.Name() << ")");
  FD_DF("SupNorm: sizeof L, K: "<< rL.Size() << ", " << rK.Size());
  
  // exceprions
  NormalityConsistencyCheck(rL,rOAlph,rK);


  //extract overall alphabet:
  EventSet allevents;
  allevents=rL.Alphabet();
      
  // involved operations from cfl_regular.h operate on the marked
  // languages -> turn generated languages into marked langusges
  Generator prL=rL;
  prL.InjectMarkedStates(rL.States());
  Generator prK=rK;
  prK.InjectMarkedStates(prK.States());
  
  // calculate "L-K"
  rResult.StateNamesEnabled(false);
  LanguageDifference(prL,prK,rResult);
  FD_DF("SupNorm: sizeof L-K: "<< rResult.Size());

  // statmin before projection for performance (?)
  //StateMin(rResult,rResult);
  //FD_DF("SupNorm: sizeof statemin(L-K): "<< rResult.Size());
 
  // calculate Pinv(P(L-K)):
  Project(rResult,rOAlph,rResult);
  FD_DF("SupNorm: sizeof p(L-K): "<< rResult.Size());
  InvProject(rResult,allevents);
  FD_DF("SupNorm: sizeof pinv(p(L-K)): "<< rResult.Size());

  //calculate remaining set difference -> supnorm(K)
  LanguageDifference(prK,rResult,rResult);
  FD_DF("SupNorm: sizeof K - pinv(p(L-K)): "<< rResult.Size());
  
  // cosmetics: remove blocking states
  rResult.Trim();
  FD_DF("SupNorm: sizeof trim(K - pinv(p(L-K))): "<< rResult.Size());
   
  // done
  rResult.Name("SupNorm("+rL.Name()+", "+rK.Name()+")");  
  return !( rResult.InitStatesEmpty() );
}

//  SupNormClosed(rL,rOAlph,rK,rResult)
bool SupNormClosed(
  const Generator& rL,
  const EventSet& rOAlph,
  const Generator& rK, 
  Generator& rResult)
{
  FD_DF("SupNormClosed(" << rL.Name() << "," << rK.Name() << "," << rOAlph.Name() << ")");
  
  // involved operations from regular.h operate on the marked
  // languages -> turn generated languages into marked langs
  Generator prL=rL;
  prL.InjectMarkedStates(prL.States());
  Generator prK=rK;
  prK.InjectMarkedStates(prK.States());

  // concitency check on closed languages
  NormalityConsistencyCheck(prL,rOAlph,prK);
  
  //extract overall alphabet:
  EventSet allevents;
  allevents=rL.Alphabet();
  
  // calculate "L-K" 
  LanguageDifference(prL,prK,rResult);

  // calculate Pinv(P(L-K)):
  Project(rResult,rOAlph,rResult);
  FD_DF("SupNormClosed: sizeof p(L-K): "<< rResult.Size());
  InvProject(rResult,allevents);
  FD_DF("SupNormClosed: sizeof pinv(p(L-K)): "<< rResult.Size());
  
  //concatenate Pinv(P(L-K)) with Sigma*: this leads to closed result
  ConcatenateFullLanguage(rResult);   
  FD_DF("SupNormClosed: sizeof pinv(p(L-K))Sigma*: "<< rResult.Size());

  //calculate remaining set difference -> supnormClosed(K)
  LanguageDifference(prK,rResult,rResult);

  // cosmetics: remove blocking states
  rResult.Trim();

  // done
  rResult.Name("SupNormClosed("+rL.Name()+", "+rK.Name()+")"); 
  FD_DF("SupNormClosed(" << rL.Name() << "," << rK.Name() << "," << rOAlph.Name() << "): done");
  return !( rResult.InitStatesEmpty() );
}


//  SupConNormClosed(rL,rCAlph,rOAlph,rK,rResult)
void SupConNormClosed(
  const Generator& rL,
  const EventSet& rCAlph,
  const EventSet& rOAlph,
  const Generator& rK, 
  Generator& rResult)
{
  FD_DF("SupConNormClosed(" << rL.Name() << "," << rK.Name() << ")");
  // determinism required
  ControlProblemConsistencyCheck(rL,rCAlph,rOAlph,rK);
  // 0.: intersect K with L to match requirements of SupNormClosed
  Generator K;
  K.StateNamesEnabled(false);
  Product(rL,rK,K);
  // 1. normal and closed sublanguage (operates on / returns generated language)
  Generator N;
  N.StateNamesEnabled(false);
  SupNormClosed(rL,rOAlph,K,N);
  // 2. project to sigma_o (generated languages)
  Generator N0,L0;
  N0.StateNamesEnabled(false);
  L0.StateNamesEnabled(false);
  Project(N,rOAlph,N0);
  Project(rL,rOAlph,L0);
  // 3. supremal controllable sublanguage (generated languages)
  EventSet sig_co = rCAlph * rOAlph;
  Generator K0;
  K0.StateNamesEnabled(false);
  SupConClosed(L0,sig_co,N0,K0);
  // 4. inverese project to sigma  (on generated language)
  InvProject(K0,rL.Alphabet());
  // 5. intersect with L (generated languages)
  LanguageIntersection(K0,rL,rResult);
  // convenience: mark the generated language
  rResult.InjectMarkedStates(rResult.States());
  rResult.Name("SupConNormClosed("+rL.Name()+", "+rK.Name()+")"); 
}  


//  SupConNorm(rL,rCAlph,rOAlph,rK,rResult)
void SupConNorm(
  const Generator& rL,
  const EventSet& rCAlph,
  const EventSet& rOAlph,
  const Generator& rK, 
  Generator& rResult)
{
  FD_DF("SupConNorm(" << rL.Name() << "," << rK.Name() << ")");
  // determinism required
  ControlProblemConsistencyCheck(rL,rCAlph,rOAlph,rK);
  // initialize: K0
  Generator K0;
  K0.StateNamesEnabled(false);
  Product(rL,rK,K0);
  K0.Coaccessible();
  // initialize: closure(rL)
  Generator L=rL;
  L.StateNamesEnabled(false);
  L.Trim();
  L.InjectMarkedStates(L.States());
  // loop
  Generator Ki=K0;
  Ki.StateNamesEnabled(false);
  while(1) {
    FD_DF("SupConNorm(" << rL.Name() << "," << rK.Name() << "): #" << Ki.Size() << " m#" << Ki.MarkedStatesSize());
    // keep copy of recent
    rResult=Ki;
    // cheep closure (for coreachable generator)
    Ki.InjectMarkedStates(Ki.States());
    // synthesise closed
    SupConNormClosed(L,rCAlph,rOAlph,Ki,Ki);
    // restrict
    Product(K0,Ki,Ki);
    Ki.Coaccessible();
    // test (sequence is decreasing anyway)
    if(LanguageInclusion(rResult,Ki)) break;
  }  
  rResult.Name("SupConNorm("+rL.Name()+", "+rK.Name()+")"); 
  FD_DF("SupConNorm(" << rL.Name() << "," << rK.Name() << "): done");
}  


// SupPrefixClosed(rK,rResult)
bool SupPrefixClosed(
  const Generator& rK,
	Generator& rResult) 
{
  
  FD_DF("SupPrefixClosed("<<rK.Name()<<")");
  
  // prepare Result:
  rResult.Name("SupPrefixClosed("+rK.Name()+")");
  
  // check for marked initial state, empty result if not
  if( (rK.InitStates() * rK.MarkedStates()).Empty() ) {
    rResult.Clear();
    return false;
  }

  rResult.Assign(rK);  
  
  // erase all transitions not leading to a marked state
  // todo: depth-first-search could be faster
  TransSet::Iterator tit=rResult.TransRelBegin();
  TransSet::Iterator tit_end=rResult.TransRelEnd();
  while(tit!=tit_end) {
    if(rResult.ExistsMarkedState(tit->X2)) { ++tit; continue;}
    rResult.ClrTransition(tit++);
  }

  // make reachable (cosmetic)
  rResult.Trim();
  
  // as there is at least one marked init state, result is nonempty
  return true;
}


// helper class
class SNOState {
public:
  // minimal interface
  SNOState() {};
  SNOState(const Idx& rq, const Idx& rx, const bool& rz) :
    q(rq), x(rx), z(rz) {};
  std::string Str(void) { return ToStringInteger(q)+"|"+
      ToStringInteger(x)+"|"+ToStringInteger(z); };
  // order
  bool operator < (const SNOState& other) const {
    if (q < other.q) return(true);
    if (q > other.q) return(false);
    if (x < other.x) return(true);
    if (x > other.x) return(false);
    if (z < other.z) return(true);
    return(false);
  }
  // member variables
  Idx q;
  Idx x;
  Idx z;
};


// SupConNormClosedUnchecked(rPlantGen, rCAlph, rOAlph, rObserverGen, rSupCandGen)
void SupConNormClosedUnchecked(
  const Generator& rPlantGen,  // aka G
  const EventSet& rCAlph,  
  const EventSet& rOAlph,  
  Generator& rObserverGen,     // aka Hobs
  Generator& rSupCandGen       // aka K
) 
{
  FD_DF("SupConNormClosedUnchecked(" << &rSupCandGen << "," << &rPlantGen << ")");

  // bail out on L(G)=0 --> closed-lopp language inclusion trivialy satisfied 
  if(rPlantGen.InitStatesEmpty()) return;

  // debugging: compare result with Lin-Brandt fromula at end of function
  // Generator K;
  // SupConNormClosed(rPlantGen, rCAlph, rOAlph, rSupCandGen, K);
 
  // loop until fixpoint
  while(true) {
    FD_DF("SupConNormClosedUnchecked(" << &rSupCandGen << "," << &rPlantGen << "): until fixpoint #" << rSupCandGen.Size());

    // record size to break loop
    Idx ssz = rSupCandGen.TransRelSize();
    Idx osz = rObserverGen.TransRelSize();

    // bail out if L(H)-0 --> closed-loop language fixpoint
    if(rSupCandGen.InitStatesEmpty()) break;

    // todo stack (state triplets)
    std::stack<SNOState> todo; 
    // relevant H states
    StateSet processed, critical;

    // various iterators
    TransSet::Iterator titg, titg_end;
    TransSet::Iterator tith, tith_end;
    TransSet::Iterator titho, titho_end;

    // current and successsor state
    SNOState current, successor;

    // push combined initial state on todo stack
    FD_DF("SupConNormClosed: todo push initial state");
    current.q = *rPlantGen.InitStatesBegin();
    current.x = *rSupCandGen.InitStatesBegin();
    current.z = *rObserverGen.InitStatesBegin();
    todo.push(current);

    // process todo stack
    while(!todo.empty()) {
      // allow for user interrupt, incl progress report
      FD_WPC(1,2,"Normality(): iterating states"); 
      // get top element from todo stack
      current = todo.top();
      todo.pop();
      FD_DF("SupConNormClosed: todo #" << todo.size() << " processed #" << processed.Size());
      FD_DF("SupConNormClosed: pop: (" << rPlantGen.SStr(current.q) << "|" << rSupCandGen.SStr(current.x) << ")");

      // break cycles 
      if(processed.Exists(current.x)) continue;
      if(critical.Exists(current.x)) continue;

      // record processed
      processed.Insert(current.x);

      // figure events disbabled by candidate K w.r.t plant G
      EventSet disabled = rPlantGen.ActiveEventSet(current.q) - rSupCandGen.ActiveEventSet(current.x);

      // if an unobservabel event is disabled, current state becomes critical
      if(!(disabled <= rCAlph)) {
        critical.Insert(current.x);
        continue;
      }
  
      // disable respective transition in observer Hobs
      titho = rObserverGen.TransRelBegin(current.z);
      titho_end = rObserverGen.TransRelEnd(current.z);
      while(titho!=titho_end) {
        if(!disabled.Exists(titho->Ev)) { ++titho; continue; }
        rObserverGen.ClrTransition(titho++);
      }

      // find successor states to push on stack
      titg = rPlantGen.TransRelBegin(current.q);
      titg_end = rPlantGen.TransRelEnd(current.q);
      tith = rSupCandGen.TransRelBegin(current.x);
      tith_end = rSupCandGen.TransRelEnd(current.x);
      titho = rObserverGen.TransRelBegin(current.z);
      titho_end = rObserverGen.TransRelEnd(current.z);
      while( (titg != titg_end) && (tith != tith_end) && (titho != titho_end) ) {
        FD_DF("SupNorm: processing g-transition: " << rPlantGen.TStr(*titg)); 
        FD_DF("SupNorm: processing h-transition: " << rSupCandGen.TStr(*tith));
        // increment case A: process common events
        if( (titg->Ev == tith->Ev) && (tith->Ev == titho->Ev) )   {
    	  FD_DF("SupNorm: processing common event " << rPlantGen.EStr(titg->Ev));
          // push successor
          if(!processed.Exists(tith->X2)) {
            successor.q=titg->X2;
            successor.x=tith->X2;
            successor.z=titho->X2;
            todo.push(successor);
	  }
	  // increment
  	  ++titg; ++tith; ++titho;
        }
        // increment case B: increment H transitions for events disabled by Hobs (when we disabled events in Hobs)
        else if (tith->Ev < titho->Ev) {
          rSupCandGen.ClrTransition(tith++);
        }
        // increment case C: increment Hobs transitions for events disabled by H (for removed critical states))
        else if (titho->Ev < tith->Ev) {
	  ++titho;
        }
        // increment case D: increment G transitions for events disabled by H
        else if (titg->Ev < tith->Ev) {
	  ++titg;
        }
        // increment case E: increment Hobs transitions for events disabled by G
        else {
          ++titho;
        }
      } // end accessible states
      // reasoning for leftovers:
      // a) if tith==end then we dont need to restrict H by Hobs anymore
      // b) if titg=end then (by L subseteq K) we have tith=end, and we argue as in a)
      // c) if titho=end then we need to restrict H by Hobs ...
      while( (tith != tith_end) ) {
        // increment case B: increment H transitions for events disabled by Hobs (when we disabled events in Hobs)
        rSupCandGen.ClrTransition(tith++);
      }
  
    } // end: todo stack

    // remove critical
    //rSupCandGen.DelStates((rSupCandGen.States()-processed) + critical);
    rSupCandGen.DelStates(critical);
  
    // break on fixpoint
    if(ssz != rSupCandGen.TransRelSize()) continue;
    if(osz != rObserverGen.TransRelSize()) continue;
    break;
      
  } //end: until fixpoint

  // cosmetic
  rSupCandGen.Accessible();

  /*
  // debugging: compare with Lin-Brandt formula
  Generator Rpr = rSupCandGen;
  MarkAllStates(Rpr);
  if(!LanguageEquality(K,Rpr)) FD_WARN("SUPNORM ERROR??? Supremal?");
  */

}



/** rti wrapper */
void SupNorm(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) 
{
  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  // execute
  SupNorm(rPlantGen,rPlantGen.ObservableEvents(),rSpecGen,*pResGen);
  // copy all attributes of input alphabet
  pResGen->EventAttributes(rPlantGen.Alphabet());
  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}

/** rti wrapper */
void SupNormClosed(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) 
{
  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  // execute
  SupNormClosed(rPlantGen,rPlantGen.ObservableEvents(),rSpecGen,*pResGen);
  // copy all attributes of input alphabet
  pResGen->EventAttributes(rPlantGen.Alphabet());
  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}


/** rti wrapper */
void SupConNormClosed(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) 
{
  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  // execute
  SupConNormClosed(rPlantGen,rPlantGen.ControllableEvents(),rPlantGen.ObservableEvents(),rSpecGen,*pResGen);
  // copy all attributes of input alphabet
  pResGen->EventAttributes(rPlantGen.Alphabet());
  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}

/** rti wrapper */
void SupConNorm(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) 
{
  FD_DF("SupConNorm(" << rPlantGen.Name() << "," << rSpecGen.Name() << "): rti wrapper");
  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  // execute
  SupConNorm(rPlantGen,rPlantGen.ControllableEvents(),rPlantGen.ObservableEvents(),rSpecGen,*pResGen);
  // copy all attributes of input alphabet
  pResGen->EventAttributes(rPlantGen.Alphabet());
  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}

} // end namespace
