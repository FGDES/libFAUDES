/** @file syn_sscon.cpp Standard synthesis consistency test */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2014  Matthias Leinfelder, Thomas Moor

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
   

#include "syn_sscon.h"
#include <stack>


namespace faudes {




/*
 ***************************************************************************************
 ***************************************************************************************

 The core algorithm is implemented as the function IsStdSynthesisConsistentUnchecked(...)
 The function assumes that the arguments satisfy their respecive conditions. For a
 detailed discussion on the algorithm see

 Moor, T.: Natural projections for the synthesis of 
 non-conflicting supervisory controllers, 
 Workshop on Discrete Event Systems (WODES), Paris, 2014.

 and

 Moor, T., Baier, Ch., Wittmann, Th.:
 Consistent abstractions for the purpose of supervisory control,
 Proc. 52nd IEEE Conference on Decision and Control (CDC), pp. 7291-7196, Firenze, 2013. 

 The initial version of this function was implemented by Matthias Leinfelder in course 
 of his bachelor thesis. 

 ***************************************************************************************
 ***************************************************************************************
 */

#undef FD_DF
#define FD_DF(msg) FD_WARN(msg)

//IsStdSynthesisConsistentUnchecked (rPlantGen, rPlant0Gen, rCAlph)
bool IsStdSynthesisConsistentUnchecked(
  const Generator& rPlantGen, 
  const EventSet&  rCAlph, 
  const Generator&  rPlant0Gen) {  
    
  // prepare: extract relevant alpahbets
  const EventSet& sig  = rPlantGen.Alphabet();
  const EventSet& sigo = rPlant0Gen.Alphabet();
  const EventSet& sigc = rCAlph;
  EventSet sigco = sigc * sigo;
  EventSet siguco = sig - sigco;

  // prepare: construct rich plant representation by parallel composition with abstraction
  // (the implementation of Parallel guarantees a trim result --- we rely on this)
  Generator gg0; 
  ProductCompositionMap gg0pmap;
  Parallel(rPlantGen,rPlant0Gen,gg0pmap,gg0);
  FD_DF("IsStdSynthesisConsistent(..): relevant states: " << gg0.Size());
  
  // prepare: construct a so-called trace generator with exactly the same reachable 
  // states as the rich plant gg0, but with only one path to reach each state; thus, 
  // each class of strings that reaches any particular state in gg0 has exaclty
  // one representative string in the generated language of the trace generator

  // initialize result and prepare the todo-stack for the loop invariant "todo states 
  // are reachable in trace by exactly one path"
  Generator trace;
  trace.InsEvents(sig);   
  std::stack<Idx> todo;      
  todo.push(gg0.InitState());       
  trace.InsInitState(gg0.InitState()); 
    
  // forward reachability search
  while(!todo.empty()){
    // pick state from todo
    Idx q = todo.top();
    todo.pop();
    // iterate all reachable states
    TransSet::Iterator tit,tit_end;               
    tit =  gg0.TransRelBegin(q);   
    tit_end =  gg0.TransRelEnd(q); 
    for(;tit != tit_end; ++tit){
      // by the invariant "tit->X2 not in trace" implies "tit->X2 is not on the stack"
      if(!trace.ExistsState(tit->X2)){ 
        todo.push(tit->X2);
        trace.InsState(tit->X2);
        trace.SetTransition(*tit);
      }
    }
  }
  // this completes the construction of the trace generator
    
  /*
  rPlantGen.GraphWrite("tmp_gen.jpg");
  rPlant0Gen.GraphWrite("tmp_gen0.jpg");
  gg0.GraphWrite("tmp_gg0.jpg");
  trace.GraphWrite("tmp_trace.jpg");
  */

  // prepare: backward reach via reverse transistion relation
  TTransSet<TransSort::X2EvX1> revtransrel; 
  gg0.TransRel().ReSort(revtransrel); 
    
  // initialize target states by marked states
  StateSet target = gg0.MarkedStates();
  // provide boundary to focus backward reach 
  StateSet btarget = target; 
     
  // grow target set by backward 'reach and test'
  while(true) {
    
    // break on success
    if(target.Size()==gg0.Size()) break;

    // report
    FD_DF("IsStdSynthesisConsistent(..): targets for one-transition tests A,B and C: #" << target.Size() << " (b" << btarget.Size() << ")");

    // inner loop to grow by fast one-transition tests A and B first
    while(true) {
      // allow for user interrupt
      FD_WPC(target.Size(),gg0.Size(),"IsStdSynthesisConsistent(): processing one-step tests"); 
      // prepare to sense when stuck
      Idx tszi=target.Size();
      // iterate over all relevant target states (using boundary)
      StateSet::Iterator sit = btarget.Begin();
      StateSet::Iterator sit_end = btarget.End();
      while(sit != sit_end){
        // break on success
        if(target.Size()==gg0.Size()) break;
	// pick target state
        Idx x2=*sit;
        FD_DF("IsStdSynthesisConsistent(..): testing predecessors of target " << gg0.StateName(x2));
	// sense if all predecessors pass    
	bool allpass=true;
	// iterate over all target predecessors
	TTransSet<TransSort::X2EvX1>:: Iterator rit =revtransrel.BeginByX2(x2); 
	TTransSet<TransSort::X2EvX1>:: Iterator rit_end=revtransrel.EndByX2(x2); 
	for(;rit != rit_end; ++rit){
	  // predecessor technically passes if allready identified as target
	  if(target.Exists(rit->X1)) continue;
	  // test A and B from [ref2]: passes if uncontrollable or unobservable event leads to the target
	  if(siguco.Exists(rit->Ev)) {
	    target.Insert(rit->X1);
	    btarget.Insert(rit->X1);
	    FD_DF("IsStdSynthesisConsistent(..): one-transition test AB passed at: " << gg0.StateName(rit->X1));
	    continue;
	  }
	  // test C adapted from [ref2]: predecessor passes if (i) the abstraction state is not marked, 
          // (ii) all events in the abstraction are controllable, and (iii) each event leads the low level 
          // plant to a successor within the target (as of 2017/06)
          if(!rPlant0Gen.ExistsMarkedState(gg0pmap.Arg2State(rit->X1))){
            EventSet act0 = rPlant0Gen.ActiveEventSet(gg0pmap.Arg2State(rit->X1));
	    EventSet::Iterator eit=act0.Begin();
	    EventSet::Iterator eit_end=act0.End();
     	    for(;eit != eit_end; ++eit)
              if(!target.Exists(gg0.SuccessorState(rit->X1,*eit))) break;
	    if(eit == eit_end) {
  	      target.Insert(rit->X1);
	      btarget.Insert(rit->X1);
              FD_DF("IsStdSynthesisConsistent(..): one-transition test C passed at: " << gg0.StateName(rit->X1));
	      continue;
	    } 
	  }
	  // record the fail
	  allpass=false;
	}
        // safely increment sit
        ++sit;
	// remove from boundary if if all predecessors passed
	if(allpass) {
          FD_DF("IsStdSynthesisConsistent(..): all predecessors of " << gg0.StateName(x2) << " have passed");
          btarget.Erase(x2);
  	}
      }
      // break inner loop when stuck
      if(target.Size()==tszi) break;
    }

    // break on success
    if(target.Size()==gg0.Size()) break;

    FD_DF("IsStdSynthesisConsistent(..): targets for multi-transition tests: #" << target.Size() << " (b" << btarget.Size() << ")");

    // looking for one pass only
    bool onepass=false;

    // inner loop to grow by comperativly fast multi-transition test D
    StateSet::Iterator sit =  btarget.Begin();
    StateSet::Iterator sit_end =  btarget.End();
    for(; (!onepass) && (sit != sit_end); ++sit){
      // iterate over all target predecessors
      TTransSet<TransSort::X2EvX1>:: Iterator rit =revtransrel.BeginByX2(*sit); 
      TTransSet<TransSort::X2EvX1>:: Iterator rit_end=revtransrel.EndByX2(*sit); 
      for(; rit != rit_end; ++rit){
	// cannot gain target if predecessor is already identified
	if(target.Exists(rit->X1)) continue;
        // allow for user interrupt
        FD_WPC(target.Size(),gg0.Size(),"IsStdSynthesisConsistent(): processing fast star-step test D"); 
        // test D from [ref2]
	// (i) compute low-level reach without attaining a low-level target state (evil reach);
	// (ii) test whether being constraint to the evil reach implies high-level blocking;
        // (iii) test whether each exist from the high-level reach leads the low level to the target (as of 2017/06)
	// skip high-level marked
        StateSet ereach;
        StateSet ereach0;
	std::stack<Idx> etodo;      
        etodo.push(rit->X1);
	bool fail=false;
        while((!etodo.empty()) && (!fail)){
	  // pick a state from todo stack
          Idx x1 = etodo.top();
          etodo.pop();
	  // not evil reach
	  if(target.Exists(x1)) continue; 
	  // high-level marked
	  if(rPlant0Gen.ExistsMarkedState(gg0pmap.Arg2State(x1))) {
	    fail=true;
	    continue;
	  }
	  // extend evil reach
	  ereach.Insert(x1);
	  ereach0.Insert(gg0pmap.Arg2State(x1));
          // iterate all reachable states
          TransSet::Iterator tit = gg0.TransRelBegin(x1);   
          TransSet::Iterator tit_end =  gg0.TransRelEnd(x1); 
          for(;tit != tit_end; ++tit)
	    if(!ereach.Exists(tit->X2))
	      etodo.push(tit->X2);
	}
	// passed (ii), need to check (iii)
	if(!fail) {
          FD_DF("IsStdSynthesisConsistent(..): multi-transition test *D: evil reach #" << ereach.Size());
	  StateSet::Iterator xit = ereach.Begin();
	  StateSet::Iterator xit_end = ereach.End();
          for(; (xit!= xit_end) && (!fail); ++xit) {
            Idx x1=*xit;
            Idx x10=gg0pmap.Arg2State(x1);
            TransSet::Iterator tit0=rPlant0Gen.TransRelBegin(x10);
	    TransSet::Iterator tit0_end=rPlant0Gen.TransRelEnd(x10);
     	    for(;tit0 != tit0_end; ++tit0) {
              if(ereach0.Exists(tit0->X2)) continue;
	      if(!target.Exists(gg0.SuccessorState(x1,tit0->Ev))) break;
	    } 
	    if(tit0 != tit0_end) fail=true;
          }   
	  if(!fail) {
            FD_DF("IsStdSynthesisConsistent(..): multi-transition test *D passed at: " << gg0.StateName(rit->X1));
            onepass=true;
            //target.InsertSet(ereach);
            //btarget.InsertSet(ereach);
  	    target.Insert(rit->X1);
	    btarget.Insert(rit->X1);
            break;
	  }
	}
      }
    }
 
    // sense success in test D
    if(onepass) continue;

    
    // iterate over thourough tests D from [ref2] E from [ref1]
    FD_DF("IsStdSynthesisConsistent(..): running multi-transition tests D and E");
    StateSet ftarget;
    StateSet::Iterator sit2 =  btarget.Begin();
    StateSet::Iterator sit2_end =  btarget.End();
    for(; (!onepass) && (sit2 != sit2_end); ++sit2){
      // iterate over all target predecessors
      TTransSet<TransSort::X2EvX1>:: Iterator rit =revtransrel.BeginByX2(*sit2); 
      TTransSet<TransSort::X2EvX1>:: Iterator rit_end=revtransrel.EndByX2(*sit2); 
      for(; (!onepass) && (rit != rit_end); ++rit){
	// cannot gain target if predecessor is already identified
	if(target.Exists(rit->X1)) continue;
	// we know this to fail
	if(ftarget.Exists(rit->X1)) continue;	
        // allow for user interrupt
        FD_WPC(target.Size(),gg0.Size(),"IsStdSynthesisConsistent(): processing star-step tests");
    
	// prepare tests D/E from [ref2]/[ref1]

        // Ls: a) s
        Generator Ls(trace);                         
        Ls.SetMarkedState(rit->X1);                      
        // Ls: b) s . sig^*        --- all future of s
        Ls.ClrTransitions(rit->X1);
        SelfLoopMarkedStates(Ls,sig);
        // Ms: a) M                --- all successful future of s
        Generator Ms=gg0;
        Ms.InjectMarkedStates(target);
        // Ms: b) Ls ^ M
	LanguageIntersection(Ls,Ms,Ms);          
        // M0s: projection of Ms   --- all successful future of s from the perspective of the abstraction
        Generator M0s;
        Project(Ms,sigo,M0s);
        // Es0: a) Ms0 . sigo^*    --- evil specification to prevent succesful future of s
        Generator E0s=M0s;
        StateSet::Iterator ssit =  E0s.MarkedStatesBegin();
        StateSet::Iterator ssit_end =  E0s.MarkedStatesEnd();
        for(; ssit!=ssit_end; ++ssit) 
          E0s.ClrTransitions(*ssit);
        SelfLoopMarkedStates(E0s,sigo);
        // Es0: b) L0 - M0s . sigo^*       
        LanguageDifference(rPlant0Gen,E0s,E0s);
        Trim(E0s);

        // apply test D: does reachability of X1 contradict with the evil specification ?
        Generator Es=E0s;
        InvProject(Es,sig);
  	PrefixClosure(Es);
        if(EmptyLanguageIntersection(Ls,Es)) {
          // pass: evil specification renders X1 unreachable 
          FD_DF("IsStdSynthesisConsistent(..): multi-transition test D passed at: " << gg0.StateName(rit->X1));
          onepass=true;
          target.Insert(rit->X1);
          btarget.Insert(rit->X1);
          continue;
	}

        // apply test E: use the supremal evil supervisor to test whether reachability of X1 complies with E0s
        Generator K0s;
        SupRelativelyPrefixClosed(rPlant0Gen,E0s,E0s); 
        SupCon(rPlant0Gen,sigco,E0s,K0s); 
	// test whether X1 is reachable under evil supervision
        Generator Ks=K0s;
        InvProject(Ks,sig);
  	PrefixClosure(Ks);
        if(EmptyLanguageIntersection(Ls,Ks)) {
          // pass: evil supervisor renders X1 unreachable 
          FD_DF("IsStdSynthesisConsistent(..): multi-transition test E passed at: " << gg0.StateName(rit->X1));
          onepass=true;
          target.Insert(rit->X1);
          btarget.Insert(rit->X1);
	  continue;
   	}

	// record failure
        ftarget.Insert(rit->X1);
	

      } // iterate target predecessors
    } // iterate targets

 
    // break outer loop if tests D/E did not gain one more target
    if(!onepass) break;

 }

  // return success
  return target.Size()==gg0.Size();  
}
  
  


/*  
 ***************************************************************************************
 ***************************************************************************************

 Application Interface

 ***************************************************************************************
 ***************************************************************************************
 */


// IsStdSynthesisConsistent(rPlantGen, rCAlph, rPlant0Gen)
bool IsStdSynthesisConsistent(
  const Generator& rPlantGen, 
  const EventSet&  rCAlph, 
  const Generator&  rPlant0Gen) 
{  
  // test if the plant is deterministic
  if(!IsDeterministic(rPlantGen)){ 
    std::stringstream errstr;
    errstr << "Plant generator must be deterministic, " << "but is nondeterministic";
    throw Exception("IsStdSynthesisConsistent", errstr.str(), 501);
  }
  // test if the Alphabet with the controllable events matches the Alphabet of the Generator
  if(! (rCAlph <= rPlantGen.Alphabet()) ){
    std::stringstream errstr;
    errstr << "Controllable events must be a subset of the alphabet specified by \"" << rPlantGen.Name() << "\"";
    throw Exception("IsStdSynthesisConsistent", errstr.str(), 506);
  }  
  // test if the abstraction alphabet matches the plant alphabet
  const EventSet& sigo = rPlant0Gen.Alphabet();
  if(! (sigo <= rPlantGen.Alphabet() ) ){
    std::stringstream errstr;
    errstr << "Abstraction alphabet must be a subset of the plant alphabet pescified by \"" << rPlantGen.Name() << "\"";
    throw Exception("IsStdSynthesisConsistent", errstr.str(), 506);
  }  
    
  // test the consistency of the plant
  return IsStdSynthesisConsistentUnchecked(rPlantGen,rCAlph,rPlant0Gen);
}
  
  
// IsStdSynthesisConsistent(rPlantGen,rPlant0Gen)
bool IsStdSynthesisConsistent(	   
  const System& rPlantGen,
  const Generator& rPlant0Gen)
{
  // extract controllable events
  const EventSet& calph = rPlantGen.ControllableEvents();
  // pass on
  return IsStdSynthesisConsistent(rPlantGen,calph,rPlant0Gen);    
}


} // name space 
