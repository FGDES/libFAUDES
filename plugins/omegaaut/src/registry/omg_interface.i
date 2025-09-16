/** @file omg_interface.i @brief bindings for omegaaut plug-in */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2025  Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */


/*
**************************************************
**************************************************
**************************************************

faudes std header

**************************************************
**************************************************
**************************************************
*/

// Set SWIG module name
// Note: must match libFAUDES plugin name
%module omegaaut
#ifndef SwigModule
#define SwigModule "SwigOmegaAut"
#endif

// Load std faudes interface
%include "faudesmodule.i"

// extra c code for swig
%{

%}


/*
**************************************************
**************************************************
**************************************************

Interface (data types)

**************************************************
**************************************************
**************************************************
*/

// faudes type RabinPair
class RabinPair : public ExtType {
public:
  // constructors/destructor
  RabinPair(void);
  RabinPair(const RabinPair& rRP);
  virtual ~RabinPair(void) {};
  // my accessors/overloads
  virtual void Clear(void);
  StateSet& RSet(void) {return mRSet;};
  //const StateSet& RSet(void) const {return mRSet;}; // mute const
  StateSet& ISet(void) {return mISet;};
  //const StateSet& ISet(void) const {return mISet;}; // mute const
  void RestrictStates(const StateSet& rDomain);
  bool operator<(const RabinPair& rOther) const;
};


// faudes fake type RabinParVector
%template(RabinPairVector) TBaseVector<RabinPair>;
typedef TBaseVector<RabinPair> RabinPairVector;

// faudes type RabinAcceptance:
class RabinAcceptance : public RabinPairVector {
public:
  // constructors/destructor
  RabinAcceptance(void);
  RabinAcceptance(const RabinAcceptance& rRA);
  RabinAcceptance(const std::string& rFileName);
  virtual ~RabinAcceptance(void) {};  
  // my members 
  void RestrictStates(const StateSet& rDomain);
};

// Rabin automaton extrqa menbers
%define SwigTrGeneratorMembers(GEN,G_ATTR,S_ATTR,E_ATTR,T_ATTR)
  void RabinAcceptance(const RabinAcceptance& rRabAcc);
  //const RabinAcceptance&  RabinAcceptance(void) const; // mute const access
  RabinAcceptance&  RabinAcceptance(void);
  void RabinAcceptance(const StateSet& rMarking);
  //void RabinAcceptanceWrite(void) const; // not yet implemented
  virtual void RestrictStates(const StateSet& rStates);
%enddef


// faudes type Rabin automaton template
template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
  class TrGenerator : public Generator {
public:
  // Construct/destruct
  SwigTaGeneratorConstructors(TrGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TaGenerator members not inherited from Generator
  SwigTaGeneratorMembers(TrGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TcGenerator members not inherited from TaGenerator
  SwigTcGeneratorMembers(TrGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TrGenerator members not inherited from TrGenerator
  SwigTrGeneratorMembers(TrGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
};

// run Rabin automaton template and announce result as RabinAutomaton
%template(RabinAutomaton) TrGenerator<RabinAcceptance,AttributeVoid,AttributeCFlags,AttributeVoid>;

// tell swig that we alo know our generator RabinAutomaton
typedef TrGenerator<RabinAcceptance,AttributeVoid,AttributeCFlags,AttributeVoid> RabinAutomaton;



/*

// CY's interface, kept for reference --- to be updated or removed  

// RabinAutomaton typedef
typedef TrGenerator<RabinAcceptance, AttributeVoid, AttributeCFlags, AttributeVoid> RabinAutomaton;

// AugmentedEvent class
class AugmentedEvent : public AttributeVoid {
public:
    AugmentedEvent(void);
    AugmentedEvent(Idx event, const EventSet& pattern);
    AugmentedEvent(const AugmentedEvent& other);
    
    Idx Event() const;
    void Event(Idx event);
    const EventSet& ControlPattern() const;
    void ControlPattern(const EventSet& pattern);
    
    bool operator==(const AugmentedEvent& other) const;
    bool operator!=(const AugmentedEvent& other) const;
};

// AugmentedAlphabet class
class AugmentedAlphabet : public TaGenerator<AugmentedEvent, AttributeVoid, AttributeVoid, AttributeVoid> {
public:
    AugmentedAlphabet(void);
    AugmentedAlphabet(const AugmentedAlphabet& other);
    
    void InsertAugmentedEvent(Idx event, const EventSet& pattern);
    //void EraseAugmentedEvent(Idx event, const EventSet& pattern);
    bool ExistsAugmentedEvent(Idx event, const EventSet& pattern) const;
    AugmentedAlphabet FindByPattern(const EventSet& pattern) const;
    EventSet Events(void) const;
    std::vector<EventSet> ControlPatterns(void) const;
};

*/

// Add entry to mini help system: introduce new topic "OmegaAut"
SwigHelpTopic("OmegaAut","Omega Automata PlugIn");

/*
**************************************************
**************************************************
**************************************************

Interface: auto generated (functions)

**************************************************
**************************************************
**************************************************
*/



// Include rti generated function interface
#if SwigModule=="SwigOmegaAut"
%include "rtiautoload.i"
#endif
 


/*
**************************************************
**************************************************
**************************************************

Finalise

**************************************************
**************************************************
**************************************************
*/

// Extra Lua code: copy module to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy synthesis to faudes name space
for k,v in pairs(omegaaut) do faudes[k]=v end
}
#endif				 
