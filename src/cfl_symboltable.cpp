/** @file cfl_symboltable.cpp @brief Class SymbolTable */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
   Copywrite (C) 2007, 2024 Thomas Moor
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


#include "cfl_symboltable.h"
#include <iostream>

namespace faudes {

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,SymbolTable,Type)

// Autoregister  for xml output
AutoRegisterType<SymbolTable> gRtiSymboltable("Symboltable");
  
// constructor
SymbolTable:: SymbolTable(void) :
  mMyName("SymbolTable"),
  mMaxIndex(std::numeric_limits<Idx>::max()), 
  mNextIndex(1) {
}

// constructor
SymbolTable:: SymbolTable(const SymbolTable& rSrc) {
  DoAssign(rSrc);
}

// asignment
void SymbolTable::DoAssign(const SymbolTable& rSrc) {
  mMyName=rSrc.mMyName;
  mIndexMap=rSrc.mIndexMap;
  mNameMap=rSrc.mNameMap;
  mMaxIndex=rSrc.mMaxIndex;
  mNextIndex=rSrc.mNextIndex;
}


// Name()
const std::string& SymbolTable::Name(void) const {
  return mMyName;
}
		
//Name(name)
void SymbolTable::Name(const std::string& rName) {
  mMyName = rName;
}

// Clear()
void SymbolTable::Clear(void) {   
  mMaxIndex=std::numeric_limits<Idx>::max();
  mNextIndex=1;
  mIndexMap.clear();
  mNameMap.clear();
}

// Size()
Idx SymbolTable::Size(void) const {   
  return mIndexMap.size();
}

// MaxIndex()
Idx SymbolTable::MaxIndex(void) const {
  return mMaxIndex;
}
		
// MaxIndex(index)
void SymbolTable::MaxIndex(Idx index) {
  if(index <= std::numeric_limits<Idx>::max()) {
    mMaxIndex = index;
    return;
  }
  std::stringstream errstr;
  errstr << "symboltable overflow in \"" << mMyName << "\"";
  throw Exception("SymbolTable::MaxIndex(inde))", errstr.str(), 40);
}
		
// LastIndex()
Idx SymbolTable::LastIndex(void) const {
  return mNextIndex - 1;
}
		
// ValidSymbol(rName)
bool SymbolTable::ValidSymbol(const std::string& rName)  {
  if(rName=="") return false;
  for(std::size_t cp=0;cp<rName.size();cp++) {
    char ch = rName[cp];
    if(iscntrl(ch)) return false;
    if(isspace(ch)) return false;
    if(ch=='"')    return false;
    if(ch=='#')    return false;
    if(isdigit(ch)) continue;
    if(isalpha(ch)) continue;
    if(isprint(ch)) continue;
    return false;
  }
  return true;
}

// UniqueSymbol(rName)
std::string SymbolTable::UniqueSymbol(const std::string& rName) const {
  if(!Exists(rName)) return (rName);
  long int count=0;
  std::string name=rName;
  std::string bname=rName;
  // if the name ends with extension "_123", remove the extension
  std::size_t up = bname.find_last_of("_");
  if(up != std::string::npos) {
    bool ad=true;
    for(std::size_t dp=up+1;dp<bname.size();dp++)
      if(!isdigit(bname[dp])) ad=false;
    if(ad) 
      bname.erase(up);
  }
  // try append extension "_123" until name is unique
  do {
    count++;
    name=bname + "_" + ToStringInteger(count);
  } while(Exists(name));
  return name;
}


// InsEntry(index, rName)
Idx SymbolTable::InsEntry(Idx index, const std::string& rName) {
  if( ! (index <= mMaxIndex)) {
    std::stringstream errstr;
    errstr << "symboltable overflow in \"" << mMyName << "\"";
    throw Exception("SymbolTable::InsEntry(index,name))", errstr.str(), 40);
  }
  Idx nidx=Index(rName);
  if((nidx!=0) && (nidx!=index)) {
    std::stringstream errstr;
    errstr << "Name " << rName << " allready exists in \"" << mMyName << "\"";
    throw Exception("SymbolTable::InsEntry(index,name)", errstr.str(), 41);
  }
  std::string idxname=Symbol(index);
  if((idxname != "") && (idxname != rName)) {
    std::stringstream errstr;
    errstr << "Index " << index << " allready exists in \"" << mMyName << "\"";
    throw Exception("SymbolTable::InsEntry(index,name)", errstr.str(), 42);
  }
  if(!ValidSymbol(rName)) {
    std::stringstream errstr;
    errstr << "Name " << rName << " is not a valid symbol";
    throw Exception("SymbolTable::InsEntry(index,name)", errstr.str(), 43);
  }

  mIndexMap[rName] = index;
  mNameMap[index] = rName;

  if(mNextIndex<=index) mNextIndex=index+1; 
  return index;
}

// InsEntry(rName)
Idx SymbolTable::InsEntry(const std::string& rName) {
  Idx index=Index(rName);
  if( index != 0) return index;
  return InsEntry(mNextIndex,rName);
}


// SetEntry(index, rName)
void SymbolTable::SetEntry(Idx index, const std::string& rName) {
  if(rName=="") {
    ClrEntry(index);
    return;
  }
  Idx nidx=Index(rName);
  if((nidx!=0) && (nidx!=index)) {
    std::stringstream errstr;
    errstr << "Name " << rName << " allready exists in \"" << mMyName << "\"";
    throw Exception("SymbolTable::SetEntry(index,name)", errstr.str(), 41);
  }
  if(!ValidSymbol(rName)) {
    std::stringstream errstr;
    errstr << "Name " << rName << " is not a valid symbol";
    throw Exception("SymbolTable::SetEntry(index,name)", errstr.str(), 43);
  }
  // remove old entry from reverse map
  std::map<Idx,std::string>::iterator nit = mNameMap.find(index);
  if(nit != mNameMap.end()) mIndexMap.erase(nit->second);
  // insert new name/index to maps
  mIndexMap[rName] = index;
  mNameMap[index] = rName;
}

// SetDefaultSymbol(index)
void SymbolTable::SetDefaultSymbol(Idx index) {
  ClrEntry(index); 
  std::string dname;
  dname = UniqueSymbol(ToStringInteger(index));
  InsEntry(index,dname);
}


// ClrEntry(index)
void SymbolTable::ClrEntry(Idx index) {
  std::map<Idx,std::string>::iterator it = mNameMap.find(index);
  if (it == mNameMap.end()) return;
  mIndexMap.erase(it->second);
  mNameMap.erase(it);
}

// ClrEntry(rName)
void SymbolTable::ClrEntry(const std::string& rName) {
  std::map<std::string,Idx>::iterator it = mIndexMap.find(rName);
  if (it == mIndexMap.end()) return;
  mNameMap.erase(it->second);
  mIndexMap.erase(it);
}

// RestrictDomain(set)
void SymbolTable::RestrictDomain(const IndexSet& rDomain) {
  // trivial cases
  if(rDomain.Empty()) { Clear(); return;}
  if(Size()==0) return;
  // lazy loop (todo: sorted iterate)
  std::map<Idx,std::string>::iterator it, oit;
  it = mNameMap.begin();
  while(it!=mNameMap.end()) {
    oit=it;
    it++;
    if(!rDomain.Exists(oit->first)) mNameMap.erase(oit);
  }
}

// Index(rName)
Idx SymbolTable::Index(const std::string& rName) const {
  std::map<std::string,Idx>::const_iterator it = mIndexMap.find(rName);
  if (it == mIndexMap.end()) 
    return 0;
  else 
    return it->second;
}

// Symbol(index)
std::string SymbolTable::Symbol(Idx index) const {
  std::map<Idx,std::string>::const_iterator it = mNameMap.find(index);
  if (it == mNameMap.end()) 
    return "";
  else 
    return it->second;
}



// Exists(index)
bool SymbolTable::Exists(Idx index) const {
  std::map<Idx,std::string>::const_iterator it = mNameMap.find(index);
  return (it != mNameMap.end());
}

// Exists(rName)
bool SymbolTable::Exists(const std::string& rName) const {
  std::map<std::string,Idx>::const_iterator it = mIndexMap.find(rName);
  return ( it != mIndexMap.end());
}


// GlobalEventSymbolTablep 
// (initialize on first use pattern)
SymbolTable* SymbolTable::GlobalEventSymbolTablep(void) {
  static SymbolTable fls;
  return &fls; 
}


//DoWrite(rTr,rLabel,pContext)
void SymbolTable::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const 
{
  // default label
  std::string label=rLabel;
  if(label=="") label="SymbolTable"; 

  // write section
  rTw.WriteBegin(label);
 
  // have two columns
  int ocol= rTw.Columns();
  rTw.Columns(2);

  // iterate over indices
  std::map< std::string, Idx >::const_iterator sit = mIndexMap.begin();
  for(; sit != mIndexMap.end();++sit) {
    rTw.WriteInteger(sit->second);
    rTw.WriteString(sit->first);
  }
  
  // restore columns
  rTw.Columns(ocol);

  // end section
  rTw.WriteEnd(label);
}


//DoRead(rTr,rLabel,pContext)
void SymbolTable::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {

  // default label
  std::string label=rLabel;
  if(label=="") label="SymbolTable"; 
  
  // read section
  rTr.ReadBegin(label);

  // iterate section  
  while(!rTr.Eos(label)) {
    Idx idx = rTr.ReadInteger();
    std::string sym= rTr.ReadString();
    InsEntry(idx,sym);
  }

  // done
  rTr.ReadEnd(label);
}




} // namespace
