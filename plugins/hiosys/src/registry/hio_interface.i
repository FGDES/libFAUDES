/** 

@file hio_interface.i 

SWIG interface for hiosys plugin. The hiosys plugin 
defines a number of additional data-types for hio systems 
and functions that operate on hio systems. This SWIG interface 
definition makes the extensions available to luafaudes.

See the luabindings README for more documentation. 

**/


/*
**************************************************
**************************************************
**************************************************

Hiosys interface preface

**************************************************
**************************************************
**************************************************
*/

// Set SWIG module name
// Note: must match libFAUDES plugin name
%module hiosys
#ifndef SwigModule
#define SwigModule "SwigHioSys"
#endif

// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy hiosys to faudes name space
for k,v in pairs(hiosys) do faudes[k]=v end
}
#endif

/*
**************************************************
**************************************************
**************************************************

Hiosys interface: HioPlant

**************************************************
**************************************************
**************************************************
*/

// HioPlant: define attribute members as a macro
%define SwigTHioPlantMembers(GEN,G_ATTR,S_ATTR,E_ATTR,T_ATTR)

  // Event attributes: Up/Yp
  void InsYpEvent(Idx index);
  Idx InsYpEvent(const std::string& rName);
  void InsUpEvent(Idx index);
  Idx InsUpEvent(const std::string& rName);
  void SetYp(Idx index);
  void SetYp(const std::string& rName);
  void SetYp(const EventSet& rEvents);
  void SetUp(Idx index);
  void SetUp(const std::string& rName);
  void SetUp(const EventSet& rEvents);
  bool IsYp(Idx index) const;
  bool IsYp(const std::string& rName) const;
  bool IsUp(Idx index) const;
  bool IsUp(const std::string& rName) const;
  EventSet YpEvents(void) const;
  EventSet UpEvents(void) const;

  // Event attributes: Ue/Ye
  void InsYeEvent(Idx index);
  Idx InsYeEvent(const std::string& rName);
  void InsUeEvent(Idx index);
  Idx InsUeEvent(const std::string& rName);
  void SetYe(Idx index);
  void SetYe(const std::string& rName);
  void SetYe(const EventSet& rEvents);
  void SetUe(Idx index);
  void SetUe(const std::string& rName);
  void SetUe(const EventSet& rEvents);
  bool IsYe(Idx index) const;
  bool IsYe(const std::string& rName) const;
  bool IsUe(Idx index) const;
  bool IsUe(const std::string& rName) const;
  EventSet YeEvents(void) const;
  EventSet UeEvents(void) const;

  // Query attribute disjunctions
  bool IsY(Idx index) const;
  bool IsY(const std::string& rName) const;
  EventSet YEvents(void) const;
  bool IsU(Idx index) const;
  bool IsU(const std::string& rName) const;
  EventSet UEvents(void) const;
  bool IsP(Idx index) const;
  bool IsP(const std::string& rName) const;
  EventSet PEvents(void) const;
  bool IsE(Idx index) const;
  bool IsE(const std::string& rName) const;    
  EventSet EEvents(void) const;

  // State attributes
  bool IsQYpYe(Idx index) const;
  bool IsQYpYe(const std::string& rName) const;
  StateSet QYpYeStates(void) const;
  bool IsQUp(Idx index) const;
  bool IsQUp(const std::string& rName) const;
  StateSet QUpStates(void) const;
  bool IsQUe(Idx index) const;
  bool IsQUe(const std::string& rName) const;
  StateSet QUeStates(void) const;
  void SetErr(Idx index);
  void SetErr(const std::string& rName);
  void SetErr(const StateSet& rStates);
  void ClrErr(Idx index);
  void ClrErr(const std::string& rName);
  void ClrErr(const StateSet& rStates);
  bool IsErr(Idx index) const;
  bool IsErr(const std::string& rName) const;
  StateSet ErrStates(void) const;
%enddef

// HioPlant template
template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
class THioPlant : public Generator {
public:
  // Construct/destruct
  SwigTaGeneratorConstructors(THioPlant,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // Additional constructor
  THioPlant(const Generator&, const EventSet& rYp, const EventSet& rUp, 
    const EventSet& rYe, const EventSet& rUe);
  // TaGenerator members not inherited from Generator
  SwigTaGeneratorMembers(THioPlant,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // THioPlant members not inherited from TaGenerator
  SwigTHioPlantMembers(THioPlant,GlobalAttr,StateAttr,EventAttr,TransAttr);
};

// Announce  HioPlant class to swig
%template(HioPlant) THioPlant<AttributeVoid, HioStateFlags, HioEventFlags, AttributeVoid>;

// Tell swig that our generator is also known as HioPlant
typedef THioPlant<AttributeVoid, HioStateFlags, HioEventFlags, AttributeVoid> HioPlant;

// Announce Hio style alphabet to swig
%template(HioAlphabet) TaNameSet<HioEventFlags>;



/*
**************************************************
**************************************************
**************************************************

Hiosys interface: HioController

**************************************************
**************************************************
**************************************************
*/

// HioController: define attribute members as a macro
%define SwigTHioControllerMembers(GEN,G_ATTR,S_ATTR,E_ATTR,T_ATTR)

  // Event attributes: Up/Yp
  void InsYpEvent(Idx index);
  Idx InsYpEvent(const std::string& rName);
  void InsUpEvent(Idx index);
  Idx InsUpEvent(const std::string& rName);
  void SetYp(Idx index);
  void SetYp(const std::string& rName);
  void SetYp(const EventSet& rEvents);
  void SetUp(Idx index);
  void SetUp(const std::string& rName);
  void SetUp(const EventSet& rEvents);
  bool IsYp(Idx index) const;
  bool IsYp(const std::string& rName) const;
  bool IsUp(Idx index) const;
  bool IsUp(const std::string& rName) const;
  EventSet YpEvents(void) const;
  EventSet UpEvents(void) const;

  // Event attributes: Uc/Yc
  void InsYcEvent(Idx index);
  Idx InsYcEvent(const std::string& rName);
  void InsUcEvent(Idx index);
  Idx InsUcEvent(const std::string& rName);
  void SetYc(Idx index);
  void SetYc(const std::string& rName);
  void SetYc(const EventSet& rEvents);
  void SetUc(Idx index);
  void SetUc(const std::string& rName);
  void SetUc(const EventSet& rEvents);
  bool IsYc(Idx index) const;
  bool IsYc(const std::string& rName) const;
  bool IsUc(Idx index) const;
  bool IsUc(const std::string& rName) const;
  EventSet YcEvents(void) const;
  EventSet UcEvents(void) const;

  // Query attribute disjunctions
  bool IsY(Idx index) const;
  bool IsY(const std::string& rName) const;
  EventSet YEvents(void) const;
  bool IsU(Idx index) const;
  bool IsU(const std::string& rName) const;
  EventSet UEvents(void) const;
  bool IsP(Idx index) const;
  bool IsP(const std::string& rName) const;
  EventSet PEvents(void) const;
  bool IsC(Idx index) const;
  bool IsC(const std::string& rName) const;    
  EventSet CEvents(void) const;

  // State attributes
  bool IsQYP(Idx index) const;
  bool IsQYP(const std::string& rName) const;
  StateSet QYPStates(void) const;
  bool IsQUp(Idx index) const;
  bool IsQUp(const std::string& rName) const;
  StateSet QUpStates(void) const;
  bool IsQUc(Idx index) const;
  bool IsQUc(const std::string& rName) const;
  StateSet QUcStates(void) const;
  bool IsQYcUp(Idx index) const;
  bool IsQYcUp(const std::string& rName) const;
  StateSet QYcUpStates(void) const;
  void SetErr(Idx index);
  void SetErr(const std::string& rName);
  void SetErr(const StateSet& rStates);
  void ClrErr(Idx index);
  void ClrErr(const std::string& rName);
  void ClrErr(const StateSet& rStates);
  bool IsErr(Idx index) const;
  bool IsErr(const std::string& rName) const;
  StateSet ErrStates(void) const;
  
%enddef

// HioController template
template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
class THioController : public Generator {
public:
  // Construct/destruct
  SwigTaGeneratorConstructors(THioController,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // Additional constructor
  THioController(const Generator& rOtherGen, const EventSet& rYc, 
    const EventSet& rUc, const EventSet& rYp, const EventSet& rUp);
  // TaGenerator members not inherited from Generator
  SwigTaGeneratorMembers(THioController,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // THioController members not inherited from TaGenerator
  SwigTHioControllerMembers(THioController,GlobalAttr,StateAttr,EventAttr,TransAttr);
};

// Announce  HioController class to swig
%template(HioController) THioController<AttributeVoid, HioStateFlags, HioEventFlags, AttributeVoid>;

// Tell swig that our generator is also known as HioController
typedef THioController<AttributeVoid, HioStateFlags, HioEventFlags, AttributeVoid> HioController;



/*
**************************************************
**************************************************
**************************************************

Hiosys interface: HioEnvironment

**************************************************
**************************************************
**************************************************
*/

// HioEnvironment: define attribute members as a macro
%define SwigTHioEnvironmentMembers(GEN,G_ATTR,S_ATTR,E_ATTR,T_ATTR)

  // Event attributes: Ue/Ye
  void InsYeEvent(Idx index);
  Idx InsYeEvent(const std::string& rName);
  void InsUeEvent(Idx index);
  Idx InsUeEvent(const std::string& rName);
  void SetYe(Idx index);
  void SetYe(const std::string& rName);
  void SetYe(const EventSet& rEvents);
  void SetUe(Idx index);
  void SetUe(const std::string& rName);
  void SetUe(const EventSet& rEvents);
  bool IsYe(Idx index) const;
  bool IsYe(const std::string& rName) const;
  bool IsUe(Idx index) const;
  bool IsUe(const std::string& rName) const;
  EventSet YeEvents(void) const;
  EventSet UeEvents(void) const;

  // Event attributes: Ul/Yl
  void InsYlEvent(Idx index);
  Idx InsYlEvent(const std::string& rName);
  void InsUlEvent(Idx index);
  Idx InsUlEvent(const std::string& rName);
  void SetYl(Idx index);
  void SetYl(const std::string& rName);
  void SetYl(const EventSet& rEvents);
  void SetUl(Idx index);
  void SetUl(const std::string& rName);
  void SetUl(const EventSet& rEvents);
  bool IsYl(Idx index) const;
  bool IsYl(const std::string& rName) const;
  bool IsUl(Idx index) const;
  bool IsUl(const std::string& rName) const;
  EventSet YlEvents(void) const;
  EventSet UlEvents(void) const;

  // Query attribute disjunctions
  bool IsY(Idx index) const;
  bool IsY(const std::string& rName) const;
  EventSet YEvents(void) const;
  bool IsU(Idx index) const;
  bool IsU(const std::string& rName) const;
  EventSet UEvents(void) const;
  bool IsE(Idx index) const;
  bool IsE(const std::string& rName) const;
  EventSet EEvents(void) const;
  bool IsL(Idx index) const;
  bool IsL(const std::string& rName) const;    
  EventSet LEvents(void) const;

  // State attributes
  bool IsQYe(Idx index) const;
  bool IsQYe(const std::string& rName) const;
  StateSet QYeStates(void) const;
  bool IsQUe(Idx index) const;
  bool IsQUe(const std::string& rName) const;
  StateSet QUeStates(void) const;
  bool IsQUl(Idx index) const;
  bool IsQUl(const std::string& rName) const;
  StateSet QUlStates(void) const;
  bool IsQYlUe(Idx index) const;
  bool IsQYlUe(const std::string& rName) const;
  StateSet QYlUeStates(void) const;
  void SetErr(Idx index);
  void SetErr(const std::string& rName);
  void SetErr(const StateSet& rStates);
  void ClrErr(Idx index);
  void ClrErr(const std::string& rName);
  void ClrErr(const StateSet& rStates);
  bool IsErr(Idx index) const;
  bool IsErr(const std::string& rName) const;
  StateSet ErrStates(void) const;

%enddef

// HioEnvironment template
template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
class THioEnvironment : public Generator {
public:
  // Construct/destruct
  SwigTaGeneratorConstructors(THioEnvironment,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // Additional constructor
  THioEnvironment(const Generator& rOtherGen, const EventSet& rYe,
   const EventSet& rUe, const EventSet& rYl, const EventSet& rUl); 
  // TaGenerator members not inherited from Generator
  SwigTaGeneratorMembers(THioEnvironment,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // THioEnvironment members not inherited from TaGenerator
  SwigTHioEnvironmentMembers(THioEnvironment,GlobalAttr,StateAttr,EventAttr,TransAttr);
};

// Announce  HioEnvironment class to swig
%template(HioEnvironment) THioEnvironment<AttributeVoid, HioStateFlags, HioEventFlags, AttributeVoid>;

// Tell swig that our generator is also known as HioEnvironment
typedef THioEnvironment<AttributeVoid, HioStateFlags, HioEventFlags, AttributeVoid> HioEnvironment;



/*
**************************************************
**************************************************
**************************************************

Hiosys interface: HioConstraint

**************************************************
**************************************************
**************************************************
*/

// HioConstraint: define attribute members as a macro
%define SwigTHioConstraintMembers(GEN,G_ATTR,S_ATTR,E_ATTR,T_ATTR)

  // Event attributes: U/Y
  void InsYEvent(Idx index);
  Idx InsYEvent(const std::string& rName);
  void InsUEvent(Idx index);
  Idx InsUEvent(const std::string& rName);
  void SetY(Idx index);
  void SetY(const std::string& rName);
  void SetY(const EventSet& rEvents);
  void SetU(Idx index);
  void SetU(const std::string& rName);
  void SetU(const EventSet& rEvents);
  bool IsY(Idx index) const;
  bool IsY(const std::string& rName) const;
  bool IsU(Idx index) const;
  bool IsU(const std::string& rName) const;
  EventSet YEvents(void) const;
  EventSet UEvents(void) const;

  // State attributes
  bool IsQY(Idx index) const;
  bool IsQY(const std::string& rName) const;
  StateSet QYStates(void) const;
  bool IsQU(Idx index) const;
  bool IsQU(const std::string& rName) const;
  StateSet QUStates(void) const;
  void SetErr(Idx index);
  void SetErr(const std::string& rName);
  void SetErr(const StateSet& rStates);
  void ClrErr(Idx index);
  void ClrErr(const std::string& rName);
  void ClrErr(const StateSet& rStates);
  bool IsErr(Idx index) const;
  bool IsErr(const std::string& rName) const;
  StateSet ErrStates(void) const;

%enddef

// HioConstraint template
template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
class THioConstraint : public Generator {
public:
  // Construct/destruct
  SwigTaGeneratorConstructors(THioConstraint,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // Additional constructor
  THioConstraint(const Generator& rOtherGen, const EventSet& rY, const EventSet& rU);
  // TaGenerator members not inherited from Generator
  SwigTaGeneratorMembers(THioConstraint,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // THioConstraint members not inherited from TaGenerator
  SwigTHioConstraintMembers(THioConstraint,GlobalAttr,StateAttr,EventAttr,TransAttr);
};

// Announce  HioConstraint class to swig
%template(HioConstraint) THioConstraint<AttributeVoid, HioStateFlags, HioEventFlags, AttributeVoid>;

// Tell swig that our generator is also known as HioEnvironment
typedef THioConstraint<AttributeVoid, HioStateFlags, HioEventFlags, AttributeVoid> HioConstraint;



/*
**************************************************
**************************************************
**************************************************

Hiosys interface: functions

**************************************************
**************************************************
**************************************************
*/




// **** Test Hio properties: controller form

bool IsHioControllerForm(
  HioController& rHioController,
  StateSet& rQUc,
  StateSet& rQYP,
  StateSet& rQUp,
  StateSet& rQYcUp,
  EventSet& rErrEvSet,
  TransSet& rErrTrSet,
  StateSet& rErrStSet,
  std::string& rStrOut);


// **** Test Hio properties: environment form

bool IsHioEnvironmentForm(
  HioEnvironment& rHioEnvironment,
  StateSet& rQYe,
  StateSet& rQUe,
  StateSet& rQUl,
  StateSet& rQYlUe,
  EventSet& rErrEvSet,
  TransSet& rErrTrSet,
  StateSet& rErrStSet,
  std::string& rStrOut);


// **** Test Hio properties: environment form

bool IsHioConstraintForm(
  HioConstraint& rHioConstraint,
  StateSet& rQY,
  StateSet& rQU,
  EventSet& rErrEvSet,
  TransSet& rErrTrSet,
  StateSet& rErrStSet,
  std::string& rStrOut);


  


/*
**************************************************
**************************************************
**************************************************

Hiosys interface: mini help system

**************************************************
**************************************************
**************************************************
*/

// Add entry to mini help system: introduce new topic "hiosys"
SwigHelpTopic("HioSys","HioSys PlugIn");

// Add entries to mini help system: data types
SwigHelpEntry("HioSys","io systems"," Access to state- and event-attributes as in C++ API");
SwigHelpEntry("HioSys","io systems","HioPlant plant model");
SwigHelpEntry("HioSys","io systems","HioController controller dynamics");
SwigHelpEntry("HioSys","io systems","HioEnvironment environment model");
SwigHelpEntry("HioSys","io systems","HioConstraint external constraint");



// Include rti generated function interface
#if SwigModule=="SwigHioSys"
%include "rtiautoload.i"
#endif
