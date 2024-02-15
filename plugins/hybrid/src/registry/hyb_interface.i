/** 

@file hyb_interface.i 

SWIG interface for hybrid plugin. This is is template
to be completed in due course

See the luabindings README for more documentation. 
 
**/


/*
**************************************************
**************************************************
**************************************************

Example interface preface

**************************************************
**************************************************
**************************************************
*/

// Set SWIG module name
// Note: must match libFAUDES plugin name
%module hybrid
#ifndef SwigModule
#define SwigModule "SwigHybrid"
#endif

// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy hybrid to faudes name space
for k,v in pairs(hybrid) do faudes[k]=v end
}
#endif

/*
**************************************************
**************************************************
**************************************************

Example interface: plugin function(s)

**************************************************
**************************************************
**************************************************
*/

// Add topic to mini help system
SwigHelpTopic("Hybird","Hybrid PlugIn function");

// not implemented: interaface to data types, i.e., lha-system

// Include rti generated functioninterface 
#if SwigModule=="SwigHybrid"
%include "../../../include/rtiautoload.i"
#endif
