/** @file cfl_indexset.cpp @brief Classes IndexSet */

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

#include "cfl_indexset.h"


namespace faudes {

/*
*********************************************************************************
*********************************************************************************
*********************************************************************************

 IndexSet 

*********************************************************************************
*********************************************************************************
*********************************************************************************
*/

// std faudes type 
FAUDES_TYPE_IMPLEMENTATION(IndexSet,IndexSet,TBaseSet<Idx>)

/*
FAUDES_TYPE_IMPLEMENTATION_NEW(IndexSet,IndexSet,TBaseSet<Idx>)
FAUDES_TYPE_IMPLEMENTATION_COPY(IndexSet,IndexSet,TBaseSet<Idx>)
FAUDES_TYPE_IMPLEMENTATION_CAST(IndexSet,IndexSet,TBaseSet<Idx>)
FAUDES_TYPE_IMPLEMENTATION_ASSIGN(IndexSet,IndexSet,TBaseSet<Idx>)
FAUDES_TYPE_IMPLEMENTATION_EQUAL(IndexSet,IndexSet,TBaseSet<Idx>)
FAUDES_TYPE_IMPLEMENTATION_REGISTER(IndexSet,IndexSet,TBaseSet<Idx>)
*/



// IndexSet()
IndexSet::IndexSet(void) : TBaseSet<Idx>() {
  FD_DC("IndexSet(" << this << ")::IndexSet()");
  Name("IndexSet");
}

// IndexSet(rOtherSet)
IndexSet::IndexSet(const IndexSet& rOtherSet) : 
  TBaseSet<Idx>(rOtherSet) 
{
  FD_DC("IndexSet(" << this << ")::IndexSet(rOtherSet " << &rOtherSet << ")");
  // copy my members (none)
}

// IndexSet(rOtherSet)
IndexSet::IndexSet(const TBaseSet<Idx>& rOtherSet) : 
  TBaseSet<Idx>(rOtherSet) 
{
  FD_DC("IndexSet(" << this << ")::IndexSet(rOtherSet " << &rOtherSet << ")");
  // copy my members (none)
}

// File constructor
IndexSet::IndexSet(const std::string& rFilename, const std::string& rLabel) :
  TBaseSet<Idx>() 
{
  FD_DC("IndexSet(" << this << ")::IndexSet(" << rFilename << ")");
  Read(rFilename, rLabel);
}

// assignment (attributes to default)
void IndexSet::DoAssign(const IndexSet& rSourceSet) {
  FD_DC("NameSet(" << this << ")::DoAssign(..)");
  // call base 
  TBaseSet<Idx>::DoAssign(rSourceSet);
} 

// DoWrite(rTw&)
void IndexSet::DoWrite(TokenWriter& rTw, const std::string& rLabel,const Type* pContext) const {
  // figure section
  std::string label=rLabel;
  if(label=="") label=Name(); 
  if(label=="") label="IndexSet"; 
  FD_DC("IndexSet(" << this << ")::DoWrite(..): section " << label);
  /*
  // figure context: default to non
  const SymbolTable* symboltable=0;
  const std::map<Idx,Idx>* indexmap=0;  
  // figure context: can use vgenerator
  if(const Generator* vgen=dynamic_cast<const Generator*>(pContext)) {
    FD_DC("TaIndexSet(" << this << ")::DoWrite(..): generator context");
    symboltable=&vgen->StateSymbolTable();
    indexmap=&vgen->MinStateIndexMap();
  }
  */
  rTw.WriteBegin(label);
  Iterator it, conit;
  // iterate states to write
  for (it = Begin(); it != End(); ++it) {
    // identify consecutive block
    Idx start = *it;
    Idx anoncount = 0;
    for(conit=it; conit != End(); ++conit) {
      if(!Attribute(*conit).IsDefault()) break;
      if(*conit != start+anoncount) break;
      ++anoncount;
    }
    // write consecutive block
    if (anoncount > FD_CONSECUTIVE) {
      rTw.WriteBegin("Consecutive");
      rTw << start;
      rTw << start+anoncount-1;
      rTw.WriteEnd("Consecutive");
      it=conit;
    } 
    // break loop
    if(it == End() )
      break;
    // write individual state
    rTw << *it;
    // write state attribute
    const AttributeVoid& attr=Attribute(*it);
    attr.Write(rTw,"",pContext);
  }
  rTw.WriteEnd(label);
}

// DoXWrite(rTw&)
void IndexSet::DoXWrite(TokenWriter& rTw, const std::string& rLabel,const Type* pContext) const {
  // Set up outer tag
  Token btag=XBeginTag(rLabel,"IndexSet");
  rTw.Write(btag);
  FD_DC("IndexSet(" << this << ")::DoXWrite(..): section " << btag.StringValue() << " #" << Size());

  /*
  // figure context: default to non
  const SymbolTable* symboltable=0;
  const std::map<Idx,Idx>* indexmap=0;  
  // figure context: can use vgenerator
  if(const Generator* vgen=dynamic_cast<const Generator*>(pContext)) {
    FD_DC("TaIndexSet(" << this << ")::DoWrite(..): generator context");
    symboltable=&vgen->StateSymbolTable();
    indexmap=&vgen->MinStateIndexMap();
  }
  */
  // Loop elements
  Iterator it, conit;
  std::string etstr=XElementTag();
  for(it = Begin(); it != End(); ++it) {
    // identify consecutive block
    Idx start = *it;
    Idx anoncount = 0;
    for(conit=it; conit != End(); ++conit) {
      if(!Attribute(*conit).IsDefault()) break;
      if(*conit != start+anoncount) break;
      ++anoncount;
    }
    // write anonymous block
    if(anoncount > FD_CONSECUTIVE) {
      Token contag;
      contag.SetEmpty("Consecutive");
      contag.InsAttributeInteger("from",start);
      contag.InsAttributeInteger("to",start+anoncount-1);
      rTw.Write(contag);
      it=conit;
    } 
    // break loop
    if(it == End()) break;
    // prepare token
    Token sttag;    
    Idx index=*it;
    sttag.SetBegin(etstr);
    sttag.InsAttributeInteger("id",index);
    rTw.Write(sttag);
    // attribute
    const AttributeVoid& attr=Attribute(index);
    if(!attr.IsDefault()) attr.XWrite(rTw);
    // done
    rTw.WriteEnd(etstr);
  }
  rTw.WriteEnd(btag.StringValue());
}

// DoRead(rTr, rLabel)
void IndexSet::DoRead(TokenReader& rTr, const std::string& rLabel,const Type* pContext) {
   // set up defaults
  std::string label=rLabel;
  std::string ftype=TypeName();
  std::string etstr=XElementTag();
  // figure section from current token
  Token token;
  if(label=="") {
    rTr.Peek(token);
    if(token.IsBegin()) label=token.StringValue();
  }
  if(label=="") label="IndexSet"; 
  Name(label);
  // read begin
  FD_DC("IndexSet(" << this << ")::DoRead(..): section " << label);
  rTr.ReadBegin(label,token); 
  if(token.ExistsAttributeString("name"))
    Name(token.AttributeStringValue("name"));
  // prepare attribute
  AttributeVoid* attrp = AttributeType()->New();
  FD_DC("IndexSet(" << this << ")::DoRead(..): attribute type " << typeid(*attrp).name());
  // loop tokens
  while(!rTr.Eos(label)) {
    rTr.Peek(token);
    // read by index (faudes format)
    if(token.IsInteger()) {
      FD_DC("TaIndexSet(" << this << ")::DoRead(..): inserting element in faudes format \"" 
	    << token.IntegerValue() << "\"");
      rTr.Get(token);
      Idx index=token.IntegerValue();
      // read attribute
      attrp->Read(rTr,"",pContext);
      // skip unknown attributes
      AttributeVoid::Skip(rTr);
      // insert element with attribute
      Insert(index); 
      Attribute(index,*attrp);
      continue;
    }
    // read consecutive block of anonymous states (both faudes or XML)
    if(token.IsBegin("Consecutive")) {
      Token ctag;
      rTr.ReadBegin("Consecutive",ctag);
      // Read up to two tokens for faudes format
      Token token1,token2;
      rTr.Peek(token1);
      if(!token1.IsEnd()) rTr.Get(token1);
      rTr.Peek(token2);
      if(!token2.IsEnd()) rTr.Get(token2);
      rTr.ReadEnd("Consecutive");
      // Try to interpret
      int from=-1;
      int to=-1;
      if(token1.IsInteger()) from=token1.IntegerValue();
      if(token2.IsInteger()) to=token2.IntegerValue();
      if(ctag.ExistsAttributeInteger("from"))
        from= ctag.AttributeIntegerValue("from");
      if(ctag.ExistsAttributeInteger("to"))
        to= ctag.AttributeIntegerValue("to");
      // Insist in valid range
      if(from<0 || to <0  || to < from) {
	delete attrp;
	std::stringstream errstr;
	errstr << "Invalid range of consecutive indices"  << rTr.FileLine();
	throw Exception("IndexSet::DoRead", errstr.str(), 50);
      }
      FD_DC("IndexSet(" << this << ")::DoRead(..): inserting range " 
	    << from << " to " << to);
      for(Idx index = (Idx) from; index <= (Idx) to; ++index) 
	Insert(index);
      continue;
    }
    // read element section (XML format)
    if(token.IsBegin(etstr)) {
      FD_DC("TaIndexSet(" << this << ")::DoRead(..): inserting element in xml format \"" 
	    << token.StringValue() << "\"");
      rTr.ReadBegin(etstr,token);
      Idx index=token.AttributeIntegerValue("id");
      // read faudes attribute
      attrp->Read(rTr,"",pContext);
      // skip unknown faudes attributes
      rTr.ReadEnd(etstr);
      // insert element with attribute
      Insert(index); 
      Attribute(index,*attrp);
      continue;
    }
    // cannot process token
    delete attrp;
    std::stringstream errstr;
    errstr << "Invalid token type " << token.Type() << " at " << rTr.FileLine();
    throw Exception("IndexSet::DoRead", errstr.str(), 50);
  }
  rTr.ReadEnd(label);
  // dispose attribute
  delete attrp;
}


// MaxIndex()
Idx IndexSet::MaxIndex(void) const {
  if(Size()==0) return 0;
  Idx res =  *(--(End()));
#ifdef FAUDES_CHECKED
  if(res==FAUDES_IDX_MAX) {
    std::stringstream errstr;
    errstr << "IndexSet(): MaxIndex exceeds platform limitations -- reimplement index allocation: \""
	   << Name() << "\"";
    throw Exception("vGenerator::EventReame(name)", errstr.str(), 61);
  }
#endif
  return res;
}

// Insert()
Idx IndexSet::Insert(void) {
  FD_DC("IndexSet(" << this << ")::Insert()");
  Idx index=MaxIndex()+1;
  TBaseSet<Idx>::Insert(index);
  FD_DC("IndexSet(" << this << ")::Insert(): done");
  return index;
}

//Insert(idx)
bool IndexSet::Insert(const Idx& rIndex) {
  FD_DC("IndexSet(" << this << ")::Insert(idx)");
  return TBaseSet<Idx>::Insert(rIndex);
}


//Valid(idx)
bool  IndexSet::Valid(const Idx& rIndex) const {
  return rIndex!=0;
}


// Signature()
Idx IndexSet::Signature(void) const {
  // prepare result
  Idx sig = 0;
  // helpers:
  Idx i = 1;
  Iterator it=Begin() ;
  Iterator it_end=End() ;
  // algorithm:
  for(; it != it_end; ++it) {
    sig += *it * i;
    ++i;
  }
  return sig;
}




} // end name space
