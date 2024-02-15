/** @file containers.i @brief luabindings for core container classes */


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


/*
**************************************************
**************************************************
**************************************************

BaseSet: Template declaration for swig

**************************************************
**************************************************
**************************************************
*/


// Members of the very base of faudes containers
// Note: the original TBaseSet uses nested iterators that
// depend on template parameters. We mimique this by a swig
// macro with the iterator type as parameter. This is
// like early days oop ...

%define SwigBaseSetConstructors(SET,TYPE,ITERATOR)

  // Construct/destruct 
  SET(void);
  SET(const SET& rOtherSet);
  SET(const std::string& rFilename, const std::string& rLabel="");
  virtual SET* New(void);
  virtual SET* Copy(void);
  virtual ~SET(void);

%enddef

%define SwigBaseSetMembers(SET,TYPE,ITERATOR)

  // Basic maintenance
  std::string Name(void) const;
  void Name(const std::string& rName);
  virtual void Clear(void);
  Idx Size(void) const;
  bool Empty(void) const;
  // Element based access 
  virtual bool Valid(const TYPE& rElem) const;
  virtual bool Insert(const TYPE& rElem);
  virtual bool Erase(const TYPE& rElem);
  bool Exists(const TYPE& rElem) const;
  // Iterator based access
  virtual ITERATOR Begin(void) const;
  virtual ITERATOR End(void) const;
  virtual ITERATOR Erase(const ITERATOR& pos); 
  virtual ITERATOR Find(const TYPE& rElem) const;
  // Lua operator [], read access via iterator (fine in swig 2.0 -- broken in swig 3.02, need explicit token io)
  %extend {
    TYPE __getitem__(const ITERATOR& iit) const { return *iit; };
  }; 
  // Set operations 
  virtual void EraseSet(const SET& rOtherSet);
  virtual void InsertSet(const SET& rOtherSet);
  virtual void RestrictSet(const SET& rOtherSet);
  // operators
  SET operator + (const SET& rOtherSet) const;
  SET operator - (const SET& rOtherSet) const;
  SET operator * (const SET& rOtherSet) const;
  bool operator <= (const SET& rOtherSet) const;
  // Lua operator <
  %extend {
    bool operator < (const SET& rOtherSet) const {
      return  !( *$self >= rOtherSet) ; };
  };
  bool operator == (const SET& rOtherSet) const;
  // Deferred copy control
  virtual void Detach(void) const;
  void Lock(void) const;

%enddef


%define SwigBaseSetOperatorOverload(ASET,SET)

  SET operator + (const SET& rOtherSet) const;
  SET operator + (const ASET& rOtherSet) const;
  SET operator - (const SET& rOtherSet) const;
  SET operator - (const ASET& rOtherSet) const;
  SET operator * (const SET& rOtherSet) const;
  SET operator * (const ASET& rOtherSet) const;
  bool operator <= (const SET& rOtherSet) const;
  bool operator <= (const ASET& rOtherSet) const;
  // Lua operator <
  %extend {
    bool operator < (const SET& rOtherSet) const {
      return  !( *$self >= rOtherSet) ; };
    bool operator < (const ASET& rOtherSet) const {
      return  !( *$self >= rOtherSet) ; };
  };
  bool operator == (const SET& rOtherSet) const;
  bool operator == (const ASET& rOtherSet) const;

%enddef

// Members of set iterators
// Note: we choose macros for the same reasones as above
// Note: DeRef returns bad value, ok for primitive type only ie int/idx 
%define SwigIteratorMembers(SET,TYPE,ITERATOR)

  // Construct/destruct
  ITERATOR(void);
  ITERATOR(const ITERATOR& rOther);
  ~ITERATOR(void); 
  // Lua version of std operators
  %extend {
    TYPE DeRef(void) const { return  ** $self; };
    void Inc(void) { ++ *$self; };
    void Dec(void) { -- *$self; };
  }
  // Lua compatible c operators
  bool operator == (const ITERATOR& rOther) const;

%enddef


/*
**************************************************
**************************************************
**************************************************

IndexSet: derived from BaseSet with specific 
template parameters

**************************************************
**************************************************
**************************************************
*/


// Extra c code to unwind nested class
%{

// Typedefs for nested classes: index set iterator 
namespace faudes {
typedef IndexSet::Iterator IndexSetIterator;
typedef IndexSet::Iterator StateSetIterator;
}

%}


// Plain index set: iterator
class IndexSetIterator {
public: 
  // BaseSet iterator members/operators
  SwigIteratorMembers(IndexSet,Idx,IndexSetIterator)
  // Convenience deref
  %extend {
    Idx Index(void) const { return  ** $self; };
  }
};

// Have StateSet alias
typedef StateSetIterator IndexSetIterator;

// Plain index set: the set
class IndexSet : public Type {
public:
  // TBaseSet members
  SwigBaseSetConstructors(IndexSet,Idx,IndexSetIterator);
  SwigBaseSetMembers(IndexSet,Idx,IndexSetIterator);
};

// Have StateSet alias
typedef IndexSet StateSet;

// Set with attributes 
// (minimal format to tell SWIG that it can upcast)
template<class Attr>
class TaIndexSet : public IndexSet {
public:
  // Construct/destruct from baseset
  SwigBaseSetConstructors(TaIndexSet,Idx,IndexSetIterator);
  // Additional constructor (for upcast)
  TaIndexSet(const IndexSet& rOtherSet);
};

// Announce template to SWIG
%template(IndexSetA) TaIndexSet<AttributeVoid>;


/*
**************************************************
**************************************************
**************************************************

NameSet: declaration, derived from BaseSet

**************************************************
**************************************************
**************************************************
*/


// Extra c code to unwind nested class
%{

namespace faudes {
typedef NameSet::Iterator NameSetIterator;
typedef NameSet::Iterator EventSetIterator;
}

%}


// Set with symbolic names: iterator
class NameSetIterator {
public: 
  // BaseSet iterator members
  SwigIteratorMembers(NameSet,Idx,NameSetIterator)
  // Convenience: deref
  %extend {
    Idx Index(void) const { return  ** $self; };
  };
  // Convenience: figure symbolic name if possible
  %extend {
    std::string Name(void) const { 
      const NameSet* nset= dynamic_cast<const NameSet*>($self->pBaseSet);
      if(!nset) return "";
      return  nset->SymbolicName(** $self) ; };
  };
};

// Convenience typedef
typedef EventSetIterator NameSetIterator;

// access the name set as EventSet
%rename(EventSet) NameSet;

// Set with symbolic names: the set
class NameSet : public Type {
public:
  // TBaseSet members
  SwigBaseSetConstructors(NameSet,Idx,NameSetIterator)
  SwigBaseSetMembers(NameSet,Idx,NameSetIterator)
  // Name/index look up
  std::string SymbolicName(const Idx& rElem) const;
  Idx Index(const std::string& rName) const;
  // Access via symbolic name
  virtual bool Insert(const std::string& rName);
  virtual bool Erase(const std::string& rName);
  bool Exists(const std::string& rName) const;
  NameSetIterator Find(const std::string& rElem) const;
  // Convenience string conversion
  std::string Str(const Idx& rElem) const;
  // Convenience access via iterator
  %extend {
    std::string SymbolicName(const NameSetIterator& iit) const { return $self->SymbolicName(*iit);};
    std::string Str(const NameSetIterator& iit) const { return $self->Str(*iit);};
  };
};

// tell swig that our C Code may use EventSet as a synonym
typedef NameSet EventSet;


// Set with symbolic names and attributes
template<class Attr>
class TaNameSet : public NameSet {
public:
  // Construct/destruct from baseset
  SwigBaseSetConstructors(TaNameSet,Idx,NameSetIterator);
  // Additional constructor
  TaNameSet(const NameSet& rOtherSet);
  // Overwrite operators in variants
  // SwigBaseSetOperatorOverload(TaNameSet,NameSet);
  // Attribute access 
  %rename(Attribute) Attributep;
  Attr* Attributep(const Idx& rIndex);
  void Attribute(const Idx& rIndex, const Attr& rAttr);
  // Attribute overloads
  virtual bool Insert(const Idx& rIndex);
  virtual bool Insert(const std::string& rName);
  virtual bool Insert(const std::string& rName, const Attr& rAttr);
  // Convenience extention: attribue by name/iterator
  %extend {
    Attr* Attribute(const std::string& rName) { return $self->Attributep($self->Index(rName)); };
    Attr* Attribute(const NameSetIterator& iit) { return $self->Attributep(*iit); };
    void Attribute(const std::string& rName, const Attr& rAttr)  { $self->Attribute($self->Index(rName), rAttr); };
    void Attribute(const NameSetIterator& iit, const Attr& rAttr)  { $self->Attribute(*iit, rAttr); };
  } 
};

// Announce template to SWIG
// Note: the Lua Alphabet is a faudes Alphabet. So we tell SWIG to provide
// a class with target name Alphabet and that it is implemented as specified
// by the above template with parameter AttributeCFlags. We also tell swig that
// our C code may refer to the type as Alphabet.

%template(Alphabet) TaNameSet<AttributeCFlags>;
typedef TaNameSet<AttributeCFlags> Alphabet;


/*
**************************************************
**************************************************
**************************************************

TransSet: template declaration, derived from BaseSet

**************************************************
**************************************************
**************************************************
*/




// Preface: rename std order to plain
%rename(TransSet) TransSetX1EvX2;
%rename(TransSetIterator) TransSetX1EvX2Iterator;


// Transition set: iterators
// Note: yet another layer of macros to manually
// unwind a nested faudes template
%define SwigTransIterator(ORDER)

// Unwind faudes nested class definition
%{
namespace faudes {
typedef TransSort:: ## ORDER ORDER;
typedef TTransSet<ORDER>::Iterator TransSet ## ORDER ## Iterator;
}
%}

// Define iterator interface
class TransSet ## ORDER ## Iterator {
public: 
  // BaseSet iterator members
  SwigIteratorMembers(TransSet ## ORDER, Transition, TransSet ## ORDER ## Iterator)
  // Convenience: deref
  %extend {
    Transition Transition(void) const { return  ** $self; };
    Idx X1(void) const { return  (**$self).X1; };
    Idx Ev(void) const { return  (**$self).Ev; };
    Idx X2(void) const { return  (**$self).X2; };
  };
};

%enddef


// Run macro to have iterator type and interface
SwigTransIterator(X1EvX2);
SwigTransIterator(X1X2Ev);
SwigTransIterator(X2EvX1);
SwigTransIterator(X2X1Ev);
SwigTransIterator(EvX1X2);
SwigTransIterator(EvX2X1);


// Transition set: the set 
// We again unwind the template definition via a macro.
%define SwigTransSet(ORDER)


// Unwind template definition
%{
namespace faudes {
typedef TTransSet<ORDER> TransSet ## ORDER;
}
%}


// define interface for swig
class TransSet ## ORDER : public Type {
public: 
  // Construct/destruct
  TransSet ## ORDER(void);
  TransSet ## ORDER(const TransSetX1EvX2& rOtherSet);
  TransSet ## ORDER(const TransSetX1X2Ev& rOtherSet);
  TransSet ## ORDER(const TransSetX2EvX1& rOtherSet);
  TransSet ## ORDER(const TransSetX2X1Ev& rOtherSet);
  TransSet ## ORDER(const TransSetEvX1X2& rOtherSet);
  TransSet ## ORDER(const TransSetEvX2X1& rOtherSet);
  ~TransSet ## ORDER(void);
  // New & Copy constructors
  virtual TransSet ## ORDER* New(void);
  virtual TransSet ## ORDER* Copy(void);
  // TBaseSet members (excl constructors)
  SwigBaseSetMembers(TransSet ## ORDER,Transition,TransSet ## ORDER ## Iterator)
  // component access
  bool Insert(Idx x1, Idx ev, Idx x2);
  bool Erase(Idx x1, Idx ev, Idx x2);
  TransSet ## ORDER ## Iterator Find(Idx x1, Idx ev, Idx x2) const;
  void EraseByX1(Idx x1);
  void EraseByX1Ev(Idx x1, Idx ev);
  void EraseByX2(Idx x2);
  void EraseByEv(Idx ev);
  void EraseByX1OrX2(Idx x);
  void RestrictStates(const StateSet& states);
  bool Exists(Idx x1, Idx ev, Idx x2) const;
  bool ExistsByX1OrX2(Idx x) const;
  bool ExistsByX1Ev(Idx x1, Idx ev) const;
  bool ExistsByX1(Idx x1) const;
  // convenience extension: event by name
  %extend {
    bool Insert(Idx x1, const std::string& rEvent, Idx x2) {
      static EventSet defeset;
      return $self->Insert(x1,defeset.SymbolTablep()->Index(rEvent),x2); };
    bool Erase(Idx x1, const std::string& rEvent, Idx x2) {
      static EventSet defeset;
      return $self->Erase(x1,defeset.SymbolTablep()->Index(rEvent),x2); };
    void EraseByEv(const std::string& rEvent) {
      static EventSet defeset;
      $self->EraseByEv(defeset.SymbolTablep()->Index(rEvent)); };
    bool Exists(Idx x1, const std::string& rEvent, Idx x2) {
      static EventSet defeset;
      return $self->Exists(x1,defeset.SymbolTablep()->Index(rEvent),x2); };
    TransSet ## ORDER ## Iterator Find(Idx x1, const std::string& rEvent, Idx x2) {
      static EventSet defeset;
      return $self->Find(x1,defeset.SymbolTablep()->Index(rEvent),x2); };
  };
  // more iterators (depend on sorting!)
  TransSet ## ORDER ## Iterator Begin(Idx x1) const;
  TransSet ## ORDER ## Iterator End(Idx x1) const;
  TransSet ## ORDER ## Iterator Begin(Idx x1, Idx ev) const;
  TransSet ## ORDER ## Iterator End(Idx x1, Idx ev) const;
  TransSet ## ORDER ## Iterator BeginByEv(Idx ev) const;
  TransSet ## ORDER ## Iterator EndByEv(Idx ev) const;
  TransSet ## ORDER ## Iterator BeginByEvX1(Idx ev, Idx x1) const;
  TransSet ## ORDER ## Iterator EndByEvX1(Idx ev, Idx x1) const;
  TransSet ## ORDER ## Iterator BeginByEvX2(Idx ev, Idx x2) const;
  TransSet ## ORDER ## Iterator EndByEvX2(Idx ev, Idx x2) const;
  TransSet ## ORDER ## Iterator BeginByX2(Idx x2) const;
  TransSet ## ORDER ## Iterator EndByX2(Idx x2) const;
  TransSet ## ORDER ## Iterator BeginByX2Ev(Idx x2, Idx ev) const;
  TransSet ## ORDER ## Iterator EndByX2Ev(Idx x2, Idx ev) const;
  // resort
  void ReSort(TransSetX1EvX2& res) const;
  void ReSort(TransSetX1X2Ev& res) const;
  void ReSort(TransSetX2EvX1& res) const;
  void ReSort(TransSetX2X1Ev& res) const;
  void ReSort(TransSetEvX1X2& res) const;
  void ReSort(TransSetEvX2X1& res) const;
  // misc
  StateSet States(void) const;
  StateSet SuccessorStates(Idx x1) const;
  StateSet SuccessorStates(const StateSet&  x1set) const;
  StateSet SuccessorStates(Idx x1, Idx ev) const;
  StateSet SuccessorStates(const StateSet&  x1set, const EventSet& evset) const;
  EventSet ActiveEvents(Idx x1, SymbolTable* pSymTab=NULL) const;
  // convenience string conversion
  std::string Str(const Transition& rTrans) const;
  // convenience string conversion via iterator
  %extend {
    std::string Str(const TransSet ## ORDER ## Iterator& tit) const { return $self->Str(*tit);};
  };
};

%enddef

// Run macro to have types and interfaces
SwigTransSet(X1EvX2);
SwigTransSet(X1X2Ev);
SwigTransSet(X2EvX1);
SwigTransSet(X2X1Ev);
SwigTransSet(EvX1X2);
SwigTransSet(EvX2X1);


/*
**************************************************
**************************************************
**************************************************

Help topics

**************************************************
**************************************************
**************************************************
*/

SwigHelpTopic("IndexSet","IndexSet methods");

SwigHelpEntry("IndexSet","Constructors","IndexSet IndexSet()");
SwigHelpEntry("IndexSet","Constructors","IndexSet IndexSet(IndexSet)");
SwigHelpEntry("IndexSet","Constructors","IndexSet Copy()");

SwigHelpEntry("IndexSet","Maintenance","string Name()");
SwigHelpEntry("IndexSet","Maintenance"," Name(string)");
SwigHelpEntry("IndexSet","Maintenance","int Size()");
SwigHelpEntry("IndexSet","Maintenance"," Lock()");
SwigHelpEntry("IndexSet","Maintenance"," Detach()");
SwigHelpEntry("IndexSet","Maintenance","bool Empty()");

SwigHelpEntry("IndexSet","Element access","bool Insert(int)");
SwigHelpEntry("IndexSet","Element access","bool Erase(int)");
SwigHelpEntry("IndexSet","Element access","iterator Erase(iterator)");
SwigHelpEntry("IndexSet","Element access","iterator Begin()");
SwigHelpEntry("IndexSet","Element access","iterator End()");
SwigHelpEntry("IndexSet","Element access","bool Exists(int)");
SwigHelpEntry("IndexSet","Element access","iterator Find(int)");

SwigHelpEntry("IndexSet","Set operations"," InsertSet(IndexSet)");
SwigHelpEntry("IndexSet","Set operations"," EraseSet(IndexSet)");
SwigHelpEntry("IndexSet","Set operations","IndexSet SetIntersection(IndexSet)");
SwigHelpEntry("IndexSet","Set operations","IndexSet SetUnion(IndexSet)");
SwigHelpEntry("IndexSet","Set operations","IndexSet operator+(IndexSet)");
SwigHelpEntry("IndexSet","Set operations","IndexSet operator*(IndexSet)");
SwigHelpEntry("IndexSet","Set operations","IndexSet operator-(IndexSet)");
SwigHelpEntry("IndexSet","Set operations","bool operator<=(IndexSet)");
SwigHelpEntry("IndexSet","Set operations","bool operator<(IndexSet)");
SwigHelpEntry("IndexSet","Set operations","bool operator==(IndexSet)");


// ******************************************************

SwigHelpTopic("EventSet","EventSet methods");

SwigHelpEntry("EventSet","Constructors","EventSet EventSet()");
SwigHelpEntry("EventSet","Constructors","EventSet EventSet(EventSet)");
SwigHelpEntry("EventSet","Constructors","EventSet Copy()");

SwigHelpEntry("EventSet","Maintenance","string Name()");
SwigHelpEntry("EventSet","Maintenance"," Name(string)");
SwigHelpEntry("EventSet","Maintenance","int Size()");
SwigHelpEntry("EventSet","Maintenance"," Lock()");
SwigHelpEntry("EventSet","Maintenance"," Detach()");
SwigHelpEntry("EventSet","Maintenance","int Index(string)");
SwigHelpEntry("EventSet","Maintenance","string SymbolicName(int)");

SwigHelpEntry("EventSet","Element access","bool Empty()");
SwigHelpEntry("EventSet","Element access","bool Insert(string)");
SwigHelpEntry("EventSet","Element access","bool Insert(int)");
SwigHelpEntry("EventSet","Element access","bool Erase(string)");
SwigHelpEntry("EventSet","Element access","bool Erase(int)");
SwigHelpEntry("EventSet","Element access","iterator Erase(iterator)");
SwigHelpEntry("EventSet","Element access","bool Exists(string)");
SwigHelpEntry("EventSet","Element access","bool Exists(int)");
SwigHelpEntry("EventSet","Element access","iterator Find(string)");
SwigHelpEntry("EventSet","Element access","iterator Find(int)");
SwigHelpEntry("EventSet","Element access","iterator Begin()");
SwigHelpEntry("EventSet","Element access","iterator End()");

SwigHelpEntry("EventSet","Set operations"," InsertSet(EventSet)");
SwigHelpEntry("EventSet","Set operations"," EraseSet(EventSet)");
SwigHelpEntry("EventSet","Set operations","EventSet SetIntersection(EventSet)");
SwigHelpEntry("EventSet","Set operations","EventSet SetUnion(EventSet)");
SwigHelpEntry("EventSet","Set operations","EventSet operator+(EventSet)");
SwigHelpEntry("EventSet","Set operations","EventSet operator*(EventSet)");
SwigHelpEntry("EventSet","Set operations","EventSet operator-(EventSet)");
SwigHelpEntry("EventSet","Set operations","bool operator<=(EventSet)");
SwigHelpEntry("EventSet","Set operations","bool operator<(EventSet)");
SwigHelpEntry("EventSet","Set operations","bool operator==(EventSet)");

// ******************************************************

SwigHelpTopic("Alphabet","Alphabet methods");

SwigHelpEntry("Alphabet","Constructors","Alphabet Alphabet()");
SwigHelpEntry("Alphabet","Constructors","Alphabet Alphabet(Alphabet)");
SwigHelpEntry("Alphabet","Constructors","Alphabet Copy()");

SwigHelpEntry("Alphabet","Maintenance","string Name()");
SwigHelpEntry("Alphabet","Maintenance"," Name(string)");
SwigHelpEntry("Alphabet","Maintenance","int Size()");
SwigHelpEntry("Alphabet","Maintenance"," Lock()");
SwigHelpEntry("Alphabet","Maintenance"," Detach()");
SwigHelpEntry("Alphabet","Maintenance","int Index(string)");
SwigHelpEntry("Alphabet","Maintenance","string SymbolicName(int)");

SwigHelpEntry("Alphabet","Element access","bool Empty()");
SwigHelpEntry("Alphabet","Element access","bool Insert(string)");
SwigHelpEntry("Alphabet","Element access","bool Insert(int)");
SwigHelpEntry("Alphabet","Element access","bool Erase(string)");
SwigHelpEntry("Alphabet","Element access","bool Erase(int)");
SwigHelpEntry("Alphabet","Element access","iterator Erase(iterator)");
SwigHelpEntry("Alphabet","Element access","bool Exists(string)");
SwigHelpEntry("Alphabet","Element access","bool Exists(int)");
SwigHelpEntry("Alphabet","Element access","iterator Find(string)");
SwigHelpEntry("Alphabet","Element access","iterator Find(int)");
SwigHelpEntry("Alphabet","Element access","iterator Begin()");
SwigHelpEntry("Alphabet","Element access","iterator End()");

SwigHelpEntry("Alphabet","Set operations"," InsertSet(Alphabet)");
SwigHelpEntry("Alphabet","Set operations"," EraseSet(Alphabet)");
SwigHelpEntry("Alphabet","Set operations","Alphabet SetIntersection(Alphabet)");
SwigHelpEntry("Alphabet","Set operations","Alphabet SetUnion(Alphabet)");
SwigHelpEntry("Alphabet","Set operations","Alphabet operator+(Alphabet)");
SwigHelpEntry("Alphabet","Set operations","Alphabet operator*(Alphabet)");
SwigHelpEntry("Alphabet","Set operations","Alphabet operator-(Alphabet)");
SwigHelpEntry("Alphabet","Set operations","bool operator<=(Alphabet)");
SwigHelpEntry("Alphabet","Set operations","bool operator<(Alphabet)");
SwigHelpEntry("Alphabet","Set operations","bool operator==(Alphabet)");

SwigHelpEntry("Alphabet","Controllablity attributes","AttributeCFlag Attribute(int)");
SwigHelpEntry("Alphabet","Controllablity attributes","AttributeCFlag Attribute(string)");
SwigHelpEntry("Alphabet","Controllablity attributes","AttributeCFlag Attribute(iterator)");
SwigHelpEntry("Alphabet","Controllablity attributes"," Attribute(int,attr)");
SwigHelpEntry("Alphabet","Controllablity attributes"," Attribute(string,attr)");
SwigHelpEntry("Alphabet","Controllablity attributes"," Attribute(iterator,attr)");



// ******************************************************


SwigHelpTopic("TransSet","TransSet methods");

SwigHelpEntry("TransSet","Constructors","TransSet<Order> TransSet<Oder>()");
SwigHelpEntry("TransSet","Constructors","TransSet<Order> TransSet<Order>(TransSet<OtherOrder>)");
SwigHelpEntry("TransSet","Constructors","TransSet Copy()");

SwigHelpEntry("TransSet","Maintenance","string Name()");
SwigHelpEntry("TransSet","Maintenance"," Name(string)");
SwigHelpEntry("TransSet","Maintenance","int Size()");
SwigHelpEntry("TransSet","Maintenance"," Lock()");
SwigHelpEntry("TransSet","Maintenance"," Detach()");
SwigHelpEntry("TransSet","Maintenance","bool Empty()");

SwigHelpEntry("TransSet","Element access","bool Insert(trans)");
SwigHelpEntry("TransSet","Element access","bool Insert(x1,ev,x2)");
SwigHelpEntry("TransSet","Element access"," Erase(trans)");
SwigHelpEntry("TransSet","Element access"," Erase(x1,ev,x2)");
SwigHelpEntry("TransSet","Element access"," EraseByX1(x1)");
SwigHelpEntry("TransSet","Element access"," EraseByX1Ev(x1,ev)");
SwigHelpEntry("TransSet","Element access"," EraseByEv(ev)");
SwigHelpEntry("TransSet","Element access"," EraseByX2(x2)");
SwigHelpEntry("TransSet","Element access"," EraseByX1OrX2(x)");
SwigHelpEntry("TransSet","Element access"," EraseByX1OrX2(IndexSet)");
SwigHelpEntry("TransSet","Element access","iterator Erase(iterator)");
SwigHelpEntry("TransSet","Element access","iterator Begin()");
SwigHelpEntry("TransSet","Element access","iterator End()");
SwigHelpEntry("TransSet","Element access","iterator Begin(x1)");
SwigHelpEntry("TransSet","Element access","iterator End(x1)");
SwigHelpEntry("TransSet","Element access","iterator BeginByEv(ev)");
SwigHelpEntry("TransSet","Element access","iterator EndByEv(ev)");
SwigHelpEntry("TransSet","Element access","iterator BeginByEvX1(ev,x1)");
SwigHelpEntry("TransSet","Element access","iterator EndByEvX1(ev,x1)");
SwigHelpEntry("TransSet","Element access","iterator BeginByX2(x2)");
SwigHelpEntry("TransSet","Element access","iterator EndByX2(x2)");
SwigHelpEntry("TransSet","Element access","iterator BeginByEvX2(ev,x2)");
SwigHelpEntry("TransSet","Element access","iterator EndByEvX2(ev,x2)");
SwigHelpEntry("TransSet","Element access","iterator BeginByX2Ev(x2,ev)");
SwigHelpEntry("TransSet","Element access","iterator EndByX2Ev(x2,ev)");
SwigHelpEntry("TransSet","Element access","bool Exists(transition)");
SwigHelpEntry("TransSet","Element access","bool Exists(x1,ev,x2)");
SwigHelpEntry("TransSet","Element access","bool ExistsByX1Ev(x1,ev)");
SwigHelpEntry("TransSet","Element access","bool ExistsByX1(x1)");
SwigHelpEntry("TransSet","Element access","bool ExistsByX1OrX2(x)");
SwigHelpEntry("TransSet","Element access","iterator Find(transition)");
SwigHelpEntry("TransSet","Element access","iterator Find(x1,ev,x2)");


SwigHelpEntry("TransSet","Set operations"," InsertSet(TransSet)");
SwigHelpEntry("TransSet","Set operations"," EraseSet(TransSet)");
SwigHelpEntry("TransSet","Set operations","TransSet SetIntersection(TransSet)");
SwigHelpEntry("TransSet","Set operations","TransSet SetUnion(TransSet)");
SwigHelpEntry("TransSet","Set operations","TransSet operator+(TransSet)");
SwigHelpEntry("TransSet","Set operations","TransSet operator*(TransSet)");
SwigHelpEntry("TransSet","Set operations","TransSet operator-(TransSet)");
SwigHelpEntry("TransSet","Set operations","bool operator<=(TransSet)");
SwigHelpEntry("TransSet","Set operations","bool operator<(TransSet)");
SwigHelpEntry("TransSet","Set operations","bool operator==(TransSet)");

SwigHelpEntry("TransSet","Misc","IndexSet States()");
SwigHelpEntry("TransSet","Misc","IndexSet SuccessorStates(x1)");
SwigHelpEntry("TransSet","Misc","IndexSet SuccessorStates(IndexSet)");
SwigHelpEntry("TransSet","Misc","IndexSet SuccessorStates(x1,ev)");
SwigHelpEntry("TransSet","Misc","IndexSet SuccessorStates(IndexSet,EventSet)");
SwigHelpEntry("TransSet","Misc"," RestrictStates(IndexSet)");
SwigHelpEntry("TransSet","Misc","EventSet ActiveEvents(x1)");


