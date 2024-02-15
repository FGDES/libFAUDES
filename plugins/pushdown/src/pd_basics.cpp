/** @file pd_basics.cpp  Attributes for pushdown automata */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_basics.h"

namespace faudes {
  
/********************************************************************

 Implementation of StackSymbolSet

********************************************************************/


// std faudes type (cannot do New() with macro)
FAUDES_TYPE_IMPLEMENTATION_COPY(Void,StackSymbolSet,NameSet)
FAUDES_TYPE_IMPLEMENTATION_CAST(Void,StackSymbolSet,NameSet)
FAUDES_TYPE_IMPLEMENTATION_ASSIGN(Void,StackSymbolSet,NameSet)
FAUDES_TYPE_IMPLEMENTATION_EQUAL(Void,StackSymbolSet,NameSet)

// StackSymbolSet::msStackSymbolTable (static)
SymbolTable StackSymbolSet::msSymbolTable;

// StackSymbolSet(void);
StackSymbolSet::StackSymbolSet(void) : NameSet() { 
  // overwrite default static symboltable 
  mpSymbolTable= &msSymbolTable; 
  NameSet::Name("StackSymbols"); 
  FD_DC("StackSymbolSet("<<this<<")::StackSymbolSet() with csymtab "<< SymbolTablep());
}

// StackSymbolSet(StackSymbolSet)
StackSymbolSet::StackSymbolSet(const StackSymbolSet& rOtherSet) : NameSet(rOtherSet) {
  FD_DC("StackSymbolSet(" << this << ")::StackSymbolSet(rOtherSet " << &rOtherSet << ")");
}

// StackSymbolSet(file);
StackSymbolSet::StackSymbolSet(const std::string& rFilename, const std::string& rLabel) : NameSet() {
  // overwrite default static symboltable 
  mpSymbolTable= &msSymbolTable; NameSet::Name("StackSymbols"); 
  // read file
  NameSet::Read(rFilename,rLabel);
}

// StackSymbolSet::New()
StackSymbolSet* StackSymbolSet::New(void) const {
  StackSymbolSet* res = new StackSymbolSet();
  res->mpSymbolTable=mpSymbolTable;
  return res;
}

// DoAssign()
void StackSymbolSet::DoAssign(const StackSymbolSet& rSourceSet) {
  // call base
  NameSet::DoAssign(rSourceSet);
}

// DoEqual()
bool StackSymbolSet::DoEqual(const StackSymbolSet& rOtherSet) const {
  // call base
  return NameSet::DoEqual(rOtherSet);
}

// StackSymbolSet::StaticSymbolTablep
SymbolTable* StackSymbolSet::StaticSymbolTablep(void) {
  return &msSymbolTable;
}

/********************************************************************

 Implementation of StackSymbol

********************************************************************/

void StackSymbol::Write(TokenWriter& tw) const{
  Write(tw,mSymbol);
}

void StackSymbol::Write(TokenWriter& tw, const std::string& rLabel) const{
  Token token;
  int oldcolumns = tw.Columns();
  tw.Columns(1);
  tw.WriteBegin(rLabel);
  
  token.SetString(mSymbol);
  tw << token;

  tw.WriteEnd(rLabel);
  tw.Columns(oldcolumns);
}

bool StackSymbol::operator!=(const StackSymbol &other) const{
  if (mSymbol.compare(other.mSymbol) == 0)
    return false;
  return true;
}

bool StackSymbol::operator==(const StackSymbol &other) const{
  if (mSymbol.compare(other.mSymbol) == 0)
    return true;
  return false;
}

bool StackSymbol::operator<(const StackSymbol &other) const{
  if (mSymbol.compare(other.mSymbol) < 0)
    return true;
  return false;
}

bool StackSymbol::IsLambda() const{
  if (mSymbol.compare(FAUDES_PD_LAMBDA) == 0)
    return true;
  return false;
}

std::string StackSymbol::ToString() const{
  return mSymbol;
}

bool StackSymbol::Empty() const{
  if (mSymbol.compare("") == 0)
    return true;
  return false;
}

} // namespace faudes

