/** 

@file faudesmodule.i 

SWIG include file for libFAUDES plugins. PlugIns that provide
luabindings are meant to include this file in their interface
definition for convenience. It sets up the namespace, imports
the corefaudes interface definitions, and adds additional c-code 
to access the mini help system.
 
**/

#ifndef Swig_FAUDESMODULE_I
#define Swig_FAUDESMODULE_I

// Fix clash between Lua string-library and SWIG std::string wrappers
%rename(CppString) std::string;

// Load std SWIG includes
%include <typemaps.i>
%include <stdint.i>
%include <std_string.i>
%include <std_except.i>

// Fix INOUT for std string
%apply std::string& INOUT {std::string& }


// Import core faudes interface ...
// ... except we are the core module, then 
// they get included rather then imported
#if SwigModule != "SwigCoreFaudes"
%import "corefaudes.i"
#endif

// let SWIG know the configuration
%include "../../../include/configuration.h"

// Extra c code in wrapper file
%{

// include faudes headers (inc. also lbp_include.h)
#include "libfaudes.h"

// use libfaudes namespace
using namespace faudes;

// access mini help system
extern void luafaudes_dict_insert_topic(const std::string& topic, const std::string& entry);
extern void luafaudes_dict_insert_entry(const std::string& topic, const std::string& key, const std::string& entry);

// make microsoft compiler happy
#ifdef _MSC_VER
#pragma warning(disable:4503)
#endif

%}


// Fix import issue: import does not account for
// extra c code --- which is fine for actual code but
// misses out on typedefs.
%{
namespace faudes {
typedef NameSet::Iterator NameSetIterator;
typedef NameSet::Iterator EventSetIterator;
typedef IndexSet::Iterator IndexSetIterator;
typedef IndexSet::Iterator StateSetIterator;
}
%}


// Extra Lua functions: debugging
#ifdef FAUDES_DEBUG_LUABINDINGS
%luacode {
-- Say hello
print("libFAUDES: loading", SwigModule)
}
#endif

// load once
#endif
