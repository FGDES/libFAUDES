/** @file pd_alg_lrp.h  functions related to parsers*/


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013/14  Stefan Jacobi, Ramon Barakat, Sven Schneider, Anne-Kathrin Hess

*/

#ifndef FAUDES_PD_ALG_LRP_H
#define FAUDES_PD_ALG_LRP_H

#include "corefaudes.h"
#include "pd_include.h"
 
namespace faudes {

/**
 * Find the successor states of a state for a given input symbol.
 * 
 * @param gotoGen
 *      the generator on which to work
 * @param state
 *      the state of which to find the successor state
 * @param symbol
 *      the input symbol
 * @return
 *      A set with the successor states. The set size is 1 if a successor state
 * is found. Set size is 0 if no successor state is found.
 */
extern FAUDES_API std::set<Idx> GeneratorGoto(const GotoGenerator& gotoGen, Idx state, const GrammarSymbolPtr& symbol);

/**
 * Find the successor state sequence of a state for a given input word. If the word
 * cannot be completely matched to a state sequence, the state sequence will be
 * partially matched (i. e., it will be as long as the part of the word that can be
 * matched).
 * 
 * @param gotoGen
 *      the generator on which to work
 * @param state
 *      the state of which to find the successor state
 * @param word
 *      the input word
 * @return
 *      A set with the successor states. The set size will be anywhere between 0 and 
 * the length of the word.
 */
extern FAUDES_API std::vector<Idx> GeneratorGotoSeq(const GotoGenerator& gotoGen, Idx startState, const GrammarSymbolVector& word);

/**
 * construct the parsing rules for shifting from a grammar and its LR(k) machine
 * 
 * @param gr
 *      the grammar on which to work
 * @param augGr
 *      augmented version of the grammar on which to work
 * @param gotoGen
 *      LR(k) machine of the grammar
 * @param k
 *      a natural number that denotes the kind of parser the function is working
 * on (LR(k) parser) (will currently only work for LR(1))
 * @return
 *      set of shift actions
 */
extern FAUDES_API std::set<Lr1ParserAction> LrpShiftRules(const Grammar& gr, const Grammar& augGr, const GotoGenerator& gotoGen, uint k);

/**
 * construct the parsing rules for reducing from a grammar and its LR(k) machine
 * 
 * @param gr
 *      the grammar on which to work
 * @param augGr
 *      augmented version of the grammar on which to work
 * @param gotoGen
 *      LR(k) machine of the grammar
 * @param k
 *      a natural number that denotes the kind of parser the function is working
 * on (LR(k) parser) (will currently only work for LR(1))
 * @return
 *      set of reduce actions
 */
extern FAUDES_API std::set<Lr1ParserAction> LrpReduceRules(const Grammar& gr, const Grammar& augGr, const GotoGenerator& gotoGen, uint k);

/**
 * construct all parsing rules from a grammar and its LR(k) machine
 * 
 * @param gr
 *      the grammar on which to work
 * @param augGr
 *      augmented version of the grammar on which to work
 * @param gotoGen
 *      LR(k) machine of the grammar
 * @param k
 *      a natural number that denotes the kind of parser the function is working
 * on (LR(k) parser) (will currently only work for LR(1))
 * @return
 *      set of parsing rules
 */
extern FAUDES_API std::set<Lr1ParserAction> LrpRules(const Grammar& gr, const Grammar& augGr, const GotoGenerator& gotoGen, uint k);

/**
 * construct an LR(k) parser from a grammar and its LR(k) machine
 * 
 * @param gr
 *      the grammar on which to work
 * @param augGr
 *      augmented version of the grammar on which to work
 * @param gotoGen
 *      LR(k) machine of the grammar
 * @param k
 *      a natural number that denotes the kind of parser the function is working
 * on (LR(k) parser) (will currently only work for LR(1))
 * @param augSymbol
 *      the terminal with which the grammar was augmented to create the augmented
 * grammar
 * @return
 *      set of parsing rules
 */
extern FAUDES_API Lr1Parser Lrp(const Grammar& gr, const Grammar& augGr, const GotoGenerator& gotoGen, uint k, const Terminal& augSymbol);

/**
 * Detach augmented symbole from Lr(1)-parser
 *
 * @param rParser
 *   reference to Lr(1)-parser
 *
 * @return
 */
extern FAUDES_API void DetachAugSymbol(Lr1Parser& rParser);

/**
 *
 * Transform action of Lr(1)-parser as follows
 * actions of the form (q,a,p,a) to ((q,lambda),a,(p,a),lambda), ((q,a),lambda,(p,a),lambda)
 * actions of the form (q,a,p,lambda) to ((q,lambda),a,(p,lambda),lambda)	, ((q,a),lambda,(p,lambda),lambda)
 *
 * @param rParser
 *   reference to Lr(1)-parser
 *
 * @return
 *   Lr(1)-parser with transformed actions
 */
extern FAUDES_API Lr1Parser TransformParserAction(const Lr1Parser& rParser);

/*
 * Transform a Lr(1)-parser to a pushdown generator.
 *
 * @param rParser
 * 		reference to LR1-parser
 *
 * @return
 * 		pushdown generator with same marked language as given parser
 */
extern FAUDES_API PushdownGenerator LrParser2EPDA(const Lr1Parser& rParser);



} // namespace faudes

#endif
