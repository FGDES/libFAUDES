/** @file hyb_hgenerator.h  Linear hybrid automata class TlhaGenerator */

/* 
   Hybrid systems plug-in for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010  Thomas Moor

*/



#ifndef FAUDES_HYB_HGENERATOR_H
#define FAUDES_HYB_HGENERATOR_H


#include "corefaudes.h"
#include "hyb_attributes.h"

//#undef FD_DG
//#define FD_DG(message) FAUDES_WRITE_CONSOLE("FAUDES_GENERATOR: " << message)


namespace faudes {



/**
 * Generator with linear hybrid automata extensions. 
 *
 * \section SecHybridHenzinger Henzinger's Linear Hybrid Automata 
 *
 * The TlhaGenerator implements a linear hybrid automaton as 
 * introduced by Henzinger et al. Here the state space consists of a discrete and a continuous
 * component. The discrete component is also referre to as a location. While within a location, 
 * the continuous state evolves according to a differential inclusion. 
 * Furthermore, each location and each discrete transition
 * is equipped with a so called invariant and a guard, respectively. While in a particular location,
 * the continuous state must satisfy the invariant. Similarly, a transition becomes enabled only
 * when the respective guard is satisfied. When a transition is executed, the continous state is
 * reset according to a resetrelation. For the linear case implemented in libFAUDES, the assume that
 * - the right hand side of the differential inclusion is constant and represented by a closed convex polyhedron;
 * - invariants and guards are represented by closed convex polyhedra,
 * - reset relations are affine transformations with an convex polyhedral offset.
 *
 *
 *
 * \section SecHybridImplement Implementation
 *
 * The TlhaGenerator is derived from the cGenerator and requires 
 * adequate attribute parameters that implement invariants, guards, resets and rates. Suitable
 * attribute classes are provided by AttributeLhaState, AttributeLhaTrans and AttributeLhaGlobal
 * which may be used either directly or as base classes for further derivatives.
 * For the event attribute, the lhaGenerator assumes the AttributeCFlags interface. A convenience
 * definition faudes::lhaGenerator is used for a minimal version with the above mentioned attribute parameters.
 *
 *
 * \section tGeneratorFileFormat File IO
 *
 * The TlhaGenerator calsses use the TaGenerator file IO, i.e. the file format is the 
 * same up to hybrid automata related  requirements from the attribute parameters. The below
 * example is from the basic version lhaGenerator.
 * @code
<Generator name="lha example" ftype="LinearHybridAutomaton">

<Alphabet>
<Event name="north"/>
<Event name="south"/>
<Event name="west"/>
</Alphabet>

<StateSet>

<State id="1" name="NorthWest">
<Initial/>
<Rate>
<AMatrix rows="4" columns="2">
 100             0             
-100             0             
 0             100             
 0            -100             
</AMatrix>
<BVector count="4">
-25         50         50        -25             
</BVector>
</Rate>
<Invariant>
<AMatrix rows="4" columns="2">
 1             0             
-1             0             
 0             1            
 0            -1             
</AMatrix>
<BVector count="4">
100            0              10             0             
</BVector>
</Invariant>
<InitialConstraint>
<AMatrix rows="1" columns="2">
 -1             0             
</AMatrix>
<BVector count="1">
-80
</BVector>
</InitialConstraint>
</State>

<State id="2" name="SouthWest">
<Rate>
<AMatrix rows="4" columns="2">
 100             0             
-100             0             
 0             100             
 0            -100             
</AMatrix>
<BVector count="4">
-25         50         -25        50             
</BVector>
</Rate>
<Invariant>
<AMatrix rows="4" columns="2">
 1             0             
-1             0             
 0             1             
 0            -1             
</AMatrix>
<BVector count="4">
100             0              10            0             
</BVector>
</Invariant>
</State>

<State id="10" name="Target">
<Rate>
<AMatrix rows="0" columns="2"/>
<BVector count="0"/>
</Rate>
<Invariant>
<AMatrix rows="4" columns="2">
 1             0             
-1             0             
 0             1             
 0            -1             
</AMatrix>
<BVector count="4">
100             0              10            0             
</BVector>
</Invariant>
</State>

</StateSet>

<TransitionRelation>

<Transition x1="1" event="north" x2="2">
<Guard>
<AMatrix rows="1" columns="2">
0              -1            
</AMatrix>
<BVector count="1">
-9             
</BVector>
</Guard>
<Reset>
<A1Matrix rows="4" columns="2">
-1         0            
 1         0            
 0        -1            
 0         1
</A1Matrix>
<A2Matrix rows="4" columns="2">
 1         0            
-1         0            
 0         1            
 0        -1
</A2Matrix>
<BVector count="4">
-1      1     1     1             
</BVector>
</Reset>
</Transition>

<Transition x1="1" event="west" x2="10">
<Guard>
<AMatrix rows="1" columns="2">
1              0            
</AMatrix>
<BVector count="1">
1             
</BVector>
</Guard>
</Transition>

<Transition x1="2" event="south" x2="1">
<Guard>
<AMatrix rows="1" columns="2">
0              1            
</AMatrix>
<BVector count="1">
1             
</BVector>
</Guard>
</Transition>

<Transition x1="2" event="west" x2="10">
<Guard>
<AMatrix rows="1" columns="2">
1              0            
</AMatrix>
<BVector count="1">
1             
</BVector>
</Guard>
</Transition>

</TransitionRelation>

<LhaStateSpace>
<AMatrix rows="4" columns="2">
1              0             
-1             0             
0              1             
0              -1             
</AMatrix>
<BVector count="4">
100             0              10             0             
</BVector>
</LhaStateSpace>
</Generator>
 * @endcode
 * 
 *
 * @ingroup HybridPlugin 
 */

template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
class TlhaGenerator : public TaGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr> {    

    public:
   /**
    * Constructor
    */
    TlhaGenerator(void);

   /** 
    * Copy constructor 
    *
    * @param rOtherGen
    */
    TlhaGenerator(const TlhaGenerator& rOtherGen);

   /** 
    * Copy constructor (no attributes)
    *
    * @param rOtherGen
    */
    TlhaGenerator(const vGenerator& rOtherGen);

    /**
     * Construct a from file
     *
     * @param rFileName
     *   Filename
     *
     * @exception Exception
     *   If opening/reading fails an Exception object is thrown (id 1, 50, 51)
     */
    TlhaGenerator(const std::string& rFileName);

   /**
    * Construct on heap. 
    * Constructs a TlhaGenerator on heap with the same attribute types and the same event- and clock-symboltable.
    *
    * @return 
    *   new Generator 
    */
    TlhaGenerator* New(void) const;

   /**
    * Construct copy on heap. 
    * Constructs a TlhaGenerator on heap with the same attribute types and the same event- and clock-symboltable.
    *
    * @return 
    *   new Generator 
    */
    TlhaGenerator* Copy(void) const;


    /**
     * Type test.
     * Uses C++ dynamic cast to test whether the specified object
     * casts to a System.
     *
     * @return 
     *   TlhaGenerator reference if dynamic cast succeeds, else NULL 
     */
     virtual const Type* Cast(const Type* pOther) const {
       return dynamic_cast< const TlhaGenerator* > (pOther); };


    /**
     * Assignment operator (uses copy)
     *  Note: you must reimplement this operator in derived 
     *  classes in order to handle internal pointers correctly
     *
     * @param rOtherGen
     *   Other generator
     */
    virtual TlhaGenerator& operator= (const TlhaGenerator& rOtherGen) { this->Assign(rOtherGen); return *this;};

    /**
     * Assignment operator (uses copy)
     *
     * @param rOtherGen
     *   Other generator
     */
     virtual TlhaGenerator& operator= (const vGenerator& rOtherGen) { this->Assign(rOtherGen); return *this;};



   /**
    * Get dimension of continuous statespace
    *
    * @return 
    *    Ref to state space polyhedron
    */
    int Dimension(void) const;

   /**
    * Get continuous statespace
    *
    * @return 
    *    Ref to state space polyhedron
    */
    const Polyhedron& StateSpace(void) const;

   /**
    * Set continuous statespace
    * 
    * Note: if the new state space doe not match the dimension
    * of the current state space, all invariants, guards etc are
    * invalidated.
    *
    * @param rStateSpace
    *    New state space
    */
    void StateSpace(const Polyhedron& rStateSpace);

    /**
    * Get invariant of state by index
    *
    * @param idx
    *    State index
    * @return  
    *    Const ref to invariant
    * @exception Exception
    *   - state does not exist (id 90, 95)
    */
    const Polyhedron& Invariant(Idx idx) const;

    /**
    * Get invariant of state by index
    *
    * @param idx
    *    State index
    * @return  
    *    Pointer to invariant
    * @exception Exception
    *   - state does not exist (id 90, 95)
    */
   Polyhedron* Invariantp(Idx idx);

   /**
    * Set invariant of state by index
    *
    * @param idx
    *    State index
    * @param rInvariant  
    *    New invariant
    * @exception Exception
    *   - dimension mismatch (id 700)
    *   - state does not exist (id 90, 95)
    */
    void Invariant(Idx idx, const Polyhedron& rInvariant);

    /**
    * Get invariant of state by name
    *
    * @param name
    *    State name
    * @return
    *    Const ref to invariant
    * @exception Exception
    *   - state does not exist (id 90, 95)
    */
    const Polyhedron& Invariant(const std::string& name) const;

   /**
    * Set invariant of state by name
    *
    * @param name
    *    State name
    * @param rInvariant  
    *    New invariant
    * @exception Exception
    *   - dimension mismatch (id 700)
    *   - state does not exist (id 90, 95)
    */
    void Invariant(const std::string& name, const Polyhedron& rInvariant);

  
    /**
    * Get initial constraint of state by index
    *
    * @param idx
    *    State index
    * @return  
    *    Const ref to initial constraint
    * @exception Exception
    *   - state does not exist (id 90, 95)
    */
    const Polyhedron& InitialConstraint(Idx idx) const;

    /**
    * Get initial constraint of state by index
    *
    * @param idx
    *    State index
    * @return  
    *    Pointer to initial constraint
    * @exception Exception
    *   - state does not exist (id 90, 95)
    */
   Polyhedron* InitialConstraintp(Idx idx);

   /**
    * Set initial constraint of state by index
    *
    * @param idx
    *    State index
    * @param rInitialConstraint  
    *    New initial constraint
    * @exception Exception
    *   - dimension mismatch (id 700)
    *   - state does not exist (id 90, 95)
    */
    void InitialConstraint(Idx idx, const Polyhedron& rInitialConstraint);

    /**
    * Get initial constraint of state by name
    *
    * @param name
    *    State name
    * @return
    *    Const ref to initial constraint
    * @exception Exception
    *   - state does not exist (id 90, 95)
    */
    const Polyhedron& InitialConstraint(const std::string& name) const;

   /**
    * Set initial constraint of state by name
    *
    * @param name
    *    State name
    * @param rInitialConstraint  
    *    New initial constraint
    * @exception Exception
    *   - dimension mismatch (id 700)
    *   - state does not exist (id 90, 95)
    */
    void InitialConstraint(const std::string& name, const Polyhedron& rInitialConstraint);

  
    /**
    * Get rate of state by index
    *
    * @param idx
    *    State index
    * @return  
    *    Const ref to rate
    * @exception Exception
    *   - state does not exist (id 90, 95)
    */
    const Polyhedron& Rate(Idx idx) const;

    /**
    * Get rate of state by index
    *
    * @param idx
    *    State index
    * @return  
    *    Pointer to rate
    * @exception Exception
    *   - state does not exist (id 90, 95)
    */
   Polyhedron* Ratep(Idx idx);

   /**
    * Set rate of state by index
    *
    * @param idx
    *    State index
    * @param rRate  
    *    New rate
    * @exception Exception
    *   - dimension mismatch (id 700)
    *   - state does not exist (id 90, 95)
    */
    void Rate(Idx idx, const Polyhedron& rRate);

    /**
    * Get rate of state by name
    *
    * @param name
    *    State name
    * @return
    *    Const ref to rate
    * @exception Exception
    *   - state does not exist (id 90, 95)
    */
    const Polyhedron& Rate(const std::string& name) const;

   /**
    * Set rate of state by name
    *
    * @param name
    *    State name
    * @param rRate  
    *    New rate
    * @exception Exception
    *   - dimension mismatch (id 700)
    *   - state does not exist (id 90, 95)
    */
    void Rate(const std::string& name, const Polyhedron& rRate);

  
   /**
    * Get guard of a transition
    *
    * @param rTrans
    *    transition to manupilate
    * @return 
    *    Guard of transition.
    * @exception Exception
    *   - transition does not exist (id 60)
    */
    const Polyhedron& Guard(const Transition& rTrans) const;

   /**
    * Get guard of a transition
    *
    * @param rTrans
    *    transition to manupilate
    * @return 
    *    Pointer to guard of transition.
    * @exception Exception
    *   - transition does not exist (id 60)
    */
    Polyhedron* Guardp(const Transition& rTrans);

   /**
    * Sets guard of a transition
    *
    * @param rTrans
    *    transition to manuipulate
    * @param rGuard
    *    new Guard of transition.
    * @exception Exception
    *   - dimension mismatch (id 700)
    *   - transition does not exist (id 60)
    */
    void Guard(const Transition& rTrans, const Polyhedron& rGuard);


   /**
    * Get reset of a transition
    *
    * @param rTrans
    *    transition to manupilate
    * @return 
    *    Reset of transition.
    * @exception Exception
    *   - transition does not exist (id 60)
    */
    const LinearRelation& Reset(const Transition& rTrans) const;

   /**
    * Get reset of a transition
    *
    * @param rTrans
    *    transition to manupilate
    * @return 
    *    Pointer to reset of transition.
    * @exception Exception
    *   - transition does not exist (id 60)
    */
    LinearRelation* Resetp(const Transition& rTrans);

   /**
    * Sets reset of a transition
    *
    * @param rTrans
    *    transition to manuipulate
    * @param rReset
    *    new Reset of transition.
    * @exception Exception
    *   - dimension mismatch (id 700)
    *   - transition does not exist (id 60)
    */
    void Reset(const Transition& rTrans, const LinearRelation& rReset);


    /** 
     * Check if generator is valid. 
     *
     * The current implementation test consitency of dimensions.
     *
     * @return 
     *   Success
     */
    virtual bool Valid(void) const;


    /**
     * Updates internal attributes. As a demo, we set
     * state flag 0x20000000 for blocking states.
     * Reimplement to your needs.
     *
     * @return true if value changed
     */
    virtual bool UpdateAttributes(void);

protected:

  /**
   * Read generator object from TokenReader, see Type::Read for public wrappers.
   *
   * We reimplement this vGenerator function in oredr to fix dimensions of default
   * invariants etec.
   *
   * @param rTr
   *   TokenReader to read from
   * @param rLabel
   *   Section to read
   * @param pContext
   *   Read context to provide contextual information (ignored)
   *
   * @exception Exception
   *   - token mismatch (id 50, 51, 52, 80, 85)
   *   - IO error (id 1)
   */
  virtual void DoRead(TokenReader& rTr,  const std::string& rLabel = "", const Type* pContext=0);
 
  /**
   * Write generator object to TokenWriter, see Type::Write for public wrappers.
   *
   * We reimplement this vGenerator function in oredr opt for XML write
   *
   * @param rTe
   *   TokenWriter to write to
   * @param rLabel
   *   Section label to write
   * @param pContext
   *   Write context to provide contextual information (ignored)
   *
   * @exception Exception
   *   - IO error (id 1)
   */
  virtual void DoWrite(TokenWriter& rTw,  const std::string& rLabel = "", const Type* pContext=0) const;
 


}; // end class TlhaGeneraator

    
/** Convenience typedef for std lhaGenerator */
typedef TlhaGenerator<AttributeLhaGlobal,AttributeLhaState,AttributeCFlags,AttributeLhaTrans> LinearHybridAutomaton;
typedef TaTransSet<AttributeLhaTrans> LhaTransSet;
typedef TaIndexSet<AttributeLhaState> LhaStateSet;



// convenient scope macors  
#define THIS TlhaGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr>
#define BASE TaGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr>
#define TEMP template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>


// TlhaGenerator(void)
TEMP THIS::TlhaGenerator(void) : BASE() {
  FD_DG("lhaGenerator(" << this << ")::lhaGenerator()");
}

// TlhaGenerator(rOtherGen)
TEMP THIS::TlhaGenerator(const TlhaGenerator& rOtherGen) : BASE(rOtherGen) {
  FD_DG("lhaGenerator(" << this << ")::lhaGenerator(rOtherGen)");
}

// TlhaGenerator(rOtherGen)
TEMP THIS::TlhaGenerator(const vGenerator& rOtherGen) : BASE(rOtherGen) {
  FD_DG("lhaGenerator(" << this << ")::lhaGenerator(rOtherGen) with csymtab");
}

// TlhaGenerator(rFilename)
TEMP THIS::TlhaGenerator(const std::string& rFileName) : BASE(rFileName) {
  FD_DG("lhaGenerator(" << this << ")::lhaGenerator(" << rFileName << ")");
}


// New
TEMP THIS* THIS::New(void) const {
  // allocate
  THIS* res = new THIS;
  // fix base data
  res->EventSymbolTablep(BASE::mpEventSymbolTable);
  res->mStateNamesEnabled=BASE::mStateNamesEnabled;
  // done
  return res;
}

// Copy
TEMP THIS* THIS::Copy(void) const {
  // allocate
  THIS* res = new THIS(*this);
  // done
  return res;
}

// Dimension() const
TEMP int THIS::Dimension(void) const {
  return BASE::pGlobalAttribute->mStateSpace.Dimension();
}

// StateSpace()
TEMP const Polyhedron& THIS::StateSpace(void) const {
  return BASE::pGlobalAttribute->mStateSpace;
}


// StateSpace()
TEMP void THIS::StateSpace(const Polyhedron& states) {
  /*
  if(Dimension()!=states.Dimension()) {
    // warning? clear all?
  }
  */
  BASE::pGlobalAttribute->mStateSpace=states; 
}

// Invariant(index)
TEMP const Polyhedron& THIS::Invariant(Idx stateindex) const {
  if(!BASE::ExistsState(stateindex)) {
      std::stringstream errstr;
      errstr << "state index " << stateindex << " not found in StateSet" << std::endl;
      throw Exception("lhaGenerator::Invariant", errstr.str(), 90);
  }
  return BASE::pStates->Attribute(stateindex).mInvariant;
}

// Invariantp(index)
TEMP Polyhedron* THIS::Invariantp(Idx stateindex) {
  if(!BASE::ExistsState(stateindex)) {
      std::stringstream errstr;
      errstr << "state index " << stateindex << " not found in StateSet" << std::endl;
      throw Exception("lhaGenerator::Invariant", errstr.str(), 90);
  }
  return &BASE::pStates->Attributep(stateindex)->mInvariant;
}

// Invariant(index, new invariant)
TEMP void THIS::Invariant(Idx stateindex, const Polyhedron& rInvariant) {
  if(!BASE::ExistsState(stateindex)) {
      std::stringstream errstr;
      errstr << "state index " << stateindex << " not found in StateSet" << std::endl;
      throw Exception("lhaGenerator::Invariant", errstr.str(), 90);
  }
  if(Dimension()!=rInvariant.Dimension()) {
      std::stringstream errstr;
      errstr << "dimension mismatch for state index " << stateindex;
      throw Exception("lhaGenerator::Invariant", errstr.str(), 700);
  }
  BASE::pStates->Attributep(stateindex)->mInvariant=rInvariant;
}

// Invariant(name)
TEMP const Polyhedron& THIS::Invariant(const std::string&  statename) const {
  Idx idx=BASE::StateIndex(statename);
  return Invariant(idx);
}

// Invariant(name, new inv)
TEMP void THIS::Invariant(const std::string&  statename,const Polyhedron& rInvariant)  {
  Idx idx=BASE::StateIndex(statename);
  Invariant(idx,rInvariant);
}



// InitialConstraint(index)
TEMP const Polyhedron& THIS::InitialConstraint(Idx stateindex) const {
  if(!BASE::ExistsState(stateindex)) {
      std::stringstream errstr;
      errstr << "state index " << stateindex << " not found in StateSet" << std::endl;
      throw Exception("lhaGenerator::InitialConstraint", errstr.str(), 90);
  }
  const Polyhedron& iconstraint = BASE::pStates->Attributep(stateindex)->mInitialConstraint;
  if(iconstraint.Dimension()==0) const_cast< Polyhedron& >(iconstraint).Dimension(Dimension());
  return iconstraint;
}

// InitialConstraintp(index)
TEMP Polyhedron* THIS::InitialConstraintp(Idx stateindex) {
  if(!BASE::ExistsState(stateindex)) {
      std::stringstream errstr;
      errstr << "state index " << stateindex << " not found in StateSet" << std::endl;
      throw Exception("lhaGenerator::InitialConstraint", errstr.str(), 90);
  }
  Polyhedron& iconstraint = BASE::pStates->Attributep(stateindex)->mInitialConstraint;
  if(iconstraint.Dimension()==0) iconstraint.Dimension(Dimension());
  return &iconstraint;
}

// InitialConstraint(index, new constraint)
TEMP void THIS::InitialConstraint(Idx stateindex, const Polyhedron& rInitialConstraint) {
  if(!BASE::ExistsState(stateindex)) {
      std::stringstream errstr;
      errstr << "state index " << stateindex << " not found in StateSet" << std::endl;
      throw Exception("lhaGenerator::InitialConstraint", errstr.str(), 90);
  }
  if(Dimension()!=rInitialConstraint.Dimension()) {
      std::stringstream errstr;
      errstr << "dimension mismatch for state index " << stateindex;
      throw Exception("lhaGenerator::InitialConstraint", errstr.str(), 700);
  }
  BASE::pStates->Attributep(stateindex)->mInitialConstraint=rInitialConstraint;
}

// InitialConstraint(name)
TEMP const Polyhedron& THIS::InitialConstraint(const std::string&  statename) const {
  Idx idx=BASE::StateIndex(statename);
  return InitialConstraint(idx);
}

// InitialConstraint(name, new constraint)
TEMP void THIS::InitialConstraint(const std::string&  statename,const Polyhedron& rInitialConstraint)  {
  Idx idx=BASE::StateIndex(statename);
  InitialConstraint(idx,rInitialConstraint);
}

// Rate(index)
TEMP const Polyhedron& THIS::Rate(Idx stateindex) const {
  if(!BASE::ExistsState(stateindex)) {
      std::stringstream errstr;
      errstr << "state index " << stateindex << " not found in StateSet" << std::endl;
      throw Exception("lhaGenerator::Rate", errstr.str(), 90);
  }
  return BASE::pStates->Attribute(stateindex).mRate;
}

// Ratep(index)
TEMP Polyhedron* THIS::Ratep(Idx stateindex) {
  if(!BASE::ExistsState(stateindex)) {
      std::stringstream errstr;
      errstr << "state index " << stateindex << " not found in StateSet" << std::endl;
      throw Exception("lhaGenerator::Rate", errstr.str(), 90);
  }
  return &BASE::pStates->Attributep(stateindex)->mRate;
}

// Rate(index, new invariant)
TEMP void THIS::Rate(Idx stateindex, const Polyhedron& rRate) {
  if(!BASE::ExistsState(stateindex)) {
      std::stringstream errstr;
      errstr << "state index " << stateindex << " not found in StateSet" << std::endl;
      throw Exception("lhaGenerator::Rate", errstr.str(), 90);
  }
  if(Dimension()!=rRate.Dimension()) {
      std::stringstream errstr;
      errstr << "dimension mismatch for state index " << stateindex;
      throw Exception("lhaGenerator::Rate", errstr.str(), 700);
  }
  BASE::pStates->Attributep(stateindex)->mRate=rRate;
}

// Rate(name)
TEMP const Polyhedron& THIS::Rate(const std::string&  statename) const {
  Idx idx=BASE::StateIndex(statename);
  return Rate(idx);
}

// Rate(name, new inv)
TEMP void THIS::Rate(const std::string&  statename,const Polyhedron& rRate)  {
  Idx idx=BASE::StateIndex(statename);
  Rate(idx,rRate);
}



// Guard(index)
TEMP const Polyhedron& THIS::Guard(const Transition& trans) const {
  if(!BASE::ExistsTransition(trans)) {
      std::stringstream errstr;
      errstr << "transition " << trans.Str() << " not found in TransRel" << std::endl;
      throw Exception("lhaGenerator::Guard", errstr.str(), 90);
  }
  const Polyhedron& guard = BASE::pTransRel->Attributep(trans)->mGuard;
  if(guard.Size()==0) const_cast< Polyhedron& >(guard).Dimension(Dimension());
  return guard;
}

// Guardp(index)
TEMP Polyhedron* THIS::Guardp(const Transition& trans) {
  if(!BASE::ExistsTransition(trans)) {
      std::stringstream errstr;
      errstr << "transition " << trans.Str() << " not found in TransRel" << std::endl;
      throw Exception("lhaGenerator::Guard", errstr.str(), 90);
  }
  Polyhedron& guard = BASE::pTransRel->Attributep(trans)->mGuard;
  if(guard.Size()==0) const_cast< Polyhedron& >(guard).Dimension(Dimension());
  return &guard;
}

// Guard(index, new invariant)
TEMP void THIS::Guard(const Transition& trans, const Polyhedron& rGuard) {
  if(!BASE::ExistsTransition(trans)) {
      std::stringstream errstr;
      errstr << "transition " << trans.Str() << " not found in TransRel" << std::endl;
      throw Exception("lhaGenerator::Guard", errstr.str(), 90);
  }
  if(Dimension()!=rGuard.Dimension()) {
      std::stringstream errstr;
      errstr << "dimension mismatch for guard at transition " << trans.Str();
      throw Exception("lhaGenerator::Guard", errstr.str(), 700);
  }
  BASE::pTransRel->Attributep(trans)->mGuard=rGuard;
}


// Reset(index)
TEMP const LinearRelation& THIS::Reset(const Transition& trans) const {
  if(!BASE::ExistsTransition(trans)) {
      std::stringstream errstr;
      errstr << "transition " << trans.Str() << " not found in TransRel" << std::endl;
      throw Exception("lhaGenerator::Reset", errstr.str(), 90);
  }
  const LinearRelation& reset = BASE::pTransRel->Attributep(trans)->mReset;
  if((reset.Identity()) && (reset.Dimension()==0)) const_cast<LinearRelation&>(reset).Identity(Dimension());
  return reset;
}

// Resetp(index)
TEMP LinearRelation* THIS::Resetp(const Transition& trans) {
  if(!BASE::ExistsTransition(trans)) {
      std::stringstream errstr;
      errstr << "transition " << trans.Str() << " not found in TransRel" << std::endl;
      throw Exception("lhaGenerator::Reset", errstr.str(), 90);
  }
  LinearRelation& reset = BASE::pTransRel->Attributep(trans)->mReset;
  if((reset.Identity()) && (reset.Dimension()==0)) reset.Identity(Dimension());
  return &reset;
}

// Reset(index, new invariant)
TEMP void THIS::Reset(const Transition& trans, const LinearRelation& rReset) {
  if(!BASE::ExistsState(trans)) {
      std::stringstream errstr;
      errstr << "transition " << trans.Str() << " not found in TransRel" << std::endl;
      throw Exception("lhaGenerator::Reset", errstr.str(), 90);
  }
  if(Dimension()!=rReset.Dimension()) {
      std::stringstream errstr;
      errstr << "dimension mismatch for guard at transition " << trans.Str();
      throw Exception("lhaGenerator::Reset", errstr.str(), 700);
  }
  BASE::pTransRel->Attributep(trans)->mReset=rReset;
}



// Valid()
TEMP bool THIS::Valid(void) const {
    FD_DG("lhaGenerator(" << this << ")::Valid()");
    //call base
    if(!BASE::Valid()) return false;
    // check my dimensions
    StateSet::Iterator sit;
    for(sit = BASE::StatesBegin(); sit!= BASE::StatesEnd(); ++sit) {
      if(Invariant(*sit).Dimension()!=Dimension()) return false;
    }
    TransSet::Iterator tit;
    for(tit = BASE::TransRelBegin(); tit!= BASE::TransRelEnd(); ++tit) {
      if(Guard(*tit).Dimension()!=Dimension()) return false;
      if(Reset(*tit).Dimension()!=Dimension()) return false;
    }
    // done
    return true;
}


// UpdateAttributes()
TEMP bool THIS::UpdateAttributes(void) {
    FD_DG("lhaGenerator(" << this << ")::UpdateAttributes()");
    return true;
}


// DoRead()
TEMP void THIS::DoRead(TokenReader& rTr,  const std::string& rLabel, const Type* pContext) {
  FD_DG("lhaGenerator(" << this << ")::DoRead(...)");
  std::cout << "lhaGenerator(" << this << ")::DoRead(...)";
  // call base for the actual reading
  BASE::DoRead(rTr,rLabel,pContext);
  // loop polyhedra and fix
  StateSet::Iterator sit;
  for(sit = BASE::StatesBegin(); sit!= BASE::StatesEnd(); ++sit) {
    if(Invariant(*sit).Dimension()==0) Invariantp(*sit)->Dimension(Dimension());
    if(Invariant(*sit).Dimension()!=Dimension()) {
      std::stringstream errstr;
      errstr << "dimension mismatch for invariant at state index " << *sit;
      throw Exception("lhaGenerator::DoRead", errstr.str(), 700);
    }
    if(InitialConstraint(*sit).Dimension()==0) InitialConstraintp(*sit)->Dimension(Dimension());
    if(InitialConstraint(*sit).Dimension()!=Dimension()) {
      std::stringstream errstr;
      errstr << "dimension mismatch for initial constraint at state index " << *sit;
      throw Exception("lhaGenerator::DoRead", errstr.str(), 700);
    }
    if(Rate(*sit).Dimension()==0) Ratep(*sit)->Dimension(Dimension());
    if(Rate(*sit).Dimension()!=Dimension()) {
      std::stringstream errstr;
      errstr << "dimension mismatch for rate at state index " << *sit;
      throw Exception("lhaGenerator::DoRead", errstr.str(), 700);
    }
  }
  TransSet::Iterator tit;
  for(tit = BASE::TransRelBegin(); tit!= BASE::TransRelEnd(); ++tit) {
    if(Guard(*tit).Dimension()==0) Guardp(*tit)->Dimension(Dimension());
    if(Guard(*tit).Dimension()!=Dimension()) {
      std::stringstream errstr;
      errstr << "dimension mismatch for guard transition index " << tit->Str();
      throw Exception("lhaGenerator::DoRead", errstr.str(), 700);
    }
    if(Reset(*tit).Dimension()==0) Resetp(*tit)->Dimension(Dimension());
    if(Reset(*tit).Dimension()!=Dimension()) {
      std::stringstream errstr;
      errstr << "dimension mismatch for reset transition index " << tit->Str();
      throw Exception("lhaGenerator::DoRead", errstr.str(), 700);
    }
  }
}

// DoWrite()
TEMP void THIS::DoWrite(TokenWriter& rTw,  const std::string& rLabel, const Type* pContext) const {
  this->DoXWrite(rTw,rLabel,pContext);
}

#undef BASE
#undef THIS
#undef TEMP



} // namespace faudes


#endif

