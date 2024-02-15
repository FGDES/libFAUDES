/** @file ios_attributes.cpp I/O-system attributes */

/* 
   Standart IO Systems Plug-In for FAU Discrete Event Systems 
   Library (libfaudes)

   Copyright (C) 2010, Thomas Wittmann, Thomas Moor

*/


#include "ios_attributes.h"
#include <cfl_types.h>

namespace faudes {


/*
*********************************************************************************
*********************************************************************************
*********************************************************************************

Implementation: event attributes 

*********************************************************************************
*********************************************************************************
*********************************************************************************
*/


// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeIosEvent,AttributeFlags)

  
// Assign my members
void AttributeIosEvent::DoAssign(const AttributeIosEvent& rSrcAttr) { 
  // call base (incl virtual clear)
  AttributeFlags::DoAssign(rSrcAttr);
}


//DoWrite(rTw,rLabel,pContext);
void AttributeIosEvent::DoWrite(TokenWriter& rTw, 
				const std::string& rLabel, 
				const Type* pContext) const 
{  
  (void) rLabel; 
  (void) pContext;
  // dont write default values
  if(IsDefault()) return;
  // debug output
  FD_DC("AttributeIosEvent(" << this << ")::DoWrite(tr)"); 
  Token token;	
  // if further flags are used, let base class write with base 16 
  if( (mFlags & ~mAllIosFlags) != 0 ) {
    AttributeFlags::DoWrite(rTw,rLabel,pContext);
    return;
  }
  // write my flags symbolically
  std::string option;
  switch (mFlags){
  case 0x10:
    option = "I";  
    break;
  case 0x20: 
    option = "O";
    break; 
  default:  
    option = "IO";
    break;
  }
  rTw.WriteOption(option);
}


//XWrite(rTw)
void AttributeIosEvent::DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
  if(IsDefault()) return;
  FD_DC("AttributeIosEvent(" << this << ")::DoXWrite(tw)");
  Token token;
  // if further flags are used, let base cxlass write with base 16 
  if( (mFlags & ~mAllIosFlags) != 0 ) {
    AttributeFlags::DoXWrite(rTw,rLabel,pContext);
  }
  // write friendly tags anyway
  if((mDefIosFlags & mInputFlag) != (mFlags & mInputFlag)) {
    token.SetEmpty("Input");
    if(!Input()) token.InsAttributeBoolean("value",0);
    rTw.Write(token);
  }
  if((mDefIosFlags & mOutputFlag) != (mFlags & mOutputFlag)) {
    token.SetEmpty("Output");
    if(!Output()) token.InsAttributeBoolean("value",0);
    rTw.Write(token);
  }
}




//DoRead(rTw,rLabel,pContext);
void AttributeIosEvent::DoRead(
  TokenReader& rTr, 
  const std::string& rLabel, 
  const Type* pContext) 
{    
  (void) rLabel;			
  (void) pContext;			
  // initialize with default value
  mFlags = mDefIosFlags;	 
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
    if(option.find( 'I', 0) != std::string::npos) SetInput();
    if(option.find( 'i', 0) != std::string::npos) ClrInput();
    if(option.find( 'O', 0) != std::string::npos) SetOutput();
    if(option.find( 'o', 0) != std::string::npos) ClrOutput();
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
    if(token.IsBegin("Input")) {
      rTr.ReadBegin("Input",token);
      SetInput();
      if(token.ExistsAttributeInteger("value"))
	if(token.AttributeIntegerValue("value")==false)
          ClrInput();
      rTr.ReadEnd("Input");
      continue;
    }
    if(token.IsBegin("Output")) {
      rTr.ReadBegin("Output",token);
      SetOutput();
      if(token.ExistsAttributeInteger("value"))
	if(token.AttributeIntegerValue("value")==false)
          ClrOutput();
      rTr.ReadEnd("Output");
      continue;
    }
    // stop at unknown tag
    break;
  }
}


/*
*********************************************************************************
*********************************************************************************
*********************************************************************************

Implementation: state attributes 

*********************************************************************************
*********************************************************************************
*********************************************************************************
*/



// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeIosState,AttributeFlags)

//DoAssign( rSrcAttr )
void AttributeIosState::DoAssign(const AttributeIosState& rSrcAttr) { 
  // call base (incl virtual clear)
  AttributeFlags::DoAssign(rSrcAttr);
}


//ToString()
std::string AttributeIosState::ToString(void) const {
  return ToStringInteger16(mFlags);
}


//DoWrite(rTw,rLabel,pContext);
void AttributeIosState::DoWrite(TokenWriter& rTw, 
				const std::string& rLabel, 
				const Type* pContext) const 
{
  (void) rLabel;	 
  (void) pContext;	
  // dont wrtite default value
  if(IsDefault()) return;
  FD_DC("AttributeIosState(" << this << ")::DoWrite(tr)");
  // if further flags are used, let base class write with base 16 
  if( (mFlags & ~mAllIosFlags) != 0 ) {
    AttributeFlags::DoWrite(rTw,rLabel,pContext);
    return;
  }
  // write my flags symbolically
  std::string option;
  if(Input()) option+="I";
  if(Output()) option+="O";
  if(Error()) option+="E";
  if(option!="")
    rTw.WriteOption(option);
}


//XWrite(rTw)
void AttributeIosState::DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
  if(IsDefault()) return;
  FD_DC("AttributeIosState(" << this << ")::DoXWrite(tw)");
  Token token;
  // if further flags are used, let base class write with base 16 
  if( (mFlags & ~mAllIosFlags) != 0 ) {
    AttributeFlags::DoXWrite(rTw,rLabel,pContext);
  }
  // write friendly tags anyway
  if((mDefIosFlags & mInputFlag) != (mFlags & mInputFlag)) {
    token.SetEmpty("Input");
    if(!Input()) token.InsAttributeBoolean("value",0);
    rTw.Write(token);
  }
  if((mDefIosFlags & mOutputFlag) != (mFlags & mOutputFlag)) {
    token.SetEmpty("Output");
    if(!Output()) token.InsAttributeBoolean("value",0);
    rTw.Write(token);
  }
  if((mDefIosFlags & mErrorFlag) != (mFlags & mErrorFlag)) {
    token.SetEmpty("Error");
    if(!Error()) token.InsAttributeBoolean("value",0);
    rTw.Write(token);
  }
}



//DoRead(rTw,rLabel,pContext);
void AttributeIosState::DoRead( 
  TokenReader& rTr, 
  const std::string& rLabel, 
  const Type* pContext) 
{    
  (void) rLabel;			
  (void) pContext;			
  // initialize with default value
  mFlags = mDefIosFlags;	 
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
    if(option.find( 'I', 0) != std::string::npos) SetInput();
    if(option.find( 'i', 0) != std::string::npos) ClrInput();
    if(option.find( 'O', 0) != std::string::npos) SetOutput();
    if(option.find( 'o', 0) != std::string::npos) ClrOutput();
    if(option.find( 'E', 0) != std::string::npos) SetError();
    if(option.find( 'e', 0) != std::string::npos) ClrError();
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
    if(token.IsBegin("Input")) {
      rTr.ReadBegin("Input",token);
      SetInput();
      if(token.ExistsAttributeInteger("value"))
	if(token.AttributeIntegerValue("value")==false)
          ClrInput();
      rTr.ReadEnd("Input");
      continue;
    }
    if(token.IsBegin("Output")) {
      rTr.ReadBegin("Output",token);
      SetOutput();
      if(token.ExistsAttributeInteger("value"))
	if(token.AttributeIntegerValue("value")==false)
          ClrOutput();
      rTr.ReadEnd("Output");
      continue;
    }
    if(token.IsBegin("Error")) {
      rTr.ReadBegin("Error",token);
      SetError();
      if(token.ExistsAttributeInteger("value"))
	if(token.AttributeIntegerValue("value")==false)
          ClrError();
      rTr.ReadEnd("Error");
      continue;
    }
    // stop at unknown tag
    break;
  }
}



}//end namespace faudes
