/** @file cfl_registry.cpp Runtime interface, registry for faudes types and functions */

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



#include "cfl_registry.h"
#include "corefaudes.h"

// auto install types
#ifndef FAUDES_MUTE_RTIAUTOLOAD
#include <map>
#include <stack>
#include <set>
#include <vector>
#include <utility>
#include <limits>
#include "allplugins.h"
#include "rtiautoload.h"
#include "rtiautoload.cpp"
#endif



namespace faudes{

//  local debuging
/*
#undef FD_DREG
#define FD_DREG(a) FD_WARN(a)  
#undef FD_DRTI
#define FD_DRTI(a) FD_WARN(a)  
*/
  
/*
********************************************************************
********************************************************************
********************************************************************

Implemantation of faudes TypeRegistry

********************************************************************
********************************************************************
********************************************************************
*/

// static members: ref to the only one instance
TypeRegistry* TypeRegistry::mpInstance = 0;

// static member: access to singleton
TypeRegistry* TypeRegistry::G(){
  // lazy initialization
  if(!mpInstance){
    FD_DREG("TypeRegistry(): Constrtuct singleton");
    mpInstance = new TypeRegistry();
  }
  return(mpInstance);
}

// clear except C++-autoregistered
void TypeRegistry::Clear() {
  FD_DREG("TypeRegistry::Clear(): begin with #" << Size());
  // prepare: delete all typedefs contained in map, except fpr autoregistered
  std::map<std::string, TypeDefinition*>::iterator mit;
  for(mit = mNameToTypeDef.begin(); mit != mNameToTypeDef.end(); mit++){
    if(mit->second != NULL)
    if(!mit->second->AutoRegistered()){
      delete mit->second;
      mit->second = NULL;
    }
    if(mit->second != NULL)
    if(mit->second->AutoRegistered()){
      FD_DREG("TypeRegistry::Clear(): detect autoreg " << mit->second->Name());
    }
  }
  // delete in NameToTypeDef
  std::map<std::string, TypeDefinition*>::iterator dit;
  for(mit = mNameToTypeDef.begin(); mit != mNameToTypeDef.end();){
    dit=mit; mit++;
    if(dit->second == NULL) mNameToTypeDef.erase(dit);
  }
  // fix Id map (this only works if autoregisreded typeids have no doublet faudes names)
  mIdToTypeDef.clear();
  for(mit = mNameToTypeDef.begin(); mit != mNameToTypeDef.end(); mit++){
    TypeDefinition* ptdef=mit->second;
    const Type* pt=ptdef->Prototype();
    FD_DREG("TypeRegistry::Clear(): keeping " << ptdef->Name());
    if(pt)
      if(mIdToTypeDef.find(typeid(*pt).name())==mIdToTypeDef.end())
        mIdToTypeDef[typeid(*pt).name()]=ptdef;
  }
}

// clear all. 
void TypeRegistry::ClearAll(void) {
  FD_DREG("TypeRegistry::ClearAll()");
  // delete all typedefs contained in map
  std::map<std::string, TypeDefinition*>::iterator mit;
  for(mit = mNameToTypeDef.begin(); mit != mNameToTypeDef.end(); mit++){
    if(mit->second != NULL){
      delete mit->second;
      mit->second = NULL;
    }
  }
  // delete maps
  mNameToTypeDef.clear();
  mIdToTypeDef.clear();
}

// query number of entries
int TypeRegistry::Size() const{
  return(mNameToTypeDef.size());
}

// read access on type map
TypeRegistry::Iterator  TypeRegistry::Begin(void) const{
  return(mNameToTypeDef.begin());
}

// read access on type map
TypeRegistry::Iterator TypeRegistry::End(void) const{
  return(mNameToTypeDef.end());
}

// insert new entry
void TypeRegistry::Insert(TypeDefinition* pTypeDef){
  FD_DREG("TypeRegistry::Insert(): type name " << pTypeDef->Name());
  FD_DREG("TypeRegistry::Insert(): prototype " << pTypeDef->Prototype()); 
  // test existence that match: ignore
  const Type* pt=pTypeDef->Prototype();
  if(Exists(pTypeDef->Name())){
    const TypeDefinition* otdef = Definitionp(pTypeDef->Name());
    const Type* opt=otdef->Prototype();
    if((pt!=0) && (opt!=0))
    if(typeid(*pt).name() == typeid(*opt).name()) {
      FD_DREG("TypeRegistry::Insert(): ignoring doublet " << pTypeDef->Name());
      return;
    }
  }
  // test existence that differ: error
  if(Exists(pTypeDef->Name())){
    std::stringstream err;
    err << "Cannot overwrite existing entry with type " << pTypeDef->Name() << std::endl;
    throw Exception("TypeRegistry::Insert()", err.str(), 46);
  }
  // test for name
  if(pTypeDef->Name()=="") {
    std::stringstream err;
    err << "Cannot have empty name"<< std::endl;
    throw Exception("TypeRegistry::Insert()", err.str(), 46);
  };
  // record in main map
  mNameToTypeDef[pTypeDef->Name()]=pTypeDef;
  // record in id map (first entry wins)
  if(pt)
    if(mIdToTypeDef.find(typeid(*pt).name())==mIdToTypeDef.end())
      mIdToTypeDef[typeid(*pt).name()]=pTypeDef;
}


// scan token stream for type definitions
void TypeRegistry::MergeDocumentation(TokenReader& rTr) {
  FD_DV("TypeRegistry::MergeDocumentation(): using " << rTr.FileName());
  // scan file
  Token token;
  while(rTr.Peek(token)) {
    // test for TypeDefinition with name
    if(!token.IsBegin("TypeDefinition")) { rTr.Get(token); continue; }   
    if(!token.ExistsAttributeString("name")) { rTr.Get(token); continue; }
    // found type def in file, extract ftype
    std::string ftype=token.AttributeStringValue("name");
    size_t pos=ftype.find("::");
    if(pos!=std::string::npos) ftype=ftype.substr(pos+2);
    FD_DRTI("TypeRegistry::MergeDocumentation(): " << ftype);
    // locate type def in map
    Iterator tit = mNameToTypeDef.find(ftype);
    // case 1: type exists (from LoadRegistry or c++-Autoregister): add documentaion
    if(tit!=mNameToTypeDef.end()) {
      tit->second->MergeDocumentation(rTr);
      continue;
    }
    // case 2: type does not exist (e.g. with ref2html): insert fake entry
    TypeDefinition* tdef = new TypeDefinition(ftype);
    tdef->MergeDocumentation(rTr);
    Insert(tdef);
  }
}

// scan file for type definitions
void TypeRegistry::MergeDocumentation(const std::string& rFileName) {
  TokenReader tr(rFileName);
  MergeDocumentation(tr);
}


// set  element tag
void TypeRegistry::ElementTag(const std::string& rTypeName, const std::string& rTag) {
  FD_DREG("TypeRegistry::ElementTag("<<rTypeName<<","<<rTag<<")");
  Iterator mit=mNameToTypeDef.find(rTypeName);
  if(mit == End()) return;
  mit->second->ElementTag(rTag);
  FD_DREG("TypeRegistry::ElementTag: ok: " << ElementTag(rTypeName));
}

// get element tag
const std::string& TypeRegistry::ElementTag(const std::string& rTypeName) const {
  Iterator mit=mNameToTypeDef.find(rTypeName);
  static std::string estr="";
  if(mit == End()) return estr;
  return mit->second->ElementTag();
}

// set element type
void TypeRegistry::ElementType(const std::string& rTypeName, const std::string& rType) {
  Iterator mit=mNameToTypeDef.find(rTypeName);
  if(mit == End()) return;
  mit->second->ElementType(rType);
}

// get element type
const std::string& TypeRegistry::ElementType(const std::string& rTypeName) const {
  Iterator mit=mNameToTypeDef.find(rTypeName);
  static std::string estr="";
  if(mit == End()) return estr;
  return mit->second->ElementType();
}

// set auto-register flag
void TypeRegistry::AutoRegistered(const std::string& rTypeName, bool flag) {
  Iterator mit=mNameToTypeDef.find(rTypeName);
  if(mit == End()) {
    FD_DREG("TypeRegistry::AutoRegistered(...): cannot access definition for faudes type " << rTypeName);
    return;
  }
  mit->second->AutoRegistered(flag);
}

// get auto-register flag
bool TypeRegistry::AutoRegistered(const std::string& rTypeName) const {
  Iterator mit=mNameToTypeDef.find(rTypeName);
  if(mit == End()) return false;
  return mit->second->AutoRegistered();
}


// construct faudes object by typename
Type* TypeRegistry::NewObject(const std::string& rName) const{
  FD_DRTI("TypeRegistry::NewObject(\"" << rName << "\")");
  Iterator mit=mNameToTypeDef.find(rName);
  if(mit == End()) {
    std::stringstream err;
    err << "Unknown Type " << rName << std::endl;
    throw Exception("TypeRegistry::NewObject()", err.str(), 47);
  }
  Type* res=mit->second->NewObject();
  if(!res) {
    std::stringstream err;
    err << "Failed to instatiate new " << rName << std::endl;
    throw Exception("TypeRegistry::NewObject()", err.str(), 47);
  }
  return res;
}

// construct faudes object by typed reference
Type* TypeRegistry::NewObject(const Type& rType) const{
  FD_DRTI("TypeRegistry::NewObject(prototype): typeid " << typeid(rType).name());
  Iterator mit;
  mit=mIdToTypeDef.find(typeid(rType).name());
  if(mit == mIdToTypeDef.end()) {
    std::stringstream err;
    err << "Unknown type by reference" << std::endl;
    throw Exception("TypeRegistry::NewObject()", err.str(), 47);
  }
  return(rType.New());
}

// access type definition by type name
const TypeDefinition& TypeRegistry::Definition(const std::string& rName) const{
  FD_DRTI("TypeRegistry::Definition( " << rName << " )");
  Iterator mit=mNameToTypeDef.find(rName);
  if(mit == End()) {
    std::stringstream err;
    err << "Type not found: \"" << rName << "\"";
    throw Exception("TypeRegistry::Definition()", err.str(), 46);
  }
  return(*(mit->second));
}

// access type definition by typed reference
const TypeDefinition& TypeRegistry::Definition(const Type& rType) const{
  FD_DRTI("TypeRegistry::Definition(): typeid " << typeid(rType).name());
  Iterator mit;
  mit=mIdToTypeDef.find(typeid(rType).name());
  if(mit!=mIdToTypeDef.end()) return *(mit->second);
  std::stringstream err;
  err << "Type not found: " << typeid(rType).name();
  throw Exception("TypeRegistry::Definition()", err.str(), 46);
}

// access type definition by type name
const TypeDefinition* TypeRegistry::Definitionp(const std::string& rName) const{
  FD_DRTI("TypeRegistry::Definitionp( " << rName << " )");
  Iterator mit=mNameToTypeDef.find(rName);
  if(mit == End()) return NULL;
  return(mit->second);
}

// access type definition by typed reference
const TypeDefinition* TypeRegistry::Definitionp(const Type& rType) const{
  FD_DRTI("TypeRegistry::Definitionp(): for typeid " << typeid(rType).name());
  Iterator mit;
  mit=mIdToTypeDef.find(typeid(rType).name());
  if(mit==mIdToTypeDef.end()) {
    FD_DRTI("TypeRegistry::Definitionp(): not found");
    return NULL;
  }
  TypeDefinition* fdp=mit->second;
  FD_DRTI("TypeRegistry::Definitionp(): found faudes type " << fdp->Name());
  return fdp;
}

// access prototype by type name
const Type* TypeRegistry::Prototype(const std::string& rName) const{
  FD_DRTI("TypeRegistry::Prototype( " << rName << " )");
  Iterator mit=mNameToTypeDef.find(rName);
  if(mit == End()) return 0;
  return(mit->second->Prototype());
}

// access type definition by typed reference
const std::string& TypeRegistry::TypeName(const Type& rType) const{
  FD_DRTI("TypeRegistry::TypeName(): typeid " << typeid(rType).name());
  Iterator mit;
  mit=mIdToTypeDef.find(typeid(rType).name());
  if(mit!=mIdToTypeDef.end()) return mit->second->Name();
  static std::string empty("");
  return empty;
}

// test type compatibility
bool TypeRegistry::TypeTest(const std::string& rTypeName, const Type& rObject) const {
  FD_DRTI("TypeRegistry::TypeTest(): typeid " << typeid(rObject).name());
  Iterator mit=mNameToTypeDef.find(rTypeName);
  if(mit == End()) return false;
  if(!mit->second->Prototype()) return false;
  FD_DRTI("TypeRegistry::TypeTest(): dst ftype " << rTypeName<< " src typeid " << typeid(rObject).name()
	  << " res " << (mit->second->Prototype()->Cast(&rObject) != 0));
  return ( mit->second->Prototype()->Cast(&rObject) != 0 );
}

// test existence by type name
bool TypeRegistry::Exists(const std::string& rName) const{
  return mNameToTypeDef.find(rName) != End();
}

// test existsnce by type name
bool TypeRegistry::Exists(const Type& rType) const{
  return mIdToTypeDef.find(typeid(rType).name()) != mIdToTypeDef.end();
}


// token write (informative/debugging)
void TypeRegistry::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  FD_DRTI("TypeRegistry::DoWrite(): file " << rTw.FileName());
  // ignore label and context
  (void) rLabel;
  (void) pContext;
  // doit
  Iterator tit;
  for(tit=Begin();tit!=End();tit++) {
     // write type definition
     rTw.WriteXmlComment("===================================================");
     rTw.WriteXmlComment("===================================================");
     rTw.WriteXmlComment("Faudes Type " + tit->second->Name());
     rTw.WriteXmlComment("===================================================");
     rTw.WriteXmlComment("===================================================");
     rTw.Endl();
     tit->second->Write(rTw);
     rTw.Endl();
     rTw.Endl();
  }
}


/*
********************************************************************
********************************************************************
********************************************************************

Implemantation of faudes FunctionRegistry

********************************************************************
********************************************************************
********************************************************************
*/

// static members: ref to the only one instnace
FunctionRegistry* FunctionRegistry::mpInstance = 0;

// static member: access to signleton
FunctionRegistry* FunctionRegistry::G(){
  // lazy initialization
  if(!mpInstance){
    FD_DREG("FunctionRegistry(): Construct singleton");
    mpInstance = new FunctionRegistry();
  }
  return(mpInstance);
}

// clear all
void FunctionRegistry::Clear(){
  FD_DREG("FunctionRegistry::Clear()");
  // delete all functiondefs contained in map
  std::map<std::string, FunctionDefinition*>::iterator mit;
  for(mit = mNameToFunctionDef.begin(); mit != mNameToFunctionDef.end(); mit++){
    if(mit->second != NULL) {
      FD_DREG("FunctionRegistry::Clear: removing " << mit->second->Name());
      delete mit->second;
      mit->second = NULL;
    }
  }
  // delete maps
  mNameToFunctionDef.clear();
  mIdToFunctionDef.clear();
}

// query number of entries
int FunctionRegistry::Size() const{
  return(mNameToFunctionDef.size());
}

// read access on function map
FunctionRegistry::Iterator  FunctionRegistry::Begin(void) const{
  return(mNameToFunctionDef.begin());
}

// read access on function map
FunctionRegistry::Iterator FunctionRegistry::End(void) const{
  return(mNameToFunctionDef.end());
}

// insert new entry
void FunctionRegistry::Insert(FunctionDefinition* pFunctionDef){
#ifdef FAUDES_DEBUG_REGISTRY
  if(pFunctionDef->Prototype()) {
    FD_DREG("FunctionRegistry::Insert(): definition for " << pFunctionDef->Name());
  } else {
    FD_DREG("FunctionRegistry::Insert(): fake entry for " << pFunctionDef->Name());
  }
#endif
  // test existence
  if(Exists(pFunctionDef->Name())){
    std::stringstream err;
    err << "Cannot overwrite existing entry with function " << pFunctionDef->Name() << std::endl;
    throw Exception("FunctionRegistry::Insert()", err.str(), 46);
  }
  // test for name
  if(pFunctionDef->Name()=="") {
    std::stringstream err;
    err << "Cannot have empty name"<< std::endl;
    throw Exception("FunctionRegistry::Insert()", err.str(), 46);
  };
  // record in maps
  mNameToFunctionDef[pFunctionDef->Name()]=pFunctionDef;
  const Type* pt=pFunctionDef->Prototype();
  if(pt)
    mIdToFunctionDef[typeid(*pt).name()]=pFunctionDef;
}


// scan token stream for function definitions
void FunctionRegistry::MergeDocumentation(TokenReader& rTr) {
  FD_DREG("FunctionRegistry::MergeDocumentation(): using " << rTr.FileName());
  // scan file
  Token token;
  while(rTr.Peek(token)) {
    // test for FunctionDefinition with name
    // note: we intentionally accept LuaFunctionDefinitions for RTI documentation 
    if(!token.IsBegin()) 
      { rTr.Get(token); continue; }   
    if((token.StringValue()!="FunctionDefinition") && (token.StringValue()!="LuaFunctionDefinition")) 
       { rTr.Get(token); continue; }
    if(!token.ExistsAttributeString("name")) 
      { rTr.Get(token); continue; }
    // found function def in file, extract ftype
    std::string ffunction=token.AttributeStringValue("name");
    size_t pos=ffunction.find("::");
    if(pos!=std::string::npos) ffunction=ffunction.substr(pos+2);
    // locate functiondef in map
    Iterator fit = mNameToFunctionDef.find(ffunction);
    // case 1: function exists (from LoadRegistry or c++-Autoregister): add documentaion
    if(fit!=mNameToFunctionDef.end()) {
      fit->second->MergeDocumentation(rTr);
      continue;
    }
    // case 2: function does not exist (e.g. with ref2html): insert fake entry
    FunctionDefinition* fdef = new FunctionDefinition(ffunction);
    fdef->MergeDocumentation(rTr);
    Insert(fdef);
  }
}


// scan file for function definitions
void FunctionRegistry::MergeDocumentation(const std::string& rFileName) {
  TokenReader tr(rFileName);
  MergeDocumentation(tr);
}


// construct faudes object by functionname
Function* FunctionRegistry::NewFunction(const std::string& rName) const{
  FD_DRTI("FunctionRegistry::NewFunction(\"" << rName << "\")");
  Iterator mit=mNameToFunctionDef.find(rName);
  if(mit == End()) {
    std::stringstream err;
    err << "Unknown function " << rName << std::endl;
    throw Exception("FunctionRegistry::NewFunction()", err.str(), 47);
  }
  Function* res=mit->second->NewFunction();
  if(!res) {
    std::stringstream err;
    err << "Failed to instatiate new " << rName << std::endl;
    throw Exception("FunctionRegistry::NewFunction()", err.str(), 47);
  }
  return res;
}

// construct faudes object by function reference
Function* FunctionRegistry::NewFunction(const Function& rFunction) const{
  FD_DRTI("FunctionRegistry::NewFunction(prototype): typeid " << typeid(rFunction).name());
  Iterator mit;
  mit=mIdToFunctionDef.find(typeid(rFunction).name());
  if(mit == mIdToFunctionDef.end()) {
    std::stringstream err;
    err << "Unknown function by reference" << std::endl;
    throw Exception("FunctionRegistry::NewFunction()", err.str(), 47);
  }
  return(rFunction.New());
}

// access function definition by function name
const FunctionDefinition& FunctionRegistry::Definition(const std::string& rName) const{
  FD_DRTI("FunctionRegistry::Definition( " << rName << " )");
  Iterator mit=mNameToFunctionDef.find(rName);
  if(mit == End()) {
    std::stringstream err;
    err << "Function not found: " << rName;
    throw Exception("FunctionRegistry::Definition()", err.str(), 46);
  }
  return(*(mit->second));
}

// access function definition by functiond reference
const FunctionDefinition& FunctionRegistry::Definition(const Function& rFunction) const{
  FD_DRTI("FunctionRegistry::Definition(): typeid " << typeid(rFunction).name());
  Iterator mit;
  mit=mIdToFunctionDef.find(typeid(rFunction).name());
  if(mit!=mIdToFunctionDef.end()) return *(mit->second);
  std::stringstream err;
  err << "Function not found: " << typeid(rFunction).name();
  throw Exception("FunctionRegistry::Definition()", err.str(), 46);
}

// access function definition by function name
const FunctionDefinition* FunctionRegistry::Definitionp(const std::string& rName) const{
  FD_DRTI("FunctionRegistry::Definition( " << rName << " )");
  Iterator mit=mNameToFunctionDef.find(rName);
  if(mit == End()) return nullptr;
  return(mit->second);
}

// access function definition by typed reference
const std::string& FunctionRegistry::FunctionName(const Function& rFunction) const{
  FD_DRTI("FunctionRegistry::FunctionName(): typeid " << typeid(rFunction).name());
  Iterator mit;
  mit=mIdToFunctionDef.find(typeid(rFunction).name());
  if(mit!=mIdToFunctionDef.end()) return mit->second->Name();
  static const std::string empty("");
  return empty;
}

// test existence by function name
bool FunctionRegistry::Exists(const std::string& rName) const{
  return mNameToFunctionDef.find(rName) != End();
}

// test existsnce by function 
bool FunctionRegistry::Exists(const Function& rFunction) const{
  return mIdToFunctionDef.find(typeid(rFunction).name()) != mIdToFunctionDef.end();
}


// token write (informative/debugging)
void FunctionRegistry::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  FD_DREG("FunctionRegistry::DoWrite(): file " << rTw.FileName());
  // ignore label and context
  (void) rLabel;
  (void) pContext;
  // doit
  Iterator tit;
  for(tit=Begin();tit!=End();tit++) {
     // write function definition
     rTw.WriteXmlComment("===================================================");
     rTw.WriteXmlComment("===================================================");
     rTw.WriteXmlComment("Faudes Function " + tit->second->Name());
     rTw.WriteXmlComment("===================================================");
     rTw.WriteXmlComment("===================================================");
     rTw << "\n";
     tit->second->Write(rTw);
     rTw << "\n" << "\n";
  }
}


/*
********************************************************************
********************************************************************
********************************************************************

Implemantation of LoadRegistry

********************************************************************
********************************************************************
********************************************************************
*/

// load from file
void LoadRegistry(const std::string& rPath) {
  FD_DREG("LoadRegistry(" << rPath << ")");
  // default path
  std::string rtipath = rPath;
  if(rtipath=="") rtipath="./libfaudes.rti";  // todo: plattform/configure
  // clear
  TypeRegistry::G()->Clear();     // note: we do not clear "c++-autoregistered" types
  FunctionRegistry::G()->Clear();

  // auto install types extracted from rti file
#ifndef FAUDES_MUTE_RTIAUTOLOAD
  LoadRegisteredTypes();          // note: this does not load "c++-autoregistered" types
#endif

  // allow build system load registry programmatic contributions defined in plugins
  // (this is currently not used by any plug-in)
#ifdef FAUDES_PLUGINS_RTILOAD
  FAUDES_PLUGINS_RTILOAD;
#endif

  // auto install functions extracted from rti file
  // (this is currently not used by any plug-in)
#ifndef FAUDES_MUTE_RTIAUTOLOAD
  LoadRegisteredFunctions();
#endif

  // merge documentation
  TypeRegistry::G()->MergeDocumentation(rtipath);
  FunctionRegistry::G()->MergeDocumentation(rtipath);

  // test and report status
#ifdef FAUDES_CHECKED
#ifndef FAUDES_MUTE_RTIAUTOLOAD
  TypeRegistry::Iterator tit;
  for(tit=TypeRegistry::G()->Begin(); tit!=TypeRegistry::G()->End(); tit++) {
    // should have prototype
    if(tit->second->PlugIn()!="IODevice") 
    if(tit->second->Prototype()==NULL) 
      FD_DREG("TypeRegistry: " << tit->second->Name() << " has no prototype");
  }
  FunctionRegistry::Iterator fit;
  for(fit=FunctionRegistry::G()->Begin(); fit!=FunctionRegistry::G()->End(); fit++) {
    // should have prototype
    if(fit->second->Prototype()==NULL) 
      FD_DREG("FunctionRegistry: " << fit->second->Name() << " has no prototype");
  }
#endif
#endif

  FD_DREG("LoadRegistry(" << rPath << "): done");
}


// save to file or std out
void SaveRegistry(const std::string& rPath) {
  FD_DRTI("SaveRegistry(" << rPath << ")");
  // have a tokenwriter
  TokenWriter* ptw;
  if(rPath=="") {
    ptw = new TokenWriter(TokenWriter::Stdout);
  } else {
    ptw = new TokenWriter(rPath,"Registry");
  }
  // do the write
  ptw->WriteBegin("Registry");
  ptw->Endl();
  TypeRegistry::G()->Write(*ptw);
  FunctionRegistry::G()->Write(*ptw);
  ptw->Endl();
  ptw->WriteEnd("Registry");
  // dispose
  delete ptw;
}


// clear all
void ClearRegistry(void) {
  FD_DRTI("ClearRegistry()");
  // clear
  FunctionRegistry::G()->Clear();
  TypeRegistry::G()->ClearAll();
}

// conveience access to singleton 
Type* NewFaudesObject(const std::string& rTypeName) { return TypeRegistry::G()->NewObject(rTypeName);}
Function* NewFaudesFunction(const std::string& rFunctName) { return FunctionRegistry::G()->NewFunction(rFunctName);}
const std::string& FaudesTypeName(const Type& rObject) { return TypeRegistry::G()->TypeName(rObject);}
bool FaudesTypeTest(const std::string& rTypeName, const Type& rObject) { return TypeRegistry::G()->TypeTest(rTypeName,rObject);}
const std::string& FaudesFunctionName(const Function& rObject) { return FunctionRegistry::G()->FunctionName(rObject);}


} // namespace
