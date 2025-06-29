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


// Set SWIG module name
// Note: must match libFAUDES plugin name
%module omegaaut
#ifndef SwigModule
#define SwigModule "SwigOmegaAut"
#endif


// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy synthesis to faudes name space
for k,v in pairs(omegaaut) do faudes[k]=v end
}
#endif				 




// extra c code for swig
%{

%}

/*
**************************************************
**************************************************
**************************************************

actual interface

**************************************************
**************************************************
**************************************************
*/

/* put interface of data structures here */


/*
**************************************************
**************************************************
**************************************************

Interface: auto generated

**************************************************
**************************************************
**************************************************
*/

// Add entry to mini help system: introduce new topic "priorities"
SwigHelpTopic("OmegaAut","Omega Automata PlugIn");

// Include rti generated function interface
#if SwigModule=="SwigOmegaAut"
%include "rtiautoload.i"
#endif
 
