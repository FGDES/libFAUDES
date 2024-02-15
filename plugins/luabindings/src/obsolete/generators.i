/** @file generators.i @brief luabindings for core generator classes */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008  Thomas Moor
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


// Extra c code for swig
%{
%}


/*
**************************************************
**************************************************
**************************************************

Generator: introduce common base

**************************************************
**************************************************
**************************************************
*/

// Generator: define core members as a macros: constructors

%define SwigVGeneratorConstructors(GEN)

  //Construct/destruct
  GEN(void);
  GEN(const GEN& rOtherGen);
  GEN(const std::string& rFileName);
  virtual ~GEN(void);
  virtual GEN* New(void);
  virtual GEN* Copy(void);
  %extend {
    GEN* Version(const std::string& pattern, const std::string& replace) const {
      GEN* res=$self->New();
      $self->Version(pattern,replace,*res);
      return res;
    }
    GEN* Version(const std::string& suffix) const {
      GEN* res=$self->New();
      $self->Version(suffix,*res);
      return res;
    }
  }

%enddef


// Generator: define core members as a macros: members, non-virtual

%define SwigVGeneratorMembers(GEN)

  // Basic maintenance
  void Name(const std::string& rName);
  std::string Name(void) const;

  // Symboltables
  /*
  static SymbolTable* GlobalEventSymbolTablep(void);
  SymbolTable* EventSymbolTablep(void) const;
  virtual void EventSymbolTablep(SymbolTable* pSymTab);
  virtual void EventSymbolTablep(const Generator& rOtherGen);
  EventSet NewEventSet(void) const;
  EventSet* NewEventSetp(void) const;
  const SymbolTable& StateSymbolTable(void) const;
  void StateSymbolTable(const SymbolTable& rSymTab);
  */

  // Symbolic event names
  Idx EventIndex(const std::string& rName) const;
  std::string EventName(Idx index) const;
  void EventName(Idx index, const std::string& rName);
  std::string UniqueEventName(const std::string& rName) const;
  bool EventRename(Idx event, const std::string& rNewName);
  bool EventRename(const std::string& rOldName, const std::string& rNewName);

  // Symbolic state names
  Idx StateIndex(const std::string& rName) const;
  std::string StateName(Idx index) const;
  void StateName(Idx index, const std::string& rName);
  std::string UniqueStateName(const std::string& rName) const;
  void ClearStateNames(void);
  void ClrStateName(Idx index);
  void ClrStateName(const std::string& rName);
  bool StateNamesEnabled(void) const;
  void StateNamesEnabled(bool flag);
  void SetDefaultStateNames(void);
  void EnforceStateNames(const std::string& rTemplate);

  // Convenience string output
  std::string EStr(Idx index) const;
  std::string SStr(Idx index) const;
  std::string TStr(const Transition& rTrans) const;

  // Extend convenience output to iterators
  %extend {
   std::string EStr(const NameSetIterator& eit) const {
     return $self->EStr(*eit); }
   std::string SStr(const IndexSetIterator& sit) const {
     return $self->SStr(*sit); }
   std::string TStr(const TransSetX1EvX2Iterator& tit) const {
     return $self->TStr(*tit); }
  }
    
  // Dot processing
  %extend {
    void GraphWrite(const std::string& rFileName, const std::string& rOutFormat="") const {
      // dont crash on missing dot
      if(!luafaudes_dotready()) {
        FD_WARN("faudes.Generator::GraphWrite(...): cannot run graphviz \"dot\" to layout graph");
        return;
      }
      // run dot
      $self->GraphWrite(rFileName,rOutFormat,luafaudes_dotpath);
    }
  }


  // Formerly virtual: container token io
  virtual void WriteAlphabet(void) const;
  virtual std::string AlphabetToString(void) const;
  virtual void WriteAlphabet(TokenWriter& rTw) const;
  virtual std::string StateSetToString(const StateSet& rStateSet) const;
  virtual void WriteStateSet(TokenWriter& rTw, const StateSet& rStateSet) const;
  virtual void WriteStateSet(const StateSet& rStateSet) const;
  virtual void DWriteStateSet(TokenWriter& rTw, const StateSet& rStateSet) const;
  virtual std::string StatesToString(void) const;
  virtual std::string MarkedStatesToString(void) const;
  virtual std::string InitStatesToString(void) const;
  virtual void WriteTransRel(void) const;
  virtual std::string TransRelToString(void) const;
  virtual void WriteTransRel(TokenWriter& rTw) const;
  virtual void DWriteTransRel(TokenWriter& rTw) const;
  virtual void DotWrite(const std::string& rFileName) const;
  virtual void DDotWrite(const std::string& rFileName) const;
  virtual void ReadStateSet(TokenReader& rTr, const std::string& rLabel, StateSet& rStateSet) const;

  // Formerly virtual: informative
  virtual bool Valid(void);
  virtual void Clear(void);
  virtual void ClearStates(void);
  virtual Idx Size(void) const;
  virtual Idx AlphabetSize(void) const;
  virtual Idx TransRelSize(void) const;
  virtual Idx InitStatesSize(void) const;
  virtual Idx MarkedStatesSize(void) const;
  virtual bool Empty(void) const;
  virtual bool AlphabetEmpty(void) const;
  virtual bool TransRelEmpty(void) const;
  virtual bool InitStatesEmpty(void) const;
  virtual bool MarkedStatesEmpty(void) const;
  virtual EventSet UsedEvents(void) const;
  virtual EventSet UnusedEvents(void) const;
  virtual EventSet ActiveEventSet(Idx x1) const;
  virtual TransSet ActiveTransSet(Idx x1) const;
  virtual StateSet TransRelStates(void) const;
  virtual StateSet SuccessorStates(Idx x1) const;
  virtual StateSet SuccessorStates(Idx x1, Idx ev) const;
  virtual Idx SuccessorState(Idx x1, Idx ev) const;

  // Formerly virtual: event read access
  virtual NameSetIterator AlphabetBegin(void) const;
  virtual NameSetIterator AlphabetEnd(void) const;
  virtual bool ExistsEvent(Idx index) const;
  virtual bool ExistsEvent(const std::string& rName) const;
  virtual NameSetIterator FindEvent(Idx index) const;
  virtual NameSetIterator FindEvent(const std::string& rName) const;
  // original: procide reference
  // virtual const EventSet& Alphabet(void) const;
  // alt: fix read only access
  %extend {
  virtual EventSet Alphabet(void) const {
    EventSet res; res=$self->Alphabet(); return res; };
  }

  // Formerly virtual: state read access
  virtual IndexSetIterator StatesBegin(void) const;
  virtual IndexSetIterator StatesEnd(void) const;
  virtual bool ExistsState(Idx index) const;
  virtual bool ExistsState(const std::string& name) const;
  virtual IndexSetIterator FindState(Idx index) const;
  virtual IndexSetIterator FindState(const std::string& rName) const;
  // fix read only access
  %extend {
  virtual StateSet States(void) const {
    StateSet res; res=$self->States(); return res; };
  }
  virtual Idx InitState(void) const;
  virtual IndexSetIterator InitStatesBegin(void) const;
  virtual IndexSetIterator InitStatesEnd(void) const;
  virtual bool ExistsInitState(Idx index) const;
  virtual IndexSetIterator FindInitState(Idx index) const;
  // fix read only access
  %extend {
  virtual StateSet InitStates(void) const {
    StateSet res; res=$self->InitStates(); return res; };
  }
  virtual IndexSetIterator MarkedStatesBegin(void) const;
  virtual IndexSetIterator MarkedStatesEnd(void) const;
  virtual bool ExistsMarkedState(Idx index) const;
  virtual IndexSetIterator FindMarkedState(Idx index) const;
  // fix read only access
  %extend {
  virtual StateSet MarkedStates(void) const {
    StateSet res; res=$self->MarkedStates(); return res; };
  }

  // Formerly virtual: transition read access
  virtual TransSetX1EvX2Iterator TransRelBegin(void) const;
  virtual TransSetX1EvX2Iterator TransRelEnd(void) const;
  virtual TransSetX1EvX2Iterator TransRelBegin(Idx x1) const;
  virtual TransSetX1EvX2Iterator TransRelEnd(Idx x1) const;
  virtual TransSetX1EvX2Iterator TransRelBegin(Idx x1, Idx ev) const;
  virtual TransSetX1EvX2Iterator TransRelEnd(Idx x1, Idx ev) const;
  virtual TransSetX1EvX2Iterator FindTransition(
     const std::string& rX1, const std::string& rEv, const std::string& rX2) const;
  virtual TransSetX1EvX2Iterator FindTransition(Idx x1, Idx ev, Idx x2) const;
  virtual TransSetX1EvX2Iterator FindTransition(const Transition& rTrans) const;
  virtual bool ExistsTransition(
     const std::string& rX1, const std::string& rEv, const std::string& rX2) const;
  virtual bool ExistsTransition(Idx x1, Idx ev, Idx x2) const;
  virtual bool ExistsTransition(const Transition& rTrans) const;
  virtual bool ExistsTransition(Idx x1, Idx ev) const;
  virtual bool ExistsTransition(Idx x1) const;
  // fix read only access
  %extend {
  virtual TransSetX1EvX2 TransRel(void) const {
    TransSet res; res=$self->TransRel(); return res; };
  }
  virtual void TransRel(TransSetX1EvX2& res) const;
  virtual void TransRel(TransSetEvX1X2& res) const;
  virtual void TransRel(TransSetEvX2X1& res) const;
  virtual void TransRel(TransSetX2EvX1& res) const;
  virtual void TransRel(TransSetX2X1Ev& res) const;
  virtual void TransRel(TransSetX1X2Ev& res) const;

  // Formerly virtual: event write access
  virtual bool InsEvent(Idx index);
  virtual Idx InsEvent(const std::string& rName);
  virtual void InsEvents(const EventSet& events);
  virtual bool DelEvent(Idx index);
  virtual bool DelEvent(const std::string& rName);
  virtual void DelEvents(const EventSet& rEvents);
  virtual bool DelEventFromAlphabet(Idx index);
  virtual void InjectAlphabet(const EventSet& newalphabet);
  virtual void RestrictAlphabet(const EventSet& restriction);

  // Formerly virtual: state write access
  virtual Idx InsState(void);
  virtual bool InsState(Idx index);
  virtual Idx InsState(const std::string& rName);
  virtual void InsStates(const StateSet& states);
  virtual bool DelState(Idx index);
  virtual bool DelState(const std::string& rName);
  virtual void DelStates(const StateSet& rDelStates);
  virtual bool DelStateFromStates(Idx index);
  virtual IndexSetIterator DelStateFromStates(IndexSetIterator pos);
  virtual void RestrictStates(const StateSet& rDelStates);
  virtual void InjectState(Idx index);
  virtual void InjectStates(const StateSet& rNewStates);
  virtual Idx InsInitState(void);
  virtual bool InsInitState(Idx index);
  virtual Idx InsInitState(const std::string& rName);
  virtual void InsInitStates(const StateSet& states);
  virtual void SetInitState(Idx index);
  virtual void SetInitState(const std::string& rName);
  virtual void ClrInitState(Idx index);
  virtual void ClrInitState(const std::string& rName);
  virtual IndexSetIterator ClrInitState(IndexSetIterator pos);
  virtual void ClearInitStates(void); 
  virtual void InjectInitStates(const StateSet& rNewInitStates);
  virtual Idx InsMarkedState(void);
  virtual bool InsMarkedState(Idx index);
  virtual Idx InsMarkedState(const std::string& rName);
  virtual void InsMarkedStates(const StateSet& states);
  virtual void SetMarkedState(Idx index);
  virtual void SetMarkedState(const std::string& rName);
  virtual void ClrMarkedState(Idx index);
  virtual void ClrMarkedState(const std::string& rName);
  virtual IndexSetIterator ClrMarkedState(IndexSetIterator pos);
  virtual void ClearMarkedStates(void);
  virtual void InjectMarkedStates(const StateSet& rNewMarkedStates);

  // Formerly virtual: transition write access
  virtual bool SetTransition(Idx x1, Idx ev, Idx x2);
  virtual bool SetTransition(const std::string& rX1, const std::string& rEv, 
					   const std::string& rX2);
  virtual bool SetTransition(const Transition& rTransition);
  virtual void ClrTransition(Idx x1, Idx ev, Idx x2);
  virtual void ClrTransition(const Transition& rTrans);
  virtual TransSetX1EvX2Iterator ClrTransition(TransSetX1EvX2Iterator it);
  virtual void ClrTransitions(Idx x1, Idx ev);
  virtual void ClrTransitions(Idx x1);
  virtual void ClearTransRel(void);
  virtual void InjectTransition(const Transition& rTrans);
  virtual void InjectTransRel(const TransSet& newtransset);

  // Formerly virtual: reachability
  virtual StateSet AccessibleSet(void) const;
  virtual bool Accessible(void);
  virtual bool IsAccessible(void) const;
  virtual StateSet CoaccessibleSet(void) const;
  virtual bool Coaccessible(void);
  virtual bool IsCoaccessible(void) const;
  virtual StateSet TrimSet(void) const;
  virtual bool Trim(void);
  virtual bool IsTrim(void) const;
  virtual StateSet BlockingStates(void) const;
  virtual bool Complete(void);
  virtual bool IsComplete(void) const;
  virtual StateSet TerminalStates(void) const;

  // Formerly virtual: misc
  virtual bool IsDeterministic(void) const;   
  virtual void SetMinStateIndexMap(void) const;
  virtual void ClearMinStateIndexMap(void) const;
  virtual Idx MinStateIndex(Idx index) const;
  virtual void MinStateIndex(void);
  virtual Idx MaxStateIndex(void) const;
  //virtual const std::map<Idx,Idx>& MinStateIndexMap(void) const;
  virtual void MinimizeAlphabet(void);


%enddef

// Announce  Generator class to swig as Generator
//%rename(Generator) Generator;

// Generator swig interface definition
class Generator : public Type {
public:
  // Generator members incl constructors
  SwigVGeneratorConstructors(Generator);
  SwigVGeneratorMembers(Generator);
};


/*
**************************************************
**************************************************
**************************************************

aGenerator: parametrised attributes

**************************************************
**************************************************
**************************************************
*/


// aGenerator: define constructors as a macro
%define SwigTaGeneratorConstructors(GEN,G_ATTR,S_ATTR,E_ATTR,T_ATTR)
  //Construct/destruct
  GEN<G_ATTR,S_ATTR,E_ATTR,T_ATTR>(void);
  GEN<G_ATTR,S_ATTR,E_ATTR,T_ATTR>(const GEN& rOtherGen);
  GEN<G_ATTR,S_ATTR,E_ATTR,T_ATTR>(const Generator& rOtherGen);
  GEN<G_ATTR,S_ATTR,E_ATTR,T_ATTR>(const std::string& rFileName);
  virtual ~GEN<G_ATTR,S_ATTR,E_ATTR,T_ATTR>(void);
  virtual GEN<G_ATTR,S_ATTR,E_ATTR,T_ATTR>* New(void);
  virtual GEN<G_ATTR,S_ATTR,E_ATTR,T_ATTR>* Copy(void);
  %extend {
    GEN<G_ATTR,S_ATTR,E_ATTR,T_ATTR>* Version(const std::string& pattern, const std::string& replace) const {
      GEN<G_ATTR,S_ATTR,E_ATTR,T_ATTR>* res=$self->New();
      $self->Version(pattern,replace,*res);
      return res;
    }
    GEN<G_ATTR,S_ATTR,E_ATTR,T_ATTR>* Version(const std::string& suffix) const {
      GEN<G_ATTR,S_ATTR,E_ATTR,T_ATTR>* res=$self->New();
      $self->Version(suffix,*res);
      return res;
    }
  }
%enddef

// aGenerator: define attribute members as a macro
%define SwigTaGeneratorMembers(GEN,G_ATTR,S_ATTR,E_ATTR,T_ATTR)

  // Attribute maintenance
  void ClearAttributes(void);
  virtual bool UpdateAttributes(void);

  // Core Memeber read access (take a copy to fix const)
  %extend {
  virtual TaIndexSet<S_ATTR> States(void) const {
    return TaIndexSet<S_ATTR>($self->States()); };
  virtual TaNameSet<E_ATTR> Alphabet(void) const {
    return TaNameSet<E_ATTR>($self->Alphabet()); };
  virtual TaTransSet<T_ATTR> TransRel(void) const {
    return TaTransSet<T_ATTR>($self->TransRel()); };
  }

  // Core Memeber read access, original C++ with const reference
  /*
  const TaIndexSet<S_ATTR>& States(void) const;
  const TaNameSet<E_ATTR>& Alphabet(void) const;
  const TaTransSet<T_ATTR>& TransRel(void) const;
  */

  // Event attributes
  void ClearEventAttributes(void);
  bool InsEvent(Idx index);
  Idx InsEvent(const std::string& rName);
  void InsEvents(const EventSet& events);
  bool InsEvent(Idx index, const EventAttr& attr);
  Idx InsEvent(const std::string& rName, const EventAttr& attr);
  void ClrEventAttribute(Idx index);
  void EventAttribute(Idx index, const EventAttr& rAttr);
  %rename(EventAttribute) EventAttributep;
  EventAttr* EventAttributep(Idx index);
  EventAttr* EventAttributep(const std::string& rName);

  // State attributes
  void ClearStateAttributes(void);
  Idx InsState(void);
  bool InsState(Idx index);
  Idx InsState(const std::string& rName);
  Idx InsState(const StateAttr& attr);
  Idx InsState(const std::string& rName, const StateAttr& attr);
  bool InsState(Idx index, const StateAttr& attr);
  void ClrStateAttribute(Idx index);
  void StateAttribute(Idx index, const StateAttr& rAttr);
  %rename(StateAttribute) StateAttributep;
  StateAttr* StateAttributep(Idx index);

  // Transition attributes
  void ClearTransAttributes(void);
  bool SetTransition(Idx x1, Idx ev, Idx x2);
  bool SetTransition(const std::string& rX1, const std::string& rEv, const std::string& rX2);
  bool SetTransition(const Transition& rTransition);
  bool SetTransition(const Transition& rTransition, const TransAttr& rAttr);
  void ClrTransAttribute(const Transition& rTrans);
  void TransAttribute(const Transition& rTrans, const TransAttr& rAttr);
  %rename(TransAttribute) TransAttributep;
  TransAttr* TransAttributep(const Transition& rTrans);
 
  // Global attribute
  void ClearGlobalAttribute(void);
  void GlobalAttribute(const GlobalAttr& rAttr) {mGlobalAttribute=rAttr;};
  %rename(GlobalAttribute) GlobalAttributep;
  GlobalAttr* GlobalAttributep(void) {return &mGlobalAttribute;};

%enddef


// aGenerator template
template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
class TaGenerator : public Generator {
public:
  // Construct/destruct
  SwigTaGeneratorConstructors(TaGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TaGenerator members not inherited from Generator
  SwigTaGeneratorMembers(TaGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
};



/*
**************************************************
**************************************************
**************************************************

System: parametrised attributes

**************************************************
**************************************************
**************************************************
*/


// System: define attribute members as a macro
%define SwigTcGeneratorMembers(GEN,G_ATTR,S_ATTR,E_ATTR,T_ATTR)

  // Controllable events
  void InsControllableEvent(Idx index);
  Idx InsControllableEvent(const std::string& rName);
  void InsUncontrollableEvent(Idx index);
  Idx InsUncontrollableEvent(const std::string& rName);
  void SetControllable(Idx index);
  void SetControllable(const std::string& rName);
  void SetControllable(const EventSet& rEvents);
  void ClrControllable(Idx index);
  void ClrControllable(const std::string& rName);
  void ClrControllable(const EventSet& rEvents);
  bool Controllable(Idx index) const;
  bool Controllable(const std::string& rName) const;
  EventSet ControllableEvents(void) const;
  EventSet UncontrollableEvents(void) const;

  // Observable events
  void InsObservableEvent(Idx index);
  Idx InsObservableEvent(const std::string& rName);
  void InsUnobservableEvent(Idx index);
  Idx InsUnobservableEvent(const std::string& rName);
  void SetObservable(Idx index);
  void SetObservable(const std::string& rName);
  void SetObservable(const EventSet& rEvents);
  void ClrObservable(Idx index);
  void ClrObservable(const std::string& rName);
  void ClrObservable(const EventSet& rEvents);
  bool Observable(Idx index) const;
  bool Observable(const std::string& rName) const;
  EventSet ObservableEvents(void) const;
  EventSet UnobservableEvents(void) const;

  // Forcible events
  void InsForcibleEvent(Idx index);
  Idx InsForcibleEvent(const std::string& rName);
  void InsUnforcibleEvent(Idx index);
  Idx InsUnforcibleEvent(const std::string& rName);
  void SetForcible(Idx index);
  void SetForcible(const std::string& rName);
  void SetForcible(const EventSet& rEvents);
  void ClrForcible(Idx index);
  void ClrForcible(const std::string& rName);
  void ClrForcible(const EventSet& rEvents);
  bool Forcible(Idx index) const;
  bool Forcible(const std::string& rName) const;
  EventSet ForcibleEvents(void) const;
  EventSet UnforcibleEvents(void) const;

  // Abstraction events
  void InsHighlevelEvent(Idx index);
  Idx InsHighlevelEvent(const std::string& rName);
  void InsLowlevelEvent(Idx index);
  Idx InsLowlevelEvent(const std::string& rName);
  void SetHighlevel(Idx index);
  void SetHighlevel(const std::string& rName);
  void SetHighlevel(const EventSet& rEvents);
  void SetLowlevel(Idx index);
  void SetLowlevel(const std::string& rName);
  void SetLowlevel(const EventSet& rEvents);
  bool Highlevel(Idx index) const;
  bool Highlevel(const std::string& rName) const;
  bool Lowlevel(Idx index) const;
  bool Lowlevel(const std::string& rName) const;
  EventSet HighlevelEvents(void) const;
  EventSet LowlevelEvents(void) const;

%enddef

// System template
template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
class TcGenerator : public Generator {
public:
  // Construct/destruct
  SwigTaGeneratorConstructors(TcGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TaGenerator members not inherited from Generator
  SwigTaGeneratorMembers(TcGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
  // TcGenerator members not inherited from TaGenerator
  SwigTcGeneratorMembers(TcGenerator,GlobalAttr,StateAttr,EventAttr,TransAttr);
};

// Announce  System class to swig as System
%template(System) TcGenerator<AttributeVoid,AttributeVoid,AttributeCFlags,AttributeVoid>;

// Tell swig that our generator is also known as System
typedef TcGenerator<AttributeVoid,AttributeVoid,AttributeCFlags,AttributeVoid> System;


/*
**************************************************
**************************************************
**************************************************

Help topics

**************************************************
**************************************************
**************************************************
*/

SwigHelpTopic("Generator","Generator methods");

SwigHelpEntry("Generator","Constructors","Generator Generator()");
SwigHelpEntry("Generator","Constructors","Generator Generator(gen)");
SwigHelpEntry("Generator","Constructors","Generator Generator(filename)");
SwigHelpEntry("Generator","Constructors","Generator Copy()");
SwigHelpEntry("Generator","Constructors","Generator Version(string,string)");
SwigHelpEntry("Generator","Constructors","Generator Version(string)");

SwigHelpEntry("Generator","Maintenance"," Name(string)");
SwigHelpEntry("Generator","Maintenance","string Name()");
SwigHelpEntry("Generator","Maintenance","bool Valid()");
SwigHelpEntry("Generator","Maintenance"," Clear()");
SwigHelpEntry("Generator","Maintenance","int Size()");
SwigHelpEntry("Generator","Maintenance","int AlphabetSize()");
SwigHelpEntry("Generator","Maintenance","int TransRelSize()");
SwigHelpEntry("Generator","Maintenance","int TransRelSize()");
SwigHelpEntry("Generator","Maintenance","int InitStatesSize()");
SwigHelpEntry("Generator","Maintenance","int MarkedStatesSize()");
SwigHelpEntry("Generator","Maintenance","bool AlphabetEmpty()");
SwigHelpEntry("Generator","Maintenance","bool TransRelEmpty()");
SwigHelpEntry("Generator","Maintenance","bool InitStatesEmpty()");
SwigHelpEntry("Generator","Maintenance","bool MarkedStatesEmpty()");
SwigHelpEntry("Generator","Maintenance","EventSet UsedEvents()");
SwigHelpEntry("Generator","Maintenance","EventSet UnusedEvents()");
SwigHelpEntry("Generator","Maintenance","bool IsDeterministic()");
SwigHelpEntry("Generator","Maintenance"," MinimizeAlphabet()");
SwigHelpEntry("Generator","Maintenance"," MinStateIndex()");


SwigHelpEntry("Generator","Symbolic names","int EventIndex(string)");
SwigHelpEntry("Generator","Symbolic names","string EventName(int)");
SwigHelpEntry("Generator","Symbolic names","string UniqueEventName(string)");
SwigHelpEntry("Generator","Symbolic names","bool RenameEvent(idx,string)");
SwigHelpEntry("Generator","Symbolic names","bool RenameEvent(string,string)");
SwigHelpEntry("Generator","Symbolic names","int StateIndex(string)");
SwigHelpEntry("Generator","Symbolic names","string StateName(int)");
SwigHelpEntry("Generator","Symbolic names","string UniqueStateName(string)");
SwigHelpEntry("Generator","Symbolic names"," ClearStateNames()");
SwigHelpEntry("Generator","Symbolic names"," ClrStateName(int)");
SwigHelpEntry("Generator","Symbolic names"," ClrStateName(string)");
SwigHelpEntry("Generator","Symbolic names","bool StateNamesEnabled()");
SwigHelpEntry("Generator","Symbolic names"," StateNamesEnabled(bool)");
SwigHelpEntry("Generator","Symbolic names"," SetDefaultStateNames()");
SwigHelpEntry("Generator","Symbolic names"," EnforceStateNames(string)");


SwigHelpEntry("Generator","Misc IO","string EStr(int)");
SwigHelpEntry("Generator","Misc IO","string SStr(int)");
SwigHelpEntry("Generator","Misc IO","string TStr(int)");
SwigHelpEntry("Generator","Misc IO"," GrapthWrite(filename,format)");
SwigHelpEntry("Generator","Misc IO"," WriteAlphabet()");
SwigHelpEntry("Generator","Misc IO","string AlphabetToString()");
SwigHelpEntry("Generator","Misc IO"," WriteStateSet(IndexSet)");
SwigHelpEntry("Generator","Misc IO"," DWriteStateSet(IndexSet)");
SwigHelpEntry("Generator","Misc IO","string StateSetToString(IndexSet)");
SwigHelpEntry("Generator","Misc IO","string StatesToString()");
SwigHelpEntry("Generator","Misc IO","string InitStatesToString()");
SwigHelpEntry("Generator","Misc IO","string MarkedStatesToString()");
SwigHelpEntry("Generator","Misc IO"," WriteTransRel()");
SwigHelpEntry("Generator","Misc IO","string TransRelToString()");
SwigHelpEntry("Generator","Misc IO"," SWrite()");
SwigHelpEntry("Generator","Misc IO","string ToSText()");
SwigHelpEntry("Generator","Misc IO","");


SwigHelpEntry("Generator","Alphabet","EventSet Alphabet()");
SwigHelpEntry("Generator","Alphabet","bool InsEvent(idx)");
SwigHelpEntry("Generator","Alphabet","bool InsEvent(name)");
SwigHelpEntry("Generator","Alphabet"," InsEvents(EventSet)");
SwigHelpEntry("Generator","Alphabet","bool DelEvent(idx)");
SwigHelpEntry("Generator","Alphabet","bool DelEvent(name)");
SwigHelpEntry("Generator","Alphabet"," DelEvents(EventSet)");
SwigHelpEntry("Generator","Alphabet","bool ExistsEvent(idx)");
SwigHelpEntry("Generator","Alphabet","bool ExistsEvent(name)");
SwigHelpEntry("Generator","Alphabet","iterator AlphabetBegin()");
SwigHelpEntry("Generator","Alphabet","iterator AlphabetEnd()");
SwigHelpEntry("Generator","Alphabet","iterator FindEvent(idx)");
SwigHelpEntry("Generator","Alphabet","iterator FindEvent(name)");
SwigHelpEntry("Generator","Alphabet","bool DelEventFromAlphabet(idx)");
SwigHelpEntry("Generator","Alphabet"," InjectAlphabet(EventSet)");
SwigHelpEntry("Generator","Alphabet"," RestrictAlphabet(EventSet)");

SwigHelpEntry("Generator","State set","IndexSet States()");
SwigHelpEntry("Generator","State set","idx InsState()");
SwigHelpEntry("Generator","State set","bool InsState(idx)");
SwigHelpEntry("Generator","State set","idx InsState(name)");
SwigHelpEntry("Generator","State set"," InsStates(StateSet)");
SwigHelpEntry("Generator","State set"," InsEvents(EventSet)");
SwigHelpEntry("Generator","State set","bool DelState(idx)");
SwigHelpEntry("Generator","State set","bool DelState(name)");
SwigHelpEntry("Generator","State set"," DelStates(StateSet)");
SwigHelpEntry("Generator","State set"," RestrictStates(StateSet)");
SwigHelpEntry("Generator","State set","bool ExistsState(idx)");
SwigHelpEntry("Generator","State set","bool ExistsState(name)");
SwigHelpEntry("Generator","State set","iterator StatesBegin()");
SwigHelpEntry("Generator","State set","iterator StatesEnd()");
SwigHelpEntry("Generator","State set","iterator FindState(idx)");
SwigHelpEntry("Generator","State set","iterator FindState(name)");
SwigHelpEntry("Generator","State set","bool DelStateFromStates(idx)");
SwigHelpEntry("Generator","State set","iterator DelStateFromStates(iterator)");
SwigHelpEntry("Generator","State set"," InjectState(idx)");
SwigHelpEntry("Generator","State set"," InjectStates(IndexSet)");
SwigHelpEntry("Generator","State set","idx MaxStateIndex()");
SwigHelpEntry("Generator","State set","");



SwigHelpEntry("Generator","Initial states","IndexSet InitStates()");
SwigHelpEntry("Generator","Initial states","idx InsInitState()");
SwigHelpEntry("Generator","Initial states","idx InsInitState(idx)");
SwigHelpEntry("Generator","Initial states"," InsInitStates(StateSet)");
SwigHelpEntry("Generator","Initial states","idx InsInitState(name)");
SwigHelpEntry("Generator","Initial states"," SetInitState(idx)");
SwigHelpEntry("Generator","Initial states"," SetInitState(name)");
SwigHelpEntry("Generator","Initial states"," ClrInitState(idx)");
SwigHelpEntry("Generator","Initial states"," ClrInitState(name)");
SwigHelpEntry("Generator","Initial states","iterator ClrInitState(iterator)");
SwigHelpEntry("Generator","Initial states","bool ExistsInitState(idx)");
SwigHelpEntry("Generator","Initial states","iterator InitStatesBegin()");
SwigHelpEntry("Generator","Initial states","iterator InitStatesEnd()");
SwigHelpEntry("Generator","Initial states","iterator FindInitState()");
SwigHelpEntry("Generator","Initial states"," ClearInitStates()");
SwigHelpEntry("Generator","Initial states"," InjectInitStates(IndexSet)");
SwigHelpEntry("Generator","Initial states","idx InitState()");

SwigHelpEntry("Generator","Marked states","IndexSet MarkedStates()");
SwigHelpEntry("Generator","Marked states","idx InsMarkedState()");
SwigHelpEntry("Generator","Marked states","idx InsMarkedState(idx)");
SwigHelpEntry("Generator","Marked states","idx InsMarkedState(name)");
SwigHelpEntry("Generator","Marked states"," InsMarkedStates(StateSet)");
SwigHelpEntry("Generator","Marked states"," SetMarkedState(idx)");
SwigHelpEntry("Generator","Marked states"," SetMarkedState(name)");
SwigHelpEntry("Generator","Marked states"," ClrMarkedState(idx)");
SwigHelpEntry("Generator","Marked states"," ClrMarkedState(name)");
SwigHelpEntry("Generator","Marked states","iterator ClrMarkedState(iterator)");
SwigHelpEntry("Generator","Marked states","bool ExistsMarkedState(idx)");
SwigHelpEntry("Generator","Marked states","iterator MarkedStatesBegin()");
SwigHelpEntry("Generator","Marked states","iterator MarkedStatesEnd()");
SwigHelpEntry("Generator","Marked states","iterator FindMarkedState()");
SwigHelpEntry("Generator","Marked states"," ClearMarkedStates()");
SwigHelpEntry("Generator","Marked states"," InjectMarkedStates(IndexSet)");
SwigHelpEntry("Generator","Marked states","idx MarkedState()");


SwigHelpEntry("Generator","Transitions","TransSet TransRel()");
SwigHelpEntry("Generator","Transitions"," TransRel(TransSet<ORDER>)");
SwigHelpEntry("Generator","Transitions","bool SetTransition(trans)");
SwigHelpEntry("Generator","Transitions","bool SetTransition(x1,ev,x2)");
SwigHelpEntry("Generator","Transitions","bool SetTransition(x1name,evname,x2name)");
SwigHelpEntry("Generator","Transitions"," ClrTransition(trans)");
SwigHelpEntry("Generator","Transitions"," ClrTransition(x1,ev,x2)");
SwigHelpEntry("Generator","Transitions","iterator ClrTransition(iterator)");
SwigHelpEntry("Generator","Transitions"," ClrTransitions(x1,ev)");
SwigHelpEntry("Generator","Transitions"," ClrTransitions(x1)");
SwigHelpEntry("Generator","Transitions","bool ExistsTransition(trans)");
SwigHelpEntry("Generator","Transitions","bool ExistsTransition(x1,ev,x2)");
SwigHelpEntry("Generator","Transitions","bool ExistsTransition(x1name,evname,x2name)");
SwigHelpEntry("Generator","Transitions","bool ExistsTransition(x1,ev)");
SwigHelpEntry("Generator","Transitions","bool ExistsTransition(x1)");
SwigHelpEntry("Generator","Transitions","iterator TransRelBegin()");
SwigHelpEntry("Generator","Transitions","iterator TransRelEnd()");
SwigHelpEntry("Generator","Transitions","iterator TransRelBegin(x1)");
SwigHelpEntry("Generator","Transitions","iterator TransRelEnd(x1)");
SwigHelpEntry("Generator","Transitions","iterator TransRelBegin(x1,ev)");
SwigHelpEntry("Generator","Transitions","iterator TransRelEnd(x1,ev)");
SwigHelpEntry("Generator","Transitions","iterator FindTransition(trans)");
SwigHelpEntry("Generator","Transitions","iterator FindTransition(x1,ev,x2)");
SwigHelpEntry("Generator","Transitions","iterator FindTransition(x1name,evname,x2name)");
SwigHelpEntry("Generator","Transitions"," ClearTransRel()");
SwigHelpEntry("Generator","Transitions"," InjectTransRel(TransSet)");
SwigHelpEntry("Generator","Transitions"," InjectTransition(trans)");


SwigHelpEntry("Generator","Reachability","IndexSet AccessibleSet()");
SwigHelpEntry("Generator","Reachability","bool Accessible()");
SwigHelpEntry("Generator","Reachability","bool IsAccessible()");
SwigHelpEntry("Generator","Reachability","IndexSet CoaccessibleSet()");
SwigHelpEntry("Generator","Reachability","bool Coaccessible()");
SwigHelpEntry("Generator","Reachability","bool IsCoaccessible()");
SwigHelpEntry("Generator","Reachability","bool Trim()");
SwigHelpEntry("Generator","Reachability","bool IsTrim()");
SwigHelpEntry("Generator","Reachability","IndexSet BlockingStates()");
SwigHelpEntry("Generator","Reachability","bool IsComplete()");
SwigHelpEntry("Generator","Reachability","bool Complete()");
SwigHelpEntry("Generator","Reachability","IndexSet TerminalStates()");
SwigHelpEntry("Generator","Reachability","IndexSet TransRelStates()");
SwigHelpEntry("Generator","Reachability","EventSet ActiveEventSet(x1)");
SwigHelpEntry("Generator","Reachability","TransSet ActiveTransSet(x1)");
SwigHelpEntry("Generator","Reachability","IndexSet SuccessorStates(x1)");
SwigHelpEntry("Generator","Reachability","IndexSet SuccessorStates(x1,ev)");
SwigHelpEntry("Generator","Reachabilits","idx SuccessorState(x1,ev)");


SwigHelpTopic("System","System methods (inherits Generator)");

SwigHelpEntry("System","Constructors","System System()");
SwigHelpEntry("System","Constructors","System System(sys)");
SwigHelpEntry("System","Constructors","System System(gen)");
SwigHelpEntry("System","Constructors","System System(filename)");
SwigHelpEntry("System","Constructors","System Copy()");

SwigHelpEntry("System","Controllable events","bool Controllable(idx)");
SwigHelpEntry("System","Controllable events","bool Controllable(name)");
SwigHelpEntry("System","Controllable events"," InsControllableEvent(idx)");
SwigHelpEntry("System","Controllable events","idx InsControllableEvent(name)");
SwigHelpEntry("System","Controllable events"," InsUncontrollableEvent(idx)");
SwigHelpEntry("System","Controllable events","idx InsUncontrollableEvent(name)");
SwigHelpEntry("System","Controllable events"," SetControllable(idx)");
SwigHelpEntry("System","Controllable events"," SetControllable(name)");
SwigHelpEntry("System","Controllable events"," SetControllable(evenset)");
SwigHelpEntry("System","Controllable events"," ClrControllable(idx)");
SwigHelpEntry("System","Controllable events"," ClrControllable(name)");
SwigHelpEntry("System","Controllable events"," ClrControllable(evenset)");
SwigHelpEntry("System","Controllable events","eventset ControllableEvents()");
SwigHelpEntry("System","Controllable events","eventset UncontrollableEvents()");
    
SwigHelpEntry("System","Observable events","bool Observable(idx)");
SwigHelpEntry("System","Observable events","bool Observable(name)");
SwigHelpEntry("System","Observable events"," InsObservableEvent(idx)");
SwigHelpEntry("System","Observable events","idx InsObservableEvent(name)");
SwigHelpEntry("System","Observable events"," InsUncontrollableEvent(idx)");
SwigHelpEntry("System","Observable events","idx InsUncontrollableEvent(name)");
SwigHelpEntry("System","Observable events"," SetObservable(idx)");
SwigHelpEntry("System","Observable events"," SetObservable(name)");
SwigHelpEntry("System","Observable events"," SetObservable(evenset)");
SwigHelpEntry("System","Observable events"," ClrObservable(idx)");
SwigHelpEntry("System","Observable events"," ClrObservable(name)");
SwigHelpEntry("System","Observable events"," ClrObservable(evenset)");
SwigHelpEntry("System","Observable events","eventset ObservableEvents()");
SwigHelpEntry("System","Observable events","eventset UnobservableEvents()");
    
SwigHelpEntry("System","Forcible events","bool Forcible(idx)");
SwigHelpEntry("System","Forcible events","bool Forcible(name)");
SwigHelpEntry("System","Forcible events"," InsForcibleEvent(idx)");
SwigHelpEntry("System","Forcible events","idx InsForcibleEvent(name)");
SwigHelpEntry("System","Forcible events"," InsUncontrollableEvent(idx)");
SwigHelpEntry("System","Forcible events","idx InsUncontrollableEvent(name)");
SwigHelpEntry("System","Forcible events"," SetForcible(idx)");
SwigHelpEntry("System","Forcible events"," SetForcible(name)");
SwigHelpEntry("System","Forcible events"," SetForcible(evenset)");
SwigHelpEntry("System","Forcible events"," ClrForcible(idx)");
SwigHelpEntry("System","Forcible events"," ClrForcible(name)");
SwigHelpEntry("System","Forcible events"," ClrForcible(evenset)");
SwigHelpEntry("System","Forcible events","eventset ForcibleEvents()");
SwigHelpEntry("System","Forcible events","eventset UnforcibleEvents()");

SwigHelpEntry("System","Abstraction events","bool Highlevel(idx)");
SwigHelpEntry("System","Abstraction events","bool Highlevel(name)");
SwigHelpEntry("System","Abstraction events","bool Lowlevel(idx)");
SwigHelpEntry("System","Abstraction events","bool Lowlevel(name)");
SwigHelpEntry("System","Abstraction events"," InsHighlevelEvent(idx)");
SwigHelpEntry("System","Abstraction events","idx InsLowlevelEvent(name)");
SwigHelpEntry("System","Abstraction events"," InsLowlevelEvent(idx)");
SwigHelpEntry("System","Abstraction events","idx InsLowlevelEvent(name)");
SwigHelpEntry("System","Abstraction events"," SetHighlevel(idx)");
SwigHelpEntry("System","Abstraction events"," SetHighlevel(name)");
SwigHelpEntry("System","Abstraction events"," SetHighlevel(evenset)");
SwigHelpEntry("System","Abstraction events"," SetLowlevel(idx)");
SwigHelpEntry("System","Abstraction events"," SetLowlevel(name)");
SwigHelpEntry("System","Abstraction events"," SetLowlevel(evenset)");
SwigHelpEntry("System","Abstraction events","eventset HighlevelEvents()");
SwigHelpEntry("System","Abstraction events","eventset LowlevelEvents()");
    
