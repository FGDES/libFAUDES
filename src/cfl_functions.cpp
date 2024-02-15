/** @file cfl_functions.cpp Runtime interface, operations on faudes types */

    /* FAU Discrete Event Systems Library (libfaudes)

    Copyright (C) 2009 Ruediger Berndt
    Copyright (C) 2009 Thomas Moor

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


#include "cfl_functions.h"
#include "cfl_registry.h"


namespace faudes{

/*
********************************************************************
********************************************************************
********************************************************************

Implementation of class Parameter

********************************************************************
********************************************************************
********************************************************************
*/

// constructor, default 
Parameter::Parameter(void) : 
  mName(""), mTDName(""), mAttr(Parameter::UnDef), mCReturn(false) 
{}

// constructor, by member values 
  Parameter::Parameter(const std::string& rName, const std::string& rTypeName, ParamAttr attr, bool cret) : 
    mName(rName), mTDName(rTypeName), mAttr(attr), mCReturn(cret) 
{}

// Desctructor 
Parameter::~Parameter(void){}

// get name
const std::string& Parameter::Name(void) const 
  { return mName;}

// set name
void Parameter::Name(const std::string& rName) 
  { mName=rName;}

// get type
const std::string& Parameter::Type(void) const 
  { return mTDName;}

// set type
void Parameter::Type(const std::string& rTypeName) 
  { mTDName=rTypeName;}

// get attribute
const Parameter::ParamAttr& Parameter::Attribute(void) const 
  { return mAttr; }

// set attribute
void Parameter::Attribute(const ParamAttr& rAttr) 
  { mAttr=rAttr;}

// get c ret flag
bool  Parameter::CReturn(void) const 
  { return mCReturn; }

// set c ret flag
void Parameter::CReturn(bool cret) 
{ mCReturn=cret;}

// set attribute
void Parameter::Attribute(const std::string& rAttrStr) { 
  mAttr=Parameter::UnDef;    
  if(rAttrStr=="In") mAttr=Parameter::In;
  if(rAttrStr=="Out") mAttr=Parameter::Out;
  if(rAttrStr=="InOut") mAttr=Parameter::InOut;
}

// textual attribute
std::string Parameter::AStr(Parameter::ParamAttr attr){
  switch(attr){
  case Parameter::In: return "In";
  case Parameter::Out: return "Out";
  case Parameter::InOut: return "InOut";
  default: break;
  }
  return "UnDef";
}

// textual parameter
std::string Parameter::Str(void) const {
  std::string res = "+" + AStr(mAttr)+ "+ " + mTDName + " " + mName;
  return res;
}

// set to "undefined"
void Parameter::Clear(void){
  mName = "";
  mTDName = "";
  mAttr = Parameter::UnDef;
  mCReturn=false;
}

// test equality
bool Parameter::operator==(const Parameter& rOther) const {
  if(mName!=rOther.mName) return false;
  if(mTDName!=rOther.mTDName) return false;
  if(mAttr!=rOther.mAttr) return false;
  if(mCReturn!=rOther.mCReturn) return false;
  return true;
}



/*
********************************************************************
********************************************************************
********************************************************************

Implementation of class Signature

********************************************************************
********************************************************************
********************************************************************
*/


// faudes type (cannot do autoregister)
FAUDES_TYPE_IMPLEMENTATION_NEW(Void,Signature,Type)
FAUDES_TYPE_IMPLEMENTATION_COPY(Void,Signature,Type)
FAUDES_TYPE_IMPLEMENTATION_CAST(Void,Signature,Type)
FAUDES_TYPE_IMPLEMENTATION_ASSIGN(Void,Signature,Type)
FAUDES_TYPE_IMPLEMENTATION_EQUAL(Void,Signature,Type)

// constructor
Signature::Signature(void) : Type() {}

// copy constructor
Signature::Signature(const Signature& rSrc) : Type()
{
  DoAssign(rSrc);
}

// std faudes type
void Signature::DoAssign(const Signature& rSrc) {
  // assign my members
  mName=rSrc.mName;
  mParameters=rSrc.mParameters;
}

// std faudes type
bool Signature::DoEqual(const Signature& rOther) const {
  // test my members
  if(mName!=rOther.mName) return false;
  if(mParameters!=rOther.mParameters) return false;
  return true;
}


// clear signature
void Signature::Clear(void){
  FD_DRTI("Signature::Clear()");
  mName="";
  mParameters.clear();
}

// get name
const std::string& Signature::Name(void) const{
  return mName;
}

// set name
void Signature::Name(const std::string& rName){
  mName = rName;
}

// get size
int Signature::Size() const{
  return mParameters.size();
}

// get parameter
const Parameter& Signature::At(int n) const{
  // check range
  if((n < 0) || (n >= Size())){
    std::stringstream err;
    err << "Index out of range: " << n << std::endl;
    throw Exception("Signature::At()", err.str(), 47);
  }
  // return const ref
  return mParameters.at(n);
}

// set parameter
void Signature::At(int n, const Parameter& rParam) {
  // check range
  if((n < 0) || (n >= Size())){
    std::stringstream err;
    err << "Index out of range: " << n << std::endl;
    throw Exception("Signature::At()", err.str(), 47);
  }
  // set ref
  mParameters[n]=rParam;
}

// append parameter
void Signature::Append(const Parameter& rParam){
  mParameters.push_back(rParam);
}



// token io
void Signature::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const{
  // ignore
  (void) rLabel;
  (void) pContext;
  // write my tag
  Token btag;
  btag.SetBegin("Signature");
  btag.InsAttribute("name",mName);
  rTw << btag;
  // parameter specs
  for(int i=0; i<Size(); i++){
    Token par;
    par.SetEmpty("Parameter");
    par.InsAttribute("name",mParameters.at(i).Name());
    par.InsAttribute("ftype",mParameters.at(i).Type());
    switch(mParameters.at(i).Attribute()){
    case Parameter::In: par.InsAttribute("access","In"); break;
    case Parameter::Out: par.InsAttribute("access","Out"); break;
    case Parameter::InOut: par.InsAttribute("access","InOut"); break;
    default: break;
    }
    if(mParameters.at(i).CReturn()) par.InsAttribute("creturn","true");
    rTw << par ;
  }
  // end section
  rTw.WriteEnd("Signature");
}

// token io
void Signature::DoRead(TokenReader& rTr,  const std::string& rLabel, const Type* pContext){
  // ignore
  (void) rLabel;
  (void) pContext;
  // read my section
  FD_DRTI("Signature::DoRead(): file " << rTr.FileName());
  Token token;
  rTr.ReadBegin("Signature",token);

  // detect and digest old pre 2.16b file format
  // note: this section will eventually be removed
  if(!token.ExistsAttributeString("name")) {
    mName=rTr.ReadString();
    FD_DRTI("Signature::DoRead(): found (pre 2.16b file)" << mName);
    while(!rTr.Eos("Signature")) {
      std::string pname=rTr.ReadString();
      std::string ptype=rTr.ReadString();
      Parameter::ParamAttr pattr= Parameter::UnDef;
      bool cret=false;
      Token token;
      rTr.Peek(token);
      while(token.Type()==Token::Option) {
        rTr.Get(token);
        if(token.StringValue()=="In") pattr= Parameter::In;
        if(token.StringValue()=="Out") pattr= Parameter::Out;
        if(token.StringValue()=="InOut") pattr= Parameter::InOut;
        if(token.StringValue()=="OutIn") pattr= Parameter::InOut;
        if(token.StringValue()=="CReturn") cret=true;
        rTr.Peek(token);
      }
      Append(Parameter(pname,ptype,pattr,cret));
    }
    rTr.ReadEnd("Signature");
    return;
  } // end pre 2.16b file format
   
  // extract name
  mName=token.AttributeStringValue("name");
  FD_DRTI("Signature::DoRead(): found " << mName);  
  // loop parameters
  while(!rTr.Eos("Signature")) {
    Token par;
    rTr.ReadBegin("Parameter",par);
    // parameter name
    std::string pname=par.AttributeStringValue("name");
    // parameter type
    std::string ptype=par.AttributeStringValue("ftype");
    // parameter access
    Parameter::ParamAttr pattr= Parameter::UnDef;
    if(par.ExistsAttributeString("access")){
      std::string paccess = par.AttributeStringValue("access");
      if(paccess=="In")    pattr= Parameter::In;
      if(paccess=="Out")   pattr= Parameter::Out;
      if(paccess=="InOut") pattr= Parameter::InOut;
      if(paccess=="OutIn") pattr= Parameter::InOut;
    }
    // parameter cret  
    bool cret=false;
    if(par.ExistsAttributeString("creturn")){
      std::string pcret = par.AttributeStringValue("creturn");
      if(pcret=="true")   cret=true;
      if(pcret=="True")   cret=true;
      if(pcret=="Yes")    cret=true;
      if(pcret=="yes")    cret=true;
    }
    // insert
    Append(Parameter(pname,ptype,pattr,cret));
    rTr.ReadEnd("Parameter");
  }
  // done
  rTr.ReadEnd("Signature");
  FD_DRTI("Signature::DoRead(): done");
}




/*
********************************************************************
********************************************************************
********************************************************************

Implementation of class FunctionDefinition

********************************************************************
********************************************************************
********************************************************************
*/

// faudes type (cannot do autoregister)
FAUDES_TYPE_IMPLEMENTATION_NEW(Void,FunctionDefinition,Documentation)
FAUDES_TYPE_IMPLEMENTATION_COPY(Void,FunctionDefinition,Documentation)
FAUDES_TYPE_IMPLEMENTATION_ASSIGN(Void,FunctionDefinition,Documentation)
FAUDES_TYPE_IMPLEMENTATION_CAST(Void,FunctionDefinition,Documentation)
FAUDES_TYPE_IMPLEMENTATION_EQUAL(Void,FunctionDefinition,Documentation)


// constructor
FunctionDefinition::FunctionDefinition(const std::string& name) : 
  Documentation(), 
  mpFunction(NULL) 
{
  Name(name);
}

// copy constructor
FunctionDefinition::FunctionDefinition(const FunctionDefinition& rSrc) :
  Documentation(), 
  mpFunction(NULL) 
{
  DoAssign(rSrc);
}

// std faudes type
void FunctionDefinition::DoAssign(const FunctionDefinition& rSrc) {
  // assign base members
  Documentation::DoAssign(rSrc);
  // assign my members
  mVariants=rSrc.mVariants;
  mVariantIndexMap=rSrc.mVariantIndexMap;
  // special member
  if(mpFunction) delete mpFunction;
  mpFunction=0;
  if(rSrc.mpFunction) {
    mpFunction=rSrc.mpFunction->New();
    mpFunction->Definition(this);
  }
}

// std faudes type
bool FunctionDefinition::DoEqual(const FunctionDefinition& rOther) const {
  // test base members
  if(!Documentation::DoEqual(rOther)) return false;
  // test my members
  if(mVariants!=rOther.mVariants) return false;
  return true;
}

// clear (all but prototype)
void FunctionDefinition::Clear(){
  FD_DRTI("FunctionDefinition::Clear(): " << Name());
  // call base
  Documentation::Clear();
  // clear my variants
  mVariants.clear();
  mVariantIndexMap.clear();
}

// clear (variants only)
void FunctionDefinition::ClearVariants(){
  FD_DRTI("FunctionDefinition::ClearVariants(): " << Name());
  // clear my variants
  mVariants.clear();
  mVariantIndexMap.clear();
}


// get prototype object
const Function* FunctionDefinition::Prototype(void) const{
  return(mpFunction);
}

// set prototype object
void FunctionDefinition::Prototype(Function* pFunc){
  // delte any existing prototype
  if(mpFunction) delete mpFunction;
  // record new prototype
  mpFunction = pFunc;
  // bail out
  if(!mpFunction) return;
  // ensure that our prototype uses us as function definition
  mpFunction->Definition(this);
}

// construct function on head
Function* FunctionDefinition::NewFunction() const{
  FD_DRTI("FunctionDefinition::NewFunction(): name " << Name());
  if(!mpFunction) return NULL;
  return(mpFunction->New());
}


// # of variants
int FunctionDefinition::VariantsSize() const{
  return(mVariants.size());
}

// existence of variant by name
bool FunctionDefinition::ExistsVariant(const std::string& varname) const {
  return mVariantIndexMap.find(varname)!= mVariantIndexMap.end();
}

// get index by variant name
int FunctionDefinition::VariantIndex(const std::string& rName) const {
  std::map<std::string,int>::const_iterator vit= mVariantIndexMap.find(rName);
  if(vit==mVariantIndexMap.end()) return -1;
  return vit->second;
}

// extend variants
void FunctionDefinition::AppendVariant(const Signature& rVariant){
  FD_DRTI("FunctionDefinition::AppendVariant()");
  // assure that no variant with the same name is yet contained in the list
  if(ExistsVariant(rVariant.Name())) {
    std::stringstream err;
    err << "Attempt to append variant with existing name: " << rVariant.Name() << " in " << Name() << std::endl;
    throw Exception("FunctionDefinition::AppendVariant()", err.str(), 47);
  }
  // do append
  mVariantIndexMap[rVariant.Name()]=mVariants.size();
  mVariants.push_back(rVariant);
}


// get signature by variant name
const Signature& FunctionDefinition::Variant(const std::string& rName) const{
  std::map<std::string,int>::const_iterator vit= mVariantIndexMap.find(rName);
  if(vit==mVariantIndexMap.end()) {
    std::stringstream err;
    err << "Access to non existing variant " << rName << " in fnct " << Name() << std::endl;
    throw Exception("FunctionDefinition::Variant()", err.str(), 47);
  }
  return mVariants.at(vit->second);
}


// get signature by variant index
const Signature& FunctionDefinition::Variant(int n) const{
  // check range
  if((n < 0) || (n >= VariantsSize())){
    std::stringstream err;
    err << "Index out of range: " << n << std::endl;
    throw Exception("FunctionDefinition::Variant()", err.str(), 47);
  }
  return mVariants.at(n);
}


// token io
void FunctionDefinition::DoRead(TokenReader& rTr,  const std::string& rLabel, const Type* pContext) {
  FD_DRTI("FunctionDefinition::DoRead()");
  // ignore
  (void) pContext;
  // label
  std::string label=rLabel;
  if(label=="") label="FunctionDefinition";
  // base can handle this
  Documentation::DoRead(rTr,label,pContext);
}


// token io
void FunctionDefinition::DoReadCore(TokenReader& rTr) {
  FD_DRTI("FunctionDefinition::DoReadCore()");
  // call base
  Documentation::DoReadCore(rTr);  
  // do variants
  rTr.ReadBegin("VariantSignatures");
  while(!rTr.Eos("VariantSignatures")) {
    Signature sig;
    sig.Read(rTr);
    // no doublets
    if(ExistsVariant(sig.Name())) continue;
    // append variant signature
    AppendVariant(sig);
  } // end: !EOS
  rTr.ReadEnd("VariantSignatures");
}


// token io
void FunctionDefinition::DoWrite(TokenWriter& rTw,  const std::string& rLabel, const Type* pContext) const {
  // label
  std::string label=rLabel;
  if(label=="") label="FunctionDefinition";
  // base can handle
  Documentation::DoWrite(rTw,label,pContext); 
}

// token io
void FunctionDefinition::DoWriteCore(TokenWriter& rTw) const {
  FD_DRTI("FunctionDefinition::DoWriteCore(): file " << rTw.FileName());
  // call base core
  Documentation::DoWriteCore(rTw);  
  // write signatures
  rTw.WriteBegin("VariantSignatures");
  if(mVariants.size()>1) rTw << "\n";
  for(unsigned int i=0; i<mVariants.size(); i++) {
    mVariants.at(i).Write(rTw);
     if(mVariants.size()>1) rTw << "\n";
  }
  rTw.WriteEnd("VariantSignatures");
  rTw << "\n";
}


/*
********************************************************************
********************************************************************
********************************************************************

Implementation of class Function

********************************************************************
********************************************************************
********************************************************************
*/

// Constructor 
Function::Function(const FunctionDefinition* fdef) : 
  pFuncDef(fdef), 
  mVariantIndex(-1) 
{
#ifdef FAUDES_DEBUG_RUNTIMEINTERFACE
  if(!pFuncDef)
    {FD_DRTI("Function::Function(): prototype object");}
  else
    {FD_DRTI("Function::Function(): " << pFuncDef->Name());}
#endif
  // have default variant
  if(pFuncDef)
  if(pFuncDef->VariantsSize()>0)
     Variant(0);
}

// set function definition
void Function::Definition(const FunctionDefinition* fdef) {
  // invalidate variant
  mVariantIndex = -1;
  mParameterValues.clear();
  // set
  pFuncDef=fdef;
  // report
#ifdef FAUDES_DEBUG_RUNTIMEINTERFACE
  if(!pFuncDef)
    {FD_DRTI("Function::Definition(): prototype object");}
  else
    {FD_DRTI("Function::Definition(): " << pFuncDef->Name());}
#endif
}

// get function definition
const FunctionDefinition* Function::Definition(void) const {
  return pFuncDef;
}


// get signature size
int Function::VariantsSize(void) const {
  if(!pFuncDef) return 0;
  return pFuncDef->VariantsSize();
}

// set variant signature
void Function::Variant(int n) {
  // pass on t o virtual interface
  DoVariant(n);
}

// set variant signature
void Function::DoVariant(int n) {
  // clear recent variant
  mVariantIndex=-1;
  mParameterValues.clear(); 
  // prototype object can not have a variant
  if(!pFuncDef) {
    std::stringstream err;
    err << "No valid function definition available" << std::endl;
    throw Exception("Function::Variant(n)", err.str(), 47);
  }
  // index out of range
  if(n<0 || n >= VariantsSize()) {
    std::stringstream err;
    err << "Variant index out of range" << std::endl;
    throw Exception("Function::Variant(n)", err.str(), 48);
  }
  // set variant 
  mVariantIndex=n;
  int nparam = pFuncDef->Variant(mVariantIndex).Size();
  while((int) mParameterValues.size()<nparam)
    mParameterValues.push_back(0);
}

// set variant signature
void Function::Variant(const std::string& rVariantName) {
  // prototype object can not have a variant
  if(!pFuncDef) {
    std::stringstream err;
    err << "No valid function definition available" << std::endl;
    throw Exception("Function::Variant(name)", err.str(), 47);
  }
  // get index
  int varid = pFuncDef->VariantIndex(rVariantName);
  // detect invalid name
  if(varid<0) {
    std::stringstream err;
    err << "Unknown variant name " << rVariantName << std::endl;
    throw Exception("Function::Variant(name)", err.str(), 48);
  }
  // set by index
  DoVariant(varid);
}

// get signature
const Signature* Function::Variant(void) const {
  if(!pFuncDef) return 0;
  if(mVariantIndex<0) return 0;
  return &pFuncDef->Variant(mVariantIndex);
}

// get signature size
int Function::ParamsSize(void) const {
  return mParameterValues.size();
}

// set parameter value
void Function::ParamValue(int n, Type* pVal){
  if((n < 0) || (n >= ParamsSize())){
    std::stringstream err;
    err << "Parameter index out of range: " << n << std::endl;
    throw Exception("Function::ParamValue()", err.str(), 47);
  }
  mParameterValues.at(n) = pVal;
}

// get parameter
Type* Function::ParamValue(int n) const{
  if((n < 0) || (n >= ParamsSize())){
    std::stringstream err;
    err << "Parameter index out of range: " << n << std::endl;
    throw Exception("Function::ParamValue()", err.str(), 47);
  }
  return(mParameterValues.at(n));
}


// allocate parameter value
void Function::AllocateValue(int n) {
  FD_DRTI("Function::AllocateValue()");
  if(!Variant()) {
    std::stringstream err;
    err << "No variant specified";
    throw Exception("Function::AllocateValue()", err.str(), 47);
  }
  if((n < 0) || (n >= ParamsSize())){
    std::stringstream err;
    err << "Parameter index out of range: " << n << std::endl;
    throw Exception("Function::AllocateValue()", err.str(), 47);
  }
  ParamValue(n,TypeRegistry::G()->NewObject(Variant()->At(n).Type()));
}

// allocate parameter value
void Function::AllocateValues(void) {
  FD_DRTI("Function::AllocateValues()");
  for(int i = 0; i < ParamsSize(); i++) 
    AllocateValue(i);
}

// allocate parameter value
void Function::FreeValues(void) {
  FD_DRTI("Function::FreeValues()");
  for(int i = 0; i < ParamsSize(); i++) { 
    delete mParameterValues.at(i);
    mParameterValues.at(i) =0;
  }
}


// type check wrapper
bool Function::TypeCheck(int n) {
  FD_DRTI("Function::TypeCheck()");
  if(mVariantIndex<0) {
    std::stringstream err;
    err << "Variant not set" << std::endl;
    throw Exception("Function::TypeCheck(n)", err.str(), 48);
  }
  if(n<0 || n>= ParamsSize()) {
    std::stringstream err;
    err << "Parameter out of range" << std::endl;
    throw Exception("Function::Typechek(n)", err.str(), 48);
  }
  bool res=DoTypeCheck(n);
  FD_DRTI("Function::TypeCheck() done, ok=" << res);
  return res;
}

// type check wrapper
bool Function::TypeCheck(void) {
  FD_DRTI("Function::TypeCheck()");
  if(mVariantIndex<0) {
    std::stringstream err;
    err << "Variant not set" << std::endl;
    throw Exception("Function::TypeCheck()", err.str(), 48);
  }
  for(int i=0; i<ParamsSize(); i++) 
    if(!DoTypeCheck(i)) return false;
  FD_DRTI("Function::TypeCheck() done, ok");
  return true;
}

// exec wrapper
void Function::Execute(void){
  FD_DRTI("Function::Execute()");
  if(!Variant()) {
    std::stringstream err;
    err << "Variant not set" << std::endl;
    throw Exception("Function::Execute()", err.str(), 48);
  }
  for(int n=0; n<ParamsSize(); n++) {
    if(!DoTypeCheck(n)) {
      std::stringstream err;
      err << "Cannot cast parameter \"" << Variant()->At(n).Name() << 
        "\" to type \"" << Variant()->At(n).Type() << "\"";
      throw Exception("Function::Execute()", err.str(), 48);
    }
  }
  DoExecute();
  FD_DRTI("Function::Execute() done");
}


// token io (informative/debug)
void Function::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const{
  (void) pContext;
  std::string label = rLabel;
  if(label == "") label = "Function";
  FD_DRTI("Function::DoWrite(): file " << rTw.FileName() << " section " << label);
  rTw.Columns(1);
  rTw.WriteBegin(label);
  rTw << "\n";
  rTw.WriteBegin("Parameters");
  for(int i = 0; i < ParamsSize(); i++){
    rTw << typeid(ParamValue(i)).name();
    rTw << "\n";
  }
  rTw.WriteEnd("Parameters");
  rTw << "\n";
  rTw.WriteEnd(label);
  FD_DRTI("Function::DoWrite(): done");
}


} // namespace

