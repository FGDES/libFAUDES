/** 

@file syn_interface.i 

SWIG interface for synthesis plugin. The synthesis plugin 
defines functions only, hence, actual bindings will be generated
from rti data base.

See the luabindings README for more documentation. 

Note: this interface has been edited to accomodate
for (experimental) Python bindings.

**/


/*
**************************************************
**************************************************
**************************************************

Synthesis interface preface

**************************************************
**************************************************
**************************************************
*/

// Set SWIG module name
// Note: must match libFAUDES plugin name
%module synthesis
#ifndef SwigModule
#define SwigModule "SwigSynthesis"
#endif


// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy synthesis to faudes name space
for k,v in pairs(synthesis) do faudes[k]=v end
}
#endif				 



/*
**************************************************
**************************************************
**************************************************

Synthesis interface: auto generated

**************************************************
**************************************************
**************************************************
*/

// Add entry to mini help system: introduce new topic "synthesis"
SwigHelpTopic("Synthesis","Synthesis PlugIn");

// Include rti generated function interface
#if SwigModule=="SwigSynthesis"
%include "../../../include/rtiautoload.i"
#endif


/*
**************************************************
**************************************************
**************************************************

Synthesis interface: extra interface SWIG only

**************************************************
**************************************************
**************************************************
*/

bool IsControllable(const Generator&, const EventSet&, const Generator&, StateSet& rCritical);
SwigHelpEntry("Synthesis", "Controllability", "Boolean IsControllable(+In+ Generator GPlant, +In+ EventSet ACntrl, +In+ Generator GCand, +Out+ StateSet Critical)");
