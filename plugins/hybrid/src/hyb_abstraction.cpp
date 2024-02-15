/** @file hyb_abstraction.cpp  Abstractions based on experments on linear hybrid automata */

/* 
   Hybrid systems plug-in  for FAU Discrete Event Systems Library 

   Copyright (C) 2017  Thomas Moor, Stefan Goetz

*/

#include "hyb_abstraction.h"

using namespace faudes;



// construct/destruct
LbdAbstraction::LbdAbstraction(void) : 
  mpExperiment(0), mExpChanged(true) {
}
LbdAbstraction::~LbdAbstraction(void) {
  if(mpExperiment) delete mpExperiment;
}


// set/get experiment (we own)
void LbdAbstraction::Experiment(faudes::Experiment* exp) {
  if(mpExperiment) delete mpExperiment;
  mpExperiment=exp;
  mExpChanged=true;
  mTivMode=false;
  mTvMode=false;
}
const Experiment& LbdAbstraction::Experiment(void) {
  if(!mpExperiment)  
    throw Exception("LbdAbstraction::Experiment()", "experiment not set yet", 90);
  return *mpExperiment;
}


// refine interface
void LbdAbstraction::RefineAt(Idx nid) {
  if(!mpExperiment)  
    throw Exception("LbdAbstraction::RefineAt(..)", "experiment not set yet", 90);
  mpExperiment->RefineAt(nid);
  mExpChanged=true;
}
void LbdAbstraction::RefineUniformly(unsigned int depth) {
  if(!mpExperiment)  
    throw Exception("LbdAbstraction::RefineAt(..)", "experiment not set yet", 90);
  mpExperiment->RefineUniformly(depth);
  mExpChanged=true;
}

// access abstractions
const Generator& LbdAbstraction::TivAbstraction(void) {
  // build from scartch
  if(!mTivMode)   { doTivAbstractionMG(); mTivMode=true; mTvMode=false; mExpChanged=false;}
  // optionl inclemental methods .. put here  
  if(mExpChanged) { doTivAbstractionMG(); mExpChanged=false;}
  // done
  return mAbstraction;
}
    
// access abstractions
const Generator& LbdAbstraction::TvAbstraction(void) {
  // build from scartch
  if(!mTvMode)   { doTvAbstraction(); mTvMode=true; mTivMode=false; mExpChanged=false;}
  // optionl inclemental methods .. put here  
  if(mExpChanged) { doTvAbstraction(); mExpChanged=false;}
  // done
  return mAbstraction;
}
    

// workers: copy tree
void LbdAbstraction::doInitAbstraction(void) {
  // prepare
  mAbstraction.Clear();
  mLeaves.Clear();
  mAbstraction.InjectAlphabet(mpExperiment->Alphabet());\
  // loop todo
  std::stack<Idx> todo;
  todo.push(mpExperiment->Root());
  mAbstraction.InsInitState(mpExperiment->Root());
  while(!todo.empty()) {
    Idx nid=todo.top();
    todo.pop();
    mAbstraction.InsState(nid);
    if(mpExperiment->IsLeave(nid)) mLeaves.Insert(nid);
    EventSet events=mpExperiment->EnabledEvents(nid);
    EventSet::Iterator eit=events.Begin();
    for(;eit!=events.End();++eit) {
      Idx x2=mpExperiment->SuccessorNode(nid,*eit);
      mAbstraction.InsState(x2);
      mAbstraction.SetTransition(nid,*eit,x2);
      todo.push(x2);
    }
  }
}

// workers: time variant
void LbdAbstraction::doTvAbstraction(void) {
  doInitAbstraction();
  /*
  // without dumpstate
  StateSet::Iterator sit=mLeaves.Begin();
  StateSet::Iterator sit_end=mLeaves.End();
  for(;sit!=sit_end;++sit) {
    EventSet::Iterator eit=mpExperiment->Alphabet().Begin();
    EventSet::Iterator eit_end=mpExperiment->Alphabet().End();
    if(mpExperiment->IsLock(*sit)) continue;
    for(;eit!=eit_end;++eit)
      mAbstraction.SetTransition(*sit,*eit,*sit);
  } 
  */
  // with dumpstate
  Idx qd=mAbstraction.InsState();
  EventSet::Iterator eit=mpExperiment->Alphabet().Begin();
  EventSet::Iterator eit_end=mpExperiment->Alphabet().End();
  for(;eit!=eit_end;++eit)
    mAbstraction.SetTransition(qd,*eit,qd);
  StateSet::Iterator sit=mLeaves.Begin();
  StateSet::Iterator sit_end=mLeaves.End();
  for(;sit!=sit_end;++sit) {
    EventSet::Iterator eit=mpExperiment->Alphabet().Begin();
    EventSet::Iterator eit_end=mpExperiment->Alphabet().End();
    if(mpExperiment->IsLock(*sit)) continue;
    for(;eit!=eit_end;++eit)
      mAbstraction.SetTransition(*sit,*eit,qd);
  } 

  
}

// workers: time invariant (Moor/Goetz style)
void LbdAbstraction::doTivAbstractionMG(void) {
  doInitAbstraction();
  StateSet::Iterator sit=mLeaves.Begin();
  StateSet::Iterator sit_end=mLeaves.End();
  // assert: root must not be a leave here
  for(;sit!=sit_end;++sit) {
    if(mpExperiment->IsLock(*sit)) continue;
    std::deque<Idx> seq=mpExperiment->Sequence(*sit);
    while(seq.size()>0) {
      seq.pop_front();
      Idx x1=mpExperiment->Find(seq);
      if(x1==0) continue;
      if(mpExperiment->IsLock(x1)) break;
      if(mpExperiment->IsLeave(x1)) continue;
      EventSet events=mpExperiment->EnabledEvents(x1);
      EventSet::Iterator eit=events.Begin();
      EventSet::Iterator eit_end=events.End();
      for(;eit!=eit_end;++eit) {
        Idx x2=mpExperiment->SuccessorNode(x1,*eit);
        mAbstraction.SetTransition(*sit,*eit,x2);
      }
      break;
    }
  }
}

// workers: time invariant (Raisch/Yang style)
void LbdAbstraction::doTivAbstractionRY(void) {
  // prepare
  mAbstraction.Clear();
  mLeaves.Clear();
  mAbstraction.InjectAlphabet(mpExperiment->Alphabet());\
  // loop todo: copy tree except leaves
  // should assert that root is not a leave
  std::stack<Idx> todo;
  todo.push(mpExperiment->Root());
  mAbstraction.InsInitState(mpExperiment->Root());
  while(!todo.empty()) {
    Idx nid=todo.top();
    todo.pop();
    if(mpExperiment->IsLeave(nid)) mLeaves.Insert(nid);  
    EventSet events=mpExperiment->EnabledEvents(nid);
    EventSet::Iterator eit=events.Begin();
    for(;eit!=events.End();++eit) {
      Idx x2=mpExperiment->SuccessorNode(nid,*eit);
      if(mpExperiment->IsLeave(x2) && !mpExperiment->IsLock(x2)) continue;
      mAbstraction.InsState(nid);
      mAbstraction.InsState(x2);
      mAbstraction.SetTransition(nid,*eit,x2);
      todo.push(x2);
    }
  }
  // fix ends
  StateSet::Iterator sit=mAbstraction.StatesBegin();
  StateSet::Iterator sit_end=mAbstraction.StatesEnd();
  for(;sit!=sit_end;++sit) {
    if(mpExperiment->IsLock(*sit)) continue;
    EventSet events=mpExperiment->EnabledEvents(*sit);
    EventSet::Iterator eit=events.Begin();
    for(;eit!=events.End();++eit) {
      if(mAbstraction.ExistsTransition(*sit,*eit)) continue;   
      std::deque<Idx> seq=mpExperiment->Sequence(*sit);
      seq.push_back(*eit);
      while(seq.size()>0) {
        seq.pop_front();
        Idx x2=mpExperiment->Find(seq);
        if(x2==0) continue;
        mAbstraction.SetTransition(*sit,*eit,x2);
	break;
      }
    }
  }
}
