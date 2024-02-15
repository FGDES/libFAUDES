/** @file pd_attributes.h  Attributes for pushdown automata */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/


#ifndef FAUDES_PD_ATTRIBUTES_H
#define FAUDES_PD_ATTRIBUTES_H

#include "corefaudes.h"
#include "pd_basics.h"
#include "pd_merge.h"
#include <utility>

namespace faudes {
  
/**
 * 
 * State attribute with substates. Merged substates should only needed to be accessed for debugging.
 *
 * @ingroup PushdownPlugin  
 *
 */

class FAUDES_API AttributePushdownState : public AttributeFlags {

FAUDES_TYPE_DECLARATION(Void,AttributePushdownState,AttributeFlags)

 public:

  /** Constructor */
  AttributePushdownState(void) : AttributeFlags() { mpMerge = NULL; mDfaState = 0;};
  
  /** Exlicit copy-constructor to care pointer member (tmoor 2016-02 )*/
  AttributePushdownState(const AttributePushdownState& rSrc) : AttributeFlags(rSrc), mpMerge(NULL) { DoAssign(rSrc);};

  /** Destructor 
   * 
   * deletes mpMerge
   */
  ~AttributePushdownState();
  
  /** 
   * Test for default value (ie default flags)
   *
   * @return
   *  True for default value
   */
  virtual bool IsDefault(void) const {return AttributeFlags::IsDefault() && mpMerge == NULL;};

  /**
   * Sets the mpMerge parameter. Allocates memory for the pointer.
   * The reference given in the parameter will be copied into the new memory.
   * Current mpMerge will be deleted. (tmoor 2016-08: pointer argument)
   * 
   * @param rMerge
   *    the merge parameter
   */
  void SetMerge(const MergeAbstract* pMerge);
  
  /**
   * Getter for Merge
   * 
   * @return
   *    mpMerge
   */
  const MergeAbstract* Merge() const { return mpMerge;}
  
  /**
   * Setter for DfaState
   * 
   * @return
   *    mDfaState
   */
  void DfaState(const Idx dfaState) { mDfaState = dfaState;}
  
  /**
   * Getter for DfaState
   * 
   * @return
   *    mDfaState
   */
  const Idx DfaState() const { return mDfaState;}
  
  private:
    
  /** Substates  */
  MergeAbstract* mpMerge;
  /** Associated DFA state */
  Idx mDfaState;
 
 protected:


  /**
   * Assignment method. 
   *
   * @param rSrcAttr
   *    Source to assign from
   */
  void DoAssign(const AttributePushdownState& rSrcAttr);


  /**
   * Test equality. 
   *
   * @param rOther
   *    Other attribute to compare with.
   */
  bool DoEqual(const AttributePushdownState& rOther) const;

  /**
   * Reads the attribute from TokenReader, see AttributeVoid for public wrappers.
   * 
   * If the current token indicates a invariant section, the method reads the invariant
   * from that section. Else, it does nothing. Exceptions may only be thrown
   * on invalid data within the timing section. The context argument is ignored, the
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
   * Writes am Invariant section to include data on the invariant. The label argument
   * can be used to set a section label different the the default Invariant. 
   * Th context argument is ignored.  
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
  
  virtual void DoXWrite(TokenWriter& rTw, const std::string& rLabel="", const Type* pContext=0) const {DoWrite(rTw,rLabel,pContext); };

}; // class AttributePushdownState

	
/**
 *
 * Transition attribute with pop and push.
 *
 * @ingroup PushdownPlugin  
 *
 */
typedef  std::set<std::pair<std::vector<Idx>,std::vector<Idx> > > PopPushSet;

class FAUDES_API AttributePushdownTransition : public AttributeVoid {

FAUDES_TYPE_DECLARATION(Void,AttributePushdownTransition,AttributeVoid)

 public:

  /** Constructor */
  AttributePushdownTransition(void) : AttributeVoid() { };

  /** 
   * Test for default value (ie default flags)
   *
   * @return
   *  True for default value
   */
  virtual bool  IsDefault(void) const {
     return mPopPush.empty();
  }; 
  
  /** Pop and Push. To allow transitions with the same start state, event and event state, but with different pop and push, pop and push are stored in a vector of pop/push pairs. */
  PopPushSet mPopPush;
  
  /**
   * Getter for PopPush
   * 
   * @return
   *    mPopPush
   */
  const PopPushSet& PopPush() const { return mPopPush;}
  
  /**
   * Delete a pop push pair from the the transitions
   * 
   * @param rPop
   *    the pop part of the pair to delete
   * * @param rPush
   *    the push part of the pair to delete
   * @return
   *    true if pair did exist and deletion was successful, else false
   */
  bool ClrPopPush(const std::vector<Idx>& rPop, const std::vector<Idx>& rPush);
  
 protected:

  /**
   * Assignment method. 
   *
   * @param rSrcAttr
   *    Source to assign from
   */
  void DoAssign(const AttributePushdownTransition& rSrcAttr);

  /**
   * Test equality. 
   *
   * @param rOther
   *    Other attribute to compare with.
   */
  bool DoEqual(const AttributePushdownTransition& rOther) const;

  /**
   * Reads the attribute from TokenReader, see AttributeVoid for public wrappers.
   * 
   * If the current token indicates a timing section, the method reads the guard and reset 
   * timing data from that section. Else it does nothing. Exceptions may only be thrown
   * on invalid data within the timing section. The context argument is ignored, the
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
   * Writes a Timing section to include data on the guard and resets. The label argument
   * can be used to set a section label different the the default Timing. 
   * Th context argument is ignored.  
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

  virtual void DoXWrite(TokenWriter& rTw, const std::string& rLabel="", const Type* pContext=0) const {DoWrite(rTw,rLabel,pContext); };



}; // class AttributePushdownTransition

/**
 *
 * Global attribute with stack alphabet and stack bottom.
 *
 * @ingroup PushdownPlugin  
 *
 */

class FAUDES_API AttributePushdownGlobal : public AttributeVoid {

FAUDES_TYPE_DECLARATION(Void,AttributePushdownGlobal,AttributeVoid)

 public:

  /** Constructor */
  AttributePushdownGlobal(void) : AttributeVoid() {
    mpStackSymbolTable=mStackSymbols.SymbolTablep();
    mStackBottom=0; // TM 2017
  };

  /** 
   * Test for default value (no stack symbols)
   *
   * @return
   *  True for default value
   */
  virtual bool IsDefault(void) const {return mStackSymbols.Empty(); };

  /** Stack Symbols */
    StackSymbolSet mStackSymbols;
    Idx mStackBottom;

  /** Pointer to stack symbol table */
    SymbolTable* mpStackSymbolTable;

 protected:

  /**
   * Assignment method. 
   *
   * @param rSrcAttr
   *    Source to assign from
   */
  void DoAssign(const AttributePushdownGlobal& rSrcAttr);

  /**
   * Test equality. 
   *
   * @param rOther
   *    Other attribute to compare with.
   */
  bool DoEqual(const AttributePushdownGlobal& rOther) const;

  /**
   * Reads the attribute from TokenReader, see AttributeVoid for public wrappers.
   * 
   * If the current token indicates a Clocks section, the method reads the global
   * timing data from that section. Else, it does nothing. Exceptions may only be thrown
   * on invalid data within the timing section. The context argument is ignored, the
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
   * Writes a Clocks section to include global timing data. The label argument
   * can be used to set a section label different the the default Clocks. 
   * Th context argument is ignored.  
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
  
  virtual void DoXWrite(TokenWriter& rTw, const std::string& rLabel="", const Type* pContext=0) const {DoWrite(rTw,rLabel,pContext); };

};





} // namespace faudes

#endif
