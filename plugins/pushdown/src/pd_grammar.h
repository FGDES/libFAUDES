/** @file pd_merge.h  grammar data structure */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/


#ifndef FAUDES_PD_GRAMMAR_H
#define FAUDES_PD_GRAMMAR_H

#include "corefaudes.h"
#include "pd_basics.h"
#include "pd_pdgenerator.h"

#ifdef FAUDES_PD_TR1
#include <tr1/memory>
namespace std {
using std::tr1::shared_ptr;
using std::tr1::dynamic_pointer_cast;
}
#else
#include <memory>
#endif

namespace faudes {

/**
* Grammar Symbol
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
class FAUDES_API GrammarSymbol{
  
  public:
  
  /**
   * To String function
   */
  virtual std::string Str() const = 0;
  
  /**
   * < operator
   * @param other
   *    cmp
   * @return 
   *    true if this < cmp, else false
   */
   virtual bool operator<(const GrammarSymbol& other) const = 0;
   
   /**
   * == operator
   * @param other
   *    cmp
   * @return 
   *    true if this == cmp, else false
   */
   bool operator==(const GrammarSymbol& other) const;
   
   /**
   * != operator
   * @param other
   *    cmp
   * @return 
   *    true if this != cmp, else false
   */
   bool operator!=(const GrammarSymbol& other) const;
   
   /**
    * Clone function. Allocates memory!
    * 
    * @return
    *   pointer to cloned object
    */
   virtual GrammarSymbol* Clone() const = 0;
  
};//class GrammarSymbol

/**
* Terminal
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
class FAUDES_API Terminal : public GrammarSymbol{

  public:
  
  /** Symbol representing the terminals */
  //std::string mSymbol;
  /** Event representing the terminal */
  Idx mEvent;
  
  
  /**
   * Constructor
   * 
   * @param event
   *    the event
   */
  Terminal(const Idx event) : mEvent(event){};
  
  /**
   * Getter for event
   * 
   * @return
   *    the event
   */
  Idx Event() const{return mEvent;}
  
  /**
   * Determine if the event is lambda
   * 
   * @return
   *    true if it is lambda, else false
   */
  bool IsLambda() const;
  
  /**
   * To String function
   */
  std::string Str() const;
  
  /**
   * < operator
   * @param other
   *    cmp
   * @return 
   *    true if this < cmp, else false
   */
   bool operator<(const GrammarSymbol& other) const;
   
   /**
    * Clone function. Allocates memory!
    * 
    * @return
    *   pointer to cloned Terminal
    */
   Terminal* Clone() const;
   
};//class Terminal

/**
 * Nonterminals are constructed from two states and a
 * stack symbol or one state and a stack symbol
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
class FAUDES_API Nonterminal : public GrammarSymbol{
  
  public: 
  
  /** The start state*/
  Idx mStartState;
  /** The stack symbols*/
  std::vector<Idx> mOnStack;
   /** The end state */
  Idx mEndState;
  
  
  /**
   * Constructor for NonterminalEnd
   */
  Nonterminal(const Idx state, const std::vector<Idx> symbols):mStartState(state), mOnStack(symbols), mEndState(0){}
  
  /**
   * Constructor for NonterminalMid
   */
  Nonterminal(const Idx state1, const std::vector<Idx> symbols, const Idx state2) : mStartState(state1), mOnStack(symbols), mEndState(state2){}
  
  /**
   * Getter for startState
   * 
   * @return
   *    Idx of the startState
   */
  Idx StartState() const{return mStartState;}
  
  /**
   * Getter for onStack
   * 
   * @return
   *    indices of the stack symbols
   */
  const std::vector<Idx>& OnStack() const {return mOnStack;}
  
  /**
   * Getter for endState
   * 
   * @return
   *    Idx of the endState
   */
  Idx EndState() const {return mEndState;}
  
  /**
   * To String function
   */
  virtual std::string Str() const;
  
  /**
   * < operator
   * @param other
   *    cmp
   * @return 
   *    true if this < cmp, else false
   */
   bool operator<(const GrammarSymbol& other) const;
   
   /**
    * Clone function. Allocates memory!
    * 
    * @return
    *   pointer to cloned nonterminal
    */
   Nonterminal* Clone() const;

};//class Nonterminal

//convenience definition for a grammar symbol pointer that manages its own memory
typedef std::shared_ptr<GrammarSymbol> GrammarSymbolPtr;
typedef std::shared_ptr<const GrammarSymbol> ConstGrammarSymbolPtr;

//convenience definition for a nonterminal pointer that manages its own memory
typedef std::shared_ptr<Nonterminal> NonterminalPtr;
typedef std::shared_ptr<const Nonterminal> ConstNonterminalPtr;

//convenience definition for a terminal pointer that manages its own memory
typedef std::shared_ptr<Terminal> TerminalPtr;
typedef std::shared_ptr<const Terminal> ConstTerminalPtr;

//convencience definition for a vector of grammar symbols
typedef std::vector<GrammarSymbolPtr> GrammarSymbolVector;

//comparator for two grammar symbol vectors
extern FAUDES_API bool CompareGsVector(const GrammarSymbolVector& lhs, const GrammarSymbolVector& rhs);

struct GsVectorComparator {
  bool operator() (const GrammarSymbolVector& lhs, const GrammarSymbolVector& rhs) const{
    return CompareGsVector(lhs,rhs);
  }
};

//equals for two grammar symbol vectors
extern FAUDES_API bool EqualsGsVector(const GrammarSymbolVector& lhs, const GrammarSymbolVector& rhs);

//comparator for two grammar symbols
extern FAUDES_API bool CompareGs(const GrammarSymbolPtr& lhs, const GrammarSymbolPtr& rhs);

struct GsComparator {
  bool operator() (const GrammarSymbolPtr& lhs, const GrammarSymbolPtr& rhs) const{
    return CompareGs(lhs,rhs);
  }
};

//convencience definition for a set of grammar symbols
typedef std::set<GrammarSymbolPtr, GsComparator> GrammarSymbolSet;

//convencience definition for a set of vectors of grammar symbols (a set of words)
typedef std::set<GrammarSymbolVector,GsVectorComparator> GrammarSymbolWordSet;

typedef std::map<GrammarSymbolVector, std::set<Terminal>, GsVectorComparator> GrammarSymbolWordMap;

/**
* Test if a given set of words contains a specific word
* 
* @param set
*      the set to be searched
* @param word
*      the word to be searched for
* @return
*      true, if the set contains the word, else false
*/
extern FAUDES_API bool ContainsWord(const GrammarSymbolWordSet& set, const GrammarSymbolVector& word);

/**
* Grammar Production
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
class FAUDES_API GrammarProduction{
  
  public:
  
  /** Nonterminal left hand side */
  Nonterminal mLhs;
  /** Right hand side, can contain both Terminals and Nonterminals */
  GrammarSymbolVector mRhs;
  
  /**
   * Constructor
   */
  GrammarProduction(const Nonterminal& lhs, const GrammarSymbolVector& rhs) : mLhs(lhs), mRhs(rhs){};
  
  /**
   * Getter for mLhs
   * 
   * @return
   *    mLhs
   */
  Nonterminal const&  Lhs() const {return mLhs;}
  
  /**
   * Getter for mRhs
   * 
   * @return
   *    mRhs
   */
  GrammarSymbolVector const& Rhs() const {return mRhs;}
  
  /**
   * To String function
   */
  std::string Str() const;
  
    /**
   * < operator
   * @param other
   *    cmp
   * @return 
   *    true if this < cmp, else false
   */
   bool operator<(const GrammarProduction& other) const;
  
};//class GrammarProduction

/**
* Grammar
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
class FAUDES_API Grammar{

  public:
  
  /** the Terminals */
  std::set<Terminal> mTerminals;
  /** the Nonterminals */
  std::set<Nonterminal> mNonterminals;
  /** the Start Symbol */
  Nonterminal mStartSymbol;
  /** the Productions */
  std::set<GrammarProduction> mGrammarProductions;
  
  /**
   * Constructor
   */
  Grammar():mStartSymbol(Nonterminal(0,std::vector<Idx>(),0)) {}
  
  /**
   * Constructor
   */
  Grammar(const Nonterminal& startSymbol):mStartSymbol(startSymbol){}
  
  
  /**
   * set the grammar's start symbol and add it to the set of nonterminals
   * 
   * @param s
   *    start symbol to set
   * @return 
   *    true, if nonterminal did not exist in grammar, else false
   */
  bool SetStartSymbol(const Nonterminal& s);
  
  /**
   * add Terminal
   * 
   * @param t
   *    terminal to add
   * @return 
   *    true, if successful (terminal did not exist in grammar), else 
   * false (terminal did already exist in grammar)
   */
  bool InsTerminal(const Terminal& t);
  
  /**
   * add Terminals
   * 
   * @param t
   *    terminals to add
   */
  void InsTerminals(const std::set<Terminal>& t);
  
   /**
   * add Nonterminal
   * 
   * @param nt
   *    nonterminal to add
   * @return 
   *    true, if successful (nonterminal did not exist in grammar), else 
   * false (nonterminal did already exist in grammar)
   */
  bool InsNonterminal(const Nonterminal& nt);
  
  /**
   * add Nonterminals
   * 
   * @param nt
   *    nonterminals to add
   */
  void InsNonterminals(const std::set<Nonterminal>& nt);
  
  /**
   * Add a grammar production to the grammar. All grammar symbols used must exist
   * in the grammar.
   * 
   * @param gp
   *    grammar production to add
   * @return 
   *    true, if successful (grammar production did not exist in grammar), else 
   * false (grammar production did already exist in grammar)
   */
  bool InsGrammarProduction(const GrammarProduction& gp);
  
  /**
   * Add grammar productions to the grammar. All grammar symbols used must exist
   * in the grammar.
   * 
   * @param gp
   *    grammar productions to add
   */
  void InsGrammarProductions(const std::set<GrammarProduction>& gp);
  
  /**
   * Getter for mTerminals
   * 
   * @return
   *    the terminals
   */
  const std::set<Terminal>& Terminals() const { return mTerminals;}
  
  /**
   * Getter for mNonterminals
   * 
   * @return
   *    the nonterminals
   */
  const std::set<Nonterminal>& Nonterminals() const { return mNonterminals;}
  
  /**
   * Getter for mStartSymbol
   * 
   * @return
   *    the start symbol
   */
  const Nonterminal& StartSymbol() const { return mStartSymbol;}
  
  /**
   * Getter for mGrammarProductions
   * 
   * @return
   *    the grammar productions
   */
  const std::set<GrammarProduction>& GrammarProductions() const { return mGrammarProductions;}
  
  /**
   * Iterator to the beginning of terminals
   * 
   * @return
   *    iterator
   */
  std::set<Terminal>::const_iterator TerminalsBegin() const;
  
  /**
   * Iterator to the end of terminals
   * 
   * @return
   *    iterator
   */
  std::set<Terminal>::const_iterator TerminalsEnd() const;
  
  /**
   * Iterator to the beginning of nonterminals
   * 
   * @return
   *    iterator
   */
  std::set<Nonterminal>::const_iterator NonterminalsBegin() const;
  
  /**
   * Iterator to the end of nonterminals
   * 
   * @return
   *    iterator
   */
  std::set<Nonterminal>::const_iterator NonterminalsEnd() const;
  
  /**
   * Iterator to the beginning of grammar productions
   * 
   * @return
   *    iterator
   */
  std::set<GrammarProduction>::const_iterator GrammarProductionsBegin() const;
  
  /**
   * Iterator to the end of grammar productions
   * 
   * @return
   *    iterator
   */
  std::set<GrammarProduction>::const_iterator GrammarProductionsEnd() const;
  
  /**
   * To String function for Terminals
   */
  std::string StrTerminals() const;
  
  /**
   * To String function for Nonterminals
   */
  std::string StrNonterminals() const;
  
  /**
   * To String function for start symbol
   */
  std::string StrStartSymbol() const;
  
  /**
   * To String function for Grammar Productions
   */
  std::string StrGrammarProductions() const;
  
   /**
   * To String function
   */
  std::string Str() const;


};//class Grammar

} // namespace faudes

#endif
