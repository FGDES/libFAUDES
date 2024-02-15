/** @file diag_attrlabelset.cpp
Implements the label representation for state estimates.
*/

#include "diag_attrlabelset.h"

using namespace std;


namespace faudes {

  // my statics
SymbolTable DiagLabelSet::msLabelSymbolTable;
Idx DiagLabelSet::msLabelN = DiagLabelSet::msLabelSymbolTable.InsEntry("N");
Idx DiagLabelSet::msLabelA = DiagLabelSet::msLabelSymbolTable.InsEntry("A");
Idx DiagLabelSet::msLabelRelN = DiagLabelSet::msLabelSymbolTable.InsEntry("relN");
Idx DiagLabelSet::msLabelSpecViolated = DiagLabelSet::msLabelSymbolTable.InsEntry("F");

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,DiagLabelSet,AttributeFlags)


// construct
DiagLabelSet::DiagLabelSet(void) { 
  // set my static table
  mDiagLabels.SymbolTablep(&msLabelSymbolTable);
  FD_DC("DiagLabelSet("<<this<<")::DiagLabelSet() with LabelSymbolTable "<< mDiagLabels.SymbolTablep());
  // configure unregistered type 
  mDiagLabels.Name("DiagLabels");
  mDiagLabels.TypeName("DiagLabels");
  mDiagLabels.XElementTag("Label");
}

// construct and set symbol table
DiagLabelSet::DiagLabelSet(SymbolTable *pSymTab) {
  // set specified table
  mDiagLabels.SymbolTablep(pSymTab);
  FD_DC("DiagLabelSet("<<this<<")::DiagLabelSet() with LabelSymbolTable "<< mDiagLabels.SymbolTablep()); 
  // configure unregistered type 
  mDiagLabels.Name("DiagLabels");
  mDiagLabels.TypeName("DiagLabels");
  mDiagLabels.XElementTag("Label");
}


// copy my members
void DiagLabelSet::DoAssign(const DiagLabelSet& rSrcAttr) {
  AttributeFlags::DoAssign(rSrcAttr); 
  mDiagLabels=rSrcAttr.mDiagLabels; 
}

// equality 
bool DiagLabelSet::DoEqual(const DiagLabelSet& rOtherSet) const {
  // call base
  if(!AttributeFlags::DoEqual(rOtherSet)) return false;
  // test my members
  if(mDiagLabels != rOtherSet.mDiagLabels) return false;
  // pass
  return true;
}


// IsDefault()
bool DiagLabelSet::IsDefault(void) const {
  return mDiagLabels.Empty();
}

// StaticLabelSymbolTablep()
SymbolTable* DiagLabelSet::StaticLabelSymbolTablep(void) {
  return &msLabelSymbolTable;
}

// LabelSymbolTablep()
void DiagLabelSet::LabelSymbolTablep(SymbolTable *pSymTab) {
  mDiagLabels.SymbolTablep(pSymTab);
}

// Empty()
bool DiagLabelSet::Empty(void) const {
  return mDiagLabels.Empty();
}

// Size()
Idx DiagLabelSet::Size(void) const {
  return mDiagLabels.Size();
}

// Exists()
bool DiagLabelSet::Exists(Idx index) const {
  return mDiagLabels.Exists(index);
}

// Insert()
bool DiagLabelSet::Insert(Idx index) {
  return mDiagLabels.Insert(index);
}

// InsertSet()
void DiagLabelSet::InsertSet(const DiagLabelSet& rSet) {
  mDiagLabels.InsertSet(rSet.mDiagLabels);
}

// Erase()
bool DiagLabelSet::Erase(Idx index) {
  return mDiagLabels.Erase(index);
}

// Clear()
void DiagLabelSet::Clear(void) {
  return mDiagLabels.Clear();
}

// IndexOfLabelN()
Idx DiagLabelSet::IndexOfLabelN(void) {
  return msLabelN;  
}

// IndexOfLabelA()
Idx DiagLabelSet::IndexOfLabelA(void) {
  return msLabelA;  
}

// IndexOfLabelRelN()
Idx DiagLabelSet::IndexOfLabelRelN(void) {
  return msLabelRelN;  
}

// IndexOfLabelRelN()
Idx DiagLabelSet::IndexOfLabelSpecViolated(void) {
  return msLabelSpecViolated;  
}

// Symbol()
string DiagLabelSet::Symbol(Idx index) {
  return msLabelSymbolTable.Symbol(index);  
}

// PrettyPrintLabelSet()
string DiagLabelSet::Str(void) const {
  string str;
  NameSet::Iterator it;
  for (it = mDiagLabels.Begin(); it != mDiagLabels.End(); it++) {
    str += mDiagLabels.SymbolicName(*it);
  }    
  return str;  
}

// DoWrite()
void DiagLabelSet::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  mDiagLabels.Write(rTw, "DiagLabels", pContext);
}

// DoWrite()
void DiagLabelSet::DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  mDiagLabels.XWrite(rTw, "DiagLabels", pContext);
}

// DoRead()
void DiagLabelSet::DoRead(TokenReader &rTr, const std::string &rLabel, const Type *pContext) {
  mDiagLabels.Read(rTr, "DiagLabels", pContext);
}

// operator *
DiagLabelSet DiagLabelSet::operator * (const DiagLabelSet& rOtherSet) const {
  FD_DC("DiagLabelSet(" << this << ")::operator * (" << &rOtherSet << ")");
  DiagLabelSet res;
  res.mDiagLabels = mDiagLabels * rOtherSet.mDiagLabels;
  return res;
}

// operator +
DiagLabelSet DiagLabelSet::operator + (const DiagLabelSet& rOtherSet) const {
  FD_DC("DiagLabelSet(" << this << ")::operator + (" << &rOtherSet << ")");
  DiagLabelSet res;
  res.mDiagLabels = mDiagLabels + rOtherSet.mDiagLabels;
  return res;
}

// operator -
DiagLabelSet DiagLabelSet::operator - (const DiagLabelSet& rOtherSet) const {
  FD_DC("DiagLabelSet(" << this << ")::operator - (" << &rOtherSet << ")");
  DiagLabelSet res;
  res.mDiagLabels = mDiagLabels - rOtherSet.mDiagLabels;
  return res;
}


// operator <=
bool DiagLabelSet::operator <= (const DiagLabelSet& rOtherSet) const {
  return mDiagLabels.operator <= (rOtherSet.mDiagLabels);
}

// operator >=
bool DiagLabelSet::operator >= (const DiagLabelSet& rOtherSet) const {
  return mDiagLabels.operator >= (rOtherSet.mDiagLabels);
}


} // namespace faudes
