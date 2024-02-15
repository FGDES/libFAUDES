/** 

@file pex_interface.i 

SWIG interface for example plugin. This is a particulary simple
case, since it defines only one additional function and no
data-types. 

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
%module example
#ifndef SwigModule
#define SwigModule "SwigExample"
#endif

// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy example to faudes name space
for k,v in pairs(example) do faudes[k]=v end
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
SwigHelpTopic("Example","Example plugin function");


// Explicit interface to my one and only function
// ... this is now done via the faudes rti.
/*
// Tell SWIG about my function
void AlternativeAccessible(Generator& rGen);
// Add entry to help system	       
SwigHelpEntry("Example","Functions", " AlternativeAccessible(gen)");
*/

// Include rti generated functioninterface 
#if SwigModule=="SwigExample"
%include "../../../include/rtiautoload.i"
#endif
