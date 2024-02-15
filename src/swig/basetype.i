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

  // maintenance
  virtual Type& Assign(const Type& rType);
  %extend {  
    Type Copy(void) const { return *($self->Copy());}; // fix swig ownership flag
  }
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


// helper: report object type
%rename(TypeName) FaudesTypeName;
const std::string& FaudesTypeName(const Type& rObject);

// helper: test faudes cast
%rename(TypeTest) FaudesTypeTest;
bool FaudesTypeTest(const std::string& rTypeName, const Type& rObject);





