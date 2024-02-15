/** @file hyp_attributes.h  Attributes of linear hybrid automata */

/* 
   Hybrid systems plug-in  for FAU Discrete Event Systems Library 

   Copyright (C) 2010  Thomas Moor

*/


#ifndef FAUDES_HYB_ATTRIBUTES_H
#define FAUDES_HYB_ATTRIBUTES_H

#include "corefaudes.h"
#include "hyb_parameter.h"

namespace faudes {


/** 
 * Linear hybrid automata transition attribute with guard and resets. 
 * The guard condition is represented by a Polyhedron, the reset by
 * a LinearRelation. This is a cheap container, both parameters can
 * be directly accessed as public members mGuard and mReset.
 *
 * @ingroup HybridPlugin  
 */

class FAUDES_API AttributeLhaTrans : public AttributeFlags {

FAUDES_TYPE_DECLARATION(Void,AttributeLhaTrans,AttributeFlags)

 public:

  /** Constructor */
    AttributeLhaTrans(void) : AttributeFlags() { };

 /** 
  * Test for default value.
  * True for no guard restriction and identity reset.
  *
  * @return
  *  True for default value
  */
  virtual bool  IsDefault(void) const {
    return AttributeFlags::IsDefault() && mGuard.Size()==0 && mReset.Identity(); };

  /** Guard  */
  Polyhedron mGuard;

  /** Resets */
  LinearRelation mReset;

 protected:

  /**
   * Assignment method. 
   *
   * @param rSrcAttr
   *    Source to assign from
   */
  AttributeLhaTrans& DoAssign(const AttributeLhaTrans& rSrcAttr);

  /**
   * Test eaulity. 
   * Note: this test refers to the parametrisation as opposed to the
   * relevant polyhedra.
   *
   * @param rOther
   *    Other attribute to compare with.
   */
  bool DoEqual(const AttributeLhaTrans& rOther) const;

  /**
   * Reads the attribute from TokenReader, see AttributeVoid for public wrappers.
   * 
   * If the current token indicates a hybrid system parameter section, the method reads 
   * the guard and reset data from that section. Else it does nothing. Exceptions may only be thrown
   * on invalid data within the parameter section. The context argument is ignored, the
   * label argument can be used to override the default section name Timing.
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
  virtual void DoRead(TokenReader& rTr,const std::string& rLabel="", const Type* pContext=0);
 
  /**
   * Writes the attribute to TokenWriter, see AttributeVoid for public wrappers.
   *
   * Writes a hybrid systems parameter section to include data on the guard and resets. 
   * The label argument can be used to set a section label different the the default LhaTransition. 
   * The context argument is ignored.  
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
  virtual void DoWrite(TokenWriter& rTw, const std::string& rLabel="", const Type* pContext=0) const;

  /**
   * Writes the attribute to TokenWriter, see AttributeVoid for public wrappers.
   *
   * Same as DoWrite but un strict XML format.
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
  virtual void DoXWrite(TokenWriter& rTw, const std::string& rLabel="", const Type* pContext=0) const;

}; // class AttributeLhaTrans


/**
 * 
 * Linear hybrid automata state attribute with invariant, rates and optional initial state constraint.
 * All enteties are represented by polyherda.
 * The initial state constraints apply when the respective location is set up as a
 * discrete initial state.
 *
 * @ingroup HybridPlugin  
 *
 */

class FAUDES_API AttributeLhaState : public AttributeFlags {

FAUDES_TYPE_DECLARATION(Void,AttributeLhaState,AttributeFlags)

 public:

  /** Constructor */
  AttributeLhaState(void) : AttributeFlags() { };

 /** 
  * Test for default value (never)
  *
  * @return
  *  True for default value
  */
 virtual bool IsDefault(void) const { return false; };

  /** Invariant  */
  Polyhedron mInvariant;
  
  /** Initial state constraint  */
  Polyhedron mInitialConstraint;
  
  /** Rate */
  Polyhedron mRate;
 
 protected:


  /**
   * Assignment method. 
   *
   * @param rSrcAttr
   *    Source to assign from
   */
  AttributeLhaState& DoAssign(const AttributeLhaState& rSrcAttr);


  /**
   * Test eaulity. 
   *
   * @param rOther
   *    Other attribute to compare with.
   */
  bool DoEqual(const AttributeLhaState& rOther) const;

  /**
   * Reads the attribute from TokenReader, see AttributeVoid for public wrappers.
   * 
   * If the current token indicates a hybrid systems parameter section, the method reads the 
   * parameters from that section. Else, it does nothing. Exceptions may only be thrown
   * on invalid data within the parameter section. The context argument is ignored, the
   * label argument can be used to override the default section name Invariant.
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
   * Writes the attribute to TokenWriter, see AttributeVoid for public wrappers.
   *
   * Writes a hybrid system parameter section to include data on the invariant. The label argument
   * can be used to set a section label different the the default LhaState. 
   * The context argument is ignored.  
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
  virtual void DoWrite(TokenWriter& rTw, const std::string& rLabel="", const Type* pContext=0) const;

  /**
   * Writes the attribute to TokenWriter, see AttributeVoid for public wrappers.
   *
   * Same as DoWrite but in strict XML format.
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
  virtual void DoXWrite(TokenWriter& rTw, const std::string& rLabel="", const Type* pContext=0) const;


}; // class AttributeLhaState


/**
 *
 * Linear hybrid automata globat attribute to specify the overall
 * state space. All invariants are interpreted as intersection with
 * the global state space.
 *
 * @ingroup HybridPlugin  
 *
 */

class FAUDES_API AttributeLhaGlobal : public AttributeVoid {

FAUDES_TYPE_DECLARATION(Void,AttributeLhaGlobal,AttributeVoid)

 public:

  /** Constructor */
  AttributeLhaGlobal(void) { };

  /** 
   * Test for default value (never)
   *
   * @return
   *  True for default value
   */
  virtual bool IsDefault(void) const {return false;};

  /** Continuous states */
  Polyhedron mStateSpace;


 protected:

  /**
   * Assignment method. 
   *
   * @param rSrcAttr
   *    Source to assign from
   */
  AttributeLhaGlobal& DoAssign(const AttributeLhaGlobal& rSrcAttr);

  /**
   * Test eaulity. 
   * Note: the test refers to the parametrisation.
   *
   * @param rOther
   *    Other attribute to compare with.
   */
  bool DoEqual(const AttributeLhaGlobal& rOther) const;

  /**
   * Reads the attribute from TokenReader, see AttributeVoid for public wrappers.
   * 
   * If the current token indicates a hybrid system parameter section, the method reads the global
   * data from that section. Else, it does nothing. Exceptions may only be thrown
   * on invalid data within the parameter section. The context argument is ignored, the
   * label argument can be used to override the default section name Clocks.
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
  virtual void DoRead(TokenReader& rTr,const std::string& rLabel="", const Type* pContext=0);
 
  /**
   * Writes the attribute to TokenWriter, see AttributeVoid for public wrappers.
   *
   * Writes hybrid automata global data. The label argument
   * can be used to set a section label different the the default LhaGlobal. 
   * The context argument is ignored.  
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
  virtual void DoWrite(TokenWriter& rTw, const std::string& rLabel="", const Type* pContext=0) const;


}; // class AttributeLhaGlobal


} // namespace faudes

#endif
