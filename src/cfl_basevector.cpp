/** @file cfl_basevector.cpp @brief  */

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

#include "cfl_basevector.h"
#include "cfl_nameset.h"

namespace faudes {


/*
******************************************************************************************
******************************************************************************************
******************************************************************************************

Implementation of TBaseVector

******************************************************************************************
******************************************************************************************
******************************************************************************************
*/

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,vBaseVector,Type)


// vBaseVector()
vBaseVector::vBaseVector(void) :
  Type()
{
  FD_DC("vBaseVector(" << this << ")::vBaseVector()");
  // my members
  mMyName="Vector";
}

  
// vBaseVector(filename)
vBaseVector::vBaseVector(const std::string& rFileName, const std::string& rLabel)  :
  Type()
{
  FD_DC("vBaseVector(" << this << ")::vBaseVector()");
  // other members
  mMyName="Vector";
  // do read;
  Read(rFileName,rLabel);  
}


// vBaseVector(rOtherSet)
vBaseVector::vBaseVector(const vBaseVector& rOtherVector) : 
  Type(rOtherVector)
{
  FD_DC("vBaseVector(" << this << ")::vBaseVector(rOtherVector " << &rOtherVector << "): copy construct");
  DoAssign(rOtherVector);
}

// destructor
vBaseVector::~vBaseVector(void) {
  FD_DC("vBaseVector(" << this << ")::~vBaseVector()");
  // delete entries
  for(Position pos=0; pos<mVector.size(); pos++) 
    if(mVector[pos].mMine) delete mVector[pos].pElement;
  // done
  FD_DC("vBaseVector(" << this << ")::~vBaseVector(): done");
}


// element prototype
const Type* vBaseVector::Elementp(void) const {
  static Type proto;
  return &proto;
}

// element prototype
const Type& vBaseVector::Element(void) const {
  return *Elementp();
}

// element factory
Type* vBaseVector::NewElement(void) {
  return Element().New();
}

// test element type
bool vBaseVector::ElementTry(const Type& rElement) const {
  return Elementp()->Cast(&rElement)!=NULL;
}

// assignment (here, we know the type to match)
void vBaseVector::DoAssign(const vBaseVector& rSourceVector) {
  FD_DC("vBaseVector(" << this << ")::DoAssign(rOtherVector " << &rSourceVector << ")");
  // bail out on selfref
  if(this==&rSourceVector) return;
  // virtual clear
  Clear();
  // allocate
  mVector.resize(rSourceVector.Size());
  // copy entries
  for(Position pos=0; pos<mVector.size(); pos++) {
    mVector[pos].pElement = rSourceVector.mVector[pos].pElement->Copy();
    mVector[pos].mMine=true;
    mVector[pos].mFileName="";
  }
  // done
  FD_DC("vBaseVector(" << this << ")::DoAssign(rOtherVector " << &rSourceVector << "): done");
}

// clear
void vBaseVector::Clear(void) {
  // delete entries
  for(Position pos=0; pos<mVector.size(); pos++) {
    if(mVector[pos].mMine) delete mVector[pos].pElement;
  }
  // resize
  mVector.resize(0);
}


// Name
const std::string& vBaseVector::Name(void) const {
  return mMyName;
}
		
// Name
void vBaseVector::Name(const std::string& rName) {
  mMyName = rName;
}
  
// Size()
Idx vBaseVector::Size(void) const {
  return (Idx) mVector.size();
}

// Size(idx)
void vBaseVector::Size(Idx len) {
  FD_DC("vBaseVector(" << this << ")::Size(..): from " << mVector.size() << " to " << len);
  // record
  Position olen=mVector.size();
  // delete
  for(Position pos=len; pos<olen; pos++) 
    if(mVector[pos].mMine) delete mVector[pos].pElement;
  // allocate
  mVector.resize(len);
  // initialize
  for(Position pos=olen; pos < len; pos++) {
    mVector[pos].pElement = NewElement();
    mVector[pos].mMine=true;
    mVector[pos].mFileName = "";
  }
  // done
  FD_DC("vBaseVector(" << this << ")::Size(..): done");
}


// Empty()
bool vBaseVector::Empty(void) const {
  return mVector.empty();
}


// At()
const Type& vBaseVector::At(const Position& pos) const {
#ifdef FAUDES_CHECKED
  if(pos >= mVector.size()) {
    std::stringstream errstr;
    errstr << "index out of range" << std::endl;
    throw Exception("vBaseVector::At", errstr.str(), 62);
  }
#endif
  return *mVector[pos].pElement;
}

// At()
Type& vBaseVector::At(const Position& pos) {
#ifdef FAUDES_CHECKED
  if(pos >= mVector.size()) {
    std::stringstream errstr;
    errstr << "index out of range" << std::endl;
    throw Exception("vBaseVector::At", errstr.str(), 62);
  }
#endif
  return *mVector[pos].pElement;
}



// replace (copy)
void vBaseVector::Replace(const Position& pos, const Type& rElem) {
#ifdef FAUDES_CHECKED
  if(pos >= mVector.size()) {
    std::stringstream errstr;
    errstr << "index out of range" << std::endl;
    throw Exception("vBaseVector::At", errstr.str(), 62);
  }
#endif
  if(!ElementTry(rElem)) {
    std::stringstream errstr;
    errstr << "cannot cast element " << std::endl;
    throw Exception("vBaseVector::Replace(pos,elem)", errstr.str(), 63);
  }
  iterator pit=mVector.begin()+pos;
  if(pit->mMine) delete pit->pElement;
  pit->pElement=rElem.Copy();
  pit->mMine=true;
  pit->mFileName="";
}

// replace (take)
void vBaseVector::Replace(const Position& pos, Type* pElem) {
#ifdef FAUDES_CHECKED
  if(pos >= mVector.size()) {
    std::stringstream errstr;
    errstr << "index out of range" << std::endl;
    throw Exception("vBaseVector::At", errstr.str(), 62);
  }
#endif
  if(!ElementTry(*pElem)) {
    std::stringstream errstr;
    errstr << "cannot cast element " << std::endl;
    throw Exception("vBaseVector::Replace(pos,elem)", errstr.str(), 63);
  }
  iterator pit=mVector.begin()+pos;
  if(pit->mMine) delete pit->pElement;
  pit->pElement=pElem;
  pit->mMine=false;
  pit->mFileName="";
}

// replace (file)
void vBaseVector::Replace(const Position& pos, const std::string& rFileName) {
#ifdef FAUDES_CHECKED
  if(pos >= mVector.size()) {
    std::stringstream errstr;
    errstr << "index out of range" << std::endl;
    throw Exception("vBaseVector::At", errstr.str(), 62);
  }
#endif
  iterator pit=mVector.begin()+pos;
  if(pit->mMine) delete pit->pElement;
  pit->pElement = NewElement();
  pit->mMine=true;
  pit->pElement->Read(rFileName);
  pit->mFileName=rFileName;
}

// erase
void vBaseVector::Erase(const Position& pos) {
#ifdef FAUDES_CHECKED
  if(pos >= mVector.size()) {
    std::stringstream errstr;
    errstr << "index out of range" << std::endl;
    throw Exception("vBaseVector::At", errstr.str(), 62);
  }
#endif
  iterator pit=mVector.begin()+pos;
  if(pit->mMine) delete pit->pElement;
  mVector.erase(pit);
}


// insert (copy)
void vBaseVector::Insert(const Position& pos, const Type& rElem) {
#ifdef FAUDES_CHECKED
  if(pos > mVector.size()) {
    std::stringstream errstr;
    errstr << "index out of range" << std::endl;
    throw Exception("vBaseVector::At", errstr.str(), 62);
  }
#endif
  if(!ElementTry(rElem)) {
    std::stringstream errstr;
    errstr << "cannot cast element " << std::endl;
    throw Exception("vBaseVector::Insert(pos,elem)", errstr.str(), 63);
  }
  ElementRecord elem;
  elem.pElement = rElem.Copy();
  elem.mMine=true;
  elem.mFileName="";
  iterator pit=mVector.begin()+pos;
  mVector.insert(pit,elem);
}
 
// insert (take)
void vBaseVector::Insert(const Position& pos, Type* pElem) {
#ifdef FAUDES_CHECKED
  if(pos > mVector.size()) {
    std::stringstream errstr;
    errstr << "index out of range" << std::endl;
    throw Exception("vBaseVector::At", errstr.str(), 62);
  }
#endif
  if(!ElementTry(*pElem)) {
    std::stringstream errstr;
    errstr << "cannot cast element " << std::endl;
    throw Exception("vBaseVector::Insert(pos,elem)", errstr.str(), 63);
  }
  ElementRecord elem;
  elem.pElement = pElem;
  elem.mMine=false;
  elem.mFileName="";
  iterator pit=mVector.begin()+pos;
  mVector.insert(pit,elem);
}
 
// insert (file)
void vBaseVector::Insert(const Position& pos, const std::string& rFileName) {
#ifdef FAUDES_CHECKED
  if(pos > mVector.size()) {
    std::stringstream errstr;
    errstr << "index out of range" << std::endl;
    throw Exception("vBaseVector::At", errstr.str(), 62);
  }
#endif
  ElementRecord elem;
  elem.pElement =NewElement();
  elem.mMine=true;
  elem.pElement->Read(rFileName);
  elem.mFileName=rFileName;
  iterator pit=mVector.begin()+pos;
  mVector.insert(pit,elem);
}
 

// append (copy)
void vBaseVector::PushBack(const Type& rElem) {
  if(!ElementTry(rElem)) {
    std::stringstream errstr;
    errstr << "cannot cast element " << std::endl;
    throw Exception("vBaseVector::PushBack(elem)", errstr.str(), 63);
  }
  ElementRecord elem;
  elem.pElement = rElem.Copy();
  elem.mMine=true;
  elem.mFileName="";
  mVector.push_back(elem);
}
 
// append (take)
void vBaseVector::PushBack(Type* pElem) {
  if(!ElementTry(*pElem)) {
    std::stringstream errstr;
    errstr << "cannot cast element " << std::endl;
    throw Exception("vBaseVector::PushBack(elem)", errstr.str(), 63);
  }
  ElementRecord elem;
  elem.pElement = pElem;
  elem.mMine=false;
  elem.mFileName="";
  mVector.push_back(elem);
}
 
// append (file)
 void vBaseVector::PushBack(const std::string& rFileName) {
  ElementRecord elem;
  elem.pElement = NewElement();
  elem.mMine=true;
  elem.pElement->Read(rFileName);
  elem.mFileName=rFileName;
  mVector.push_back(elem);
}
 

// append (copy)
void vBaseVector::Append(const Type& rElem) {
  PushBack(rElem);
}

// append (take)
void vBaseVector::Append(Type* pElem) {
  PushBack(pElem);
}

// append (file)
void vBaseVector::Append(const std::string& rFileName) {
  PushBack(rFileName);
}
 

// FilenameAt()
const std::string& vBaseVector::FilenameAt(const Position& pos) const {
#ifdef FAUDES_CHECKED
  if(pos >= mVector.size()) {
    std::stringstream errstr;
    errstr << "index out of range" << std::endl;
    throw Exception("vBaseVector::FilenameAt", errstr.str(), 62);
  }
#endif
  return mVector[pos].mFileName;
}

// FilenameAt()
void vBaseVector::FilenameAt(const Position& pos, const std::string& rFileName) {
#ifdef FAUDES_CHECKED
  if(pos >= mVector.size()) {
    std::stringstream errstr;
    errstr << "index out of range" << std::endl;
    throw Exception("vBaseVector::FilenameAt", errstr.str(), 62);
  }
#endif
  mVector[pos].mFileName = rFileName;
}

// take ownership
void vBaseVector::TakeCopies(void) {
  iterator pit=mVector.begin();
  for(;pit!=mVector.end();++pit) {
    if(pit->mMine) continue;
    pit->pElement=pit->pElement->Copy();
    pit->mMine=true;
  }
}

// take ownership
void vBaseVector::TakeOwnership(void) {
  iterator pit=mVector.begin();
  for(;pit!=mVector.end();++pit) 
    pit->mMine=true;
}

// DoWrite(tw, label, context)
void vBaseVector::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  // figure whether we write individual files
  bool ifiles=rTw.FileMode();
  for(Position pos=0; pos<mVector.size() && ifiles; pos++) 
    if(mVector[pos].mFileName=="") ifiles=false;
  // extract base directory
  std::string dirname="";
  if(rTw.FileMode()) 
    dirname = ExtractDirectory(rTw.FileName());
  // have a section
  Token btag=XBeginTag(rLabel,"Vector");
  FD_DC("vBaseVector(" << this << ")::DoWrite(..):  #" << Size());
  rTw.Write(btag);
  // loop entries
  for(Position pos=0; pos<mVector.size(); pos++) {
    // just stream tokens
    if(!ifiles) {
      mVector[pos].pElement->Write(rTw,"",pContext);
      continue;
    } 
    // write individual files
    std::string filename= ExtractFilename(mVector[pos].mFileName);
    rTw.WriteString(filename);
    mVector[pos].pElement->Write(PrependPath(dirname,filename),"",pContext);    
  }
  rTw.WriteEnd(btag.StringValue());
}


// DoDWrite(tw,rLabel,context)
void vBaseVector::DoDWrite(TokenWriter& rTw,const std::string& rLabel, const Type* pContext) const {
  // have a section
  Token btag=XBeginTag(rLabel,"Vector");
  FD_DC("vBaseVector(" << this << ")::DoWrite(..):  #" << Size());
  rTw.Write(btag);
  for(Position pos=0; pos<mVector.size(); pos++) {
    mVector[pos].pElement->DWrite(rTw,"",pContext);
  }
  rTw.WriteEnd(btag.StringValue());
}


// DoSWrite(tw)
void vBaseVector::DoSWrite(TokenWriter& rTw) const {
  FD_DC("vBaseVector(" << this << ")::DoSWrite(..)");
  rTw.WriteComment(" Vector Size: "+ ToStringInteger(Size()));
  for(Position pos=0; pos<mVector.size(); pos++) {
    rTw.WriteComment(" Vector Entry " + ToStringInteger(pos));
    mVector[pos].pElement->SWrite(rTw);
  }
  rTw.WriteComment("");
}


// DoRead(rTr, rLabel, pContext)
void vBaseVector::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  //prepare token
  Token token;
  //prepare string 
  std::string filename = "";      
  std::string dirname = "";       
  std::string path;               
  // have default section
  std::string label=rLabel;
  if(label=="") label="Vector"; 
  Name(label);
  rTr.ReadBegin(label); 
  // fill my entries from token stream
  while(!rTr.Eos(label)){
    //peek token
    rTr.Peek(token);
    // if Token is a String we assume its the name of a file containing a device
    if(token.Type()==Token::String) {
      //read Token
      rTr.Get(token);
      // read relative filename
      filename = token.StringValue();
      // build up path to base-file
      if(rTr.SourceMode()==TokenReader::File) dirname = ExtractDirectory(rTr.FileName());
      //build up path to specified file
      path = dirname.append(filename);
      //insert device
      Insert(mVector.size(),path);
      continue;
    }
    // if its not a file it has to be an entry
    else if(token.Type()==Token::Begin) {
      // prepare
      Type* elemp = NewElement();
      // read entry
      elemp->Read(rTr);
      // insert device mDevices
      Insert(mVector.size(),elemp);
      // fix ownership
      (--mVector.end())->mMine=true;
      continue;
    }
    // token mismatch
    std::stringstream errstr;
    errstr << "token mismatch" << std::endl;
    throw Exception("vBaseVector::At", errstr.str(), 50);
  } 
  // done
  rTr.ReadEnd(label); 
}




} // namespace faudes

