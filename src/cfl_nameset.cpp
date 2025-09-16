/** @file cfl_nameset.cpp @brief Classes NameSet */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
   Copyright (C) 2007,2025 Thomas Moor.
   
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
  mElementTagDef="Event";
  Name("NameSet");
  FD_DC("NameSet("<<this<<")::NameSet() with symtab "<< mpSymbolTable); 
}

// constructor from nameset
NameSet::NameSet(const NameSet& rOtherSet) : TBaseSet<Idx>() {
  FD_DC("NameSet(" << this << ")::NameSet(rOtherSet " << &rOtherSet << ")");
  mpSymbolTable = rOtherSet.mpSymbolTable;
  mElementTagDef="Event";
  Assign(rOtherSet);
  FD_DC("NameSet(" << this << ")::NameSet(rOtherSet " << &rOtherSet << "): done");
}

// read file constructor
NameSet::NameSet(const std::string& rFilename, const std::string& rLabel) : TBaseSet<Idx>() {
  FD_DC("NameSet(" << this << ")::NameSet(" << rFilename << ")");
  mpSymbolTable = SymbolTable::GlobalEventSymbolTablep();
  mElementTagDef="Event";
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
  FD_DC("NameSet(" << this << ")::DoAssign(from " << &rSourceSet <<")");
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
  if(label=="") label=TypeName();  
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
  std::string etstr=ElementTag();
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
  std::string etstr=ElementTag();
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

// Str()
std::string NameSet::Str(void) const {
  return TBaseSet<Idx>::Str();
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


/*
*********************************************************************************
*********************************************************************************
*********************************************************************************

 RelabelMap 

*********************************************************************************
*********************************************************************************
*********************************************************************************
*/

// register with RTI  
AutoRegisterType<RelabelMap> gRtiRelabelMap("RelabelMap");
AutoRegisterElementType<RelabelMap> gRtiRelabelMapEType("RelabelMap","EventSet");
AutoRegisterElementTag<RelabelMap> gRtiRelabelMapETag("RelabelMap","Label");


// std faudes type
FAUDES_TYPE_IMPLEMENTATION_NEW(RelabelMap,RelabelMap,TaNameSet<NameSet>) 
FAUDES_TYPE_IMPLEMENTATION_COPY(RelabelMap,RelabelMap,TaNameSet<NameSet>) 
FAUDES_TYPE_IMPLEMENTATION_CAST(RelabelMap,RelabelMap,TaNameSet<NameSet>) 
FAUDES_TYPE_IMPLEMENTATION_ASSIGN(RelabelMap,RelabelMap,TaNameSet<NameSet>)
FAUDES_TYPE_IMPLEMENTATION_EQUAL(RelabelMap,RelabelMap,TaNameSet<NameSet>)


// empty constructor
RelabelMap::RelabelMap(void) : TaNameSet<NameSet>(){
  FD_DC("RelabelMap("<<this<<")::RelabelMap() with symtab "<< mpSymbolTable);
  mObjectName="RelabelMap";
  Lock();
}

// constructor from RelabelMap
RelabelMap::RelabelMap(const RelabelMap& rOtherMap) : TaNameSet<NameSet>() {
  FD_DC("RelabelMap(" << this << ")::RelabelMap(rOtherMap " << &rOtherMap << ")");
  Assign(rOtherMap);
  FD_DC("RelabelMap(" << this << ")::RelabelMap(rOtherSet " << &rOtherSet << "): done");
  Lock();
}

// read file constructor
RelabelMap::RelabelMap(const std::string& rFilename, const std::string& rLabel) : TaNameSet<NameSet>() {
  FD_DC("RelabelMap(" << this << ")::RelabelMap(" << rFilename << ")");
  Read(rFilename, rLabel);
  Lock();
}

// destructor
RelabelMap::~RelabelMap(void) {
  FD_DC("RelabelMap("<<this<<")::~RelabelMap()");
}

// extra accessors
bool RelabelMap::Insert(const Idx& rSrc, const Idx& rDst) {
  Insert(rSrc);
  return Attributep(rSrc)->Insert(rDst);
}

// extra accessors
bool RelabelMap::Insert(const std::string& rSrc, const std::string& rDst) {
  Insert(rSrc);
  return Attributep(rSrc)->Insert(rDst);
}
  
// extra accessors
const NameSet& RelabelMap::Target(const Idx& rSrc) const {
  return Attribute(rSrc);
}

// extra accessors
NameSet& RelabelMap::Target(const Idx& rSrc) {
  return *Attributep(rSrc);
}
  
// extra accessors
const NameSet& RelabelMap::Target(const std::string& rSrc) const {
  return Attribute(rSrc);
}
  

// extra accessors
NameSet& RelabelMap::Target(const std::string& rSrc) {
  return *Attributep(rSrc);
}

// extra accessors
void RelabelMap::Target(const std::string& rSrc, const NameSet& rTarget) {
  return Attribute(rSrc,rTarget);
}
  

// extra accessors
void RelabelMap::Target(const Idx& rSrc, const NameSet& rTarget) {
    return Attribute(rSrc,rTarget);
}

// read STL
void RelabelMap::FromStlMap(const std::map<Idx, std::set<Idx> >& rMap) {
  Clear();
  NameSet target;
  std::map<Idx, std::set<Idx> >::const_iterator mit;
  mit=rMap.begin();
  while(mit!=rMap.end()) {
    target.FromStl(mit->second);
    TAttrMap<Idx,NameSet>::Insert(mit->first,target);
    ++mit;
  }
}

// write STL
void RelabelMap::ToStlMap(std::map<Idx, std::set<Idx> >& rMap) const {
  rMap.clear();
  std::set<Idx> target;
  Iterator it=Begin();
  while(it!=End()) {
    Attribute(*it).ToStl(target);
    rMap[*it]=target;
    ++it;
  }
}

// pretty print
std::string RelabelMap::Str(void) const { 
  std::stringstream str;
  NameSet::Iterator tit, tit_end;
  NameSet::Iterator dit=Begin();
  NameSet::Iterator dit_end=End();
  while(true) {
    str << Str(*dit) << " -> { ";
    const NameSet& target=Target(*dit);
    tit=target.Begin();
    tit_end=target.End();
    for(;tit!=tit_end;++tit) 
      str << NameSet::Str(*tit) << " ";
    str << "}";
    ++dit;
    if(dit==dit_end) break;
    str << std::endl;
  }
  return str.str();
}
  
  
  
// apply the map to a NameSet 
void ApplyRelabelMap(const RelabelMap& rMap, const NameSet& rSet, NameSet& rRes) {
#ifdef FAUDES_CHECKED
  // test for matching symboltables
  if(rMap.SymbolTablep()!=rSet.SymbolTablep()) {
    std::stringstream errstr;
    errstr << "symboltable mismatch aka not implemented" << std::endl;
    throw Exception("ApplyRelableMap", errstr.str(), 67);
  }
#endif
  // set up effective domain and range
  /*
  NameSet domain, range;
  NameSet::Iterator dit=rSet.Begin()
  NameSet::Iterator dit_end=rSet.End()
  for(;dit=dit_end;++dit) {
    if(!rMat.Exists(*dit)) continue;
    domain.Insert(*dit);
    range.InsertSet(rMap.Target(*dit));
  }
  */
  // have intermediate targets
  /*
  SymbolTable& symtab = *rSet.SymbolTablep();
  std::map<Idx,Idx> target2alt;
  std::map<Idx,Idx> alt2target;
  NameSet::Iterator rit=range.Begin()
  NameSet::Iterator rit_end=range.End()
  for(;rit=rit_end;++rit) {
    std::string  altstr=symtab.UniqueSymbol("TMP_1");
    Idx altidx = symtab.InsEntry(altstr);
    target2alt[*rit]=altidx;
    alt2target[altidx]=*rit;
  }
  */
  NameSet& inselem = *rSet.New();
  NameSet delelem;
  NameSet::Iterator tit, tit_end;
  NameSet::Iterator dit=rSet.Begin();
  NameSet::Iterator dit_end=rSet.End();
  for(;dit!=dit_end;++dit) {
    if(!rMap.Exists(*dit)) continue;
    delelem.Insert(*dit);
    const NameSet& target=rMap.Target(*dit);
    AttributeVoid* attrp = rSet.Attribute(*dit).Copy();
    tit=target.Begin();
    tit_end=target.End();
    for(;tit!=tit_end;++tit) {
      inselem.Insert(*tit);
      inselem.Attribute(*tit,*attrp);
    }
    delete attrp;      
  }
  rRes.Assign(rSet);
  rRes.EraseSet(delelem);
  rRes.InsertSet(inselem);
  delete &inselem;
}

  

} // namespace faudes



