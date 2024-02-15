/** 

@file op_interface.i 

SWIG interface for observer plugin. The observer plugin 
defines a number of additional functions and an aditional 
data-type for its relabeling maps. This SWIG interface
definition makes the extensions available to luafaudes.

See the luabindings README for more documentation.

Note: this interface has been edited to accomodate
for (experimental) Python bindings.
 
**/


/*
**************************************************
**************************************************
**************************************************

Observer interface preface

**************************************************
**************************************************
**************************************************
*/

// Set SWIG module name (must match libFAUDES plugin name)
%module observer
#ifndef SwigModule
#define SwigModule "SwigObserver"
#endif


// Load std faudes interface
%include "faudesmodule.i"



// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy observer to faudes name space
for k,v in pairs(observer) do faudes[k]=v end
}
#endif

/*
**************************************************
**************************************************
**************************************************

Observer interface: data types

**************************************************
**************************************************
**************************************************
*/

// extend relabel map to support inspection as string
class EventRelabelMap : public Type {
 public:
  %extend {
    std::string ToString(void) { 
      EventSet eset;
      std::stringstream str;
      std::map<Idx, std::set<Idx> >::const_iterator i;
      for(i=$self->StlMap().begin(); i!=$self->StlMap().end(); i++) {
        str << eset.Str(i->first) << " -> { ";
        std::set<Idx>::const_iterator j;
        for(j= i->second.begin(); j!= i->second.end(); j++) { 
          str << eset.Str(*j) << " ";
        }
        str << "}" << std::endl;
      };
      return str.str();
    };
    std::string __str__(void) { return self->ToString(); }
  };
};





/*
**************************************************
**************************************************
**************************************************

Observer interface: plugin functions vio rti

**************************************************
**************************************************
**************************************************
*/


// Add entry to mini help system: introduce new topic "observer"
SwigHelpTopic("Observer","Observer plugin functions");

// Include rti generated functioninterface 
#if SwigModule=="SwigObserver"
%include "../../../include/rtiautoload.i"
#endif
