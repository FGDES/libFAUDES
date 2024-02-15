/** @file vectors.i @brief bindings for core vector classes */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2023  Thomas Moor
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

BaseVector: Template declaration

**************************************************
**************************************************
**************************************************
*/


template<class T>
class TBaseVector : public Type {
public:
  // constructors/destructors
  TBaseVector(void);
  TBaseVector(const TBaseVector& rOtherSet);
  TBaseVector(const std::string& rFilename, const std::string& rLabel = "BaseVector");
  virtual ~TBaseVector(void);
  // token io (explicit)
  SwigTypeMembers()
  // maintenance
  const std::string& Name(void) const;
  void Name(const std::string& rName);
  Idx Size(void) const;
  void Size(Idx len);
  bool Empty(void) const;
  //void TakeOwnership(void); // not needed, copies anyway
  //void TakeCopies(void);    // not needed, copies anyway
  // element access
  typedef std::vector<int>::size_type Position;
  virtual const T& At(const Position& pos) const; 
  virtual T& At(const Position& pos); 
  // operator [], friendly read/write access via integer iterator
  %extend {
    T& __getitem__(unsigned int pos) { 
      if(pos>=$self->Size()) $self->Size(pos+1); return $self->At(pos); };
  }; 
  %extend {
    void __setitem__(unsigned int pos, const T& elem) { 
       if(pos>=$self->Size()) $self->Size(pos+1); $self->Replace(pos,elem); };
  }; 
  // vector edit (only take references, ie maintain internal copy)
  virtual void Replace(const Position& pos, const T& rElem);
  //virtual void Replace(const Position& pos, T* pElem);
  virtual void Replace(const Position& pos, const std::string& rFileName);
  virtual void Erase(const Position& pos); 
  virtual void Insert(const Position& pos, const T& rElem);
  //virtual void Insert(const Position& pos, T* rElem);
  virtual void Insert(const Position& pos, const std::string& rFileName);
  virtual void PushBack(const T& rElem);
  //virtual void PushBack(T* rElem);
  virtual void PushBack(const std::string& rFileName);
  virtual void Append(const T& rElem);
  //virtual void Append(T* rElem);
  virtual void Append(const std::string& rFileName);
  void FilenameAt(const Position& pos, const std::string& rFileName);
  const std::string& FilenameAt(const Position& pos) const;
};

/*
**************************************************
**************************************************
**************************************************

BaseVector: actual data types

**************************************************
**************************************************
**************************************************
*/

// Announce template to SWIG
// Note: declare SWIG internal type and let SWIG know that 
// there is a corresponding C++ type
%template(EventSetVector) TBaseVector<EventSet>;
typedef TBaseVector<EventSet> EventSetVector;

// Announce template to SWIG
// Note: declare SWIG internal type and let SWIG know that 
// there is a corresponding C++ type
%template(AlphabetVector) TBaseVector<Alphabet>;
typedef TBaseVector<Alphabet> AlphaberVector;

// Announce template to SWIG
// Note: declare SWIG internal type and let SWIG know that 
// there is a corresponding C++ type
%template(GeneratorVector) TBaseVector<Generator>;
typedef TBaseVector<Generator> GeneratorVector;

// Announce template to SWIG
// Note: declare SWIG internal type and let SWIG know that 
// there is a corresponding C++ type
%template(SystemVector) TBaseVector<System>;
typedef TBaseVector<System> SystemVector;

// Announce template to SWIG
// Note: declare SWIG internal type and let SWIG know that 
// there is a corresponding C++ type
%template(IndexSetVector) TBaseVector<IndexSet>;
typedef TBaseVector<IndexSet> IndexSetVector;

/*
**************************************************
**************************************************
**************************************************

Help topics

**************************************************
**************************************************
**************************************************
*/

SwigHelpTopic("Vectors","Vector data types");

SwigHelpEntry("Vectors","Constructors", " GeneratorVector()");
SwigHelpEntry("Vectors","Constructors", " SystemVector()");
SwigHelpEntry("Vectors","Constructors", " EventSetVector()");

SwigHelpEntry("Vectors","Maintenance","string Name()");
SwigHelpEntry("Vectors","Maintenance"," Name(string)");
SwigHelpEntry("Vectors","Maintenance"," Clear()");
SwigHelpEntry("Vectors","Maintenance","int Size()");
SwigHelpEntry("Vectors","Maintenance"," Size(int)");
SwigHelpEntry("Vectors","Maintenance","bool Empty()");

SwigHelpEntry("Vectors","Element access","elem At(pos)");
SwigHelpEntry("Vectors","Element access"," Insert(pos,elem)");
SwigHelpEntry("Vectors","Element access"," Replace(pos,elem)");
SwigHelpEntry("Vectors","Element access"," Append(elem)");
SwigHelpEntry("Vectors","Element access"," PushBack(elem)");

SwigHelpEntry("Vectors","Misc","string FilenameAt(pos)");
SwigHelpEntry("Vectors","Misc"," FilenameAt(pos,string)");


