/** @file omg_rabinacc.h Rabin acceptance condition */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2025 Thomas Moor
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

#ifndef FAUDES_OMG_RABINACC_H
#define FAUDES_OMG_RABINACC_H

#include "corefaudes.h"

namespace faudes {

/**
 * The class RabinPair models a pair of state sets. 
 * We use faudes Type as base class to faciltate serialisation
 * of sets of RabinPairs aka RabinAcceptance.
 *
 * @ingroup OmegaautPlugin
 */
class FAUDES_API RabinPair : public ExtType {

FAUDES_TYPE_DECLARATION(Void,RabinPair,ExtType)
  
public:
 /**
  * Default constructor, all empty
  */
  RabinPair(void);

 /** 
  * Copy constructor.
  */
  RabinPair(const RabinPair& rRP);
 
 /** 
  * Destructor 
  */
  virtual ~RabinPair(void) {};

 /**
  * Clear to default (all empty)
  */
  virtual void Clear(void);

  /*
   * Member access R
   *
   * @return
   *   writable ref to Set of states type R
   */
  StateSet& RSet(void) {return mRSet;};

  /*
   * Member access R
   *
   * @return
   *   const ref to Set of states type R
   */
  const StateSet& RSet(void) const {return mRSet;};

  /**
   * Member access I
   *
   * @return
   *   writable ref to Set of states type I
   */
  StateSet& ISet(void) {return mISet;};

  /**
   * Member access I
   *
   * @return
   *   const ref to Set of states type I
   */
  const StateSet& ISet(void) const {return mISet;};

  /**
   * Restrict to specified State set
   *
   * @param rDomain
   *  State set to restrict to
   */
  void RestrictStates(const StateSet& rDomain);

  /**
   * Compare for set-ordering
   *
   * @param rOther
   *   other Rabin pair to compare with
   * @return
   *   true if "this>other"
   */
  bool operator<(const RabinPair& rOther) const;

protected:

  /**
   * set of states type R
   */
  StateSet mRSet;

  /**
   * set of states type I
   */
  StateSet  mISet;

  /**
   * Assignment method.
   *
   * @param rSrc
   *    Source to assign from
   */
  void DoAssign(const RabinPair& rSrc);
  
  /**
   * Test equality of configuration data.
   *
   * @param rOther
   *   RabinPair to compare with
   * @return
   *   True on match.
   */
  bool DoEqual(const RabinPair& rOther) const;
  
  /**
   * Read configuratioj from TokenReader, see Type for public wrappers.
   * Use am optional Generator context for symbolic state names.
   *
   * @param rTr
   *   TokenReader to read from
   * @param rLabel
   *   Section to read
   * @param pContext
   *   Provide contextual information
   *
   * @exception Exception
   *   - IO error (id 1)
   */
  virtual void DoRead(TokenReader& rTr, const std::string& rLabel="", const Type* pContext=nullptr);

  /**
    * Writes configuration to TokenWriter, see Type for public wrappers.
    * Use am optional Generator context for symbolic state names.
    *
    * @param rTw
    *   TokenWriter to write to
    * @param rLabel
    *   Section to write
    * @param pContext
    *   Provide contextual information
    *
    * @exception Exception
    *   - IO error (id 2)
    */
  virtual void DoWrite(TokenWriter& rTw,const std::string& rLabel="", const Type* pContext=nullptr) const;

  /**
    * Writes configuration to TokenWriter, see Type for public wrappers.
    * Use am optional Generator context for symbolic state names.
    *
    * @param rTw
    *   TokenWriter to write to
    * @param rLabel
    *   Section to write
    * @param pContext
    *   Provide contextual information
    *
    * @exception Exception
    *   - IO error (id 2)
    */
  
  virtual void DoXWrite(TokenWriter& rTw,const std::string& rLabel="", const Type* pContext=nullptr) const;

};


/**
 * The class RabinAcceptance is a set of RabinPairs.
 * We use faudes::TBaseVector as the base class and perhaps
 * add some convenience accessors in due course.
 *
 * @ingroup OmegaautPlugin
 */
class FAUDES_API RabinAcceptance : public TBaseVector<RabinPair> {

FAUDES_TYPE_DECLARATION(Void,RabinAcceptance,vBaseVector<RabinPair>)
  
public:
 /**
  * Default constructor (all empty)
  */
  RabinAcceptance(void);

 /** 
  * Copy constructor.
  */
  RabinAcceptance(const RabinAcceptance& rRA);
 
 /** 
  * Construct from file.
  */
  RabinAcceptance(const std::string& rFileName);

  /** 
  * Destructor 
  */
  virtual ~RabinAcceptance(void) {};

  /**
   * Restrict to specified State set
   *
   * @param rDomain
   *  State set to restrict to
   */
  void RestrictStates(const StateSet& rDomain);


protected:

  /** 
   * Token output, see Type::SWrite for public wrappers.
   * Statistics include sizes of members and is meant to record test cases
   *
   * @param rTw
   *   Reference to TokenWriter
   */
  virtual void DoSWrite(TokenWriter& rTw) const;

};

}//namespace
#endif//.H
