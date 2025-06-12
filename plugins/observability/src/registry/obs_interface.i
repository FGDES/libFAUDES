/** 

@file obs_interface.i 

SWIG interface for observability plugin. The observability plugin 
defines functions only, hence, actual bindings will be generated
from rti data base.

See the luabindings README for more documentation. 

**/


/*
**************************************************
**************************************************
**************************************************

Observability interface preface

**************************************************
**************************************************
**************************************************
*/

// Set SWIG module name
// Note: must match libFAUDES plugin name
%module observability
#ifndef SwigModule
#define SwigModule "SwigObservability"
#endif

// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy observability to faudes name space
for k,v in pairs(observability) do faudes[k]=v end
}
#endif


/*
**************************************************
**************************************************
**************************************************

Observability interface: mini help system

**************************************************
**************************************************
**************************************************
*/

// Add entry to mini help observability: introduce new topic "observability"
SwigHelpTopic("Observability","Observability PlugIn");

// Include rti generated function interface
#if SwigModule=="SwigObservability"
%include "rtiautoload.i"
#endif
