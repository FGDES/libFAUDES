/** 

@file faudesmodule.i 

common SWIG include file for all libFAUDES plugins. PlugIns that
provide luabindings are meant to include this file in their interface
definition for convenience. It sets up the namespace and imports
the corefaudes interface definitions.
 
**/

#ifndef FAUDESMODULE_I
#define FAUDESMODULE_I

// Fix clash between Lua string-library and SWIG std::string wrappers
// (obsolete as of  SWIG 4.xx series)
//#ifdef SWIGLUA
//%rename(CppString) std::string;
//#endif

// Load std SWIG includes
%include <typemaps.i>
%include <stdint.i>
%include <std_string.i>
%include <std_except.i>

// Fix INOUT for std string (obsolete as of  SWIG 4.xx series)
//#ifdef SWIGLUA
//%apply std::string& INOUT {std::string& }
//#endif

// Import corefaudes interface ...
// ... except we are the core module (or the overall monolithic module), then 
// will be included rather then imported
#if SwigModule != "SwigCoreFaudes"
#if SwigModule != "SwigLibFaudes"
%import "corefaudes.i"
#endif
#endif

// let SWIG know the configuration
%include "../../../include/configuration.h"

// Extra C code in wrapper file (will NOT generate bindings)
%{

// include faudes headers (inc. also pbp_include.h, lbp_include.h etc)
#include "libfaudes.h"
#include "swg_utils.h"
  

// use libfaudes namespace
using namespace faudes;


// make microsoft compiler happy
#ifdef _MSC_VER
#pragma warning(disable:4503)
#endif

%}


// Fix import issue: import does not account for
// extra c code --- which is fine for actual code but
// misses out on typedefs
%{
namespace faudes {
typedef NameSet::Iterator NameSetIterator;
typedef NameSet::Iterator EventSetIterator;
typedef IndexSet::Iterator IndexSetIterator;
typedef IndexSet::Iterator StateSetIterator;
}
%}


// Extra Python functions: debugging
#ifdef SWIGPYTHON
#ifdef FAUDES_DEBUG_PYBINDINGS
%pythoncode {
print("faudes bindings: loading", SwigModule)
}  
#endif
#endif


// Extra Lua functions: debugging
#ifdef SWIGLUA
#ifdef FAUDES_DEBUG_LUABINDINGS
%luacode {
print("faudes bindings: loading", SwigModule)
}
#endif
#endif

// guard faudesmodule.i
#endif
