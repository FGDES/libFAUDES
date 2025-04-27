/** @file pev_pgenerator.h Classes pGenerator */


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

#ifndef FAUDES_PEV_PGENERATOR_H
#define FAUDES_PEV_PGENERATOR_H

#include "corefaudes.h"
#include "pev_priorities.h"

namespace faudes {


/*!
 * \brief The AttributePGenGl class
 * Class wraping various global attributes of a FPGen including
 * Fairness and lowest event priority.
 * Note: this is not yet a propper faudes Attribute, it misses out on
 * serialisation.
 */

class FAUDES_API AttributePGenGl : public AttributeVoid {

FAUDES_TYPE_DECLARATION(Void,AttributePGenGl,AttributeVoid)

public:
    /**
    * Default constructor
    */
    AttributePGenGl(void) : AttributeVoid() {}

    /** Destructor */
    virtual ~AttributePGenGl(void) {}

    /** Access members */
    void LowestPriority(const Idx rPriority) { mPLowest = rPriority; }
    Idx LowestPriority(void) const { return mPLowest;  }
    void Fairness(const FairnessConstraints& rFair) { mFairConsts = rFair; }
    const FairnessConstraints& Fairness(void) const { return mFairConsts; }

    /**
    * Clear (mandatory for serialisation)
    */
    void Clear(void) { mPLowest = 0; mFairConsts.Clear();}

protected:
    /**
    * Assignment method.
    *
    * @param rSrcAttr
    *    Source to assign from
    */
    void DoAssign(const AttributePGenGl& rSrcAttr){mPLowest = rSrcAttr.mPLowest; mFairConsts = rSrcAttr.mFairConsts;}

    /**
    * Test equality of configuration data.
    *
    * @param rOther
    *    Other attribute to compare with.
    * @return
    *   True on match.
    */
    bool DoEqual(const AttributePGenGl& rOther) const {return (mPLowest == rOther.mPLowest && mFairConsts == rOther.mFairConsts);}

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
    virtual void DoRead(TokenReader& rTr, const std::string& rLabel="", const Type* pContext=nullptr){}

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
    virtual void DoWrite(TokenWriter& rTw,const std::string& rLabel="", const Type* pContext=nullptr) const {}

    /*!
     * \brief mFairConst
     * fairness constraints
     */
    FairnessConstraints mFairConsts;

    /**
    * lowest priority value of globally all events (not only my alphabet).
    * 0 is highest.
    */
    Idx mPLowest = 0;
};


/**
 * Generator with priositised events. 
 * 
 * @section PGeneratorOverview Overview
 * 
 * The TpGenerator is a variant of the TcGenerator to add an interface for priositised events and fairness
 *
 * Technically, the construct is based on the specialized attribute class faudes::AttributePriority
 * derived from faudes::AttributeCFlags. The TpGenerator expects an event attribute template parameter
 * with the minimum interface defined in AttribuePriority and AttributeCFlags..
 * For convenience, the configuration with the minimum attributes is been typedef-ed as PriositisedSystem.
 *
 * @ingroup GeneratorClasses
 */

template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
    class FAUDES_TAPI TpGenerator : public TcGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr> {    
  public:


    /**
     * Creates an emtpy PrioritisedSystem object 
     */
    TpGenerator(void);

    /** 
     * PrioritisedSystem from a std Generator. Copy constructor 
     *
     * @param rOtherGen
     */
    TpGenerator(const vGenerator& rOtherGen);
        
    /** 
     * PriositisedSystem from a PriositisedSystem. Copy constructor 
     *
     * @param rOtherGen
     */
    TpGenerator(const TpGenerator& rOtherGen);

    /**
     * Construct from file
     *
     * @param rFileName
     *   Filename
     *
     * @exception Exception
     *   If opening/reading fails an Exception object is thrown (id 1, 50, 51)
     */
    TpGenerator(const std::string& rFileName);

    /**
     * Construct on heap
     *
     * @return 
     *   new Generator 
     */
     TpGenerator* New(void) const;

    /**
     * Construct copy on heap
     *
     * @return 
     *   new Generator 
     */
     TpGenerator* Copy(void) const;

    /**
     * Type test.
     * Uses C++ dynamic cast to test whether the specified object
     * casts to a Priositised System.
     *
     * @param pOther
     *   poinetr to object to test
     *
     * @return 
     *   TpGenerator reference if dynamic cast succeeds, else NULL 
     */
     virtual const Type* Cast(const Type* pOther) const {
       return dynamic_cast< const TpGenerator* > (pOther); };


    /**
     * Construct on stack
     *
     * @return 
     *   new Generator 
     */
     TpGenerator NewPGen(void) const;

    /**
     * Assignment operator (uses Assign)
     *
     * Note: you must reimplement this operator in derived 
     * classes in order to handle internal pointers correctly
     *
     * @param rOtherGen
     *   Other generator
     */
     /*virtual*/ TpGenerator& operator= (const TpGenerator& rOtherGen);
  
    /**
     * Assignment method
     *
     * Note: you must reimplement this method in derived 
     * classes in order to handle internal pointers correctly
     *
     * @param rSource
     *   Other generator
     */
     virtual TpGenerator& Assign(const Type& rSource);
   
    /**
     * Get priority by event index
     *
     * @param index
     *   Event index
     * @return
     *   Priority of specified event
     */
    Idx Priority(const Idx index) const;

    /**
     * Get priority by event name
     *
     * @param rName
     *   Event name
     * @return
     *   Priority of specified event
     */
    Idx Priority(const std::string& rName) const;

    /**
     * Set priority by event index
     *
     * @param index
     *   Specify event
     * @param prio
     *   Specify priority
     */
    void Priority(const Idx index, const Idx prio);
    
    /**
     * Set priority by event name
     *
     * @param rName
     *   Specify event
     * @param prio
     *   Specify priority
     */
    void Priority(const std::string& rName, const Idx prio);
    
    /**
     * Set Priorities from other prioritised event set
     *
     * @param rOtherSet
     *   set to get priorities from
     *
     */
    void Priorities(const TpEventSet<EventAttr>& rOtherSet);

    /**
     * Get Priorities 
     *
     *
     */
     EventPriorities Priorities(void) const;

    /**
     * Get lowest  priority
     * Note: this is a dumb member -- you need to set it programatically
     *
     * @return
     *  lowest priority
     *
     */
    Idx LowestPriority(void) const;

    /**
     * Set lowest priority
     * Note: this is a dumb member -- you need to set it programatically
     *
     * @param
     *  lowest priority
     *
     */
    void LowestPriority(Idx prio);

    /**
     * Get fairness constraints
     *
     * @return
     *  vector of fainess eventsets
     *
     */
    FairnessConstraints Fairness(void) const;

    /**
     * Set fairness constraints
     *
     * @param
     *  fairness constraints
     *
     */
    void Fairness(const FairnessConstraints& rFair);



}; // end class TpGenerator

    
/** 
 * Convenience typedef for std prioritised generator 
 */
typedef TpGenerator<AttributePGenGl, AttributeVoid, AttributePriority, AttributeVoid> PrioritisedGenerator;


// Tyoedef for compatibility with YT's original code / internal use
typedef TpGenerator<AttributePGenGl, AttributeVoid, AttributePriority, AttributeVoid> pGenerator;
  
/** 
 * Convenience typedef for vectors of priositised systems
 * \ingroup GeneratorClasses
 */
 //typedef  TBaseVector<PriositisedSystem> PrioritisedSystemVector;  




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
#define THIS TpGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr>
#define BASE TcGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr>
#define TEMP template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>


// TpGenerator(void)
TEMP THIS::TpGenerator(void) : BASE() {
  FD_DG("TpGenerator(" << this << ")::TpGenerator()");
}

// TpGenerator(rOtherGen)
TEMP THIS::TpGenerator(const TpGenerator& rOtherGen) : BASE(rOtherGen) {
  FD_DG("TpGenerator(" << this << ")::TpGenerator(rOtherGen)");
}

// TpGenerator(rOtherGen)
TEMP THIS::TpGenerator(const vGenerator& rOtherGen) : BASE(rOtherGen) {
  FD_DG("TpGenerator(" << this << ")::TpGenerator(rOtherGen)");
}

// TpGenerator(rFilename)
TEMP THIS::TpGenerator(const std::string& rFileName) : BASE(rFileName) {
  FD_DG("TpGenerator(" << this << ")::TpGenerator(rFilename) : done");
}

// operator=
TEMP THIS& THIS::operator= (const TpGenerator& rOtherGen) {
  FD_DG("TpGenerator(" << this << ")::operator = [v]" << &rOtherGen);
  return Assign(rOtherGen);
}

// copy from other faudes type
TEMP THIS& THIS::Assign(const Type& rSrc) {
  FD_DG("TpGenerator(" << this << ")::Assign([type] " << &rSrc << ")");
  // bail out on match
  if(&rSrc==static_cast<const Type*>(this)) return *this;
  // pass on to base
  FD_DG("TpGenerator(" << this << ")::Assign([type] " << &rSrc << "): call base");
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


// Priority(index)
TEMP Idx THIS::Priority(const Idx index) const {
 return this->EventAttribute(index).Priority();
} 
	
// Priority(name)
TEMP Idx THIS::Priority(const std::string& rName) const {
 return this->EventAttribute(rName).Priority();
} 
	
// Priority(index,prio)
TEMP void THIS::Priority(const Idx index, const Idx prio) {
 this->EventAttributep(index)->Priority(prio);
} 
	
// Priority(name,prio)
TEMP void THIS::Priority(const std::string& rName, Idx prio) {
  this->EventAttributep(rName)->Priority(prio);
} 
	
// Priorities(otherset)
TEMP void THIS::Priorities(const TpEventSet<EventAttr>& rOtherSet) {
  NameSet::Iterator eit=this->AlphabetBegin();  
  NameSet::Iterator eit_end=this->AlphabetEnd();
  for(;eit!=eit_end;++eit) {
    if(rOtherSet.Exists(*eit))
      Priority(*eit,rOtherSet.Priority(*eit));
  }
}

// Priorities()
TEMP EventPriorities THIS::Priorities(void) const {
  EventPriorities res;
  NameSet::Iterator eit=this->AlphabetBegin();  
  NameSet::Iterator eit_end=this->AlphabetEnd();
  for(;eit!=eit_end;++eit) {
    res.InsPriority(*eit,this->Priority(*eit));
  }
  return res;    
}
// LowestPriority
TEMP Idx THIS::LowestPriority(void) const {
  return this->GlobalAttribute().LowestPriosity();
}

// LowestPriority
TEMP void THIS::LowestPriority(Idx prio) {
  this->GlobalAttribute().LowestPriosity(prio);
}

// Fairness
TEMP FairnessConstraints THIS::Fairness(void) const {
  return this->GlobalAttribute().Fairness();
}

// Fairness
TEMP void THIS::Fairness(const FairnessConstraints& rFair) {
  this->GlobalAttributep()->Fairness(rFair);
}
  


  
#undef TEMP
#undef BASE
#undef THIS



} // namespace faudes
#endif // PEV_GENERATOR_H
