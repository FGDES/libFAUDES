/** @file omg_rabinacc.cpp Rabin acceptance condition */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2025 Thomas Moor
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


      
#include "omg_rabinacc.h" 

namespace faudes {

// local debugging
//#undef FD_DC
//#define FD_DC(m) FD_WARN(m)
  
/*
********************************
Autoregister 
********************************
*/

AutoRegisterType<RabinPair> gRtiRabinPair("RabinPair");
AutoRegisterType<RabinAcceptance> gRtiRabinAcceptance("RabinAcceptance");
AutoRegisterElementType<RabinAcceptance> gRtiRabinAcceptanceEType("RabinAcceptance","RabinPair");
AutoRegisterElementTag<RabinAcceptance> gRtiRabinAcceptanceETag("RabinAcceptance","RabinPair");
  
/*
********************************
implementation RabinPair
********************************
*/

// faudes Type std
FAUDES_TYPE_IMPLEMENTATION(Void,RabinPair,ExtType)

// Construct
RabinPair::RabinPair(void) : ExtType() {
  FD_DC("RabinPair(" << this << ")::RabinPair()");
  Name("RabinPair");
  mRSet.Name("R");
  mISet.Name("I");
}

// Copy construct
RabinPair::RabinPair(const RabinPair& rRP) : ExtType()  {
  FD_DC("RabinPair(" << this << ")::RabinPair(other): " << rRP.Name());
  Name("RabinPair");
  mRSet.Name("R");
  mISet.Name("I");
  DoAssign(rRP);
}

// Assign my members
void RabinPair::DoAssign(const RabinPair& rSrc) {
  mRSet=rSrc.mRSet;
  mISet=rSrc.mISet;
}

// Clear configuration
void RabinPair::Clear(void) {
  mRSet.Clear();
  mISet.Clear();
}

// Ordering  
bool RabinPair::operator<(const RabinPair& rOther) const {
  if(mRSet<rOther.mRSet) return true;
  if(rOther.mRSet<mRSet) return false;
  return mISet<rOther.mISet;
}
  
// Test equality
bool RabinPair::DoEqual(const RabinPair& rOther) const {
  if(mRSet != rOther.mRSet) return false;
  if(mISet != rOther.mISet) return false;
  return true;
}

// Write 
void RabinPair::DoWrite(TokenWriter& rTw,const std::string& rLabel, const Type* pContext) const {
  FD_DC("RabinPair(" << this << ")::DoWrite(tw)");
  // insist in label
  std::string label="RabinPair";
  // set up outer tag with name
  Token btag;
  btag.SetBegin(label);
  btag.InsAttributeString("name",Name());
  // begin tag
  rTw.Write(btag);
  // use context if its a generator (todo:xml)
  const vGenerator* vg = dynamic_cast<const vGenerator*>(pContext);
  if(vg!=nullptr) {
    FD_DC("RabinPair(" << this << ")::DoWrite(tw): using context");
    vg->WriteStateSet(rTw, mRSet,"R");
    vg->WriteStateSet(rTw, mISet, "I");
  } else {
    FD_DC("RabinPair(" << this << ")::DoWrite(tw): no context");
    mRSet.Write(rTw, "R", pContext);
    mISet.Write(rTw, "I", pContext);
  }
  // close tag
  rTw.WriteEnd(label);
}


// Write XML
void RabinPair::DoXWrite(TokenWriter& rTw,const std::string& rLabel, const Type* pContext) const{
  FD_DC("RabinPair(" << this << ")::DoXWrite(tw)");
  // default label
  std::string label=rLabel;
  if(label=="") label="RabinPair";
  // set up outer tag
  Token btag=XBeginTag(label,"RabinPair");
  rTw.Write(btag);
  // use context if its a generator (todo:xml)
  const vGenerator* vg = dynamic_cast<const vGenerator*>(pContext);
  if(vg!=nullptr) {
    FD_DC("RabinPair(" << this << ")::DoXWrite(tw): using context");
    vg->XWriteStateSet(rTw, mRSet, "R");
    vg->XWriteStateSet(rTw, mISet, "I");
  } else {
    FD_DC("RabinPair(" << this << ")::DoXWrite(tw): no context");
    mRSet.XWrite(rTw, "R", pContext);
    mISet.XWrite(rTw, "I", pContext);
  }
  // close tag
  rTw.WriteEnd(btag.StringValue());
}

//DoRead(rTr)
void RabinPair::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  FD_DC("RabinPair(" << this << ")::DoRead(tr)");
  // default label
  std::string label=rLabel;
  if(label=="") label="RabinPair";
  // read begin
  Token btag;
  rTr.ReadBegin(label,btag);
  FD_DC("RabinPair(" << this << ")::DoRead(tr): found " << btag.Str()); 
  // figure name from begin tag
  Name("");
  if(btag.ExistsAttributeString("name"))
    Name(btag.AttributeStringValue("name"));
  FD_DC("RabinPair(" << this << ")::DoRead(tr): object name " << Name()); 
  // use context if its a generator (todo:xml)
  const vGenerator* vg = dynamic_cast<const vGenerator*>(pContext);
  if(vg!=nullptr) {
    vg->ReadStateSet(rTr, "R", mRSet);
    vg->ReadStateSet(rTr, "I", mISet);
  } else {
    mRSet.Read(rTr,"R",pContext);
    mISet.Read(rTr,"I",pContext);
  }
  // end tag
  rTr.ReadEnd(label);
}

  
// restrict domain
void RabinPair::RestrictStates(const StateSet& rDomain) {
  mRSet.RestrictSet(rDomain);
  mISet.RestrictSet(rDomain);
}


/*
********************************
implementation RabinAcceptance
********************************
*/

// faudes Type std
FAUDES_TYPE_IMPLEMENTATION(Void,RabinAcceptance,TBaseVector<RabinPair>)

// Consttruct
RabinAcceptance::RabinAcceptance(void) : TBaseVector<RabinPair>()  {
  FD_DC("RabinAcceptance(" << this << ")::RabinAcceptance()");
  Name("RabinAcceptance");
}
  
// Copy construct
RabinAcceptance::RabinAcceptance(const RabinAcceptance& rRA) : TBaseVector<RabinPair>() {
  FD_DC("RabinAcceptance(" << this << ")::RabinAcceptance(other): " << rRA.Name());
  Name("RabinAcceptance");
  DoAssign(rRA);
}

// Condtruct from file
RabinAcceptance::RabinAcceptance(const std::string& rFileName) : TBaseVector<RabinPair>() {
  Name("RabinAcceptance");
  Read(rFileName);
}

  

// DoSWrite()
void RabinAcceptance::DoSWrite(TokenWriter& rTw) const {
  //TBaseVector::DoSWrite(rTw); // also use base?
  Type::DoSWrite(rTw);
  rTw.WriteComment(" RabinPairs: " + ToStringInteger(Size()));
  CIterator rit;
  for(rit=Begin();rit!=End();++rit)
    rTw.WriteComment(" R/I-States: " + ToStringInteger(rit->RSet().Size()) +
       "/" + ToStringInteger(rit->ISet().Size()));
  rTw.WriteComment(" ");
}
  
// restrict domain
void RabinAcceptance::RestrictStates(const StateSet& rDomain) {
  Iterator rit;
  for(rit=Begin();rit!=End();++rit)
    rit->RestrictStates(rDomain);
}


}// namespace


