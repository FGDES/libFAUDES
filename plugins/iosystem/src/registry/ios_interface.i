/** 

@file ios_interface.i 

SWIG interface for iosystem plug-in. The iosystem plug-in 
defines a number of additional data-types for io systems 
and functions that operate on io systems. This SWIG interface 
definition makes the extensions available to luafaudes.

See the luabindings README for more documentation. 

**/


/*
**************************************************
**************************************************
**************************************************

iosystem interface preface

**************************************************
**************************************************
**************************************************
*/

// Set SWIG module name
// Note: must match libFAUDES plugin name
%module iosystem
#ifndef SwigModule
#define SwigModule "SwigIoSystem"
#endif

// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy iosystem to faudes name space
for k,v in pairs(iosystem) do faudes[k]=v end
}
#endif

/*
**************************************************
**************************************************
**************************************************

IoSystem interface: IoSystem

**************************************************
**************************************************
**************************************************
*/

// IoSystem: define attribute members as a macro
%define SwigTioGeneratorMembers(GEN,G_ATTR,S_ATTR,E_ATTR,T_ATTR)
  
  // Convenience vent insertion 
  void InsOutputEvent(Idx index);
  Idx InsOutputEvent(const std::string& rName);
  void InsInputEvent(Idx index);
  Idx InsInputEvent(const std::string& rName);

  // Event attributes
  EventSet OutputEvents(void) const;
  bool OutputEvent(Idx index) const;
  bool OutputEvent(const std::string& rName) const;
  void SetOutputEvent(Idx index);
  void SetOutputEvent(const std::string& rName);
  void SetOutputEvent(const EventSet& rEventSet);
  void ClrOutputEvent(Idx index);
  void ClrOutputEvent(const std::string& rName);
  void ClrOutputEvent(const EventSet& rEventSet);
  EventSet InputEvents(void) const;
  bool InputEvent(Idx index) const;
  bool InputEvent(const std::string& rName) const;
  void SetInputEvent(Idx index);
  void SetInputEvent(const std::string& rName);
  void SetInputEvent(const EventSet& rEventSet);
  void ClrInputEvent(Idx index);
  void ClrInputEvent(const std::string& rName);
  void ClrInputEvent(const EventSet& rEventSet);

  // State attributes
  StateSet OutputStates(void) const;
  bool OutputState(Idx index) const;
  bool OutputState(const std::string& rName) const;
  void SetOutputState(Idx index);
  void SetOutputState(const std::string& rName);
  void SetOutputState(const StateSet& rStateSet);
  void ClrOutputState(Idx index);
  void ClrOutputState(const std::string& rName);
  void ClrOutputState(const StateSet& rStateSet);
  StateSet InputStates(void) const;
  bool InputState(Idx index) const;
  bool InputState(const std::string& rName) const;
  void SetInputState(Idx index);
  void SetInputState(const std::string& rName);
  void SetInputState(const StateSet& rStateSet);
  void ClrInputState(Idx index);
  void ClrInputState(const std::string& rName);
  void ClrInputState(const StateSet& rStateSet);
  StateSet ErrorStates(void) const;
  bool ErrorState(Idx index) const;
  bool ErrorState(const std::string& rName) const;
  void SetErrorState(Idx index);
  void SetErrorState(const std::string& rName);
  void SetErrorState(const StateSet& rStateSet);
  void ClrErrorState(Idx index);
  void ClrErrorState(const std::string& rName);
  void ClrErrorState(const StateSet& rStateSet);

%enddef

// IoSystem template
template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
class TioGenerator : public Generator {
public:
  // Construct/destruct
  SwigTaGeneratorConstructors(TioGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TaGenerator members not inherited from Generator
  SwigTaGeneratorMembers(TioGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TioGenerator members not inherited from TaGenerator
  SwigTioGeneratorMembers(TioGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
};

// Announce  IoSystem class to swig
%template(IoSystem) TioGenerator<AttributeVoid, AttributeIosState, AttributeIosEvent, AttributeVoid>;

// Tell swig that our generator is also known as IoSystem
typedef TioGenerator<AttributeVoid, AttributeIosState, AttributeIosEvent, AttributeVoid> IoSystem;

// Announce io style alphabet to swig
%template(IoAlphabet) TaNameSet<AttributeIosEvent>;



/*
**************************************************
**************************************************
**************************************************

iosystem interface: mini help system

**************************************************
**************************************************
**************************************************
*/

// Add entry to mini help system: introduce new topic "IoSystem"
SwigHelpTopic("IoSystem","I/O System PlugIn");

// Add entries to mini help system: data types
SwigHelpEntry("IoSystem","io systems"," Access to state- and event-attributes as in C++ API");
SwigHelpEntry("IoSystem","io systems","IoSystem plant/controller model");



// Include rti generated function interface
#if SwigModule=="SwigIoSystem"
%include "rtiautoload.i"
#endif
