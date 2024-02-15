/** @file hio_attributes.cpp Event and state attributes for hierarchical I/O systems */

/* Hierarchical IO Systems Plug-In for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Sebastian Perk 
   Copyright (C) 2006  Thomas Moor 
   Copyright (C) 2006  Klaus Schmidt

*/

#include "hio_attributes.h"

namespace faudes {


/***********************************************************************************
 *
 * implementation of HioEventFlags 
 *
 */

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,HioEventFlags,AttributeFlags)

// Assign my members
void HioEventFlags::DoAssign(const HioEventFlags& rSrcAttr) { 
  // call base (incl virtual clear)
  AttributeFlags::DoAssign(rSrcAttr);
}

//DoWrite(rTw,rLabel,pContext);
void HioEventFlags::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {  

  (void) rLabel; (void) pContext;
  if(IsDefault()) return;
  FD_DC("HioEventFlags(" << this << ")::DoWrite(tr)");
  
    Token token;
    if( (mFlags & ~mAllHioEventFlags) == 0 ) {
      std::string option;
    switch (mFlags)
    {
           case 4:  option = "U";break;   //0000 0100
           case 8:  option = "Y";break;   //0000 1000
           case 36:  option = "UE";break; //0010 0100
           case 40:  option = "YE";break; //0010 1000
           case 20:  option = "UP";break; //0001 0100
           case 24:  option = "YP";break; //0001 1000
           case 68:  option = "UC";break; //0100 0100
           case 72:  option = "YC";break; //0100 1000
           case 132: option = "UL";break; //1000 0100
           case 136: option = "YL";break; //1000 1000
           default:  option = "??";break;
           }
      token.SetOption(option);
    } 
    // if other flags used, write hex
    else { 
      token.SetInteger16(mFlags);
    }
    rTw << token;
  }
//}


//DoRead(rTw,rLabel,pContext);
void HioEventFlags::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; 
  (void) pContext;
  // initialize with dfeualt
  mFlags=mDefHioEventFlags;
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

    if(option.find( 'Y', 0) != std::string::npos) SetY();
    if(option.find( 'U', 0) != std::string::npos) SetU();
    if(option.find( 'P', 0) != std::string::npos){ SetP(); ClrE();}
    if(option.find( 'E', 0) != std::string::npos){ SetE(); ClrP();}
    if(option.find( 'C', 0) != std::string::npos){ SetC(); ClrP();}
    if(option.find( 'L', 0) != std::string::npos){ SetL(); ClrE();}
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
    /* 
       xml format not yet implemented, 
       see iosystem for template
    */
    // stop at unknown tag
    break;
  }
}


/***********************************************************************************
 *
 * implementation of HioStateFlags 
 *
 */
/***************************************************************************/

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,HioStateFlags,AttributeFlags)

// Assign my members
void HioStateFlags::DoAssign(const HioStateFlags& rSrcAttr) { 
  // call base (incl virtual clear)
  AttributeFlags::DoAssign(rSrcAttr);
}

//DoWrite(rTw,rLabel,pContext);
void HioStateFlags::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rLabel; (void) pContext;
  if(IsDefault()) return;
  FD_DC("HioStateFlags(" << this << ")::DoWrite(tr)");
    Token token;
    if( (mFlags & ~mAllHioStateFlags) == 0 ) {
      std::string option;
      if(IsErr()) option = "Err";
    switch (mFlags&0xFF) // AllHioStateFlags but not ErrFlag
    {
           case   1: option = option+"QY";     break; //0000 0001
           case   2: option = option+"QU";     break; //0000 0010
           case   4: option = option+"QC";     break; //0000 0100
           case   6: option = option+"QUc";     break; //0000 0110
           case   8: option = option+"QP";     break; //0000 1000
           case   9: option = option+"QYp";     break; //0000 1001
           case   10: option = option+"QUp";     break; //0000 1010
           case  16: option = option+"QE";     break; //0001 0000
           case  17: option = option+"QYe";     break; //0001 0001
           case  18: option = option+"QUe";     break; //0001 0010
           case  25: option = option+"QYpYe";     break; //0001 1001
           case  32: option = option+"QL";     break; //0010 0000
           case  34: option = option+"QUl";     break; //0010 0010
           case  64: option = option+"QYcUp"; break; //0100 0000
           case 128: option = option+"QYlUe"; break; //1000 0000
           
           default: if( !IsErr() ) option = option+"??";      break;
           }
      
      token.SetOption(option);
    } 
    // if other flags used, write hex
    else { 
      token.SetInteger16(mFlags);
    }
    rTw << token;
  }

//ToString()
std::string HioStateFlags::ToString(void) const {
  return ToStringInteger16(mFlags);
}

//DoRead(rTw,rLabel,pContext);
void HioStateFlags::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; 
  (void) pContext;
  // initialize with dfeualt
  mFlags=mDefHioStateFlags;
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

    if(option.find( "QY", 0) != std::string::npos){ SetQY();}
    if(option.find( "QU", 0) != std::string::npos){ SetQU();}
    if(option.find( "QC", 0) != std::string::npos){ SetQC();}
    if(option.find( "QUc", 0) != std::string::npos){ SetQU(); SetQC();}
    if(option.find( "QP", 0) != std::string::npos){ SetQP();}
    if(option.find( "QYp", 0) != std::string::npos){ SetQY(); SetQP();}
    if(option.find( "QUp", 0) != std::string::npos){ SetQU(); SetQP();}
    if(option.find( "QE", 0) != std::string::npos){ SetQE();}
    if(option.find( "QYe", 0) != std::string::npos){ SetQY(); SetQE();}
    if(option.find( "QUe", 0) != std::string::npos){ SetQU(); SetQE();}
    if(option.find( "QYpYe", 0) != std::string::npos){ SetQY(); SetQP(); SetQE();}
    if(option.find( "QL", 0) != std::string::npos){ SetQL();}
    if(option.find( "QUl", 0) != std::string::npos){ SetQU(); SetQL();}
    if(option.find( "QYcUp", 0) != std::string::npos){ SetQYcUp();}
    if(option.find( "QYlUe", 0) != std::string::npos){ SetQYlUe();}
    if(option.find( "Err", 0) != std::string::npos){ SetErr();}
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
    /* 
       xml format not yet implemented, 
       see iosystem for template
    */
    // stop at unknown tag
    break;
  }
}




} //End namespace
