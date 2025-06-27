/** @file lbp_function.cpp luafaudes class to run scripts as rti functions */

/* 
FAU Discrete Event Systems Library (libfaudes)

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
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA 


*/


// my header
#include "lbp_function.h"

// all lua headers, incl Lua nd lbp_addons.h
#include "lbp_include.h"



namespace faudes{

/*
********************************************************************
********************************************************************
********************************************************************

Borrow from SWIG type system

********************************************************************
********************************************************************
********************************************************************
*/


/*
SWIGS representation of user data (lua runtime version 4, swig 1.3.36)
Note: the below interface has also been verified to meet SWIG 1.3.40 
and 2.0.1. Other versions of SWIG may or may not have compatible interfaces
and, hence, should be carefully verified befor use.
*/
typedef void *(*swig_converter_func)(void *, int *);
typedef struct swig_type_info *(*swig_dycast_func)(void **);
typedef struct swig_cast_info {
  swig_type_info         *type;			/* pointer to type that is equivalent to this type */
  swig_converter_func     converter;		/* function to cast the void pointers */
  struct swig_cast_info  *next;			/* pointer to next cast in linked list */
  struct swig_cast_info  *prev;			/* pointer to the previous cast */
} swig_cast_info;
typedef struct swig_type_info {
  const char             *name;			/* mangled name of this type */
  const char             *str;			/* human readable name of this type */
  swig_dycast_func        dcast;		/* dynamic cast function down a hierarchy */
  struct swig_cast_info  *cast;			/* linked list of types that can cast into this type */
  void                   *clientdata;		/* language specific type data */
  int                    owndata;		/* flag if the structure owns the clientdata */
} swig_type_info;
typedef struct {
  swig_type_info   *type;
  int               own;                        /* 1 <> owned by swig (garbadge collector wont destroy) */  
  void              *ptr;
} swig_lua_userdata;



/* 
Swig runtime sytem: dynamic cast of swig userdata pointers.
We need this to cast swig's userdata to the faudes base type 
faudes::Type. In contrast to the original SWIG runtime function
SWIG_ConvertPtr, the below variant is are more picky on types 
that do mot cast to faufes::Type. For such types, we return NULL
*/
void* SwigCastPtr(void* ptr, swig_type_info *from, swig_type_info *ty) {
  void* res=ptr;
  if(!ptr) return 0;
  if(!from) return 0;
  if(!ty) return 0;
  swig_cast_info *iter = ty->cast;
  while(iter) {
    if(iter->type == from) {
      if(iter == ty->cast) break;
      iter->prev->next = iter->next;
      if(iter->next)
        iter->next->prev = iter->prev;
      iter->next = ty->cast;
      iter->prev = 0;
      if(ty->cast) ty->cast->prev = iter;
      ty->cast = iter;
      break;
    }
    iter = iter->next;
  }
  if(!iter) return 0;
  if(!iter->converter) return 0;
  int nm;
  res= (*iter->converter)(ptr,&nm);
  return res;
}

/*
Test whether userdata is SWIG generated, i.e., can be examined by 
the SWIG type system
*/
swig_lua_userdata* SwigUserData(lua_State* L, int index) {
  if(!lua_isuserdata(L,index)) return NULL;
  // get the metatable
  if(!lua_getmetatable(L,index)) return NULL;
  // get the ".type" entry
  lua_pushstring(L,".type");
  lua_rawget(L,-2);
  if(!lua_isstring(L,-1)) { lua_pop(L,2); return NULL;};
  const char* ftype = lua_tostring(L,-1);
  if(!ftype) {lua_pop(L,2); return NULL;};
  // todo: test whether this is a faudes type?
  // todo: test whether the metatable is a registered by swig
  lua_pop(L,2);
  return (swig_lua_userdata*) lua_touserdata(L,index);
}


/*
********************************************************************
********************************************************************
********************************************************************

local helper, may go to cfl_helper.*

********************************************************************
********************************************************************
********************************************************************
*/

std::string MangleString(const std::string& str) {
  std::string res=str;
  std::size_t src=0;
  std::size_t dst=0;
  while(src<res.length()) {
    if(isalnum(res.at(src))) {res[dst++]=res[src++]; continue;}
    res[dst++]='_';
    for(;src<res.length();src++) 
      if(isalnum(res.at(src))) break;
  }
  if(dst>2)
  if(res[dst-1]=='_')
    dst--;
  res.resize(dst);
  return res;
}


/*
********************************************************************
********************************************************************
********************************************************************

Implementation of class LuaFunctionDefinition

********************************************************************
********************************************************************
********************************************************************
*/

// faudes type
FAUDES_TYPE_IMPLEMENTATION(LuaFunctionDefinition,LuaFunctionDefinition,FunctionDefinition)

// construct
LuaFunctionDefinition::LuaFunctionDefinition(const std::string& name) : 
  FunctionDefinition(name),
  pDefaultL(0) 
{
  Prototype(new LuaFunction(0));
  FD_DLB("LuaFunctionDefinition::LuaFunctionDefinition("<<this<<"): name   " << name);
  FD_DLB("LuaFunctionDefinition::LuaFunctionDefinition("<<this<<"): proto   " << mpFunction);
  FD_DLB("LuaFunctionDefinition::LuaFunctionDefinition("<<this<<"): has def " << mpFunction->Definition());
}

// copy construct
LuaFunctionDefinition::LuaFunctionDefinition(const LuaFunctionDefinition& rSrc) : 
  FunctionDefinition(rSrc.Name()),
  pDefaultL(0) 
{
  FD_DLB("LuaFunctionDefinition::LuaFunctionDefinition(copy)");
  DoAssign(rSrc);
}

// std faudes type
void LuaFunctionDefinition::DoAssign(const LuaFunctionDefinition& rSrc) {
  FD_DLB("LuaFunctionDefinition::DoAssign()");
  // assign base members
  FunctionDefinition::DoAssign(rSrc);
  // assign my members
  mLuaCode=rSrc.mLuaCode;
  mLuaFile=rSrc.mLuaFile;
  // special member
  pLuaFunction=dynamic_cast<LuaFunction*>(mpFunction);
  // report
  FD_DLB("LuaFunctionDefinition::DoAssign("<<this<<"): name   " << mName);
  FD_DLB("LuaFunctionDefinition::DoAssign("<<this<<"): proto   " << mpFunction);
  FD_DLB("LuaFunctionDefinition::DoAssign("<<this<<"): has def " << mpFunction->Definition())
}

// std faudes type
bool LuaFunctionDefinition::DoEqual(const LuaFunctionDefinition& rOther) const {
  // test base members
  if(!FunctionDefinition::DoEqual(rOther)) return false;
  // test my members
  if(mLuaCode!=rOther.mLuaCode) return false;
  if(mLuaFile!=rOther.mLuaFile) return false;
  return true;
}

// clear (all but prototype)
void LuaFunctionDefinition::Clear(){
  FD_DLB("LuaFunctionDefinition::Clear(): " << Name());
  // call base
  FunctionDefinition::Clear();
  // clear my data
  mLuaCode.clear();
}


// set prototype object
void LuaFunctionDefinition::Prototype(Function* pFunc){
  FunctionDefinition::Prototype(pFunc);
  // set typed version
  pLuaFunction=dynamic_cast<LuaFunction*>(pFunc);
}


// get lua code
const std::string& LuaFunctionDefinition::LuaCode(void) const {
  return mLuaCode;
}

// set lua code
void LuaFunctionDefinition::LuaCode(const std::string& rCode) {
  mLuaCode=rCode;
}

// get/set default lua state
LuaState* LuaFunctionDefinition::DefaultL(void) const {
  if(!pDefaultL)  return const_cast<LuaFunctionDefinition*>(this)->pDefaultL=LuaState::G();
  return pDefaultL;
}

// get/set default lua state
void LuaFunctionDefinition::DefaultL(LuaState* pL) {
  pDefaultL=pL;
}

// test all variants
std::string LuaFunctionDefinition::SyntaxCheck(void) {
  // trivial case ... use evaluate
  if(VariantsSize()==0) {
    FD_DLB("LuaFunctionDefinition::SyntaxCheck(): no variants defined, evaluate");
    return Evaluate();
  }    
  // allocate function
  faudes::LuaFunction* lfnct = pLuaFunction->New();
  // iterate variants
  std::string err = "";
  for(int i=0; i<lfnct->VariantsSize(); i++) {
    // select
    lfnct->Variant(i);
    FD_DLB("LuaFunctionDefinition::SyntaxCheck(): variant " << 
      lfnct->Variant()->Name());
    // allocate args (exception on unknown type, will lead to mem leak)
    try {
      lfnct->AllocateValues();
      // let luafunction have a go
      if(err=="") try {
        lfnct->SyntaxCheck();
      } catch(const faudes::Exception& ex)  {
        err = ex.What();
      }
      // free args
      lfnct->FreeValues();
    // alloc err
    } catch(const faudes::Exception& ex)  {
      err = "cannot allocate parameters for variant " + lfnct->Variant()->Name();
    }
  } // iterate variants
  delete lfnct;
  // done
  return err;
}


// test all variants
std::string LuaFunctionDefinition::Evaluate(LuaState* pL) {
  // allocate function and set state
  faudes::LuaFunction* lfnct = pLuaFunction->New();
  lfnct->L(pL);
  // evaluate
  std::string err = "";
  try {
    lfnct->Evaluate();
  } catch(const faudes::Exception& ex)  {
    err = ex.What();
  }
  // done
  delete lfnct;
  return err;
}

// install myself to a lua state
void LuaFunctionDefinition::Install(LuaState* pL) const {
  if(!pL) pL=DefaultL();
  Install(pL->LL());
}

// install myself to a lua state
void LuaFunctionDefinition::Install(lua_State* pLL) const {
  FD_DLB("LuaFunctionDefinition::Install(): " << Name());

  // bail out if no signature (it's a DESTool plain script)
  if(VariantsSize()==0) {
    FD_DLB("LuaFunctionDefinition::Install(): no signatures for fnct " << Name());
    return;
  }

  // function name (ctype overwrites name)
  std::string lfname = Name();  
  if(CType()!="") {
    size_t pos=CType().find("faudes::");
    if(pos!=std::string::npos) 
    lfname=CType().substr(std::string("faudes::").length());
  }

  // parse signatures, test consistence
  std::vector< std::string > lffnct;                          // fnct names
  std::vector< int > lfparcnt;                                // # actual parameters
  std::vector< std::vector<std::string> > lfparams;           // par names
  std::vector< std::vector<Parameter::ParamAttr> > lfattrib;  // par access
  std::vector< std::vector<bool> > lfretval;                  // actual return value
  std::vector< std::vector<bool> > lfparval;                  // actual parameter
  lffnct.resize(VariantsSize());
  lfparcnt.resize(VariantsSize());
  lfparams.resize(VariantsSize());
  lfattrib.resize(VariantsSize());
  lfretval.resize(VariantsSize());
  lfparval.resize(VariantsSize());
  for(int i=0; i<VariantsSize(); i++) {
    const Signature& sigi=Variant(i);
    int parcnt=0;
    for(int j=0; j<sigi.Size(); j++) {
      // retrieve faudes type and attrib
      std::string ftype=sigi.At(j).Type();
      Parameter::ParamAttr fattr=sigi.At(j).Attribute();
      // autodefault c-return flag
      bool fcret=false;
      if(fattr==Parameter::Out) fcret=true;
      if(fattr==Parameter::InOut) fcret=true;
      // get creturn from signature for non-elementary
      if(ftype!="Boolean")
      if(ftype!="Integer")
      if(ftype!="String")
         fcret=sigi.At(j).CReturn();  
      // bail out on non-out ret value
      if(fcret)
      if((fattr!=Parameter::Out) && (fattr!=Parameter::InOut)) 
        break;
      // bail out on unknown faudestype
      if(ftype!="Boolean")
      if(ftype!="Integer")
      if(ftype!="String")
      if(!TypeRegistry::G()->Exists(ftype)) 
        break;
      // figure and count actual parameters
      bool fcpar=true;
      if(fcret && fattr==Parameter::Out) fcpar=false;
      if(fcpar) parcnt++;
      // bail out on undef attribute
      if(fattr==Parameter::UnDef) break;
      // param ok
      lfparams.at(i).push_back(ftype);
      lfattrib.at(i).push_back(fattr);
      lfretval.at(i).push_back(fcret);
      lfparval.at(i).push_back(fcpar);
    } 
    // test for signature error
    if((int) lfparams.at(i).size()!=sigi.Size()) {
      FD_WARN("LuaFunctionDefinition::Install(): cannot interpret signature \"" << sigi.Name() << "\"");
      lffnct.resize(i);
      break;
    }
    // record function name
    lffnct.at(i)=MangleString(sigi.Name()); 
    lfparcnt.at(i)=parcnt;
  }
  FD_DLB("LuaFunctionDefinition::Install(): #" << lffnct.size() << " valid signatures");

  // todo: filter out doublets as in rti2code
  // prepare lua code for signature dispatch 
  std::vector< std::string > lftest1;
  std::vector< std::string > lftest2;
  std::vector< std::string > lfcall;
  std::vector< std::string > lfsig;
  for(unsigned int i=0; i<lffnct.size(); i++) {
    // create type conditional, part1: lua types
    std::string test1="(argn==" + ToStringInteger(lfparcnt.at(i)) + ")" ;    
    unsigned int pj=0;    
    for(unsigned int j=0; j<lfparams.at(i).size(); j++) {
      if(!lfparval.at(i).at(j)) continue;
      pj++;
      test1=test1+" and ";
      std::string ltype="userdata";
      if(lfparams.at(i).at(j) == "Integer") ltype="number";
      if(lfparams.at(i).at(j) == "Boolean") ltype="bool";
      if(lfparams.at(i).at(j) == "String") ltype="string";
      test1=test1 + "(type(arg[" + ToStringInteger(pj) + "])=='" + ltype + "')";
    }
    lftest1.push_back(test1);
    // create type conditional, part2: faudes types
    std::string test2="";
    pj=0;    
    for(unsigned int j=0; j<lfparams.at(i).size(); j++) {
      if(!lfparval.at(i).at(j)) continue;
      pj++;
      if(lfparams.at(i).at(j) == "Integer") continue;
      if(lfparams.at(i).at(j) == "Boolean") continue;
      if(lfparams.at(i).at(j) == "String") continue;
      if(test2.size()>0) test2=test2+" and ";
      test2=test2 + "faudes.TypeTest(\"" + lfparams.at(i).at(j) + "\", arg[" + ToStringInteger(pj) + "])";
    }
    lftest2.push_back(test2);
    // create function call
    std::string call= lffnct.at(i) + "(";
    pj=0;
    for(unsigned int j=0; j<lfparams.at(i).size(); j++) {
      if(!lfparval.at(i).at(j)) continue;
      pj++;
      if(pj>1) call = call + ", ";
      call = call + "arg[" + ToStringInteger(pj) + "]";
    }
    call = call + ")";
    lfcall.push_back(call);
    // create nice signature. note: this should match the respective code in rti2code.cpp
    std::string nsig = " " + lfname + "(";
    bool leftcomma = false;
    bool rightcomma = false;
    for(unsigned int j=0; j<lfparams.at(i).size(); j++) {
      // return value
      if(lfretval.at(i).at(j)) {
        if(leftcomma) nsig = "," + nsig;
	nsig=lfparams.at(i).at(j) + nsig;
        leftcomma=true;
      }
      // parameter value
      if(lfparval.at(i).at(j)) {
        if(rightcomma) nsig += ", ";
        const Signature& sigi=Variant(i);
        nsig += sigi.At(j).Str();
        rightcomma=true;
      }
    }  
    nsig+=")";
    lfsig.push_back(nsig);
  }
 
  // add to help system
  if(TextDoc()!="") {
    for(unsigned int i=0; i< lfsig.size(); i++) {
      std::string topic= PlugIn();
      std::string key1=  KeywordAt(1);
      if(topic.length()>0) topic.at(0)=toupper(topic.at(0));
      if(key1.length()>0) key1.at(0)=toupper(key1.at(0));
      faudes_dict_insert_entry(topic,key1,lfsig.at(i));
    }
  }


  // set up wrapper frunction
  std::stringstream lfwrap;
  lfwrap << "-- LuaFunctionDefinition.Install() " << Name() << std::endl;
  lfwrap << std::endl;
  lfwrap << LuaCode();
  lfwrap << std::endl;
  lfwrap << std::endl;
  lfwrap << "-- LuaFunctionDefinition.Install(): dispatch / typecheck" << std::endl;
  lfwrap << "function faudes." << lfname << "(...)" << std::endl;
  lfwrap << "  local arg  = {...}" << std::endl; // Lua >= 5.2.x, omitt for earlier versions
  lfwrap << "  local argn = #arg" << std::endl;  // Lua >= 5.2.x, use "local argn=arg['n']" for earlier versions
  for(unsigned int i=0; i< lftest1.size(); i++) {
    lfwrap << "  if (" << lftest1.at(i) << ") then " << std::endl;
    lfwrap << "  if (" << lftest2.at(i) << ") then " << std::endl;
    lfwrap << "    return faudes." << lfcall.at(i) << std::endl;
    lfwrap << "  end" << std::endl;
    lfwrap << "  end" << std::endl;
  }
  lfwrap << "  faudes.Error([[" << lfname << ": parameter mismatch: expected signature(s): " << std::endl;
  for(unsigned int i=0; i< lfsig.size(); i++) {
    lfwrap << lfsig.at(i) << std::endl;
  }
  lfwrap << "]])" << std::endl;
  lfwrap << "end" << std::endl;


  // debugging report
  FD_DLB("LuaFunctionDefinition:Install(): code:" << std::endl << lfwrap.str());
  FD_DLB("LuaFunctionDefinition:Install(): code: done");

  // finally install (aka run) the wrapper
  int errexec=luaL_dostring(pLL,lfwrap.str().c_str());	
  if(errexec!=0) {
    std::string lerr= std::string(lua_tostring(pLL, -1));
    int c1 = lerr.find_first_of(':');
    if(c1<0) c1=0;
    int c2 = lerr.find_first_of(':',c1+1);
    if(c2<0) c2=1;
    std::string line = lerr.substr(c1+1,c2-c1-1);  
    if(c2>1) {
      lerr="error in Lua script: line " + line + ": " + lerr.substr(c2+2);
    }
    throw Exception("LuaFunctionDefinition::Install(): " + Name(), lerr, 49);
  }
}

// token io
void LuaFunctionDefinition::DoRead(TokenReader& rTr,  const std::string& rLabel, const Type* pContext) {
  FD_DLB("LuaFunctionDefinition::DoRead()");
  // ignore
  (void) pContext;
  // label
  std::string label=rLabel;
  if(label=="") label="LuaFunctionDefinition";
  // base can handle this
  FunctionDefinition::DoRead(rTr,label,pContext);
  // report
  FD_DLB("LuaFunctionDefinition::DoRead(): done " << mPlugIn << "::" << mName);
}


// register all definitions froma file with the function registry
void LuaFunctionDefinition::Register(const std::string& rFilename) {
  FD_DLB("LuaFunctionDefinition::Register(): file " << rFilename);
  TokenReader tr(rFilename);
  Token token;
  while(tr.Peek(token)) {
    if(!token.IsBegin("LuaFunctionDefinition")) {
      tr.Get(token);
      continue;
    }
    LuaFunctionDefinition*  plfd = new LuaFunctionDefinition();
    plfd->Read(tr);
    FD_DLB("LuaFunctionDefinition::Register(): found " << plfd->Name());
    if(FunctionRegistry::G()->Exists(plfd->Name())) {
      FD_DLB("LuaFunctionDefinition::Register(): skipping doublet " << plfd->Name());
      delete plfd;
      continue;
    }
    // Registry takes ownership
    FunctionRegistry::G()->Insert(plfd);
  }
}


// token io
void LuaFunctionDefinition::DoReadCore(TokenReader& rTr) {
  FD_DLB("LuaFunctionDefinition::DoReadCore()");
  // call base
  FunctionDefinition::DoReadCore(rTr);  
  // read my members
  Token token;
  rTr.Peek(token);
  // case a: embedded lua code
  if(token.IsBegin())
  if(token.StringValue()=="LuaCode") {
    mLuaFile="";
    rTr.ReadVerbatim("LuaCode",mLuaCode);
  }
  // case b: lua file
  if(token.IsBegin())
  if(token.StringValue()=="LuaFile") {
    rTr.ReadBegin("LuaFile");
    std::string mLuaFile=rTr.ReadString();
    // todo: read that file
    rTr.ReadEnd("LuaFile");
  }
}


// token io
void LuaFunctionDefinition::DoWrite(TokenWriter& rTw,  const std::string& rLabel, const Type* pContext) const {
   // label
  std::string label=rLabel;
  if(label=="") label="LuaFunctionDefinition";
  // base can handle
  Documentation::DoWrite(rTw,label,pContext); 
}

// token io
void LuaFunctionDefinition::DoWriteCore(TokenWriter& rTw) const {
  FD_DLB("LuaFunctionDefinition::DoWriteCore(): file " << rTw.FileName());
  // call base core
  FunctionDefinition::DoWriteCore(rTw);  
  // case a: embedded code
  if(mLuaFile=="") {
    rTw.WriteVerbatim("LuaCode",mLuaCode);
  }
  // case a: embedded code
  if(mLuaFile!="") {
    rTw.WriteBegin("LuaFile");
    rTw.WriteString(mLuaFile);
    rTw.WriteEnd("LuaFile");
    // todo: write that file
  }
}

/*
********************************************************************
********************************************************************
********************************************************************

Implementation of class LuaFunction

********************************************************************
********************************************************************
********************************************************************
*/

// Constructor 
LuaFunction::LuaFunction(const LuaFunctionDefinition* fdef) : 
  Function(fdef),
  pLuaFuncDef(fdef),
  pL(0),
  pLL(0)
{
  FD_DLB("LuaFunction::LuaFunction(): fdef  " << pFuncDef);
}

// new on heap
LuaFunction* LuaFunction::New(void) const {
  return new LuaFunction(pLuaFuncDef);
}

// set function definition
void LuaFunction::Definition(const FunctionDefinition* fdef) {
  // cast and pass to base
  pLuaFuncDef = dynamic_cast<const LuaFunctionDefinition*>(fdef);
  Function::Definition(pLuaFuncDef);
}

// get function definition
const LuaFunctionDefinition* LuaFunction::Definition(void) const {
  return pLuaFuncDef;
}

// set variant signature (accept -1)
void LuaFunction::DoVariant(int n) {
  // std case by base 
  if(n!=-1) {
    Function::DoVariant(n);
    return;
  }
  // special case -1
  mVariantIndex=-1;
  mParameterValues.clear(); 
}


// implement typecheck
bool LuaFunction::DoTypeCheck(int n) {
  FD_DLB("LuaFunction::DoTypeCheck("<< n << "): for " << Variant()->At(n).Type());
  const Type* proto = TypeRegistry::G()->Prototype(Variant()->At(n).Type()); 
  if(!proto) {
    FD_DLB("LuaFunction::DoTypeCheck("<< n << "): unknown type");
    return false;
  }
  if(!proto->Cast(ParamValue(n))) {
    FD_DLB("LuaFunction::DoTypeCheck("<< n << "): could not cast param value");
    return false;
  }
  return true;
}


// get/set lua state
LuaState* LuaFunction::L(void) {
  if(!pL) pL=Definition()->DefaultL();
  return pL;
}

// get/set default lua state
void LuaFunction::L(LuaState* l) {
 pL=l;
}


// implement execute: run stages
void LuaFunction::DoExecute(void) {
  FD_DLB("LuaFunction::DoExecute()");
  /*
  for(int i=0; i< Variant()->Size(); i++) {
    FD_DLB("LuaFunction::DoExecute(): value at #" << i);
    ParamValue(i)->Write();
  }
  */
  // use the global state
  if(!pL) pL=L();
  pLL=pL->LL();
  mEntryStack=lua_gettop(pLL);
  // run stages
  DoExecuteA();
  DoExecuteB();
  DoExecuteC();
  DoExecuteD();
  DoExecuteE();
  // done
  lua_settop(pLL,mEntryStack);
  FD_DLB("LuaFunction::DoExecute(): done");
}

// implement execute: syntaxcheck
void LuaFunction::SyntaxCheck(void) {
  FD_DLB("LuaFunction::SyntaxCheck()");
  if(!Variant()) {
    std::stringstream errstr;
    errstr << "no valid variant";
    throw Exception("LuaFunction::Syntaxcheck()", errstr.str(), 47);
  }
  // use the global state
  if(!pL) pL=L();
  pLL=pL->LL();
  mEntryStack=lua_gettop(pLL);
  // run stages
  DoExecuteA();
  DoExecuteB();
  // done
  lua_settop(pLL,mEntryStack);
  FD_DLB("LuaFunction::SyntaxCheck(): done");
}

// implement execute: plain evaluation
void LuaFunction::Evaluate(void) {
  FD_DLB("LuaFunction::SyntaxCheck()");
  // use the global state
  if(!pL) pL=L();
  pLL=pL->LL();
  mEntryStack=lua_gettop(pLL);
  // run stages
  DoExecuteA();
  // done
  lua_settop(pLL,mEntryStack);
  FD_DLB("LuaFunction::SyntaxCheck(): done");
}

// implement execute: run lua code i.e. define lua functions
void LuaFunction::DoExecuteA(void) {
  FD_DLB("LuaFunction::DoExecuteA()");
  // Lua stack: empty
  // load my script 
  const char* script = pLuaFuncDef->LuaCode().c_str();
  int script_len = pLuaFuncDef->LuaCode().size();
  int errload=luaL_loadbuffer(pLL, script, script_len, "luafaudes");
  if(errload!=0) {
    std::string lerr= std::string(lua_tostring(pLL, -1));
    int c1 = lerr.find_first_of(':');
    if(c1<0) c1=0;
    int c2 = lerr.find_first_of(':',c1+1);
    if(c2<0) c2=1;
    std::string line = lerr.substr(c1+1,c2-c1-1);  
    if(c2>1) {
      lerr="error in Lua script: line " + line + ": " + lerr.substr(c2+2);
    }
    lua_settop(pLL,mEntryStack);
    throw Exception("LuaFunction::DoExecuteA()", lerr, 49);
  }
  // Lua stack: script
  // install all functions aka run the script
  int errrun=lua_pcall(pLL, 0, 0, 0); 
  if(errrun!=0) {
    std::stringstream errstr;
    errstr << "failed to run script: ";
    errstr << std::string(lua_tostring(pLL, -1));
    lua_settop(pLL,mEntryStack);
    throw Exception("LuaFunction::DoExecuteA()", errstr.str(), 49);
  }
  // stack: []
}

// implement execute: find the function of this variant
void LuaFunction::DoExecuteB(void) {
  FD_DLB("LuaFunction::DoExecuteB()");
  // Lua stack: empty
  // get the swig generated table of faudes functions
  lua_getglobal(pLL,"faudes");
  mFtable=lua_gettop(pLL);
  if(!lua_istable(pLL,-1)) {
    lua_settop(pLL,mEntryStack);
    throw Exception("LuaFunction::DoExecuteB()", "failed to load faudes table", 49);
  }
  // stack: [faudes]
  // find my function: look up in faudes name space ...
  std::string fname = MangleString(Variant()->Name());
  lua_pushstring(pLL,fname.c_str());
  lua_gettable(pLL,mFtable);
  if(!lua_isfunction(pLL,-1)) {
    lua_pop(pLL, 1); // pop nil.
    lua_getglobal(pLL,fname.c_str()); // ... or as global (compatibility with pre 2.19)
    if(!lua_isfunction(pLL,-1)) {
      std::stringstream errstr;
      errstr << "missing function \"" << fname << "\"";
      lua_settop(pLL,mEntryStack);
      throw Exception("LuaFunction::DoExecuteB()", errstr.str(), 49);
    }
  }
  // stack: [faudes, luafnct]
  // construct a plain Type usrdata 
  lua_pushstring(pLL,"Type");
  lua_gettable(pLL,mFtable);
  if(!lua_isfunction(pLL,-1)) 
  if(!lua_istable(pLL,-1)) {   /* tmoor 201407 for swig 3.02 */
    lua_settop(pLL,mEntryStack);
    throw Exception("LuaFunction::DoExecuteB():", "failed to construct plain Type (1)", 49);
  }
  // stack: [faudes, luafnct, Type constructor]
  if(lua_pcall(pLL, 0, 1, 0) != 0) { 
    lua_settop(pLL,mEntryStack);
    throw Exception("LuaFunction::DoExecuteB():", "failed to construct plain Type (2)", 49);
  }
  // stack: [faudes, luafnct, Type variable]
  swig_lua_userdata* susr = (swig_lua_userdata*) lua_touserdata (pLL, -1);
  if(!susr) {
    lua_settop(pLL,mEntryStack);
    throw Exception("LuaFunction::DoExecuteB():", "failed to construct plain Type (3)", 49);
  }
  FD_DLB("LuaFunction::DoExecuteB(): plain type is " << susr->type->name);
  // record swig type of faudes plain Type
  mFType=susr->type;
  lua_pop(pLL, 1); 
  // stack: [faudes, luafnct]
}



// implement execute: prepare parameters
void LuaFunction::DoExecuteC(void) {
  FD_DLB("LuaFunction::DoExecuteC()");
  // stack: [faudes, luafnct]
  // interpret signature
  mLReturn.resize(Variant()->Size());
  mLParameter.resize(Variant()->Size());
  mLReturnCount=0;
  mLParameterCount=0;
  for(int i=0; i< Variant()->Size(); i++) {
    const std::string& ftype= Variant()->At(i).Type();
    // default: parameter except for explicit creturn && out
    mLParameter.at(i)= !(Variant()->At(i).CReturn() && (Variant()->At(i).Attribute()==Parameter::Out));
    // default: explicitly declared creturn 
    mLReturn.at(i)= Variant()->At(i).CReturn();
    // autofix elementary parameters 
    if((ftype=="Integer") || (ftype=="Boolean") || (ftype=="String")) {
      // ... out becomes return value only
      if(Variant()->At(i).Attribute()==Parameter::Out) {
        mLReturn.at(i)=true;
        mLParameter.at(i)=false;
      }
      // ... inout becomes return value and parameter
      if(Variant()->At(i).Attribute()==Parameter::InOut) {
        mLReturn.at(i)=true;
        mLParameter.at(i)=true;
      }
      // ... in becomes parameter only
      if(Variant()->At(i).Attribute()==Parameter::In) {
        mLReturn.at(i)=false;
        mLParameter.at(i)=true;
      }
    }
    // keep counts
    if(mLReturn.at(i)) mLReturnCount++;
    if(mLParameter.at(i)) mLParameterCount++;
  }

  FD_DLB("LuaFunction::DoExecuteC(): found " << mLReturnCount << " return values and " 
	 << mLParameterCount << " parameters");
  // stack: [faudes, luafnct]
  // construct my parameters in Lua
  for(int i=0; i< Variant()->Size(); i++) {
    const std::string& ftype= Variant()->At(i).Type();
    // skip non-parameter 
    if(!mLParameter.at(i)) {
      FD_DLB("LuaFunction::DoExecuteC(): skip nonparameter value pos " << i);
      continue;
    }
    // special case: int as lua number
    if(ftype=="Integer") {
      lua_pushnumber(pLL,((Integer*)ParamValue(i))->CValue());
      FD_DLB("LuaFunction::DoExecuteC(): created ftype " << ftype);
      continue;
    }
    // special case: bool as lua bool
    if(ftype=="Boolean") {
      lua_pushboolean(pLL,((Boolean*)ParamValue(i))->CValue());
      FD_DLB("LuaFunction::DoExecuteC(): created ftype " << ftype);
      continue;
    }
    // special case: str as lua string
    if(ftype=="String") {
      lua_pushstring(pLL,((String*)ParamValue(i))->CValue().c_str());
      FD_DLB("LuaFunction::DoExecuteC(): created ftype " << ftype);
      continue;
    }
    // std case: faudes type: construct 1
    lua_pushstring(pLL,ftype.c_str());
    lua_gettable(pLL,mFtable);
    if(!lua_isfunction(pLL,-1)) 
    if(!lua_istable(pLL,-1)) {   /* tmoor 201407 for swig 3.02 */
      std::stringstream errstr;
      errstr << "failed to load constructor for \"" << ftype << "\"";
      lua_settop(pLL,mEntryStack);
      throw Exception("LuaFunction::DoExecuteC()", errstr.str(), 49);
    }
    // std case: faudes type: construct 2
    if(lua_pcall(pLL, 0, 1, 0) != 0) { 
      std::stringstream errstr;
      errstr << "failed to construct for \"" << ftype << "\" (1)";
      lua_settop(pLL,mEntryStack);
      throw Exception("LuaFunction::DoExecuteC()", errstr.str(), 49);
    }
    // std case: test user data pointer
    swig_lua_userdata* susr = (swig_lua_userdata*) lua_touserdata (pLL, -1);
    if(!susr) {
      std::stringstream errstr;
      errstr << "failed to construct for\"" << ftype << "\" (2)";
      lua_settop(pLL,mEntryStack);
      throw Exception("LuaFunction::DoExecuteB()", errstr.str(), 49);
    }
    // inspect stack 
    FD_DLB("LuaFunction::DoExecuteC(): created stype " << susr->type->name << " for ftype " << ftype);
    FD_DLB("LuaFunction::DoExecuteC(): faudes parameter at " << ParamValue(i));
    FD_DLB("LuaFunction::DoExecuteC(): swig usrdata ptr  " << susr->ptr);
    FD_DLB("LuaFunction::DoExecuteC(): swig usrdata own flag  " << susr->own);
    /*
    // variant a: copy parameter value
    void* fptr=SwigCastPtr(susr->ptr,susr->type,(swig_type_info*)mFType);
    FD_DLB("LuaFunction::DoExecuteB(): faudes::Type converted ptr " << fptr);
    if(Variant()->At(i).Attribute()!=Parameter::Out) {
      FD_DLB("LuaFunction::DoExecuteB(): copy parameter value");
      ((Type*)fptr)->Assign(*ParamValue(i));
    }
    */
    // variant b: use references
    if(susr->own) free(susr->ptr);
    susr->own=0;
    susr->ptr = dynamic_cast<void*>(ParamValue(i)); // dynamic-up-cast: needed for multiple inheritance (!!)
  }
  // stack: [faudes, luafnct, rp_1 ... rp_n]
  FD_DLB("LuaFunction::DoExecuteC(): done");
}

// implement execute: execute the function
void LuaFunction::DoExecuteD(void) {
  FD_DLB("LuaFunction::DoExecuteD()");
  // stack: [faudes, luafnct, rp_1 ... rp_n]
  // duplicate all my parameters (incl. the actual function for convenience)
  // note: needed only for variant a to retrieve values of faudes typed parameters 
  // note: duplication is cheap since these are references
  int t1=lua_gettop(pLL)-mLParameterCount;
  int t2=lua_gettop(pLL);
  for(int i= t1; i<=t2; i++) {
    lua_pushvalue(pLL,i);
#ifdef  FAUDES_DEBUG_LUABINDINGS
    // report user data
    swig_lua_userdata* susr = (swig_lua_userdata*) lua_touserdata (pLL, -1);
    if(!susr) continue;
    // inspect user data stack 
    FD_DLB("LuaFunction::DoExecuteD(): inspect stype " << susr->type->name);
    FD_DLB("LuaFunction::DoExecuteD(): swig usrdata ptr  " << susr->ptr);
    FD_DLB("LuaFunction::DoExecuteD(): swig usrdata own flag  " << susr->own);
    void* fptr=SwigCastPtr(susr->ptr,susr->type,(swig_type_info*)mFType);
    FD_DLB("LuaFunction::DoExecuteD(): faudes::Type converted ptr " << fptr);
#endif
  }
  // stack: [faudes, luafnct, rp_1 ... rp_n, luafnct, rp_1 ... rp_n]
  // run my function
  if(lua_pcall(pLL, mLParameterCount, mLReturnCount, 0) != 0) { 
    FD_DLB("LuaFunction::DoExecuteD(): detect error");
    std::string lerr= std::string(lua_tostring(pLL, -1));
    // user request via loopback exception
    if(lerr.find("break on application request")!=std::string::npos) {
      lerr="break on application request";
    }
    // user request via explicit exception
    else if(lerr.find("luafaudes script:")!=std::string::npos) {
      std::size_t c1=lerr.find("luafaudes script:");
      lerr=lerr.substr(c1);
    }
    // have line number ?
    else if(lerr.size()>=2) {
      std::size_t c1 = lerr.find_first_of(':');
      if(c1==std::string::npos) c1=0;
      if(c1+1>=lerr.size()) c1=0;
      std::size_t c2 = lerr.find_first_of(':',c1+1);
      if(c2==std::string::npos) c2=c1+1;
      if(c2>c1+1) {
        std::string line = lerr.substr(c1+1,c2-c1-1);  
        lerr="error in Lua script: line " + line + ": " + lerr.substr(c2+2);
      }
    }
    // anyway: fix stack
    lua_settop(pLL,mEntryStack);
    throw Exception("LuaFunction::DoExecuteD()", lerr, 49);
  }
  FD_DLB("LuaFunction::DoExecuteD():done ");
  // stack: [faudes, luafnct, rp_1 ... rp_n, lp1 ... lpm]
}

// implement execute: retrieve results
void LuaFunction::DoExecuteE(void) {
  FD_DLB("LuaFunction::DoExecuteE()");
  // stack: [faudes, luafnct, rp_1 ... rp_n, lp1 ... lpm]
  // retrieve results from stack: return values
  for(int i=Variant()->Size()-1; i>=0; i--) {
    // skip non-return values
    if(!mLReturn.at(i)) continue;
    // switch on ftype
    const std::string& ftype= Variant()->At(i).Type();
    // int as lua number
    if(ftype=="Integer" && lua_isnumber(pLL,-1)) {
      FD_DLB("LuaFunction::DoExecuteE(): retrieve type " << ftype);
      ((Integer*)ParamValue(i))->CValue(lua_tonumber(pLL,-1));
      lua_pop(pLL, 1); 
      continue;
    }
    // bool as lua bool
    if(ftype=="Boolean" && lua_isboolean(pLL,-1)) {
      ((Boolean*)ParamValue(i))->CValue(lua_toboolean(pLL,-1));
      FD_DLB("LuaFunction::DoExecuteE(): retrieved type " << ftype);
      lua_pop(pLL, 1); 
      continue;
    }
    // str as lua string
    if(ftype=="String" && lua_isstring(pLL,-1)) {
      ((String*)ParamValue(i))->CValue(lua_tostring(pLL,-1));
      FD_DLB("LuaFunction::DoExecuteE(): retrieved type " << ftype);
      lua_pop(pLL, 1); 
      continue;
    }
    // report error
    std::stringstream errstr;
    errstr << "invalid return values in \"" << Variant()->Name() << "\"";
    lua_settop(pLL,mEntryStack);
    throw Exception("LuaFunction::DoExecuteE()", errstr.str(), 49);
  }

  // stack: [faudes, luafnct, rp_1 ... rp_n]

  /*
  // variant a: need to copy results

  // retrieve results from stack: duplicate references to parameters
  for(int i=Variant()->Size()-1; i>=0; i--) {
    // skip return values
    if(mLReturn.at(i)) continue;
    // switch on type type
    const std::string& ftype= Variant()->At(i).Type();
    // discrad/ignore duplicate parameters with access In
    if(Variant()->At(i).Attribute()==Parameter::In) {
      FD_DLB("LuaFunction::DoExecuteD(): ignore in-paremeter of type " << ftype);
      lua_pop(pLL, 1); 
      continue;
    }
    // discrad/ignore elementary types (lua call by value)
    if((ftype=="Integer") || (ftype=="Boolean") || (ftype=="String")) {
      FD_DLB("LuaFunction::DoExecuteD(): ignore elementary type parameter " << ftype);
      lua_pop(pLL, 1); 
      continue;
    }
    // std case: get updated value
    if(lua_isuserdata(pLL,-1)) {
      FD_DLB("LuaFunction::DoExecuteD(): try to retrieve type " << ftype);
      swig_lua_userdata* susr = (swig_lua_userdata*) lua_touserdata(pLL, -1);
      if(susr) {
        FD_DLB("LuaFunction::DoExecuteD(): found swig type " << susr->type->name << " at " << susr->ptr);
        FD_DLB("LuaFunction::DoExecuteD(): swig usrdata ptr " << susr->ptr);
        FD_DLB("LuaFunction::DoExecuteD(): swig usrdata own flag " << susr->own);
        void* fptr=SwigCastPtr(susr->ptr,susr->type,(swig_type_info*)mFType);
        FD_DLB("LuaFunction::DoExecuteD(): swig usrdata ptr faudes::Type converted " << fptr);
        FD_DLB("LuaFunction::DoExecuteD(): parameter value " << ParamValue(i));
        FD_DLB("LuaFunction::DoExecuteD(): copy parameter value");
        ParamValue(i)->Assign(*((Type*)fptr));
        // pop
        lua_pop(pLL, 1); 
        continue;
      }
    }

  */

  // variant b: discard duplicate parameters (did not need them for variant b anyway)
  for(int i=Variant()->Size()-1; i>=0; i--) {
    if(mLReturn.at(i)) continue;
    lua_pop(pLL, 1); 
  }

} 



/*
********************************************************************
********************************************************************
********************************************************************

LuaState implementation

********************************************************************
********************************************************************
********************************************************************
*/

// include Mike Pall's completer 
#include "lbp_completion.cpp"


// construct/destruct
LuaState::LuaState(void) : mpLL(0) { Open(); }
LuaState::~LuaState(void) { Close(); }   

//access
lua_State* LuaState::LL(void) { return mpLL; }

// re-init
void LuaState::Reset(void) { 
  // have a new state
  Close(); 
  Open(); 
  // install from function registry
  FunctionRegistry::Iterator fit=FunctionRegistry::G()->Begin();
  for(;fit!=FunctionRegistry::G()->End(); fit++) {
    const LuaFunctionDefinition* lfd=dynamic_cast<const LuaFunctionDefinition*>(fit->second);
    if(!lfd) continue;
    lfd->Install(mpLL);
  }
}

// install extension
void LuaState::Install(const std::string& rFilename) {
  Install(mpLL, rFilename);
}

// static state (convenience)
LuaState* LuaState::G(void) {
  static LuaState* sls=NULL;
  if(!sls) sls=new LuaState();
  return sls;
}

// open/close lua state
void LuaState::Open(void) {
  Close();
  FD_DLB("LuaState::Open()");
  mpLL=luaL_newstate();
  Initialize(mpLL);
  faudes_print_register(mpLL);  // todo:  configure this
  faudes_hook_register(mpLL);   // todo:  configure this
  FD_DLB("LuaState::Done()");
}

// open/close lua state
void LuaState::Close(void) {
  if(!mpLL) return;
  FD_DLB("LuaState::Close()");
  lua_close(mpLL);
  mpLL=NULL;
}

// lua style interface to initialize lua state
void LuaState::Initialize(lua_State* pLL) {
  lua_gc(pLL, LUA_GCSTOP, 0);     /* stop collector during initialization */
  luaL_openlibs(pLL);             /* open libraries */
  luaopen_faudes_allplugins(pLL); /* install my namespace */
  lua_gc(pLL, LUA_GCRESTART, 0);  /* restart collector */
}



// convenience function: load all luafunctions defined in a file
void LuaState::Install(lua_State* pLL, const std::string& rFilename) {
  FD_DLB("LuaState::Insatll(): file " << rFilename);
  TokenReader tr(rFilename);
  Token token;
  while(tr.Peek(token)) {
    // todo: figure title  and ... faudes_dict_insert_topic(const std::string& topic, const std::string& text);
    if(!token.IsBegin("LuaFunctionDefinition")) {
      tr.Get(token);
      continue;
    }
    LuaFunctionDefinition lfd;
    lfd.Read(tr);
    FD_DLB("LuaState::Install(): found " << lfd.Name());
    lfd.Install(pLL);
  }
}

// push a faudes typed object on the stack
void LuaState::Push(const Type* fdata) {
  Push(mpLL,fdata);
}

// push a faudes typed object on the stack
void LuaState::Push(lua_State* pLL, const Type* fdata) {
  FD_DLB("LuaFunction::Push()");
  int savetop=lua_gettop(pLL);
  // stack: []
  // get the swig generated table of faudes functions
  lua_getglobal(pLL,"faudes");
  int ftable=lua_gettop(pLL);
  if(!lua_istable(pLL,-1)) {
    lua_settop(pLL,savetop);
    throw Exception("LuaState::Push()", "failed to load faudes table", 49);
  }
  // stack: [faudes]
  lua_pushstring(pLL,"Type");
  lua_gettable(pLL,ftable);
  if(!lua_isfunction(pLL,-1)) 
  if(!lua_istable(pLL,-1)) {   /* tmoor 201607 for swig 3.02 */
    lua_settop(pLL,savetop);
    throw Exception("LuaState::Push()", "failed to construct plain Type (1)", 49);
  }
  // stack: [faudes, Type constructor]
  if(lua_pcall(pLL, 0, 1, 0) != 0) { 
    lua_settop(pLL,savetop);
    throw Exception("LuaState::Push()", "failed to construct plain Type (2)", 49);
  }
  // stack: [faudes, Type variable]
  swig_lua_userdata* susr = (swig_lua_userdata*) lua_touserdata (pLL, -1);
  if(!susr) {
    lua_settop(pLL,savetop);
    throw Exception("LuaState::Push()", "failed to construct plain Type (3)", 49);
  }
  FD_DLB("LuaState::Push(): plain type is " << susr->type->name);
  // record swig type of faudes plain Type
  swig_type_info* stype =susr->type;
  lua_pop(pLL, 1); 
  // stack: [faudes]
  // construct my data in Lua
  const std::string& ftype= FaudesTypeName(*fdata);
  // special case: int as lua number
  if(ftype=="Integer") {
    lua_pushnumber(pLL,((const Integer&)*fdata));
    FD_DLB("LuaState::Push(): created ftype " << ftype);
  }
  // special case: bool as lua bool
  else if(ftype=="Boolean") {
    lua_pushboolean(pLL,((const Boolean&)*fdata));
    FD_DLB("LuaState::Push(): created ftype " << ftype);
  }
  // special case: str as lua string
  else if(ftype=="String") {
    lua_pushstring(pLL,((const String&)*fdata).CValue().c_str());
    FD_DLB("LuaState::Push(): created ftype " << ftype);
  }
  // std case: faudes type
  else {
    // construct 1
    lua_pushstring(pLL,ftype.c_str());
    lua_gettable(pLL,ftable);
    if(!lua_isfunction(pLL,-1)) 
    if(!lua_istable(pLL,-1)) {   /* tmoor 201607 for swig 3.02 */
      std::stringstream errstr;
      errstr << "failed to load constructor for \"" << ftype << "\"";
      lua_settop(pLL,savetop);
      throw Exception("LuaState::Push()", errstr.str(), 49);
    }
    // construct 2
    if(lua_pcall(pLL, 0, 1, 0) != 0) { 
      std::stringstream errstr;
      errstr << "failed to construct for \"" << ftype << "\" (1)";
      lua_settop(pLL,savetop);
      throw Exception("LuaState::Push()", errstr.str(), 49);
    }
    // test user data pointer
    swig_lua_userdata* susr = (swig_lua_userdata*) lua_touserdata (pLL, -1);
    if(!susr) {
      std::stringstream errstr;
      errstr << "failed to construct for\"" << ftype << "\" (2)";
      lua_settop(pLL,savetop);
      throw Exception("LuaState::Push()", errstr.str(), 49);
    }
    // inspect stack 
    FD_DLB("LuaState::Push(): created stype " << susr->type->name << " for ftype " << ftype);
    FD_DLB("LuaState::Push(): faudes parameter at " << fdata);
    FD_DLB("LuaState::Push(): swig usrdata ptr  " << susr->ptr);
    FD_DLB("LuaState::Push(): swig usrdata own flag  " << susr->own);
    // cast to plain Type
    void* fptr=SwigCastPtr(susr->ptr,susr->type,stype);
    FD_DLB("LuaState::Push(): faudes plain Type converted ptr " << fptr);
    if(!fptr) {
      std::stringstream errstr;
      errstr << "failed cast from " << ftype;
      lua_settop(pLL,savetop);
      throw Exception("LuaState::Push()", errstr.str(), 49);
    }
    ((Type*)fptr)->Assign(*fdata);
  }
  // stack: [faudes, fdata]
  lua_replace(pLL,-2);
  FD_DLB("LuaFunction::DoExecuteB(): done");
}

// pop a faudes typed object from the stack
Type* LuaState::Pop(void) {
  return Pop(mpLL);
}

// pop a faudes typed object from the stack
Type* LuaState::Pop(lua_State* pLL) {
  FD_DLB("LuaFunction::Pop()");
  int savetop=lua_gettop(pLL);
  Type* res=NULL;
  // stack: [fobject]
  // special cases: integer
  if(lua_isnumber(pLL,-1)) {
    res= new Integer(lua_tointeger(pLL,-1));
    lua_pop(pLL,1);
    return res;
  }
  // special cases: string
  if(lua_isstring(pLL,-1)) {
    res= new String(lua_tostring(pLL,-1));
    lua_pop(pLL,1);
    return res;
  }
  // special cases: string
  if(lua_isboolean(pLL,-1)) {
    res= new Boolean(lua_toboolean(pLL,-1));
    lua_pop(pLL,1);
    return res;
  }
  // stack: [fobject]
  // get the swig generated table of faudes functions
  lua_getglobal(pLL,"faudes");
  int ftable=lua_gettop(pLL);
  if(!lua_istable(pLL,-1)) {
    lua_settop(pLL,savetop);
    throw Exception("LuaState::Pop()", "failed to load faudes table", 49);
  }
  // stack: [fobject, faudes]
  // construct a plain Type usrdata 
  lua_pushstring(pLL,"Type");
  lua_gettable(pLL,ftable);
  if(!lua_isfunction(pLL,-1)) 
  if(!lua_istable(pLL,-1)) {   /* tmoor 201607 for swig 3.02 */
    lua_settop(pLL,savetop);
    throw Exception("LuaState::Pop()", "failed to construct plain Type (1)", 49);
  }
  // stack: [fobject, faudes, Type constructor]
  if(lua_pcall(pLL, 0, 1, 0) != 0) { 
    lua_settop(pLL,savetop);
    throw Exception("LuaState::Pop()", "failed to construct plain Type (2)", 49);
  }
  // stack: [fobject, faudes, Type variable]
  swig_lua_userdata* susr = (swig_lua_userdata*) lua_touserdata (pLL, -1);
  if(!susr) {
    lua_settop(pLL,savetop);
    throw Exception("LuaState::Pop()", "failed to construct plain Type (3)", 49);
  }
  FD_DLB("LuaState::Pop(): plain type is " << susr->type->name);
  // record swig type of faudes plain Type
  swig_type_info* stype =susr->type;
  lua_pop(pLL, 2); 
  // stack: [fobject]
  // test user data pointer 1 (this could be any user data, so we use SwigUserData to test)
  susr = SwigUserData(pLL, -1);
  if(!susr) {
    std::stringstream errstr;
    errstr << "unknown data type";
    lua_settop(pLL,savetop);
    throw Exception("LuaState::Pop()", errstr.str(), 49);
  }
  // cast to faudes Type
  void* fptr=SwigCastPtr(susr->ptr,susr->type,stype);
  FD_DLB("LuaState::Pop(): swig  usrdata ptr " << susr->ptr);
  FD_DLB("LuaState::Pop(): swig stype " << susr->type->name);
  FD_DLB("LuaState::Pop(): faudes::Type converted ptr " << fptr);
  if(!fptr) {
    std::stringstream errstr;
    errstr << "faild to cast stype \"" << susr->type->name << "\" to plain Type";
    lua_settop(pLL,savetop);
    throw Exception("LuaState::Pop()", errstr.str(), 49);
  }
  // copy data
  res=((Type*)fptr)->Copy();
  // stack: [fobject]
  lua_pop(pLL,1);
  FD_DLB("LuaFunction::Pop(): done");
  return res;
}

// access global variables
Type* LuaState::Global(const std::string& gname, const Type* fdata) {
  return Global(mpLL,gname,fdata);
}

// access global variables
Type* LuaState::Global(lua_State* pLL, const std::string& gname, const Type* fdata) {
  // get
  if(fdata==NULL) {
    //lua_getfield(pLL, LUA_GLOBALSINDEX, gname.c_str());
    lua_getglobal(pLL,gname.c_str());
    return Pop(pLL);
  }
  // set
  else {
    Push(pLL,fdata);
    //lua_setfield(pLL, LUA_GLOBALSINDEX, gname.c_str());   
    lua_setglobal(pLL,gname.c_str());
    return 0;
  }
}


// evaluate a Lua expression
void LuaState::Evaluate(const std::string& expr) {
  Evaluate(mpLL,expr);
}

// evaluate a Lua expression
void LuaState::Evaluate(lua_State* pLL, const std::string& expr) {
  FD_DLB("LuaFunction::Evaluate()");

  // record top of stack
  int top = lua_gettop(pLL);

  // load
  std::string cerr;
  if(cerr=="")
  if(luaL_loadbuffer(pLL, expr.c_str(), expr.size(), "string")) {
    cerr=std::string(lua_tostring(pLL, -1));
    lua_pop(pLL, 1);
  }

  // evaluate
  if(cerr=="")
  if(lua_pcall(pLL, 0, LUA_MULTRET, 0)){
    cerr=std::string(lua_tostring(pLL, -1));
    lua_pop(pLL, 1);
  }

  // fix stack (ignore results etc)
  lua_settop(pLL,top);
  lua_gc(pLL, LUA_GCCOLLECT, 0);

  // throw on error
  if(cerr!="") {
    lua_settop(pLL,top);
    throw Exception("LuaState::Evaluate()", cerr, 49);
  }

  return;
}


// completion
std::list< std::string > LuaState::Complete(const std::string& word) {
  return Complete(mpLL,word);
}

// evaluate a Lua expression
std::list< std::string >  LuaState::Complete(lua_State* pLL, const std::string& word) {
  FD_DLB("LuaFunction::Complete(" << word <<")");
  return faudes_rl_complete(pLL,word); 
}



} // namespace

