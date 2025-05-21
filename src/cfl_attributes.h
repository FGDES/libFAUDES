/** @file cfl_attributes.h Classes AttributeVoid and AttributeFlags  */


/* FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2007, 2024  Thomas Moor
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


#ifndef FAUDES_ATTRIBUTES_H
#define FAUDES_ATTRIBUTES_H

#include "cfl_types.h"
#include "cfl_registry.h"
#include "cfl_tokenreader.h"
#include "cfl_tokenwriter.h"

namespace faudes {

// v2.33 transitioning: select minimal attribute to be AttrType
typedef AttrType AttributeVoid;  
  

/** Convenience typdef flag data */
typedef unsigned long int fType;

/**
 * Boolean flags Attribute. This attribute class uses a flag word to represent 
 * boolean values. The current implementation uses a long int type and hence handles
 * up to 32 flags. Each flag is accessed by the according bit mask. 
 *
 * The current version of libFAUDES uses bits 0,1,2 of event attributes for controllability
 * properties (see AttributeCFlags) and the bits 31 and 32 of state attributes for the Qt based
 * GUI project. Further versions may use the lower 8 bits of the event aflags and the higher 8
 * bits of state flags. You are free to use any flags for your application, but if possible try
 * to avoid the above mentioned. For extensive use of flags, you may also consider to
 * define a separate attribute class, perhaps with a different fType.
 */


class FAUDES_API AttributeFlags : public AttributeVoid {

FAUDES_TYPE_DECLARATION(Void,AttributeFlags,AttributeVoid)

public:

  /** modern compilers seem to insist in explicit using */
  using AttributeVoid::operator=;
  using AttributeVoid::operator==;
  using AttributeVoid::operator!=;

  /** Default constructor */
  AttributeFlags(void) : AttributeVoid(), mFlags(mDefFlags) {};

  /** Copy constructor */
  AttributeFlags(const AttributeFlags& rOther) : AttributeVoid(), mFlags(rOther.mFlags) {};

  /** Destructor */
  virtual ~AttributeFlags(void) {};

  /**
   * Test a flag
   */
  bool Test(fType mask) const { return ( (mFlags & mask) != 0 ); };

  /**
   * Test multible flags, combine by "and"
   */
  bool TestAll(fType mask) const { return ( (mFlags & mask) == mask ); };

  /**
   * Test multible flags, combine by "or"
   */
  bool TestSome(fType mask) const { return ( (mFlags & mask) != 0 ); };

  /**
   * Test multible flags, combine by "not or"
   */
  bool TestNone(fType mask) const { return ( (mFlags & mask) == 0 ); };

  /**
   * Set multiple flags
   */
  void Set(fType mask) {mFlags |= mask; };

  /**
   * Clear multiple flags
   */
  void Clr(fType mask) {mFlags &= ~mask; };

  /** 
   * Test for default value
   */
  virtual bool IsDefault(void) const {return mFlags==mDefFlags;};

  /** Flags (public access for convenience)  */
  fType mFlags;

  /* default flags */
  const static fType mDefFlags=0x0;


protected:

  /**
   * Assignment method. 
   *
   * @param rSrcAttr
   *    Source to assign from
   */
  void DoAssign(const AttributeFlags& rSrcAttr);

  /**
   * Test equality of configuration data.
   *
   * @param rOther 
   *    Other object to compare with.
   * @return 
   *   True on match.
   */
  bool DoEqual(const AttributeFlags& rOther) const;


  /**
   * Reads attribute from TokenReader, see Type for public wrappers.
   *
   * Test whether the current token is an integer with base 16. If so, read the token to the
   * flags value. Else, dont take any tokens. The label and context arguments are ignored.
   *
   * @param rTr
   *   TokenReader to read from
   * @param rLabel
   *   Section to read
   * @param pContext
   *   Read context to provide contextual information
   *
   * @exception Exception
   *   - IO error (id 1)
   */
  virtual void DoRead(TokenReader& rTr,  const std::string& rLabel = "", const Type* pContext=0);
 
  /**
   * Write to TokenWriter, see Type for public wrappers.
   *
   * If not the defult value, write the flag value as base 16 integer token. Else,
   * do nothing. The label and context arguments are ignored.
   *
   * @param rTw
   *   Reference to TokenWriter
   * @param rLabel
   *   Label of section to write
   * @param pContext
   *   Write context to provide contextual information
   *
   * @exception Exception 
   *   - IO errors (id 2)
   */
  virtual void DoWrite(TokenWriter& rTw, const std::string& rLabel="",const Type* pContext=0) const;

  /**
   * Write to TokenWriter, see Type for public wrappers.
   *
   * If not the defult value, write the flags in XML format.
   * Else, do nothing. The label and context arguments are ignored.
   *
   * @param rTw
   *   Reference to TokenWriter
   * @param rLabel
   *   Label of section to write
   * @param pContext
   *   Write context to provide contextual information
   *
   * @exception Exception 
   *   - IO errors (id 2)
   */
  virtual void DoXWrite(TokenWriter& rTw, const std::string& rLabel="",const Type* pContext=0) const;



}; // class AttributeFlags


/**
 * Attribute class to model event controllability properties.
 *
 * This attribute is meant to be an event attribute and can distinguish between
 * controllable, observable, forcible and abstraction events. It is based on faudes::AttributeFlags
 * and uses the lower four bits in the flag word to store the respective boolean values.
 * The AttributeCFlags class adds convenience functions to access these bits and a default-value 
 * that corresponds to observable and neiter controllable nor forcible.
 *
 * Presuming that only controllability flags are uses (different from default), the
 * token representation is by an Option String consisting of the initials <tt>c</tt>,<tt>o</tt>,<tt>f</tt> 
 * and <tt>a</tt>, where initials are capitatised for set flags and default values 
 * are not written; eg <tt>+C+</tt> 
 * for a controllable event that is observable (default),  not forcible (default) and  
 * an abstraction event (default).
 * If other than the four controllability bits are used, std. hex format is used.
 * 
 */


class FAUDES_API AttributeCFlags : public AttributeFlags {

FAUDES_TYPE_DECLARATION(Void,AttributeCFlags,AttributeFlags)

 public:

  using AttributeFlags::operator=;
  using AttributeFlags::operator==;
  using AttributeFlags::operator!=;

  /**
   * Default constructor 
   */
  AttributeCFlags(void) : AttributeFlags() { mFlags=mDefCFlags; };

  /** Destructor */
  virtual ~AttributeCFlags(void) {};

  /**
   * Set controllable flag 
   */
  void SetControllable(void) { mFlags |= mControllableFlag; }

  /**
   * Clear controllable flag 
   */

  void ClrControllable(void) { mFlags &= ~mControllableFlag; };
     
  /**
   * Query controllablility 
   */
  bool Controllable(void) const {return ( (mFlags & mControllableFlag) != 0 ); }


  /**
   * Set observable flag 
   */
  void SetObservable(void) { mFlags |= mObservableFlag; }

  /**
   * Clear observable flag 
   */
  void ClrObservable(void) { mFlags &= ~mObservableFlag; };
     
  /**
   * Query observablility 
   */
  bool Observable(void) const {return ( (mFlags & mObservableFlag) != 0 ); }


  /**
   * Set forcible flag 
   */
  void SetForcible(void) { mFlags |= mForcibleFlag; }

  /**
   * Clear forcible flag 
   */

  void ClrForcible(void) { mFlags &= ~mForcibleFlag; };
     
  /**
   * Query forcibility 
   */
  bool Forcible(void) const {return ( (mFlags & mForcibleFlag) != 0 ); }


  /**
   * Set abstraction flag 
   */
  void SetHighlevel(void) { mFlags |= mAbstractionFlag; }

  /**
   * Clear abstraction flag 
   */
  void SetLowlevel(void) { mFlags &= ~mAbstractionFlag; };
     
  /**
   * Query abstaction flag
   */
  bool Highlevel(void) const {return ( (mFlags & mAbstractionFlag) != 0 ); }

  /**
   * Query abstaction flag
   */
  bool Lowlevel(void) const {return ( (mFlags & mAbstractionFlag) == 0 ); }


  /** 
   * Test for default value
   */
  virtual bool  IsDefault(void) const {return mFlags==mDefCFlags;};

  // flag masks for the three properties
  const static fType mControllableFlag =0x01;
  const static fType mObservableFlag   =0x02;
  const static fType mForcibleFlag     =0x04;
  const static fType mAbstractionFlag  =0x08;

 private:
  /** Overall default value */
  const static fType mDefCFlags         =0x0a;

  /** All flags used by CFlags */
  const static fType mAllCFlags         =0x0f;

 protected:

  /**
   * Assignment method. 
   *
   * @param rSrcAttr
   *    Source to assign from
   */
  void DoAssign(const AttributeCFlags& rSrcAttr);

  /**
   * Test equality of configuration data.
   *
   * @param rOther 
   *    Other attribute to compare with.
   * @return 
   *   True on match.
   */
  bool DoEqual(const AttributeCFlags& rOther) const;

  /**
   * Reads attribute from TokenReader, see AttributeVoid for public wrappers.
   * Reads a single token if it can be interpreted as AttributeCFlag, that is, if
   * it is a respective option string or hex number. Label and Context
   * argument are ignored. No token mismatch exceptions are thrown on error.
   *
   * @param rTr
   *   TokenReader to read from
   * @param rLabel
   *   Section to read
   * @param pContext
   *   Read context to provide contextual information
   *
   * @exception Exception
   *   - IO error (id 1)
   */
  virtual void DoRead(TokenReader& rTr, const std::string& rLabel="", const Type* pContext=0);
 
  /**
   * Writes attribute to TokenWriter, see AttributeVoid for public wrappers.
   * Label and Context argument are ignored.  
   *
   * @param rTw
   *   TokenWriter to write to
   * @param rLabel
   *   Section to write
   * @param pContext
   *   Write context to provide contextual information
   *
   * @exception Exception
   *   - IO error (id 2)
   */
  virtual void DoWrite(TokenWriter& rTw,const std::string& rLabel="", const Type* pContext=0) const;


  /**
   * Writes attribute to TokenWriter (XML format), see AttributeVoid for public wrappers.
   * Label and Context argument are ignored.  
   *
   * @param rTw
   *   TokenWriter to write to
   * @param rLabel
   *   Section to write
   * @param pContext
   *   Write context to provide contextual information
   *
   * @exception Exception
   *   - IO error (id 2)
   */
  virtual void DoXWrite(TokenWriter& rTw,const std::string& rLabel="", const Type* pContext=0) const;



}; // class AttributeCFlags

} // namespace faudes

#endif
