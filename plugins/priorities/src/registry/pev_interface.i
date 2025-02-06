/** @file pev_interface.i @brief bindings for priorities plug-in */


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


// Set SWIG module name
// Note: must match libFAUDES plugin name
%module priorities
#ifndef SwigModule
#define SwigModule "SwigPriorities"
#endif


// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy synthesis to faudes name space
for k,v in pairs(priorities) do faudes[k]=v end
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

// priority attribute
class AttributePriority : public AttributeCFlags {
 public:
  // construct/destruct
  AttributePriority(void);
  virtual ~AttributePriority(void);
  // extra access to c features
  Idx Priority(void);
  void Priority(Idx idx);
};


// Make Swig to run the template (internal use only)
%template(EventPrioritiesPlain) TaNameSet<AttributePriority>;
// make the plain variant accessible
typedef TaNameSet<AttributePriority> EventPrioritiesPlain;

// Add extra access mathods
class EventPriorities : public EventPrioritiesPlain {
public:
  // Construct/destruct from baseset
  SwigBaseSetConstructors(EventPriorities,Idx,NameSetIterator);
  // Additional constructor
  EventPriorities(const NameSet& rOtherSet);
  // Extra access methods
  Idx Priority(const Idx idx) const;
  Idx Priority(const std::string& rName) const;
  void Priority(const Idx idx, const Idx prio);
  void Priority(const std::string& rName, const Idx prio);
  void InsPriority(const Idx idx, const Idx prio);
  void InsPriority(const std::string& rName, const Idx prio);
  void Priorities(const EventPriorities& rOther);
  Idx LowestPriority(void) const;
};



// help entry
			   
SwigHelpTopic("EventPriorities","EventPriorities methods on top of Alphabet");

SwigHelpEntry("EventPriorities","Priorities","int Priority(int)");
SwigHelpEntry("EventPriorities","Priorities","int Priority(string)");
SwigHelpEntry("EventPriorities","Priorities","int Priority(int,int)");
SwigHelpEntry("EventPriorities","Priorities","int Priority(string,int)");
SwigHelpEntry("EventPriorities","Priorities"," Priorities(EventPriorities)");
SwigHelpEntry("EventPriorities","Priorities","int LowestPriority()");

/*
**************************************************
**************************************************
**************************************************

Synthesis interface: auto generated

**************************************************
**************************************************
**************************************************
*/

// Add entry to mini help system: introduce new topic "priorities"
SwigHelpTopic("Priorities","Priorities PlugIn");

// Include rti generated function interface
#if SwigModule=="SwigPriorities"
%include "../../../include/rtiautoload.i"
#endif
 
