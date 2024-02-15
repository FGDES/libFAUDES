/** @file pd_parser.h  parser data structure*/


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/


#ifndef FAUDES_PD_PARSER_H
#define FAUDES_PD_PARSER_H

#include "corefaudes.h"
#include "pd_grammar.h"

namespace faudes {

/**
* Lr1 Configuration
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
  
class FAUDES_API Lr1Configuration {

  public:
  
  Nonterminal mLhs;
  GrammarSymbolVector mBeforeDot;
  GrammarSymbolVector mAfterDot;
  Terminal mLookahead;
  
  /**
   * Constructor TODO make sure that vectors are not empty
   * 
   * @param lhs
   *    lefthand side nonterminal
   * @param beforeDot
   *    before dot grammar symbols
   * @param afterDot
   *    after dot grammar symbols
   * @param lookahead
   *    lookahead terminal
   */
  Lr1Configuration (const Nonterminal& lhs, const GrammarSymbolVector& beforeDot, const GrammarSymbolVector& afterDot, const Terminal& lookahead);
      
  /**
   * Getter for mLhs
   * 
   * @return
   *    mLhs
   */
  Nonterminal const& Lhs() const { return mLhs; }
  
  /**
   * Getter for mBeforeDot
   * 
   * @return
   *    mBeforeDot
   */
  GrammarSymbolVector const& BeforeDot() const { return mBeforeDot; }
  
  /**
   * Getter for mAfterDot
   * 
   * @return
   *    mAfterDot
   */
  GrammarSymbolVector const& AfterDot() const { return mAfterDot; }
  
  /**
   * Getter for mLookahead
   * 
   * @return
   *    mLookahead
   */
  Terminal const& Lookahead() const { return mLookahead; }
  
  /**
   * < operator
   * @param other
   *    cmp
   * @return 
   *    true if this < cmp, else false
   */
  bool operator<(const Lr1Configuration& other) const;
  
  /**
   * == operator
   * @param other
   *    cmp
   * @return 
   *    true if this == cmp, else false
   */
  bool operator==(const Lr1Configuration& other) const;
  
  /**
   * != operator
   * @param other
   *    cmp
   * @return 
   *    true if this != cmp, else false
   */
  bool operator!=(const Lr1Configuration& other) const;
  
   
   /**
   * To String function
   */
  std::string Str() const;

};//class Lr1Configuration

//comparator for two sets of Lr1Configurations
extern FAUDES_API bool CompareConfigSet(const std::set<Lr1Configuration>& lhs, const std::set<Lr1Configuration>& rhs);

struct ConfigSetComparator {
  bool operator() (const std::set<Lr1Configuration>& lhs, const std::set<Lr1Configuration>& rhs) const{
    return CompareConfigSet(lhs,rhs);
  }
};

//convencience definition for a set of sets of Lr1Configurations
typedef std::set<std::set<Lr1Configuration>, ConfigSetComparator> Lr1ConfigurationSetSet;

//convencience definition for a pair of a set of Lr1Configurations and a GrammarSymbolPtr
typedef std::pair<std::set<Lr1Configuration>, GrammarSymbolPtr> ConfigSetGsPair;

//convencience definition for a vector of sets of Lr1Configurations
typedef std::vector<std::set<Lr1Configuration> > ConfigSetVector;

//comparator for two pairs of configuration set and grammar symbol pointer
bool CompareConfigGsPair(const ConfigSetGsPair& lhs, const ConfigSetGsPair& rhs);

struct ConfigGsPairComparator {
  bool operator() (const ConfigSetGsPair& lhs, const ConfigSetGsPair& rhs) const{
    return CompareConfigGsPair(lhs,rhs);
  }
};

//convencience definition for a map of ConfigSetGsPairs to sets of Lr1Configurations
typedef std::map<ConfigSetGsPair, std::set<Lr1Configuration>, ConfigGsPairComparator> LrmTransitionMap;

//convenience definition for a set of ConfigSetGsPairs
typedef std::set<ConfigSetGsPair, ConfigGsPairComparator> ConfigSetGsPairSet;

/**
 * To string function for configuration sets.
 * 
 * @param configs
 *      the configuration set to turn into a string
 * @return
 *      the configuration set as a string
 */
extern FAUDES_API std::string ConfigSetToStr(const std::set<Lr1Configuration>& configs);

/**
 * To string function for configuration set sets.
 * 
 * @param configSetSet
 *      the configuration set sets to turn into a string
 * @return
 *      the configuration set sets as a string
 */
extern FAUDES_API std::string ConfigSetSetToStr(const Lr1ConfigurationSetSet configSetSet);

/**
 * To string function for a transition map.
 * 
 * @param transitionMap
 *      the transition map to turn into a string
 * @return
 *      the transition map as a string
 */
extern FAUDES_API std::string TransitionMapToStr(const LrmTransitionMap& transitionMap);

/**
* 
* Lr1ParserActionElement
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
class FAUDES_API Lr1ParserActionElement{
  
  public:
  
  std::vector<Idx> mStateStack;
  Terminal mNextTerminal;
  
  /**
   * Constructor
   * 
   * @param stateStack
   *    stack of states
   * @param terminal
   *    terminal
   */
  Lr1ParserActionElement(const std::vector<Idx>& stateStack, const Terminal& terminal) : mStateStack(stateStack), mNextTerminal(terminal) {}
  
  /**
   * Getter for mStateStack
   * 
   * @return
   *    mStateStack
   */
  std::vector<Idx> const& StateStack() const { return mStateStack; }
  
  /**
   * Getter for mTerminal
   * 
   * @return
   *    mTerminal
   */
  Terminal const& NextTerminal() const { return mNextTerminal; }
  
  /**
   * < operator
   * @param other
   *    cmp
   * @return 
   *    true if this < cmp, else false
   */
   bool operator<(const Lr1ParserActionElement& other) const;
   
   /**
   * To String function
   */
  std::string Str() const;
  
};//class Lr1ParserActionElement

/**
* 
* Lr1ParserAction
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
class FAUDES_API Lr1ParserAction{
  
  public:
  
  Lr1ParserActionElement mLhs;
  Lr1ParserActionElement mRhs;
  GrammarProduction mProduction;
  bool mIsReduce;
  
  /**
   * Constructor for shift actions
   * 
   * @param lhs
   *    lhs
   * @param rhs
   *    rhs
   */
   Lr1ParserAction(const Lr1ParserActionElement& lhs, const Lr1ParserActionElement& rhs): mLhs(lhs), mRhs(rhs), mProduction(GrammarProduction(Nonterminal(0,std::vector<Idx>()), GrammarSymbolVector())), mIsReduce(false) {}
  
  /**
   * Constructor for reduce actions
   * 
   * @param lhs
   *    lhs
   * @param rhs
   *    rhs
   * @param production
   *    grammar production that causes this rule
   */
  Lr1ParserAction(const Lr1ParserActionElement& lhs, const Lr1ParserActionElement& rhs, const GrammarProduction production) : mLhs(lhs), mRhs(rhs), mProduction(production), mIsReduce(true) {}
  
  /**
   * Getter for mLhs
   * 
   * @return
   *    mLhs
   */
  Lr1ParserActionElement const& Lhs() const { return mLhs; }
  
  /**
   * Getter for mRhs
   * 
   * @return
   *    mRhs
   */
  Lr1ParserActionElement const& Rhs() const { return mRhs; }
  
  /**
   * < operator
   * @param other
   *    cmp
   * @return 
   *    true if this < cmp, else false
   */
   bool operator<(const Lr1ParserAction& other) const;
   
   /**
   * To String function
   */
  std::string Str() const;
  
};//class Lr1ParserAction

/**
* 
* Lr1Parser
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
class FAUDES_API Lr1Parser {
  public:
  
  Idx mStartState;
//  Idx mFinalState;
  std::set<Idx> mFinalStates;
  std::set<Idx> mNonterminals;
  std::set<Terminal> mTerminals;
  std::set<Lr1ParserAction> mActions;
  Terminal mAugSymbol;
  
  /**
   * default constructor
   */
  Lr1Parser(): mAugSymbol(Terminal(0)) {};

  /**
   * Getter for mStartState
   * 
   * @return
   *    mStartState
   */
  Idx StartState() const { return mStartState; }
  
  /**
   * Getter for mFinalState
   * 
   * @return
   *    mFinalState
   */
 // Idx FinalState() const { return mFinalState; }
  std::set<Idx> const& FinalStates() const { return mFinalStates;}

  void ClrFinalStates(){mFinalStates.clear();} //TODO

  /**
   * Getter for mNonterminals
   * 
   * @return
   *    mNonterminals
   */
  std::set<Idx> const& Nonterminals() const { return mNonterminals; }
  
  /**
   * Getter for mTerminals
   * 
   * @return
   *    mTerminals
   */
  std::set<Terminal> const& Terminals() const { return mTerminals; }
  
  /**
   * Getter for mActions
   * 
   * @return
   *    mActions
   */
  std::set<Lr1ParserAction> const& Actions() const { return mActions; }
  
  /**
   * Getter for mAugSymbol
   * 
   * @return
   *    mAugSymbol
   */
  Terminal AugSymbol() const { return mAugSymbol; }
  
  /**
   * set the parser's start state and add it to the set of nonterminals
   * 
   * @param start
   *    start state to set
   * @return 
   *    true, if nonterminal did not exist in parser, else false
   */
  bool SetStartState(Idx start);
  
  /**
   * set the parser's final state and add it to the set of nonterminals
   * 
   * @param final
   *    final state to set
   * @return 
   *    true, if nonterminal did not exist in parser, else false
   */
  bool InsFinalState(Idx final);
  
  /**
   * Add a new nonterminal to the parser.
   * 
   * @param nt
   *    the nonterminal to add
   * @return
   *    true if the nonterminal did not exist and was successfully added,
   * else false
   */
  bool InsNonterminal(Idx nt);
  
  /**
   * TODO description
   * 
   * @param t
   *    aug symbol
   */
  void SetAugSymbol(Terminal t);
  
  /**
   * Add a new terminal to the parser.
   * 
   * @param t
   *    the terminal to add
   * @return
   *    true if the terminal did not exist and was successfully added,
   * else false
   */
  bool InsTerminal(const Terminal& t);
  
  /**
   * Add a new Lr1ParserAction to the parser
   * 
   * @param action
   *    the action to add
   * @return
   *    true if the action did not exist and was successfully added, else false
   */
  bool InsAction(const Lr1ParserAction& action);
  
  /**
    * To String function for all nonterminals
    */
   std::string StrFinalStates() const;

  /**
   * To String function for all nonterminals
   */
  std::string StrNonterminals() const;
  
  /**
   * To String function for all terminals
   */
  std::string StrTerminals() const;
  
  /**
   * To String function for actions
   */
  std::string StrActions(std::string separator) const;
  
  /**
   * To String function
   */
  std::string Str() const;
    
};//class Lr1Parser

} // namespace faudes

#endif
