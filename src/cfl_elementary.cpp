/** @file cfl_elementary.cpp Runtime interface, elementary types */

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


#include "cfl_elementary.h"

namespace faudes{



/*
********************************************************************
********************************************************************
********************************************************************

Implementation of class Integer

********************************************************************
********************************************************************
********************************************************************
*/


// public:
Integer::Integer() {
  CValue(0);
}

Integer::Integer(Int val) {
  CValue(val);
}

Integer* Integer::New() const{
  return(new Integer());
}

Integer* Integer::Copy() const{
  return(new Integer(mCInteger));
}

const Integer* Integer::Cast(const Type* pOther) const{
  return dynamic_cast<const Integer*>(pOther);
}

void Integer::CValue(Int val){
  mCInteger = val;
}

Int Integer::CValue() const{
  return(mCInteger);
}

Int* Integer::CReference() {
  return &mCInteger;
}

void Integer::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const{
  (void) pContext;
  std::string label=rLabel;
  std::string ftype="Integer";
  if(label=="") label=ftype;
  Token btag;
  btag.SetBegin(label);
  if(ftype!=label) btag.InsAttributeString("ftype",ftype);
  FD_DRTI("Integer::DoWrite(): file " << rTw.FileName() << " section " << label);
  rTw.Write(btag);
  rTw.WriteFloat(mCInteger);
  rTw.WriteEnd(label);
}

void Integer::DoRead(TokenReader& rTr,  const std::string& rLabel, const Type* pContext) {
  (void) pContext;
  std::string label = rLabel;
  if(label == "") label = "Integer";
  FD_DRTI("Integer()::DoRead(): file " << rTr.FileName() << "  section " << label);
  rTr.ReadBegin(label);
  mCInteger = (Int)rTr.ReadFloat();
  rTr.ReadEnd(label);
  FD_DRTI("Integer::DoRead(): done");
}


// integer sum, uniform rti api
long int IntegerSum(const Integer& arg1, const Integer& arg2) {
  return arg1+arg2;
}

// integer sum, uniform rti api
long int IntegerSum(const IntegerVector& intvect) {
  long int res=0;
  for(unsigned int i=0; i< intvect.Size(); i++) 
    res+=intvect.At(i);
  return res;
}


/*
********************************************************************
********************************************************************
********************************************************************

Implementation of class String

********************************************************************
********************************************************************
********************************************************************
*/


// constructor
String::String() {
  CValue("");
}

// constructor
String::String(std::string val) {
  CValue(val);
}

// factory constructor
String* String::New() const{
  return new String();
}

// factory constructor
String* String::Copy() const{
  return new String(mCString);
}

// cast
const String* String::Cast(const Type* pOther) const{
  return dynamic_cast<const String*>(pOther);
}

// c value
std::string String::CValue() const{
  return mCString;
}

// c value
void String::CValue(std::string s){
  mCString = s;
}

// c ref
std::string* String::CReference() {
  return &mCString;
}

// token io
void String::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const{
  (void) pContext;
  // write begin tag
  std::string label=rLabel;
  std::string ftype="String";
  if(label=="") label=ftype;
  Token btag;
  btag.SetBegin(label);
  if(ftype!=label) btag.InsAttributeString("ftype",ftype);
  FD_DRTI("String::DoWrite(): file " << rTw.FileName() << " section " << label);
  // write as possibly quoted string token: no escape characters, no formating
  std::string escstr="<>&\n\r\v";
  if(mCString.find_first_of(escstr)==std::string::npos) {
    rTw.Write(btag);
    rTw.WriteString(mCString);
    rTw.WriteEnd(label);
  }
  // nontrivial perhaps longe string: write section with CDATA elements
  else {
    rTw.WriteVerbatim(btag, mCString);
  }
}

// token io
void String::DoRead(TokenReader& rTr,  const std::string& rLabel, const Type* pContext) {
  (void) pContext;
  std::string label = rLabel;
  if(label == "") label = "String";
  FD_DRTI("String::DoRead(): file " << rTr.FileName() << "  section " << label);
  // note: ReadVerbatim will read both plain or verbatim string
  rTr.ReadVerbatim(label, mCString);
  FD_DRTI("String::DoRead(): done");
}


/*
********************************************************************
********************************************************************
********************************************************************

Implementation of class Boolean

********************************************************************
********************************************************************
********************************************************************
*/


// constructor
Boolean::Boolean() {
  CValue(true);
}

// constructor
Boolean::Boolean(bool val) {
  CValue(val);
}

// factory constructor
Boolean* Boolean::New() const{
  return new Boolean();
}

// factory constructor
Boolean* Boolean::Copy() const{
  return new Boolean(mCBool);
}

// cast
const Boolean* Boolean::Cast(const Type* pOther) const{
  return dynamic_cast<const Boolean*>(pOther);
}

// cvaliu
void Boolean::CValue(bool val){
  mCBool = val;
}

// cvalue 
bool Boolean::CValue() const{
  return(mCBool);
}

// c ref
bool* Boolean::CReference() {
  return &mCBool;
}

// token io
void Boolean::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const{
  (void) pContext;
  std::string label=rLabel;
  std::string ftype="Boolean";
  if(label=="") label=ftype;
  Token btag;
  btag.SetBegin(label);
  if(ftype!=label) btag.InsAttributeString("ftype",ftype);
  FD_DRTI("String::DoWrite(): file " << rTw.FileName() << " section " << label);
  rTw.Write(btag);
  if(mCBool) rTw.WriteString("true");
  else rTw.WriteString("false");
  rTw.WriteEnd(label);
}


// token io
void Boolean::DoRead(TokenReader& rTr,  const std::string& rLabel, const Type* pContext) {
  (void) pContext;
  std::string label = rLabel;
  if(label == "") label = "Boolean";
  FD_DRTI("Boolean::DoRead(): file " << rTr.FileName() << "  section " << label);
  rTr.ReadBegin(label);
  std::string value = rTr.ReadString();
  std::transform(value.begin(), value.end(), value.begin(), tolower);
  if(value=="true") mCBool=true;
  else if(value=="false") mCBool=false;
  else {
    std::stringstream err;
    err << "Expected true or false: " << rTr.FileLine();
    throw Exception("Boolean::DoRead()", err.str(), 52);
  }
  rTr.ReadEnd(label);
  FD_DRTI("Boolean::DoRead(): done");
}



} //namspace

