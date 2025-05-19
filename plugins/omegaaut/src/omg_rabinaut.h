/** @file omg_rabinaut.h Class RabinAutomaton */


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

#ifndef FAUDES_OMG_RABINAUT_H
#define FAUDES_OMG_RABINAUT_H

#include "corefaudes.h"
#include "omg_rabinacc.h"

namespace faudes {


/**
 * Generator with Rabin Acceptance conditiom. 
 * 
 * @section SecRabinAut Overview
 * 
 * The TrGenerator is a variant of the TcGenerator to add an interface for a Rabin acceptance condition.
 *
 * Technically, the construct is based on the global attribute class faudes::RabinAcceptance
 * derived from faudes::AttributeVoid. Hence TrGenerator expects an event attribute template parameter
 * with the minimum interface defined in RabinAcceptance.
 *
 * For convenience, the configuration with the minimum attributes is been typedef-ed as RabinAutomaton.
 *
 * @ingroup GeneratorClasses
 */

template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
    class FAUDES_TAPI TrGenerator : public TcGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr> {    
  public:


    /**
     * Creates an emtpy RabinAutomaton object 
     */
    TrGenerator(void);

    /** 
     * RabinAutomaton from a std Generator. Copy constructor 
     *
     * @param rOtherGen
     */
    TrGenerator(const vGenerator& rOtherGen);
        
    /** 
     * RabinAutomaton from RabinAutomaton. Copy constructor 
     *
     * @param rOtherGen
     */
    TrGenerator(const TrGenerator& rOtherGen);

    /**
     * Construct from file
     *
     * @param rFileName
     *   Filename
     *
     * @exception Exception
     *   If opening/reading fails an Exception object is thrown (id 1, 50, 51)
     */
    TrGenerator(const std::string& rFileName);

    /**
     * Construct on heap
     *
     * @return 
     *   new Generator 
     */
     TrGenerator* New(void) const;

    /**
     * Construct copy on heap
     *
     * @return 
     *   new Generator 
     */
     TrGenerator* Copy(void) const;

    /**
     * Type test.
     * Uses C++ dynamic cast to test whether the specified object
     * casts to a RabinAutomaton.
     *
     * NOT IMPLEMENTED
     *
     * @param pOther
     *   poinetr to object to test
     *
     * @return 
     *   TpGenerator reference if dynamic cast succeeds, else NULL 
     */
     virtual const Type* Cast(const Type* pOther) const {
       return dynamic_cast< const TrGenerator* > (pOther); };


    /**
     * Construct on stack
     *
     * @return 
     *   new Generator 
     */
     TrGenerator NewRGen(void) const;

    /**
     * Assignment operator (uses Assign)
     *
     * Note: you must reimplement this operator in derived 
     * classes in order to handle internal pointers correctly
     *
     * @param rOtherGen
     *   Other generator
     */
     TrGenerator& operator= (const TrGenerator& rOtherGen);
  
    /**
     * Assignment method
     *
     * Note: you must reimplement this method in derived 
     * classes in order to handle internal pointers correctly
     *
     * @param rSource
     *   Other generator
     */
     virtual TrGenerator& Assign(const Type& rSource);
   
    /**
     * Set Rabin acceptance Condition
     *
     * NOT IMPLEMENTED
     *
     * @param rOtherSet
     *   set to get priorities from
     *
     */
    void RabinAcceptance(const RabinAcepptance& rOther);

    /**
     * Get Rabin acceotance condition
     *
     * NOT IMPLEMENTED
     *
     */
    RabinAcceptance RabinAcceptance(void) const;




}; // end class TpGenerator

    
/** 
 * Convenience typedef for std prioritised generator 
 */
typedef TrGenerator<AttributeVoid, AttributeVoid, AttributCFlags, RabinAcceptance> RabinAutomaton;



/*
***************************************************************************
***************************************************************************
***************************************************************************

Implementation pGenerator

***************************************************************************
***************************************************************************
***************************************************************************
*/

/* convenience access to relevant scopes */
#define THIS TrGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr>
#define BASE TcGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr>
#define TEMP template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>


// TrGenerator(void)
TEMP THIS::TrGenerator(void) : BASE() {
  FD_DG("TrGenerator(" << this << ")::TrGenerator()");
}

// TrGenerator(rOtherGen)
TEMP THIS::TrGenerator(const TrGenerator& rOtherGen) : BASE(rOtherGen) {
  FD_DG("TrGenerator(" << this << ")::TrGenerator(rOtherGen)");
}

// TrGenerator(rOtherGen)
TEMP THIS::TrGenerator(const vGenerator& rOtherGen) : BASE(rOtherGen) {
  FD_DG("TrGenerator(" << this << ")::TrGenerator(rOtherGen)");
}

// TrGenerator(rFilename)
TEMP THIS::TrGenerator(const std::string& rFileName) : BASE(rFileName) {
  FD_DG("TrGenerator(" << this << ")::TrGenerator(rFilename) : done");
}

// operator=
TEMP THIS& THIS::operator= (const TrGenerator& rOtherGen) {
  FD_DG("TrGenerator(" << this << ")::operator = [v]" << &rOtherGen);
  return Assign(rOtherGen);
}

// copy from other faudes type
TEMP THIS& THIS::Assign(const Type& rSrc) {
  FD_DG("TrGenerator(" << this << ")::Assign([type] " << &rSrc << ")");
  // bail out on match
  if(&rSrc==static_cast<const Type*>(this)) return *this;
  // pass on to base
  FD_DG("TrGenerator(" << this << ")::Assign([type] " << &rSrc << "): call base");
  BASE::Assign(rSrc);  
  return *this;
}

// New
TEMP THIS* THIS::New(void) const {
  // allocate
  THIS* res = new THIS;
  // fix base data
  res->EventSymbolTablep(BASE::mpEventSymbolTable);
  res->mStateNamesEnabled=BASE::mStateNamesEnabled;
  res->mReindexOnWrite=BASE::mReindexOnWrite;  
  return res;
}

// Copy
TEMP THIS* THIS::Copy(void) const {
  // allocate
  THIS* res = new THIS(*this);
  // done
  return res;
}

// NewPGen
TEMP THIS THIS::NewPGen(void) const {
  // call base (fixes by assignment constructor)
  THIS res= BASE::NewCGen();
  return res;
}


// CAST
//TEMP const Type* THIS::Cast(const Type* pOther) const {
//  return dynamic_cast< const THIS* > (pOther);
//}

  
#undef TEMP
#undef BASE
#undef THIS



} // namespace faudes
#endif // _H
