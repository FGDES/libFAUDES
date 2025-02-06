#include "corefaudes.h"
#include "pev_pgenerator.h"
#include "pev_operators.h"
#include "pev_abstraction.h"

#define PCOMPVER_VERB1(msg) \
  { if((!faudes::ConsoleOut::G()->Mute()) && (faudes::ConsoleOut::G()->Verb() >=1 )) { \
      std::ostringstream cfl_line; cfl_line << msg << std::endl; faudes::ConsoleOut::G()->Write(cfl_line.str(),0,0,0);} }
#define PCOMPVER_VERB0(msg) \
  { if((!faudes::ConsoleOut::G()->Mute()) && (faudes::ConsoleOut::G()->Verb() >=0 )) { \
      std::ostringstream cfl_line; cfl_line << msg << std::endl; faudes::ConsoleOut::G()->Write(cfl_line.str(),0,0,0);} }

namespace faudes {

/*
***********
BEGIN Candidate
*********
*/

// Merge equivalent classes, i.e. perform quotient abstraction
// (this implementation works fine with a small amount of small equiv classes)
void Candidate::MergeEquivalenceClasses(
  Generator& rGen,
  TransSetX2EvX1& rRevTrans,
  const std::list< StateSet >& rClasses,
  const EventSet& silent
)
{
  // iterators
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  TransSetX2EvX1::Iterator rit;
  TransSetX2EvX1::Iterator rit_end;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  std::map<Idx,Idx> result;
  // record for delayed delete
  StateSet delstates;
  // iterate classes
  std::list< StateSet >::const_iterator qit=rClasses.begin();
  for(;qit!=rClasses.end();++qit) {
    sit=qit->Begin();
    sit_end=qit->End();
    Idx q1=*(sit++); // this than denotes the name of quotient
    result.insert({q1,q1});
    for(;sit!=sit_end;++sit){
      Idx q2=*sit;
      result.insert({q2,q1});
      // merge outgoing transitions form q2 to q1
      tit = rGen.TransRelBegin(q2);
      tit_end = rGen.TransRelEnd(q2);
      for(;tit!=tit_end;++tit) {
//        // if X2 of this transition is still in this class with a silent ev, skip -- we shall do selfloop removal extern
//        if (silent.Exists(tit->Ev) && (*qit).Exists(tit->X2)) continue;
        rGen.SetTransition(q1,tit->Ev,tit->X2);
        rRevTrans.Insert(q1,tit->Ev,tit->X2);
      }
      // merge incomming transitions form q2 to q1
      rit = rRevTrans.BeginByX2(q2);
      rit_end = rRevTrans.EndByX2(q2);
      for(;rit!=rit_end;++rit) {
//        // if X1 of this transition is still in this class with a silent ev, skip -- we shall do selfloop removal extern
//        if (silent.Exists(rit->Ev) && (*qit).Exists(rit->X1)) continue;
        rGen.SetTransition(rit->X1,rit->Ev,q1);
        rRevTrans.Insert(rit->X1,rit->Ev,q1);
      }
      // fix marking: if q2 was not marked, need to un-mark q1 (original by Michael Meyer)
      // if(!(rGen.ExistsMarkedState(q2)))
      //  rGen.ClrMarkedState(q1);
      // fix marking: if q2 was marked, so becomes  q1 (quotient by the books)
      if(rGen.ExistsMarkedState(q2))
        rGen.InsMarkedState(q1);
      // fix initial states: if q2 was initial, so becomes q1
      if((rGen.ExistsInitState(q2)))
        rGen.InsInitState(q1);
      // log for delete
      delstates.Insert(q2);
    }
  }
  // do delete
  rGen.DelStates(delstates);

  // update mergemap (only used for counter example)
  std::map<Idx,Idx>::iterator mit = mMergeMap.begin();
  for(;mit!=mMergeMap.end();mit++){
      std::list< StateSet >::const_iterator classit = rClasses.begin();
      for (;classit!=rClasses.end();classit++){
          if ((*classit).Exists(mit->second)){
              // this shall be consistent with codes above, in that the Idx of quotient class
              // is always the FIRST member of the class. Here is something to notice: StateSet
              // is based on TBaseSet, whose core is the std::set which is automatically sorted.
              // This means, as long as the eqclass is not inserted with new elements (note the
              // input rClasses is const), the first element of the class will not change, i.e.
              // always the "smallest" element.
              mit->second = *(*classit).Begin();
              break;
          }
      }
  }
}
// Compute extended transition relation '=>', i.e. relate each two states that
// can be reached by one non-slilent event and an arbitrary amount of silent
// events befor/after the non-silent event
//
// Note: currently all silent events are treated equivalent, as if they
// had been substituted by a single silent event "tau"; we should perform
// this substitution beforehand.
// YT: the original implementation is replaced by the following new iteration
// where for each iteration we only consider those new augmented transitions.

// but it seem that the codes is not obviously getting faster.

void Candidate::ExtendedTransRel(const Generator& rGen, const EventSet& rSilentAlphabet, TransSet& rXTrans) {

  // HELPERS:
  TransSet::Iterator tit1;
  TransSet::Iterator tit1_end;
  TransSet::Iterator tit2;
  TransSet::Iterator tit2_end;
  TransSet newtrans;

  // initialize result with original transitionrelation
  rXTrans=rGen.TransRel();
  newtrans = rGen.TransRel(); // initialize iteration
  // loop for fixpoint
  while(!newtrans.Empty()) {
    // store new transitions for next iteration
    TransSet nextnewtrans;
    // loop over all transitions in newtrans
    tit1=newtrans.Begin();
    tit1_end=newtrans.End();
    for(;tit1!=tit1_end; ++tit1) {
      // if it is silent add transition to non silent successor directly
      if(rSilentAlphabet.Exists(tit1->Ev)) {
         tit2=rXTrans.Begin(tit1->X2);
         tit2_end=rXTrans.End(tit1->X2);
         for(;tit2!=tit2_end; ++tit2) {
//         if(!rSilentAlphabet.Exists(tit2->Ev)) // tmoor 18-09-2019
           if (!rXTrans.Exists(tit1->X1,tit2->Ev,tit2->X2))
             nextnewtrans.Insert(tit1->X1,tit2->Ev,tit2->X2);
        }
      }
      // if it is not silent add transition to silent successor directly
      else {
        tit2=rXTrans.Begin(tit1->X2);
        tit2_end=rXTrans.End(tit1->X2);
        for(;tit2!=tit2_end; ++tit2) {
          if(rSilentAlphabet.Exists(tit2->Ev)){
            if (!rXTrans.Exists(tit1->X1,tit1->Ev,tit2->X2))
              nextnewtrans.Insert(tit1->X1,tit1->Ev,tit2->X2);
          }
        }
      }
    }
    // insert new transitions
    rXTrans.InsertSet(nextnewtrans);
    // update trans for next iteration.
    newtrans = nextnewtrans;
  }
}

void Candidate::ObservationEquivalentQuotient(Generator& g, const EventSet& silent){
  FD_DF("ObservationEquivalentQuotient(): prepare for t#"<<g.TransRelSize());

  // have extendend/reverse-ordered transition relations
  TransSetX2EvX1 rtrans;
  g.TransRel().ReSort(rtrans);

  Generator xg(g);  // a copy of input g, which will be augmented with some fancy transitions
  TransSet xtrans;
  ExtendedTransRel(xg,silent,xtrans);
  FD_DF("ObservationEquivalentQuotient(): ext. trans t#"<<xtrans.Size());

  // figure observation equivalent states
  std::list< StateSet > eqclasses;
  xg.InjectTransRel(xtrans);
  // install silent selfloops
  if (!silent.Empty()){
    StateSet::Iterator sit = xg.StatesBegin();
    for(;sit!=xg.StatesEnd();sit++){
      EventSet::Iterator eit = silent.Begin();
      xg.SetTransition(*sit,*eit,*sit);
    }
  }
  ComputeBisimulationCTA(xg,eqclasses);


  // merge  classes
  FD_DF("ObservationEquivalentQuotient(): merging classes #"<< eqclasses.size());
  MergeEquivalenceClasses(g,rtrans,eqclasses,silent);

  FD_DF("ObservationEquivalentQuotient(): done with t#"<<g.TransRelSize());
}

// perform quotient construction for reverse obsevation equivalence with active tau,
// see C. Pilbrow and R. Malik 2015. Current algorithm is perhaps suboptimal as it
// install a total new generator copy with all transitions reversed. Then the normal
// bisim-algorithm is performed, where eqclasses are refined to such that all states
// in a class has active tau event.
void Candidate::ReverseObservationEquivalentQuotient(Generator &g, const EventSet &silent){
    Generator rev_g; // have a copy to reverse all trans
    rev_g.InsEvents(g.Alphabet());
    rev_g.InsStates(g.States());

    // install reversed transition
    TransSet::Iterator tit = g.TransRelBegin();
    TransSet::Iterator tit_end = g.TransRelEnd();
    for(;tit!=tit_end;tit++){
        rev_g.SetTransition(tit->X2,tit->Ev,tit->X1);
    }

    // implement explicit "reverse_omega" for original initial states
    Idx rev_omega = rev_g.InsEvent("_REV_OMEGA_");
    Idx rev_dummy_omega = rev_g.InsEvent("_REV_DUMMY_OMEGA_");
    Idx rev_terminal = rev_g.InsState("_REV_TERMINAL_");
    rev_g.SetTransition(rev_terminal,rev_dummy_omega,rev_terminal);
    StateSet::Iterator sit = g.InitStatesBegin();
    StateSet::Iterator sit_end = g.InitStatesEnd();
    for(;sit!=sit_end;sit++){
        rev_g.SetTransition(*sit,rev_omega,rev_terminal);
    }

    // figure out eqclasses, see e.g. ObservationEquivalentQuotient
    TransSet xtrans;
    ExtendedTransRel(rev_g,silent,xtrans);
    rev_g.InjectTransRel(xtrans);
    if (!silent.Empty()){
        StateSet::Iterator sit2 = rev_g.StatesBegin();
        StateSet::Iterator sit2_end = rev_g.StatesEnd();
        for(;sit2!=sit2_end;sit2++){
            rev_g.SetTransition(*sit2,*silent.Begin(),*sit2);
        }
    }
    std::list<StateSet> eqclasses;
    ComputeBisimulationCTA(rev_g,eqclasses);

    // refine eqclasses in that all states in a class has outgoing tau
    std::list<StateSet> eqclasses_fine; // store result in new list, as some classes may be directly abandoned
    std::list<StateSet>::iterator classit = eqclasses.begin();
    std::list<StateSet>::iterator classit_end = eqclasses.end();
    for (;classit!=classit_end;classit++){
        StateSet newclass; // record refined class
        StateSet::Iterator sit2 = classit->Begin();
        StateSet::Iterator sit2_end = classit->End();
        for(;sit2!=sit2_end;sit2++){
            if (!(g.ActiveEventSet(*sit2) * silent).Empty()) newclass.Insert(*sit2);
        }
        if (newclass.Size()>=2) eqclasses_fine.push_back(newclass); // only store non-trivial classes
    }


    // merge refined eqclasses
    TransSetX2EvX1 rtrans;
    g.TransRel().ReSort(rtrans);
    MergeEquivalenceClasses(g,rtrans,eqclasses_fine,silent);

}

// Performs weak observation equivalent abstraction;
// This is a re-imp of ObservationEquivalentQuotient in that we delete silent transitions
// after transition augmentation. FURHTERMORE, WE SHALL NOTICE that the resulting quotient
// is not built on input generator, but on the saturated generator with all tau deleted.
void Candidate::WeakObservationEquivalentQuotient(Generator& g, const EventSet& silent){
  FD_DF("ObservationEquivalentQuotient(): prepare for t#"<<g.TransRelSize());

  // have extendend/reverse-ordered transition relations

  TransSet xtrans;
  ExtendedTransRel(g,silent,xtrans);
  FD_DF("ObservationEquivalentQuotient(): ext. trans t#"<<xtrans.Size());

  // remove silent events from extended transition relation
  // yt: in weak observation equivalence, see e.g. Rong Su et al. (2010),
  // only non-empty transitions are relevant
  xtrans.RestrictEvents(g.Alphabet()-silent);

  // figure observation equivalent states
  std::list< StateSet > eqclasses;
  g.InjectTransRel(xtrans);
  ComputeBisimulationCTA(g,eqclasses);
  // merge  classes
  TransSetX2EvX1 rtrans;
  g.TransRel().ReSort(rtrans);
  FD_DF("ObservationEquivalentQuotient(): merging classes #"<< eqclasses.size());
  MergeEquivalenceClasses(g,rtrans,eqclasses,silent); // here silent as input is dummy

  FD_DF("ObservationEquivalentQuotient(): done with t#"<<g.TransRelSize());
}

// compute the incoming trans set of a state in a saturated
// and tau-removed generator. Instead of directly return a
// set of TransSet, a set of pair <Idx (source state), Idx (ev)>
// is returned for a better performance, hopefully. Moreover,
// for the sake of reachability from some intial state, a special pair
// <0,0> indicates that this state can be silently reached from some
// initial state.
void Candidate::IncomingTransSet(
        const Generator& rGen,
        const EventSet& silent,
        const Idx& state,
        std::set<std::pair<Idx, Idx>>& result){
    result.clear(); // initialize result
    TransSetX2EvX1 rtrans;
    rGen.TransRel().ReSort(rtrans);
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
        TransSetX2EvX1::Iterator rtit = rtrans.BeginByX2(cstate);
        TransSetX2EvX1::Iterator rtit_end = rtrans.EndByX2(cstate);
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

// compute the non-silent active events of a given generator.
// this algo is similar to IncomingTransSet, but we shall notice
// that IncomingTransSet record the source state as well but here not.
void Candidate::ActiveNonTauEvs(
        const Generator &rGen,
        const EventSet &silent,
        const Idx &state,
        EventSet &result){
    result.Clear(); // initialize result
    std::stack<Idx> todo; // states to process
    std::set<Idx> visited; // states processed (stack shall also work)
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

// Active events rule; see cited literature 3.2.1
// Note: this is a re-write of Michael Meyer's implementation to refer to the
// ext. transistion relation, as indicated by the literature.
// YT: Now it also partly supports Enabled Continuation Rule, see C. Pilbrow and R. Malik 2015
void Candidate::ActiveEventsANDEnabledContinuationRule(
        Generator& g,
        const EventSet& silent){
    // convenient typedef for incoming trans. This is for the incoming eq partition and,
    // instead of directly set up std::set<TransSet>, we hope the set of pairs<Idx(source state), Idx(ev)>
    // will perform better.
    typedef std::set<std::pair<Idx,Idx>> TransSetX1Ev;

    // start the incoming equivalence partition
    StateSet::Iterator sit = g.StatesBegin();
    StateSet::Iterator sit_end = g.StatesEnd();

    // set up a map <incoming transset->class> to fast access incoming transset
    // of existing classes. This map is only meant to check incoming equivalence.
    std::map<TransSetX1Ev,StateSet> incomingeqclasses;
    std::map<TransSetX1Ev,StateSet>::iterator mit;
    for(;sit!=sit_end;sit++){
        TransSetX1Ev rec_income;
        IncomingTransSet(g,silent,*sit,rec_income);
        mit = incomingeqclasses.find(rec_income);
        if (mit != incomingeqclasses.end()){ // existing class found
            mit->second.Insert(*sit);
        }
        else { // no class for this state found
            StateSet newclass;
            newclass.Insert(*sit);
            incomingeqclasses.insert({rec_income,newclass});
        }
    }

    // Following the explanation in Chapter 4.2 of C. Pilbrow and R. Malik 2015,
    // we first merge states via ActiveEventsRule. Then the incoming eq classes
    // will be updated, then merge states via EnabledContinuationRule.
    // And we avoid the the second time of ActiveEventsRule, which is acturally
    // suggested in the aforementioned paper

    std::list<StateSet> eqclasses; // store result
    TransSetX2EvX1 rtrans; // convenient declaration for MergeEqClasses

    // 1 - ActiveEventsRule
    mit = incomingeqclasses.begin();
    for (;mit!=incomingeqclasses.end();mit++){
        StateSet fineclass; // refined class
        StateSet::Iterator sit = mit->second.Begin();
        fineclass.Insert(*sit);
        EventSet activeevs;
        ActiveNonTauEvs(g,silent,*sit,activeevs); // xg: exclude tau
        StateSet::Iterator sit_compare = sit;
        sit_compare++;
        while(sit_compare!=mit->second.End()){
            EventSet activeevs_compare;
            ActiveNonTauEvs(g,silent,*sit,activeevs_compare);
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
    g.TransRel().ReSort(rtrans);
    MergeEquivalenceClasses(g,rtrans,eqclasses,silent);

    // 2 - EnabledContinuationRule
    eqclasses.clear();
    rtrans.Clear();
    mit = incomingeqclasses.begin();
    for (;mit!=incomingeqclasses.end();mit++){
        StateSet fineclass; // refined class
        StateSet::Iterator sit = mit->second.Begin();
        while(sit!=mit->second.End()){
            if (!(g.ActiveEventSet(*sit) * silent).Empty()){ // g: has outgoing tau
                fineclass.Insert(*sit);
                // (this #if is actually unnecessary if we dont apply ActiveEventsRule again)
                if (fineclass.Size()>1){ // fineclass is untrivial
                    StateSet::Iterator todelete = sit;
                    sit++;
                    mit->second.Erase(todelete);
                }
                else sit++;
            }
            else sit++;
        }
        if (fineclass.Size()>1) eqclasses.push_back(fineclass);
    }
    g.TransRel().ReSort(rtrans);
    MergeEquivalenceClasses(g,rtrans,eqclasses,silent);
}



// simple function removing tau self loops
void Candidate::RemoveTauSelfloops(Generator &g, const EventSet &silent){
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
void Candidate::MergeSilentLoops(Generator &g, const EventSet &silent){

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

    MergeEquivalenceClasses(g,rtrans,eqclasses,silent);
}

// Certain conflicts. see cited literature 3.2.3
// -- remove outgoing transitions from not coaccessible states
void Candidate::RemoveNonCoaccessibleOut(Generator& g){
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
void Candidate::BlockingSilentEvent(Generator& g,const EventSet& silent){
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
void Candidate::MergeNonCoaccessible(Generator& g){
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

// Certain conflicts. see cited literature 3.2.3
// -- if a transition blocks, remove all transitions from the same state with the same label
void Candidate::BlockingEvent(Generator& g,const EventSet& silent){
  FD_DF("BlockingEvent(): prepare for t#"<<g.TransRelSize());
  StateSet coacc=g.CoaccessibleSet();
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  TransSet::Iterator dit;
  TransSet::Iterator dit_end;
  TransSet deltrans;
  // loop all transitions to figure transitions to blocking states
  tit=g.TransRelBegin();
  tit_end=g.TransRelEnd();
  for(;tit!=tit_end;++tit) {
    // silent events are treated by a separat rule;
    if(silent.Exists(tit->Ev)) continue;
    // look for transitions that block
    if(coacc.Exists(tit->X1)) continue;
    if(coacc.Exists(tit->X2)) continue;
    // consider all transitions with the same event to block
    dit=g.TransRelBegin(tit->X1,tit->Ev);
    dit_end=g.TransRelEnd(tit->X1,tit->Ev);
    for(;dit!=dit_end;++dit) {
      // keep selfloops (Michael Meyer)
      if(dit->X1==dit->X2) continue;
      // rcord to delete later
      deltrans.Insert(*dit);
    }
  }
  // delete transitions
  dit=deltrans.Begin();
  dit_end=deltrans.End();
  for(;dit!=dit_end;++dit)
    g.ClrTransition(*dit);
  FD_DF("BlockingEvent(): done with t#"<<g.TransRelSize());
}

// Only silent incomming rule; see cited literature 3.2.4
// Note: input generator must be silent-SCC-free
// Note: this is a complete re-re-write and needs testing for more than one candidates
void Candidate::OnlySilentIncoming(Generator& g, const EventSet& silent){

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

      if (tit2!=tit_end) { // there is at least one tau outgoing
          // redirect transitions
          TransSetX2EvX1 rtrans;
          g.TransRel().ReSort(rtrans);
          TransSetX2EvX1::Iterator rtit = rtrans.BeginByX2(*sit);
          TransSetX2EvX1::Iterator rtit_end = rtrans.EndByX2(*sit);
          for(;rtit!=rtit_end;rtit++){
              TransSet::Iterator tit3 = g.TransRelBegin(*sit);
              TransSet::Iterator tit3_end = g.TransRelEnd(*sit);
              for (;tit3!=tit3_end;tit3++){
                  g.SetTransition(rtit->X1,tit3->Ev,tit3->X2);
              }
          }
          StateSet::Iterator todelete = sit;
          sit++;
          g.DelState(*todelete);
      }
      else sit++;
  }

  // yt: interestingly, according to the discussion with R. Malik,
  // we do not need to assign the merge map information in this step -- this is counter-intuitive
  // meaning that the deleted state has its merge map unchanged, but it will never be used because
  // this state is "skipped" and thus can never be the final state of a counter example. Thus, it
  // will not be utilized by line 12 of Algo 1, R. Malik and S. Ware 2018. Note we need the merge map
  // only when executing line 12 of Algo 1.

}


// Only silent outgoing rule; see cited literature 3.2.5
// Note: input generator must be silent-SCC-free
void Candidate::OnlySilentOutgoing(Generator& g,const EventSet& silent){
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

void Candidate::HidePrivateEvs(EventSet& silent){
    mGenHidden=mGenRaw;
    EventSet msilentevs=mGenRaw.Alphabet()*silent;
    if(msilentevs.Empty()){
        mGenMerged = mGenHidden;
        return;
    }
    Idx tau=*(msilentevs.Begin());
    SetTau(tau);
    SetSilentevs(msilentevs);
    msilentevs.Erase(tau); // from now on, msilentevs exclude tau
    silent.EraseSet(msilentevs);
    if (msilentevs.Empty()){// in this case, only one 1 silent event is set to tau and no need to hide
        mGenMerged = mGenHidden;
        return;
    }
    TransSet::Iterator tit=mGenHidden.TransRelBegin();
    TransSet::Iterator tit_end=mGenHidden.TransRelEnd();
    for(;tit!=tit_end;) {
        if(!msilentevs.Exists(tit->Ev)) {++tit; continue;}
        Transition t(tit->X1,tau,tit->X2);
        mGenHidden.ClrTransition(tit++);
        if (!mGenHidden.ExistsTransition(t))
            mGenHidden.SetTransition(t);
    }
    mGenHidden.InjectAlphabet(mGenHidden.Alphabet()-msilentevs);
    mGenMerged = mGenHidden;
}

void Candidate::ConflictEquivalentAbstraction(EventSet& silent){
    // hide must be performed beforehand.
    // we use tau itself as silent event set to
    // access abstraction algorithms
    HidePrivateEvs(silent);
    EventSet tau;
    if (mtau!=0)
        tau.Insert(mtau);

    // current codes does not support counterexample computation for certain conflicts.
    // Anyway, in R. Malik and S. Ware 2018, it shows no definitely postive result
    // when considering certain conf in counter example computation due to the fact
    // that we need to compute paralle product at each step of iteration. Furthermore,
    // we would like to have counter example which goes into the blocking zone as
    // far as possible, namely ends up at either a deadlocking state or a state in a livelock
    // -- this also contradicts with certain conflicts conceptionally.

    // ***************** abstraction rules *******************
    RemoveTauSelfloops(mGenMerged,tau);
    MergeSilentLoops(mGenMerged,tau);
//    OnlySilentIncoming(mGenMerged,tau);
//    OnlySilentOutgoing(mGenMerged,tau);
    ObservationEquivalentQuotient(mGenMerged,tau);
    ActiveEventsANDEnabledContinuationRule(mGenMerged,tau);
//    ReverseObservationEquivalentQuotient(mGenMerged,tau);
}


  /*
************************************
END: Candidate
************************************
*/


  



  
void PrintEventTable(const pGenerator& rPGen){
    EventSet::Iterator eit = rPGen.AlphabetBegin();
    for(;eit!=rPGen.AlphabetEnd();eit++)
        std::cout<<"EvName: "<<rPGen.EventName(*eit)<<"\t\t"<<"Index:"<<ToStringInteger(*eit)
                <<"\t\t"<<"Priority: "<<ToStringInteger(rPGen.EventAttribute(*eit).Priority())<<std::endl;
}

void PrintEqclasses(const std::list<StateSet>& rEqclasses){
    std::list<StateSet>::const_iterator eqit = rEqclasses.begin();
    for(;eqit!=rEqclasses.end();eqit++){
        StateSet::Iterator sit = eqit->Begin();
        std::cout<<"[";
        for(;sit!=eqit->End();sit++){
            std::cout<<ToStringInteger(*sit)+"\t";
        }
        std::cout<<"]"<<std::endl;
    }

}

void AppendOmegaTermination(pGenerator& rPGen, EventPriorities& rPrios){
    Idx omega = rPGen.InsEvent("_OMEGA_");
    Idx omega_terminal = rPGen.InsEvent("_OMEGA_TERMINAL_");

    // handle event prioriy
    const Idx lowest = rPGen.GlobalAttribute().PLowest();
    rPGen.EventAttributep(omega)->Priority(lowest);
    rPGen.EventAttributep(omega_terminal)->Priority(lowest);
    rPrios.InsPriority("_OMEGA_",lowest);
    rPrios.InsPriority("_OMEGA_TERMINAL_",lowest);
    Idx terminal = rPGen.InsState("_TERMINAL_");
    rPGen.SetTransition(terminal,omega_terminal,terminal);
    StateSet::Iterator sit = rPGen.MarkedStatesBegin();
    for(;sit!=rPGen.MarkedStatesEnd();sit++){
        rPGen.SetTransition(*sit,omega,terminal);
    }
    rPGen.SetMarkedState(terminal); // cosmetic

    // handle fairness constraints
    EventSet fairc;
    fairc.Insert(omega);
    fairc.Insert(omega_terminal);
    Fairness fair;
    fair.insert(fairc);
    rPGen.GlobalAttributep()->SetFairConsts(fair);
}

// install priority to a plain automaton
/*  
  void InstallPrio(pGenerator& rPGen, const std::map<std::string, Idx>& rPrioMap){
    EventSet::Iterator eit = rPGen.AlphabetBegin();
    for(;eit!=rPGen.AlphabetEnd();eit++){
        std::map<std::string,Idx>::const_iterator mit = rPrioMap.find(rPGen.EventName(*eit));
        if (mit != rPrioMap.end()){
            rPGen.EventAttributep(mit->first)->Priority(mit->second);
        }
        else{
            rPGen.EventAttributep(*eit)->Priority(0);
            std::cout<< "InstallPrio()::WARNING: undefined priority for event "<< rPGen.EventName(*eit)
                     <<", set to 0"<<std::endl;
        }
    }
}
*/

// convenient wrapper to show priority of each event
void WritePrio (const pGenerator& rPGen){
    EventSet::Iterator eit = rPGen.AlphabetBegin();
    for(;eit!=rPGen.AlphabetEnd();eit++){
        std::cout<< rPGen.EventName(*eit) << "\t\t";
        std::cout<<ToStringInteger(rPGen.EventAttribute(*eit).Priority())<<std::endl;
    }
}

// shaping priosities but only for specified preempting events
// (used for Upsilon-shaping)
// NOTE: retains unreachable states
void ShapeUpsilon(pGenerator& rPGen, const EventSet& rPEvs){
    StateSet::Iterator sit = rPGen.StatesBegin();
    for(;sit!=rPGen.StatesEnd();sit++){
        Idx highest = rPGen.GlobalAttribute().PLowest();
        EventSet active = rPGen.ActiveEventSet(*sit);
        EventSet::Iterator eit = active.Begin();
        for(;eit!=active.End();eit++){
            if (rPGen.EventAttribute(*eit).Priority()<highest
                    && rPEvs.Exists(*eit)) // conjunct events in rPEvs as prioritised
                highest = rPGen.EventAttribute(*eit).Priority();
        }
        if (highest == rPGen.GlobalAttribute().PLowest()) continue;
        else{
            TransSet::Iterator tit = rPGen.TransRelBegin(*sit);
            while(tit!=rPGen.TransRelEnd(*sit)){
                if(rPGen.EventAttribute(tit->Ev).Priority()>highest){
                    if (rPGen.EventName(tit->Ev)=="_OMEGA_")
                        rPGen.ClrMarkedState(tit->X1); // unmark if remove omega transition, cosmetic
                    rPGen.ClrTransition(tit++);

                }
                else tit++;
            }
        }
    }
//    rPGen.Accessible();
}

// ordinary shaping by priority
void ShapePriorities(pGenerator& rPGen){
    ShapeUpsilon(rPGen,rPGen.Alphabet());
}

// shape by set of preempting events only (TM2025: need this old version to compile/link example)
void ShapePreemption(Generator& rGen, const EventSet &pevs){
    StateSet::Iterator sit = rGen.StatesBegin();
    StateSet::Iterator sit_end = rGen.StatesEnd();
    for(;sit!=sit_end; sit++){
        if ((rGen.ActiveEventSet(*sit) * pevs).Empty()) continue; // not a preemptive state
        TransSet::Iterator tit = rGen.TransRelBegin(*sit);
        TransSet::Iterator tit_end = rGen.TransRelEnd(*sit);

        while(tit!=tit_end){
            if (pevs.Exists(tit->Ev)) tit++;
            else rGen.ClrTransition(tit++);
        }
    }
    rGen.Accessible();
    rGen.Name("S(" + rGen.Name() + ")");
}
  

// require WF!
void RemoveRedSilentSelfloops (pGenerator& rPGen, const EventSet& rSilent){
    TransSet::Iterator tit=rPGen.TransRelBegin();
    while(tit!=rPGen.TransRelEnd()){
        if (tit->X1!=tit->X2) {tit++; continue;}
        if (!rSilent.Exists(tit->Ev)) {tit++; continue;}
        TransSet::Iterator tit2 = rPGen.TransRelBegin(tit->X1);
        for(;tit2!=rPGen.TransRelEnd(tit->X1);tit2++){
            if (rSilent.Exists(tit2->Ev) && tit2->X2 != tit->X1)
                break;
        }
        if (tit2!=rPGen.TransRelEnd(tit->X1)){
            rPGen.ClrTransition(tit++);
        }
        else tit++;
    }
}

// OLD NOTE: EXCLUDE unprioritised events, since this function evaluates the potential
// of a transition being preempted by NonsilHigherThen events.
EventSet NonsilHigherThen (const pGenerator& rPGen,
                       const EventSet& rSilent,
                       const Idx& rState,
                       const Idx& rK){
    EventSet result;
    if (rK==0) return result;
    result = rPGen.ActiveEventSet(rState);
    EventSet::Iterator eit = result.Begin();
    while(eit!=result.End()){
        if (!rSilent.Exists(*eit)
                && rPGen.EventAttribute(*eit).Priority() < rK){
            eit++;
        }
        else{
            EventSet::Iterator thiseit = eit++;
            result.Erase(*thiseit);
        }
    }
    return result;
}

// find the silent event at given priority. OLD: MUST BE PRIORITISED
Idx SilentEvAtPrio (const pGenerator& rPGen, const EventSet& rSilent, const Idx& rK){
    EventSet::Iterator eit = rPGen.AlphabetBegin();
    for(;eit!=rPGen.AlphabetEnd();eit++){
        if (!rSilent.Exists(*eit)) continue;
        if (rPGen.EventAttribute(*eit).Priority()==rK) return *eit;
    }
    std::cout<<"no such silent event with given priority found. Maybe bad priority or not silent?";
    throw;
}

void MergeEquivalenceClasses(
        pGenerator& rPGen,
        const EventSet& rSilent,
        const std::map< StateSet, Idx >& rLivelocks,
        const std::list< StateSet >& rClasses)
{
    TransSetX2EvX1 rtrans;
    rPGen.TransRel().ReSort(rtrans);
    // iterators
    StateSet::Iterator sit;
    StateSet::Iterator sit_end;
    TransSetX2EvX1::Iterator rtit;
    TransSetX2EvX1::Iterator rtit_end;
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
            tit = rPGen.TransRelBegin(q2);
            tit_end = rPGen.TransRelEnd(q2);
            for(;tit!=tit_end;++tit) {
                // skip silent self-loop (handled later)
                if (qit->Exists(tit->X2)&&rSilent.Exists(tit->Ev))
                    continue;
                rPGen.SetTransition(q1,tit->Ev,tit->X2);
                rtrans.Insert(q1,tit->Ev,tit->X2);
            }
            // merge incomming transitions form q2 to q1
            rtit = rtrans.BeginByX2(q2);
            rtit_end = rtrans.EndByX2(q2);
            for(;rtit!=rtit_end;++rtit) {
                // skip silent self-loop (handled later)
                if (qit->Exists(rtit->X1)&&rSilent.Exists(rtit->Ev))
                    continue;
                rPGen.SetTransition(rtit->X1,rtit->Ev,q1);
                rtrans.Insert(rtit->X1,rtit->Ev,q1);
            }
            if(rPGen.ExistsMarkedState(q2))
                rPGen.InsMarkedState(q1);
            if((rPGen.ExistsInitState(q2)))
                rPGen.InsInitState(q1);
            delstates.Insert(q2);
        }
        // handle livelock selfloop
        std::map<StateSet,Idx>::const_iterator livelockit = rLivelocks.find(*qit);
        if(livelockit!=rLivelocks.end()){
            rPGen.SetTransition(*(qit->Begin()),livelockit->second,*(qit->Begin()));
        }
    }
    // do delete
    rPGen.DelStates(delstates);
}


// return a generator with silent transitions only (non silent are REMOVED, not hidden)
// p -i-> q (i for prio) implies that there is a transition  p => q on which the lowest priorit
// (maximal value) is i.
// NOTE: generates tau_0 self-loops.
void SaturateLowestPrio(const pGenerator& rPGen, const EventSet& rSilent, pGenerator& rResult){
    rResult=rPGen;
    // only preserve silent transitions
    TransSet::Iterator tit = rResult.TransRel().Begin();
    while (tit!=rResult.TransRelEnd()){
        if (!rSilent.Exists(tit->Ev)) // if not a silent trans, delete
            rResult.ClrTransition(tit++);
        else tit++;
    }

    if (rSilent.Empty()) return;
    // HELPERS:
    TransSet::Iterator tit1;
    TransSet::Iterator tit1_end;
    TransSet::Iterator tit2;
    TransSet::Iterator tit2_end;

    // initialize result with original transitionrelation
    TransSet xTrans=rResult.TransRel();
    TransSet newtrans = xTrans; // initialize iteration
    // loop for fixpoint
    while(!newtrans.Empty()) {
        // store new transitions for next iteration
        TransSet nextnewtrans;
        // loop over all transitions in newtrans
        tit1=newtrans.Begin();
        tit1_end=newtrans.End();
        for(;tit1!=tit1_end; ++tit1) {
            tit2=xTrans.Begin(tit1->X2);
            tit2_end=xTrans.End(tit1->X2);
            for(;tit2!=tit2_end; ++tit2) {
                // saturate the lower priority silent transition
                // copy first tit1's event
                if (rResult.EventAttribute(tit1->Ev).Priority()>rResult.EventAttribute(tit2->Ev).Priority()){
                    if (!xTrans.Exists(tit1->X1,tit1->Ev,tit2->X2))
                    nextnewtrans.Insert(tit1->X1,tit1->Ev,tit2->X2);
                }
                else{
                    if (!xTrans.Exists(tit1->X1,tit2->Ev,tit2->X2))
                    nextnewtrans.Insert(tit1->X1,tit2->Ev,tit2->X2);
                }
            }
        }
        // insert new transitions
        xTrans.InsertSet(nextnewtrans);
        // update trans for next iteration.
        newtrans = nextnewtrans;
    }
    rResult.InjectTransRel(xTrans);

    // install self loops of tau_0 on all states
    StateSet::Iterator sit = rPGen.StatesBegin();
    StateSet::Iterator sit_end = rPGen.StatesEnd();
    for(;sit!=sit_end;sit++){
        rResult.SetTransition(*sit,SilentEvAtPrio(rPGen,rSilent,0),*sit);
    }
}

// saturate the transition (x)=epsilon=>->_Sig:k where
// Sig is the set of active events of x if mode == 0, or
// also include the successive =epsilon=>_0 part if mode==1
void SaturatePDelayed(const pGenerator& rPGen,
                      const EventSet& rSilent,
                      const bool& _mode,
                      pGenerator& rResult){
    rResult=rPGen;

    if (rSilent.Empty()) return;
    // HELPERS:
    TransSet::Iterator tit1;
    TransSet::Iterator tit1_end;
    TransSet::Iterator tit2;
    TransSet::Iterator tit2_end;

    // initialize result with all self loops
    TransSet xTrans;
    StateSet::Iterator sit = rPGen.StatesBegin();
    for(;sit!=rPGen.StatesEnd();sit++){
        Int k = rPGen.GlobalAttribute().PLowest();
        for(;k>=0;k--){
            xTrans.Insert(*sit,SilentEvAtPrio(rPGen,rSilent,k),*sit);
        }
    }
    TransSet newtrans = xTrans; // initialize iteration
    // loop for fixpoint
    while(!newtrans.Empty()) {
        // store new transitions for next iteration
        TransSet nextnewtrans;
        // loop over all transitions in newtrans
        tit1=newtrans.Begin();
        tit1_end=newtrans.End();
        for(;tit1!=tit1_end; ++tit1) {
            if (_mode==0 && !rSilent.Exists(tit1->Ev)) continue; // in mode 0, do not saturate non-sil trans
            tit2=rPGen.TransRelBegin(tit1->X2);
            tit2_end=rPGen.TransRelEnd(tit1->X2);
            Idx tit1EvPrio = rPGen.EventAttribute(tit1->Ev).Priority();
            for(;tit2!=tit2_end; ++tit2) {
                Idx tit2EvPrio = rPGen.EventAttribute(tit2->Ev).Priority();
                // saturate the lower priority silent transition
                // copy first tit1's event
                if (rSilent.Exists(tit2->Ev)){ // if the next trans is silent
                    if (rSilent.Exists(tit1->Ev)){ // both tit1's ev and tit2's are silent
                        // to saturate, if must be at priority not lower than tit1's ev, and the
                        // active ev inclusion must hold
                        if (tit2EvPrio <= tit1EvPrio
                            && NonsilHigherThen(rPGen,rSilent,tit2->X1,tit1EvPrio)<=NonsilHigherThen(rPGen,rSilent,tit1->X1,tit1EvPrio)){
                            if (!xTrans.Exists(tit1->X1,tit1->Ev,tit2->X2))
                            nextnewtrans.Insert(tit1->X1,tit1->Ev,tit2->X2);
                        }
                    }
                    else if (_mode==1){ // tit1 not silent, then only saturate in mode1
                        if (tit2EvPrio == 0){
                            if (!xTrans.Exists(tit1->X1,tit1->Ev,tit2->X2))
                            nextnewtrans.Insert(tit1->X1,tit1->Ev,tit2->X2);
                        }
                    }
                }
                else{ // the next transition is not silent
                    // we also need to check that tit1 is silent
                    if (rSilent.Exists(tit1->Ev)
                        && tit2EvPrio >= tit1EvPrio
                        && NonsilHigherThen(rPGen,rSilent,tit2->X1,tit1EvPrio)<=NonsilHigherThen(rPGen,rSilent,tit1->X1,tit1EvPrio)){
                        if (!xTrans.Exists(tit1->X1,tit2->Ev,tit2->X2)){
                            if (_mode==0)
                                xTrans.Insert(tit1->X1,tit2->Ev,tit2->X2); // no need to iterate again in mode 0
                            else
                                nextnewtrans.Insert(tit1->X1,tit2->Ev,tit2->X2);
                            }
                    }
                }
            }
        }
        // insert new transitions
        xTrans.InsertSet(nextnewtrans);
        // update trans for next iteration (saturation front)
        newtrans = nextnewtrans;
    }
    rResult.InjectTransRel(xTrans);
}


// detect livelocks and store in rResult. rResult is a map of livelock->the SILENT EV whose priority describes the livelo
// NOTE: WELL-FORMEDNESS REQUIRED!
void DetectLiveLocks(const pGenerator &rPGen, const EventSet &rSilent, std::map<StateSet, Idx>& rResult){
    rResult.clear();

    // have a generator copy where only silent transitions are preserved
    pGenerator copyg(rPGen);
    TransSet::Iterator tit = copyg.TransRel().Begin();
    while (tit!=copyg.TransRelEnd()){
        if (!rSilent.Exists(tit->Ev)) // if not a silent trans, delete
            copyg.ClrTransition(tit++);
        else tit++;
    }

    // compute eqclass and merge on original generator
    std::list<StateSet> eqclasses;
    StateSet dummy;
    ComputeScc(copyg,eqclasses,dummy);

    // delete spurious SCCs which are: trivial but non-selflooping, loop but with silent exit
    // note: well-formedness required
    std::list<StateSet>::const_iterator sccit = eqclasses.begin();
    for (;sccit!=eqclasses.end();sccit++){
        if ((*sccit).Size()==1){
            EventSet activesil = rPGen.ActiveEventSet(*sccit->Begin()) * rSilent;
            if (!activesil.Empty()) {
                TransSet::Iterator tit = rPGen.TransRelBegin(*sccit->Begin(),*activesil.Begin());
                for (;tit!=rPGen.TransRelEnd(*sccit->Begin(),*activesil.Begin());tit++){
                    if (tit->X2!=tit->X1){ // there is a silent exit (regardless the existence of a sil selfloop)
                        break;
                    }
                }
                if (tit==rPGen.TransRelEnd(*sccit->Begin(),*activesil.Begin())){ // break was not executed
                    rResult[*sccit] = *activesil.Begin();
                }
            }
        }
        else{ // a SCC with at least two states
            StateSet::Iterator sit = sccit->Begin();
            Idx livelockprio = 0; // initialise livelock lowest priority
            bool skip = false; // set to true if *sccit is not a livelock
            for(;sit!=sccit->End();sit++){
                EventSet activesil = rPGen.ActiveEventSet(*sit) * rSilent;
                if (!activesil.Empty()) {
                    TransSet::Iterator tit = rPGen.TransRelBegin(*sit,*activesil.Begin());
                    for (;tit!=rPGen.TransRelEnd(*sit,*activesil.Begin());tit++){
                        if (!(*sccit).Exists(tit->X2)){ // there is a silent exit
                            skip = true;
                            break;
                        }
                    }
                    if (skip) break;
                    Idx newprio = rPGen.EventAttribute(*activesil.Begin()).Priority();
                    if (newprio>livelockprio) livelockprio = newprio;
                }
            }
            if (!skip){
                rResult[*sccit] = SilentEvAtPrio(rPGen,rSilent,livelockprio);
            }
        }
    }
}


// typedef of incoming transitions. The 3 elements in the tuple are referred to as:
//     0: from which state (=0 as "any initial state")
//     1: via which (non-silent) event (=0 as "silently from any initial state")
//     2: all silent prio events, each of which is denotes a hookarrow transition
typedef std::map<std::pair<Idx, Idx>, EventSet> IncTransSet;

// compute the incoming non-tau trans set of a state in a saturated
// and tau-removed generator. Instead of directly return a
// set of TransSet, a set of pair <Idx (source state), Idx (ev)>
// is returned for a better performance, hopefully. Moreover,
// for the sake of reachability from some intial state, a special pair
// <0,0> indicates that this state can be silently reached from some
// initial state.
// ******input arguments:
//         rClosureFull: a result from SaturatePDelay with mode = 0, i.e. closure of =>->_Sig:k
//         rClosureLowest: a restricted result of SaturateLowestPrio
// ******return bool: false if some silent pred has higher prio non-sil ev
bool IncomingTransSet(
        const pGenerator& rPGen,
        const pGenerator& rClosureFull,
        const pGenerator& rClosureLowest,
        const TransSetX2EvX1& rtrans_full,
        const TransSetX2EvX1& rtrans_lowest,
        const EventSet& rSilent,
        const Idx& rState,
        IncTransSet& rResult){
    TransSetX2EvX1 trans_lowest = rtrans_lowest;
    rResult.clear();
    // 1. test if there are silent pred with higher prio non-sil ev
    // if a silent pred can execute tau, then (due to wf) this tau must reach
    // rState.
    TransSetX2EvX1::Iterator rtit = rtrans_full.BeginByX2(rState);
    for(;rtit!=rtrans_full.EndByX2(rState);rtit++){
        if (!rSilent.Exists(rtit->Ev)) continue;
        EventSet activesil = rPGen.ActiveEventSet(rtit->X1) * rSilent;
        if (activesil.Empty()) continue;
        Idx k = rPGen.EventAttribute(*activesil.Begin()).Priority();
        if (!NonsilHigherThen(rPGen,rSilent,rtit->X1,k).Empty()) return false; // abort if test fails
    }
    // 2. test passed. compute incoming transset
    TransSetX2EvX1::Iterator rtit_lowest = trans_lowest.BeginByX2(rState);
    for (;rtit_lowest!=trans_lowest.EndByX2(rState);rtit_lowest++){
        EventSet ks;
        ks.Insert(rtit_lowest->Ev);
        // collect all possible hookarrows to rtit->X2
        TransSet::Iterator tit = rClosureLowest.TransRelBegin(rtit_lowest->X1);
        for(;tit!=rClosureLowest.TransRelEnd(rtit_lowest->X1);tit++){
            if (tit->X2 != rtit_lowest->X2
                || tit->Ev == rtit_lowest->Ev) continue; // skip self or wrong successor
            ks.Insert(tit->Ev);
            trans_lowest.Erase(*tit); // this cannot be the same as rtit, so OK
        }
        // find all non-sil predeccessors (or check if is initial state)
        if (rClosureFull.InitStates().Exists(rtit_lowest->X1)){
            std::pair<Idx,Idx> cstate = std::make_pair(0,0);
            IncTransSet::iterator visited = rResult.find(cstate);
            if(visited == rResult.end()){
                rResult[cstate] = ks;
            }
            else{
                rResult[cstate].InsertSet(ks);
            }
        }
        TransSetX2EvX1::Iterator rtit_original = rtrans_full.BeginByX2(rtit_lowest->X1);
        for(;rtit_original != rtrans_full.EndByX2(rtit_lowest->X1);rtit_original++){
            if (rSilent.Exists(rtit_original->Ev)) continue; // skip silent event
            std::pair<Idx,Idx> cstate = std::make_pair(rtit_original->X1,rtit_original->Ev);
            IncTransSet::iterator visited = rResult.find(cstate);
            if(visited == rResult.end()){
                rResult[cstate] = ks;
            }
            else{
                rResult[cstate].InsertSet(ks);
            }
        }
    }
    return true; // TM2025
}

// return eq classes of =_inc. Instead of returning list of state sets,
// this function retunrs a map where the keys are the (source state, (non-tau)event) - pairs
// and the value of each key is the corresponding eq class. Effectively, the keys are just
// for efficient internal search of existing classes and will not be utilised in any other
// functions which requies incoming eq (they will only need the values)
void IncomingEquivalentClasses(const pGenerator& rPGen, const EventSet& rSilent, std::list<StateSet>& rResult){
    // Preparation: saturate closureFull and closureLowest (necessary for incomingtransset)
    pGenerator closurefull;
    SaturatePDelayed(rPGen,rSilent,0,closurefull); // do not saturate the successive 0 part
    pGenerator closurelowest;
    // since hookarrow does not want the silent transitions to have active higher priority events
    pGenerator rPGen_shaped(rPGen);
    ShapePriorities(rPGen_shaped);
    SaturateLowestPrio(rPGen_shaped,rSilent,closurelowest);

    TransSetX2EvX1 trans_lowest; // buffer the rev trans of closurelowest
    closurelowest.TransRel().ReSort(trans_lowest);
    TransSetX2EvX1 trans_full; // buffer the rev trans of original gen
    closurefull.TransRel().ReSort(trans_full);

    // start the incoming equivalence partition
    StateSet::Iterator sit = rPGen.StatesBegin();
    StateSet::Iterator sit_end = rPGen.StatesEnd();
    // set up a map <incoming transset->class> to fast access incoming transset
    // of existing classes. This map is only meant to check incoming equivalence.
    std::map<IncTransSet,StateSet> incomingeqclasses;
    std::map<IncTransSet,StateSet>::iterator mit;
    for(;sit!=sit_end;sit++){
        IncTransSet rec_income;
        bool ok = IncomingTransSet(rPGen,closurefull,closurelowest,trans_full,trans_lowest,rSilent,*sit,rec_income);
        if (!ok) continue;
        if (rec_income.empty()) continue; // this should be redundant, as covered by "ok" above
        mit = incomingeqclasses.find(rec_income);
        if (mit != incomingeqclasses.end()){ // existing class found
            mit->second.Insert(*sit);
        }
        else { // no class for this state found
            StateSet newclass;
            newclass.Insert(*sit);
            incomingeqclasses[rec_income] = newclass;
        }
    }

    // return non-trivial classes
    rResult.clear();
    mit = incomingeqclasses.begin();
    for(;mit!=incomingeqclasses.end();mit++){
        if (mit->second.Size()>1) rResult.push_back(mit->second);
    }
}

// partition according to active events rule (AE) or silent continuation rule (SC)
void AESCClasses(const pGenerator& rPGen, const EventSet& rSilent, std::list<StateSet>& rResult){
    rResult.clear();
    std::map<StateSet, Idx> livelocks; // only need keys, values are livelock prios which are irrelevant
    DetectLiveLocks(rPGen,rSilent,livelocks);
    //
    std::list<StateSet> inceq;
    IncomingEquivalentClasses(rPGen,rSilent,inceq); //prepartition inceq into result. then refine
    std::list<StateSet>::const_iterator inceqit = inceq.begin();
    // refine each inceq class
    for(;inceqit!=inceq.end();inceqit++){
        // never equalise states in livelocks
        std::map<StateSet,Idx>::const_iterator livelocksit = livelocks.begin();
        for(;livelocksit!=livelocks.end();livelocksit++){
            if (!(livelocksit->first * (*inceqit)).Empty())
                break;
        }
        if  (livelocksit!=livelocks.end())
            continue;
        StateSet todo = *inceqit;
        // refine this eq class
        while (!todo.Empty()){
            Idx cstate = *todo.Begin();
            todo.Erase(todo.Begin());
            StateSet newclass;
            newclass.Insert(cstate);
            EventSet activesil = rPGen.ActiveEventSet(cstate)*rSilent;
            if (activesil.Empty()){ // no active silent evs. try AE
                StateSet::Iterator sit = todo.Begin();
                while(sit!=todo.End()){
                    if (rPGen.ActiveEventSet(*sit) == rPGen.ActiveEventSet(cstate)){
                        newclass.Insert(*sit);
                        todo.Erase(sit++);
                    }
                    else
                        sit++;
                }
            }
            else{ // then try SC
                StateSet::Iterator sit = todo.Begin();
                while(sit!=todo.End()){
                    Idx tauprio = rPGen.EventAttribute(*activesil.Begin()).Priority();
                    if ((rPGen.ActiveEventSet(*sit)*rSilent) == activesil // same active silent event
                        &&
                        NonsilHigherThen(rPGen,rSilent,*sit,tauprio).Size()+ NonsilHigherThen(rPGen,rSilent,cstate,tauprio).Size()==0){
                        newclass.Insert(*sit);
                        todo.Erase(sit++);
                    }
                    else
                        sit++;
                }
            }
            if (newclass.Size()>1) rResult.push_back(newclass);
        }
    }
}

void MergeAESC(pGenerator& rPGen, const EventSet& rSilent){
    std::map<StateSet, Idx> livelocks;
    DetectLiveLocks(rPGen,rSilent,livelocks);
    std::list<StateSet> eqclasses;
    AESCClasses(rPGen, rSilent, eqclasses);
    MergeEquivalenceClasses(rPGen,rSilent,livelocks,eqclasses);
}

// saturate pwb by connecting =>_<k (with target state not having tau<k) and =>_Sig:k closure
void SaturatePWB(const pGenerator& rPGen,
                 const EventSet& rSilent,
                 pGenerator& rResult){
    rResult = rPGen;
    pGenerator closurelowest;
    SaturateLowestPrio(rPGen,rSilent,closurelowest);
    pGenerator closurefull;
    SaturatePDelayed(rPGen,rSilent, 1,closurefull);
    // figure out the highest possible k for =>_<k (with target not tau<k) transitions for each two states
    TransSet resulttrans; // put saturated trans here. Inject to rResult eventually
    StateSet::Iterator sit = closurelowest.StatesBegin();
    for(;sit!=closurelowest.StatesEnd();sit++){
        StateSet visitedtarget; // buffer a set of visited target states of transitions from sit
        TransSet::Iterator tit_lowest = closurelowest.TransRelBegin(*sit);
        for(;tit_lowest!=closurelowest.TransRelEnd(*sit);tit_lowest++){
            if (visitedtarget.Exists(tit_lowest->X2))
                continue;
            visitedtarget.Insert(tit_lowest->X2);
            // find the highest lowest priority to tit->X2 (X2 not <k not treated yet)
            Idx highest = rPGen.EventAttribute(tit_lowest->Ev).Priority();
            if (highest>0){ // 0 must be the highest
                TransSet::Iterator tit2_lowest = closurelowest.TransRelBegin(*sit);
                for(;tit2_lowest!=closurelowest.TransRelEnd(*sit);tit2_lowest++){
                    if (tit2_lowest->X2 != tit_lowest->X2) continue;
                    Idx newprio = rPGen.EventAttribute(tit2_lowest->Ev).Priority();
                    if (newprio < highest) highest = newprio;
                    if (highest==0) break;
                }
            }
            // now check if X2 cannot <k, if k>0
            // in the following, highest's meaning is changed to "the highest k tit allows for successive transitions"
            bool iszero = highest==0? true:false; // highest = 0 is a special case
                                                  // (does not need ++ to allow successive 0 trans)
            highest++; // for highest \neq 0, the highest allowed prioity is acturally highest++
            Idx lowest = rPGen.GlobalAttribute().PLowest(); // allowed lowest priority, changed if some tau is active on X2
            EventSet activesil = rPGen.ActiveEventSet(tit_lowest->X2) * rSilent;
            if (!activesil.Empty()){
                Idx k_activesil = rPGen.EventAttribute(*activesil.Begin()).Priority();
                lowest = k_activesil;
                // bail out when transition extension is already impossible
                if (!iszero && lowest < highest)  continue;
            }
            // now we have the highest k allowed. Check for possible trans extension from closurefull
            TransSet::Iterator tit_full = closurefull.TransRelBegin(tit_lowest->X2); // sit is tit->X1
            for(;tit_full!=closurefull.TransRelEnd(tit_lowest->X2);tit_full++){
                Idx k_succ = rPGen.EventAttribute(tit_full->Ev).Priority();
                if ((k_succ >= highest && k_succ <= lowest)
                     ||  (iszero && k_succ==0)){ // its OK!
                    resulttrans.Insert(*sit,tit_full->Ev,tit_full->X2);
                }
            }
        }
    }
    rResult.InjectTransRel(resulttrans);
}

void MergePWB(pGenerator& rPGen, const EventSet& rSilent){
    std::map<StateSet, Idx> livelocks;
    DetectLiveLocks(rPGen,rSilent,livelocks);
    pGenerator pwbclosure;
    SaturatePWB(rPGen,rSilent,pwbclosure);
    std::list<StateSet> eqclasses;
    ComputeBisimulation(pwbclosure,eqclasses);
    MergeEquivalenceClasses(rPGen,rSilent,livelocks,eqclasses);
}


void RedundantSilentStep(pGenerator& rPGen, const EventSet& rSilent){
    TransSetX2EvX1 rtrans;
    rPGen.TransRel().ReSort(rtrans);
    StateSet::Iterator sit = rPGen.StatesBegin();
    while(sit!=rPGen.StatesEnd()){
        if ((rPGen.InitStates() + rPGen.MarkedStates()).Exists(*sit)) {sit++;continue;}
        TransSet::Iterator tit = rPGen.TransRelBegin(*sit);
        Idx count = 0;
        for(;tit!=rPGen.TransRelEnd(*sit);tit++){
            count++;
        }
        if (count!=1) {sit++;continue;} // only have one outgoing trans
        tit = rPGen.TransRelBegin(*sit);
        if (!rSilent.Exists(tit->Ev)) {sit++;continue;} // and this trans must be silent
        // are all incoming trans silent with no higher priority?
        Idx k = rPGen.EventAttribute(tit->Ev).Priority();
        TransSetX2EvX1::Iterator rtit = rtrans.BeginByX2(*sit);
        for(;rtit!=rtrans.EndByX2(*sit);rtit++){
            if (!rSilent.Exists(rtit->Ev)) break;
            if (rPGen.EventAttribute(rtit->Ev).Priority()<=k) break;
        }
        if (rtit!=rtrans.EndByX2(*sit)) {sit++;continue;}
        // sit qualified. Remove and redirect transition
        rtit=rtrans.BeginByX2(*sit);
        for(;rtit!=rtrans.EndByX2(*sit);rtit++){
            rPGen.SetTransition(rtit->X1,rtit->Ev,tit->X2);
        }
        StateSet::Iterator todelete = sit++;
        rPGen.DelState(*todelete);
        // update rtrans
        rtrans.Clear();
        rPGen.TransRel().ReSort(rtrans);
    }
}

// input automaton should be redundant silent loop free
void OnlySilentIncoming(pGenerator& rPGen, const EventSet& rSilent, const StateSet& rLivelocks){
    // figure states with only silent incomming transitions
    TransSetX2EvX1 rtrans;
    rPGen.TransRel().ReSort(rtrans);
    // pre-filter uncapable states
    StateSet cand=rPGen.States()-rPGen.InitStates()-rLivelocks; // lasily exclude initial states
    TransSet::Iterator tit = rPGen.TransRelBegin();
    TransSet::Iterator tit_end = rPGen.TransRelEnd();
    for(;tit!=tit_end;++tit){
        if(!rSilent.Exists(tit->Ev)
           || rPGen.EventAttribute(tit->Ev).Priority()!=0) cand.Erase(tit->X2);
    }
    // bail out on trivial
    if(cand.Size()==0)  return;

    StateSet::Iterator sit = cand.Begin();
    StateSet::Iterator sit_end = cand.End();
    while (sit!=sit_end){
        TransSet::Iterator tit2 = rPGen.TransRelBegin(*sit);
        TransSet::Iterator tit2_end = rPGen.TransRelEnd(*sit);
        for (;tit2!=tit2_end;tit2++){
            if (rSilent.Exists(tit2->Ev)&&rPGen.EventAttribute(tit2->Ev).Priority()==0){
                break;
            }
        }
        if (tit2!=tit2_end) { // there is at least one tau_0 outgoing
            // redirect transitions
            TransSetX2EvX1::Iterator rtit = rtrans.BeginByX2(*sit);
            TransSetX2EvX1::Iterator rtit_end = rtrans.EndByX2(*sit);
            for(;rtit!=rtit_end;rtit++){
                TransSet::Iterator tit3 = rPGen.TransRelBegin(*sit);
                TransSet::Iterator tit3_end = rPGen.TransRelEnd(*sit);
                for (;tit3!=tit3_end;tit3++){
                    if (rtit->X1 == tit3->X2 && rSilent.Exists(tit3->Ev)) continue; // avoid generating silent self loop
                    rPGen.SetTransition(rtit->X1,tit3->Ev,tit3->X2);
                }
                // if the state to delete is marked, then pred-state should be marked as well
                if (rPGen.ExistsMarkedState(*sit))
                    rPGen.SetMarkedState(rtit->X1);
            }
            StateSet::Iterator todelete = sit++;
            rPGen.DelState(*todelete);
        }
        else sit++;

        // update rtrans
        rtrans.Clear();
        rPGen.TransRel().ReSort(rtrans);
    }
}

void OnlySilentOutgoing(pGenerator& rPGen, const EventSet& rSilent, const StateSet& rLivelocks){
    TransSetX2EvX1 rtrans;
    rPGen.TransRel().ReSort(rtrans);
    StateSet::Iterator sit = rPGen.StatesBegin();
    StateSet::Iterator sit_end = rPGen.StatesEnd();
    while(sit!=sit_end){
        // figure out whether this state is only silent outgoing
        if ((rPGen.MarkedStates()+rLivelocks).Exists(*sit)) {sit++;continue;}
        TransSet::Iterator tit2 = rPGen.TransRelBegin(*sit);
        TransSet::Iterator tit2_end = rPGen.TransRelEnd(*sit);
        if (tit2==tit2_end) {sit++;continue;} // sit points to a deadend state (nonmarked state without outgoing trans)
        for (;tit2!=tit2_end;tit2++){
            if (!rSilent.Exists(tit2->Ev)) break;// sit has non-silent outgoing trans
            if (rPGen.EventAttribute(tit2->Ev).Priority()!=0) break; // must be with prio = 0
        }
        if (tit2 != tit2_end) {sit++;continue;}

        // sit has passed the test. relink outgoing transitions of predecessor
        // (repair initial state quicker by first iterate outgoing trans)
        tit2 = rPGen.TransRelBegin(*sit);
        tit2_end = rPGen.TransRelEnd(*sit);
        for (;tit2!=tit2_end;tit2++){
            TransSetX2EvX1::Iterator rtit = rtrans.BeginByX2(*sit); // incoming trans to *sit
            TransSetX2EvX1::Iterator rtit_end = rtrans.EndByX2(*sit);
            for (;rtit!=rtit_end;rtit++){
                if (rtit->X1 == tit2->X2 && rSilent.Exists( rtit->Ev)) continue; // avoid generating silent self loop
                rPGen.SetTransition(rtit->X1,rtit->Ev,tit2->X2);
            }
            if (rPGen.ExistsInitState(*sit))
                rPGen.SetInitState(tit2->X2);
        }
        StateSet::Iterator todelete = sit++;
        rPGen.DelState(*todelete);

        // update rtrans
        rtrans.Clear();
        rPGen.TransRel().ReSort(rtrans);
    }
}

// return the set of set of strongly coacc states depending on
// the fairness constraints.
// NOTE: this shall be considered as a member function of pGenerator,
// to reimplement
StateSet StronglyCoaccessibleSet(const pGenerator& rPGen){
    Generator copy = rPGen;
    copy.Accessible();
    StateSet result = copy.States();
    Fairness::const_iterator fairit = rPGen.GlobalAttribute().FairConsts().begin();
    for(;fairit!=rPGen.GlobalAttribute().FairConsts().end();fairit++){
        copy.ClearMarkedStates();
        StateSet::Iterator sit = copy.StatesBegin();
        for(;sit!=copy.StatesEnd();sit++){
            // if a state can execute a fair event, it's "marked" for this fairness constraint
            if(!(copy.ActiveEventSet(*sit) * (*fairit)).Empty()){
                copy.SetMarkedState(*sit);
            }
        }
        result = result * copy.CoaccessibleSet(); //this is the ordinary function for marked states

    }
    return result;
}

// Certain conflicts. see cited literature 3.2.3
// -- remove outgoing transitions from not coaccessible states
void RemoveNonCoaccessibleOut(pGenerator& rPGen){
    StateSet notcoacc=rPGen.States()-StronglyCoaccessibleSet(rPGen);
    StateSet::Iterator sit=notcoacc.Begin();
    StateSet::Iterator sit_end=notcoacc.End();
    for(;sit!=sit_end;++sit){
        TransSet::Iterator tit=rPGen.TransRelBegin(*sit);
        TransSet::Iterator tit_end=rPGen.TransRelEnd(*sit);
        while(tit!=tit_end)
            rPGen.ClrTransition(tit++);
    }
    rPGen.Accessible();
}

// Certain conflicts. see cited literature 3.2.3
// -- remove outgoing transitions from states that block by a silent event
void BlockingSilentEvent(pGenerator& rPGen,const EventSet& rSilent){
    FD_DF("BlockingSilentEvent(): prepare for t#"<<g.TransRelSize());
    StateSet coacc=StronglyCoaccessibleSet(rPGen);
    StateSet sblock; // states with outgoing silent event with highest active priority and leads to block
    TransSet::Iterator tit;
    TransSet::Iterator tit_end;
    StateSet::Iterator sit;
    StateSet::Iterator sit_end;
    // loop all transitions to figure certain blocking states
    tit=rPGen.TransRelBegin();
    tit_end=rPGen.TransRelEnd();
    for(;tit!=tit_end;++tit) {
        if(rSilent.Exists(tit->Ev))
            if(!coacc.Exists(tit->X2)){
                // is this silent event with the highest priority at this state?
                EventSet active = rPGen.ActiveEventSet(tit->X1);
                EventSet::Iterator eit = active.Begin();
                Idx highest = rPGen.GlobalAttribute().PLowest();
                for(;eit!=active.End();eit++){
                    if (rPGen.EventAttribute(*eit).Priority()<highest)
                        highest = rPGen.EventAttribute(*eit).Priority();
                }
                if (rPGen.EventAttribute(tit->Ev).Priority()==highest)
                    sblock.Insert(tit->X1);
            }
    }
    // unmark blocking states and eliminate possible future
    sit=sblock.Begin();
    sit_end=sblock.End();
    for(;sit!=sit_end;++sit) {
        rPGen.ClrMarkedState(*sit);
        tit=rPGen.TransRelBegin(*sit);
        tit_end=rPGen.TransRelEnd(*sit);
        while(tit!=tit_end)
            rPGen.ClrTransition(tit++);
    }
    rPGen.Accessible();
    FD_DF("BlockingSilentEvent(): done with t#"<<g.TransRelSize());
}

// -- merge all states that block to one representative
void MergeNonCoaccessible(pGenerator& rPGen){
    StateSet notcoacc=rPGen.States()-StronglyCoaccessibleSet(rPGen);
    // bail out on trivial case
    if(notcoacc.Size()<2) return;
    // bail out with blocking init state
    if((notcoacc * rPGen.InitStates()).Size()>0){
        rPGen.DelStates(rPGen.States());
        Idx qnc = rPGen.InsState();
        rPGen.SetInitState(qnc);
        return;
    }
    // have a new state
    Idx qnc=rPGen.InsState();
    // fix transitions by iterating over predecessors of notcoacc
    TransSetX2EvX1 rtrans;
    rPGen.TransRel().ReSort(rtrans);
    StateSet::Iterator sit = notcoacc.Begin();
    for(;sit!=notcoacc.End();sit++){
        TransSetX2EvX1::Iterator rtit = rtrans.BeginByX2(*sit);
        for(;rtit!=rtrans.EndByX2(*sit);rtit++){
            rPGen.SetTransition(rtit->X1,rtit->Ev,qnc);
        }
    }
    // delete original not coacc
    rPGen.DelStates(notcoacc);
}


// -- if a transition blocks, remove all transitions from the same state with the same label
void BlockingEvent(pGenerator& rPGen,const EventSet& rSilent){
    FD_DF("BlockingEvent(): prepare for t#"<<g.TransRelSize());
    StateSet coacc=StronglyCoaccessibleSet(rPGen);
    StateSet notcoacc = rPGen.States()-coacc;
    TransSetX2EvX1 rtrans;
    rPGen.TransRel().ReSort(rtrans);
    TransSet::Iterator tit;
    TransSet::Iterator tit_end;
    StateSet::Iterator sit;
    StateSet::Iterator sit_end;
    TransSetX2EvX1::Iterator rtit;
    TransSetX2EvX1::Iterator rtit_end;
    TransSet deltrans;
    // loop over notcoacc (normally smaller than coacc) and find coacc pred
    sit = notcoacc.Begin();
    sit_end = notcoacc.End();
    for(;sit!=sit_end;sit++){
        rtit = rtrans.BeginByX2(*sit);
        rtit_end = rtrans.EndByX2(*sit);
        for(;rtit!=rtit_end;rtit++){
            if (rSilent.Exists(rtit->Ev)) continue; // silent blocking evs are handled elsewhere
            if (!coacc.Exists(rtit->X1)) continue; // incoming trans from coacc
            // current rtit is such that X1 is coacc but X2 is not, Ev is nonsilent
            // record transitions from the X1 with the same event BUT not rtit
            tit = rPGen.TransRelBegin(rtit->X1);
            tit_end = rPGen.TransRelEnd(rtit->X1);
            for(;tit!=tit_end;tit++){
                if(tit->X2 == rtit->X2) continue; // skip for tit==rtit
                if(tit->Ev == rtit->Ev)
                    deltrans.Insert(*tit);
            }
        }
    }
    // delete transitions
    tit=deltrans.Begin();
    tit_end=deltrans.End();
    for(;tit!=tit_end;++tit)
        rPGen.ClrTransition(*tit);
    FD_DF("BlockingEvent(): done with t#"<<g.TransRelSize());
}

void PConflictPreservingAbstraction(
        pGenerator& rPGen,
        const EventSet& rSilent){
    if (rSilent.Empty()) return;
    // preparation: augment fictive silent events if not existing in rPGen's alphabet
    EventSet augsilent = rSilent;
    std::set<Idx> existingsil_k; // record existing silent ev priorities
    EventSet::Iterator eit = rSilent.Begin(); // NOTE: rSilent should be a subset of rPGen.Alphabet
    for(;eit!=rSilent.End();eit++){
        existingsil_k.insert(rPGen.EventAttribute(*eit).Priority());
    }
    Idx k = 0;
    for(;k<=rPGen.GlobalAttribute().PLowest();k++){
        if (existingsil_k.find(k) != existingsil_k.end()) continue;
        std::string silevname = ":::" + ToStringInteger(k);
        if (rPGen.Alphabet().Exists(silevname)){
            std::cout<<"PConflictPreservingAbstraction()::" +rPGen.Name()
                       +" contains invalid event name (do not begin with :::)";
            throw;
        }
        Idx silev = rPGen.InsEvent(silevname);
        rPGen.EventAttributep(silev)->Priority(k);
        augsilent.Insert(silev);
    }
    ShapeUpsilon(rPGen,augsilent); // enforce well-formedness
    rPGen.Accessible();


    // certain conflicts
    // ******************NOTE: empty fairness constraints = trivial Nonblocking!
    if (rPGen.GlobalAttribute().FairConsts().size()>0){ // cosmetically skip
        BlockingEvent(rPGen,augsilent);
        BlockingSilentEvent(rPGen,augsilent);
        RemoveNonCoaccessibleOut(rPGen);
        MergeNonCoaccessible(rPGen);
    }
    // redundant silent step rule
    RedundantSilentStep(rPGen,augsilent); // quick, may produce silent selfloop, guarantee well-formedness
    // only silent in/out rules
    RemoveRedSilentSelfloops(rPGen,augsilent); // no redundant silent loops any more
    std::map<StateSet,Idx> livelocks;
    DetectLiveLocks(rPGen,augsilent,livelocks);
    StateSet skipstates;
    std::map<StateSet,Idx>::const_iterator livelockit = livelocks.begin();
    for(;livelockit!=livelocks.end();livelockit++){
        skipstates.InsertSet(livelockit->first);
    }
    OnlySilentIncoming(rPGen,augsilent,skipstates);
    OnlySilentOutgoing(rPGen,augsilent,skipstates);
    // AESC and PWB
    MergePWB(rPGen,augsilent);
    MergeAESC(rPGen,augsilent);

    // cosmetically set clean state names
    rPGen.SetDefaultStateNames();

    // OPTIONAL: remove augmented silent events
    EventSet remove = augsilent - rSilent;
    if (!remove.Empty())
        rPGen.DelEvents(remove);
}

bool IsHideable(const pGenerator& rPGen,
                const EventSet& rSilent,
                const Transition& rTrans){
    if (!rSilent.Exists(rTrans.Ev)) return false; // this is not a silent event at all (e.g. _OMEGA_)
    Fairness relevant; // collect all fairness constraints containing rTrans->Ev
    Fairness::const_iterator fairit = rPGen.GlobalAttribute().FairConsts().begin();
    for(;fairit!=rPGen.GlobalAttribute().FairConsts().end();fairit++){
        if (fairit->Exists(rTrans.Ev))
            relevant.insert(*fairit);
    }
    if (relevant.empty()) return true; // this event is not in any fairness constraint
    // get the =>_Sig:k closure from rTrans->X2
    StateSet todo;
    StateSet visited;
    const Idx k = rPGen.EventAttribute(rTrans.Ev).Priority();
    const EventSet x1active = NonsilHigherThen(rPGen,rSilent,rTrans.X1,k);
    if (NonsilHigherThen(rPGen,rSilent,rTrans.X2,k)<=x1active){
        todo.Insert(rTrans.X2);
    }
    while (!todo.Empty()){
        Idx cstate = *todo.Begin();
        todo.Erase(todo.Begin());
        visited.Insert(cstate);
        Fairness relevant_copy = relevant; // buffer a copy of relevant fairness constraints
        StateSet todo_new; // buffer a state set for qualified silent successors
        TransSet::Iterator tit = rPGen.TransRelBegin(cstate);
        for(;tit!=rPGen.TransRelEnd(cstate);tit++){
            if (!rSilent.Exists(tit->Ev)) continue;
            if (rPGen.EventAttribute(tit->Ev).Priority()>k) continue; // required both for colouring or silent successor
            if (*tit == rTrans) continue;
            // how many relevant fairness constraints are satisfied?
            Fairness::iterator fairit = relevant_copy.begin();
            while(fairit!=relevant_copy.end()){
                if (fairit->Exists(tit->Ev)){
                    relevant_copy.erase(fairit++); // remove when this fairness cons is satisfied
                }
                else fairit++;
            }
            if (relevant_copy.empty()) return true;
            // is tit->X2 a suitable silent successor?
            if (NonsilHigherThen(rPGen,rSilent,tit->X2,k)<=x1active){
                todo_new.Insert(tit->X2);
            }
        }
        todo.InsertSet(todo_new - visited);
    }
    return false;
}

// substitute all private events with a new event with the same priority
// while also fix the private event set rPrivate
// THE fairness version
//  pUnhideable: a set of always unhideable events
EventSet HidePriviateEvs(pGenerator& rPGen,
                         EventSet& rPrivate,
                         EventPriorities& rPrios,
                         Idx& rTau,
                         const EventSet* pUnHideable = nullptr){
    EventSet result;
    EventSet msilentevs=rPGen.Alphabet() * rPrivate;
    if(msilentevs.Empty()){
        return result;
    }
    // install tau events to alphabet
    std::vector<Idx> tauevs; // buffer a vector for fast search. Position index = priority
    tauevs.reserve(rPGen.GlobalAttribute().PLowest()+1);
    Idx k = 0; // priority
    for(;k<=rPGen.GlobalAttribute().PLowest();k++){
        std::string tauevname = "__TAU"+ToStringInteger(rTau)+":"+ToStringInteger(k);
        if (rPGen.Alphabet().Exists(tauevname)){
            std::cout<<"HidePrivateEvs:: Generator "<<rPGen.Name()
                     <<" contains illegal event name: "<<tauevname;
            throw;
        }
        Idx tauev = rPGen.InsEvent(tauevname);
        tauevs.push_back(tauev); // the index coincides the k value
        result.Insert(tauev);
        rPGen.EventAttributep(tauev)->Priority(k);
        rPrios.InsPriority(tauevname,k);
    }
    rTau++; // updates tau index
    TransSet::Iterator tit = rPGen.TransRelBegin();
    EventSet unhideable;
    if (pUnHideable != nullptr) unhideable = *pUnHideable;
    while(tit!=rPGen.TransRelEnd()){
        if (!IsHideable(rPGen,msilentevs+result,*tit)||result.Exists(tit->Ev)||unhideable.Exists(tit->Ev)){
            tit++;
            continue;
        }
        Transition trans(tit->X1,tauevs.at(rPGen.EventAttribute(tit->Ev).Priority()),tit->X2);
        rPGen.SetTransition(trans);
        rPGen.ClrTransition(tit++);
    }
    rPrivate.EraseSet(msilentevs); // benefits other automata
    return result;
}

// monolithic fairness check
// NOTE: Does not shape. For prioritised automaton consider Shape first.
bool IsStronglyNonblocking(const pGenerator& rPGen){
    Generator copy = rPGen;
    copy.Accessible();
    Fairness::const_iterator fairit = rPGen.GlobalAttribute().FairConsts().begin();
    for(;fairit!=rPGen.GlobalAttribute().FairConsts().end();fairit++){
        copy.ClearMarkedStates();
        StateSet::Iterator sit = copy.StatesBegin();
        for(;sit!=copy.StatesEnd();sit++){
            // if a state can execute a fair event, it's "marked" for this fairness constraint
            if(!(copy.ActiveEventSet(*sit) * (*fairit)).Empty()){
                copy.SetMarkedState(*sit);
            }
        }
        if (!faudes::IsNonblocking(copy)){
            StateSet blocking = copy.BlockingStates();
            std::cout<<"****** THE BLOCKING: "+ToStringInteger(*blocking.Begin())<<std::endl;
            return false; // fails at this fairness constraint
        }
    }
    return true;
}


// merge the fairness constraint of two pGenerators
void MergeFairness(const pGenerator& rPGen1, const pGenerator& rPGen2, Fairness& rFairRes){
    rFairRes = rPGen1.GlobalAttribute().FairConsts();
    Fairness::const_iterator fairit = rPGen2.GlobalAttribute().FairConsts().begin();
    for(;fairit!=rPGen2.GlobalAttribute().FairConsts().end();fairit++){
        rFairRes.insert(*fairit);
    }
}


// the main function for non-conflict check
// **************
// This function includes multiple functionalities which are configured by pFairVec
//     pFairVec: if not specified, treat omega-termination as the acceptance condition
// (New plug-in style interface, no explicit fainess, TM 2025)
bool IsPNonblocking(const GeneratorVector& rGvec,
                       const EventPriorities& rPrios,
                       const std::vector<Fairness>* pFairVec) {
    // tranform back to YT original data type
    /*
    std::map<std::string, Idx> priomap;  
    NameSet::Iterator eit=rPrios.Begin();
    NameSet::Iterator eit_end=rPrios.End();
    for(;eit!=eit_end;++eit)
      priomap[rPrios.SymbolicName(*eit)]=rPrios.Priority(*eit);
    */  

    // have a writable copy
    EventPriorities prios=rPrios;

    // consistency check for coloured marking
    if(pFairVec!=nullptr){
        if (rGvec.Size()!=pFairVec->size()){
            std::cout<< "InPNonconflicting:: consistency check fails. Size of gen vector and fairness vector unequal."<<std::endl;
            throw;
        }
        Idx git = 0;
        for(;git!=rGvec.Size();git++){
            Fairness::const_iterator fairit = pFairVec->at(git).begin();
            for(;fairit!=pFairVec->at(git).end();fairit++){
                if (!(*fairit<=rGvec.At(git).Alphabet())){
                    std::cout<< "InPNonconflicting:: consistency check fails. Generator "<<rGvec.At(git).Name()
                             <<" contains fairness event not in its alphabet."<<std::endl;
                    throw;
                }
            }
        }
    }
    Idx tau = 0; // index for "full hiding"

    EventSet merging;
    std::map<std::string,Idx>::const_iterator mit;
    // if isSFC==1, set up merging events. Also check priority (disjunct is checked by SFC_Parallel)


    PCOMPVER_VERB0("ISPNonconflicting:: Installing priority and appending omega termination")
    // get lowest priority
    Idx lowest = prios.LowestPriority();
    lowest++; // PWB need this
    // encode priority and fairness information into pgen
    std::vector<pGenerator> pgenvec;
    pgenvec.reserve(rGvec.Size());
    Idx git = 0;
    for(;git!=rGvec.Size();git++){
        pGenerator pgen=rGvec.At(git);
	pgen.Priorities(prios);
        pgen.GlobalAttributep()->SetPLowest(lowest);
        if (pFairVec==nullptr){ // go for omega termination
            AppendOmegaTermination(pgen,prios);
        }
        else{ // install predefined fairness
            pgen.GlobalAttributep()->SetFairConsts(pFairVec->at(git));
        }
        pgenvec.push_back(pgen);
    }
    bool firstCycle = true;
    while (true){
        PCOMPVER_VERB0("========================================")
        PCOMPVER_VERB0("Remaining automata: "<<pgenvec.size())

        // trivial cases
        if(pgenvec.size()==0) return true;
        if(pgenvec.size()==1) break;

        // figure silent events
        EventSet silent, all, shared;
        Idx git = 0;
        while(true){
            all = all+pgenvec.at(git).Alphabet();
            Idx git_next = git+1;
            if (git_next == pgenvec.size()) break;
            for(;git_next!=pgenvec.size();git_next++){
            shared = shared
                  + (pgenvec.at(git).Alphabet())
                  * (pgenvec.at(git_next).Alphabet());
            }
            git++;
        }
        silent=all-shared; // all silent events in all current candidates
        // normalize for one silent event at each priority level per generator, and then abstract.
        // note from the second iteration, this is only necessary for the
        // automaton composed from former candidates. This is realized by
        // the break at the end
        git = 0;
        for(;git!=pgenvec.size();git++){
            //abstraction
            pGenerator& g = pgenvec.at(git);
            PCOMPVER_VERB0("Abstracting Automaton "<< g.Name()<<", with state count: "<<g.Size())
            ShapeUpsilon(g,silent); // enforce well-formedness
            g.Accessible();
            EventSet upsilon = HidePriviateEvs(g,silent,prios,tau,&merging);
            PConflictPreservingAbstraction(g, upsilon);
            PCOMPVER_VERB0("State count after abstraction: "<<g.Size())
            if (!firstCycle) break;
        }
        firstCycle = false;

        // candidate choice heuritics. Branch by different tasks
        Idx imin = 0;
        Idx jmin = 0;

//        // candidat pairs with fewest transitions 'minT'
//        git = 1;
//        for(;git!=pgenvec.size();git++){
//            if(pgenvec.at(git).TransRelSize()<pgenvec.at(imin).TransRelSize());
//                imin = git;
//        }
//        // candidat pairs with most common events 'maxC'
//        git = jmin;
//        Int score=-1;
//        for(; git!=pgenvec.size(); git++){
//            if(git==imin) continue;
//            Int sharedsize = (pgenvec.at(git).Alphabet() * pgenvec.at(imin).Alphabet()).Size();
//            if ( sharedsize > score){
//                jmin = git;
//                score = sharedsize;
//            }
//        }
        // compose candidate pair
        imin = 0;
        jmin = 1;

        // get new private event for SParallel
        EventSet myevs;
        EventSet otherevs;
        git = 0;
        for(;git!=pgenvec.size();git++){
            if (git==imin || git==jmin){
                myevs = myevs + pgenvec.at(git).Alphabet();
            }
            else{
                otherevs = otherevs + pgenvec.at(git).Alphabet();
            }
        }
        EventSet privateevs = myevs-otherevs;

        // *************** compose Gi and Gj and reinstall attributes to gij
        std::vector<pGenerator> newpgenvec;
        pGenerator gij;
        Fairness newfairness;
        PCOMPVER_VERB0("Composing automata "<<pgenvec.at(imin).Name()<<" and "<<pgenvec.at(jmin).Name());
        SUParallel(pgenvec.at(imin),pgenvec.at(jmin),merging,privateevs,prios,gij);
        pGenerator pgij = gij;
	pgij.Priorities(prios);
        newpgenvec.push_back(pgij); // the composed generator is always the first element
        UParallel_MergeFairness(pgenvec.at(imin), pgenvec.at(jmin),gij, merging, newfairness);

        newpgenvec[0].GlobalAttributep()->SetFairConsts(newfairness);
        newpgenvec[0].GlobalAttributep()->SetPLowest(lowest);
        // and retain other uncomposed automata
        git = 0;
        for(;git!=pgenvec.size();git++){
            if (git == imin || git == jmin) continue;
            newpgenvec.push_back(pgenvec.at(git)); // all other candidates are just copied to the next iteraion
        }
        pgenvec = newpgenvec;
    }
    ShapePriorities(pgenvec.at(0));
    pgenvec.at(0).Accessible();
    std::cout<<"Final state count: "<<ToStringInteger(pgenvec.at(0).Size())<<std::endl;
    return IsStronglyNonblocking(pgenvec.at(0));
}

    

 
/*
***********
NEED OLD VERSION DUMMIES TO COMPILE (TM 2025)
******
*/

void PCandidate::MergeSilentLoops(Generator &g, const EventSet &silent){
}

void PCandidate::HidePrivateEvs(EventSet& silent){
    mGenHidden = mGenRaw;
    mGenHidden.Name(mGenRaw.Name() + "_H");
    EventSet msilentevs = (mGenRaw.Alphabet()*silent) - mPevs;
    EventSet mpsilentevs = (mGenRaw.Alphabet()*silent) * mPevs;
    if(msilentevs.Empty() && mpsilentevs.Empty()){
        mGenMerged = mGenHidden;
        mGenMerged.Name(mGenRaw.Name()+"_M");
        return;
    }
    Idx tau;
    Idx ptau;
    if (!msilentevs.Empty()){
        tau=*(msilentevs.Begin());
        SetTau(tau);
        SetSilentevs(msilentevs);
        msilentevs.Erase(tau); // from now on, msilentevs exclude tau
    }
    if (!mpsilentevs.Empty()){
        ptau = *(mpsilentevs.Begin());
        SetPtau(ptau);
        SetPSilentevs(mpsilentevs);
        mpsilentevs.Erase(ptau);
    }

    silent.EraseSet(msilentevs);
    silent.EraseSet(mpsilentevs);

    if (msilentevs.Empty() && mpsilentevs.Empty()){// in this case, only one 1 silent event is set to tau and no need to hide
        mGenMerged = mGenHidden;
        mGenMerged.Name(mGenRaw.Name()+"_M");
        return;
    }
    TransSet::Iterator tit=mGenHidden.TransRelBegin();
    TransSet::Iterator tit_end=mGenHidden.TransRelEnd();
    for(;tit!=tit_end;) {
        if(!(msilentevs+mpsilentevs).Exists(tit->Ev)) {++tit; continue;}
        Transition t;
        if (msilentevs.Exists(tit->Ev))
            t = Transition(tit->X1,tau,tit->X2);
        else
            t = Transition(tit->X1,ptau,tit->X2);
        mGenHidden.ClrTransition(tit++);
        if (!mGenHidden.ExistsTransition(t))
            mGenHidden.SetTransition(t);
    }
    mGenHidden.InjectAlphabet(mGenHidden.Alphabet()-msilentevs-mpsilentevs);
    mGenMerged = mGenHidden;
    mGenMerged.Name(mGenRaw.Name()+"_M");
}

void PCandidate::ObservationEquivalenceQuotient_NonPreemptive(Generator &g,
        const EventSet &silent){
}

void PCandidate::ObservationEquivalenceQuotient_Preemptive(Generator &g, const EventSet &silent, const bool& flag){
}

void PCandidate::ConflictEquivalentAbstraction(EventSet& silent){
}


  
} // namespace faudes
