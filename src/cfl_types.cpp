/** @file cfl_types.cpp Runtime interface, faudes types */

/* FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2009 Ruediger Berndt
Copyright (C) 2010 Thomas Moor

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


#include "cfl_types.h"
#include "cfl_registry.h"

namespace faudes{

/*
********************************************************************
********************************************************************
********************************************************************

implemantation of faudes Type

********************************************************************
********************************************************************
********************************************************************
*/

// statics
std::string Type::msStringVoid("Void");
std::string Type::msStringEmpty("");

// constructor
Type::Type(void)  {}

// copy constructor
Type::Type(const Type& rType)  {(void) rType;}

// destructor
Type::~Type(void) {}

// pointer constructor
Type* Type::New(void) const { 
  FD_WARN("Type(" << this << ")::New(): not reimplemented for " << typeid(*this).name());
  return new Type(); 
}

// pointer copy constructor
Type* Type::Copy(void) const { 
  FD_WARN("Type(" << this << ")::Copy(): not reimplemented for " << typeid(*this).name());
  return new Type(*this); 
}

// cast
const Type* Type::Cast(const Type* pOther) const {
  FD_WARN("Type(" << this << ")::Cast(" << pOther << "): not reimplemented for " << typeid(*this).name());
  return pOther;
}

// Clear()
void Type::Clear(void) {
  FD_DC("Type::Clear(): not re-implemented in " << typeid(*this).name());
}

// assign
Type& Type::Assign(const Type& rSource) {
  FD_DC("Type(" << this << ")::Assign(" << &rSource << ")");
  Clear();
  return *this;
}

// equality (relaxed)
bool Type::Equal(const Type& rOther) const { 
  return true;
}   

// equality (strict)
bool Type::operator==(const Type& rOther) const { 
  return DoEqual(rOther);
}

// equality
bool Type::operator!=(const Type& rOther) const { 
  return !DoEqual(rOther);
}

// assign
Type& Type::operator=(const Type& rSource) {
  FD_DC("Type(" << this << ")::AssignementOperator(" << &rSource << ")");
  Clear();
  DoAssign(rSource);
  return *this;
}

// assign
void Type::DoAssign(const Type& rSource) {
  FD_DC("Type(" << this << ")::DoAssign(" << &rSource << ")");
}

// equality
bool Type::DoEqual(const Type& rOther) const {
  //typeid(*this)==typeid(rOther); TM 2025
  return true;
}

// Name(rName)
void Type::Name(const std::string& rName) {
  (void) rName;
}

// Name()
const std::string& Type::Name(void) const {
  return msStringEmpty;
}

// TypeDefinitionp()
const TypeDefinition* Type::TypeDefinitionp(void) const {
  return TypeRegistry::G()->Definitionp(*this);
}

// TypeName()
const std::string& Type::TypeName(void) const {
  const TypeDefinition* fdp=TypeDefinitionp();
  if(!fdp) return msStringEmpty;
  return fdp->Name();
}

// Write(context)
void Type::Write(const Type* pContext) const {
  TokenWriter tw(TokenWriter::Stdout);
  DoWrite(tw,"",pContext);
}

// Write(rFileName, label, context, openmode)
void Type::Write(const std::string& rFileName, const std::string& rLabel, 
		 const Type* pContext, std::ios::openmode openmode) const {
  try {
    TokenWriter tw(rFileName, openmode);
    DoWrite(tw, rLabel, pContext);
  }
  catch(std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << rFileName << "\"";
    throw Exception("Type::Write", errstr.str(), 2);
  }
}

// Write(rFileName, openmode)
void Type::Write(const std::string& rFileName, std::ios::openmode openmode) const {
  Write(rFileName,"",0,openmode);
}

// Write(tw,label,context)
void Type::Write(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  DoWrite(rTw,rLabel,pContext);
}

// ToString(label, context)
std::string Type::ToString(const std::string& rLabel, const Type* pContext) const {
  TokenWriter tw(TokenWriter::String);
  tw.Endl(false);
  DoWrite(tw,rLabel,pContext);
  return tw.Str();
}

// ToSText()
std::string Type::ToSText(void) const {
  TokenWriter tw(TokenWriter::String);
  tw.Endl(true);
  DoSWrite(tw);
  return tw.Str();
}

// ToString(label, context)
std::string Type::ToText(const std::string& rLabel, const Type* pContext) const {
  TokenWriter tw(TokenWriter::String);
  tw.Endl(true);
  DoWrite(tw,rLabel,pContext);
  return tw.Str();
}

// XWrite(context)
void Type::XWrite(const Type* pContext) const {
  FD_DC("Type(" << this << ")::XWrite(): for " << typeid(*this).name());
  TokenWriter tw(TokenWriter::Stdout);
  DoXWrite(tw,"",pContext);
}

// XWrite(rFileName, label, context)
void Type::XWrite(const std::string& rFileName, const std::string& rLabel, const Type* pContext) const {
  std::string ftype=TypeName();
  if(ftype=="") {
    std::stringstream errstr;
    errstr << "Cannot write unregistered faudes-type object \"" << typeid(*this).name() << "\"";
    throw Exception("Type::XWrite", errstr.str(), 2);
  }
  try {
    TokenWriter tw(rFileName,ftype);
    XWrite(tw, rLabel, pContext);
  }
  catch(std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << rFileName << "\"";
    throw Exception("Type::XWrite", errstr.str(), 2);
  }
}

// XWrite(tw,label,context)
void Type::XWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  FD_DC("Type(" << this << ")::XWrrite(): for " << typeid(*this).name());
  DoXWrite(rTw,rLabel,pContext);
}

// DWrite(context)
void Type::DWrite(const Type* pContext) const {
  TokenWriter tw(TokenWriter::Stdout);
  DoDWrite(tw,"",pContext);
}

// DWrite(rFileName, label, context, openmode)
void Type::DWrite(const std::string& rFileName, const std::string& rLabel, 
		  const Type* pContext, std::ios::openmode openmode) const {
  try {
    TokenWriter tw(rFileName, openmode);
    DoDWrite(tw, rLabel, pContext);
  }
  catch(std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << rFileName << "\"";
    throw Exception("Type::DWrite", errstr.str(), 2);
  }
}

// DWrite(tw,label,context)
void Type::DWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  DoDWrite(rTw,rLabel,pContext);
}

// SWrite()
void Type::SWrite(void) const {
  TokenWriter tw(TokenWriter::Stdout);
  DoSWrite(tw);
}

// SWrite(tw)
void Type::SWrite(TokenWriter& rTw) const {
  DoSWrite(rTw);
}

// Read(rFilename, rLabel,context)
void Type::Read(const std::string& rFilename, const std::string& rLabel, const Type* pContext) {
  Clear();
  TokenReader tr(rFilename);
  if(rLabel!="") tr.SeekBegin(rLabel);
  DoRead(tr,rLabel,pContext);
}

// Read(rTr, rLabel, context)
void Type::Read(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  Clear();
  DoRead(rTr,rLabel,pContext);
}

// FromString(rString, rLabel, context)
void Type::FromString(const std::string& rString, const std::string& rLabel, const Type* pContext) {
  Clear();
  TokenReader tr(TokenReader::String,rString);
  DoRead(tr,rLabel,pContext);
}
  
//DoWrite(rTr,rLabel,pContext)
void Type::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) rTw; (void) rLabel; (void) pContext;
  FD_DC("Type::DoWrite(): not re-implemented in " << typeid(*this).name());
}

//DoXWrite(rTr,rLabel,pContext)
void Type::DoXWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  DoWrite(rTw, rLabel, pContext);
}

//DoDWrite(rTr,rLabel,pContext)
void Type::DoDWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  FD_DC("Type::DoDWrite(): not re-implemented in " << typeid(*this).name() << ", using DoDWrite instead");
  DoWrite(rTw,rLabel,pContext);
}

//DoSWrite(rTr,rLabel,pContext)
void Type::DoSWrite(TokenWriter& rTw) const {
  FD_DC("Type::DoSWrite(): not re-implemented in " << typeid(*this).name());
  rTw.WriteComment("");
  rTw.WriteComment(" Statistics for " + Name());
  rTw.WriteComment("");
}

//DoRead(rTr,rLabel,pContext)
void Type::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  (void) rLabel; (void) pContext; (void) rTr;
  FD_DC("Type::DoRead(): not re-implemented in " << typeid(*this).name());
}



// configure begin token
Token Type::XBeginTag(const std::string& rLabel,const std::string& rFallbackLabel) const {
  // provided
  std::string label=rLabel;
  std::string ftype=TypeName();
  // fallback label 
  if(label=="") label=ftype;
  if(label=="") label=rFallbackLabel;
  Token btag;
  btag.SetBegin(label);
  if(Name()!=label && Name()!="") btag.InsAttributeString("name",Name());
  if(ftype!=label && ftype!="") btag.InsAttributeString("ftype",ftype);
  // done
  return btag;
}



/*
********************************************************************
********************************************************************
********************************************************************

Implementation of class Documentation

********************************************************************
********************************************************************
********************************************************************
*/

// faudes type (cannot do autoregister)
FAUDES_TYPE_IMPLEMENTATION_NEW(Void,Documentation,Type)
FAUDES_TYPE_IMPLEMENTATION_COPY(Void,Documentation,Type)
FAUDES_TYPE_IMPLEMENTATION_CAST(Void,Documentation,Type)
FAUDES_TYPE_IMPLEMENTATION_ASSIGN(Void,Documentation,Type)
FAUDES_TYPE_IMPLEMENTATION_EQUAL(Void,Documentation,Type)

// construct
Documentation::Documentation(void) : Type() {
 mAutoRegistered=false;
 mApplicationRegistered=false;
}

// construct
Documentation::Documentation(const Documentation& rOther) : Type() {
  DoAssign(rOther);
}

// std faudes type
void Documentation::DoAssign(const Documentation& rSrc) {
  // assign my members
  mName=rSrc.mName;
  mPlugIn=rSrc.mPlugIn;
  mHtmlDoc=rSrc.mHtmlDoc;
  mTextDoc=rSrc.mTextDoc;
  mKeywords=rSrc.mKeywords;
}

// std faudes type
bool Documentation::DoEqual(const Documentation& rOther) const {
  // test my members
  if(mName!=rOther.mName) return false;
  if(mPlugIn!=rOther.mPlugIn) return false;
  if(mHtmlDoc!=rOther.mHtmlDoc) return false;
  if(mTextDoc!=rOther.mTextDoc) return false;
  if(mKeywords!=rOther.mKeywords) return false;
  return true;
}

// clear all
void Documentation::Clear(void){
  FD_DRTI("Documentation::Clear()");
  mName="";
  mPlugIn="CoreFaudes";
  mHtmlDoc="";
  mTextDoc="";
  mKeywords="";
  mAutoRegistered=false;
  mApplicationRegistered=false;
}

//  read access members
const std::string& Documentation::Name(void) const{ return mName; }
const std::string& Documentation::PlugIn(void) const{ return mPlugIn; }
const std::string& Documentation::CType(void) const{ return mCType; }
const std::string& Documentation::TextDoc() const{ return mTextDoc; }
const std::string& Documentation::HtmlDoc() const{ return mHtmlDoc; }
const std::string& Documentation::Keywords() const{ return mKeywords; }
bool Documentation::AutoRegistered(void) const{ return mAutoRegistered; }
bool Documentation::ApplicationRegistered(void) const{ return mApplicationRegistered; }

//  write access members
void Documentation::Name(const std::string& name){mName = name; }
void Documentation::PlugIn(const std::string& plugin){mPlugIn = plugin; }
void Documentation::CType(const std::string& name){mCType = name; }
void Documentation::TextDoc(const std::string& textdoc){mTextDoc = textdoc;}
void Documentation::HtmlDoc(const std::string& fname){mHtmlDoc = fname;}
void Documentation::AutoRegistered(bool flag){ mAutoRegistered = flag;}
void Documentation::ApplicationRegistered(bool flag){ mApplicationRegistered = flag;}

//  write access keyword
void Documentation::AddKeyword(const std::string& rKeyword){
  if(mKeywords.empty()){
    mKeywords = rKeyword;
  } else{
    mKeywords.append(1, mDelim);
    mKeywords.append(rKeyword);
  }
}

// regex match keywords (todo)
std::string Documentation::MatchKeyword(const std::string& rPattern) const{
  FD_DRTI("Documentation::MatchKeyword(" << rPattern << ")");
  // be case insensitive (inefficient)
  std::string keys = mKeywords;
  std::transform(keys.begin(), keys.end(), keys.begin(), tolower);
  std::string match = rPattern;
  std::transform(match.begin(), match.end(), match.begin(), tolower);
  // find match
  std::string res="";
  std::size_t posk=keys.find(match);
  if(posk==std::string::npos) return res;
  // find delimiters 
  std::size_t posa=0;
  std::size_t posb=keys.length();
  while(true) {
    posb=keys.find(mDelim,posa);
    if(posb==std::string::npos) posb=mKeywords.length();
    if(posb>posk) break;
    posa=posb+1;
  };
  // return original case match
  FD_DRTI("Documentation::MatchKeyword(" << rPattern << "): " << mKeywords.substr(posa,posb-posa));
  return mKeywords.substr(posa,posb-posa);
}


// positional keyword
std::string Documentation::KeywordAt(int pos) const{
  FD_DRTI("Documentation::KeywordAt(" << pos << ")");
  // default result
  std::string res="";
  // find delimiter
  std::size_t posa=0;
  std::size_t posb=mKeywords.length();
  while(pos>0) {
    if(posa>=mKeywords.length()) return res;
    posb=mKeywords.find(mDelim,posa);
    if(posb==std::string::npos) return res;
    posa=posb+1;
    pos--;
  };
  // default result
  res=mKeywords.substr(posa);
  // find next delimiter
  posb=res.find(mDelim);
  if(posb==std::string::npos) return res;
  res=res.substr(0,posb);
  return res;
}

// keyword
int Documentation::KeywordsSize(void) const{
  int cnt=0;
  // find delimiter
  std::size_t pos=0;
  while(pos<mKeywords.length()) {
    cnt++;
    pos=mKeywords.find(mDelim,pos);
    if(pos==std::string::npos) break;
    pos++;
  };
  return cnt;
}


// merge with docu from token io
void Documentation::MergeDocumentation(TokenReader& rTr) {
  FD_DRTI("Documentation::MergeDocumentation(): for " << mName);
  // get first token to test type definition
  Token token;
  rTr.Peek(token);
  if(!token.IsBegin()) return;   
  std::string label=token.StringValue();
  // get ftype and test for match
  std::string ftype=token.AttributeStringValue("name");
  size_t pos=ftype.find("::");
  if(pos!=std::string::npos) ftype=ftype.substr(pos+2);
  // match?
  if( (mName!=ftype && mName!="") || (ftype=="")) {
    std::stringstream errstr;
    errstr << "Documentation mismatch in file \"" << rTr.FileName() << "\" : " << mName << "!=" << ftype;
    throw Exception("Documentation::MergeDocumentation", errstr.str(), 48);
  };
  // do read
  DoRead(rTr,label);
}

// token io
void Documentation::DoRead(TokenReader& rTr,  const std::string& rLabel, const Type* pContext) {
  FD_DRTI("Documentation::DoRead(): file " << rTr.FileName());
  // ignore
  (void) pContext;
  // figure my section
  std::string label = rLabel;
  if(label=="") label="Documentation";
  // get first token for ftype and ctype
  Token token;
  rTr.ReadBegin(label,token);

  // sense and digest pre 2.16b format
  if(!token.ExistsAttributeString("name")) {
    Token ptoken;
    rTr.Peek(ptoken);
    mName=ptoken.StringValue();
    size_t pos=mName.find("::");
    if(pos!=std::string::npos) mName=mName.substr(pos+2);
    mPlugIn=ptoken.StringValue();
    pos=mPlugIn.find("::");
    if(pos!=std::string::npos) mPlugIn=mPlugIn.substr(0,pos);
    else mPlugIn = "CoreFaudes";
    DoReadCore(rTr);
    rTr.ReadEnd(label);
    return;
  } // end pre 2.16b format

  // get ftype
  mName=token.AttributeStringValue("name");
  size_t pos=mName.find("::");
  if(pos!=std::string::npos) mName=mName.substr(pos+2);
  // get plugin
  mPlugIn=token.AttributeStringValue("name");
  pos=mPlugIn.find("::");
  if(pos!=std::string::npos) mPlugIn=mPlugIn.substr(0,pos);
  else mPlugIn = "CoreFaudes";
  // get ctype
  mCType=token.AttributeStringValue("ctype");

  // get autoreg flag (note: let flag survive if originally set and no attribute)
  if(token.ExistsAttributeInteger("autoregister"))
    mAutoRegistered = (token.AttributeIntegerValue("autoregister"));
  // report
  FD_DRTI("Documentation::DoRead(): found " << mName << " " << mCType);
  // do read
  DoReadCore(rTr);
  // done
  rTr.ReadEnd(label);
}

// token read
void Documentation::DoReadCore(TokenReader& rTr) {
  FD_DRTI("Documentation::DoReadCore()");
  Token token;
  mTextDoc="";
  mHtmlDoc="";
  mKeywords="";

  // sense and digest pre 2.16 format
  rTr.Peek(token);
  if(token.IsString()) {
    mName=token.StringValue();
    size_t pos=mName.find("::");
    if(pos!=std::string::npos) mName=mName.substr(pos+2);
    mPlugIn=token.StringValue();
    pos=mPlugIn.find("::");
    if(pos!=std::string::npos) mPlugIn=mPlugIn.substr(0,pos);
    else mPlugIn = "CoreFaudes";
    rTr.ReadString();
    mCType="";
    rTr.Peek(token);
    if(token.IsOption())
      mCType = rTr.ReadOption();
    while(true) {
      rTr.Peek(token);
      if(token.IsBegin())
      if(token.StringValue()=="TextDoc") {
        rTr.ReadBegin("TextDoc");
        mTextDoc = rTr.ReadString();
        rTr.ReadEnd("TextDoc");
        continue;
      }
      if(token.IsBegin())
      if(token.StringValue()=="HtmlDoc") {
        rTr.ReadBegin("HtmlDoc");
        mHtmlDoc = rTr.ReadString();
        rTr.ReadEnd("HtmlDoc");
        continue;
      }
      if(token.IsBegin())
      if(token.StringValue()=="Keywords") {
        rTr.ReadBegin("Keywords");
        while(!rTr.Eos("Keywords")) 
          AddKeyword(rTr.ReadString());
        rTr.ReadEnd("Keywords");
        continue;
      }
      break;
    }
    return;
  } // end pre 2.16 format

  // loop other known entries
  while(true) {
    rTr.Peek(token);
    // do the read: docu
    if(token.IsBegin())  
    if(token.StringValue()=="Documentation") {
      if(token.ExistsAttributeString("ref"))
        mHtmlDoc=token.AttributeStringValue("ref");
      rTr.ReadText("Documentation",mTextDoc);
      continue;
    }
    // do the read: keys
    if(token.IsBegin())  
    if(token.StringValue()=="Keywords") {
      rTr.ReadBegin("Keywords");
      while(!rTr.Eos("Keywords")) 
        AddKeyword(rTr.ReadString());
      rTr.ReadEnd("Keywords");
      continue;
    }
    // unknown -> derived class take over
    break;
  }
}


// token write
void Documentation::DoWrite(TokenWriter& rTw,  const std::string& rLabel, const Type* pContext) const {
  // ignore
  (void) pContext;
  // figure my section
  std::string label = rLabel;
  if(label=="") label="Documentation";
  // begin tag
  Token btag;
  btag.SetBegin(label);
  if(mPlugIn!="" && mName!="")
    btag.InsAttribute("name",mPlugIn+"::"+mName);
  if(mPlugIn=="" && mName!="")
    btag.InsAttribute("name",mName);
  if(mCType!="")
    btag.InsAttribute("ctype",mCType);
  if(mAutoRegistered)
    btag.InsAttributeBoolean("autoregister",true);
  rTw << btag;
  // data
  DoWriteCore(rTw);
  // end tag
  rTw.WriteEnd(label);  
}

// token write
void Documentation::DoWriteCore(TokenWriter& rTw) const {
  // write text doc
  if(mTextDoc!="" || mHtmlDoc!="") {
    Token btag;
    btag.SetBegin("Documentation");
    if(mHtmlDoc!="") 
      btag.InsAttributeString("ref",mHtmlDoc);
    rTw.WriteText(btag,mTextDoc);
  }
  // write keys
  int ksz=KeywordsSize();
  if(ksz>0) {
    rTw.WriteBegin("Keywords");
    for(int i=0; i<ksz; i++) 
      rTw.WriteString(KeywordAt(i));
    rTw.WriteEnd("Keywords");
  }
}



/*
********************************************************************
********************************************************************
********************************************************************

Implementation of class TypeDefinition

********************************************************************
********************************************************************
********************************************************************
*/


// faudes type (cannot do autoregister)
FAUDES_TYPE_IMPLEMENTATION_NEW(Void,TypeDefinition,Documentation)
FAUDES_TYPE_IMPLEMENTATION_COPY(Void,TypeDefinition,Documentation)
FAUDES_TYPE_IMPLEMENTATION_ASSIGN(Void,TypeDefinition,Documentation)
FAUDES_TYPE_IMPLEMENTATION_CAST(Void,TypeDefinition,Documentation)
FAUDES_TYPE_IMPLEMENTATION_EQUAL(Void,TypeDefinition,Documentation)


// Typedefinition constructor function
TypeDefinition* TypeDefinition::Constructor(Type* pProto, const std::string& rTypeName){
  FD_DRTI("TypeDefinition::Construct(" << typeid(*pProto).name() << ", " << rTypeName << ")");
  TypeDefinition* td = new TypeDefinition();
  td->Prototype(pProto);
  std::string name=rTypeName;
  if(name=="") name=typeid(*pProto).name();
  td->Name(name);
  return(td);
}


// clear (all except prototype)
void TypeDefinition::Clear(){
  FD_DRTI("TypeDefinition::Clear()");
  // call base
  Documentation::Clear();
  // my members
  mXElementTag="";
}


// std faudes type
void TypeDefinition::DoAssign(const TypeDefinition& rSrc) {
  // assign base members
  Documentation::DoAssign(rSrc);
  // assign my members
  mXElementTag=rSrc.mXElementTag;
  // special member
  if(mpType) delete mpType;
  mpType=0;
  if(rSrc.mpType) 
    mpType=rSrc.mpType->New();
}

// std faudes type
bool TypeDefinition::DoEqual(const TypeDefinition& rOther) const {
  // test base members
  if(!Documentation::DoEqual(rOther)) return false;
  // test my members
  if(mXElementTag!=rOther.mXElementTag) return false;
  return true;
}


// token io
void TypeDefinition::DoRead(TokenReader& rTr,  const std::string& rLabel, const Type* pContext) {
  FD_DRTI("TypeDefinition::DoRead(): file " << rTr.FileName());
  // ignore
  (void) pContext;
  // figure my section
  std::string label = rLabel;
  if(label=="") label="TypeDefinition";
  // base can handle this
  Documentation::DoRead(rTr,label,pContext);
  // my members
}

// token io
void TypeDefinition::DoReadCore(TokenReader& rTr) {
  FD_DRTI("TypeDefinition::DoReadCore(): file " << rTr.FileName());
  // call base core
  Documentation::DoReadCore(rTr);  
  // my data
  while(true) {
    Token token;
    rTr.Peek(token);
    // element tag
    if(token.IsBegin())
    if(token.StringValue()=="XElementTag") {
      rTr.ReadBegin("XElementTag",token);
      if(token.ExistsAttributeString("value"))
        mXElementTag=token.AttributeStringValue("value");
      rTr.ReadEnd("XElementTag");
      continue;
    }
    // break un unknown
    break;
  }
}

// token io
void TypeDefinition::DoWrite(TokenWriter& rTw,  const std::string& rLabel, const Type* pContext) const {
  // label
  std::string label=rLabel;
  if(label=="") label="TypeDefinition";
  // base can handle
  Documentation::DoWrite(rTw,label,pContext); 
}

// token io
void TypeDefinition::DoWriteCore(TokenWriter& rTw) const {
  FD_DRTI("TypeDefinition::DoWriteCore(): file " << rTw.FileName());
  // call base core
  Documentation::DoWriteCore(rTw);  
  // my data
  if(mXElementTag!="") {
    Token etag;
    etag.SetEmpty("XElementTag");
    etag.InsAttributeString("value", mXElementTag);
    rTw << etag;
  }
}


// access prototype
const Type* TypeDefinition::Prototype(void) const{
  return(mpType);
}

// set prototype
void TypeDefinition::Prototype(faudes::Type* pType){
  if(mpType) delete mpType;
  mpType = pType;
  // test factory method
#ifdef FAUDES_CHECKED
  if(mpType!=0) {
    Type* nobj = mpType->New();
    if(typeid(*nobj) !=  typeid(*mpType)) {
      FD_WARN("TypeDefinition::Prototype(): factory method not implemented for c++-type " << typeid(*pType).name());
    }
    delete nobj;
  }
#endif
}

// construct new object with faudes type
Type* TypeDefinition::NewObject() const{
  FD_DRTI("TypeDefinition::NewObject()");
  // bail out 
  if(!mpType) return NULL;
  // use prototype
  return mpType->New();
}



// parameter access
const std::string& TypeDefinition::XElementTag(void) const {
  return mXElementTag;
}

// parameter access
void TypeDefinition::XElementTag(const std::string& rTag)  {
  mXElementTag=rTag;
}



} //namspace

