/** @file hyb_attributes.cpp  Attributes of linear hybrid automata */

/* 
   Hybrid systems plug-in  for FAU Discrete Event Systems Library 

   Copyright (C) 2010  Thomas Moor

*/


#include "hyb_attributes.h"

namespace faudes {


/*******************************
 * 
 * Implementation of AttributeLhaTrans
 *
 */

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeLhaTrans,AttributeFlags)

// Assign my members
AttributeLhaTrans& AttributeLhaTrans::DoAssign(const AttributeLhaTrans& rSrcAttr) { 
  // call base (incl. virtual clear)
  AttributeFlags::DoAssign(rSrcAttr);
  // my additional members
  mGuard=rSrcAttr.mGuard;
  mReset=rSrcAttr.mReset;
  return *this;
}

// Equality
bool AttributeLhaTrans::DoEqual(const AttributeLhaTrans& rOther) const {
  // base
  if(!AttributeFlags::DoEqual(rOther)) return false;
  // my members
  if(mGuard!=rOther.mGuard) return false;
  if(mReset!=rOther.mReset) return false;
  // pass
  return true;
}


//DoWrite(rTw,rLabel,pContext);
void AttributeLhaTrans::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  DoXWrite(rTw,rLabel,pContext);
}

//DoXWrite(rTw,rLabel,pContext);
void AttributeLhaTrans::DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  AttributeFlags::DoXWrite(rTw,"",pContext);
  if(mGuard.Size()==0 && mReset.Identity()) return;
  if(rLabel!="") rTw.WriteBegin(rLabel);
  if(mGuard.Size()!=0) 
    mGuard.Write(rTw,"Guard");
  if(!mReset.Identity()) 
    mReset.Write(rTw,"Reset");
  if(rLabel!="") rTw.WriteEnd(rLabel);
}

//DoRead(rTr,rLabel,pContext)
void AttributeLhaTrans::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  // call base first
  AttributeFlags::DoRead(rTr,"",pContext);
  // initialise my data
  mGuard.Clear();
  mReset.Clear();
  // if section specified, read begin label
  if(rLabel!="") rTr.ReadBegin(rLabel);
  // search my data
  if(rTr.ExistsBegin("Guard")) 
    mGuard.Read(rTr,"Guard");
  // search my data
  if(rTr.ExistsBegin("Reset")) 
    mReset.Read(rTr,"Reset");
  // if section specified, read end label
  if(rLabel!="") rTr.ReadEnd(rLabel);
}


/*******************************
 * 
 * Implementation of AttributeLhaState
 *
 */

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeLhaState,AttributeFlags)

// Assign my members
AttributeLhaState& AttributeLhaState::DoAssign(const AttributeLhaState& rSrcAttr) { 
  // call base (incl. virtual clear)
  AttributeFlags::DoAssign(rSrcAttr);
  // my additional members
  mInvariant=rSrcAttr.mInvariant;
  mInitialConstraint=rSrcAttr.mInitialConstraint;
  mRate= rSrcAttr.mRate;
  return *this;
}

// Equality
bool AttributeLhaState::DoEqual(const AttributeLhaState& rOther) const {
  // base
  if(!AttributeFlags::DoEqual(rOther)) return false;
  // my members
  if(mInvariant!=rOther.mInvariant) return false;
  if(mInitialConstraint!=rOther.mInitialConstraint) return false;
  if(mRate!=rOther.mRate) return false;
  // pass
  return true;
}


//DoWrite(rTw,rLabel,pContext);
void AttributeLhaState::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  DoXWrite(rTw,rLabel,pContext);
}

//DoXWrite(rTw,rLabel,pContext);
void AttributeLhaState::DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  AttributeFlags::DoXWrite(rTw,"",pContext);
  if(rLabel!="") rTw.WriteBegin(rLabel);
  if(mInvariant.Size()!=0) 
    mInvariant.Write(rTw,"Invariant");
  if(mInitialConstraint.Size()!=0) 
    mInitialConstraint.Write(rTw,"InitialConstraint");
  mRate.Write(rTw,"Rate");
  if(rLabel!="") rTw.WriteEnd(rLabel);
}

//DoRead(rTr,rLabel,pContext)
void AttributeLhaState::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  // call base first
  AttributeFlags::DoRead(rTr,"",pContext);
  // clear my data
  mInvariant.Clear();
  mInitialConstraint.Clear();
  mRate.Clear();
  // if section specified, read begin label
  if(rLabel!="") rTr.ReadBegin(rLabel);
  // search my data
  if(rTr.ExistsBegin("Invariant")) {
     mInvariant.Read(rTr,"Invariant");
  }
  // search my data
  if(rTr.ExistsBegin("InitialConstraint")) {
    mInitialConstraint.Read(rTr,"InitialConstraint");
  } 
  // search my data
  if(rTr.ExistsBegin("Rate")) {
     mRate.Read(rTr,"Rate");
  } else {
    std::stringstream errstr;
    errstr << "invalid lha state [missing rate]" << rTr.FileLine();
    throw Exception("AttributeLhaState::Read", errstr.str(), 52);
  }
  // if section specified, read end label
  if(rLabel!="") rTr.ReadEnd(rLabel);
}


/*******************************
 * 
 * Implementation of AttributeLhaGlobal
 *
 */

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeLhaGlobal,AttributeVoid)

// Assign my members
AttributeLhaGlobal& AttributeLhaGlobal::DoAssign(const AttributeLhaGlobal& rSrcAttr) { 
  // call base (incl. virtual clear)
  AttributeVoid::DoAssign(rSrcAttr);
  // my additional members
  mStateSpace=rSrcAttr.mStateSpace;
  return *this;
}

// Equality
bool AttributeLhaGlobal::DoEqual(const AttributeLhaGlobal& rOther) const {
  // my members
  if(mStateSpace!=rOther.mStateSpace) return false;
  // pass
  return true;
}

//DoWrite(rTw,rLabel,pContext);
void AttributeLhaGlobal::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  if(IsDefault()) return;
  std::string label=rLabel;
  if(label=="") label="LhaStateSpace";
  FD_DC("AttributeLhaGlobal(" << this << ")::DoWrite(tr): to section " << label);
  mStateSpace.Write(rTw,label);
}

//DoRead(rTr,rLabel,pContext)
void AttributeLhaGlobal::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  std::string label=rLabel;
  if(label=="") label="LhaStateSpace";
  FD_DC("AttributeLhaGlobal(" << this << ")::DoRead(tr): from section " << label);
  (void) pContext;
  mStateSpace.Clear();
  Token token;
  rTr.Peek(token);
  if(token.Type()!=Token::Begin)  return;
  if(token.StringValue()!=label) return;
  mStateSpace.Read(rTr,label);
}






} // namespace faudes

