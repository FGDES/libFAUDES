/** 

@file mtc_interface.i 

SWIG interface for multitasking plugin. The multitasking plugin 
defines three additional data-types, namely the faudes::ColorSet,
faudes::AttributeColoredState and faudes::MtcSystem. This SWIG 
interface definition makes the extensions available to luafaudes. 
The interface for functions is also genrated by SWIG, but uses 
input from the rti data base.

See the luabindings README for more documentation. 

**/


/*
**************************************************
**************************************************
**************************************************

Multitasking interface preface

**************************************************
**************************************************
**************************************************
*/

// Set SWIG module name
// Note: must match libFAUDES plugin name
%module multitasking
#ifndef SwigModule
#define SwigModule "SwigMultitasking"
#endif

// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy multitasking to faudes name space
for k,v in pairs(multitasking) do faudes[k]=v end
}
#endif


/*
**************************************************
**************************************************
**************************************************

Multitasking interface: ColorSet

**************************************************
**************************************************
**************************************************
*/


// Set with symbolic colors
class ColorSet : public NameSet {
public:
  // Use TBaseSet constructors
  SwigBaseSetConstructors(ColorSet,Idx,NameSetIterator)
};


/*
**************************************************
**************************************************
**************************************************

Multitasking interface: AttributeColoredState

**************************************************
**************************************************
**************************************************
*/


class AttributeColoredState : public AttributeFlags {
public:
  // construct/destruct
  AttributeColoredState(void);
  AttributeColoredState(const AttributeColoredState&);
  // extra access to color features
  const ColorSet& Colors(void) const;
  ColorSet& Colors(void);
  void Colors(const ColorSet& rColors);
  NameSetIterator ColorsBegin() const;
  NameSetIterator ColorsEnd() const;
};


/*
**************************************************
**************************************************
**************************************************

Multitasking interface: mtgGenerator

**************************************************
**************************************************
**************************************************
*/

// MtSystem: define attribute related members as a macro
%define SwigTmtcGeneratorMembers(GEN,G_ATTR,S_ATTR,E_ATTR,T_ATTR)
  // edit colors
 Idx InsColoredState(const std::string& rStateName, const std::string& rColorName);
 Idx InsColoredState(const std::string& rStateName, Idx colorIndex);
 Idx InsColoredState(const std::string& rStateName, const ColorSet& rColors);
 Idx InsColoredState(const ColorSet& rColors);
 Idx InsColor(Idx stateIndex, const std::string& IN);
 void InsColor(Idx stateIndex, Idx colorIndex);
 void InsColors(Idx stateIndex, const ColorSet& rColors);
 void DelColor(Idx stateIndex, const std::string& rColorName);
 void DelColor(Idx stateIndex, Idx colorIndex);
 void DelColor(const std::string& rColorName);
 void DelColor(Idx colorIndex);
 void ClrColors(Idx stateIndex);
 // access colors
 void Colors(ColorSet& rColors) const;
 ColorSet Colors(void) const;
 const ColorSet& Colors(Idx stateIndex) const;
 StateSet ColoredStates(Idx colorIndex) const;
 StateSet ColoredStates(const std::string& rColorName) const;
 StateSet ColoredStates() const;
 StateSet UncoloredStates() const;
 bool ExistsColor(Idx colorIndex) const;
 bool ExistsColor(const std::string& rColorName) const;
 bool ExistsColor(Idx stateIndex, Idx colorIndex) const;
 bool IsColored() const;
 bool IsColored(Idx stateIndex) const;  
 // colored reachability
 StateSet StronglyCoaccessibleSet(void) const;
 bool StronglyCoaccessible(void);
 bool IsStronglyCoaccessible(void) const;
 StateSet StronglyTrimSet(void) const;
 bool StronglyTrim(void);
 bool IsStronglyTrim(void) const;
%enddef

// MtcSystem template
template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
class TmtcGenerator : public Generator {
public:
  // Construct/destruct
  SwigTaGeneratorConstructors(TmtcGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TaGenerator members not inherited from Generator
  SwigTaGeneratorMembers(TmtcGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TcGenerator members not inherited from aGenerator
  SwigTcGeneratorMembers(TmtcGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TmtcGenerator members not inherited from TaGenerator
  SwigTmtcGeneratorMembers(TmtcGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
};

// Announce MtcSystem class to swig as MtcSystem
%template(MtcSystem) TmtcGenerator<AttributeVoid,AttributeColoredState,AttributeCFlags,AttributeVoid>;

// Tell swig that our generator is also known as MtcSystem
typedef TmtcGenerator<AttributeVoid,AttributeColoredState,AttributeCFlags,AttributeVoid> MtcSystem;



/*
**************************************************
**************************************************
**************************************************

Multitasking interface: mini help system

**************************************************
**************************************************
**************************************************
*/

// Add entry to mini help system: introduce new topic
SwigHelpTopic("Multitasking","Multitasking PlugIn");

// Add entries to mini help system: data types
SwigHelpEntry("Multitasking","Types for Multitasking", "MtcSystem System with colored states (full Lua interface)");
SwigHelpEntry("Multitasking","Types for Multitasking", "ColorSet Set of colors (Lua interface as NameSet)");
SwigHelpEntry("Multitasking","Types for Multitasking", "AttributeColoredState Attribute with ColorSet (access via Colors())");


// Include rti generated function interface
#if SwigModule=="SwigMultitasking"
%include "../../../include/rtiautoload.i"
#endif
