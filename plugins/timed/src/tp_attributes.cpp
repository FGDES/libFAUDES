/** @file tp_attributes.cpp  Attributes for timed automata */


/* Timeplugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2007  Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

*/

#include "tp_attributes.h"

namespace faudes {


/*******************************
 * 
 * Implementation of AttributeTimedTrans
 *
 */

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeTimedTrans,AttributeFlags)

// Assign my members
void AttributeTimedTrans::DoAssign(const AttributeTimedTrans& rSrcAttr) { 
  // call base (incl. virtual clear)
  AttributeFlags::DoAssign(rSrcAttr);
  // my additional members
  mGuard=rSrcAttr.mGuard;
  mResets=rSrcAttr.mResets;
}

// Equality
bool AttributeTimedTrans::DoEqual(const AttributeTimedTrans& rOther) const {
  // base
  if(!AttributeFlags::DoEqual(rOther)) return false;
  // my members
  if(mGuard!=rOther.mGuard) return false;
  if(mResets!=rOther.mResets) return false;
  // pass
  return true;
}


//DoWrite(rTw,rLabel,pContext);
void AttributeTimedTrans::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  AttributeFlags::DoWrite(rTw,"",pContext);
  if(mGuard.Empty() && mResets.Empty()) return; 
  std::string label=rLabel;
  FD_DC("AttributeTimedTrans(" << this << ")::DoWrite(tr): to section " << label);
  if(label=="") label="Timing";
  rTw.WriteBegin(label);
  if(!mGuard.Empty()) 
     mGuard.Write(rTw);
  if(!mResets.Empty()) 
    mResets.Write(rTw);
  rTw.WriteEnd(label);
}


//DoRead(rTr,rLabel,pContext)
void AttributeTimedTrans::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  // call base first
  AttributeFlags::DoRead(rTr,"",pContext);
  // find my section
  std::string label=rLabel;
  if(label=="") label="Timing";
  FD_DC("AttributeTimedTrans(" << this << ")::DoRead(tr): from section " << label);
  // initialise my data
  mGuard.Clear();
  mResets.Clear();
  // test for  my data
  Token token;
  rTr.Peek(token);
  if(!token.IsBegin()) return;
  if(token.StringValue()!=label) return;
  // test read  my data (could throw exceptions now)
  rTr.ReadBegin(label);
  while (rTr.Peek(token)) {
    // 0: looking for "begin" only
    if (token.Type() != Token::Begin) break;
    // 1: guard
    if (token.StringValue() == "Guard") {
       mGuard.Read(rTr,"Guard");
       continue;
    }
    // 2: resets
    if (token.StringValue() == "Resets") {
       mResets.Read(rTr,"Resets");
       continue;
    }
    // 3:
    std::stringstream errstr;
    errstr << "invalid transition timing" << rTr.FileLine();
    throw Exception("AttributeTimedTrans::Read", errstr.str(), 52);
  }
  mGuard.Name("Guard");
  mResets.Name("Resets");
  rTr.ReadEnd(label);
}


/*******************************
 * 
 * Implementation of AttributeTimedState
 *
 */

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeTimedState,AttributeFlags)

// Assign my members
void AttributeTimedState::DoAssign(const AttributeTimedState& rSrcAttr) { 
  // call base (incl. virtual clear)
  AttributeFlags::DoAssign(rSrcAttr);
  // my additional members
  mInvariant=rSrcAttr.mInvariant;
}

// Equality
bool AttributeTimedState::DoEqual(const AttributeTimedState& rOther) const {
  // base
  if(!AttributeFlags::DoEqual(rOther)) return false;
  // my members
  if(mInvariant!=rOther.mInvariant) return false;
  // pass
  return true;
}


//DoWrite(rTw,rLabel,pContext);
void AttributeTimedState::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  if(IsDefault()) return;
  AttributeFlags::DoWrite(rTw,"",pContext);
  std::string label=rLabel;
  if(label=="") label="Invariant";
  FD_DC("AttributeTimedState(" << this << ")::DoWrite(tr): to section " << label);
  if(!mInvariant.Empty())
    mInvariant.Write(rTw,label);
}


//DoRead(rTr,rLabel,pContext)
void AttributeTimedState::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  // call base first
  AttributeFlags::DoRead(rTr,"",pContext);
  // figure my section
  std::string label=rLabel;
  if(label=="") label="Invariant";
  FD_DC("AttributeTimedState(" << this << ")::DoRead(tr): from section " << label);
  // clear my data
  mInvariant.Clear();
  // test my section
  Token token;
  rTr.Peek(token);
  if(!token.IsBegin())  return;
  if(token.StringValue()!=label) return;
  // read my section (can throw exceptions now)
  mInvariant.Read(rTr,label);
}


/*******************************
 * 
 * Implementation of AttributeTimedGlobal
 *
 */

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeTimedGlobal,AttributeVoid)

// Assign my members
void AttributeTimedGlobal::DoAssign(const AttributeTimedGlobal& rSrcAttr) { 
  // call base (incl. virtual clear)
  AttributeVoid::DoAssign(rSrcAttr);
  // my additional members
  mClocks=rSrcAttr.mClocks;
  mpClockSymbolTable=rSrcAttr.mpClockSymbolTable;
}

// Equality
bool AttributeTimedGlobal::DoEqual(const AttributeTimedGlobal& rOther) const {
  // my members
  if(mClocks!=rOther.mClocks) return false;
  // pass
  return true;
}

//DoWrite(rTw,rLabel,pContext);
void AttributeTimedGlobal::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  if(IsDefault()) return;
  std::string label=rLabel;
  if(label=="") label="Clocks";
  FD_DC("AttributeTimedGlobal(" << this << ")::DoWrite(tr): to section " << label);
  mClocks.Write(rTw,label);
}

//DoRead(rTr,rLabel,pContext)
void AttributeTimedGlobal::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  std::string label=rLabel;
  if(label=="") label="Clocks";
  FD_DC("AttributeTimedGlobal(" << this << ")::DoRead(tr): from section " << label);
  (void) pContext;
  mClocks.Clear();
  Token token;
  rTr.Peek(token);
  if(!token.IsBegin())  return;
  if(token.StringValue()!=label) return;
  mClocks.Read(rTr,label);
}






} // namespace faudes

