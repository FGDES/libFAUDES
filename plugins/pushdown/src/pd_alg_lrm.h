/** @file pd_alg_lrm.h  functions related to LR machines*/


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/


#ifndef FAUDES_PD_ALG_LRM_H
#define FAUDES_PD_ALG_LRM_H

#include "corefaudes.h"
#include "pd_include.h"


namespace faudes {
  
  /**
   * Obtain the immediate descendants of a configuration if the dot were pushed
   * one nonterminal further.
   * 
   * @param gr
   *    the grammar on which to work
   * @param k
   *    a natural number that denotes the kind of parser the function is working
   * on (LR(k) parser) (will currently only work for LR(1))
   * @param config
   *    the configuration whose descendants are to be obtained
   * @return
   *    set of all descendant parser configurations
   */
  extern FAUDES_API std::set<Lr1Configuration> Desc11(const Grammar& gr, uint k, const Lr1Configuration& config);
  
  /**
   * Obtain the immediate descendants of configurations if the dot were pushed
   * one nonterminal further.
   * 
   * @param gr
   *    the grammar on which to work
   * @param k
   *    a natural number that denotes the kind of parser the function is working
   * on (LR(k) parser) (will currently only work for LR(1))
   * @param configs
   *    a set of configurations whose descendants are to be obtained
   * @return
   *    set of all immediate descendant parser configurations
   */
  extern FAUDES_API std::set<Lr1Configuration> Desc1(const Grammar& gr, uint k, const std::set<Lr1Configuration>& configs);
  
  /**
   * Obtain all descendants of a configuration set.
   * 
   * @param gr
   *    the grammar on which to work
   * @param k
   *    a natural number that denotes the kind of parser the function is working
   * on (LR(k) parser) (will currently only work for LR(1))
   * @param config
   *    a set of configurations whose descendants are to be obtained
   * @return
   *    set of all descendant parser configurations
   */
  extern FAUDES_API std::set<Lr1Configuration> Desc(const Grammar& gr, uint k, const std::set<Lr1Configuration>& config);
  
  /**
   * Try to shift the dot in a configuration over a specified symbol.
   * 
   * @param config
   *    the configuration
   * @param symbol
   *    the symbol over which to shift
   * @return
   *    set of size one that contains the new configuration or an empty set if the 
   * specified symbol was not found directly after the dot
   */
  extern FAUDES_API std::set<Lr1Configuration> PassesX(const Lr1Configuration& config, const GrammarSymbolPtr& symbol);
  
  /**
   * Try to shift the dots in a configuration set over a specified symbol
   * 
   * @param configs
   *    the configurations
   * @param symbol
   *    the symbol over which to shift
   * @return
   *    set that contains all shifted configurations
   */
  extern FAUDES_API std::set<Lr1Configuration> Basis(const std::set<Lr1Configuration> configs, const GrammarSymbolPtr& symbol);
  
  /**
   * Try to shift the dots in a configurations set and obtain the shifted
   * configuration set's descendants.
   * 
   * @param gr
   *    the grammar on which to work
   * @param k
   *    a natural number that denotes the kind of parser the function is working
   * on (LR(k) parser) (will currently only work for LR(1))
   * @param configs
   *    the configurations
   * @param symbol
   *    the symbol over which to shift
   * @return
   *    set containing the shifted configuration's descendant configurations
   */
  extern FAUDES_API std::set<Lr1Configuration> GoTo(const Grammar& gr, uint k, const std::set<Lr1Configuration> configs, const GrammarSymbolPtr& symbol);
  
  /**
   * Generate outgoing transitions for an LR(k) machine for a given configuration set.
   * 
   * @param gr
   *    the grammar on which to work
   * @param k
   *     a natural number that denotes the kind of parser the function is working
   * on (LR(k) parser) (will currently only work for LR(1))
   * @param configs
   *    the configuration set from which to generate the transitions
   * @return
   *    outgoing transitions from the given configuration set as a map mapping 
   * the original configuration set and a grammar symbol to another configuration set
   */
  extern FAUDES_API LrmTransitionMap Lrm1(const Grammar& gr, uint k, const Lr1ConfigurationSetSet& configSetSet);
  
  /**
   * Recursively generate all transitions and states for an LR(k) machine.
   * @param gr
   *    the grammar on which to work
   * @param k
   *     a natural number that denotes the kind of parser the function is working
   * on (LR(k) parser) (will currently only work for LR(1))
   * @param transitions
   *    the transitions that are already found
   * @param states
   *    the states that are already found
   * @param examineStates
   *    the states that need to be examined for outgoing transitions
   * @return
   *    a pair with the transition map and all states
   */
  extern FAUDES_API std::pair<LrmTransitionMap,Lr1ConfigurationSetSet> LrmLoop(const Grammar& gr, uint k, const LrmTransitionMap& transitions, const Lr1ConfigurationSetSet& states, Lr1ConfigurationSetSet examineStates);
  
  /**
   * Determine the initial parser configurations for a grammar.
   * 
   * @param gr
   *    the grammar
   * @return
   *    a set with the initial configurations
   */
  extern FAUDES_API std::set<Lr1Configuration> DescInitial(const Grammar& gr);
  
  /**
   * Determine the descendants of the initial parser configurations.
   * 
   * @param gr
   *    the grammar
   * @param k
   *    a natural number that denotes the kind of parser the function is working
   * on (LR(k) parser) (will currently only work for LR(1))
   * @return
   *    a set with the descendants of the initial configurations
   */
  std::set<Lr1Configuration> ValidEmpty(const Grammar& gr, uint k);
  
  /**
   * Generate an LR(k) machine for a grammar.
   * 
   * @param gr
   *    the grammar
   * @param k
   *    a natural number that denotes the kind of parser the function is working
   * on (LR(k) parser) (will currently only work for LR(1))
   * @return
   *    the LR(k) machine
   */
  extern FAUDES_API GotoGenerator Lrm(const Grammar& gr, uint k);
  
  /**
   * Augments the grammar with a nonterminal S and  a terminal $ such that a new grammar
   * production  will be inserted. S is the new start symbol of the grammar
   * and S' is the old start symbol of the grammar
   * 
   * @param gr
   *    the grammar to be augmented
   * @param startSymbol
   *    the new start symbol S. S must not exist in the grammar
   * @param augSymbol
   *    the $ symbol which augments the grammar. $ must not exist in the grammar
   * @return
   *    augmented grammar with a new production S -> $ S' $
   */
  extern FAUDES_API Grammar Aug(const Grammar& gr, const Nonterminal& startSymbol, const Terminal& augSymbol);
  

} // namespace faudes

#endif
