/** @file pd_alg_sub.h  algorithm subfunctions*/

/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

 Copyright (C) 2013/14 Ramon Barakat, Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

 */

#ifndef FAUDES_PD_ALG_SUB_H
#define FAUDES_PD_ALG_SUB_H

#include "corefaudes.h"
#include "pd_include.h"


namespace faudes {

//Convenience definition for a vector which contains each transformation from PDA to SPDA by function SPDA
typedef std::vector<std::vector<std::pair<Idx,MergeTransition> > > TransformationHistoryVec;

/**
 * Associates each state of the generator with an annotation. Each state's
 * merge attribute will be set to a MergeStateAnnotation. The MergeStateAnnotation
 * will contain the annotation.
 *
 * (previously called RenQ)
 *
 * @param word
 *    word with which the states will be annotated
 * @param pd
 *    reference to pushdown automaton to be annotated
 * @return
 *    copy of the parameter automaton with annotated states
 */
extern FAUDES_API PushdownGenerator AnnotatePdStates(const std::string word,
		const PushdownGenerator& pd);

/**
 * Adds an annotation to each stack symbol of the generator. Each stack symbol will
 * be copied and added to to generator with the annotation preceding the original
 * name.
 *
 * (previously called RenG)
 *
 * @param word
 *    word with which the stack symbols will be annotated
 * @param pd
 *    reference to pushdown automaton to be annotated
 *
 * @return
 *    copy of the parameter automaton with annotated states
 */
extern FAUDES_API PushdownGenerator RenameStackSymbold(const std::string word,
		const PushdownGenerator& pd);

/**
 * Make generator trim
 *
 * This function removes all inaccessible states,
 * useless transitions and unused stack symbols,
 * with regard to the top n elements of the stacks.
 * Useless transitions are those outgoing transitions, that requiring stack-top never fulfills.
 * The function also removes states, that do not reach a marked state (with regard to the top n elements)
 * by calling vGenerator::trim after removing foregoing transitions.
 *
 * Note: For n = 0 the pushdown generator will call vGenerator::trim,
 * therefore no useless transitions or unused stack symbols will be removed.
 *
 * @param pd
 *    reference to pushdown automaton to be trim
 * @para n
 *    lookahead
 * @param debug
 * 	  Set true to debug
 *
 * @return
 *   True if resulting generator contains at least one initial state and at least one marked state.
 */
extern FAUDES_API bool PushdownTrim(PushdownGenerator& rPd, const uint n, bool debug = false);

/**
 * Fill map. For each state entered, which states are reachable (directly or transitively)
 * with regard to the top n elements of the stacks.
 *
 * @param rPd
 * 		reference to pushdown generator
 * @param n
 * 		lookahead
 * @param rMap
 * 		reference to resulting map
 *
 * @return
 */
extern FAUDES_API void ReachableStateMap(const PushdownGenerator& rPd, std::map<Idx, StateSet>& rMap);

/**
 * (previously called Rpp)
 * Restricts a pushdown generator to a simple pushdown generator SPDA with transitions that
 * are either
 * read (p,a,lambda,lambda,q),
 * pop (p,lambda,x,lambda,q) or
 * push (p,lambda,x,yx,q).
 *
 * The parameter pTransformHistory is necessary to remember the transformations of each iteration.
 * To  rebuild the former generator see RebuildFromSPDA()
 *
 * Note: To rebuild the former generator, the idx's of the states must not have been changed.
 *
 * @param pd
 *    the generator, which to convert into SPDA
 *
 * @param pTransformHistory
 * 	  pointer to vector to remember the transformations for each iteration
 * 	  By default no pointer is needed (NULL)
 *
 * @return
 *    simple pushdown generator
 */
extern FAUDES_API PushdownGenerator SPDA(const PushdownGenerator& pd,
		TransformationHistoryVec* pTransformHistory = NULL);

/**
 * Input must have passed SPDA() !
 * Rebuild a pushdown generator which passed SPDA as far as possible.
 * This mean, if a transition t of the original generator was split into two or more transitions t1...tn of the SPDA,
 * this transition t only will be rebuild, iff t1...tn still exists.
 * After rebuilding, inaccessible states will be removed.
 *
 * Note: Between the call of SPDA() and RebuildFromSPDA()
 * the indices of states and events as well as stack symbol names must not have be changed.
 *
 * @param rSPDA
 * 		reference to the generator which passed SPDA
 * @param rTransformHistory
 * 		reference to vector of each transformation per iteration
 *
 * @return
 * 		pushdown generator
 */
extern FAUDES_API PushdownGenerator RebuildFromSPDA(const PushdownGenerator& rSPDA,
		const TransformationHistoryVec& rTransformHistory);

/**
 * Input must have passed SPDA() !
 * Compares the given pushdown generator with the given grammar and removes all transition,
 * which cannot be find in productions of the grammar.
 * After that, inaccessible states will be removed.
 *
 * @param rGr
 * 		reference to grammar
 * @param pd
 * 		reference to generator
 *
 * @return
 * 		remaining generator
 */
extern FAUDES_API PushdownGenerator RemainingPd(const Grammar& rGr, const PushdownGenerator& pd);

/**
 * Input must have passed SPDA() !
 * Remodels the generator to prevent double (or multiple) acceptance of the same
 * input string.
 *
 * @param pd
 *    the generator
 * @return
 *    remodeled generator without double acceptance
 *
 * @exception Exception
 * 	  if a transition is neither read nor pop nor push transition
 */
extern FAUDES_API PushdownGenerator Nda(const PushdownGenerator& pd);

/**
 * Input must have passed SPDA() and Nda() !
 * Transform a simple pushdown generator into a context free grammar.
 * To reduce the number of to create productions, the generator will be trim
 * (by calling PushdownGeneratorTrim )
 *
 * @param pd
 *    reference to pushdown generator
 * @param n
 *    the lookahead for calling PushdownGeneratorTrim
 * @param ignorReducible
 *    by set ignorReducible to false, only productions are generated which are coaccessible.
 * @param debug
 * 	  Set true, to print out single steps of the function
 *
 * @return
 *    the grammar
 *
 * @exception Exception
 * 	  if a transition is neither read nor pop nor push transition
 */
extern FAUDES_API Grammar Sp2Lr(const PushdownGenerator& rPd, uint n=0, bool ignorReducible = false, 	bool debug = false);

/**
 * Extract all symbols from the word that are in the symbol set
 *
 * @param symbolSet
 *    the  symbol set
 * @param w
 *    the word
 * @return
 *    set of found symbols
 */
extern FAUDES_API std::set<Nonterminal> Filter(const std::set<Nonterminal>& symbolSet, const GrammarSymbolVector& w);

/**
 * Find all nonterminals that are eliminable in one step, assuming that a certain
 * set of nonterminals has already been eliminated.
 *
 * @param gr
 *    the grammar
 * @param ntSet
 *    the set of already eliminated nonterminals
 * @return
 *    eliminable symbols including the already eliminated ones
 */
extern FAUDES_API std::set<Nonterminal> Rnpp1(const Grammar& gr, const std::set<Nonterminal>& ntSet);

/**
 * Find all nonterminals that are eliminable in as many steps as needed, assuming
 * that a certain set of nonterminals has already been eliminated.
 *
 * @param gr
 *    the grammar
 * @param ntSet
 *    the set of already eliminated nonterminals
 * @return
 *    eliminable symbols including the already eliminated ones
 */
std::set<Nonterminal> Rnppl(const Grammar& gr, const std::set<Nonterminal>& ntSet);

/**
 * Remove all productions from a grammar that are nonproductive, i. e. those
 * containing nonterminals that are not eliminable.
 *
 * @param gr
 * 	reference to grammar
 * @return
 *    grammar with only productive productions
 */
extern FAUDES_API Grammar Rnpp(const Grammar& gr);

/**
 * Remove all unreachable productions and nonterminals from the grammar
 *
 * @param gr
 *    grammar with productions to be removed
 *
 * @return
 *    grammar with only reachable productions
 */
extern FAUDES_API Grammar Rup(const Grammar& gr);


/**
 * (previously called Rep2)
 * Removes transitions popping more than one stack symbol.
 * Inserts new transitions instead that only pop one stack symbol.
 *
 * @param pd
 *    the generator, which must NOT contain lambda pops
 * @return
 *    generator with altered transitions
 *
 * @exception Exception
 * 	  if a lambda pop transition exists
 */
extern FAUDES_API PushdownGenerator RemoveMultPop(const PushdownGenerator& pd);

/**
 * Getting the number of incoming transitions of a state
 *
 * @param trg
 * 		idx of target state
 * @param rPd
 * 		reference to pushdown generator
 *
 * @return
 * 		number of incoming transitions
 */
extern FAUDES_API uint InTransitionSize(Idx trg,const PushdownGenerator& rPd);


/**
 * Merge adjacent transitions until no more mergers are possible.
 * This will reduce the number of transitions and states.
 * The marked language does not change.
 *
 * Technical detail:
 * 	Let qb be the state between transitions t1 and t2.
 * 	t1 and t2 will be merged, if
 *  	* qb is no marked state
 *  	* t1 is the only transition, that leads to qb
 *  	* for all transitions t' from qp:
 *  		- t' is no self-loop transition
 *  		- if t1->Ev != lambda then t'->Ev = lambda
 *  	* the from t2 requiring stack-top is pushed by t1
 *
 * @param rPd
 * 		reference to pushdown generator
 */
extern FAUDES_API void MergeAdjacentTransitions(PushdownGenerator& rPd);

/**
 * Merge transitions from state q with adjacent transitions
 * without changing the marked language. ( see PossibleAdjacentMerges() )
 *
 * @param q
 * 		idx of state q
 * @param rPd
 * 		reference to pushdown generator
 * @param next
 * 		reference to set of states to be checked next.
 * 		If no transitions were merged, all successor states of q will be added to next.
 * 		If transitions were merged, state q will be added to next.
 * @param done
 * 		reference to set of states which already checked
 *		If no transitions was merged, state q will be added to done.
 * @return
 * 		true, if a merger was made. False if not
 */
extern FAUDES_API bool CombinedTransitions(Idx q,PushdownGenerator& rPd, /*std::set<Idx>&*/ StateSet& next,/*std::set<Idx>&*/ StateSet& done);

/*
 * Ascertain on transitions (from state q) adjacent transitions which are possibly mergeable
 * without changing the marked language.
 *
 *Technical details:
 * Let qb be the state between transitions t1 and t2.
 * t1 and t2 are possible mergeable if:
 *  	* qb is no marked state
 *  	* t1 is the only transition, that leads to qb
 *  	* for all transitions t' from qp:
 *  		- t' is no self-loop transition
 *  		- if t1->Ev != lambda then t'->Ev = lambda
 *
 * @param q
 * 		idx of state q
 * @param pd
 * 		reference to pushdown generator
 *
 * @return
 * 		set of pairs of possible mergeable transitions
 */
extern FAUDES_API std::set<std::pair <Transition,Transition> > PossibleAdjacentMerges (Idx q, const PushdownGenerator& pd);


/**
 * (previously called Rep0)
 * Removes all transitions popping lambda.
 * Expect the generator to be deterministic
 *
 * Note:
 * By the new treatment of Transitions like (p,a,pop,push,q) with a != lambda and pop = push,
 * there is no need to clear old init state and create a new init state which
 * derived from pd.InitState()
 *
 * @param pd
 *    pushdown automaton
 * @return
 *    pushdown automaton without lambda popping edges
 */
extern FAUDES_API PushdownGenerator RemoveLambdaPop(const PushdownGenerator& pd);

} // namespace faudes

#endif
