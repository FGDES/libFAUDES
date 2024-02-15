/** @file cfl_symbolset.cpp @brief Class SymbolSet */

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

#include "cfl_symbolset.h"

namespace faudes {

// std faudes type
FAUDES_TYPE_IMPLEMENTATION(SymbolSet,SymbolSet,TBaseSet<std::string>)


// SymbolSet()
SymbolSet::SymbolSet(void) : 
  TBaseSet<std::string>() 
{
  FD_DC("SymbolSet(" << this << ")::SymbolSet()");
  Name("SymbolSet");
}

// SymbolSet(rOtherSet)
SymbolSet::SymbolSet(const TBaseSet<std::string>& rOtherSet) : 
  TBaseSet<std::string>(rOtherSet) 
{
  FD_DC("SymbolSet(" << this << ")::SymbolSet(rOtherSet " << &rOtherSet << ")");
}

// File constructor
SymbolSet::SymbolSet(const std::string& rFilename, const std::string& rLabel) :
  TBaseSet<std::string>() 
{
  FD_DC("SymbolSet(" << this << ")::SymbolSet(" << rFilename << ")");
  Read(rFilename, rLabel);
}

// DoAssign
void SymbolSet::DoAssign(const SymbolSet& rSourceSet) {
  TBaseSet<std::string>DoAssign(rSourceSet);
}


// DoWrite(rTw&)
void SymbolSet::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  std::string label=rLabel;
  if(label=="") label=Name(); 
  if(label=="") label="SymbolSet"; 
  rTw.WriteBegin(label);
  Iterator it;
  // iterate symbols to write
  for (it = Begin(); it != End(); ++it) {
    rTw << *it;
  }
  rTw.WriteEnd(label);
}

// DoRead(rTr, rLabel, context)
void SymbolSet::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  std::string label=rLabel;
  if(label=="") label="SymbolSet"; 
  Name(label);
  Clear();
  rTr.ReadBegin(label); 
  Token token;
  while(!rTr.Eos(label)) {
    rTr.Get(token);
    // read individual symbol
    if (token.Type() == Token::String) {
      FD_DC("SymbolSet(" << this << ")::DoRead(..): inserting symbol \"" 
	    << token.StringValue() << "\"");
      // insert element
      Insert(token.StringValue());
      continue;
    }
    // ignore unknown attributes
    AttributeVoid attr;
    attr.Read(rTr);
  }
  rTr.ReadEnd(label);
}

//Insert(symbol)
bool SymbolSet::Insert(const std::string& symbol) {
  return TBaseSet<std::string>::Insert(symbol);
}


//Valid(idx)
bool  SymbolSet::Valid(const std::string& symbol) const {
  return SymbolTable::ValidSymbol(symbol);
}




} // end name space
