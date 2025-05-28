/** @file syn_ctrlpfx.h Controllability prefix */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2025  Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

 
#ifndef FAUDES_SYN_CTRLPFX_H
#define FAUDES_SYN_CTRLPFX_H

#include "corefaudes.h"

namespace faudes {
    

/**
 * Operator on state sets
 *
 * Light weight base class to be used in fixpoint iterations. See
 * faudes::CtrlPfxOperator for an example of a derived class.
 *
 * @ingroup SynthesisPlugIn
 */
class FAUDES_API StateSetOperator : public ExtType {

public:

  /** construct */
  StateSetOperator(void) : ExtType(){};

  /** disable copy construct */
  StateSetOperator(const StateSetOperator&)= delete;

  /** destruct */
  ~StateSetOperator(void) {};
    
  /**
   * Domain
   *
   * Some operations need to take complements and thus refer to the full state set.
   * The base class returns the empty set as a dummy. Reimplement this function
   * if you need that extra functionality. See faudes::CtrlPfxOperator for a derived class. 
   *
   * @return
   *   Full state set.
   **/
  virtual const StateSet&  Domain(void) const;

  /**
   * Evaluate opertor on arguments
   *
   * This is a wrapper for the protected method DoEvaluate. The latter
   * should be re-implemented by derived classes to encode the actual operator.
   *
   * @param rAggs
   *   State-set valued arguments the operator performs on
   * @parem rRes
   *   Resulting state set
   **/
  void Evaluate(StateSetVector& rArgs, StateSet& rRes) const;

  /**
   * Evaluate opertor on arguments
   *
   * This is a convenience wrapper for the protected method DoEvaluate for
   * operator with orny one argument.
   *
   * @param rArg
   *   State-set valued argument
   * @parem rRes
   *   Resulting state set
   **/
  void Evaluate(StateSet& rArg, StateSet& rRes) const;

  /**
   * Evaluate opertor on arguments
   *
   * This is a convenience wrapper for the protected method DoEvaluate for
   * operators with no arguments.
   *
   * @parem rRes
   *   Resulting state set
   **/
  void Evaluate(StateSet& rRes) const;

  /** signature, i.e., the number of arguments we expect */
  StateSetVector::Position ArgCount(void) const;

  /** signature, i.e., argument names (cosmetic) */
  const std::string& ArgName(StateSetVector::Position pos) const;
  
  /** argument stats (debugging/development) */
  std::string ArgStatistics(const StateSetVector& rArgs) const;

protected:

 /** signature */
 StateSetVector::Position mArgCount;
  
 /** support cosmetic  siganture */
 std::vector<std::string> mArgNames;

  /**
   * Evaluate opertor on arguments (protected virtual)
   *
   * The arguments are given as a vector of state sets. For fixpoint iterations, the last
   * entry in the vector becomes the iteration variable, while the remaining entries are constant
   * parameters. Re-implement this function for the oparater you want to iterate on. See
   * faudes::CtrlPfxOperator for a derived class.
   *
   * @param rAggs
   *   State-set valued arguments the operator performs on
   * @parem rRes
   *   Resulting state set
   **/
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) const =0;

};

/**
 * Operator used for the computation of the controllability prefix.
 *
 * @ingroup SynthesisPlugIn
 */
class FAUDES_API CtrlPfxOperator : public StateSetOperator {

public:

  /** construct */
  CtrlPfxOperator(const vGenerator& rGen, const EventSet& rSigmaUc);

  /** destruct */
  ~CtrlPfxOperator() {};
  
  /**
   * Domain
   *
   * Report the universe of all states, as given by the generator on construction.
   *
   * @return
   *   Full state set.
   **/
  virtual const StateSet&  Domain(void) const;

protected:

  /**
   * Evaluate opertor on arguments
   *
   * For the purpose of the controllability prefix, we evalate to
   *
   * eval([Y,X]) =
   *  (pre_exitential(X) union marked_states) intersected with  pre_universal_ctrl(Y)
   *
   * Note: the order of the argument vector matters. The last entry is the iterate
   * in the inner most iteration.
   *
   * @param rAggs
   *   Argument [Y,X] in that order
   * @parem rRes
   *   Resulting state set
   **/
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) const;
  

  /** set up context references */
  const vGenerator& mrGen;

  /** set up conbtext references */
  const EventSet& mrSigmaUc;

  /** set up context references */
  const TransSet& mrTransRel;

  /** have a reverse transition relation*/
  TransSetX2EvX1  mRevTransRel;

};  

/**
 * Mu-iterations on state sets.
 *
 * Given an opertor on state stes, this class facilitates nested
 * fixpoint iterations as in the mu-calculus. In tis specific class,
 * we implement the mu-iteration, i.e., we seek for the smallest fixpoint.
 *
 * The implementation is meant for a simple API, we do not care about
 * performance at this stage.
 *
 * @ingroup SynthesisPlugIn
 */
class FAUDES_API MuIteration : public StateSetOperator {

  public:

  /**
   * Construct from operator
   *
   * The last entry of the operator argument becomes the variable we iterate in.
   * The preceeding entries are interpreted ans constant paramters in our scope.
   *
   * @param rOp
   *   Operator to iterate
   */
   MuIteration(const StateSetOperator& rOp);

   /** detsruct */
   ~MuIteration(void){};

  /**
   * Domain
   *
   * Report the universe of all states, as given by the operator on construction.
   *
   * @return
   *   Full state set.
   **/
  virtual const StateSet&  Domain(void) const;

protected:

  /**
   * Implement the mu-iteration to find the smallest fixpoint.
   *
   * Given the parameter vector rArgs, we append one additional state set
   * X for which we seek a fixpoint of Op(rArgs,X). Effectively we iterate
   *
   * X(0) = emptyset
   * X(i+1) = X(i) union Op(rArgs,X(i))
   *
   * until the fixpoint is achieved.
   *
   * @param rArgs
   *   Arguments the operator performs on
   * @parem rRes
   *   Resulting state set
   **/
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) const;
  
  /** the base operator to iterate with */
  const StateSetOperator& mrOp;
};


/**
 * Nu-iterations on state sets.
 *
 * Given an opertor on state stes, this class facilitates nested
 * fixpoint iterations as in the mu-calculus. In tis specific class,
 * we implement the nu-iteration, i.e., we seek for the greatest fixpoint.
 *
 * The implementation is meant for a simple API, we do not care about
 * performance at this stage.
 *
 * @ingroup SynthesisPlugIn
 */
class FAUDES_API NuIteration : public StateSetOperator {

  public:

  /**
   * Construct from operator
   *
   * The last entry of the operator argument becomes the variable we iterate in.
   * The preceeding entries are interpreted ans constant paramters in our scope.
   *
   * @param rOp
   *   Operator to iterate
   */
   NuIteration(const StateSetOperator& rOp);

  /** detsruct */
   ~NuIteration(void){};

  /**
   * Domain
   *
   * Report the universe of all states, as given by the operator on construction.
   *
   * @return
   *   Full state set.
   **/
  virtual const StateSet&  Domain(void) const;

  
protected:

  /**
   * Implement the nu-iteration to find the smallest fixpoint.
   *
   * Given the parameter vector rArgs, we append one additional state set
   * X for which we seek a fixpoint of Op(rArgs,X). Effectively we iterate
   *
   * X(0) = Domain
   * X(i+1) = X(i) intersected with  Op(rArgs,X(i))
   *
   * until the fixpoint is achieved.
   *
   * @param rArgs
   *   Arguments the operator performs on
   * @parem rRes
   *   Resulting state set
   **/
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) const;

  /** the base operator to iterate on */
  const StateSetOperator& mrOp;
};

  

  


} // namespace faudes

#endif 


