/** @file omg_rabinacc.cpp Rabin acceptance condition */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2025 Yiheng Tang, Thomas Moor
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
#undef FD_DC
#define FD_DC(m) FD_WARN(m)
  
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
FAUDES_TYPE_IMPLEMENTATION(Void,RabinPair,Type)

// Construct
RabinPair::RabinPair(void) {
  mName="RabinPair";
  mRSet.Name("R");
  mISet.Name("I");
}

// Copy construct
RabinPair::RabinPair(const RabinPair& rRP) {
  mName="RabinPair";
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
  if(mRSet!=rOther.mRSet) return false;
  if(mRSet<rOther.mRSet) return false;
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
  FD_DC("RabinPair(" << this << ")::DoXWrite(tw)");
  // insist in label
  std::string label="RabinPair";
  // begin tag
  rTw.WriteBegin(label);
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
  // default label
  std::string label=rLabel;
  if(label=="") label="RabinPair";
  // figure name from begin tag
  Name("");
  Token btag;
  rTr.Peek(btag);
  if(btag.ExistsAttributeString("name"))
    Name(btag.AttributeStringValue("name"));
  // read begin
  rTr.ReadBegin(label);
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



/*
********************************
implementation RabinAcceptance
********************************
*/

// faudes Type std
FAUDES_TYPE_IMPLEMENTATION(Void,RabinAcceptance,TBaseSet<RabinPair>)

// Consttruct
RabinAcceptance::RabinAcceptance(void) : TBaseSet<RabinPair>()  {
  Name("RabinAcceptance");
}
  
// Copy construct
RabinAcceptance::RabinAcceptance(const RabinAcceptance& rRA) : TBaseSet<RabinPair>() {
  Name("RabinAcceptance");
  DoAssign(rRA);
}

// Condtruct from file
RabinAcceptance::RabinAcceptance(const std::string& rFileName) : TBaseSet<RabinPair>() {
  Name("RabinAcceptance");
  Read(rFileName);
}

// DoWriteElement(tw,cpntext)
void RabinAcceptance::DoWriteElement(TokenWriter& rTw,const RabinPair& rElem, const std::string& rLabel, const Type* pContext) const {
  rElem.Write(rTw,"RabinPair",pContext);
}

// DoXWriteElement(tw,cpntext)
void RabinAcceptance::DoXWriteElement(TokenWriter& rTw,const RabinPair& rElem, const std::string& rLabel, const Type* pContext) const {
  rElem.XWrite(rTw,"RabinPair",pContext);
}

// DoInsertElement(rTr, rLabel, pContext)
void RabinAcceptance::DoInsertElement(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  RabinPair rpair;
  rpair.Read(rTr, rLabel, pContext);
  Insert(rpair); ///hmm ... insert should return an Iterator
}

// DoSWrite()
void RabinAcceptance::DoSWrite(TokenWriter& rTw) const {
  //TBaseSet::DoSWrite(rTw);
  Type::DoSWrite(rTw);
  rTw.WriteComment(" RabinPairs: " + ToStringInteger(Size()));
  Iterator rit;
  for(rit=Begin();rit!=End();++rit)
    rTw.WriteComment(" R/I-States: " + ToStringInteger(rit->RSet().Size()) +
       "/" + ToStringInteger(rit->ISet().Size()));
  rTw.WriteComment(" ");
}
  



}// namespace


