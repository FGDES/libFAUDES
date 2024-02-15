/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2015  Michael Meyer, Thomnas Moor.
   Copyright (C) 2021,2023  Yiheng Tang, Thomnas Moor.
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

#include "cfl_conflequiv.h"
#include "cfl_bisimcta.h"
#include "cfl_regular.h"
#include "cfl_graphfncts.h"


/** Two debug levels for functions in this source file **/
//#define FAUDES_DEBUG_COMPVER0
//#define FAUDES_DEBUG_COMPVER1
#ifdef FAUDES_DEBUG_COMPVER0
#define FD_CV0(message) FAUDES_WRITE_CONSOLE("FAUDES_CONFEQ: "  << message)
#else
#define FD_CV0(message)
#endif
#ifdef FAUDES_DEBUG_COMPVER1
#define FD_CV1(message) FAUDES_WRITE_CONSOLE("FAUDES_CONFEQ: "  << message)
#else
#define FD_CV1(message)
#endif


namespace faudes {



// insert manually omega event for given automaton
// YT: it has also been considered that to use a omega-selfloop over all
// marked states. This is not functionally harmful. However, this will enlarge
// the set of incoming transitions for marked states, and thus will make many
// partitions unnecesaritly finer, e.g. those incoming-eq-based paritions and
// only silent incoming rules.
void AppendOmegaTermination(Generator& rGen){
    if (rGen.ExistsEvent("_OMEGA_")||rGen.ExistsEvent("_OMEGA_TERMINAL_")||rGen.ExistsState("_TERMINAL_")){
        throw Exception("AppendOmegaTermination", "please don't use event names _OMEGA_, _OMEGA_TERMINAL_ and state name _TERMINAL_", 100);
    }
    Idx omega = rGen.InsEvent("_OMEGA_");
    Idx terminal = rGen.InsState("_TERMINAL_"); // this will be marked later on
    StateSet::Iterator sit = rGen.MarkedStatesBegin();
    for(;sit!=rGen.MarkedStatesEnd();sit++){ // each marked state has a omega trans to terminal
        rGen.SetTransition(*sit,omega,terminal);
    }
    rGen.SetMarkedState(terminal);
    Idx omega_terminal = rGen.InsEvent("_OMEGA_TERMINAL_");
    rGen.SetTransition(terminal,omega_terminal,terminal); // need a special selfloop for terminal state, so that partition will always separate it
}

// Convenience typedef for incoming transsitions to a given state (or set of states).
// Since we know the target state X2 from the context, we only record X1 and Ev to
// experience better performance
typedef std::set<std::pair<Idx,Idx>> SetX1Ev;

// Merge equivalent classes, i.e. perform quotient abstraction
// (this implementation works fine with a small amount of small equiv classes)
void MergeEquivalenceClasses(
			     Generator& rGen,
			     TransSetX2EvX1& rRevTrans,
			     const std::list< StateSet >& rClasses)
{
  // iterators
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  TransSetX2EvX1::Iterator rit;
  TransSetX2EvX1::Iterator rit_end;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  // record for delayed delete
  StateSet delstates;
  // iterate classes
  std::list< StateSet >::const_iterator qit=rClasses.begin();
  for(;qit!=rClasses.end();++qit) {
    sit=qit->Begin();
    sit_end=qit->End();
    Idx q1=*(sit++); // this than denotes the name of quotient, i.e.[q1]
    for(;sit!=sit_end;++sit){
      Idx q2=*sit;
      // merge outgoing transitions form q2 to q1
      tit = rGen.TransRelBegin(q2);
      tit_end = rGen.TransRelEnd(q2);
      for(;tit!=tit_end;++tit) {
        rGen.SetTransition(q1,tit->Ev,tit->X2);
        rRevTrans.Insert(q1,tit->Ev,tit->X2);
      }
      // merge incomming transitions form q2 to q1
      rit = rRevTrans.BeginByX2(q2);
      rit_end = rRevTrans.EndByX2(q2);
      for(;rit!=rit_end;++rit) {
        rGen.SetTransition(rit->X1,rit->Ev,q1);
        rRevTrans.Insert(rit->X1,rit->Ev,q1);
      }
      if(rGen.ExistsMarkedState(q2))
        rGen.InsMarkedState(q1);
      if((rGen.ExistsInitState(q2)))
        rGen.InsInitState(q1);
      delstates.Insert(q2);
    }
  }
  // do delete
  rGen.DelStates(delstates);
}


// construct observation equivalent quotient
// in Pilbrow & Malik 2015, the so called "weak observation eq" is introduced which
// seems to be coarser. However, according to our discussion with Mr. Malik,
// the quotient cannot be constructed from the original automaton (with tau events),
// but from the saturated and tau-removed automaton. In various test cases, weak-ob-eq
// shows worse performance then ob-eq and thus abandoned
void ObservationEquivalentQuotient(Generator& g, const EventSet& silent){
  FD_DF("ObservationEquivalentQuotient(): prepare for t#"<<g.TransRelSize());

  // have extendend/reverse-ordered transition relations
  TransSetX2EvX1 rtrans;
  g.TransRel().ReSort(rtrans);  
  std::list<StateSet> eqclasses;
  ComputeWeakBisimulationSatCTA(g,silent,eqclasses);

  // merge  classes
  FD_DF("ObservationEquivalentQuotient(): merging classes #"<< eqclasses.size());
  MergeEquivalenceClasses(g,rtrans,eqclasses);

  FD_DF("ObservationEquivalentQuotient(): done with t#"<<g.TransRelSize());
}


// Compute the incoming non-tau transitions of a state in a saturated
// and tau-removed generator. Since the target X2 is known, we return
// a set of pairs {X1,Ev}. Moreover, for the sake of reachability from
// some intial state, a special pair <0,0> indicates that this state
// can be silently reached from some initial state. Since IncomingTransSet
// typically is called within a loop, we require to  pre-compute the reverse
// transitionrelation
void IncomingTransSet(
		      const Generator& rGen,
		      const TransSetX2EvX1& rRTrans,
		      const EventSet& silent,
		      const Idx& state,
		      SetX1Ev& result){
  result.clear(); // initialize result
  std::stack<Idx> todo; // states to process
  std::set<Idx> visited; // states processed
  todo.push(state);
  visited.insert(state);

  while (!todo.empty()){
    Idx cstate = todo.top();
    todo.pop();
    // a "flag" indicating that this state can be silently reached by
    // some initial state, see remark at the beginning
    if (rGen.InitStates().Exists(cstate))
      result.insert({0,0});
    TransSetX2EvX1::Iterator rtit = rRTrans.BeginByX2(cstate);
    TransSetX2EvX1::Iterator rtit_end = rRTrans.EndByX2(cstate);
    for(;rtit!=rtit_end;rtit++){
      if (!silent.Exists(rtit->Ev))
	result.insert({rtit->X1,rtit->Ev});
      else{
	if (visited.find(rtit->X1) == visited.end()){
	  todo.push(rtit->X1);
	  visited.insert(rtit->X1);
	}
      }
    }
  }
}

// compute the non-silent active events of a state of agiven generator.
// this algo is similar to IncomingTransSet, but we shall notice
// that IncomingTransSet record the source state as well but here not.
void ActiveNonTauEvs(
		     const Generator &rGen,
		     const EventSet &silent,
		     const Idx &state,
		     EventSet &result){
  result.Clear(); // initialize result
  std::stack<Idx> todo; // states to process
  std::set<Idx> visited;
  todo.push(state);
  visited.insert(state);
  while (!todo.empty()){
    Idx cstate = todo.top();
    todo.pop();
    TransSet::Iterator rtit = rGen.TransRelBegin(cstate);
    TransSet::Iterator rtit_end = rGen.TransRelEnd(cstate);
    for(;rtit!=rtit_end;rtit++){
      if (!silent.Exists(rtit->Ev))
	result.Insert(rtit->Ev);
      else{
	if (visited.find(cstate) == visited.end()){
	  todo.push(rtit->X2);
	  visited.insert(rtit->X2);
	}
      }
    }
  }
}

// return eq classes of =_inc. Instead of returning list of state sets,
// this function retunrs a map where the keys are the (source state, (non-tau)event) - pairs
// and the value of each key is the corresponding eq class. Effectively, the keys are just
// for efficient internal search of existing classes and will not be utilised in any other
// functions which requies incoming eq (they will only need the values)
std::map<SetX1Ev,StateSet> IncomingEquivalentClasses(const Generator& rGen, const EventSet& silent){
  FD_CV1("-- IncomingEquivalentClasses: candidats");
  // have reverse transition relation
  TransSetX2EvX1 rtrans;
  rGen.TransRel().ReSort(rtrans);
  //StateSet candidates;  
  //TransSet::Iterator tit = rGen.StatesBegin();
  //TransSet::Iterator tit_end = rGen.StatesEnd();
  // start the incoming equivalence partition
  StateSet::Iterator sit = rGen.StatesBegin();
  StateSet::Iterator sit_end = rGen.StatesEnd();
  // set up a map <incoming transset->class> to fast access incoming transset
  // of existing classes. This map is only meant to check incoming equivalence.
  std::map<SetX1Ev,StateSet> incomingeqclasses;
  std::map<SetX1Ev,StateSet>::iterator mit;
  for(;sit!=sit_end;sit++){
    SetX1Ev rec_income;
    IncomingTransSet(rGen,rtrans,silent,*sit,rec_income);
    mit = incomingeqclasses.find(rec_income);
    if (mit != incomingeqclasses.end()){ 
      mit->second.Insert(*sit);
    } else {
      StateSet newclass;
      newclass.Insert(*sit);
      incomingeqclasses.insert({rec_income,newclass});
    }
  }
  return incomingeqclasses;
}

// Active events rule; see cited literature 3.2.1
// Note: this is a re-write of Michael Meyer's implementation to refer to the
// ext. transistion relation, as indicated by the literature.
// YT: And now I have rewritten this again, hopefully for better readability
// YT: This rule was suggested to be implemented together with enabled continuation rule
// to avoid computing incoming eq multiple times, see C. Pilbrow and R. Malik 2015. However,
// since enabled continuation rule requires tau-loop-free automaton and active events rule
// may generate tau, i have separated them
void ActiveEventsRule(Generator& g, const EventSet& silent){

  std::map<SetX1Ev,StateSet> incomingeqclasses = IncomingEquivalentClasses(g,silent);
  std::list<StateSet> eqclasses; // store result
  TransSetX2EvX1 rtrans; // convenient declaration for MergeEqClasses

  FD_CV1("-- Refine for ActiveEventRule")
  // iterate
  std::map<SetX1Ev,StateSet>::iterator mit = incomingeqclasses.begin();
  for (;mit!=incomingeqclasses.end();mit++){
    // refine each =_inc class. in each refined class, states shall
    // have the same set of non-tau active event
    // in the course of refinement, merged
    while(mit->second.Size()>1){
      StateSet fineclass; // refined class
      StateSet::Iterator sit = mit->second.Begin();
      fineclass.Insert(*sit);
      EventSet activeevs;
      ActiveNonTauEvs(g,silent,*sit,activeevs); // xg: exclude tau
      StateSet::Iterator sit_compare = sit;
      sit_compare++; // next state in this =_inc class. compare active non-tau ev with *sit
      mit->second.Erase(*sit);// delete to avoid duplets
      while(sit_compare!=mit->second.End()){
	EventSet activeevs_compare;
	ActiveNonTauEvs(g,silent,*sit_compare,activeevs_compare);
	if (activeevs_compare==activeevs){
	  fineclass.Insert(*sit_compare);
	  StateSet::Iterator todelete = sit_compare;
	  sit_compare++;
	  mit->second.Erase(todelete);
	}
	else sit_compare++;
      }
      if (fineclass.Size()>1) eqclasses.push_back(fineclass);
    }
  }
  g.TransRel().ReSort(rtrans);
  MergeEquivalenceClasses(g,rtrans,eqclasses);
}

// YT: this is a generalized rule of silent continuation rule, see Pilbrow and Malik 2015.
// Note this function requires tau-loop free automata.
void EnabledContinuationRule(Generator &g, const EventSet &silent){
  std::map<SetX1Ev,StateSet> incomingeqclasses = IncomingEquivalentClasses(g,silent);
  std::list<StateSet> eqclasses; // store result
  TransSetX2EvX1 rtrans; // convenient declaration for MergeEqClasses
  // YT: we only consider states both with active tau. Both with always enabled evs,
  // as suggested in Pilbrow & Malik, are neglected
  eqclasses.clear();
  rtrans.Clear();
  std::map<SetX1Ev,StateSet>::iterator mit = incomingeqclasses.begin();
  for (;mit!=incomingeqclasses.end();mit++){
    StateSet fineclass; // refined class
    StateSet::Iterator sit = mit->second.Begin();
    while(sit!=mit->second.End()){
      if (!(g.ActiveEventSet(*sit) * silent).Empty()){ // g: has outgoing tau
        fineclass.Insert(*sit);
        sit++;
      }
      else sit++;
    }
    if (fineclass.Size()>1) eqclasses.push_back(fineclass);
  }
  g.TransRel().ReSort(rtrans);
  MergeEquivalenceClasses(g,rtrans,eqclasses);
}

// simple function removing tau self loops
void RemoveTauSelfloops(Generator &g, const EventSet &silent){
  TransSet::Iterator tit = g.TransRelBegin();
  TransSet::Iterator tit_end = g.TransRelEnd();
  while(tit!=tit_end){
    if (tit->X1 == tit->X2 && silent.Exists(tit->Ev)) g.ClrTransition(tit++);
    else tit++;
  }
}

// as a special case of observation equivalence, states on a tau-loop are all equivalent
// and can be merged to a single state. This step is preferred to be done before
// (weak) observation equivalence, as they require transition saturation which is quite
// expensive.
void MergeSilentLoops(Generator &g, const EventSet &silent){

  TransSetX2EvX1 rtrans;
  g.TransRel().ReSort(rtrans);

  // have a generator copy where only silent transitions are preserved
  Generator copyg(g);
  TransSet::Iterator tit = copyg.TransRel().Begin();
  while (tit!=copyg.TransRelEnd()){
    if (!silent.Exists(tit->Ev)) // if not a silent trans, delete
      copyg.ClrTransition(tit++);
    else tit++;
  }

  // compute eqclass and merge on original generator
  std::list<StateSet> eqclasses;
  StateSet dummy;
  ComputeScc(copyg,eqclasses,dummy);

  // delete trivial eqclasses (yt: I dont want to hack into the computescc function)
  std::list<StateSet>::iterator sccit = eqclasses.begin();
  while (sccit!=eqclasses.end()){
    if ((*sccit).Size()==1) eqclasses.erase(sccit++);
    else sccit++;
  }

  MergeEquivalenceClasses(g,rtrans,eqclasses);
}

// Certain conflicts. see cited literature 3.2.3
// -- remove outgoing transitions from not coaccessible states
void RemoveNonCoaccessibleOut(Generator& g){
  StateSet notcoaccSet=g.States()-g.CoaccessibleSet();
  StateSet::Iterator sit=notcoaccSet.Begin();
  StateSet::Iterator sit_end=notcoaccSet.End();
  for(;sit!=sit_end;++sit){
    TransSetX1EvX2::Iterator tit=g.TransRelBegin(*sit);
    TransSetX1EvX2::Iterator tit_end=g.TransRelEnd(*sit);
    for(;tit!=tit_end;) g.ClrTransition(tit++);
    //FD_DF("RemoveCertainConflictA: remove outgoing from state "<<*sit);
  }
}

// Certain conflicts. see cited literature 3.2.3
// -- remove outgoing transitions from states that block by a silent event
void BlockingSilentEvent(Generator& g,const EventSet& silent){
  FD_DF("BlockingSilentEvent(): prepare for t#"<<g.TransRelSize());
  StateSet coacc=g.CoaccessibleSet();
  StateSet sblock;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  // loop all transitions to figure certain blocking states
  tit=g.TransRelBegin();
  tit_end=g.TransRelEnd();
  for(;tit!=tit_end;++tit) {
    if(silent.Exists(tit->Ev))
      if(!coacc.Exists(tit->X2))
        sblock.Insert(tit->X1);
  }
  // unmark blocking states and eliminate possible future
  sit=sblock.Begin();
  sit_end=sblock.End();
  for(;sit!=sit_end;++sit) {
    g.ClrMarkedState(*sit);
    tit=g.TransRelBegin(*sit);
    tit_end=g.TransRelEnd(*sit);
    for(;tit!=tit_end;)
      g.ClrTransition(tit++);
  }
  FD_DF("BlockingSilentEvent(): done with t#"<<g.TransRelSize());
}

// Certain conflicts. see cited literature 3.2.3
// -- merge all states that block to one representative
void MergeNonCoaccessible(Generator& g){
  StateSet notcoacc=g.States()-g.CoaccessibleSet();
  // bail out on trovial case
  if(notcoacc.Size()<2) return;
  // have a new state
  Idx qnc=g.InsState();
  // fix init status
  if((notcoacc * g.InitStates()).Size()>0)
    g.SetInitState(qnc);
  // fix transitions
  TransSet::Iterator tit=g.TransRelBegin();
  TransSet::Iterator tit_end=g.TransRelEnd();
  for(;tit!=tit_end;++tit){
    if(notcoacc.Exists(tit->X2))
      g.SetTransition(tit->X1,tit->Ev,qnc);
  }
  // delete original not coacc
  g.DelStates(notcoacc);
}


// Only silent incomming rule; see cited literature 3.2.4
// Note: input generator must be silent-SCC-free
// Note: this is a complete re-re-write and needs testing for more than one candidates
void OnlySilentIncoming(Generator& g, const EventSet& silent){

  // figure states with only silent incomming transitions
  // note: Michael Meyer proposed to only consider states with at least two incomming
  // events -- this was dropped in the re-write
  StateSet cand=g.States()-g.InitStates(); // note the initial state set is preserved
  TransSet::Iterator tit = g.TransRelBegin();
  TransSet::Iterator tit_end = g.TransRelEnd();  
  for(;tit!=tit_end;++tit)
    if(!silent.Exists(tit->Ev)) cand.Erase(tit->X2);

  // bail out on trivial
  if(cand.Size()==0) {
    return;
  }

  StateSet::Iterator sit = cand.Begin();
  StateSet::Iterator sit_end = cand.End();
  while (sit!=sit_end){
    TransSet::Iterator tit2 = g.TransRelBegin(*sit);
    TransSet::Iterator tit2_end = g.TransRelEnd(*sit);
    for (;tit2!=tit2_end;tit2++){
      if (silent.Exists(tit2->Ev)){
        break;
      }
    }

    if (tit2!=tit2_end) { // there is at least one tau outgoing
      // redirect transitions
      TransSetX2EvX1 rtrans;
      g.TransRel().ReSort(rtrans);
      TransSetX2EvX1::Iterator rtit = rtrans.BeginByX2(*sit);
      TransSetX2EvX1::Iterator rtit_end = rtrans.EndByX2(*sit);
      for(;rtit!=rtit_end;rtit++){
        if (g.ExistsMarkedState(*sit)){ // mark predecessor if the state to remove is marked
          g.SetMarkedState(rtit->X1);
        }
        TransSet::Iterator tit3 = g.TransRelBegin(*sit);
        TransSet::Iterator tit3_end = g.TransRelEnd(*sit);
        for (;tit3!=tit3_end;tit3++){
          g.SetTransition(rtit->X1,tit3->Ev,tit3->X2);
        }
      }
      StateSet::Iterator todelete = sit++;
      g.DelState(*todelete);
    }
    else sit++;
  }
  // remark: with this implementation, incoming events (incl. tau) will not be changed
  // through state removal. A candidate state in "cand" will always be a legit candidate
}


// Only silent outgoing rule; see cited literature 3.2.5
// Note: input generator must be silent-SCC-free
void OnlySilentOutgoing(Generator& g,const EventSet& silent){
  StateSet::Iterator sit = g.StatesBegin();
  StateSet::Iterator sit_end = g.StatesEnd();
  while(sit!=sit_end){
    // figure out whether this state is only silent outgoing
    if (g.MarkedStates().Exists(*sit)) {sit++;continue;}
    TransSet::Iterator tit2 = g.TransRelBegin(*sit);
    TransSet::Iterator tit2_end = g.TransRelEnd(*sit);
    if (tit2==tit2_end) {sit++;continue;} // sit points to a deadend state (nonmarked state without outgoing trans)
    for (;tit2!=tit2_end;tit2++){
      if (!silent.Exists(tit2->Ev)) break;
    }
    if (tit2 != tit2_end) {sit++;continue;} // sit has non-silent outgoing trans

    // sit has passed the test. relink outgoing transitions of predecessor
    TransSetX2EvX1 rtrans;
    g.TransRel().ReSort(rtrans);
    // (repair intial state quicker by first iterate outgoing trans)
    tit2 = g.TransRelBegin(*sit);
    tit2_end = g.TransRelEnd(*sit);
    for (;tit2!=tit2_end;tit2++){
      TransSetX2EvX1::Iterator rtit = rtrans.BeginByX2(*sit); // incoming trans to *sit
      TransSetX2EvX1::Iterator rtit_end = rtrans.EndByX2(*sit);
      for (;rtit!=rtit_end;rtit++){
        g.SetTransition(rtit->X1,rtit->Ev,tit2->X2);
      }
      if (g.ExistsInitState(*sit))
        g.SetInitState(tit2->X2);
    }
    StateSet::Iterator todelete = sit;
    sit++;
    g.DelState(*todelete);
  }
}

EventSet HidePriviateEvs(Generator& rGen, EventSet& silent){
  EventSet result;
  EventSet msilentevs=rGen.Alphabet()*silent;
  if(msilentevs.Empty())
    return result;
  Idx tau=*(msilentevs.Begin());
  result.Insert(*(msilentevs.Begin()));
  msilentevs.Erase(tau); // from now on, msilentevs exclude tau
  silent.EraseSet(msilentevs);
  if (msilentevs.Empty()) // in this case, only one silent event is set to tau and no need to hide
    return result;
  TransSet::Iterator tit=rGen.TransRelBegin();
  TransSet::Iterator tit_end=rGen.TransRelEnd();
  for(;tit!=tit_end;) {
    if(!msilentevs.Exists(tit->Ev)) {++tit; continue;}
    Transition t(tit->X1,tau,tit->X2);
    rGen.ClrTransition(tit++);
    if (!rGen.ExistsTransition(t))
      rGen.SetTransition(t);
  }
  rGen.InjectAlphabet(rGen.Alphabet()-msilentevs);
  return result;
}

// convenient wrapper for tau-loop removal. Technically, the following algos are
// very efficient and is suggested to perform prior to other abstraction. Furthermore,
// input automata being tau-loop free is necessary for the following abstraction rules:
// - OnlySilentIncoming
// - OnlySilentOutgoing
// - EnabledContinuationRule
// NOTE: currently, ObservationEquivalenceQuotient utilises saturation based algo which does
// not require tau-loop-free. If
void RemoveTauLoops(Generator& rGen, const EventSet& silent){
  MergeSilentLoops(rGen,silent);
  RemoveTauSelfloops(rGen,silent);
}


// apply all of the above rules once 
void ConflictEquivalentAbstractionOnce(Generator& rGen, EventSet& silent){
  // hiding must be performed beforehand.
  EventSet tau = HidePriviateEvs(rGen, silent);

  // ***************** abstraction rules *******************
  FD_CV1("Applying only silent incoming rule")
  RemoveTauLoops(rGen,tau);
  OnlySilentIncoming(rGen,tau);
  FD_CV1("Applying only silent outgoing rule")
  // tau-loop-free is required. Nevertheless, after onlysilentincoming, no tau-loops will emerge
  OnlySilentOutgoing(rGen,tau);
  FD_CV1("Applying enabled continuation rule")
  RemoveTauLoops(rGen,tau);
  EnabledContinuationRule(rGen,tau);
  FD_CV1("Applying active events rule")
  ActiveEventsRule(rGen,tau);
  FD_CV1("Applying observation eq quotient")
  ObservationEquivalentQuotient(rGen,tau);
  FD_CV1("Applying certain conflicts rule")
  BlockingSilentEvent(rGen,tau);
  MergeNonCoaccessible(rGen);
  FD_DF("ConflictEquivalentAbstraction(): done with t#"<<rGen.TransRelSize());
}

// apply all of the above repeatedly until a fixpoint is attained
void ConflictEquivalentAbstractionLoop(vGenerator& rGen, EventSet& rSilentEvents){
  Idx sz0=rGen.Size();
  (void) sz0; // make compiler happy
  while(true) {
    Idx sz1=rGen.Size();      
    FD_CV1("ConflictEquivalentAbstraction(): loop with states #"<<rGen.TransRelSize());
    FD_WPC(sz0, sz1, "ConflictEquivalentAbstraction: fixpoint iteration states #" << sz1);
    ConflictEquivalentAbstractionOnce(rGen,rSilentEvents);
    // break on slow progress
    //if(rGen.Size()> 1000 && rGen.Size()>0.95*sz1) break;
    if(rGen.Size()==sz1) break;
  }
  FD_CV1("ConflictEquivalentAbstraction(): done with states #"<<rGen.TransRelSize());
}
  
// select variant
void ConflictEquivalentAbstraction(vGenerator& rGen, EventSet& rSilentEvents){
  ConflictEquivalentAbstractionOnce(rGen,rSilentEvents);
  //ConflictEquivalentAbstractionLoop(rGen,rSilentEvents);
}  


// API wrapper  
bool IsNonconflicting(const GeneratorVector& rGvec) {

  GeneratorVector gvec = rGvec;

  FD_CV0("Appending Omega event")
    Idx git = 0;
  for(;git!=gvec.Size();git++){
    AppendOmegaTermination(gvec.At(git));
  }

  bool firstCycle = true;
  while (true){
    FD_CV0("========================================")
    FD_CV0("Remaining automata: #"<<gvec.Size())

    // trivial cases
    if(gvec.Size()==0) return true;
    if(gvec.Size()==1) break;

    // figure silent events
    EventSet silent, all, shared;
    Idx git = 0;
    while(true){
      all = all+gvec.At(git).Alphabet();
      Idx git_next = git+1;
      if (git_next == gvec.Size()) break;
      for(;git_next!=gvec.Size();git_next++){
	shared = shared
	  + (gvec.At(git).Alphabet())
	  * (gvec.At(git_next).Alphabet());
      }
      git++;
    }
    silent=all-shared; // all silent events in all current candidates

    // normalize for one silent event per generator, and then abstract.
    // note from the second iteration, this is only necessary for the
    // automaton composed from former candidates. This is realized by
    // the break at the end
    git = 0;
    for(;git!=gvec.Size();git++){
      // abstraction
      FD_CV0("Abstracting Automaton "<<gvec.At(git).Name()<<", with state count: "<<gvec.At(git).Size())
      ConflictEquivalentAbstraction(gvec.At(git), silent);
      FD_CV0("State count after abstraction: "<<gvec.At(git).Size())
      if(!firstCycle) break;
    }
    firstCycle = false;

    // candidate choice heuritics. Branch by different tasks
    Idx imin = 0;
    Idx jmin = 0;

    // candidat with fewest transitions 'minT'
    git = 1;
    for(;git!=gvec.Size();git++){
      if(gvec.At(git).TransRelSize()<gvec.At(imin).TransRelSize())
	imin = git;
    }
    // candidat with most common events 'maxC'
    git = jmin;
    Int score=-1;
    for(; git!=gvec.Size(); git++){
      if(git==imin) continue;
      Int sharedsize = (gvec.At(git).Alphabet() * gvec.At(imin).Alphabet()).Size();
      if ( sharedsize > score){
	jmin = git;
	score = sharedsize;
      }
    }
    // compose candidate pair
    Generator gij;
    FD_CV0("Composing automata "<<gvec.At(imin).Name()<<" and "<<gvec.At(jmin).Name())
      Parallel(gvec.At(imin),gvec.At(jmin),gij);
    GeneratorVector newgvec;
    newgvec.Append(gij); // the composed generator is always the first element
    git = 0;
    for(;git!=gvec.Size();git++){
      if (git == imin || git == jmin) continue;
      newgvec.Append(gvec.At(git)); // all other candidates are just copied to the next iteraion
    }
    gvec = newgvec;
  }
  return IsNonblocking(gvec.At(0));
}

// API wrapper  
bool IsNonblocking(const GeneratorVector& rGvec) {
  return IsNonconflicting(rGvec);
}


} // namespace  



/*
earlier revison for inspection




// intentend user interface 
bool IsNonblocking(const GeneratorVector& rGenVec) {
  Idx i,j;

  // trivial cases
  if(rGenVec.Size()==0) return true;
  if(rGenVec.Size()==1) return IsNonblocking(rGenVec.At(0));

  // have a local copy of input generator
  GeneratorVector gvec=rGenVec;

  // figure silent events
  EventSet silent, all, shared;
  all=gvec.At(0).Alphabet();
  for(i=0;i<gvec.Size()-1;++i){
    for(j=i+1;j<gvec.Size();++j){
      const Generator& g1=gvec.At(i);
      const Generator& g2=gvec.At(j);
      shared=shared+g1.Alphabet()*g2.Alphabet();
    }
    all=all+gvec.At(i).Alphabet();
  }      
  silent=all-shared;

  // normalize for one silent event per generator
  for(i=0;i<gvec.Size();++i){
    Generator& gi=gvec.At(i);
    EventSet sili=gi.Alphabet()*silent;
    if(sili.Size()<=1) continue;
    Idx esi=*(sili.Begin());
    sili.Erase(esi);
    silent.EraseSet(sili);
    all.EraseSet(sili);
    TransSet::Iterator tit=gi.TransRelBegin();
    TransSet::Iterator tit_end=gi.TransRelEnd();
    for(;tit!=tit_end;) {
      if(!sili.Exists(tit->Ev)) {++tit; continue;}
      Transition t(tit->X1,esi,tit->X2);
      gi.ClrTransition(tit++);
      gi.SetTransition(t);
    }
    gi.InjectAlphabet(gi.Alphabet()-sili);
  } 

  // reduce all generators 
  for(i=0;i<gvec.Size();i++){
    Generator& g=gvec.At(i);
    FD_DF("Abstract generator " << g.Name());
    FD_DF("Silent events #" << (g.Alphabet()*silent).Size());
    ConflictEquivalentAbstraction(g,g.Alphabet()*silent);
    if(g.InitStates().Size()>0)
      if(g.MarkedStates().Size()==0){
        FD_DF("No marked states (A)");
        return false;
      }
  }

  // loop until resolved
  while(gvec.Size()>=2) {

    // candidat pairs with fewest transitions 'minT'
    Idx imin=0;
    for(i=1;i<gvec.Size();i++){
      if(gvec.At(i).TransRelSize()<gvec.At(imin).TransRelSize()) 
        imin=i;
    }
    
    // candidat pairs with most local events 'maxL'
    Idx jmin=0;
    Int score=-1;
    for(i=0;i<gvec.Size();i++){
      if(i==imin) continue;
      const Generator& gi=gvec.At(imin);
      const Generator& gj=gvec.At(i);
      EventSet aij=gi.Alphabet()+gj.Alphabet();
      EventSet shared;
      for(j=0;j<gvec.Size();j++){
        if(j==imin) continue;
        if(j==i) continue;
        shared=shared + gvec.At(j).Alphabet()*aij;
      }
      Int jscore= aij.Size()-shared.Size();
      if(jscore>score) {score=jscore; jmin=i;}
    }

    // candidat pairs with fewest states when composed 'minS'
    //Idx jmin=0;
    //Float score=-1;
    //for(i=0;i<gvec.Size();i++){
    //  if(i==imin) continue;
    //  const Generator& gi=gvec.At(imin);
    //  const Generator& gj=gvec.At(i);
    //  Int jscore= gi.Size()*gj.Size()/((Float) gi.AlphabetSize()*gj.AlphabetSize()); // rough estimate
    //  if(jscore<score || score<0) {score=jscore; jmin=i;}
    // }

    // compose candidate pair
    Generator& gimin=gvec.At(imin);
    Generator& gjmin=gvec.At(jmin);
    FD_DF("Compose generator " << gimin.Name() << " and " << gjmin.Name());
    Parallel(gimin,gjmin,gimin);
    gvec.Erase(jmin);
    FD_DF("Composition done t#"<< gimin.TransRelSize());

    // update shared events
    EventSet silent, all, shared;
    all=gvec.At(0).Alphabet();
    for(i=0;i<gvec.Size()-1;++i){
      for(j=i+1;j<gvec.Size();++j){
        const Generator& g1=gvec.At(i);
        const Generator& g2=gvec.At(j);
        shared=shared+g1.Alphabet()*g2.Alphabet();
      }
      all=all+gvec.At(i).Alphabet();
    }      
    silent=all-shared;

    // normalize for one silent event per generator
    EventSet sili=gimin.Alphabet()*silent;
    if(sili.Size()>1) {
      Idx esi=*(sili.Begin());
      sili.Erase(esi);
      silent.EraseSet(sili);
      TransSet::Iterator tit=gimin.TransRelBegin();
      TransSet::Iterator tit_end=gimin.TransRelEnd();
      for(;tit!=tit_end;) {
        if(!sili.Exists(tit->Ev)) {++tit; continue;}
        Transition t(tit->X1,esi,tit->X2);
        gimin.ClrTransition(tit++);
        gimin.SetTransition(t);
      }
      gimin.InjectAlphabet(gimin.Alphabet()-sili);
    }

    // abstract
    if(gvec.Size()>1){
      FD_DF("Abstract generator " << gimin.Name());
      FD_DF("Silent events #" << (gimin.Alphabet()*silent).Size());
      ConflictEquivalentAbstraction(gimin,gimin.Alphabet()*silent);
      if(gimin.InitStates().Size()>0)
      if(gimin.MarkedStates().Size()==0){
        FD_DF("No marked states (A)");
       	  return false;
      }
    }
  
  }

  FD_DF("Testing final stage with t#" << gvec.At(0).TransRelSize() << "/s#" << gvec.At(0).Size());
  bool res=gvec.At(0).BlockingStates().Size()==0;
  FD_DF("IsNonblocking(): done");
  return res;
}

*/


