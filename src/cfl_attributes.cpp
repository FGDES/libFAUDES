/** @file cfl_attributes.cpp Classes AttributeVoid and AttributeFlags  */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
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


#include "cfl_attributes.h"


namespace faudes {

/***********************************************************************************
 *
 * implementation of AttributeVoid 
 *
 */

// faudes type
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeVoid,Type)

// constructor
AttributeVoid::AttributeVoid(void) {
  FAUDES_OBJCOUNT_INC("Attribute");
}

// constructor
AttributeVoid::AttributeVoid(const AttributeVoid& rOther) {
  FAUDES_OBJCOUNT_INC("Attribute");
  DoAssign(rOther);  
}

  // destructor
AttributeVoid::~AttributeVoid(void) {
  FAUDES_OBJCOUNT_DEC("Attribute");
}

//DoWrite(rTr,rLabel,pContext)
void AttributeVoid::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rTw; (void) rLabel; (void) pContext;
  FD_DC("AttributeVoid::DoWrite()");
}

//DoWrite(rTr,rLabel,pContext)
void AttributeVoid::DoDWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rTw; (void) rLabel; (void) pContext;
  FD_WARN("AttributeVoid::DoWrite()");
  rTw.WriteComment(std::string("Attr Type ") + typeid(*this).name());
  DoWrite(rTw,rLabel,pContext);
}

//DoRead(rTr,rLabel,pContext)
void AttributeVoid::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; (void) pContext; (void) rTr;
  FD_DC("AttributeVoid::DoRead()");
}

//Skip(rTr)
void AttributeVoid::Skip(TokenReader& rTr) {
  FD_DC("AttributeVoid::Skip()");
  Token token;
  while(rTr.Peek(token)) {
    // break on index or name, since this is the next element
    if((token.Type()==Token::String) || (token.Type()==Token::Integer)) {
      break;
    }
    // break on end, since this is the end of the set
    if(token.Type()==Token::End) {
      break;
    }
    // break on Consecutive section, since this belongs to the set
    if((token.Type()==Token::Begin) && (token.StringValue() == "Consecutive")) {
      break;
    }
    // skip any attribute section from other file format
    if(token.Type()==Token::Begin){
      rTr.ReadBegin(token.StringValue());
      rTr.ReadEnd(token.StringValue());
      continue;
    }  
    // skip any other token from other file format
    rTr.Get(token);
  }
}



/***********************************************************************************
 *
 * implementation of AttributeFlags 
 *
 */

// faudes type
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeFlags,AttributeVoid)

// Assign my members
void AttributeFlags::DoAssign(const AttributeFlags& rSrcAttr) { 
  // call virtual clear: TODO: dont clear in virtual function
  Clear();
  // assign my members
  mFlags=rSrcAttr.mFlags; 
}

// Test my members for equality
bool AttributeFlags::DoEqual(const AttributeFlags& rOther) const { 
  return ( mFlags==rOther.mFlags ); 
}

//DoWrite(rTw)
// Note: you should write attributes in a section, so that
// the AttributeVoid read method can detect and skip them.
// Here, we make an execption of the rule ...
void AttributeFlags::DoWrite(TokenWriter& rTw,const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
  if(!AttributeFlags::IsDefault()) {
    FD_DC("AttributeFlags(" << this << ")::DoWrite(tr)");
    Token token;
    token.SetInteger16(mFlags);
    rTw << token;
  }
}


//DoXWrite(rTw)
void AttributeFlags::DoXWrite(TokenWriter& rTw,const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
  if(!AttributeFlags::IsDefault()) {
    FD_DC("AttributeFlags(" << this << ")::DoWrite(tr)");
    Token token;
    token.SetEmpty("Flags");
    token.InsAttributeInteger16("value",mFlags);
    rTw << token;
  }
}


//DoRead(rTr)
void AttributeFlags::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; (void) pContext;
  FD_DC("AttributeFlag(" << this << ")::DoRead(tr)");
  Token token;
  rTr.Peek(token);
  // faudes format
  if(token.IsInteger16()) { 
    rTr.Get(token);
    mFlags=token.IntegerValue();
    return;
  } 
  // XML format
  if(token.IsBegin())
  if(token.StringValue()=="Flags") {
    rTr.ReadBegin("Flags",token);
    mFlags=token.AttributeIntegerValue("value");
    rTr.ReadEnd("Flags");
    return;
  }
  // default
  mFlags=mDefFlags;
}


} // namespace
