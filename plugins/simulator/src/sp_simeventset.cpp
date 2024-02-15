/** @file sp_simeventset.cpp Eventsets with execution data for simulation */


/* 
   Copyright (C) 2008  Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt
*/

#include "sp_simeventset.h"
 
namespace faudes {


// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,SimEventAttribute,AttributeCFlags)


// Assign my members
void SimEventAttribute::DoAssign(const SimEventAttribute& rSrcAttr) { 
  // call base (incl. virtual clear)
  AttributeCFlags::DoAssign(rSrcAttr);
  // my additional members
  mStochastic=rSrcAttr.mStochastic;
  mPriority=rSrcAttr.mPriority;
  mPriorityAttribute=rSrcAttr.mPriorityAttribute;
  mStochasticAttribute=rSrcAttr.mStochasticAttribute;
  // my dynamic state
  mScheduledFor=rSrcAttr.mScheduledFor;
  mExpiresAt=rSrcAttr.mExpiresAt;
  mDelayFor=rSrcAttr.mDelayFor;
  mReferenceInterval=rSrcAttr.mReferenceInterval;
}


// Test equality
bool SimEventAttribute::DoEqual(const SimEventAttribute& rOther) const { 
  // call base 
  if(!AttributeCFlags::DoEqual(rOther)) return false;
  // my additional members
  if(mStochastic!=rOther.mStochastic) return false;
  if(mPriority!=rOther.mPriority) return false;
  if(!(mPriorityAttribute==rOther.mPriorityAttribute)) return false;
  if(!(mStochasticAttribute==rOther.mStochasticAttribute)) return false;
  // pass
  return true;
}


//DoWrite(rTw);
void SimEventAttribute::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel;
  // do base first
  AttributeCFlags::DoWrite(rTw,"",pContext);
  FD_DC("SimEventAttribute(" << this << ")::DoWrite(tr)");
  Token token;
  // write priority
  if(mPriority) {
    rTw.WriteBegin("Priority");
    rTw.WriteInteger(mPriorityAttribute.mPriority);
    rTw.WriteEnd("Priority");
  }
  // write stochastic
  if(mStochastic) {
    rTw.WriteBegin("Stochastic");
    if(mStochasticAttribute.mType==SimStochasticEventAttribute::Extern) rTw.WriteOption("Extern");
    if(mStochasticAttribute.mType==SimStochasticEventAttribute::Trigger) rTw.WriteOption("Trigger");
    if(mStochasticAttribute.mType==SimStochasticEventAttribute::Delay) rTw.WriteOption("Delay");
    if(mStochasticAttribute.mPdf==SimStochasticEventAttribute::Exponential) rTw.WriteOption("Exponential");
    if(mStochasticAttribute.mPdf==SimStochasticEventAttribute::Gauss) rTw.WriteOption("Gauss");
    if(mStochasticAttribute.mPdf==SimStochasticEventAttribute::Uniform) rTw.WriteOption("Uniform");
    if(mStochasticAttribute.mParameter.size()>0) {
      rTw.WriteBegin("Parameter");
      std::vector<Float>::const_iterator pit=mStochasticAttribute.mParameter.begin();
      for(; pit!=mStochasticAttribute.mParameter.end(); pit++) {
	rTw.WriteFloat(*pit);
      }
      rTw.WriteEnd("Parameter");
    }
    rTw.WriteEnd("Stochastic");
  }
}

//DoRead(rTr)
void SimEventAttribute::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel;
  // call base first
  AttributeCFlags::DoRead(rTr,"",pContext);

  // report
  FD_DC("SimEventAttribute(" << this << ")::DoRead(tr)");

  // clear myself
  mStochastic=false;
  mPriority=false;

  // read as long as we can handle
  Token token;
  bool err=false;
  while(rTr.Peek(token)) {

    // try priority
    if(token.IsBegin() && token.StringValue()=="Priority") {
      rTr.ReadBegin("Priority");
      mPriority=true;
      rTr.Get(token);
      if(!token.IsFloat()) {
        err=true; 
      } else {
        mPriorityAttribute.mPriority=(int) token.FloatValue();
      }
      rTr.ReadEnd("Priority");
      continue;
    }

    // try stochastic
    if(token.IsBegin() && token.StringValue()=="Stochastic") {
      rTr.ReadBegin("Stochastic");
      // stochastic default
      mStochastic=true;
      mStochasticAttribute.mType = SimStochasticEventAttribute::Extern;
      mStochasticAttribute.mPdf = SimStochasticEventAttribute::Exponential;
      mStochasticAttribute.mParameter.clear();
      mStochasticAttribute.mParameter.push_back(1);
      // mandatory option: intern/extern/delay
      rTr.Get(token);
      if(!token.IsOption()) {
	err=true; 
      } else {
        if(token.OptionValue() == "Extern") 
	  {mStochasticAttribute.mType = SimStochasticEventAttribute::Extern; };
        if(token.OptionValue() == "Trigger") 
	  {mStochasticAttribute.mType = SimStochasticEventAttribute::Trigger; };
        if(token.OptionValue() == "Delay") 
	  {mStochasticAttribute.mType = SimStochasticEventAttribute::Delay; };
      } 
      // mandatory option: exponential/gauss/ etc
      rTr.Get(token);
      if(!token.IsOption()) {
	err=true; 
      } else {
        if(token.OptionValue() == "Exponential") 
  	  {mStochasticAttribute.mPdf = SimStochasticEventAttribute::Exponential;};
        if(token.OptionValue() == "Gauss") 
	  {mStochasticAttribute.mPdf = SimStochasticEventAttribute::Gauss;};
        if(token.OptionValue() == "Uniform") 
  	  {mStochasticAttribute.mPdf = SimStochasticEventAttribute::Uniform;};
      }  
      // optional data: parameter vector
      rTr.Peek(token);
      if(token.StringValue()=="Parameter" && token.IsBegin()) {
        mStochasticAttribute.mParameter.clear();
        rTr.ReadBegin("Parameter");
        while (!rTr.Eos("Parameter")) {
          Float par = rTr.ReadFloat();
          mStochasticAttribute.mParameter.push_back(par);      
        }
        rTr.ReadEnd("Parameter");
      }
      // old file format: fix missing restricted support pdfs
      if(mStochasticAttribute.mPdf == SimStochasticEventAttribute::Exponential 
         && mStochasticAttribute.mParameter.size() == 1) {
        Float lambda= mStochasticAttribute.mParameter.at(0);
        mStochasticAttribute.mParameter.clear();
	mStochasticAttribute.mParameter.push_back(0);
	mStochasticAttribute.mParameter.push_back(std::numeric_limits<Float>::max());
	mStochasticAttribute.mParameter.push_back(lambda);
      }
      if(mStochasticAttribute.mPdf == SimStochasticEventAttribute::Gauss 
         && mStochasticAttribute.mParameter.size() == 2) {
        Float mue= mStochasticAttribute.mParameter.at(0);
        Float sigma= mStochasticAttribute.mParameter.at(1);
        mStochasticAttribute.mParameter.clear();
	mStochasticAttribute.mParameter.push_back(0);
	mStochasticAttribute.mParameter.push_back(std::numeric_limits<Float>::max());
	mStochasticAttribute.mParameter.push_back(mue);
	mStochasticAttribute.mParameter.push_back(sigma);
      }
      // check parameter vector consistency
      if(mStochasticAttribute.mPdf == SimStochasticEventAttribute::Exponential 
         && mStochasticAttribute.mParameter.size() != 3) 
        err=true;
      if(mStochasticAttribute.mPdf == SimStochasticEventAttribute::Gauss 
         && mStochasticAttribute.mParameter.size() != 4) 
        err=true;
      if(mStochasticAttribute.mPdf == SimStochasticEventAttribute::Uniform 
         && mStochasticAttribute.mParameter.size() != 2) 
        err=true;
      rTr.ReadEnd("Stochastic");
    }

    // report error
    if(err) {
      std::stringstream errstr;
      errstr << "invalid simulation event property" << rTr.FileLine();
      throw Exception("SimEventAttribute::Read", errstr.str(), 52);
    }

    // cannot digest? -> done
    break;
    
  }
}


// debug str
std::string SimEventAttribute::Str(void) const {
  std::stringstream retstr;
  //retstr << "[" << this << "] ";
  if(IsPriority()) {
    retstr<< "priority=" << Priority().mPriority << " ";
  }
  if(IsStochastic()) {
    retstr<< "stochastic=" << SimStochasticEventAttribute::TStr(Stochastic().mType) << " " ;
    retstr<< "scheduled_for=" << Time::Str(mScheduledFor) << " ";
    retstr<< " expires_at=" << Time::Str(mExpiresAt) << " ";
    retstr<< " delay_for=" << Time::Str(mDelayFor) << " ";
  }
  return retstr.str();
}



} // namespace faudes

