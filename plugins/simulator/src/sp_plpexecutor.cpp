/** @file sp_plpexecutor.cpp Executor that proposes transitions to execute */

/* 
   FAU Discrete Event System Simulator 

   Copyright (C) 2007  Christoph Doerr
   Copyright (C) 2008  Thomas Moor
   Exclusive copyright is granted to Thomas Moor

*/

#include "sp_plpexecutor.h"
#include <cmath>

namespace faudes {

// std faudes type
FAUDES_TYPE_IMPLEMENTATION(ProposingExecutor, ProposingExecutor, ParallelExecutor)

// constructor
ProposingExecutor::ProposingExecutor(void) : LoggingExecutor() { 
  mPValid=false;
}

// copy constructor
ProposingExecutor::ProposingExecutor(const ProposingExecutor& rOther) : LoggingExecutor() 
{ 
  mPValid=false;
  Assign(rOther);
}

// clear static data
void ProposingExecutor::Clear(void){
  LoggingExecutor::Clear();
  mSimEvents.Clear();
  mPValid=false;
}

// DoAssign(other)
void ProposingExecutor::DoAssign(const ProposingExecutor& rOther) {
  FD_DX("ProposingExecutor(" << this << ")::DoAssign(other)");
  // my members
  mSimEvents.Assign(rOther.mSimEvents);
  // base
  LoggingExecutor::DoAssign(rOther);
}

// lookup event attribute
const SimEventAttribute& ProposingExecutor::EventAttribute(Idx index) const {
  return mSimEvents.Attribute(index);
}
     
// set simulation event attribute
void ProposingExecutor::EventAttribute(Idx index, const SimEventAttribute& rAttr) {
  mSimEvents.Attribute(index,rAttr);
}

// set execution properties from attributed event set
void ProposingExecutor::Alphabet(const sEventSet& rAlphabet) {
  // set ...
  mSimEvents=rAlphabet;
  // ... and ensure to cover all relevant alphabet
  mSimEvents.InsertSet(LoggingExecutor::Alphabet()); 
}

// clear dynamic data (reset state) 
void ProposingExecutor::ResetProposer(long int seed){
  FD_DS("ProposingExecutor::ResetProposer()");
  //Initialize random number generator with computer system time as seed
  if(seed == 0) {
    faudes_systime_t now;
    faudes_gettimeofday(&now);
    seed=now.tv_sec;
  }
  ran_init(seed);
  // schedule all events
  EventSet::Iterator eit;
  for(eit=Alphabet().Begin(); eit!=Alphabet().End(); ++eit){
    SimEventAttribute* pattr= mSimEvents.Attributep(*eit);
    pattr->mScheduledFor=Time::UnDef();
    pattr->mDelayFor=Time::UnDef();
    pattr->mExpiresAt=0;
    if(pattr->IsPriority()) continue;
    if(!pattr->IsStochastic()) continue;
    Schedule(*eit,pattr);
  }
  // invalidate proposal
  mPValid=false;
  FD_DS("ProposingExecutor::ResetProposer(): done");


}

// clear dynamic data (reset state) 
void ProposingExecutor::Reset(long int seed){
  FD_DS("ProposingExecutor::Reset()");
  LoggingExecutor::Reset();
  mSimEvents.InsertSet(LoggingExecutor::Alphabet()); 
  ResetProposer(seed);
  FD_DS("ProposingExecutor::Reset(): done");
}


//EventStatesToString
std::string ProposingExecutor::EventStatesToString() const {
  std::stringstream retstr;
  EventSet::Iterator eit;
  for(eit=mSimEvents.Begin(); eit!=mSimEvents.End(); ++eit){
    const SimEventAttribute& attr= mSimEvents.Attribute(*eit);
    retstr<< "% simulation state: "<< ExpandString(mSimEvents.Str(*eit),FD_NAMELEN) << ": ";
    retstr<< "enabled: " << ExpandString(EnabledEventTime(*eit).Str(),FD_NAMELEN) << ": ";
    retstr<< attr.Str();
    if( eit!= --mSimEvents.End()) {
      retstr << std::endl;
    }
  }
  return retstr.str();
}



//ExecuteTime: todo: may hang + performance issue 
bool ProposingExecutor::ExecuteTime(Time::Type duration){
  FD_DS("ProposingExecutor::ExecuteTime(): LoggingExecutor to execute time "<< duration);
  if(!LoggingExecutor::ExecuteTime(duration)) return false;
  // update stochastic event state
  EventSet::Iterator eit;
  for(eit=mSimEvents.Begin(); eit!=mSimEvents.End(); ++eit) {
    SimEventAttribute* pattr= mSimEvents.Attributep(*eit);
    // stochastic only
    if(!pattr->IsStochastic()) continue;
    // shift relative time: schedule/expiring time
    if(pattr->mScheduledFor!=Time::UnDef()) 
    if(pattr->mScheduledFor!=Time::Max()) 
      pattr->mScheduledFor -= duration;
    if(pattr->mExpiresAt!=Time::UnDef()) 
      if(pattr->mExpiresAt!=Time::Max()) 
      pattr->mExpiresAt -= duration;
    // shift reference interval for type trigger
    if(pattr->Stochastic().mType==SimStochasticEventAttribute::Trigger) {
      pattr->mReferenceInterval.PositiveLeftShift(duration);
    }
    // shift reference interval for type delay
    if(pattr->Stochastic().mType==SimStochasticEventAttribute::Delay) {
      if(!pattr->mReferenceInterval.Empty()) {
        Time::Type delay = duration - pattr->mReferenceInterval.LB();
        if(delay>0) pattr->mDelayFor-=delay;
        pattr->mReferenceInterval.PositiveLeftShift(duration);
      }
    }
    // re-schedule (required for type extern and delay)
    if(pattr->mExpiresAt<=0) Schedule(*eit,pattr);
  }
  FD_DS("ProposingExecutor::ExecuteTime(): LoggingExecutor to execute time: done ");
  // case a: no valid proposal anyway
  if(!mPValid) return true;
  // case b: proposal remains valid
  if(mProposal.mTime==Time::Max()) return true;
  // case c: proposal becomes invalid
  if(mProposal.mTime < duration) { mPValid=false; return true;}
  // case d: adjust proposal
  mProposal.mTime-=duration;
  mPValid= (mProposal.mTime>0) || (mProposal.mEvent!=0);
  return true;
}

//ExecuteEvent
bool ProposingExecutor::ExecuteEvent(Idx event){
  FD_DS("ProposingExecutor::ExecuteEvent(): ProposingExecutor to execute event "<< mSimEvents.Str(event));

  // (try to) execute
  if(!LoggingExecutor::ExecuteEvent(event)) return false;
  FD_DS("ProposingExecutor::ExecuteEvent(): executed "<< EventName(event));  

  // record enabled events
  const EventSet newActiveEvents=ActiveEventSet(CurrentParallelState());		
  FD_DS("ProposingExecutor::ExecuteEvent(): new active events "<< newActiveEvents.ToString());   


  // invalidate/re-schedule event states
  EventSet::Iterator eit;
  for(eit=mSimEvents.Begin(); eit!=mSimEvents.End(); ++eit) {
    SimEventAttribute* pattr= mSimEvents.Attributep(*eit);
    // stochastic only
    if(!pattr->IsStochastic()) continue;
    // invalidate schedule: type trigger 
    if(pattr->Stochastic().mType==SimStochasticEventAttribute::Trigger) {
      TimeInterval gtime=EnabledGuardTime(*eit);
      gtime.Canonical();
      if(gtime!=pattr->mReferenceInterval) {
        FD_DS("ProposingExecutor::ExecuteEvent(): invalidating trigger type event "<< EStr(*eit));
        pattr->mExpiresAt = 0;
      }
    }
    // update state: type delay (only if not expired)
    if(pattr->Stochastic().mType==SimStochasticEventAttribute::Delay) 
    if(pattr->mExpiresAt > 0) {
      TimeInterval etime=EnabledEventTime(*eit);
      etime.Canonical();
      pattr->mReferenceInterval=etime;
      pattr->mScheduledFor=Time::UnDef();
      pattr->mExpiresAt=Time::Max();
      Time::Type schedule= etime.LB()+ pattr->mDelayFor;
      if(etime.In(schedule)) {
        pattr->mScheduledFor=schedule;
        pattr->mExpiresAt=pattr->mScheduledFor+1;
        FD_DS("ProposingExecutor::ExecuteEvent(): delay event " << EStr(*eit) << ": etime " 
             << etime.Str() << " scheduled " << schedule);
      }
    }
    // invalidate schedule: event executed
    if(*eit==event) {
      FD_DS("ProposingExecutor::ExecuteEvent(): invalidating state for executed event "<< EStr(*eit));
      pattr->mExpiresAt = 0;
    }
    /*
    // invalidate schedule: type all reset
    if(pattr->Stochastic().mType==SimStochasticEventAttribute::State) {
      pattr->mExpiresAt = 0;
      pattr->mScheduledFor = 0;
      pattr->mDelayFor = Time::UnDef();
      FD_DS("ProposingExecutor::ExecuteEvent(): invalidating state for state-type " << EStr(*eit));
    }
    */
    /*
    // invalidate schedule: type intern
    if(pattr->Stochastic().mType==SimStochasticEventAttribute::Intern) {
      if(!EventValidity(*eit, event, oldActiveStateVec,newActiveStateVec)){
        pattr->mExpiresAt = 0;
        pattr->mScheduledFor = 0;
        pattr->mDelayFor = Time::UnDef();
        FD_DS("ProposingExecutor::ExecuteEvent(): invalidating state for intern-type " << EStr(*eit));
      }
    }
    */
    // re-schedule
    if(newActiveEvents.Exists(*eit))
    if(pattr->mExpiresAt<=0) Schedule(*eit,pattr);
  } // loop all events

  // invalidate proposal
  mPValid=false;
  // done
  FD_DS("ProposingExecutor::ExecuteEvent(): done");
  return true;	
}


//ExecuteTransition (rel time!) (event 0 for time only)
bool ProposingExecutor::ExecuteTransition(const TimedEvent& executeEvent){

  if(!ExecuteTime(executeEvent.mTime)) return false;
  if(executeEvent.mEvent==0) return true;
  if(!ExecuteEvent(executeEvent.mEvent)) return false;

  return true;
}



//ProposeNextTransition (event 0 for time only)
// todo: combine loops for performance
const TimedEvent& ProposingExecutor::ProposeNextTransition(void){

  FD_DS("ProposingExecutor::ProposeNextTransition()");

  // return valid proposal
  if(mPValid) return mProposal;

  // compute valid proposal now
  mPValid=true;

  // vars
  EventSet::Iterator eit;
  TimeInterval enabledInterval=EnabledInterval();
  TimeInterval enabledTime=EnabledTime();
  const EventSet enabledEvents = EnabledEvents();

  // report
  FD_DS("\n" << EventStatesToString());
  FD_DS("ProposingExecutor::ProposeNextTransition(): current time: " << 
    CurrentTime()); 
  FD_DS("ProposingExecutor::ProposeNextTransition(): timed state: " <<
    CurrentParallelTimedStateStr());
  FD_DS("ProposingExecutor::ProposeNextTransition(): active events: " <<
      ActiveEventSet(CurrentParallelState()).ToString());
  FD_DS("ProposingExecutor::ProposeNextTransition(): enabled interval: " <<
    enabledInterval.Str());
  FD_DS("ProposingExecutor::ProposeNextTransition(): enabled time: " <<
    enabledTime.Str());
  FD_DS("ProposingExecutor::ProposeNextTransition(): enabled events: " << std::endl <<
    EnabledEvents().ToString());

  // find candidate to execute
  Idx candidate=0;
  std::vector<faudes::Idx> candidates;
  Time::Type passtime=0;

  //check if a priority event occurs in analyzed interval 
  long int prio = -1;
  for(eit=enabledEvents.Begin(); eit!=enabledEvents.End(); ++eit) {
    SimEventAttribute* pattr= mSimEvents.Attributep(*eit);
    if(!pattr->IsPriority()) continue;
    // priority wins
    if(pattr->Priority().mPriority >prio) {
      prio= pattr->Priority().mPriority;
      candidate=*eit;
      candidates.clear();
      candidates.push_back(candidate);
    }
    // indicate non determinism
    if((pattr->Priority().mPriority ==prio) && (prio>=0)) 
      candidates.push_back(*eit);
  }

  // choose arbitrarily
  if(candidates.size()>1) {
    candidate=candidates[ran_uniform_int(0,candidates.size())];
  }

  // propose priority candidate now
  if(candidate>0) {
    FD_DS("ProposingExecutor::ProposeNextTransition(): propose by priority: " << mSimEvents.Str(candidate));
    mProposal.mEvent=candidate;
    mProposal.mTime=0;
    return mProposal;
  }

  //check if stochastic  events are enabled
  Time::Type occurence = -1;
  for(eit=enabledEvents.Begin(); eit!=enabledEvents.End(); ++eit){
    SimEventAttribute* pattr= mSimEvents.Attributep(*eit);
    if(!pattr->IsStochastic()) continue;
    if(pattr->mScheduledFor == Time::UnDef()) continue; // dont need this anyway
    if(enabledInterval.In(pattr->mScheduledFor)) { 
      // occurence wins
      if(pattr->mScheduledFor < occurence || occurence < 0) {
        occurence= pattr->mScheduledFor;
        candidate=*eit;
        candidates.clear();
        candidates.push_back(candidate);
      }
      // indicate non determinism
      if(pattr->mScheduledFor == occurence) 
        candidates.push_back(*eit);
    }
  }

  // choose arbitrarily
  if(candidates.size()>1) {
    candidate=candidates[ran_uniform_int(0,candidates.size())];
  }

  // propose stochastic candidate now
  if(candidate>0) {
    FD_DS("ProposingExecutor::ProposeNextTransition(): propose by stochastic: " << mSimEvents.Str(candidate) << " for " << Time::Str(occurence));
    mProposal.mEvent=candidate;
    mProposal.mTime=occurence;
    return mProposal;
  }

  // check if we can pass time
  if(!enabledTime.Empty()) {
    // pass enabled interval
    passtime=enabledInterval.UB();
    if(enabledInterval.Empty()) passtime=0;
    // be sure to be beyond enabled interval but within enabled time
    if(enabledInterval.UBincl()) passtime+=Time::Step();
    // be sure to be within enabled time
    if(!enabledTime.In(passtime)) passtime-=Time::Step();
    // fix infty
    if(enabledTime.UBinf() && enabledInterval.UBinf()) passtime=Time::Max();
    // if we can go for ever,  step with minstep 
    // if(passtime==Time::Max()) passtime=Time::Step(); 
    // only execute relevant steps -- otherwise max execute priority event
    if(passtime!=0) {    
      FD_DS("ProposingExecutor::ProposeNextTransition(): propose to pass time: "<< Time::Str(passtime));
      mProposal.mEvent=0;
      mProposal.mTime=passtime;
      return mProposal;
    }
  }

  //check if prevent priority event occurs in analyzed interval 
  prio =0;
  for(eit=enabledEvents.Begin(); eit!=enabledEvents.End(); ++eit){
    SimEventAttribute* pattr= mSimEvents.Attributep(*eit);
    if(!pattr->IsPriority()) continue;
    // priority wins
    if(pattr->Priority().mPriority >prio || prio==0) {
      prio= pattr->Priority().mPriority;
      candidate=*eit;
      candidates.clear();
      candidates.push_back(candidate);
    }
    // alphabetical  
    if((pattr->Priority().mPriority ==prio) && (prio <0)) 
      candidates.push_back(*eit);
  }

  // choose arbitrarily
  if(candidates.size()>1) {
    candidate=candidates[ran_uniform_int(0,candidates.size())];
  }

  // propose prevent priority candidate now
  if(candidate>0) {
    FD_DS("ProposingExecutor::ProposeNextTransition(): propose by priority: " << mSimEvents.Str(candidate));
    mProposal.mEvent=candidate;
    mProposal.mTime=passtime;
    return mProposal;
  }

  //Deadlock request
  if(IsDeadlocked()){
    FD_DS("ProposingExecutor::ProposeNextTransition(): Deadlocked");
    mProposal.mEvent=0;
    mProposal.mTime=Time::UnDef();
    return mProposal;
  }														

  // nothing to propose but time 
  FD_DS("ProposingExecutor::ProposeNextTransition(): Lifelock");
  mProposal.mEvent=0;
  mProposal.mTime=Time::Step();
  return mProposal;
}


//ExecuteNextTransition
TimedEvent ProposingExecutor::ExecuteNextTransition(void){
  FD_DS("ProposingExecutor::ExecuteNextTransition() *********************************************************** ");

  Time::Type currentTime=0;
  TimedEvent execTimedEvent;

  //********************************************
  //Execution loop
  //********************************************
  while(true){

    // get proposal
    execTimedEvent = ProposeNextTransition();

    // deadlocked? (todo: mintime/min float number etc)
    if((execTimedEvent.mEvent==0) && (execTimedEvent.mTime==0)) break;
    if(IsDeadlocked()) break;

    // execute (time and/or event)
    ExecuteTransition(execTimedEvent);
 
    // done if there was an event
    if(execTimedEvent.mEvent!=0) break;

    // record time
    currentTime+=execTimedEvent.mTime;

  }

  // success  
  if(execTimedEvent.mEvent!=0) {
     execTimedEvent.mTime=currentTime;
     return execTimedEvent;
  }

  // failure
  execTimedEvent.mTime=Time::UnDef();
  execTimedEvent.mEvent=0;
  return execTimedEvent;
}



//Schedule() by attribute
Time::Type ProposingExecutor::Schedule(Idx event, SimEventAttribute* pattr) {
  FD_DS("ProposingExecutor::Schedule(" << EStr(event) << ")");
  // insist in stochastic attribute
  if(!pattr->IsStochastic()) {
    FD_DS("ProposingExecutor::Schedule(" << EStr(event) << "): no stoxchastic attribute found");
    pattr->mScheduledFor=Time::UnDef();
    return Time::UnDef();
  }
  // only reschedule when expired
  if(pattr->mExpiresAt>0) {
    FD_DS("ProposingExecutor::Schedule(" << EStr(event) << "): event not expired");
    return pattr->mScheduledFor;
  }
  // invalidate event state;
  Time::Type recentschedule=pattr->mScheduledFor;
  pattr->mScheduledFor=Time::UnDef();
  pattr->mExpiresAt=Time::UnDef();
  pattr->mDelayFor=Time::UnDef();
  pattr->mReferenceInterval.SetEmpty();
  // target interval for sample, defaults to positive axis
  TimeInterval atarget;
  atarget.SetPositive();
  // trigger type events have restricted target
  if(pattr->Stochastic().mType == SimStochasticEventAttribute::Trigger) {
    atarget.Intersect(EnabledGuardTime(event));
    atarget.Canonical();
  }
  // cannot have empty target
  if(atarget.Empty()) {
     FD_DS("ProposingExecutor::Schedule(" << EStr(event) << "): empty target");
     return Time::UnDef();
  }   
  // report timed automaton target
  FD_DS("ProposingExecutor::Schedule(" << EStr(event) << "): for timed automaton target " << atarget.Str());
  // get pdf parameters to figure restricted support 
  std::vector<Float> pdfparavec=pattr->Stochastic().mParameter;
  TimeInterval starget;
  starget.LB(Time::FromFloat(pdfparavec[0]));
  starget.UB(Time::FromFloat(pdfparavec[1]));
  starget.LBincl(true);
  starget.UBincl(false);	
  atarget.Intersect(starget);
  FD_DS("ProposingExecutor::Schedule(" << EStr(event) << "): for restricted target " << atarget.Str());
  if(atarget.Empty()) {
    FD_DS("ProposingExecutor::Schedule(" << EStr(event) << "): empty target");
    return Time::UnDef();
  }
  // do the sample
  double occtime=-1; 
  switch(pattr->Stochastic().mPdf){
  // exponential
  case SimStochasticEventAttribute::Exponential:
    occtime=ran_exponential(pdfparavec[2],atarget.LB(),atarget.UB());
    break;
  // gauss
  case SimStochasticEventAttribute::Gauss:
    occtime=ran_gauss(pdfparavec[2],pdfparavec[3],atarget.LB(),atarget.UB());
    break;
  //Uniform
  case SimStochasticEventAttribute::Uniform: 
    occtime=ran_uniform(atarget.LB(),atarget.UB());
    break;
  //Undefined 
  case SimStochasticEventAttribute::Vector:
    FD_DS("ProposingExecutor::Schedule(" << EStr(event) << ") internal error: no valid stochastic defs");
    return Time::UnDef();
    break;
  }
  // report failure
  if(occtime < 0)  {
    FD_DS("ProposingExecutor::Schedule(" << EStr(event) << "): sampling failed: A");
    return Time::UnDef();
  }
  // discretize Time::Type 
  Time::Type schedule = Time::Quantize(occtime);
  FD_DS("ProposingExecutor::Schedule(" << EStr(event) << "): random sample " << schedule 
    << "(" << occtime << ")");
  // interpret sample
  switch(pattr->Stochastic().mType){
  // interpret sample: trigger type
  case SimStochasticEventAttribute::Trigger:
    // paranoid ... fix rounding issues
    if(!atarget.In(schedule)) schedule+=Time::Step();
    if(!atarget.In(schedule)) schedule-=2*Time::Step();
    if(!atarget.In(schedule)) schedule= Time::UnDef();
    // paranoid ... report failure
    if(!atarget.In(schedule))  {
      FD_DS("ProposingExecutor::Schedule(" << EStr(event) << "): sampling failed: B");
      return Time::UnDef();
    }
    // set schedule
    pattr->mScheduledFor=schedule;
    pattr->mExpiresAt=atarget.UB();
    pattr->mDelayFor=Time::UnDef();
    pattr->mReferenceInterval=atarget;
    break;   
  // interpret sample: extern type // issue: re-sampling can hang
  case SimStochasticEventAttribute::Extern:
    pattr->mScheduledFor=schedule;
    if(recentschedule!=Time::UnDef()) 
      pattr->mScheduledFor+=recentschedule;
    pattr->mExpiresAt=pattr->mScheduledFor+1;
    pattr->mDelayFor=Time::UnDef();
    pattr->mReferenceInterval.SetEmpty();
    if(pattr->mScheduledFor<0) {   
      FD_DS("ProposingExecutor::Schedule(" << EStr(event) << "): re-sampling");
      return Schedule(event,pattr);
    }
    break;
  // interpret sample: delay type
  case SimStochasticEventAttribute::Delay:
    TimeInterval etime=EnabledEventTime(event);
    etime.Canonical();
    FD_DS("ProposingExecutor::Schedule(" << EStr(event) << "): delay type: etime " 
          << etime.Str() << " delay for " << schedule);
    pattr->mDelayFor=schedule;
    pattr->mReferenceInterval=etime;
    pattr->mScheduledFor=Time::UnDef();
    pattr->mExpiresAt=Time::Max();
    schedule+=etime.LB();
    if(etime.In(schedule)) {
      pattr->mScheduledFor=schedule;
      pattr->mExpiresAt=pattr->mScheduledFor+1;
    }
    return pattr->mScheduledFor;
  }
  // done
  return pattr->mScheduledFor;
}
	 


//DoWrite(rTr,rLabel)
void ProposingExecutor::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  std::string label=rLabel;
  if(label=="") label = "Executor";
  rTw.WriteBegin(label);
  ParallelExecutor::DoWriteGenerators(rTw);
  Conditions().Write(rTw,"Conditions",this);
  mSimEvents.Write(rTw,"SimEvents");
  rTw.WriteEnd(label);
}

//DoRead(rTr,rLabel)
void ProposingExecutor::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) pContext;
  FD_DC("ProposingExecutor::DoRead(rTr, " << rLabel<<")");
  std::string label=rLabel;
  if(label=="") label = "Executor";
  rTr.ReadBegin(label);

  while(!rTr.Eos(label)) {
    FD_DC("ProposingExecutor::DoRead(rTr, " << rLabel<<"): loop");
    // peek token
    Token token;
    rTr.Peek(token);
    // case 1: generators
    if(token.Type()==Token::Begin)
    if(token.StringValue()=="Generators") {
      ParallelExecutor::DoReadGenerators(rTr);
      continue;
    }
    // case 2: conditions
    if(token.Type()==Token::Begin)
    if(token.StringValue()=="Conditions") {
      SimConditionSet conditions;
      conditions.Read(rTr,"Conditions",this);
      Conditions(conditions);
      continue;
    }
    // case 3: event attributes
    if(token.Type()==Token::Begin)
    if(token.StringValue()=="SimEvents") {
      mSimEvents.Read(rTr,"SimEvents");
      continue;
    }
    // case 3: event attributes (compatibility mode)
    if(token.Type()==Token::Begin)
    if(token.StringValue()=="SimEventAttributes") {
      mSimEvents.Read(rTr,"SimEventAttributes");
      continue;
    }
    // else report error
    std::stringstream errstr;
    errstr << "Invalid token, generators, conditions or simeventattribute section expected, " << rTr.FileLine();
    throw Exception("ProposingExecutor::DoRead", errstr.str(), 502);
  }
  rTr.ReadEnd(label);
  FD_DC("ProposingExecutor::DoRead(rTr, " << rLabel<<"): done");
  Reset();
}


// revert executors state to past step from buffer
bool ProposingExecutor::RevertToStep(Idx step) {
  FD_DX("ProposingExecutor(" << this << ")::RevertToStep("<< step << ")");
  // call base
  bool res=LoggingExecutor::RevertToStep(step);
  // bail out
  if(!res) return false;
  // reset my dynamic data
  ResetProposer();
  // done
  return true;
}

} // end namespace





/*
      Scratch: how to record relevant clocks in Schedule()
      This might be interesting to sense resets that invalidate delay type events

      // record relevant clocks
      pattr->mRelevantClocks.Clear();
      for(Iterator git=Begin(); git!=End(); git++){ 
        pattr->mRelevantClocks.InsertSet(git->Generator().Invariant(git->CurrentState()).ActiveClocks());
        TransSet::Iterator tit = git->Generator().TransRelBegin(git->CurrentState(),*eit);
        pattr->mRelevantClocks.InsertSet(git->Generator().Guard(*tit).ActiveClocks());
      }
      FD_DS("ProposingExecutor::ExecuteEvent(): relevant clocks " << pattr->mRelevantClocks.ToString());

*/




/*
      Scratch: how to figure whether relevant clocks, guards, or invariants changed
      This was required for the old internal type events. Depreciated.


//EventValidity(Idx, Idx, T_state_vec, T_state_vec) 
bool ProposingExecutor::EventValidity(Idx ev, Idx executedEvent, 
      const ParallelState& oldStateVec, const ParallelState& newStateVec) {
  unsigned int i=0;
  Iterator xit;
  TransSet::Iterator tit;
  TimeConstraint::Iterator oldcit,newcit;
  ClockSet::Iterator clit;
  ClockSet* pclocks = &mSimEvents.Attributep(ev)->mRelevantClocks;

  //Check if event was indeed active and is is still active
  if(!Active(ev, oldStateVec)){ 
    FD_DS("ProposingExecutor::EventValidity(): Event "<<Alphabet().SymbolicName(ev)
	       << " has not been active in last parallel state");
    return false;
  }
  if(!Active(ev, newStateVec)){ 
    FD_DS("ProposingExecutor::EventValidity(): Event "<<Alphabet().SymbolicName(ev)
             <<" is not active in current parallel state");
    return false;
  }


  //Check if relevant clocks are affected
  FD_DS("ProposingExecutor::EventValidity(): test clocks for " << Alphabet().Str(ev));

  for(xit=Begin(); xit!=End(); ++xit, i++){
    // If this excutor did not execute anything, nothing changed at all
    if(!xit->Generator().ExistsEvent(executedEvent)) continue;	
    // If this excutor does not share the event, no guards/invs/resets affects its enabled status
    if(!xit->Generator().ExistsEvent(ev)) continue;	
    //Check if a reset of a relevant clock was executed
    tit=xit->Generator().TransRelBegin(oldStateVec[i],executedEvent);	
    FD_DS("ProposingExecutor::EventValidity(): test resets of " << Generator().TStr(*tit));
    const ClockSet resetClocks=xit->Generator().Resets(*tit);
    if(!(resetClocks * (*pclocks)).Empty()) {
      FD_DS("ProposingExecutor::EventValidity(): relevant clock reset in " << xit->Name());
      return false;
    }
    // If the state didnt change, neither did guards/invariants
    if(oldStateVec[i]==newStateVec[i]) continue;	
    //Check if guards have changed wrt relevant clocks
    tit=xit->Generator().TransRelBegin(oldStateVec[i],ev);
    FD_DS("ProposingExecutor::EventValidity(): compare old guard of " << Generator().TStr(*tit));
    TimeConstraint oldGuard=xit->Generator().Guard(*tit);
    tit=xit->Generator().TransRelBegin(newStateVec[i],ev);
    FD_DS("ProposingExecutor::EventValidity(): ... with guard of " << Generator().TStr(*tit));
    TimeConstraint newGuard=xit->Generator().Guard(*tit);
    for(ClockSet::Iterator cit=pclocks->Begin(); cit!=pclocks->End(); cit++) {
      if(oldGuard.Interval(*cit)!=newGuard.Interval(*cit)) { 
        FD_DS("ProposingExecutor::EventValidity(): invalidate for change in guard wrt clock " << 
          pclocks->SymbolicName(*cit));
        FD_DS("ProposingExecutor::EventValidity(): old guard " << oldGuard.ToString());
        FD_DS("ProposingExecutor::EventValidity(): new guard " << newGuard.ToString());
        return false;
      }
    }
    //Check if invariants have changed
    TimeConstraint oldInv=xit->Generator().Invariant(oldStateVec[i]);
    TimeConstraint newInv=xit->Generator().Invariant(newStateVec[i]);
    for(ClockSet::Iterator cit=pclocks->Begin(); cit!=pclocks->End(); cit++) {
      if( oldInv.Interval(*cit)!=newInv.Interval(*cit)) { 
        FD_DS("ProposingExecutor::EventValidity(): invalidate for change in invariant wrt clock " << 
          pclocks->SymbolicName(*cit));
        FD_DS("ProposingExecutor::EventValidity(): old inv " << oldInv.ToString());
        FD_DS("ProposingExecutor::EventValidity(): new inv " << newInv.ToString());
        return false;
      }
    }
  } // loop all generators
  FD_DS("ProposingExecutor::EventValidity(): Event " << Alphabet().SymbolicName(ev)<< " is still valid"); 
  return true;
}

*/







