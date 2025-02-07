/** @file cfl_cgenerator.cpp Classes TcGenerator, System and AttributeCFlags */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
   Copyright (C) 2007  Thomas Moor
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


      
#include "cfl_cgenerator.h" 


namespace faudes {

// instantiate to export symbols for API type System (clang 8.0.0 issue, 2017/12 )
/*
template class FAUDES_API TaNameSet<AttributeCFlags>;
template class FAUDES_TAPI TcGenerator<AttributeVoid, AttributeVoid, AttributeCFlags,AttributeVoid>; 
*/


/***********************************************************************************
 *
 * implementation of AttributeCFlags 
 *
 */


// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeCFlags,AttributeFlags)

// Assign my members
void AttributeCFlags::DoAssign(const AttributeCFlags& rSrcAttr) { 
  // call base
  AttributeFlags::DoAssign(rSrcAttr);
}

// Test my members
bool AttributeCFlags::DoEqual(const AttributeCFlags& rOther) const { 
  // call base
  if(!AttributeFlags::DoEqual(rOther)) return false;
  // no additional members
  return true;
}

//Write(rTw)
// Note: you should write attributes in a section, so that
// the AttributeVoid read method can detect and skip them.
// Here, we make an execption of the rule ...
void AttributeCFlags::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
  if(IsDefault()) return;
  FD_DC("AttributeCFlags(" << this << ")::DoWrite(tr)");
  Token token;
  // if no other flags used, write option string
  if( (mFlags & ~mAllCFlags) == 0 ) {
    std::string option;
    if((mDefCFlags & mControllableFlag) != (mFlags & mControllableFlag)) {
      if(Controllable()) option = option+"C";
      else option = option+"c";
    }
    if((mDefCFlags & mObservableFlag) != (mFlags & mObservableFlag)) {
      if(Observable()) option = option+"O";
      else option = option+"o";
    }
    if((mDefCFlags & mForcibleFlag) != (mFlags & mForcibleFlag)) {
      if(Forcible()) option = option+"F";
      else option = option+"f";
    }
    if((mDefCFlags & mAbstractionFlag) != (mFlags & mAbstractionFlag)) {
      if(Highlevel()) option = option+"A";
      else option = option+"a";
    }
    if(option!="") {
      token.SetOption(option);
      rTw << token;
    }
  } 
  // if other flags used, write hex
  else { 
    token.SetInteger16(mFlags);
    rTw << token;
  }
}

//XWrite(rTw)
void AttributeCFlags::DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
  if(IsDefault()) return;
  FD_DC("AttributeCFlags(" << this << ")::DoXWrite(tw)");
  Token token;
  // if further flags are used, let base class write with base 16 
  if( (mFlags & ~mAllCFlags) != 0 ) {
    AttributeFlags::DoXWrite(rTw,rLabel,pContext);
  }
  // write friendly tags anyway
  std::string option;
  if((mDefCFlags & mControllableFlag) != (mFlags & mControllableFlag)) {
    token.SetEmpty("Controllable");
    if(!Controllable()) token.InsAttributeBoolean("value",0);
    rTw.Write(token);
  }
  if((mDefCFlags & mObservableFlag) != (mFlags & mObservableFlag)) {
    token.SetEmpty("Observable");
    if(!Observable()) token.InsAttributeBoolean("value",0);
    rTw.Write(token);
  }
  if((mDefCFlags & mForcibleFlag) != (mFlags & mForcibleFlag)) {
    token.SetEmpty("Forcible");
    if(!Forcible()) token.InsAttributeBoolean("value",0);
    rTw.Write(token);
  }
  if((mDefCFlags & mAbstractionFlag) != (mFlags & mAbstractionFlag)) {
    token.SetEmpty("HighLevel");
    if(!Highlevel()) token.SetEmpty("LowLevel");
    rTw.Write(token);
  } 
}

//DoRead(rTr)
void AttributeCFlags::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; (void) pContext;
  mFlags=mDefCFlags;
  Token token;
  rTr.Peek(token);
  // faudes format, flags as integer
  if(token.IsInteger16()) { 
    AttributeFlags::DoRead(rTr,rLabel,pContext);
    return;
  } 
  // faudes format, flags as option string
  if(token.IsOption()) {
    rTr.Get(token);
    std::string option=token.OptionValue();
    if(option.find( 'C', 0) != std::string::npos) SetControllable();
    if(option.find( 'c', 0) != std::string::npos) ClrControllable();
    if(option.find( 'O', 0) != std::string::npos) SetObservable();
    if(option.find( 'o', 0) != std::string::npos) ClrObservable();
    if(option.find( 'F', 0) != std::string::npos) SetForcible();
    if(option.find( 'f', 0) != std::string::npos) ClrForcible();
    if(option.find( 'A', 0) != std::string::npos) SetHighlevel();
    if(option.find( 'a', 0) != std::string::npos) SetLowlevel();
    return;
  }
  // xml format 
  while(true) {
    rTr.Peek(token);
    // explicit integer
    if(token.IsBegin("Flags")) {
      AttributeFlags::DoRead(rTr,rLabel,pContext);
      continue;
    }
    // known bits
    if(token.IsBegin("Controllable")) {
      rTr.ReadBegin("Controllable",token);
      SetControllable();
      if(token.ExistsAttributeInteger("value"))
	if(token.AttributeIntegerValue("value")==false)
          ClrControllable();
      rTr.ReadEnd("Controllable");
      continue;
    }
    if(token.IsBegin("Observable")) {
      rTr.ReadBegin("Observable",token);
      SetObservable();
      if(token.ExistsAttributeInteger("value"))
	if(token.AttributeIntegerValue("value")==false)
          ClrObservable();
      rTr.ReadEnd("Observable");
      continue;
    }
    if(token.IsBegin("Forcible")) {
      rTr.ReadBegin("Forcible",token);
      SetForcible();
      if(token.ExistsAttributeInteger("value"))
	if(token.AttributeIntegerValue("value")==false)
          ClrForcible();
      rTr.ReadEnd("Forcible");
      continue;
    }
    if(token.IsBegin("HighLevel")) {
      rTr.ReadBegin("HighLevel",token);
      SetHighlevel();
      if(token.ExistsAttributeInteger("value"))
	if(token.AttributeIntegerValue("value")==false)
          SetLowlevel();
      continue;
    }
    if(token.IsBegin("LowLevel")) {
      rTr.ReadBegin("LowLevel",token);
      SetLowlevel();
      if(token.ExistsAttributeInteger("value"))
	if(token.AttributeIntegerValue("value")==false)
          SetHighlevel();
      continue;
    }
    // stop at unknown tag
    break;
  }
}


} // end namespace
