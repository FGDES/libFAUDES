/** @file mtc_colorset.cpp

Implements color sets for multitasking automata

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


#include "mtc_colorset.h"



namespace faudes {


// std faudes type (cannot do New() with macro)
FAUDES_TYPE_IMPLEMENTATION_COPY(ColorSet,ColorSet,NameSet)
FAUDES_TYPE_IMPLEMENTATION_CAST(ColorSet,ColorSet,NameSet)
FAUDES_TYPE_IMPLEMENTATION_ASSIGN(ColorSet,ColorSet,NameSet)
FAUDES_TYPE_IMPLEMENTATION_EQUAL(ColorSet,ColorSet,NameSet)


// msColorSymbolTable (static)
SymbolTable ColorSet::msSymbolTable;

// DoAssign()
void ColorSet::DoAssign(const ColorSet& rSourceSet) {
  // call base
  NameSet::DoAssign(rSourceSet);
}

// Colorset::New()
ColorSet* ColorSet::New(void) const {
  ColorSet* res = new ColorSet();
  res->mpSymbolTable=mpSymbolTable;
  return res;
}


// StaticSymbolTablep()
SymbolTable* ColorSet::StaticSymbolTablep(void) {
  return &msSymbolTable;
}

// ColorSymbolTablep()
void ColorSet::ColorSymbolTablep(SymbolTable *pSymTab) {
  mpSymbolTable = pSymTab;
}

// operators
ColorSet ColorSet::operator * (const ColorSet& rOtherSet) const {
  FD_DC("ColorSet(" << this << ")::operator * (" << &rOtherSet << ")");
  ColorSet res;
  res.Assign( NameSet::operator * (rOtherSet) );
  return res;
}

// operator +
ColorSet ColorSet::operator + (const ColorSet& rOtherSet) const {
  FD_DC("ColorSet(" << this << ")::operator + (" << &rOtherSet << ")");
  ColorSet res;
  res.Assign( NameSet::operator + (rOtherSet) );
  return res;
}

// operator -
ColorSet ColorSet::operator - (const ColorSet& rOtherSet) const {
  FD_DC("ColorSet(" << this << ")::operator - (" << &rOtherSet << ")");
  ColorSet res;
  res.Assign( NameSet::operator - (rOtherSet) );
  return res;
}

// operator <=
bool ColorSet::operator <= (const ColorSet& rOtherSet) const {
  return NameSet::operator <= (rOtherSet);
}

// operator >=
bool ColorSet::operator >= (const ColorSet& rOtherSet) const {
  return NameSet::operator >= (rOtherSet);
}

} // namespace faudes
