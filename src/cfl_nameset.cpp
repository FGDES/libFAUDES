/** @file cfl_nameset.cpp @brief Classes NameSet */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
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


#include "cfl_nameset.h"

namespace faudes {

/*
*********************************************************************************
*********************************************************************************
*********************************************************************************

 NameSet 

*********************************************************************************
*********************************************************************************
*********************************************************************************
*/


// std faudes type (cannot do New() with macro)
FAUDES_TYPE_IMPLEMENTATION_COPY(EventSet,NameSet,TBaseSet<Idx>)
FAUDES_TYPE_IMPLEMENTATION_CAST(EventSet,NameSet,TBaseSet<Idx>)
FAUDES_TYPE_IMPLEMENTATION_ASSIGN(EventSet,NameSet,TBaseSet<Idx>)
FAUDES_TYPE_IMPLEMENTATION_EQUAL(EventSet,NameSet,TBaseSet<Idx>)


// empty constructor
NameSet::NameSet(void) : TBaseSet<Idx>(){
  mpSymbolTable = SymbolTable::GlobalEventSymbolTablep();
  Name("NameSet");
  FD_DC("NameSet("<<this<<")::NameSet() with symtab "<< mpSymbolTable); 
}

// constructor from nameset
NameSet::NameSet(const NameSet& rOtherSet) : TBaseSet<Idx>() {
  FD_DC("NameSet(" << this << ")::NameSet(rOtherSet " << &rOtherSet << ")");
  mpSymbolTable = rOtherSet.mpSymbolTable;
  Assign(rOtherSet);
  FD_DC("NameSet(" << this << ")::NameSet(rOtherSet " << &rOtherSet << "): done");
}

// read file constructor
NameSet::NameSet(const std::string& rFilename, const std::string& rLabel) : TBaseSet<Idx>() {
  FD_DC("NameSet(" << this << ")::NameSet(" << rFilename << ")");
  mpSymbolTable = SymbolTable::GlobalEventSymbolTablep();
  Read(rFilename, rLabel);
}

// destructor
NameSet::~NameSet(void) {
  FD_DC("NameSet("<<this<<")::~NameSet()");
}

// New()
NameSet* NameSet::New(void) const {
  NameSet* res= new NameSet();
  res->mpSymbolTable=mpSymbolTable;
  return res;
}


// copy (attributes to default)
void NameSet::DoAssign(const NameSet& rSourceSet) {
  FD_DC("NameSet(" << this << ")::DoAssign(..)");
  // fix my symboltable
  mpSymbolTable=rSourceSet.mpSymbolTable;
  // call base 
  TBaseSet<Idx>::DoAssign(rSourceSet);
} 

// Compare
bool NameSet::DoEqual(const NameSet& rOtherSet) const {
  FD_DC("NameSet::DoEqual()");
#ifdef FAUDES_CHECKED
  if(rOtherSet.mpSymbolTable!=mpSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismazch aka not implemented" << std::endl;
    throw Exception("NameSet::DoEqual()", errstr.str(), 67);
  }
#endif
  return TBaseSet<Idx>::DoEqual(rOtherSet);
}

// SymbolTablep()
SymbolTable* NameSet::SymbolTablep(void) const {
  return mpSymbolTable;
}

// SymbolTablep(pSymTab)
void NameSet::SymbolTablep(SymbolTable* pSymTab) {
  FD_DC("NameSet(" << this << ")::SymbolTablep(" << pSymTab << ")");
  if(!Empty()) {
    Clear();
  } 
  mpSymbolTable=pSymTab;
}

// DoWrite(tw, rLabel)
void NameSet::DoWrite(TokenWriter& tw, const std::string& rLabel, const Type* pContext) const {
  std::string label=rLabel;
  if(label=="") label=Name();  
  FD_DC("NameSet(" << this << ")::DoWrite(..): section " << label << " #" << Size());
  tw.WriteBegin(label);
  Token token;
  Iterator it;
  for (it = Begin(); it != End(); ++it) {
#ifdef FAUDES_DEBUG_CODE
    if (SymbolicName(*it) == "") {
      FD_ERR("NameSet::Write(): "
	   << "index " << *it << " not in SymbolTable. aborting...");
      abort();
    }
#endif
    token.SetString(SymbolicName(*it));
    tw << token;
    Attribute(*it).Write(tw,"",pContext);
  }
  tw.WriteEnd(label);
}

// DoDWrite()
void NameSet::DoDWrite(TokenWriter& tw, const std::string& rLabel, const Type* pContext) const {
  std::string label=rLabel;
  if(label=="") label=Name(); 
  if(label=="") label=TypeName(); 
  tw.WriteBegin(label);
  Token token;
  Iterator it;
  for (it = Begin(); it != End(); ++it) {
    tw << Str(*it);
    Attribute(*it).Write(tw,"",pContext);
  }
  tw.WriteEnd(label);
}


// DoXWrite(tw, rLabel)
void NameSet::DoXWrite(TokenWriter& tw, const std::string& rLabel, const Type* pContext) const {
  // Set up outer tag
  Token btag=XBeginTag(rLabel,"NameSet");
  tw.Write(btag);
  FD_DC("NameSet(" << this << ")::DoXWrite(..): section " << btag.StringValue() << " #" << Size());
  // loop elements
  std::string etstr=XElementTag();
  for (Iterator it = Begin(); it != End(); ++it) {
    Token etoken;
    const AttributeVoid& attr=Attribute(*it);
    // case a: no attribute value
    if(attr.IsDefault()) {
      etoken.SetEmpty(etstr);
      etoken.InsAttributeString("name",SymbolicName(*it));
      tw << etoken;
    } 
    // case b: incl attribute value
    if(!attr.IsDefault()) {
      etoken.SetBegin(etstr);
      etoken.InsAttributeString("name",SymbolicName(*it));
      tw << etoken;
      Attribute(*it).XWrite(tw,"",pContext);
      etoken.SetEnd(etstr);
      tw << etoken;
    }
  }
  tw.WriteEnd(btag.StringValue());
}

// DoRead(rTr, rLabel, pContext)
void NameSet::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  // set up defaults
  std::string label=rLabel;
  std::string ftype=TypeName();
  std::string etstr=XElementTag();
  // figure section
  Token token;
  if(label=="") {
    rTr.Peek(token);
    if(token.Type()==Token::Begin) label=token.StringValue();
  }
  if(label=="") label=ftype; 
  Name(label);
  // read begin
  rTr.ReadBegin(label,token); 
  if(token.ExistsAttributeString("name"))
    Name(token.AttributeStringValue("name"));
  FD_DC("NameSet(" << this << ")::DoRead(..): section " << label << " with symtab " << mpSymbolTable);
  // prepare attribute
  AttributeVoid* attrp = AttributeType()->New();
  FD_DC("NameSet(" << this << ")::DoRead(..): attribute type " << typeid(*attrp).name());
  // loop tokens
  std::string name;
  while(!rTr.Eos(label)) {
    rTr.Peek(token);
    // read by name (faudes format)
    if(token.IsString()) {
      FD_DC("TaNameSet(" << this << ")::DoRead(..): inserting element in faudes format \"" 
	    << token.StringValue() << "\"");
      rTr.Get(token);
      name=token.StringValue();
      // read faudes attribute
      attrp->Read(rTr,"",pContext);
      // skip unknown faudes attributes
      AttributeVoid::Skip(rTr);
      // insert element with attribute
      Idx index=Insert(name); 
      Attribute(index,*attrp);
      continue;
    }
    // read element section (XML format)
    if(token.IsBegin(etstr)) {
      FD_DC("TaNameSet(" << this << ")::DoRead(..): inserting element in xml format \"" 
	    << token.StringValue() << "\"");
      rTr.ReadBegin(etstr,token);
      name=token.AttributeStringValue("name");
      // read faudes attribute
      attrp->Read(rTr,"",pContext);
      // skip unknown faudes attributes
      rTr.ReadEnd(etstr);
      // insert element with attribute
      Idx index=Insert(name); 
      Attribute(index,*attrp);
      continue;
    }
    // cannot process token
    delete attrp;
    std::stringstream errstr;
    errstr << "Invalid token of type " << token.Type() << " at " << rTr.FileLine();
    throw Exception("NameSet::DoRead", errstr.str(), 50);
  }
  rTr.ReadEnd(label);
  delete attrp;
  FD_DC("NameSet(" << this << ")::DoRead(tr," << label << ", " << pContext << "): done");
}


// Insert(index)
bool NameSet::Insert(const Idx& index) { 
#ifdef FAUDES_CHECKED
  if(!mpSymbolTable->Exists(index)) {
    std::stringstream errstr;
    errstr << "index " << index << " not known to symboltable" << std::endl;
    throw Exception("NameSet::Insert", errstr.str(), 65);
  }
#endif
  return TBaseSet<Idx>::Insert(index);
}

// Insert(rName)
Idx NameSet::Insert(const std::string& rName) {
  FD_DC("NameSet(" << this << ")::Insert(" << rName <<")");
  Idx index = mpSymbolTable->InsEntry(rName);
  TBaseSet<Idx>::Insert(index);
  return index;
}

// InsertSet(set)
void NameSet::InsertSet(const TBaseSet<Idx>& rOtherSet) {
#ifdef FAUDES_CHECKED
  const NameSet* nset = dynamic_cast<const NameSet*>(&rOtherSet);
  if(!nset) {
    std::stringstream errstr;
    errstr << "cannot cast to nameset" << std::endl;
    throw Exception("NameSet::InsertSet", errstr.str(), 67);
  }
  if(nset->mpSymbolTable!=mpSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismatch aka not implemented" << std::endl;
    throw Exception("NameSet::InsertSet", errstr.str(), 67);
  }
#endif
  TBaseSet<Idx>::InsertSet(rOtherSet);
}


// InsertSet(set)
void NameSet::InsertSet(const NameSet& rOtherSet) {
  FD_DC("NameSet(" << this << ")::InsertSet(" << rOtherSet.ToString() << ")");
#ifdef FAUDES_CHECKED
  if(rOtherSet.mpSymbolTable!=mpSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismatch aka not implemented" << std::endl;
    throw Exception("NameSet::InsertSet", errstr.str(), 67);
  }
#endif
  TBaseSet<Idx>::InsertSet(rOtherSet);
}
		
// Erase(index)
bool NameSet::Erase(const Idx& index) {
  FD_DC("NameSet(" << this << ")::Erase(" << index <<")");
  return TBaseSet<Idx>::Erase(index);
}

// Erase(rName)
bool NameSet::Erase(const std::string& rName) {
  FD_DC("NameSet(" << this << ")::Erase(" << rName <<")");
  Idx index = mpSymbolTable->Index(rName);
#ifdef FAUDES_CHECKED
  if (index == 0) {
    std::stringstream errstr;
    errstr << "name \"" << rName << "\" not found in NameSet" << std::endl;
    throw Exception("NameSet::Erase", errstr.str(), 66);
  }
#endif
  return TBaseSet<Idx>::Erase(index);
}

// Erase(pos)
NameSet::Iterator NameSet::Erase(const Iterator& pos) {
  return TBaseSet<Idx>::Erase(pos);
}

// EraseSet(set)
void NameSet::EraseSet(const TBaseSet<Idx>& rOtherSet) {
#ifdef FAUDES_CHECKED
  const NameSet* nset = dynamic_cast<const NameSet*>(&rOtherSet);
  if(!nset) {
    std::stringstream errstr;
    errstr << "cannot cast to nameset" << std::endl;
    throw Exception("NameSet::EraseSet", errstr.str(), 67);
  }
  if(nset->mpSymbolTable!=mpSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismatch aka not implemented" << std::endl;
    throw Exception("NameSet::EraseSet", errstr.str(), 67);
  }
#endif
  TBaseSet<Idx>::EraseSet(rOtherSet);
}


// EraseSet(set)
void NameSet::EraseSet(const NameSet& rOtherSet) {
#ifdef FAUDES_CHECKED
  if(rOtherSet.mpSymbolTable!=mpSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismatch aka not implemented" << std::endl;
    throw Exception("NameSet::EraseSet", errstr.str(), 67);
  }
#endif
  TBaseSet<Idx>::EraseSet(rOtherSet);
}


// RestrictSet(set)
void NameSet::RestrictSet(const TBaseSet<Idx>& rOtherSet) {
#ifdef FAUDES_CHECKED
  const NameSet* nset = dynamic_cast<const NameSet*>(&rOtherSet);
  if(!nset) {
    std::stringstream errstr;
    errstr << "cannot cast to nameset" << std::endl;
    throw Exception("NameSet::RestrictSet", errstr.str(), 67);
  }
  if(nset->mpSymbolTable!=mpSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismatch aka not implemented" << std::endl;
    throw Exception("NameSet::RestrictSet", errstr.str(), 67);
  }
#endif
  TBaseSet<Idx>::RestrictSet(rOtherSet);
}

// RestrictSet(set)
void NameSet::RestrictSet(const NameSet& rOtherSet) {
#ifdef FAUDES_CHECKED
  if(rOtherSet.mpSymbolTable!=mpSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismatch aka not implemented" << std::endl;
    throw Exception("NameSet::RestrictSet", errstr.str(), 67);
  }
#endif
  TBaseSet<Idx>::RestrictSet(rOtherSet);
}

// SymbolicName(index)
std::string NameSet::SymbolicName(Idx index) const {
  return mpSymbolTable->Symbol(index);
}

// SymbolicName(index, name)
void NameSet::SymbolicName(Idx index, const std::string& rName) {
  FD_DC("NameSet(" << this << ")::SymbolicName(" << index << ", " << rName <<")");
#ifdef FAUDES_CHECKED
  if (! Exists(index)) {
    std::stringstream errstr;
    errstr << "index " << index << " not in this set" << std::endl;
    throw Exception("NameSet::SymbolicName", errstr.str(), 60);
  }
#endif
  mpSymbolTable->SetEntry(index, rName);
}

// SymbolicName(name, name)
void NameSet::SymbolicName(const std::string& rName, 
				  const std::string& rNewName) {
  FD_DC("NameSet(" << this << ")::SymbolicName(" << rName << ", " 
	<< rNewName <<")");
#ifdef FAUDES_CHECKED
  if (! Exists(rName)) {
    std::stringstream errstr;
    errstr << "name \"" << rName << "\" not found in NameSet" << std::endl;
    throw Exception("NameSet::Symbolic", errstr.str(), 66);
  }
#endif
  mpSymbolTable->SetEntry(Index(rName), rNewName);
}

// Index(rName)
Idx NameSet::Index(const std::string& rName) const {
  return mpSymbolTable->Index(rName);
}

// Exists(index)
bool NameSet::Exists(const Idx& index) const {
  return TBaseSet<Idx>::Exists(index);
}

// Exists(rName)
bool NameSet::Exists(const std::string& rName) const {
  return TBaseSet<Idx>::Exists(mpSymbolTable->Index(rName)) ;
}

// Find(index) const
NameSet::Iterator NameSet::Find(const Idx& index) const {
  return TBaseSet<Idx>::Find(index);
}

// Find(rName) const
NameSet::Iterator NameSet::Find(const std::string& rName) const {
  return TBaseSet<Idx>::Find(mpSymbolTable->Index(rName));
}


// operator +
NameSet NameSet::operator + (const NameSet& rOtherSet) const {
  FD_DC("NameSet(" << this << ")::operator + (" << &rOtherSet << ")");
#ifdef FAUDES_CHECKED
  if(rOtherSet.mpSymbolTable!=mpSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismazch aka not implemented" << std::endl;
    throw Exception("NameSet::Operator+", errstr.str(), 67);
  }
#endif
  NameSet res;
  res.mpSymbolTable=mpSymbolTable;
  res.InsertSet(*this);
  res.InsertSet(rOtherSet);
  return res;
}

// operator -
NameSet NameSet::operator - (const NameSet& rOtherSet) const {
  FD_DC("NameSet(" << this << ")::operator - (" << &rOtherSet << ")");
#ifdef FAUDES_CHECKED
  if(rOtherSet.mpSymbolTable!=mpSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismazch aka not implemented" << std::endl;
    throw Exception("NameSet::Operator-", errstr.str(), 67);
  }
#endif
  NameSet res;
  res.mpSymbolTable=mpSymbolTable;
  res.InsertSet(*this);
  res.EraseSet(rOtherSet);
  return res;
}

     
// operator*
NameSet NameSet::operator * (const NameSet& rOtherSet) const {
  FD_DC("NameSet(" << this << ")::operator * (" << &rOtherSet << ")");
#ifdef FAUDES_CHECKED
  if(rOtherSet.mpSymbolTable!=mpSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismazch aka not implemented" << std::endl;
    throw Exception("NameSet::Operator*", errstr.str(), 67);
  }
#endif
  NameSet res;
  res.mpSymbolTable=mpSymbolTable;
  res.InsertSet(*this);
  res.RestrictSet(rOtherSet);
  return res;
}

// operator <=
bool NameSet::operator <= (const NameSet& rOtherSet) const {
#ifdef FAUDES_CHECKED
  if(rOtherSet.mpSymbolTable!=mpSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismazch aka not implemented" << std::endl;
    throw Exception("NameSet::Operator<=", errstr.str(), 67);
  }
#endif
  return TBaseSet<Idx>::operator <= (rOtherSet);
}

// operator >=
bool NameSet::operator >= (const NameSet& rOtherSet) const {
#ifdef FAUDES_CHECKED
  if(rOtherSet.mpSymbolTable!=mpSymbolTable) {
    std::stringstream errstr;
    errstr << "symboltable mismazch aka not implemented" << std::endl;
    throw Exception("NameSet::Operator>=", errstr.str(), 67);
  }
#endif
  return TBaseSet<Idx>::operator >= (rOtherSet);
}

// Str(index)
std::string NameSet::Str(const Idx& index) const {
  return mpSymbolTable->Symbol(index)+"#"+faudes::ToStringInteger(index);
}


/*
*********************************************************************************
*********************************************************************************
*********************************************************************************

 Misc

*********************************************************************************
*********************************************************************************
*********************************************************************************
*/


// RTI convenience function
void SetIntersection(const EventSetVector& rSetVec, EventSet& rRes) {
  // prepare result
  rRes.Clear();
  // ignore empty
  if(rSetVec.Size()==0) return;
  // copy first
  rRes.Assign(rSetVec.At(0));
  // perform intersecttion 
  for(EventSetVector::Position i=1; i<rSetVec.Size(); i++) 
    SetIntersection(rSetVec.At(i),rRes,rRes);
}


// RTI convenience function
void SetUnion(const EventSetVector& rSetVec, EventSet& rRes) {
  // prepare result
  rRes.Clear();
  // ignore empty
  if(rSetVec.Size()==0) return;
  // copy first
  rRes.Assign(rSetVec.At(0));
  // perform union
  for(EventSetVector::Position i=1; i<rSetVec.Size(); i++) 
    SetUnion(rSetVec.At(i),rRes,rRes);
}


} // namespace faudes


