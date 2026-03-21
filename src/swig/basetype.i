/** @file basetype.i @brief bindings for faudes base type */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008, 2023  Thomas Moor
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

Faudes atomics Idx and Transition

**************************************************
**************************************************
**************************************************
*/

// faudes index type
typedef uint32_t Idx;
typedef long int Int;
typedef double Float;

// faudes transition type
class Transition {
 public:
  // construct/destruct  
  Transition(void);
  Transition(Idx x1, Idx ev,Idx x2);
  // operators
  bool operator < (const Transition& othertrans) const;
  bool operator == (const Transition& othertrans) const;
  // maintenance/access
  bool Valid(void) const;
  Idx X1;
  Idx Ev; 
  Idx X2;
  // convenience
  std::string Str(void) const;
  // provide automatic conversion
  %extend {
    std::string __str__(void) { return $self->Str();};
  };
};

// convenience: construct transition by event name
%extend Transition {
   Transition(Idx x1, const std::string& rEvent, Idx x2) {
   static EventSet defeset;
   return new Transition(x1,defeset.SymbolTablep()->Index(rEvent),x2); };
};

/*
**************************************************
**************************************************
**************************************************
Type: faudes base class

**************************************************
**************************************************
**************************************************
*/


// macro for universal faudes base class members
%define SwigTypeMembers()

  // make Copy() available
  %extend {  
    %newobject Copy;
    Type* Copy(void) const { return $self->NewCpy();}; 
  }
  // legacy Assign() [from]
  %extend {
    void Assign(const Type& rType) { $self->Copy(rType);};
  }

  // clear
  virtual void Clear(void);

  // token output
  void Write(const Type* pContext=0) const;
  void Write(const std::string& pFileName, const std::string& rLabel="", const Type* pContext=0) const;
  std::string ToString(const std::string& rLabel="", const Type* pContext=0) const;
  void DWrite(const Type* pContext=0) const;
  void DWrite(const std::string& pFileName, const std::string& rLabel="", const Type* pContext=0) const;
  void SWrite(void) const;
  void XWrite(const Type* pContext=0) const;
  void XWrite(const std::string& pFileName, const std::string& rLabel="", const Type* pContext=0) const;
  // token based input
  void Read(const std::string& rFileName, const std::string& rLabel = "", const Type* pContext=0);
  void FromString(const std::string& rString, const std::string& rLabel = "", const Type* pContext=0);
  // provide automatic conversion
  %extend {
    std::string __str__(void) { return $self->ToString();};
  }
%enddef


// universal faudes base class
class Type {
public:
  // construct/destruct
  Type(void);
  Type(const Type& rType);
  virtual ~Type();

  // have token io
  SwigTypeMembers()
};


// macro for universal faudes base class members, extended as ov v2.33)
%define SwigExtTypeMembers()
  const std::string& Name(void) const;
  void Name(const std::string& rName);
%enddef  

// universal faudes base class (extenden as ov v2.33)
class ExtType : public Type {
public:
  // construct/destruct
  ExtType(void);
  ExtType(const ExtType& rType);
  virtual ~ExtType();

  // have token io etc
  SwigExtTypeMembers()
};


// helper: report object type
const std::string& TypeName(const Type& rObject);

// helper: test faudes cast
%feature("autodoc","1");
bool TypeTest(const std::string& rTypeName, const Type& rObject);

// helper: instatiate by tpeanme
%rename(New) NewObject;
%feature("autodoc","1");
Type*  NewObject(const std::string& rTypeName);

// helper: instatiate from string
%rename(NewFromString) NewObjectFromString;
%feature("autodoc","1");
Type*  NewObjectFromString(const std::string& rTypeName, const std::string& rData);

// helper: instatiate from file
%rename(NewFromFile) NewObjectFromFile;
%feature("autodoc","1");
Type*  NewObjectFromFile(const std::string& rTypeName, const std::string& rData);






