/** @file pd_gotoattributes.h  Attributes for LR machines */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/


#ifndef FAUDES_PD_GOTOATTRIBUTES_H
#define FAUDES_PD_GOTOATTRIBUTES_H

#include "corefaudes.h"
#include "pd_parser.h"

namespace faudes {
 
  /**
 *
 * Event attribute with grammar symbols.
 *
 * @ingroup PushdownPlugin  
 *
 */

class FAUDES_API AttributeGotoTransition : public AttributeVoid {

FAUDES_TYPE_DECLARATION(Void,AttributeGotoTransition,AttributeVoid)

 public:

  /** Constructor */
  AttributeGotoTransition(void) { mpGrammarSymbol = GrammarSymbolPtr(); };

  /** 
   * Test for default value (no grammar symbol set)
   *
   * @return
   *  True for default value
   */
  virtual bool IsDefault(void) const {return !mpGrammarSymbol; };

  /** Grammar Symbols */
  GrammarSymbolPtr mpGrammarSymbol;
  
  /**
   * Getter for mpGrammarSymbol
   * 
   * @return
   *    the grammar symbol
   */
  const GrammarSymbolPtr& Symbol() const { return mpGrammarSymbol; }
  
  /**
   * Setter for mpGrammarSymbol
   * 
   * @param symbol
   *    the grammar symbol
   */
  void Symbol(const GrammarSymbolPtr& symbol) { mpGrammarSymbol = symbol; }
  
 protected:

  /**
   * Assignment method. 
   *
   * @param rSrcAttr
   *    Source to assign from
   */
  void DoAssign(const AttributeGotoTransition& rSrcAttr);

  /**
   * Test equality. 
   *
   * @param rOther
   *    Other attribute to compare with.
   */
  bool DoEqual(const AttributeGotoTransition& rOther) const;

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

};//class AttributeGotoTransition
  
  
/**
 * 
 * State attribute with configuration set
 *
 * @ingroup PushdownPlugin  
 *
 */

class FAUDES_API AttributeGotoState : public AttributeFlags {

FAUDES_TYPE_DECLARATION(Void,AttributeGotoState,AttributeFlags)

 public:

  /** Constructor */
  AttributeGotoState(void) : AttributeFlags() { };

  /** 
   * Test for default value (ie default flags)
   *
   * @return
   *  True for default value
   */
  virtual bool IsDefault(void) const {return AttributeFlags::IsDefault() && mConfigSet.empty(); };

  /** Configuration set  */
  std::set<Lr1Configuration> mConfigSet;
  
  /**
   * Getter for mConfigSet
   * 
   * @return
   *    the config set
   */
  const std::set<Lr1Configuration>& ConfigSet() const { return mConfigSet; }
  
  /**
   * Setter for ConfigSet
   * 
   * @param configs
   *    the config set
   */
  void ConfigSet(const std::set<Lr1Configuration>& configs) { mConfigSet = configs; }
 
 protected:
  
   

  /**
   * Assignment method. 
   *
   * @param rSrcAttr
   *    Source to assign from
   */
  void DoAssign(const AttributeGotoState& rSrcAttr);


  /**
   * Test eaulity. 
   *
   * @param rOther
   *    Other attribute to compare with.
   */
  bool DoEqual(const AttributeGotoState& rOther) const;

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


}; // class AttributePushdownState

    









} // namespace faudes

#endif
