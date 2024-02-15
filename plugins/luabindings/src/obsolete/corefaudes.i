// SWIG core module 
%module faudes

// Modul name as macro (sense, whether this has been set) 
#ifndef SwigModule
#define SwigModule "SwigCoreFaudes"
#endif

// Std faudes includes
%include "faudesmodule.i"

// include corefaudes interfaces
%include "commonfaudes.i"
%include "attributes.i"
%include "containers.i"
%include "generators.i"
%include "vectors.i"
%include "functions.i"


// Extra Lua functions: Copy to global name space
%luacode {
-- Copy faudes to global name space
function faudes.MakeGlobal()
  for k,v in pairs(faudes) do _G[k]=v end
end
}

// include rti generated interface
#if SwigModule=="SwigCoreFaudes"
%include "../../../include/rtiautoload.i"
#endif
