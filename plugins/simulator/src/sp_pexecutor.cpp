/** @file sp_pexecutor.cpp Executor for multiple synchronized timed generators  */

/* 
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2007, 2008 Thomas Moor
   Copyright (C) 2007  Ruediger Berndt
   Exclusive copyright is granted to Klaus Schmidt

*/


#include "sp_pexecutor.h"

namespace faudes {


// std faudes type
FAUDES_TYPE_IMPLEMENTATION(ParallelExecutor,ParallelExecutor,Type)

// ParallelExecutor(void)
ParallelExecutor::ParallelExecutor(void) : Type() {
  FD_DX("ParallelExecutor(" << this << ")::ParallelExecutor()");
}

// ParallelExecutor(other)
ParallelExecutor::ParallelExecutor(const ParallelExecutor& rOther) : Type() {
  FD_DX("ParallelExecutor(" << this << ")::ParallelExecutor(other)");
  Assign(rOther);
}

// ParallelExecutor(rFileName)
ParallelExecutor::ParallelExecutor(const std::string& rFileName) : Type() {
  FD_DX("ParallelExecutor(" << this << ")::ParallelExecutor(" << rFileName << ")");
  Read(rFileName);
}

// ParallelExecutor(void)
ParallelExecutor::~ParallelExecutor(void) { 
  FD_DX("ParallelExecutor(" << this << ")::~ParallelExecutor()");
}

// DoAssign(other)
void ParallelExecutor::DoAssign(const ParallelExecutor& rOther) {
  FD_DX("ParallelExecutor(" << this << ")::DoAssign(other)");
  mExecutors=rOther.mExecutors;
  mExecutorNames=rOther.mExecutorNames;
  Compile();
}

//UpdateParallelTimedState()
void ParallelExecutor::UpdateParallelTimedState(void) {
  mCurrentParallelTimedState.State.clear();
  mCurrentParallelTimedState.Clock.clear();
  for(Iterator xit=Begin(); xit!=End(); xit++){
    mCurrentParallelTimedState.State.push_back(xit->CurrentTimedState().State);
    mCurrentParallelTimedState.Clock.push_back(xit->CurrentTimedState().ClockValue);
  }
}

// Compile()
void ParallelExecutor::Compile(void) {
  iterator xit;
  FD_DX("ParallelExecutor(" << this << ")::Compile(): with #" << Size() << " generators");
  // reset executors, incl compile
  for(xit=mExecutors.begin(); xit!=mExecutors.end(); xit++) 
    xit->Reset();
  // update state
  UpdateParallelTimedState();
  // compute alphabet
  mAlphabet.Clear();
  mAlphabet.Name("Alphabet");
  for(xit=mExecutors.begin(); xit!=mExecutors.end(); xit++) 
    mAlphabet.InsertSet(xit->Generator().Alphabet());
  // reset other members
  mCurrentTime=0;
  mCurrentStep=0;
  mRecentEvent=0;
  mEValid=false;
  FD_DX("ParallelExecutor(" << this << ")::Compile(): done");
}

// Reset()
void ParallelExecutor::Reset(void) {
  FD_DX("ParallelExecutor(" << this << ")::Reset()");
  // compile includes reset 
  Compile();
}

// Insert(rFileName)
void ParallelExecutor::Insert(const std::string& rFileName) {
  FD_DX("ParallelExecutor(" << this << ")::Insert(" << rFileName << ")" );
  TokenReader tr(rFileName);
  // create executor and read generator
  mExecutors.push_back(Executor());
  mExecutorNames.push_back(rFileName);
  // read generator
  mExecutors.back().Read(tr);
  FD_DX("ParallelExecutor(" << this << ")::Insert(" << rFileName 
        << "): found " << mExecutors.back().Name());
  // compile (incl check determinism)
  Compile();
}

// Insert(rGen)
void ParallelExecutor::Insert(const TimedGenerator& rGen) {
  FD_DX("ParallelExecutor(" << this << ")::Insert(rGen): " << rGen.Name() << " at #" << Size());
  mExecutors.push_back(Executor(rGen));
  mExecutorNames.push_back("");
  // compile (incl check determinism)
  Compile();
}

// Clear()
void ParallelExecutor::Clear(void) {
  FD_DX("ParallelExecutor(" << this << ")::Clear()");
  mExecutors.clear();
  mExecutorNames.clear();
  Compile();
}

// Size()
Idx ParallelExecutor::Size(void) const {
  return (Idx) mExecutors.size();
}



//DoWrite(rTr,rLabel)
void ParallelExecutor::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  std::string label=rLabel;
  if(label=="") label = "Executor";
  rTw.WriteBegin(label);
  DoWriteGenerators(rTw);
  rTw.WriteEnd(label);
}

// DoXWrite()
/*
void ParallelExecutor::DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  // Set up outer tag
  std::string label=rLabel;
  std::string ftype=TypeName();
  if(label=="") label="Executor";
  Token btag;
  btag.SetBegin(label);
  if(Name()!=label && Name()!="") btag.InsAttributeString("name",Name());
  if(ftype!=label && ftype!="") btag.InsAttributeString("ftype",ftype);
  FD_DG("vGenerator(" << this << ")::DoXWrite(..): section " << btag.StringValue() << " #" << Size());
  rTw.Write(btag);
  DoWriteGenerators(rTw);
  rTw.WriteEnd(label);
}
*/

//DoWriteGenerators(rTr,rLabel)
void ParallelExecutor::DoWriteGenerators(TokenWriter& rTw) const {
  rTw.WriteBegin("Generators");
  for(Idx i=0; i<Size(); i++) {
    // write filename
    if(mExecutorNames.at(i)!="") {
      rTw.WriteString(mExecutorNames.at(i));
    }
    // write generator
    if(mExecutorNames.at(i)=="") {
      mExecutors.at(i).Write(rTw);
    }
  }
  rTw.WriteEnd("Generators");
}

//DoRead(rTr,rLabel)
void ParallelExecutor::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) pContext;
  // report
  FD_DX("ParallelExecutor(" << this << ")::DoRead(tr, " << rLabel << ")");
  // get my section
  std::string label=rLabel;
  if(label=="") label = "Executor";
  // read section
  rTr.ReadBegin(label);
  // read generators
  DoReadGenerators(rTr);
  // done
  rTr.ReadEnd(label);
  // reset
  Reset();
}


//DoReadGenerators(rTr,rLabel)
void ParallelExecutor::DoReadGenerators(TokenReader& rTr) {
  // get relevant directory
  std::string dirname="";
  if(rTr.SourceMode()==TokenReader::File) 
    dirname=ExtractDirectory(rTr.FileName());
  // report
  FD_DX("ParallelExecutor(" << this << ")::DoReadGenerators(tr): dirname " << dirname);
  // read section
  rTr.ReadBegin("Generators");
  Token token;
  while(!rTr.Eos("Generators")) {
    rTr.Peek(token);
    // is it a file name?
    if(token.Type()==Token::String) {
      Insert(PrependDirectory(dirname,token.StringValue()));
      rTr.Get(token);
      continue;
    }
    // is it a generator?
    if(token.Type()==Token::Begin) 
    if(token.StringValue()=="Generator") {
      TimedGenerator gen;
      gen.Read(rTr);
      Insert(gen);
      continue;
    }
    // else report error
    std::stringstream errstr;
    errstr << "Invalid token" << rTr.FileLine();
    throw Exception("ParallelExecutor::DoReadGenerators", errstr.str(), 502);
  }
  rTr.ReadEnd("Generators");
}

// Alphabet()
const EventSet& ParallelExecutor::Alphabet(void) const {
  return mAlphabet;
}

// ComputeEnabled() fake const
void ParallelExecutor::ComputeEnabled(void) const {
  ParallelExecutor* fakeconst = const_cast<ParallelExecutor*>(this);
  fakeconst->ComputeEnabledNonConst();
}

// ComputeEnabled()
void ParallelExecutor::ComputeEnabledNonConst(void) {
  iterator xit;
  // compute members (may remove this)
  FD_DX("ParallelExecutor(" << this << ")::ComputeEnabled(): members");
  for(xit=mExecutors.begin(); xit != mExecutors.end(); xit++) 
    xit->IsDeadlocked();
  // compute etime
  FD_DX("ParallelExecutor(" << this << ")::ComputeEnabled(): time");
  mETime.SetPositive();
  for(xit=mExecutors.begin(); xit != mExecutors.end(); xit++) 
    mETime.Intersect(xit->EnabledTime());
  // compute e/d events
  FD_DX("ParallelExecutor(" << this << ")::ComputeEnabled(): e/d events");
  mDEvents.Name("DisabledEvents");
  mDEvents.Clear();
  for(xit=mExecutors.begin(); xit != mExecutors.end(); xit++) 
    mDEvents.InsertSet(xit->DisabledEvents());
  mEEvents=mAlphabet;
  mEEvents.Name("EnabledEvents");
  mEEvents.EraseSet(mDEvents);  
  // compute einterval // TODO: this is conservative
  FD_DX("ParallelExecutor(" << this << ")::ComputeEnabled(): interval");
  mEInterval.SetPositive();
  for(xit=mExecutors.begin(); xit != mExecutors.end(); xit++) 
    mEInterval.Intersect(xit->EnabledInterval());
  // done
  mEValid=true; 
}

// EnabledTime(void)  
const TimeInterval& ParallelExecutor::EnabledTime(void) const {
  if(!mEValid) ComputeEnabled();
  return mETime;

}

// EnabledEvents(void)  
const EventSet& ParallelExecutor::EnabledEvents(void) const {
  if(!mEValid) ComputeEnabled();
  return mEEvents;
}

// DisabledEvents(void)  
const EventSet& ParallelExecutor::DisabledEvents(void) const {
  if(!mEValid) ComputeEnabled();
  return mDEvents;
}

// EnabledInterval(void)  
const TimeInterval& ParallelExecutor::EnabledInterval(void) const {
  if(!mEValid) ComputeEnabled();
  return mEInterval;
}

// EnabledEventInterval(event)								
TimeInterval ParallelExecutor::EnabledEventTime(Idx event) const {
  TimeInterval retInterval;
  retInterval.SetPositive();
  Iterator xit;
  for(xit=Begin(); xit != End(); xit++){
    retInterval.Intersect(xit->EnabledEventTime(event));
  }
  FD_DX("ParalelExecutor(" << this << ")::EnabledEventTime(" << event << "):"<< retInterval.Str());
  return retInterval;
}

// EnabledGuardInterval(event)								
TimeInterval ParallelExecutor::EnabledGuardTime(Idx event) const {
  TimeInterval retInterval;
  retInterval.SetPositive();
  Iterator xit;
  for(xit=Begin(); xit != End(); xit++){
    retInterval.Intersect(xit->EnabledGuardTime(event));
  }
  FD_DX("ParalelExecutor(" << this << ")::EnabledGuardTime(" << event << "):"<< retInterval.Str());
  return retInterval;
}

//GetCurrentStateVec
const ParallelExecutor::ParallelState& ParallelExecutor::CurrentParallelState(void) const {
  return mCurrentParallelTimedState.State;
}
	
// get current state
const ParallelExecutor::ParallelTimedState& ParallelExecutor::CurrentParallelTimedState(void) const {
  return mCurrentParallelTimedState;
}

// set current state
bool ParallelExecutor::CurrentParallelTimedState(const ParallelTimedState& ptstate) {
  FD_DX("ParalelExecutor(" << this << ")::CurrentParallelState(ptstate): set " << PTSStr(ptstate));
  // prepare
  if(ptstate.State.size()!=Size()) return false;
  if(ptstate.Clock.size()!=Size()) return false;
  bool res=true;
  ParallelTimedState oldstate=CurrentParallelTimedState();
  // loop and set for all executors
  int i=0;
  for(iterator xit=mExecutors.begin(); xit!=mExecutors.end(); xit++, i++){
    TimedState tstate;
    tstate.State=ptstate.State[i];
    tstate.ClockValue=ptstate.Clock[i];
    res = res && xit->CurrentTimedState(tstate);
  }	
  // reject
  if(!res) {
    ParallelExecutor::CurrentParallelTimedState(oldstate);
    return false;
  }
  // reset time (dont call the virtual fncts here)
  ParallelExecutor::CurrentTime(0);
  ParallelExecutor::CurrentStep(0);
  // fix state rep
  UpdateParallelTimedState();
  // invalidate
  mEValid=false;
  mRecentEvent=0;
  FD_DX("ParalelExecutor(" << this << ")::CurrentParallelState(ptstate): done");
  return true;
}



// ExecuteTime(time) 
bool ParallelExecutor::ExecuteTime(Time::Type time) {
  if(mCurrentTime>=Time::Max()) return false;
  if(time==0) return true;
  if(!mEValid) ComputeEnabled();
  if(!mETime.In(time) && !((time==Time::Max()) && mETime.UBinf()) ) {
    FD_DX("ParalelExecutor(" << this << ")::ExecuteTime(time): execution of  " << time 
       << " conflicts with enabled status " );
    return false; 
  }
  // progress current time
  mCurrentTime += time;
  // fix infinity
  if(time==Time::Max()) mCurrentTime=Time::Max();
  // progress members 
  bool success=true;
  for(iterator xit=mExecutors.begin(); xit != mExecutors.end(); xit++) 
    success &= xit->ExecuteTime(time);
  // update state
  UpdateParallelTimedState();
  // indicate invalid (conservative)
  mEValid=false;
  return success;
}

// ExecuteEvent(event) 
bool ParallelExecutor::ExecuteEvent(Idx event) {
  if(!mEValid) ComputeEnabled();
  if(!mEEvents.Exists(event)) {
    FD_DX("ParallelExecutor(" << this << ")::ExecuteEvent(): execution of event " << EStr(event) 
       << " conflicts with enabled status " );
    return false; 
  }
  // progress members
  bool success=true;
  for(iterator xit=mExecutors.begin(); xit != mExecutors.end(); xit++) 
    if(xit->Generator().ExistsEvent(event))
      success &= xit->ExecuteEvent(event);
  if(!success) {
    // should throw exception
    FD_DX("ParallelExecutor(" << this << ")::ExecuteEvent(): execution of event " << EStr(event) 
       << " conflicts with internal state data " );
    return false;
  }
  // progress current time
  mCurrentStep += 1;
  // update state
  UpdateParallelTimedState();
  // record event
  mRecentEvent=event;
  // invalidate
  mEValid=false;
  return true;
}  

// CurrentTime(time)
void ParallelExecutor::CurrentTime(Time::Type time) {
  mCurrentTime=time;
  for(iterator xit=mExecutors.begin(); xit != mExecutors.end(); xit++) 
    xit->CurrentTime(time);
  mEValid=false;
}
  
// CurrentTime()
Time::Type ParallelExecutor::CurrentTime(void) const {
  return mCurrentTime;
}
  
// CurrentStep(step)
void ParallelExecutor::CurrentStep(int step) {
  mCurrentStep=step;
  for(iterator xit=mExecutors.begin(); xit != mExecutors.end(); xit++) 
    xit->CurrentStep(0);
  mEValid=false;
}
  
// CurrentStep()
int ParallelExecutor::CurrentStep(void) const {
  return mCurrentStep;
}
  
// IsDeadlocked() 
bool ParallelExecutor::IsDeadlocked(void) const {
  if(!mEValid) ComputeEnabled();
  if(!mEEvents.Empty()) return false;
  if(!(mETime.UB()<=0)) return false;
  return true;
}

// PTSStr(ptstate)
std::string ParallelExecutor::PTSStr(const ParallelTimedState& ptstate) const {
  if(Size()!=ptstate.State.size()) return("(undef)");
  std::stringstream res;
  res << "(state ";
  for(unsigned int i=0; i< Size(); i++){
    res << mExecutors[i].Generator().SStr(ptstate.State[i]); 
    if(i+1<Size()) res << " x ";
  }
  res << ")  (clocks (";
  for(unsigned int i=0; i< Size(); i++){
    const Executor* execp=&mExecutors[i];
    ClockSet::Iterator cit;
    for(cit=execp->Generator().ClocksBegin();cit!=execp->Generator().ClocksEnd();cit++){
      if(cit!=execp->Generator().ClocksBegin()) res << " ";
      res << CStr(*cit) << "=";
      std::map<Idx,Time::Type>::const_iterator cvit=ptstate.Clock[i].find(*cit);
      if(cvit!=ptstate.Clock[i].end()) res << cvit->second;
      else res << "undef";
    }
    res<<")";
    if(i+1<Size()) res << " x (";
  }
  res << ")";
  return res.str();
}

// PSStr(pstate)                                                            
std::string ParallelExecutor::PSStr(const ParallelState& pstate) const {
  if(Size()!=pstate.size()) return("(undef)");
  std::stringstream res;
  res << "(state ";
  for(unsigned int i=0; i< Size(); i++){
    res << mExecutors[i].Generator().SStr(pstate[i]); 
    if(i+1<Size()) res << " x ";
  }
  res << ")";
  return res.str();
}

// TEStr(tevent)
std::string ParallelExecutor::TEStr(const TimedEvent& tevent) const {
  if(Size()==0) return "(undef)";
  return Begin()->TEStr(tevent);
}

// EStr(event)
std::string ParallelExecutor::EStr(Idx event) const {
  if(Size()==0) return "(undef)";
  return Begin()->EStr(event);
}

// CStr(clock)
std::string ParallelExecutor::CStr(Idx clock) const {
  if(Size()==0) return "(undef)";
  return Begin()->CStr(clock);
}

// CurrentParallelTimedStateStr()
std::string ParallelExecutor::CurrentParallelTimedStateStr(void) const {
  return PTSStr(mCurrentParallelTimedState);
}

// CurrentParallelTimedStateStr()
std::string ParallelExecutor::CurrentParallelStateStr(void) const {     
  return PSStr(mCurrentParallelTimedState.State);
}

//ActiveEventSet     
EventSet ParallelExecutor::ActiveEventSet(const ParallelState& stateVec) const {
  EventSet retEventSet=Alphabet();
  Iterator xit;
  int i;
  for(i=0, xit=Begin(); xit!=End(); xit++, i++) {
    retEventSet.EraseSet( xit->Generator().Alphabet() - xit->Generator().ActiveEventSet(stateVec[i]));			
  }
  return retEventSet;  
}

//Active(Idx)   
bool ParallelExecutor::Active(Idx ev) const{
  return Active(ev, mCurrentParallelTimedState.State);
}

//Active(Idx, ParallelState) 
bool ParallelExecutor::Active(Idx ev, const ParallelState& stateVec) const {
  Iterator xit;
  int i;
  for(xit=Begin(), i=0; xit!=End(); ++xit, i++){
    if(xit->Generator().Alphabet().Exists(ev))
    if(xit->Generator().TransRelBegin(stateVec[i],ev)
       == xit->Generator().TransRelEnd(stateVec[i],ev))
      return false;
  }
  return true;
}


//DoWrite(rTr,rLabel,pContext)
void ParallelExecutor::ParallelTimedState::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  // allow for two versions only
  std::string label=rLabel;
  if(label!="DiscreteState") 
  if(label!="TimedState") 
    label="TimedState";
  FD_DC("ParallelExecutor::ParallelTimedState::DoWrite(): section " << rLabel << " context " << pContext);
  // figure context
  const ParallelExecutor* exe=dynamic_cast<const ParallelExecutor*>(pContext);
  if(exe) if(exe->Size()!=State.size()) exe=0;
  if(exe) if(exe->Size()!=Clock.size()) exe=0;
  // do write
  if(rLabel=="TimedState") rTw.WriteBegin("TimedState");
  // discrete state
  rTw.WriteBegin("DiscreteState");
  for(unsigned int i=0; i< State.size(); i++) {
    std::string name="";
    if(exe) name=exe->At(i).StateName(State.at(i));
    if(name!="") rTw.WriteString(name);
    else rTw.WriteInteger(State.at(i));
  };
  rTw.WriteEnd("DiscreteState");
  // figure whether to write clocks
  bool doclocks=false;
  if(rLabel=="TimedState") 
    for(unsigned int i=0; i< Clock.size(); i++) 
      if(Clock.at(i).size()>0) doclocks=true;
  // write clocks
  if(doclocks) {
    rTw.WriteBegin("ClockMaps");
    for(unsigned int i=0; i< Clock.size(); i++) {    
      rTw.WriteBegin("ClockMap");
      std::map<Idx,Time::Type>::const_iterator cit;
      for(cit=Clock.at(i).begin(); cit!=Clock.at(i).end(); cit++) {
        std::string name="";
        if(exe) name=exe->At(i).Generator().ClockName(cit->first);
        if(name!="") rTw.WriteString(name);
        else rTw.WriteInteger(cit->first);
        rTw.WriteInteger(cit->second);
      }
      rTw.WriteEnd("ClockMap");
    }
    rTw.WriteEnd("ClockMaps");
  }
  // do write end
  if(rLabel=="TimedState") rTw.WriteEnd("TimedState");
}

//DoRead(rTr,rLabel,pContext)
  void ParallelExecutor::ParallelTimedState::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; (void) pContext; (void) rTr;
  FD_DC("ParallelExecutor::ParallelTimedState::DoRead()");
}


} // namespace faudes


