/** @file pev_priorities.h Classes EventPriorities */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2025 Yiheng Tang, Thomas Moor
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

#ifndef FAUDES_PEV_PRIORITIES_H
#define FAUDES_PEV_PRIORITIES_H

#include "corefaudes.h"

namespace faudes {

/*!
 * \brief The AttributePriority class
 * Class denoting the event priority of events in a
 * prioritised generator (PGenerator)
 */

class FAUDES_API AttributePriority : public AttributeCFlags {

FAUDES_TYPE_DECLARATION(Void,AttributePriority,AttribiteCFlags)

public:
  /**
   * Default constructor, sets priority to 0
   */
  AttributePriority(void) : AttributeCFlags() {mPriority=0;}

  /** 
   * Copy constructor.
   */
  AttributePriority(const AttributePriority& rSrc) : AttributeCFlags() { 
    DoAssign(rSrc); } 

  /** Destructor */
  virtual ~AttributePriority(void) {}

  /**
   * Clear to default (mandatory for serialisation)
   */
  void Clear(void) { mPriority = 0; }

  /** 
   * Test for default value 
   * 
   * @return
   *   True, if this attribute has its default value
   */
  virtual bool IsDefault(void) const {
    return (mPriority==0) && AttributeCFlags::IsDefault(); };

  /** set priority */
  void Priority (Idx prio) {mPriority = prio;}

  /** get priority */
  Idx Priority(void) const { return mPriority;}

  /** compare priority (strictly ensure that a<b iff a preempts b)*/
  bool operator<(const AttributePriority& rOther) const {
    if(mPriority < rOther.mPriority ) return true;
    return false;
  }

protected:
  /**
   * priority value. 0 is highest.
   */
  Idx mPriority = 0;

  /**
   * Assignment method.
   *
   * @param rSrcAttr
   *    Source to assign from
   */
  void DoAssign(const AttributePriority& rSrcAttr);
  
  /**
   * Test equality of configuration data.
   *
   * @param rOther
   *    Other attribute to compare with.
   * @return
   *   True on match.
   */
  bool DoEqual(const AttributePriority& rOther) const;
  
  /**
   * Reads attribute from TokenReader, see AttributeVoid for public wrappers.
   * Label and Context argument are ignored.
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
  virtual void DoRead(TokenReader& rTr, const std::string& rLabel="", const Type* pContext=nullptr);

  /**
    * Writes attribute to TokenWriter, see AttributeVoid for public wrappers.
    * Note: this class always wrtites in XML.
    * Note: Label and Context argument are ignored.
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
  virtual void DoWrite(TokenWriter& rTw,const std::string& rLabel="", const Type* pContext=nullptr) const;

  /**
    * Writes attribute to TokenWriter, see AttributeVoid for public wrappers.
    * Note: this class always wrtites in XML.
    * Note: Label and Context argument are ignored.
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
  virtual void DoXWrite(TokenWriter& rTw,const std::string& rLabel="", const Type* pContext=nullptr) const; 


};

/*
******************************************************** 
******************************************************** 
******************************************************** 

TpEventSet

convenience class to declare universal event priorities

******************************************************** 
******************************************************** 
******************************************************** 
*/

template <class Attr>
class FAUDES_TAPI TpEventSet : public TaEventSet<Attr> {

  /** std faudes type declarations */
  FAUDES_TYPE_TDECLARATION(Void,TpEventSet,TaEventSet<Attr>)

  /**
   * Default constructor
   */
   TpEventSet(void) : TaEventSet<Attr>(){};

  /** 
   * Copy constructor
   *
   * @param rOther
   */
  TpEventSet(const EventSet& rOther) : TaEventSet<Attr>(rOther){};
        
  /** 
   * Copy constructor
   *
   * @param rOther
   */
  TpEventSet(const TpEventSet& rOther) : TaEventSet<Attr>(rOther) {};
        
  /**
   * Construct from file
   * 
   * @param rFileName
   *   Filename
   * @param rLabel
   *   Label to read from 
   *
   * @exception Exception
   *   If opening/reading fails an Exception object is thrown (id 1, 50, 51)
   */
   TpEventSet(const std::string& rFileName, const std::string& rLabel="") : TaEventSet<Attr>(rFileName,rLabel) {};


  /** recycle Iterator from nameset */
  using NameSet::Iterator;
      

  /**
   * Get priority by symbolic name
   *
   * @param rName
   *   specify element
   *
   * @exception Exception
   *   Base class with throw if synbol does not exist
   *
   * @return priority of specified event
   */
  Idx Priority(const std::string& rName) const { return this->Attribute(rName).Priority(); }

  /**
   * Get Priority by index
   *
   * @param rIdx
   *   specify element
   *
   * @exception Exception
   *   Base class with throw if specified element does not exist
   *
   * @return priority of specified event
   */
  Idx Priority(const Idx idx) const { return this->Attribute(idx).Priority(); }

  /**
   * Get lowest priority
   *
   * @return lowest priority
   */
  Idx LowestPriority(void) const {
    Idx low=0;
    NameSet::Iterator eit = this->Begin();
    NameSet::Iterator eit_end = this->End();
    for(;eit!=eit_end;eit++){
      Idx prio=Priority(*eit);
      if(prio>low) low =prio;
    }
    return low;
  }
  
   /**
   * Set priority by symbolic name
   *
   * @param rName
   *   specify element
   *
   * @param prio
   *   specify priority
   *
   * @exception Exception
   *   Base class with throw if synbol does not exist
   *
   */
  void Priority(const std::string& rName, const Idx prio) { this->Attributep(rName)->Priority(prio); }

  /**
   * Set Priority by index
   *
   * @param idx
   *   spcify element
   *
   * @param prio
   *   specify priority
   *
   * @exception Exception
   *   Base class with throw if specified element does not exist
   *
   */
  void Priority(const Idx idx, const Idx prio) { this->Attributep(idx)->Priority(prio); }
  
  /**
   * Insert with priority by index
   *
   * @param idx
   *   spcify element
   *
   * @param prio
   *   specify priority
   *
   */
  void InsPriority(const Idx idx, const Idx prio) {
    this->Insert(idx); Priority(idx,prio);
  }
  
  /**
   * Insert with priority by name
   *
   * @param rName
   *   spcify element
   *
   * @param prio
   *   specify priority
   *
   */
  void InsPriority(const std::string& rName, const Idx prio) {
    Idx idx=this->Insert(rName);
    Priority(idx,prio);
  }
  
  /**
   * Set Priorities from other prioritised event set
   *
   * @param rOtherSet
   *   set to get priorities from
   *
   */
  void Priorities(const TpEventSet& rOtherSet) {
    NameSet::Iterator eit=this->Begin();  
    NameSet::Iterator eit_end=this->End();
    for(;eit!=eit_end;++eit) {
      if(rOtherSet.Exists(*eit))
	Priority(*eit,rOtherSet.Priority(*eit));
    }
  }
    

 protected:

  /**
   * Reimplement DoWrite to enforce XML token-io
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
  virtual void DoWrite(TokenWriter& rTw,const std::string& rLabel="", const Type* pContext=nullptr) const {
    NameSet::DoXWrite(rTw,rLabel,pContext);
  }
};



 

/** 
 * Convenience typedef for std alphabet with priorities
 *
 */
typedef TpEventSet<AttributePriority> EventPriorities;


/*
******************************************************** 
******************************************************** 
******************************************************** 

TpEventSet: implementation

******************************************************** 
******************************************************** 
******************************************************** 
*/
 
/* convenience access to relevant scopes */
#define THIS TpEventSet<Attr>
#define BASE TaEventSet<Attr>

FAUDES_TYPE_TIMPLEMENTATION(Void,TpEventSet<Attr>,TaEventSet<Attr>,template <class Attr>)
 
#undef BASE
#undef THIS




}//namespace
#endif//.H
