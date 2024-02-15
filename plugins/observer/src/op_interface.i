/** 

@file op_interface.i 

SWIG interface for observer plugin. The observer plugin 
defines a number of additional functions and an aditional 
data-type for its relabeling maps. This SWIG interface
definition makes the extensions available to luafaudes.

See the luabindings README for more documentation. 
 
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

// Set SWIG module name
// Note: must match libFAUDES plugin name
%module observer

// Load std SWIG includes
%include <std_string.i>
%include <std_except.i>

// Import core faudes interface
// Note: this includes swig macros 
%import "corefaudes.i"

// Extra c code in wrapper file
%{

// include relevant faudes headers
#include "corefaudes.h"
#include "op_include.h"

%}

// Extra Lua functions: copy to faudes name space
%luacode {
-- Copy observer to faudes name space
for k,v in pairs(observer) do faudes[k]=v end
}


// Extra Lua functions: debugging
#ifdef FAUDES_DEBUG_LUABINDINGS
%luacode {
-- Say hello
print("libFAUDES: observer plugin")
}
#endif

// Extra c code in wrapper file
%{
// use libfaudes namespace
using namespace faudes;

// access mini help system
extern void luafaudes_dict_insert_topic(const std::string& topic, const std::string& entry);
extern void luafaudes_dict_insert_entry(const std::string& topic, const std::string& key, const std::string& entry);

%}



/*
**************************************************
**************************************************
**************************************************

Observer interface: data types

**************************************************
**************************************************
**************************************************
*/

// extend relabel map to support Lua string conversion
class EventRelabelMap : public Type {
 public:
  %extend {
    std::string __str__(void) { 
      EventSet eset;
      std::stringstream str;
      std::map<Idx, std::set<Idx> >::const_iterator i;
      for(i=$self->STLMap().begin(); i!=$self->STLMap().end(); i++) {
        str << eset.Str(i->first) << " -> { ";
        std::set<Idx>::const_iterator j;
        for(j= i->second.begin(); j!= i->second.end(); j++) { 
          str << eset.Str(*j) << " ";
        }
        str << "}" << std::endl;
      };
      return str.str();
    };
  };
};



/*
**************************************************
**************************************************
**************************************************

Observer interface: plugin functions

**************************************************
**************************************************
**************************************************
*/

// *** observer computation

/*
// Tell SWIG with nice target names
%rename(CalcAbstAlphObs) calcAbstAlphObs;
void calcAbstAlphObs(System& rGenObs, EventSet& rHighAlph, EventSet& rNewHighAlph, EventRelabelMap&  rMapRelabeledEvents); 
%rename(InsertRelabeledEvents) insertRelabeledEvents;
void insertRelabeledEvents(System& rGenPlant, const EventRelabelMap& rMapRelabeledEvents, Alphabet& rNewEvents);
void insertRelabeledEvents(System& rGenPlant, const EventRelabelMap& rMapRelabeledEvents);
*/


// *** observer verification

// Tell SWIG with nice target names
%rename(IsObserver) IsObs;
bool IsObs(const Generator& rLowGen, const EventSet& rHighAlph);
%rename(IsLocalControlConsistent) IsLCC;
bool IsLCC(const System& rLowGen, const EventSet& rHighAlph);


/*
**************************************************
**************************************************
**************************************************

Observer interface: mini help system

**************************************************
**************************************************
**************************************************
*/

// Add entry to mini help system: introduce new topic "observer"
SwigHelpTopic("Observer","Observer plugin functions");

// Add entries to mini help system: introduce observer functions
SwigHelpEntry("Observer","observer synthesis"," CalcAbstAlphObs(Generator,EventSet,EventSet,EventRelabelMap)");
SwigHelpEntry("Observer","observer synthesis"," InsertRelabeledEvents(Generator,EventRelabelMap,EventSet)");
SwigHelpEntry("Observer","observer synthesis"," InsertRelabeledEvents(Generator,EventRelabelMap)");
SwigHelpEntry("Observer","observer tests","bool IsObserver(Generator,EventSet)");
SwigHelpEntry("Observer","observer tests","bool IsLocalControlConsitent(Generator,EventSet)");
