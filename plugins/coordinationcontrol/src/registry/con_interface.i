/** @file con_interface.i @brief luabindings basic types */

/**

SWIG interface for coordinationcontrol plug-in. The coordinationcontrol plugin 
defines functions only, hence, actual bindings will be generated
from rti data base.

See the luabindings README for more documentation. 

**/


/*
**************************************************
**************************************************
**************************************************

Conditionadecomposability interface preface

**************************************************
**************************************************
**************************************************
*/

// Set SWIG module name
// Note: must match libFAUDES plugin name
%module coordinationcontrol
#ifndef SwigModule
#define SwigModule "SwigCoordinationControl"
#endif

// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy coordinationcontrol to faudes name space
for k,v in pairs(coordinationcontrol) do faudes[k]=v end
}
#endif


/*
**************************************************
**************************************************
**************************************************

CoordinationControl interface: mini help system

**************************************************
**************************************************
**************************************************
*/

// Add entry to mini help coordinationcontrol: introduce new topic "coordinationcontrol"
SwigHelpTopic("CoordinationControl","CoordinationControl PlugIn");

// Include rti generated function interface
#if SwigModule=="SwigCoordinationControl"
%include "../../../include/rtiautoload.i"
#endif
