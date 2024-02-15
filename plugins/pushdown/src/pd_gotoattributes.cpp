/** @file pd_gotoattributes.cpp  Attributes for LR machines*/


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_gotoattributes.h"

namespace faudes {
  
  
  /*******************************
  * 
  * Implementation of AttributeGotoTransition
  *
  */

  // faudes type std
  FAUDES_TYPE_IMPLEMENTATION(Void,AttributeGotoTransition,AttributeVoid)

  // Assign my members
  void AttributeGotoTransition::DoAssign(const AttributeGotoTransition& rSrcAttr) { 
    // call base (incl. virtual clear)
    AttributeVoid::DoAssign(rSrcAttr);
    // my additional members
    mpGrammarSymbol = rSrcAttr.mpGrammarSymbol;
  }

  // Equality
  bool AttributeGotoTransition::DoEqual(const AttributeGotoTransition& rOther) const {
    // my members
     if(*mpGrammarSymbol!=*rOther.mpGrammarSymbol) return false;
    // pass
    return true;
  }

  //DoWrite(rTw,rLabel,pContext);
  void AttributeGotoTransition::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
    (void) pContext;
    if(IsDefault()) return;
    std::string label=rLabel;
    if(label=="") label="";
    FD_DC("AttributeGotoTransition(" << this << ")::DoWrite(tr): to section " << label);
    
    rTw.WriteText(mpGrammarSymbol->Str());
    
  }

  //DoRead(rTr,rLabel,pContext)
  void AttributeGotoTransition::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
    std::string label=rLabel;
    if(label=="") label="";
    FD_DC("AttributeGotoTransition(" << this << ")::DoRead(tr): from section " << label);
    (void) pContext;
    mpGrammarSymbol = GrammarSymbolPtr();
    Token token;
    rTr.Peek(token);
    if(!token.IsBegin())  return;
    if(token.StringValue()!=label) return;
    //mStackSymbols.Read(rTr,label);
    //TODO implement?
  }

  
/*******************************
 * 
 * Implementation of AttributeGotoState
 *
 */

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeGotoState,AttributeFlags)

// Assign my members
void AttributeGotoState::DoAssign(const AttributeGotoState& rSrcAttr) { 
  // call base (incl. virtual clear)
  AttributeFlags::DoAssign(rSrcAttr);
  // my additional members
  mConfigSet=rSrcAttr.mConfigSet;
}

// Equality
bool AttributeGotoState::DoEqual(const AttributeGotoState& rOther) const {
  // base
  if(!AttributeFlags::DoEqual(rOther)) return false;
  // my members
  if(CompareConfigSet(mConfigSet,rOther.mConfigSet) || CompareConfigSet(rOther.mConfigSet,mConfigSet)) return false;
  // pass
  return true;
}


//DoWrite(rTw,rLabel,pContext);
void AttributeGotoState::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  if(IsDefault()) return;
  AttributeFlags::DoWrite(rTw,"",pContext);
  std::string label=rLabel;
  if(label=="") label="State";
  FD_DC("AttributeGotoState(" << this << ")::DoWrite(tr): to section " << label);
  
  rTw.WriteText(ConfigSetToStr(mConfigSet) + "\n");
}


//DoRead(rTr,rLabel,pContext)
void AttributeGotoState::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  // call base first
  AttributeFlags::DoRead(rTr,"",pContext);
  // figure my section
  std::string label=rLabel;
  if(label=="") label="State";
  FD_DC("AttributeGotoState(" << this << ")::DoRead(tr): from section " << label);
  // clear my data
  mConfigSet.clear();
  // test my section
  Token token;
  rTr.Peek(token);
  if(!token.IsBegin())  return;
  if(token.StringValue()!=label) return;
  // read my section (can throw exceptions now)
  //TODO implement?
}



} // namespace faudes

