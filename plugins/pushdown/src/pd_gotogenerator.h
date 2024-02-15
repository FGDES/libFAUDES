/** @file pd_gotogenerator.h  LR machine */

/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/


#ifndef FAUDES_PD_GOTOGENERATOR_H
#define FAUDES_PD_GOTOGENERATOR_H

#include "corefaudes.h"
#include "pd_gotoattributes.h"
#include "pd_parser.h"


namespace faudes {

/**
 * Simple generator for parsing with goto tables
* 
*
* 
*
* @ingroup PushdownPlugin
* 
* @section Overview
*  Overview
* 
* @section Contents
*  Contents
*/
class FAUDES_API GotoMachine{
 
private:
  
  LrmTransitionMap mTransitions;
  Lr1ConfigurationSetSet mStates;
  std::set<Lr1Configuration> mInitState;
  
public:
  
  /**
   * Setter for transitions
   * 
   * @param transitions
   *    the transitions to set
   */
  void Transitions(const LrmTransitionMap& transitions){ mTransitions = transitions;}
  
  /**
   * Getter for transitions
   * 
   * @return
   *    the transitions
   */
  const LrmTransitionMap& Transitions() const { return mTransitions; }
  
  /**
   * Setter for config sets
   * 
   * @param states
   *    the transitions to set
   */
  void States(const Lr1ConfigurationSetSet& states){ mStates = states; }
  
  /**
   * Getter for states
   * 
   * @return
   *    the states
   */
  const Lr1ConfigurationSetSet& States() const { return mStates; }
  
  /**
   * Setter for initState
   * 
   * @param initState
   *    the initStates to set
   */
  void InitState(const std::set<Lr1Configuration>& initState){ mInitState = initState; }
  
  /**
   * Getter for initState
   * 
   * @return
   *    the initState
   */
  const std::set<Lr1Configuration>& InitState() const { return mInitState; }
  
};
//end class GotoMachine

/**
 * Generator for parsing with goto tables
 *
 * 
 *
 * @ingroup PushdownPlugin
 */

template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
class FAUDES_API TgotoGenerator : public TaGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr> {  
    
public:
    /**
     * Constructor
     */
    TgotoGenerator(void);
    
    /** 
     * Copy constructor 
     *
     * @param rOtherGen
     */
    TgotoGenerator(const TgotoGenerator& rOtherGen);
    
    /** 
     * Copy constructor (no attributes)
     *
     * @param rOtherGen
     */
    TgotoGenerator(const vGenerator& rOtherGen);
    
    /**
     * Assignment operator (uses copy)
     *
     *  Note: you must reimplement this operator in derived 
     *  classes in order to handle internal pointers correctly
     *
     * @param rOtherGen
     *   Other generator
     */
    virtual TgotoGenerator& operator= (const TgotoGenerator& rOtherGen) {this->Assign(rOtherGen); return *this;};

    /* Assignment method
     *
     * Note: you must reimplement this method in derived 
     * classes in order to handle internal pointers correctly
     *
     * @param rOtherGen
     *   Other generator
     */
     virtual TgotoGenerator& Assign(const Type& rSource);
  
    
    /**
     * Construct from file
     *
     * @param rFileName
     *   Name of file
     *
     * @exception Exception
     *   - file format errors (id 1, 50, 51, 52)
     */
    TgotoGenerator(const std::string& rFileName);
    
    /**
     * Construct on heap. 
     * Constructs a TgotoGenerator on heap.
     *
     * @return 
     *   new Generator 
     */
    TgotoGenerator* New(void) const;
    
    /**
     * Construct copy on heap. 
     * Constructs a TgotoGenerator on heap.
     *
     * @return 
     *   new Generator 
     */
    TgotoGenerator* Copy(void) const;
    
    /**
     * Type test.
     * Uses C++ dynamic cast to test whether the specified object
     * casts to a TgotoGenerator.
     *
     * @return 
     *   TgotoGenerator reference if dynamic cast succeeds, else NULL 
     */
    virtual const Type* Cast(const Type* pOther) const {
        return dynamic_cast< const TgotoGenerator* > (pOther);
    };
    
    
    /**
     * Construct on stack.
     * Constructs a TgotoGenerator on stack.
     *
     * @return 
     *   new Generator 
     */
    TgotoGenerator NewGotoGen(void) const;
    
    /** 
     * Add a transition to generator by indices. States
     * must already exist!
     *
     * @param x1 
     *   Predecessor state index 
     * @param symbol 
     *   grammar symbol used for the transition 
     * @param x2
     *   Successor state index
     *
     * @return
     *   True, if the transition was new the generator
     *
     * @exception Exception
     *   - state or event not in generator (id 95)
     */
    bool SetTransition(Idx x1, const GrammarSymbolPtr& symbol, Idx x2);
    
    /**
     * Getter for the symbol of a transitions
     * 
     * @param rTrans
     *      the transition
     * @return
     *      the symbol
     */
    GrammarSymbolPtr Symbol(const Transition& rTrans) const;
    
    /**
     * Add a transition to generator
     * 
     * @param rTrans
     *      the transition
     * @param symbol
     *      grammar symbol used for the transitions
     * 
     * @return
     *      True, if the transition was new in the generator
     * 
     * @exception Exception
     *   - state or event not in generator (id 95)
     */
    bool SetTransition(const Transition& rTrans, const GrammarSymbolPtr& symbol);
    
    /**
      * Setter for the configuration set of a state
      * 
      * @param index
      *      the index of the state
      * @param configs
      *     the configuration set
      * 
      */
     void ConfigSet(Idx index, const std::set<Lr1Configuration>& configs);
    
     /**
      * Getter for the configuration set of a state
      * 
      * @param index
      *      the index of the state
      * @return
      *     the configuration set of the state
      */
     std::set<Lr1Configuration> const& ConfigSet(Idx index) const;
     
     /**
      * Get first state with the specified configuration set as attribute
      * 
      * @param configs
      *     the configuration set
      * @return
      *     state index if the state was found, else 0
      */
     Idx StateIndex(const std::set<Lr1Configuration>& configs) const;
    
    /** 
     * Check if generator is valid 
     *
     * @return 
     *   Success
     */
    virtual bool Valid(void) const;
    
}; //end class TgotoGenerator

/** Convenience typedef for std GotoGenerator */
typedef TgotoGenerator<AttributeVoid, AttributeGotoState, AttributeVoid, AttributeGotoTransition>
GotoGenerator;


// convenient scope macros  
#define THISGOTO TgotoGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr>
#define BASEGOTO TaGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr>
#define TEMPGOTO template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>

// TgotoGenerator(void)
TEMPGOTO THISGOTO::TgotoGenerator(void) : BASEGOTO() {
  // set basic members (cosmetic)
  FD_DG("GotoGenerator(" << this << ")::GotoGenerator()");
}

// TgotoGenerator(rOtherGen)
TEMPGOTO THISGOTO::TgotoGenerator(const TgotoGenerator& rOtherGen) : BASEGOTO(rOtherGen) {
  FD_DG("GotoGenerator(" << this << ")::GotoGenerator(rOtherGotoGen)");
}

// TgotoGenerator(rOtherGen)
TEMPGOTO THISGOTO::TgotoGenerator(const vGenerator& rOtherGen) : BASEGOTO(rOtherGen) {
  // set basic members (cosmetic)
  FD_DG("GotoGenerator(" << this << ")::GotoGenerator(rOtherGen)");
}

// TgotoGenerator(rFilename)
TEMPGOTO THISGOTO::TgotoGenerator(const std::string& rFileName) : BASEGOTO(rFileName) {
  FD_DG("GotoGenerator(" << this << ")::GotoGenerator(file)");
}


// copy from other faudes type
TEMPGOTO THISGOTO& THISGOTO::Assign(const Type& rSrc) {
  FD_DG("TgotoGenerator(" << this << ")::Assign([type] " << &rSrc << ")");
  // bail out on match
  if(&rSrc==static_cast<const Type*>(this)) return *this;
  // pass on to base
  BASEGOTO::Assign(rSrc);  
  return *this;
}

// New
TEMPGOTO THISGOTO* THISGOTO::New(void) const {
  // allocate
  THISGOTO* res = new THISGOTO;
  // fix base data
  res->EventSymbolTablep(BASEGOTO::mpEventSymbolTable);
  res->mStateNamesEnabled=BASEGOTO::mStateNamesEnabled;
  res->mReindexOnWrite=BASEGOTO::mReindexOnWrite;  
  // fix my data
  return res;
}

// Copy
TEMPGOTO THISGOTO* THISGOTO::Copy(void) const {
  // allocate
  THISGOTO* res = new THISGOTO(*this);
  // done
  return res;
}

// NewTGen
TEMPGOTO THISGOTO THISGOTO::NewGotoGen(void) const {
  // call base (fixes by assignment constructor)
  THISGOTO res= BASEGOTO::NewAGen();
  // fix my data
  return res;
}
//SetTransition(rTrans, symbol)
TEMPGOTO bool THISGOTO::SetTransition(const Transition& rTrans, const GrammarSymbolPtr& symbol){  
  TransAttr attr;
  attr.Symbol(symbol);
  return BASEGOTO::SetTransition(rTrans,attr);
}

// SetTransition(x1, symbol, x2)
TEMPGOTO bool THISGOTO::SetTransition(Idx x1, const GrammarSymbolPtr& symbol, Idx x2) {
  //events dont matter, so just insert a dummy event
  Idx ev = BASEGOTO::InsEvent("dummy");
  return SetTransition(Transition(x1,ev,x2), symbol);
}
//Symbol(rTrans)
TEMPGOTO GrammarSymbolPtr THISGOTO::Symbol(const Transition& rTrans) const{
  return BASEGOTO::pTransRel->Attribute(rTrans).Symbol();
}

//ConfigSet(index, configs)
TEMPGOTO void THISGOTO::ConfigSet(Idx index, const std::set<Lr1Configuration>& configs){
  BASEGOTO::pStates->Attributep(index)->ConfigSet(configs);
}

//ConfigSet(index)
TEMPGOTO std::set<Lr1Configuration> const& THISGOTO::ConfigSet(Idx index) const{
  return BASEGOTO::pStates->Attributep(index)->ConfigSet();
}

//StateIdx(configs)
TEMPGOTO Idx THISGOTO::StateIndex(const std::set<Lr1Configuration>& configs) const{
  
  StateSet::Iterator stateit;
  //iterate over all states
  for(stateit = BASEGOTO::StatesBegin(); stateit != BASEGOTO::StatesEnd(); stateit++){
    
    //look for the first state with the correct config set
    std::set<Lr1Configuration> curretConfigs = BASEGOTO::pStates->Attributep(*stateit)->ConfigSet();
    
    //found the set
    if(!(CompareConfigSet(configs, curretConfigs) || CompareConfigSet(curretConfigs, configs))){
      return *stateit;
    }
  }
  return 0;
}


// Valid() TODO checks?
TEMPGOTO bool THISGOTO::Valid(void) const {
    FD_DV("GotoGenerator(" << this << ")::Valid()");
    //call base
    if(!BASEGOTO::Valid()) return false;
    // check my names
   
    // check my clockset
    
    // check all clocksymboltables
    
    return true;
}
  
  
} // namespace faudes


#endif

