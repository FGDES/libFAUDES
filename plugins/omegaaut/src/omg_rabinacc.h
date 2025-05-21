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
class FAUDES_API RabinPair : public Type {

FAUDES_TYPE_DECLARATION(Void,RabinPair,Type)
  
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

  /** 
   * Return name of RabinPair
   * (need to implement this because Type is the direct base) 
   *
   * @return
   *   Name to get
   */
  const std::string& Name(void) const {return mName;};
        
  /**
   * Set name of RabinPair
   * (need to implement this because Type is the direct base) 
   *
   * @param rName
   *   Name to set
   */
  void Name(const std::string& rName) {mName=rName;};


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
   * cosmetic name (not provided by plain base faudes Type);
   */
  std::string mName;

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
 * We use faudes::vBaseSet as the base class and perhaps
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


protected:

  /** 
   * Overwrite base TBaseSet to write individial RobinPairs
   *
   * @param rTw
   *   Reference to TokenWriter
   * @param rElem
   *   The element to write
   * @param rLabel
   *   Label of section to write, defaults to XElemenTag
   * @param pContext
   *   Write context to provide contextual information
   */
  virtual void DoWriteElement(TokenWriter& rTw, const RabinPair& rElem, const std::string &rLabel="", const Type* pContext=0) const;

  /** 
   * Overwrite base TBaseSet to write individial RobinPairs
   *
   * @param rTw
   *   Reference to TokenWriter
   * @param rElem
   *   The element to write
   * @param rLabel
   *   Label of section to write, defaults to XElemenTag
   * @param pContext
   *   Write context to provide contextual information
   */
  virtual void DoXWriteElement(TokenWriter& rTw, const RabinPair& rElem, const std::string &rLabel="", const Type* pContext=0) const;

  /** 
   * Token output, see Type::SWrite for public wrappers.
   * Statistics include sizes of members and is meant to record test cases
   *
   * @param rTw
   *   Reference to TokenWriter
   */
  virtual void DoSWrite(TokenWriter& rTw) const;

  /**
   * Token input for individual RobinPair, incl. factoru and insertion
   *
   * @param rTr
   *   Reference to TokenReader
   * @param rLabel
   *   Label of section to read, defaults to name of set
   * @param pContext
   *   Read context to provide contextual information
   */
  virtual void DoInsertElement(TokenReader& rTr, const std::string& rLabel, const Type* pContext);

};

}//namespace
#endif//.H
