/** @file sp_simconditionset.cpp  Set of named simulation conditions */


/* 
   Copyright (C) 2008  Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt
*/

#include "sp_simconditionset.h"
#include "sp_lpexecutor.h"

namespace faudes {

/*
************************************************
************************************************
************************************************

implementation of attributes

************************************************
************************************************
************************************************
*/




// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeSimCondition,AttributeFlags)


// construct
AttributeSimCondition::AttributeSimCondition(void) : 
  AttributeFlags(),
  mEventCondition(false), 
  mStateCondition(false)
{ 
  FD_DX("AttributeSimCondition::AttributeSimCondition()");
  mFlags=mDefSCFlags;
  Reset();
}

// construct
AttributeSimCondition::AttributeSimCondition(const AttributeSimCondition& rOther) : 
  AttributeFlags(),
  mEventCondition(false), 
  mStateCondition(false) 
{ 
  FD_DX("AttributeSimCondition::AttributeSimCondition(attr)");
  DoAssign(rOther);
}

// Assign my members
void AttributeSimCondition::DoAssign(const AttributeSimCondition& rSrcAttr) { 
  // call base (incl. virtual clear)
  AttributeFlags::DoAssign(rSrcAttr);
  // my additional members
  mEventCondition=rSrcAttr.mEventCondition;
  mStateCondition=rSrcAttr.mStateCondition;
  mEventConditionAttribute=rSrcAttr.mEventConditionAttribute;
  mStateConditionAttribute=rSrcAttr.mStateConditionAttribute;
  // my state
  mSatisfied=false;
  mActivationTime=0;
  mSamplesPeriod.Clear();
  mSamplesDuration.Clear();
}


// Test for equality
bool AttributeSimCondition::DoEqual(const AttributeSimCondition& rAttr) const { 
  // call base (incl. virtual clear)
  if(!AttributeFlags::DoEqual(rAttr)) return false;
  // test my defining members
  if(mEventCondition!=rAttr.mEventCondition) return false;
  if(mStateCondition!=rAttr.mStateCondition) return false;
  if(!(mEventConditionAttribute==rAttr.mEventConditionAttribute)) return false;
  if(!(mStateConditionAttribute==rAttr.mStateConditionAttribute)) return false;
  // pass
  return true;
}


//DoWrite(rTw);
void AttributeSimCondition::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel;
  Token token;

  // report context
  FD_DC("AttributeSimCondition::DoWrite(rTr," << rLabel <<", " << pContext << "): cast " <<
     dynamic_cast<const ParallelExecutor*>(pContext));

  // write event condition
  if(mEventCondition) {
    rTw.WriteBegin("EventCondition");
    if(Enabled()) rTw.WriteOption("Enabled");
    if(!Enabled()) rTw.WriteOption("Disabled");
    if(Breakpoint()) rTw.WriteOption("Break");
    mEventConditionAttribute.mStart.Write(rTw,"StartEvents");
    mEventConditionAttribute.mStop.Write(rTw,"StopEvents");
    rTw.WriteEnd("EventCondition");
  }

  // write state condition
  if(mStateCondition) {
    rTw.WriteBegin("StateCondition");
    if(Enabled()) rTw.WriteOption("Enabled");
    if(!Enabled()) rTw.WriteOption("Disabled");
    if(Breakpoint()) rTw.WriteOption("Break");
    if(mStateConditionAttribute.mAllFlag) rTw.WriteOption("Conjunction");
    if(!mStateConditionAttribute.mAllFlag) rTw.WriteOption("Disjunction");
    // get context
    const ParallelExecutor* executor= dynamic_cast<const ParallelExecutor*>(pContext);
    Idx n=mStateConditionAttribute.mStateSets.size();
    // test context
    if(executor)
      if(executor->Size()!=n) {
        std::stringstream errstr;
        errstr << "Invalid ParalleExecutor context (dimension mismatch)";
        throw Exception("AttributeSimCondition::DoWrite", errstr.str(), 501);
      }
    // write the sets
    for(Idx i=0; i<n; i++) {
      const StateSet& set=mStateConditionAttribute.mStateSets.at(i);
      if(executor) executor->At(i).Generator().WriteStateSet(rTw,set);
      else set.Write(rTw,"StateSet");
    }
    rTw.WriteEnd("StateCondition");
  }
}

//DoRead(rTr)
void AttributeSimCondition::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel;
  bool err;
  Token token;

  // report context
  FD_DC("AttributeSimCondition::DoRead(rTr," << rLabel <<", " << pContext << "): cast " <<
     dynamic_cast<const ParallelExecutor*>(pContext));

  // clear myself
  err=false;
  mEventCondition=false;
  mStateCondition=false;
  Enabled(true);
  Breakpoint(false);
  mEventConditionAttribute.mStart.Clear();
  mEventConditionAttribute.mStop.Clear();
  mStateConditionAttribute.mStateSets.clear();
  mStateConditionAttribute.mAllFlag=false;

  // switch by toke
  rTr.Peek(token);

  // read event condition
  if(token.Type()==Token::Begin && token.StringValue()=="EventCondition") {
    rTr.ReadBegin("EventCondition");
    mEventCondition=true;
    while(!rTr.Eos("EventCondition")) {
      rTr.Peek(token);
      if((token.IsOption()) && (token.OptionValue()=="Enabled")) {
	Enabled(true);
	rTr.Get(token);
        continue;
      }
      if((token.IsOption()) && (token.OptionValue()=="Disabled")) {
	Enabled(false);
	rTr.Get(token);
        continue;
      }
      if((token.IsOption()) && (token.OptionValue()=="Break")) {
	Breakpoint(true);
	rTr.Get(token);
        continue;
      }
      if((token.IsBegin()) && (token.StringValue()=="StartEvents")) {
        mEventConditionAttribute.mStart.Read(rTr,"StartEvents");
        continue;
      }
      if((token.IsBegin()) && (token.StringValue()=="StopEvents")) {
        mEventConditionAttribute.mStop.Read(rTr,"StopEvents");
        continue;
      }
      // ignore errors
      rTr.Get(token);
    }
    rTr.ReadEnd("EventCondition");
  }

  // read state condition
  if(token.Type()==Token::Begin && token.StringValue()=="StateCondition") {
    rTr.ReadBegin("StateCondition");
    mStateCondition=true;
    while(!rTr.Eos("StateCondition")) {
      rTr.Peek(token);
      if((token.IsOption()) && (token.OptionValue()=="Enabled")) {
	Enabled(true);
	rTr.Get(token);
        continue;
      }
      if((token.IsOption()) && (token.OptionValue()=="Disabled")) {
	Enabled(false);
	rTr.Get(token);
        continue;
      }
      if((token.IsOption()) && (token.OptionValue()=="Break")) {
	Breakpoint(true);
	rTr.Get(token);
        continue;
      }
      if((token.IsOption()) && (token.OptionValue()=="Conjunction")) {
	mStateConditionAttribute.mAllFlag=true;
	rTr.Get(token);
        continue;
      }
      if((token.IsBegin()) && (token.StringValue()=="StateSet")) {
        // get context
        const ParallelExecutor* executor= dynamic_cast<const ParallelExecutor*>(pContext);
        Idx i=mStateConditionAttribute.mStateSets.size();
        // test context
        if(executor)
          if(executor->Size()<=i) {
            std::stringstream errstr;
            errstr << "Invalid ParalleExecutor context (dimension mismatch)";
            throw Exception("AttributeSimCondition::DoRead", errstr.str(), 501);
          }
        // read the set
        StateSet set;
        if(executor) executor->At(i).Generator().ReadStateSet(rTr,"StateSet",set);
        else set.Read(rTr,"StateSet");
        set.Name("StateSet");
        mStateConditionAttribute.mStateSets.push_back(set);
        continue;
      }
      // ignore errors
      rTr.Get(token);
    }
    rTr.ReadEnd("StateCondition");
  }
  // report error
  if(err) {
    std::stringstream errstr;
    errstr << "invalid simulation condition" << rTr.FileLine();
    throw Exception("AttributeSimCondition::Read", errstr.str(), 52);
  }
}

// condition state: sample
void AttributeSimCondition::Satisfied(bool on, Time::Type now) {
  if(on==mSatisfied) return;
  mSatisfied=on;
  // newly activated
  if(mSatisfied) {
    if(mActivationTime>0) mSamplesPeriod.Sample(now-mActivationTime); 
    mActivationTime=now;
  }
  // newly deactivated
  if(!mSatisfied) {
    mSamplesDuration.Sample(now-mActivationTime); 
  }
}


/*
************************************************
************************************************
************************************************

implementation of condition set

************************************************
************************************************
************************************************
*/

// std faudes type (except new)
FAUDES_TYPE_IMPLEMENTATION_COPY(Void,SimConditionSet,TaNameSet<AttributeSimCondition>)
FAUDES_TYPE_IMPLEMENTATION_CAST(Void,SimConditionSet,TaNameSet<AttributeSimCondition>)
FAUDES_TYPE_IMPLEMENTATION_ASSIGN(Void,SimConditionSet,TaNameSet<AttributeSimCondition>)
FAUDES_TYPE_IMPLEMENTATION_EQUAL(Void,SimConditionSet,TaNameSet<AttributeSimCondition>)


// Default constructor 
SimConditionSet::SimConditionSet(void) : TaNameSet<AttributeSimCondition>() {
  FD_DX("SimConditionSet::SimConditionSet()");
}

// Copy constructor 
SimConditionSet::SimConditionSet(const SimConditionSet& rOtherSet) : TaNameSet<AttributeSimCondition>(rOtherSet) {
  FD_DX("SimConditionSet::SimConditionSet()");
}


// std faudes new
SimConditionSet* SimConditionSet::New(void) const {
  SimConditionSet* res= new SimConditionSet();
  res->mpSymbolTable=mpSymbolTable;
  return res;
}

// get enabled set
SimConditionSet SimConditionSet::EnabledConditions(void) {
  SimConditionSet result;
  for(Iterator cit=Begin(); cit != End(); ++cit) 
    if(Enabled(*cit)) result.Insert(*cit,Attribute(*cit));
  return result;
}

// condition state: reset alls
void SimConditionSet::Reset(void) {
  FD_DX("SimConditionSet::Reset()");
  for(Iterator cit=Begin(); cit != End(); ++cit) {
    AttributeSimCondition* pattr=Attributep(*cit);
    pattr->Reset();
    pattr->mSamplesPeriod.Name(SymbolicName(*cit)+" - Period");
    pattr->mSamplesDuration.Name(SymbolicName(*cit)+" - Duration");
  }
  FD_DX("SimConditionSet::Reset(): done");
}



} // namespace faudes

