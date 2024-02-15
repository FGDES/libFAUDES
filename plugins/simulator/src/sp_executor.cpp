/** @file sp_executor.cpp Execute transitions in a timed generator  */

/* 
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2007  Ruediger Berndt
   Copyright (C) 2007  Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt
	
*/


#include "sp_executor.h"

namespace faudes {

// std faudes type
FAUDES_TYPE_IMPLEMENTATION(Executor,Executor,TimedGenerator)


// Executor(void)
Executor::Executor(void) : TimedGenerator() {
  FD_DX("Executor(" << this << ")::Executor()");
  InsInitState("init");
  Compile();
}

// Exector(rGen)
Executor::Executor(const TimedGenerator& rGen) : TimedGenerator() {
  FD_DX("Executor(" << this << ")::Executor(rGen)");
  Assign(rGen);
}  

// Exector(filename)
Executor::Executor(const std::string& rFileName) : TimedGenerator() {
  FD_DX("Executor(" << this << ")::Executor("<< rFileName <<")");
  Read(rFileName);
}  

// Generator()
const TimedGenerator& Executor::Generator(void) const{
  return *this;
}

// Generator(rGen)
void Executor::Generator(const TimedGenerator& rGen) {
  FD_DX("Executor::Generator(" << &rGen << ")");
  Assign(*this);
}  


// Compile()
void Executor::Compile(void) {
  FD_DX("Executor(" << this << ")::Compile()");
  // throw exception on non deterministic generator
  if(!IsDeterministic() || InitState()==0) {
    std::stringstream errstr;
    errstr << "nondeterministic generator in simulation" << std::endl;
    throw Exception("Executor::Compile", errstr.str(), 501);
  }
  // compile guards
  TransSet::Iterator tit;
  for(tit=TransRelBegin(); tit!= TransRelEnd(); tit++) {
    FD_DX("Executor(" << this << ")::Compile(): trans " << TStr(*tit));
    TimeConstraint guard=Guard(*tit);
    std::map<Idx,TimeInterval> guardtime;
    ClockSet aclocks=guard.ActiveClocks();
    ClockSet::Iterator cit;
    for(cit=aclocks.Begin(); cit!= aclocks.End(); cit++) 
      guardtime[*cit]=guard.Interval(*cit);
    mTransClockIntervalMap[*tit]=guardtime;
  }
  // compile invariants
  StateSet::Iterator sit;
  for(sit=StatesBegin(); sit!= StatesEnd(); sit++) {
    FD_DX("Executor(" << this << ")::Compile(): state " << SStr(*sit));
    TimeConstraint invariant=Invariant(*sit);
    std::map<Idx,TimeInterval> invtime;
    ClockSet aclocks=invariant.ActiveClocks();
    ClockSet::Iterator cit;
    for(cit=aclocks.Begin(); cit!= aclocks.End(); cit++) 
      invtime[*cit]=invariant.Interval(*cit);
    mStateClockIntervalMap[*sit]=invtime;
  }
  // get ready
  Reset();
  FD_DX("Executor(" << this << ")::Compile(): done");
}

// Clear()
void Executor::Clear(void) {
  FD_DX("Executor(" << this << ")::Clear(): invalid executor");
  TimedGenerator::Clear();
  mCurrentTimedState.State=0;
  mCurrentTimedState.ClockValue.clear();
  mCurrentTime=0;
  mCurrentStep=0;
  mEValid=false;
}

// Reset()
void Executor::Reset(void) {
  mCurrentTimedState.State=*InitStatesBegin();
  mCurrentTimedState.ClockValue.clear();
  ClockSet::Iterator cit;
  for(cit=GlobalAttribute().mClocks.Begin(); cit!=GlobalAttribute().mClocks.End(); cit++)
    mCurrentTimedState.ClockValue[*cit]=0;
  mCurrentTime=0;
  mCurrentStep=0;
  mEValid=false;
}


// DoRead(rTr)
void Executor::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  FD_DX("Executor(" << this << ")::DoRead(rTr)");
  TimedGenerator::DoRead(rTr,rLabel,pContext);
  Compile();
}  

// DoWrite(rTr)
void Executor::DoWrite(TokenWriter& rTw,const std::string& rLabel, const Type* pContext) const {
  FD_DX("Executor(" << this << ")::DoWrite(rTw)");
  TimedGenerator::DoWrite(rTw,rLabel,pContext);
}

// DoAssign
void Executor::DoAssign(const Executor& rSrc) {
  // call base
  TimedGenerator::DoAssign(rSrc);
  // fox my data
  Compile();
}



// ComputeEnabled() fake const
void Executor::ComputeEnabled(void) const {
  Executor* fakeconst = const_cast<Executor*>(this);
  fakeconst->ComputeEnabledNonConst();
}

// ComputeEnabledNoneConst()
void Executor::ComputeEnabledNonConst(void) {
  FD_DX("Executor(" << this << ")::ComputeEnabled()");
  // time is up: clear all
  if(mCurrentTime>=Time::Max()) {
    mAEvents.Clear();
    mATrans.Clear();
    mETime.SetEmpty();
    mEInterval.SetEmpty();
    mEEvents.Clear();
    mDEvents=Alphabet();
    mEGuardInterval.clear();
    mEValid=true;
    FD_DX("Executor(" << this << ")::ComputeEnabled(): time is up");
    return;
  }
  // set active events/transitions
  mAEvents= TimedGenerator::ActiveEventSet(mCurrentTimedState.State);
  mATrans=  TimedGenerator::ActiveTransSet(mCurrentTimedState.State);
  // hypothesis: all time can pass [0,inf)
  mETime.SetPositive();
  mEInterval.SetPositive();
  // hypothesis: no events can occur
  mEEvents.Clear();
  mDEvents=Alphabet();
  mEGuardInterval.clear();
  // inspect invariant to restrict enabled time
  std::map<Idx,TimeInterval> clockintervalmap=mStateClockIntervalMap[mCurrentTimedState.State];
  std::map<Idx,TimeInterval>::const_iterator cit;
  for(cit=clockintervalmap.begin(); cit!=clockintervalmap.end(); cit++) {
    Idx clock =  cit-> first;
    Time::Type clockvalue = mCurrentTimedState.ClockValue[clock];
    TimeInterval interval = cit->second;
    // if a clock violates an invariant constraint return deadlock
    if(! interval.In( clockvalue ) ) {
      FD_DX("Executor(" << this << ")::ComputeEnabled(): clock " << CStr(cit->first) 
        << " at " <<  clockvalue << " violates invariant condition " << interval.Str() );
      mETime.SetEmpty();
      mEValid=true; 
      return;
    }
    // left shift interval by clock value to obtain an interval relative to current time
    interval.PositiveLeftShift(clockvalue);
    // intersect with enabled time
    mETime.Intersect(interval);
  }
  FD_DX("Executor(" << this << ")::ComputeEnabled(): invariant is satisfied for " 
     << mETime.Str() );
  // no events for all time that can pass to begin with ...
  mEEvents.Clear();
  mEEvents.Name("EnabledEvents");
  mDEvents=Alphabet();
  mDEvents.Name("DisabledEvents");
  mEInterval=mETime;
  // iterate over all transitions and check guards
  TransSet::Iterator tit;
  for(tit=mATrans.Begin(); tit!= mATrans.End(); tit++) {
    // hypothesis: transition is enabled for all time
    bool enabled=true;
    TimeInterval enabledtime;
    enabledtime.SetPositive(); 
    // check all clocks
    std::map<Idx,TimeInterval> clockintervalmap=mTransClockIntervalMap[*tit];
    std::map<Idx,TimeInterval>::const_iterator cit;
    for(cit=clockintervalmap.begin(); cit!=clockintervalmap.end(); cit++) {
      Idx clock =  cit->first;
      Time::Type clockvalue = mCurrentTimedState.ClockValue[clock];
      TimeInterval interval = cit->second;
      // reject transition if a clock violates a guard constraint
      if(!  interval.In(clockvalue) ) enabled=false;
      // left shift interval by clock value to obtain an interval relative to current time
      interval.PositiveLeftShift(clockvalue);
      // intersect with enabled interval
      enabledtime.Intersect(interval);
    }
    // record guard interval
    mEGuardInterval[tit->Ev]=enabledtime;
    // intersect with invariant
    enabledtime.Intersect(mETime);
    // intersect with time hypothesis
    if(enabled) {
      // easy: intersect enabledtime with mEInterval  
      FD_DX("Executor(" << this << ")::ComputeEnabled(): event " << EStr(tit->Ev) 
         << " is enabled for " << enabledtime.Str());
      mEEvents.Insert(tit->Ev);
      mDEvents.Erase(tit->Ev);
      mEInterval.Intersect(enabledtime);
    } else {
      // first compute disabledtime, then intersect
      TimeInterval disabledtime;
      disabledtime.SetPositive();
      if(!enabledtime.Empty()) {
        disabledtime.UB(enabledtime.LB());
        disabledtime.UBincl(!enabledtime.LBincl());
      }
      FD_DX("Executor(" << this << ")::ComputeEnabled(): event " << EStr(tit->Ev) 
	    << " is disabled for " << disabledtime.Str() << " and enabled for " << enabledtime.Str());
      mEInterval.Intersect(disabledtime);
    }
  }  // loop transitions
  FD_DX("Executor(" << this << ")::ComputeEnabled(): e/d status constant on "
    << mEInterval.Str() );    
  mEValid=true; 
}

// EnabledTime(void)  
const TimeInterval& Executor::EnabledTime(void) const {
  if(!mEValid) ComputeEnabled();
  return mETime;
}

// EnabledEvents(void)  
const EventSet& Executor::EnabledEvents(void) const {
  if(!mEValid) ComputeEnabled();
  return mEEvents;
}

// DisabledEvents(void)  
const EventSet& Executor::DisabledEvents(void) const {
  if(!mEValid) ComputeEnabled();
  return mDEvents;
}

// EnabledInterval(void)  
const TimeInterval& Executor::EnabledInterval(void) const {
  if(!mEValid) ComputeEnabled();
  return mEInterval;
}

// ActiveEventSet(void)  
const EventSet& Executor::ActiveEventSet(void) const {
  if(!mEValid) ComputeEnabled();
  return mAEvents;
}

// ActiveTransSet(void)  
const TransSet& Executor::ActiveTransSet(void) const {
  if(!mEValid) ComputeEnabled();
  return mATrans;
}

// EnabledEventInterval(event)	 
TimeInterval Executor::EnabledEventTime(Idx event) const {
  if(!mEValid) ComputeEnabled();
  TimeInterval res;  
  std::map<Idx,TimeInterval>::const_iterator eit;
  eit=mEGuardInterval.find(event);
  if(eit!= mEGuardInterval.end()) {
    res = eit->second;
    res.Intersect(mETime);
    return res;
  }
  if(Alphabet().Exists(event)) {
     res.SetEmpty();
     return res;
  }
  res.SetPositive();
  return res;
}

// EnabledGuardInterval(event)	 
TimeInterval Executor::EnabledGuardTime(Idx event) const {
  if(!mEValid) ComputeEnabled();
  std::map<Idx,TimeInterval>::const_iterator eit;
  eit=mEGuardInterval.find(event);
  if(eit!= mEGuardInterval.end()) return eit->second;
  TimeInterval res;  
  res.SetEmpty();
  if(Alphabet().Exists(event)) return res;
  res.SetPositive();
  return res;
}

// ExecuteTime(time) 
bool Executor::ExecuteTime(Time::Type time) {
  if(mCurrentTime>=Time::Max()) return false;
  if(!mEValid) ComputeEnabled();
  if(!mETime.In(time) && !((time==Time::Max()) && mETime.UBinf()) ) {
    FD_DX("Executor(" << this << ")::ExecuteTime: execution of  " << time 
       << " conflicts with enabled status " );
    return false; 
  }
  FD_DX("Executor(" << this << ")::ExecuteTime(" << time << ")");
  // progress current time
  mCurrentTime += time;
  // progres clocks
  ClockSet::Iterator cit;
  for(cit=ClocksBegin(); cit!=ClocksEnd(); cit++) 
    mCurrentTimedState.ClockValue[*cit]= mCurrentTimedState.ClockValue[*cit]+time;  
  // fix infinity
  if(time==Time::Max()) {
    mCurrentTime=Time::Max();
    for(cit=ClocksBegin(); cit!=ClocksEnd(); cit++) 
      mCurrentTimedState.ClockValue[*cit]= Time::Max();
    mEValid=false;
  }  
  // progress enabled interval or invalidate enabled status 
  if(mEInterval.In(time) && mEValid) {
    mEInterval.PositiveLeftShift(time);
    mETime.PositiveLeftShift(time);
    std::map<Idx,TimeInterval>::iterator iit;
    for(iit=mEGuardInterval.begin();iit != mEGuardInterval.end(); iit++) {
      iit->second.PositiveLeftShift(time);
    }
  } else {
    mEValid=false;
  }
  return true;
}

// ExecuteEvent(event) 
bool Executor::ExecuteEvent(Idx event) {
  if(mCurrentTime>=Time::Max()) return false;
  if(!mEValid) ComputeEnabled();
  if(!mEEvents.Exists(event)) {
    FD_DX("Executor(" << this << ")::ExecuteEvent: execution of event " << EStr(event) 
       << " conflicts with enabled status " );
    return false; 
  }
  FD_DX("Executor(" << this << ")::ExecuteEvent(" << EStr(event) << ")");
  // pick transition
  TransSet::Iterator tit=TransRelBegin(mCurrentTimedState.State,event);
  // TODO: invalid iterator error     
  // execute resets
  ClockSet resets=Resets(*tit);
  ClockSet::Iterator cit;
  for(cit = resets.Begin(); cit!=resets.End(); cit++) 
    mCurrentTimedState.ClockValue[*cit]=0;
  // progress state
  mCurrentTimedState.State=tit->X2;
  // progress current time
  mCurrentStep += 1;
  // invalidate
  mEValid=false;
  return true;
}  


// CurrentTimedState(tstate)
bool Executor::CurrentTimedState(const TimedState& tstate) {
  // test consitency
  if(!ExistsState(tstate.State)) return false;
  if(tstate.ClockValue.size()!=ClocksSize()) return false;
  std::map<Idx,Time::Type>::const_iterator cvit;
  for(cvit=tstate.ClockValue.begin(); cvit!=tstate.ClockValue.end(); cvit++)
    if(!ExistsClock(cvit->first)) return false;
  // set state
  mCurrentTimedState=tstate;
  mEValid=false;
  return true; 
}
  
// CurrentTimedState()
const Executor::TimedState& Executor::CurrentTimedState(void) const {
  return mCurrentTimedState;
}
  
// CurrentState(idx)
bool Executor::CurrentState(Idx index) {
  if(!ExistsState(index)) return false;
  mCurrentTimedState.State=index;
  mEValid=false;
  return true;
}
  
// CurrentState()
Idx Executor::CurrentState(void) const {
  return mCurrentTimedState.State;
}
  
// CurrentClockValue(idx,time)
bool Executor::CurrentClockValue(Idx index, Time::Type time) {
  if(!ExistsClock(index)) return false;
  mCurrentTimedState.ClockValue[index]=time;
  mEValid=false;
  return true;
}
  
// CurrentClockValue(idx)
Time::Type Executor::CurrentClockValue(Idx index) const {
  std::map<Idx,Time::Type>::const_iterator cvit;
  cvit=mCurrentTimedState.ClockValue.find(index);
  if(cvit==mCurrentTimedState.ClockValue.end()) {}; // todo: error
  return cvit->second;
}
  
// CurrentTime(time)
void Executor::CurrentTime(Time::Type time) {
  mCurrentTime=time;
  mEValid=false;
}
  
// CurrentTime()
Time::Type Executor::CurrentTime(void) const {
  return mCurrentTime;
}
  
// CurrentStep(time)
void Executor::CurrentStep(int step) {
  mCurrentStep=step;
  mEValid=false;
}
  
// CurrentStep()
int Executor::CurrentStep(void) const {
  return mCurrentStep;
}
  
// IsDeadlocked() 
bool Executor::IsDeadlocked(void) const {
  if(!mEValid) ComputeEnabled();
  if(!mEEvents.Empty()) return false;
  if(!(mETime.UB()<=0)) return false;
  return true;
}

// TSStr(tstate)
std::string Executor::TSStr(const TimedState& tstate) const {
  std::stringstream res;
  res << "(state " << SStr(tstate.State); 
  if(Generator().ExistsMarkedState(tstate.State))  res << " [marked]";
  res << ")  (clocks";
  ClockSet::Iterator cit;
  for(cit=ClocksBegin();cit!=ClocksEnd();cit++){
    res << " " << CStr(*cit) << "=";
    std::map<Idx,Time::Type>::const_iterator cvit=tstate.ClockValue.find(*cit);
    if(cvit!=tstate.ClockValue.end()) 
      res << cvit->second;
    else
      res << "undef";
  }
  res << ")";
  return res.str();
}

// CurrentTimedStateStr()
std::string Executor::CurrentTimedStateStr(void) const {
  return TSStr(mCurrentTimedState);
}

// TEStr(tevent)
std::string Executor::TEStr(const TimedEvent& tevent) const {
  std::stringstream res;
  res << "(" << EStr(tevent.mEvent) << " at " << tevent.mTime << ")";
  return res.str();
}

// EStr(event)
std::string Executor::EStr(Idx event) const {
  return TimedGenerator::EStr(event);
}

// CStr(clock)
std::string Executor::CStr(Idx clock) const {
  return TimedGenerator::CStr(clock);
}

// SStr(clock)
std::string Executor::SStr(Idx state) const {
  return TimedGenerator::SStr(state);
}


} // namespace faudes


