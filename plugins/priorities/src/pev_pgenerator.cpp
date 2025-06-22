/** @file pev_pgenerator.cpp Classes pGenerator */


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


      
#include "pev_pgenerator.h" 

namespace faudes {

/*
********************************
AttributePGenGl implementation 
********************************
*/

FAUDES_TYPE_IMPLEMENTATION(Void,AttributePGenGl,AttributeVoid)

/*
********************************
Autoregister
********************************
*/

AutoRegisterType<pGenerator> gRtiRegisterPGenerator("pGenerator");
  
AutoRegisterType<FairnessConstraints> gRtiRegisterFairCons("FairnessConstraints");
AutoRegisterElementTag<FairnessConstraints> gRtiRegisterFairConsETag("FairnessConstraints","EventSet");
AutoRegisterElementType<FairnessConstraints> gRtiRegisterFairConsEType("FairnessConstraints","EventSet");


/*
********************************
Implementation global attribute
********************************
*/

// construct  
AttributePGenGl::AttributePGenGl(void) : AttributeVoid() {
  mFairCons.Name("FairnessConstraints");
}
  
// token io
void AttributePGenGl::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  std::string label=rLabel;
  if(label.empty()) label="FairnessConstraints";
  Token tok;
  rTr.Peek(tok);
  if(!tok.IsBegin(label)) return;
  mFairCons.Read(rTr,label,pContext);
}

// token io
void AttributePGenGl::DoWrite(TokenWriter& rTw,const std::string& rLabel, const Type* pContext) const {
  std::string label=rLabel;
  if(label.empty()) label="FairnessConstraints";
  mFairCons.Write(rTw,label,pContext);
}

// token io
void AttributePGenGl::DoXWrite(TokenWriter& rTw,const std::string& rLabel, const Type* pContext) const {
  std::string label=rLabel;
  if(label.empty()) label="FairnessConstraints";
  mFairCons.XWrite(rTw,label,pContext);
}


  

}// namespace
