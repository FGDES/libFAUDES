/** @file attributes.i @brief bindings for core attribute classes */


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





// extra c code for swig
%{

%}


// the plain attribute
class AttributeVoid {
 public:
  // construct/destruct
  AttributeVoid(void);
  virtual ~AttributeVoid(void);
  // token output
  void Write(const Type* pContext=0) const;
  void Write(const std::string& pFileName, const std::string& rLabel="",
	     const Type* pContext=0) const;
  std::string ToString(const std::string& rLabel="", const Type* pContext=0) const;
  // token input
  void Read(const std::string& rFileName, const std::string& rLabel = "", const Type* pContext=0);
  // provide lua string conversion
  %extend {
    std::string __str__(void) { return $self->ToString();};
  }
  // basic maintenance
  virtual bool IsDefault(void);
  virtual void SetDefault(void);
  virtual void Clear(void);
}; 


// faudes flag type
typedef unsigned long int fType;

// flag attribute
class AttributeFlags : public AttributeVoid {
 public:
  // construct/destruct
  AttributeFlags(void);
  virtual ~AttributeFlags(void);
  // read acces
  bool Test(fType mask) const;
  bool TestAll(fType mask) const;
  bool TestSome(fType mask) const;
  bool TestNone(fType mask) const;
  // write access
  void Set(fType mask);
  void Clr(fType mask);
  // convenience access
  fType mFlags;
}; 

// control system flags
class AttributeCFlags : public AttributeFlags {
 public:
  // construct/destruct
  AttributeCFlags(void);
  virtual ~AttributeCFlags(void);
  // extra access to c features
  void SetControllable(void);
  void ClrControllable(void);
  bool Controllable(void) const;
  void SetObservable(void);
  void ClrObservable(void);
  bool Observable(void) const;
  void SetForcible(void);
  void ClrForcible(void);
  bool Forcible(void) const;
  // flag masks for the three properties
  static const fType mControllableFlag=0x01;
  static const fType mObservableFlag  =0x02;
  static const fType mForcibleFlag    =0x04;
};
