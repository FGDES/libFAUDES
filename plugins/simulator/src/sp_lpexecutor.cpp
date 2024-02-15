/** @file sp_lpexecutor.cpp Executor with logging facilities  */

/* 
   FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2007, 2008 Thomas Moor
   Copyright (C) 2007  Ruediger Berndt
   Exclusive copyright is granted to Klaus Schmidt

*/


#include "sp_lpexecutor.h"

namespace faudes {


// std faudes type
FAUDES_TYPE_IMPLEMENTATION(LoggingExecutor,LoggingExecutor,ParallelExecutor)

// LoggingExecutor(void)
LoggingExecutor::LoggingExecutor(void) 
  : ParallelExecutor(), pLogTokenWriter(0), mLogFile(""), mLogMode(0) 
{
  FD_DX("LoggingExecutor(" << this << ")::LoggingExecutor()");
  TraceClear(0);
}

// LoggingExecutor(void)
LoggingExecutor::LoggingExecutor(const LoggingExecutor& rOther) 
  : ParallelExecutor(), pLogTokenWriter(0), mLogFile(""), mLogMode(0) 
{
  FD_DX("LoggingExecutor(" << this << ")::LoggingExecutor()");
  TraceClear(0);
  DoAssign(rOther);
}

// LoggingExecutor(rFileName)
LoggingExecutor::LoggingExecutor(const std::string& rFileName) 
  : ParallelExecutor(), pLogTokenWriter(0), mLogFile(""), mLogMode(0) 
{
  FD_DX("LoggingExecutor(" << this << ")::LoggingExecutor(" << rFileName << ")");
  TraceClear(0);
  Read(rFileName);
}

// LoggingExecutor
LoggingExecutor::~LoggingExecutor(void) { 
  FD_DX("LoggingExecutor(" << this << ")::~LoggingExecutor()");
  LogClose();
}

// DoAssign(other)
void LoggingExecutor::DoAssign(const LoggingExecutor& rOther) {
  FD_DX("LoggingExecutor(" << this << ")::DoAssign(other)");
  // my members
  mConditions.Assign(rOther.mConditions);
  // base
  ParallelExecutor::DoAssign(rOther);
}

// Compile()
void LoggingExecutor::Compile(void) {
  FD_DX("LoggingExecutor(" << this << ")::Compile(): #" << Size());
  // call base
  ParallelExecutor::Compile();
  // care about my members
  CompileConditions();
  FD_DX("LoggingExecutor(" << this << ")::Compile(): done");
}

// Reset()
void LoggingExecutor::Reset(void) {
  FD_DX("LoggingExecutor(" << this << ")::Reset()");
  // log trace
  LogWritePause();
  // call base (incl compile)
  ParallelExecutor::Reset();
  // log trace
  LogWriteResume();
  // clear and initialise trace (base was called first)
  TraceClear();
  // figure initial state
  ConditionsReset();
}


// Clear()
void LoggingExecutor::Clear(void) {
  FD_DX("LoggingExecutor(" << this << ")::Clear()");
  // clear my members
  LogClose();
  mConditions.Clear();
  mBreakCondition=false;
  TraceClear();
  // call base
  ParallelExecutor::Clear();
}

//DoWrite(rTr,rLabel)
void LoggingExecutor::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  std::string label=rLabel;
  if(label=="") label = "Executor";
  rTw.WriteBegin(label);
  ParallelExecutor::DoWrite(rTw,"Generators");
  mConditions.Write(rTw,"Conditions",this);
  rTw.WriteEnd(label);
}

//DoRead(rTr,rLabel)
void LoggingExecutor::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) pContext;
  FD_DC("LoggingExecutor::DoRead(rTr, " << rLabel<<")");
  std::string label=rLabel;
  if(label=="") label = "Executor";
  rTr.ReadBegin(label);
  while(!rTr.Eos(label)) {
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
      mConditions.Read(rTr,"Conditions",this);
      continue;
    }
    // else report error
    std::stringstream errstr;
    errstr << "Invalid token, generators or conditions section expected, " << rTr.FileLine();
    throw Exception("LoggingExecutor::DoRead", errstr.str(), 502);
  }
  rTr.ReadEnd(label);
  Reset();
}



// ExecuteTime(time) 
bool LoggingExecutor::ExecuteTime(Time::Type time) {
  // call base
  bool res=ParallelExecutor::ExecuteTime(time);
  // bail out 
  if(!res) return false;
  // update trace (after base)
  TraceUpdateTime();
  // done
  return res;
}

// ExecuteEvent(event) 
bool LoggingExecutor::ExecuteEvent(Idx event) {
  // call base
  bool res=ParallelExecutor::ExecuteEvent(event);
  // bail out 
  if(!res) return false;
  // process conditions
  ConditionsProcess();
  // record log
  LogWriteTime();
  LogWriteEvent();
  LogWriteState();
  // record trace
  TraceUpdateTransition(event);
  return true;
}  


// set logic time
void LoggingExecutor::CurrentStep(int step) {
  FD_DX("LoggingExecutor(" << this << ")::CurrentStep(step)");
  // call base
  ParallelExecutor::CurrentStep(step);
  // clear trace
  TraceClear();
  mConditions.Reset();
}

// set physical time
void LoggingExecutor::CurrentTime(Time::Type time) {
  FD_DX("LoggingExecutor(" << this << ")::CurrentTime(time)");
  // call base
  ParallelExecutor::CurrentTime(time);
  // clear trace
  TraceClear();
  mConditions.Reset();
}

// set current state (clear trace)
bool LoggingExecutor::CurrentParallelTimedState(const ParallelTimedState& ptstate) {
  FD_DX("LoggingExecutor(" << this << ")::CurrentParallelTimedState(ptstate)");
  // call base
  bool res=ParallelExecutor::CurrentParallelTimedState(ptstate);
  // clear trace
  TraceClear();
  ConditionsReset();
  return res;
}



// get conditions
const SimConditionSet& LoggingExecutor::Conditions(void) const {
  return mConditions;
}

// set conditions
void LoggingExecutor::Conditions(const SimConditionSet& rConditions) {
    FD_DC("ProposingExecutor::DoRead(rTr,): loop X");
  mConditions=rConditions;
    FD_DC("ProposingExecutor::DoRead(rTr, ): loop Y");
  CompileConditions();
    FD_DC("ProposingExecutor::DoRead(rTr, loop Z");
}

// get condition
const AttributeSimCondition& LoggingExecutor::Condition(const std::string& rName) const {
  return mConditions.Attribute(mConditions.Index(rName));
}

// get condition
const AttributeSimCondition& LoggingExecutor::Condition(Idx cond) const {
  return mConditions.Attribute(cond);
}

/*
// get condition pointer
AttributeSimCondition* LoggingExecutor::Conditionp(const std::string& rName) {
  return mConditions.Attributep(mConditions.Index(rName));
}
*/

/*
// get condition pointer
AttributeSimCondition* LoggingExecutor::Conditionp(Idx cond) {
  return mConditions.Attributep(cond);
}
*/

// add/edit one condition
Idx LoggingExecutor::SetCondition(const std::string& rName, const AttributeSimCondition& rCondition) {
  Idx cond=mConditions.Insert(rName,rCondition);
  CompileConditions();
  return cond;
}

// edit one condition
void LoggingExecutor::SetCondition(Idx cond, const AttributeSimCondition& rCondition) {
  mConditions.Insert(cond,rCondition);
  CompileConditions();
}

// remove condition
void LoggingExecutor::ClrCondition(const std::string& rName) {
  mConditions.Erase(rName);  
  CompileConditions();
}

// remove condition
void LoggingExecutor::ClrCondition(Idx cond) {
  mConditions.Erase(cond);  
  CompileConditions();
}

// token io
void LoggingExecutor::ConditionsWrite(TokenWriter& rTw, const std::string& rLabel) const {
  mConditions.Write(rTw,rLabel);
}

// token io
void LoggingExecutor::ConditionsRead(TokenReader& rTr, const std::string& rLabel) {
  mConditions.Read(rTr,rLabel);
  CompileConditions();
}


// iterators
LoggingExecutor::ConditionIterator LoggingExecutor::ConditionsBegin(void) const {
  return mConditions.Begin();
}

// iterators
LoggingExecutor::ConditionIterator LoggingExecutor::ConditionsEnd(void) const {
  return mConditions.End();
}

// set up internal datastructure
void LoggingExecutor::CompileConditions(void) {
  FD_DX("LoggingExecutor::CompileConditions()");
  // need a lock to prevent realocation of attributes
  mConditions.Lock();
  // have a minmal copy to avoid pointless iterations
  mEnabledConditions.clear();
  ConditionIterator cit=mConditions.Begin();
  for(; cit != mConditions.End(); ++cit) {
    FD_DX("LoggingExecutor::CompileConditions(): compile " << mConditions.Str(*cit));
    AttributeSimCondition* pattr=mConditions.Attributep(*cit);
    mConditions.DValid();
    // reject if not enabled
    if(!pattr->Enabled()) continue; 
    // reject incompatible dims
    if(pattr->IsStateCondition()) 
    if(pattr->StateCondition().mStateSets.size()!=Size()) continue; 
    // have it
    mEnabledConditions.push_back(pattr); 
  }
  // reset all states (incl disabled)
  mConditions.Reset();
  mBreakCondition=false;
  FD_DX("LoggingExecutor(" << this << ")::CompileConditions(): #" << mEnabledConditions.size());
}

// logging: reset
void LoggingExecutor::ConditionsReset(void) {
  FD_DX("LoggingExecutor(" << this << ")::ConditionsReset()");
  CompileConditions();
  ConditionsProcess();
}

// logging: test
void LoggingExecutor::ConditionsProcess(void) {
  FD_DX("LoggingExecutor(" << this << ")::ConditionsProcess()");
  mBreakCondition=false;
  std::vector<AttributeSimCondition*>::iterator ait=mEnabledConditions.begin();
  // loop over all enabled conditions
  for(; ait != mEnabledConditions.end(); ++ait) {
    AttributeSimCondition* pattr= *ait;
    // case 1: event condition
    if(pattr->IsEventCondition()) {
      // positive edge
      if(!pattr->Satisfied())
      if(pattr->EventCondition().mStart.Exists(mRecentEvent)) {
        pattr->Satisfied(true,CurrentTime());
        if(pattr->Breakpoint()) mBreakCondition=true;
      }
      // negative edge
      if(pattr->Satisfied())
      if(pattr->EventCondition().mStop.Exists(mRecentEvent)) {
        pattr->Satisfied(false,CurrentTime());
      }
      continue;
    }
    // case 2: state condition
    if(pattr->IsStateCondition()) {
      // stage a: figure state
       bool satisfied;
       if(pattr->StateCondition().mAllFlag) {
         // conjunctive
         satisfied=true;
         for(Idx i=0; i<Size(); i++) {
	   Idx state = CurrentParallelState().at(i);
           const StateSet& set = pattr->StateCondition().mStateSets.at(i);
           if(set.Empty()) continue;
           if(!set.Exists(state)) {
             satisfied=false;
             break;
	   }
         }
       } else {
         // disjunctive
         satisfied=false;
         for(Idx i=0; i<Size(); i++) {
	   Idx state = CurrentParallelState().at(i);
           if(pattr->StateCondition().mStateSets.at(i).Exists(state)) {
             satisfied=true;
             break;
	   }
         }
       }
       // state b: figure edge  
       if(!pattr->Satisfied() && satisfied) {
        pattr->Satisfied(true,CurrentTime());
        if(pattr->Breakpoint()) mBreakCondition=true;
      }
      if(pattr->Satisfied() && !satisfied) {
        pattr->Satisfied(false,CurrentTime());
      }
      continue;       
    }
  } // loop conditions
}


//  logging io: start
void LoggingExecutor::LogOpen(TokenWriter& rTw, int logmode) {
  FD_DX("LoggingExecutor(" << this << ")::LogOpen()");
  pLogTokenWriter= &rTw; 
  mLogFile="";
  mLogMode=logmode;
  pLogTokenWriter->WriteBegin("ExecutionLog");
  pLogTokenWriter->WriteBegin("Mode");
  if(mLogMode & LogStatistics) pLogTokenWriter->WriteOption("Statistics");
  if(mLogMode & LogStates) pLogTokenWriter->WriteOption("States");
  if(mLogMode & LogEvents) pLogTokenWriter->WriteOption("Events");
  if(mLogMode & LogTime) pLogTokenWriter->WriteOption("Time");
  pLogTokenWriter->WriteEnd("Mode");
  LogWriteTime();
  LogWriteState();
}

//  logging io: start
void LoggingExecutor::LogOpen(const std::string& rFileName, int logmode, std::ios::openmode openmode) {
  FD_DX("LoggingExecutor(" << this << ")::LogOpen(" << rFileName << ")");
  pLogTokenWriter= new TokenWriter(rFileName,openmode);
  LogOpen(*pLogTokenWriter, logmode);
  mLogFile=rFileName;
}


//  logging io: stop
void LoggingExecutor::LogClose(void) {
  if(mLogMode & LogStatistics) {
    LogWriteStatistics();
  }
  if(mLogMode != 0) {
    FD_DX("LoggingExecutor(" << this << ")::LogClose(" << mLogFile << ")");
    *pLogTokenWriter << "\n";
    *pLogTokenWriter << "\n";
    pLogTokenWriter->WriteEnd("ExecutionLog");
  }
  if(mLogFile!="") {
    delete pLogTokenWriter;
  }
  mLogFile="";
  pLogTokenWriter=0;
  mLogMode=0;
}

// logging: report statistics
void LoggingExecutor::LogWriteStatistics(void) {
  if(!(mLogMode & LogStatistics)) return;
  FD_DX("LoggingExecutor(" << this << ")::LogWriteStatistics()");
  *pLogTokenWriter << "\n";
  *pLogTokenWriter << "\n";
  pLogTokenWriter->WriteBegin("Statistics");
  std::vector<AttributeSimCondition*>::iterator ait=mEnabledConditions.begin();
  for(; ait != mEnabledConditions.end(); ++ait) {
    AttributeSimCondition* pattr= *ait;
    pattr->mSamplesPeriod.Compile();
    pattr->mSamplesPeriod.Write(*pLogTokenWriter);
    pattr->mSamplesDuration.Compile();
    pattr->mSamplesDuration.Write(*pLogTokenWriter);
    *pLogTokenWriter << "\n";
  }
  pLogTokenWriter->WriteEnd("Statistics");
  *pLogTokenWriter << "\n";
  *pLogTokenWriter << "\n";
}

// logging: report state
void LoggingExecutor::LogWriteState(void) {
  if(!(mLogMode & LogStates)) return;
  if(mLogMode & LogTime) {
    CurrentParallelTimedState().Write(*pLogTokenWriter,"TimedState",this);
  } else
    CurrentParallelTimedState().Write(*pLogTokenWriter,"DiscreteState",this);
  *pLogTokenWriter << "\n";
}

// logging: report event
void LoggingExecutor::LogWriteEvent(void) {
  if(!(mLogMode & LogEvents)) return;
  if(!(mLogMode & LogStates)) {
    pLogTokenWriter->WriteString(Alphabet().SymbolicName(mRecentEvent));
    *pLogTokenWriter << "\n";
  } else {
    pLogTokenWriter->WriteBegin("Event");
    pLogTokenWriter->WriteString(Alphabet().SymbolicName(mRecentEvent));
    pLogTokenWriter->WriteEnd("Event");
  }
}

// loggging report time
void LoggingExecutor::LogWriteTime(void) {
  if(!(mLogMode & LogTime)) return;
  if(!(mLogMode & LogStates)) {
    pLogTokenWriter->WriteFloat(CurrentTime());
    *pLogTokenWriter << "\n";
  } else {
    pLogTokenWriter->WriteBegin("Time");
    pLogTokenWriter->WriteFloat(CurrentTime());
    pLogTokenWriter->WriteEnd("Time");
  }
}

// logging: pause
void LoggingExecutor::LogWritePause(void) {
  FD_DX("LoggingExecutor(" << this << ")::LogWritePause()");
  if(mLogMode == 0) return;
  LogWriteStatistics();
  *pLogTokenWriter << "\n";
  pLogTokenWriter->WriteEnd("ExecutionLog");
  *pLogTokenWriter << "\n";
  *pLogTokenWriter << "\n";
  *pLogTokenWriter << "\n";
}

// logging report pause
void LoggingExecutor::LogWriteResume(void) {
  FD_DX("LoggingExecutor(" << this << ")::LogWriteResume()");
  if(mLogMode == 0) return;
  pLogTokenWriter->WriteBegin("ExecutionLog");
  LogWriteState();
}

// trace: clear all
void LoggingExecutor::TraceClear(int length) {
  FD_DX("LoggingExecutor(" << this << ")::TraceClear(" << length <<")");
  // clear
  mTraceBuffer.clear();
  mTraceTimeToStep.clear();
  // set max length
  if(length>-2) mTraceMax=length;
  // set first step
  mTraceFirstStep=CurrentStep();
  // bail out
  if(mTraceMax==0) return;
  // set first entry
  TraceAddSample();
}

// trace: access
LoggingExecutor::TraceIterator LoggingExecutor::TraceBegin(void) const {
  return mTraceBuffer.begin();
}

// trace: access
LoggingExecutor::TraceIterator LoggingExecutor::TraceEnd(void) const {
  return mTraceBuffer.end();
}

// trace: access
const LoggingExecutor::TraceSample* LoggingExecutor::TraceAtStep(int step) const {
  int n = step-mTraceFirstStep;
  if(n<0) return 0;
  if(((unsigned int)n)>=mTraceBuffer.size()) return 0;
  return &mTraceBuffer[n];
}

// trace: access
const LoggingExecutor::TraceSample* LoggingExecutor::TraceAtTime(Time::Type time) const {
  std::map<Time::Type,int>::const_iterator sit=mTraceTimeToStep.find(time);
  if(sit== mTraceTimeToStep.end()) 
    return 0;
  int step = sit->second;
  return TraceAtStep(step);
}

// trace: access
const LoggingExecutor::TraceSample* LoggingExecutor::TraceCurrent(void) const {
  if(mTraceBuffer.size()==0) return 0;
  return  &mTraceBuffer.back(); 
}

// trace: access
const LoggingExecutor::TraceSample* LoggingExecutor::TraceRecent(void) const {
  if(mTraceBuffer.size()<2) return 0;
  return  &mTraceBuffer[mTraceBuffer.size()-2];
}

// trace: access
const LoggingExecutor::TraceSample* LoggingExecutor::TraceFirst(void) const {
  if(mTraceBuffer.size()<1) return 0;
  return  &mTraceBuffer[0];
}

// trace: access
int LoggingExecutor::TraceLength(void) const {
  return mTraceBuffer.size();
}

// trace: add empty sample
void LoggingExecutor::TraceAddSample(void) {
  // initialize new sample
  TraceSample sample;
  sample.mState=CurrentParallelTimedState();
  sample.mStep=CurrentStep();
  sample.mTime=CurrentTime(); 
  sample.mDuration=0;
  sample.mEvent=0;            
  // add to buffer
  mTraceBuffer.push_back(sample);
  // drop first sample
  if(mTraceMax>0) 
    while(mTraceBuffer.size()> (unsigned int) mTraceMax)
      mTraceBuffer.pop_front();
  // fix timing
  if(mTraceMax>0) 
    mTraceFirstStep=mTraceBuffer.front().mStep;
  // todo: time map
}

// trace:: update after transition
void LoggingExecutor::TraceUpdateTransition(Idx event) {
  // bail out
  if(mTraceMax==0) return;
  // fix last entry
  TraceSample& sample=mTraceBuffer.back();
  sample.mEvent=event;
  sample.mDuration=CurrentTime()-sample.mTime;
  // add entry
  TraceAddSample();
}

// trace:: update time
void LoggingExecutor::TraceUpdateTime(void) {
  // bail out
  if(mTraceMax==0) return;
  // fix last entry
  TraceSample& sample=mTraceBuffer.back();
  sample.mDuration=CurrentTime()-sample.mTime;
}



// trace: tokenwriter output
void LoggingExecutor::TraceWrite(TokenWriter& rTw, const TraceSample& sample) const {
  rTw.WriteBegin("Sample");
  rTw.WriteComment(" State " + PTSStr(sample.mState));
  rTw << sample.mStep;
  rTw << sample.mTime;
  rTw << sample.mDuration;
  rTw << "\n";
  if(sample.mEvent>0) 
    rTw << EventName(sample.mEvent);
  rTw.WriteEnd("Sample");
}

// trace: console output
void LoggingExecutor::TraceWrite(const TraceSample& sample) const {
  TokenWriter tw(TokenWriter::Stdout);
  TraceWrite(tw, sample);
}

// trace: console output (all)
void LoggingExecutor::TraceWrite() const {
  for(TraceIterator sit=TraceBegin(); sit!=TraceEnd(); sit++) {
    TraceWrite(*sit);
  };
}

// trace: string output
std::string LoggingExecutor::TraceToString(const TraceSample& sample) const {
  TokenWriter tw(TokenWriter::String);
  TraceWrite(tw, sample);
  return tw.Str();
}

// Revert executors state to past step from buffer
bool LoggingExecutor::RevertToStep(Idx step) {
  FD_DX("LoggingExecutor(" << this << ")::RevertToStep("<< step << ")");
  bool res=true;
  const TraceSample* samplep = TraceAtStep(step);
  if(!samplep) return false;
  FD_DX("LoggingExecutor(" << this << ")::RevertToStep("<< step << "): found step");
  // set state
  res=ParallelExecutor::CurrentParallelTimedState(samplep->mState);
  if(!res) return false;
  // care log trace
  LogWritePause();
  // set time
  ParallelExecutor::CurrentTime(samplep->mTime);
  ParallelExecutor::CurrentStep(samplep->mStep);
  LogWriteResume();
  // care trace: remove
  FD_DX("LoggingExecutor(" << this << ")::RevertToStep("<< step << "): fixing trace");
  while(mTraceBuffer.size()>0) {
    const TraceSample& lsample= mTraceBuffer.back();
    if(lsample.mStep<=step) break;
    mTraceBuffer.pop_back();
  }
  // care trace: invalidate last sample
  if(mTraceBuffer.size()>0) {
    TraceSample& lsample= mTraceBuffer.back();
    lsample.mEvent=0;
    lsample.mDuration=0;
  }
  // clear condition state
  mConditions.Reset();
  FD_DX("LoggingExecutor(" << this << ")::RevertToStep("<< step << "): done");
  return true;
}

} // namespace faudes


