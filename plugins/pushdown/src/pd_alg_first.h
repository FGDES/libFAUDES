/** @file pd_alg_first.h  functions related to the first function*/


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/


#ifndef FAUDES_PD_ALG_FIRST_H
#define FAUDES_PD_ALG_FIRST_H

#include "corefaudes.h"
#include "pd_pdgenerator.h"
#include "pd_alg_sub.h"

namespace faudes {
  
  /**
   * Construct the postfix closure of a word.
   * 
   * @param word
   *    the word
   * @return
   *    all postfixes of the word including lambda
   */
  extern FAUDES_API GrammarSymbolWordSet PostCl(const GrammarSymbolVector& word);
  
  /**
   * Determine certain words in a grammar. The words are all lefthand sides and
   * all righthand sides as well as the righthand sides' postfixes
   *
   * @param gr
   *    the grammar
   * @return
   *    all lefthand sides and all righthand sides' postfixes as words 
   */
  extern FAUDES_API GrammarSymbolWordSet Fds(const Grammar& gr);
 
  /**
   * Updates the function which maps words to their first possible terminal symbols. Must
   * be called until no changes are made in order to have a complete mapping.
   * 
   * @param gr
   *    the grammar on which to work
   * @param f
   *    the mapping function (word -> first symbols)
   * @param madeChanges
   *    this variable will be written to true if changes were made and written to
   * false if no changes were made
   * @return
   *    the updated first function
   */
  extern FAUDES_API GrammarSymbolWordMap First1(const Grammar& gr, const GrammarSymbolWordMap& f, bool* madeChanges);
  
  /**
   * Builds a function that maps words to their first possible terminal
   * symbols by recursively calling First1.
   * 
   * @param gr
   *    the grammar on which to work
   * @param f
   *    the mapping function (word -> first symbols)
   * @return
   *    the first function
   */
  extern FAUDES_API GrammarSymbolWordMap FirstL(const Grammar& gr, const GrammarSymbolWordMap& f);
  
  /**
   * Determines which terminals can come first for a given word. The word must be
   * in Fds()!
   * 
   * @param gr
   *    the grammar on which to work
   * @param word
   *    the word whose first terminals are to be determined
   * @return
   *    set of first terminals
   */
  extern FAUDES_API std::set<Terminal> FirstA(const Grammar& gr, const GrammarSymbolVector& word);
  
  /**
   * Determines which terminals can come first for a given word.
   * 
   * @param gr
   *    the grammar on which to work
   * @param word
   *    the word whose first terminals are to be determined
   * @return
   *    set of first terminals
   */
  extern FAUDES_API std::set<Terminal> First(const Grammar& gr, const GrammarSymbolVector& word);
  
  /**
   * Calls first, but only if all symbols in the word are in the grammar as well.
   * 
   * @param gr
   *    the grammar
   * @param word
   *    the word
   * @return
   *    set of first terminals or empty set if the word contained symbols not in
   *    the grammar
   */
  extern FAUDES_API std::set<Terminal> FirstRed(const Grammar& gr, const GrammarSymbolVector& word);
  
  /**
   * Extract the first symbol from the word that is in the symbol set
   * 
   * @param symbolSet
   *    the  symbol set
   * @param w
   *    the word
   * @return
   *    pointer to the found symbol or NULL if none is found
   */
  extern FAUDES_API NonterminalPtr Filter1(const std::set<Nonterminal>& symbolSet, const GrammarSymbolVector& w);
  
  /**
   * Determines which terminals can come first for a given word and ensures that 
   * the word can be entirely reduced.
   * 
   * @param gr
   *    the grammar on which to work
   * @param word
   *    the word whose first terminals are to be determined
   * @return
   *    set of first terminals or empty set, if the word cannot be entirely reduced
   */
  extern FAUDES_API std::set<Terminal> FirstAll(const Grammar& gr, const GrammarSymbolVector& word);
  
  /**
   * Determine which terminals can come first for a given word for any k <= 1.
   * 
   * @param gr
   *    the grammar on which to work
   * @param k
   *    a natural number k
   * @param word
   *    the word whose first terminals are to be determined
   * @return
   *    for k = 1 and k = 0: set of first terminals
   *    for k = 0 and set of first terminals not empty: lambda
   *    for k > 1: empty set 
   */
  extern FAUDES_API std::set<Terminal> FirstLeq1(const Grammar& gr, uint k, const GrammarSymbolVector& word);
    
  /**
  * Convenience print function for first map function
  * 
  * @param f
  *     map function to print
  * @param changed
  *     indicator if changes were made, defaults to false
  */
  extern FAUDES_API void WriteMap(GrammarSymbolWordMap f, bool changed = false);
  

} // namespace faudes

#endif
