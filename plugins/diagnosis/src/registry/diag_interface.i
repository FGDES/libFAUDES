/** 

@file diag_interface.i 

SWIG interface for diagnosis plugin. The diagnosis plugin 
defines a number of additional data-types for the diagnosis
of discrete event systems. This SWIG interface definition makes 
the extensions available to luafaudes. The interface for functions is
also genrated by SWIG, but uses input from the rti data base.

See the luabindings README for more documentation. 

**/


/*
**************************************************
**************************************************
**************************************************

Diagnosis interface preface

**************************************************
**************************************************
**************************************************
*/

// Set SWIG module name
// Note: must match libFAUDES plugin name
%module diagnosis
#ifndef SwigModule
#define SwigModule "SwigDiagnosis"
#endif

// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy hiosys to faudes name space
for k,v in pairs(diagnosis) do faudes[k]=v end
}
#endif

/*
**************************************************
**************************************************
**************************************************

Diagnosis interface: FailureTypeMap

**************************************************
**************************************************
**************************************************
*/

/*
Notes:
- this is a minimal interface only (basically token io)
- the failure map is an attribute only for historic reasons
*/


// rename to get rid of historic prefix
%rename(FailureTypeMap) AttributeFailureTypeMap;

class AttributeFailureTypeMap : public AttributeFlags {
public:
//  TaNameSet<AttributeFailureEvents> mFailureTypeMap;
  AttributeFailureTypeMap(void);
  AttributeFailureTypeMap(const std::string& rFileName);
  ~AttributeFailureTypeMap(void);
  bool Empty(void) const;
  Idx AddFailureTypeMapping(std::string failureType, const EventSet& rfailureEvents);
  Idx FailureType(Idx failureEvent) const;
  EventSet AllFailureEvents(void) const;
};


/*
**************************************************
**************************************************
**************************************************

Diagnosis interface: diagGen

**************************************************
**************************************************
**************************************************
*/

// Diagnoser: define attribute related members as a macro
%define SwigTdiagGeneratorMembers(GEN,G_ATTR,S_ATTR,E_ATTR,T_ATTR)
  Idx InsFailureTypeMapping(const std::string& failureType, const EventSet& rfailureEvents);
%enddef

// Diagnoser template
template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
class TdiagGenerator : public Generator {
public:
  // Construct/destruct
  SwigTaGeneratorConstructors(TdiagGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TaGenerator members not inherited from Generator
  SwigTaGeneratorMembers(TdiagGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TdiagGenerator members not inherited from TaGenerator
  SwigTdiagGeneratorMembers(TdiagGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
};

// Announce diagnoser class to swig
%template(Diagnoser) TdiagGenerator<AttributeFailureTypeMap, AttributeDiagnoserState, AttributeCFlags, AttributeVoid>;

// Tell swig that our generator is also known as Diagnoser
typedef TdiagGenerator<AttributeFailureTypeMap, AttributeDiagnoserState, AttributeCFlags, AttributeVoid> Diagnoser;



/*
**************************************************
**************************************************
**************************************************

Diagnosis interface: mini help system

**************************************************
**************************************************
**************************************************
*/

// Add entry to mini help system: introduce new topic "hiosys"
SwigHelpTopic("Diagnosis","Diagnosis PlugIn");

// Add entries to mini help system: data types
SwigHelpEntry("Diagnosis","Types for Diagnosis", "Diagnoser Generator with state estimate attributes (minimal lua interface)");
SwigHelpEntry("Diagnosis","Types for Diagnosis", "FailureTypeMap (minimal lua interface)");


// Include rti generated function interface
#if SwigModule=="SwigDiagnosis"
%include "../../../include/rtiautoload.i"
#endif
