#include "ios_algorithms.h"
#include "syn_wsupcon.h"

namespace faudes {

// IsIoSystem() implementation
bool IsIoSystem(const IoSystem& rIoSystem,
  StateSet& rQU,
  StateSet& rQY,
  StateSet& rQErr)
{
  FD_DIO("IsIoSystem("<< rIoSystem.Name() << ",...)");
  // prepare result
  rQU.Clear();
  rQY.Clear();
  rQErr.Clear();
  rQErr.Name("ErrorStates");
  // completeness (iterate over accessible states only)
  FD_DIO("IsIoSystem("<< rIoSystem.Name() << ",...): testing completeness");
  StateSet acc = rIoSystem.AccessibleSet();
  StateSet coacc = rIoSystem.CoaccessibleSet();
  StateSet::Iterator sit=acc.Begin();
  StateSet::Iterator sit_end=acc.End();
  for(;sit!=sit_end;sit++){
    // cannot extend 
    TransSet::Iterator tit=rIoSystem.TransRelBegin(*sit);
    TransSet::Iterator tit_end=rIoSystem.TransRelEnd(*sit);
    if(tit==tit_end) rQErr.Insert(*sit);
    // not coreachable
    if(!coacc.Exists(*sit)) rQErr.Insert(*sit);
  }
  if(!rQErr.Empty()) {
    FD_DIO("IsIoSystem("<< rIoSystem.Name() << ",...): not complete");
    return false;
  }
  // insist in a u-y partition
  EventSet errev;
  bool hasu=false;
  bool hasy=false;
  EventSet::Iterator eit = rIoSystem.AlphabetBegin(); 
  EventSet::Iterator eit_end= rIoSystem.AlphabetEnd(); 
  for(; eit != eit_end; ++eit) {
    if(rIoSystem.InputEvent(*eit))
      hasu=true;
    if(rIoSystem.InputEvent(*eit))
      hasy=true;
    if(rIoSystem.InputEvent(*eit))
    if(rIoSystem.OutputEvent(*eit)) 
      errev.Insert(*eit);
    if(!rIoSystem.InputEvent(*eit))
    if(!rIoSystem.OutputEvent(*eit))
      errev.Insert(*eit);
  }
  if(!errev.Empty()) { 
    FD_DIO("IsIoSystem("<< rIoSystem.Name() << ",...): not  a u-y partition of events");
    TransSet::Iterator tit=rIoSystem.TransRelBegin();
    TransSet::Iterator tit_end=rIoSystem.TransRelEnd();
    for(; tit!=tit_end; tit++) 
      if(errev.Exists(tit->Ev)) rQErr.Insert(tit->X1);
    return false;
  }
  if(!hasu || !hasy) {
    FD_DIO("IsIoSystem("<< rIoSystem.Name() << ",...): trivial partition");
    return false;
  }
  // io-alternation: fill todo stack with initial states
  FD_DIO("IsIoSystem("<< rIoSystem.Name() << ",...): i/o alternation");
  std::stack<Idx> todo;
  sit = rIoSystem.InitStatesBegin(); 
  sit_end= rIoSystem.InitStatesEnd(); 
  for(; sit != sit_end; ++sit) {
    // figure type of initial state
    TransSet::Iterator tit=rIoSystem.TransRelBegin(*sit);
    TransSet::Iterator tit_end=rIoSystem.TransRelEnd(*sit);
    for(; tit!=tit_end; tit++) {
      if(rIoSystem.InputEvent(tit->Ev)) rQU.Insert(*sit);
      if(rIoSystem.OutputEvent(tit->Ev)) rQY.Insert(*sit);
    }
    // push
    todo.push(*sit);
  }
  // io-alternation: multiple initialstates are fine, but must be of same type.
  if(!rQU.Empty() && !rQY.Empty()) {
    sit = rIoSystem.InitStatesBegin(); 
    sit_end= rIoSystem.InitStatesEnd(); 
    for(; sit != sit_end; ++sit) {
      rQErr.Insert(*sit);
      return false;
    }
  }
  // io-alternation: process stack
  while(!todo.empty()) {
    const Idx current = todo.top();
    todo.pop();
    bool uok = rQU.Exists(current);
    bool yok = rQY.Exists(current);
    // iterate all transitions
    TransSet::Iterator tit=rIoSystem.TransRelBegin(current);
    TransSet::Iterator tit_end=rIoSystem.TransRelEnd(current);
    for(; tit!=tit_end; tit++) {
      if(!rQY.Exists(tit->X2) && !rQU.Exists(tit->X2)) 
	  todo.push(tit->X2);        
      if(rIoSystem.InputEvent(tit->Ev)) {
        rQY.Insert(tit->X2);
        if(!uok) rQErr.Insert(current);
      }
      if(rIoSystem.OutputEvent(tit->Ev)) {
        rQU.Insert(tit->X2);
        if(!yok) rQErr.Insert(current);
      }
    }
  }
  if(!rQErr.Empty()) {
    return false;
  }
  // done 
  return true;
}
    

// IsIoSystem wrapper function
bool IsIoSystem(IoSystem& rIoSystem) {
  StateSet QU,QY, QErr;
  bool res= IsIoSystem(rIoSystem, QU, QY, QErr);
  rIoSystem.InputStates(QU);
  rIoSystem.OutputStates(QY);
  rIoSystem.ErrorStates(QErr);
  return res;
}
    

// rti function interface
void IoStatePartition(IoSystem& rIoSystem) {
  IsIoSystem(rIoSystem);
}


// IsInputLocallyFree wrapper function
bool IsInputLocallyFree(IoSystem& rIoSystem) {
  FD_DIO("IsInputLocallyFree("<< rIoSystem.Name() << ",...)");
  StateSet QErr;
  bool res=IsInputLocallyFree(rIoSystem, QErr);
  rIoSystem.ErrorStates(QErr);
  return res;
}
    
// IsInputLocallyFree implementation
bool IsInputLocallyFree(const IoSystem& rIoSystem, StateSet& rQErr) {
  FD_DIO("IsInputLocallyFree("<< rIoSystem.Name() << ",...)");
  // prepare result
  rQErr.Clear();
  rQErr.Name("ErrorStates");
  // have set of all input events
  EventSet sigu=rIoSystem.InputEvents();
  // test all states
  StateSet::Iterator sit = rIoSystem.StatesBegin(); 
  StateSet::Iterator sit_end= rIoSystem.StatesEnd(); 
  for(; sit != sit_end; ++sit) {
    // get all enabled inputs
    EventSet lsigu;
    TransSet::Iterator tit=rIoSystem.TransRelBegin(*sit);
    TransSet::Iterator tit_end=rIoSystem.TransRelEnd(*sit);
    for(; tit!=tit_end; tit++) 
      if(rIoSystem.InputEvent(tit->Ev)) lsigu.Insert(tit->Ev);
    //FD_DIO("DUMP " << rIoSystem.StateName(*sit) << " "<< lsigu.ToString());
    // no inputs? fine
    if(lsigu.Empty()) continue;
    // all inputs? fine
    if(lsigu == sigu) continue;
    // error
    rQErr.Insert(*sit);
    //FD_DIO("DUMP " << *sit << " " << rQErr.ToString());
  }
  return rQErr.Empty();
}


// IsInputOmegaFree wrapper function
bool IsInputOmegaFree(IoSystem& rIoSystem) {
  FD_DIO("IsInputOmegaFree("<< rIoSystem.Name() << ",...)");
  StateSet QErr;
  bool res=IsInputOmegaFree(rIoSystem, QErr);
  rIoSystem.ErrorStates(QErr);
  return res;
}
    
// Is InputOmegaFree implementation
bool IsInputOmegaFree(const IoSystem& rIoSystem, StateSet& rQErr) {
  FD_DIO("IsInputOmegaFree("<< rIoSystem.Name() << ",...)");

  // test for locally free input first
  rQErr.Clear();  
  if(!IsInputLocallyFree(rIoSystem,rQErr)) {
    FD_DIO("IsInputOmegaFree("<< rIoSystem.Name() << ",...): failed for locally free");
    return false;
  }

  // prepare good state iteration
  StateSet goodstates=rIoSystem.MarkedStates();
  EventSet yalph=rIoSystem.OutputEvents();
  rQErr=rIoSystem.AccessibleSet()-goodstates;
  rQErr.Name("ErrorStates");

  // control to good states by choosing strategic y-events
  while(true) {
    // test individual states
    FD_DIO("IsInputOmegaFree(...): iterate over good states");
    bool found=false;
    StateSet::Iterator sit = rQErr.Begin();
    while(sit!=rQErr.End()) {
      // pre-increment
      StateSet::Iterator cit=sit++;
      // goodstate anyway
      if(goodstates.Exists(*cit)) continue;
      // test transitions
      TransSet::Iterator tit = rIoSystem.TransRelBegin(*cit);
      TransSet::Iterator tit_end = rIoSystem.TransRelEnd(*cit);
      // no transitions at all: no chance
      if(tit==tit_end) continue;
      // iterate successors
      bool exgood=false;
      for(; tit!=tit_end; ++tit) {
        if(goodstates.Exists(tit->X2)) { exgood=true; continue; };
        if(!yalph.Exists(tit->Ev)) break; // break on not-good successor with non-y-event (aka u-event)
      }
      // succes: good successor exists and all not-good successors are y-events [rev tmoor 201507]
      if(exgood && (tit==tit_end)) {
        FD_DIO("IsInputOmegaFree(): ins good state " << rIoSystem.SStr(*cit));
        goodstates.Insert(*cit);
        rQErr.Erase(cit);
        found=true;
      }
    }
    // exit
    if(!found) break;
  };

  // errorstates
  if(rQErr.Empty()) {
    FD_DIO("IsInputOmegaFree(): accessible <= good: passed");
    return true;
  }

  // fail
  FD_DIO("IsInputOmegaFree(): accessible <= good: failed");
  return false;
}


// IoFreeInput() wrapper
void IoFreeInput(IoSystem& rIoSystem) {
  IoFreeInput(rIoSystem,rIoSystem.InputEvents());
}

// IoFreeInput()
void IoFreeInput(Generator& rGen, const EventSet& rUAlph) {
  FD_DIO("IoFreeInput("<< rGen.Name() << ",...)");
  // test alphabet
  if(!(rUAlph <= rGen.Alphabet())){
    std::stringstream errstr;
    errstr << "Input alphabet must be contained in generator alphabet";
    throw Exception("IoFreeInput(..)", errstr.str(), 100);
  }
  // prepare error states
  Idx qyerr=0;
  Idx querr=0;  
  // declare some local vars
  EventSet::Iterator eit;
  EventSet::Iterator eit_end;
  // test all states
  StateSet::Iterator sit = rGen.StatesBegin(); 
  StateSet::Iterator sit_end= rGen.StatesEnd(); 
  for(; sit != sit_end; ++sit) {
    // get all enabled inputs
    EventSet lsigu;
    TransSet::Iterator tit=rGen.TransRelBegin(*sit);
    TransSet::Iterator tit_end=rGen.TransRelEnd(*sit);
    for(; tit!=tit_end; tit++) 
      if(rUAlph.Exists(tit->Ev)) lsigu.Insert(tit->Ev);
    // no inputs? fine
    if(lsigu.Empty()) continue;
    // all inputs? fine
    if(lsigu == rUAlph) continue;
    // no error states yet? insert them
    if(qyerr==0) {
      // todo: be smart in state names when enabled
      qyerr = rGen.InsMarkedState();
      querr = rGen.InsMarkedState();
      // enable all transition
      eit=rGen.Alphabet().Begin();
      eit_end=rGen.Alphabet().End();
      for(; eit!=eit_end; eit++) {
        if(rUAlph.Exists(*eit))
  	  rGen.SetTransition(querr,*eit,qyerr);
        else
	  rGen.SetTransition(qyerr,*eit,querr);
      }
    }
    // fix the state at hand
    eit=rUAlph.Begin();
    eit_end=rUAlph.End();
    for(; eit!=eit_end; eit++) 
      if(!lsigu.Exists(*eit)) 
	rGen.SetTransition(*sit,*eit,qyerr);
    // continue with next state
  }
}

// IoRemoveDummyStates
void RemoveIoDummyStates(IoSystem& rIoSystem) {
  FD_DIO("RemoveIoDummyStates("<< rIoSystem.Name() << ",...)");
  // have set of all input/output events
  EventSet sigu=rIoSystem.InputEvents();
  EventSet sigy=rIoSystem.OutputEvents();
  // have results
  StateSet qerr1;  // a) find all outputs to unique successor
  StateSet qerr2;  // b) collect successors from a)
  StateSet qerr2a; // c) from qerr2 only keep all with unique successor
  StateSet qerr;   // d) restrict candidates to cyclic behaviour
  // find states with all outputs leading to the same successor
  // record as type 1 candidate
  StateSet::Iterator sit = rIoSystem.StatesBegin(); 
  StateSet::Iterator sit_end= rIoSystem.StatesEnd(); 
  for(; sit != sit_end; ++sit) {
    // get all enabled events, track for unique successor
    EventSet lsig;
    Idx qsuc=0;
    bool qunique=true;
    TransSet::Iterator tit=rIoSystem.TransRelBegin(*sit);
    TransSet::Iterator tit_end=rIoSystem.TransRelEnd(*sit);
    for(; tit!=tit_end; tit++) {
      if(qsuc==0) qsuc=tit->X2;
      if(qsuc!=tit->X2) { qunique=false; break;}
      lsig.Insert(tit->Ev);
    }
    // non unique successor? discard
    if(!qunique || qsuc==0) continue;
    // outputs not enabled? discard
    if(!(lsig == sigy)) continue;
    // record candidate
    qerr1.Insert(*sit);
    qerr2.Insert(qsuc);
  }
  FD_DIO("RemoveIoDummyStates(): Candidates type 1 " << qerr1.ToString());
  FD_DIO("RemoveIoDummyStates(): Candidates type 2 " << qerr2.ToString());
  // only keep type 2 candidates with all inputs enabled and 
  // leading to a type 1 candidate
  sit = qerr2.Begin(); 
  sit_end= qerr2.End(); 
  for(; sit != sit_end; ++sit) {
    // get all enabled events, track for unique successor
    EventSet lsig;
    Idx qsuc=0;
    bool qunique=true;
    TransSet::Iterator tit=rIoSystem.TransRelBegin(*sit);
    TransSet::Iterator tit_end=rIoSystem.TransRelEnd(*sit);
    for(; tit!=tit_end; tit++) {
      if(qsuc==0) qsuc=tit->X2;
      if(qsuc!=tit->X2) { qunique=false; break;}
      lsig.Insert(tit->Ev);
    }
    // non unique successor? discard
    if(!qunique) continue;
    // successor not in candidates? discard
    if(!qerr1.Exists(qsuc)) continue;
    // inputs not enabled? discard
    if(!(lsig == sigu)) continue;
    // record candidate
    qerr2a.Insert(*sit);
  }
  FD_DIO("RemoveIoDummyStates(): Candidates type 2 (approved) " << qerr2a.ToString());
  // only keep loops
  while(1) {
    StateSet qrm1;
    sit = qerr1.Begin(); 
    sit_end= qerr1.End(); 
    for(; sit != sit_end; ++sit) {
      TransSet::Iterator tit=rIoSystem.TransRelBegin(*sit);
      TransSet::Iterator tit_end=rIoSystem.TransRelEnd(*sit);
      if(tit==tit_end) { qrm1.Insert(*sit); break;}
      if(!qerr2a.Exists(tit->X2)) { qrm1.Insert(*sit); break;}
    }
    qerr1.EraseSet(qrm1);
    StateSet qrm2;
    sit = qerr2a.Begin(); 
    sit_end= qerr2a.End(); 
    for(; sit != sit_end; ++sit) {
      TransSet::Iterator tit=rIoSystem.TransRelBegin(*sit);
      TransSet::Iterator tit_end=rIoSystem.TransRelEnd(*sit);
      if(tit==tit_end) { qrm2.Insert(*sit); break;}
      if(!qerr1.Exists(tit->X2)) { qrm2.Insert(*sit); break;}
    }
    qerr2a.EraseSet(qrm2);
    if(qrm1.Empty() && qrm2.Empty()) break;
  }
  qerr=qerr1 + qerr2;
  FD_DIO("RemoveIoDummyStates(): Dummy states" << qerr.ToString());
  sit = qerr.Begin(); 
  sit_end= qerr.End(); 
  for(; sit != sit_end; ++sit) 
    rIoSystem.DelState(*sit);
  FD_DIO("RemoveIoDummyStates(): done");
}

// IoSynthesis(rPlant,rSpec,rSup,rErrorStates)
void IoSynthesisNB(const IoSystem& rPlant, const Generator& rSpec, IoSystem& rSup) {
  FD_DIO("IosSynthesisNB");
   
  // synthesis
  EventSet ualph = rPlant.InputEvents();
  EventSet yalph = rPlant.OutputEvents();
  OmegaSupConNB(rPlant,ualph,rSpec,rSup) ;   

  // fix event attributes
  rSup.InputEvents(yalph);
  rSup.OutputEvents(ualph);
}

// IoSynthesis(rPlant,rSpec,rSup,rErrorStates)
void IoSynthesis(const IoSystem& rPlant, const Generator& rSpec, IoSystem& rSup) {
  FD_DIO("IosSynthesis");
   
  // synthesis
  EventSet ualph = rPlant.InputEvents();
  EventSet yalph = rPlant.OutputEvents();
  SupConCmplClosed(rPlant,ualph,rSpec,rSup) ;   

  // fix event attributes
  rSup.InputEvents(yalph);
  rSup.OutputEvents(ualph);
}


}// end: namespace faudes
