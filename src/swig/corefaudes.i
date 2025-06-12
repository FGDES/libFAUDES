/** 

@file corefaudes.i 

SWIG interface for corefaudes library.


**/

// Set Modul name
// (must be faude for corefaudes)
%module faudes
#ifndef SwigModule
#define SwigModule "SwigCoreFaudes"
#endif

// basic faudes module
%include "faudesmodule.i"

// include corefaudes interfaces
%include "coreaddons.i"
%include "basetype.i"
%include "attributes.i"
%include "containers.i"
%include "generators.i"
%include "vectors.i"
%include "functions.i"


// Extra Lua functions: Copy to global name space
#ifdef SWIGLUA
%luacode {
-- Copy faudes to global name space
function faudes.MakeGlobal()
  for k,v in pairs(faudes) do _G[k]=v end
end
}				
#endif
				
// include rti generated interface
#if SwigModule=="SwigCoreFaudes"
%include "rtiautoload.i"
#endif
