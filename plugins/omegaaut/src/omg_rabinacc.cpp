/** @file pev_priorities.cpp Classes AttributePriority, EventPriorities*/


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


      
#include "pev_priorities.h" 

namespace faudes {

  
/*
********************************
Autoregister 
********************************
*/

AutoRegisterType<EventPriorities> gRtiEventPriorities("EventPriorities");
AutoRegisterType<FairnessConstraints> gRtiFairnessConstraints("FairnessConstraints");
  

/*
********************************
implementation AttributePriority
********************************
*/

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributePriority,AttributeCFlags)


// Assign my members
void AttributePriority::DoAssign(const AttributePriority& rSrcAttr) { 
  // call base (incl. virtual clear)
  AttributeCFlags::DoAssign(rSrcAttr);
  // my additional members
  mPriority = rSrcAttr.mPriority;
}


// Test equality
bool AttributePriority::DoEqual(const AttributePriority& rOther) const {
  FD_DC("AttributePriority::DoEqual() A");
  // call base 
  if(!AttributeCFlags::DoEqual(rOther)) return false;
  // my additional members
  FD_DC("AttributePriority::DoEqual() B "  << mPriority << " vs " << rOther.mPriority);
  return mPriority == rOther.mPriority;
}

// Write XML only
void AttributePriority::DoXWrite(TokenWriter& rTw,const std::string& rLabel, const Type* pContext) const{
  (void) pContext;
  if(IsDefault()) return;
  FD_DC("AttributeCFlags(" << this << ")::DoXWrite(tw)");
  // let base calls do their job
  AttributeCFlags::DoXWrite(rTw,rLabel,pContext);
  // add mine
  Token token;
  if(mPriority!=0) {
    token.SetEmpty("Priority");
    token.InsAttributeInteger("value",mPriority);
    rTw.Write(token);
  }
}

// Write (XML only)
void AttributePriority::DoWrite(TokenWriter& rTw,const std::string& rLabel, const Type* pContext) const {
  DoXWrite(rTw,rLabel,pContext);
}

//DoRead(rTr)
void AttributePriority::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  // let base do their job
  AttributeCFlags::DoRead(rTr,rLabel,pContext);
  // read my XML
  Token token;
  while(true) {
    rTr.Peek(token);
    // explicit integer
    if(token.IsBegin("Priority")) {
      rTr.ReadBegin("Priority",token);
      mPriority=0;
      if(token.ExistsAttributeInteger("value"))
	mPriority=token.AttributeIntegerValue("value");
      rTr.ReadEnd("Priority");
      continue;
    }
    // stop at unknown tag
    break;
  }
}



  

}// namespace
// 
