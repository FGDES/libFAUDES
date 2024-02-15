/** @file mtc_attributes.cpp

Color attributes for states

*/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008  Matthias Singer
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


#include "mtc_attributes.h"

namespace faudes {

// faudes type
FAUDES_TYPE_IMPLEMENTATION(Void,AttributeColoredState,AttributeFlags)

// assignment
void AttributeColoredState::DoAssign(const AttributeColoredState& rSrcAttr) { 
  // call base (incl virtual clear)
  AttributeFlags::DoAssign(rSrcAttr);
  // no additional members
  ColorSymTab(rSrcAttr.mpColorSymbolTable); 
  mColors=rSrcAttr.mColors;
}

bool AttributeColoredState::DoEqual(const AttributeColoredState& rOther) const {
  // call base
  if(!AttributeFlags::DoEqual(rOther)) return false;
  // my data
  if(mColors!=rOther.mColors) return false;
  // pass
  return true;
}



//Write(rTw);
void AttributeColoredState::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  mColors.Write(rTw,"Colors");
}
//Write(rTw);
void AttributeColoredState::DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  mColors.XWrite(rTw,"Colors");
}

// DoRead(rTr)
void  AttributeColoredState::DoRead(TokenReader& rTr,const std::string& rLabel, const Type* pContext) {
  FD_DC("AttributeColoredState(" << this << ")::Read(tr)");
  mColors.Clear();
  Token token;
  rTr.Peek(token);
  if(!token.IsBegin("Colors")) return;
  mColors.Read(rTr,"Colors");
}

// Begin()
NameSet::Iterator AttributeColoredState::ColorsBegin() const {
  return mColors.Begin();
}

// End()
NameSet::Iterator AttributeColoredState::ColorsEnd() const{
  return mColors.End();
}

// ColorSymTab(pSymTab)
void AttributeColoredState::ColorSymTab(SymbolTable *pSymTab) {
  mColors.ColorSymbolTablep(pSymTab);
}

// ColorSymTabp()
SymbolTable *AttributeColoredState::ColorSymTabp(void) const {
  return mColors.SymbolTablep();
}

} // namespace faudes
