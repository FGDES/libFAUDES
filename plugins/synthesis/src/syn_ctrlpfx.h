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
  StateSetOperator(void) : ExtType() {
    FD_WARN("StateSetOperator()");
  };

  /** disable copy construct */
  StateSetOperator(const StateSetOperator&)= delete;

  /** destruct */
  ~StateSetOperator(void) {};
    
  /**
   * Domain
   *
   * Some operations need to take complements and thus refer to the full state set.
   * Reimplement this function if you need that extra functionality. See
   * faudes::CtrlPfxOperator for a derived class.
   *
   * @return
   *   Full state set.
   **/
  virtual const StateSet&  Domain(void) const {
    static StateSet empty;
    return empty;
  }

  /**
   * Evaluate opertor on arguments
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
  virtual void Evaluate(StateSetVector& rArgs, StateSet& rRes) const =0;

  /** signature, i.e., the number of arguments we expect */
  StateSetVector::Position ArgCount(void) const { return mArgCount;};

  /** signature, i.e., argument names (cosmetic) */
  const std::string& ArgName(StateSetVector::Position pos) const {
    static std::string range="?";
    if(pos>= mArgNames.size()) return range;
    return mArgNames.at(pos);
  }
  

protected:

 /** signature */
 StateSetVector::Position mArgCount;
  
 /** support cosmetic  siganture */
 std::vector<std::string> mArgNames;

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
  virtual void Evaluate(StateSetVector& rArgs, StateSet& rRes) const;
  
private:

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
  virtual void Evaluate(StateSetVector& rArgs, StateSet& rRes) const;
  
private:
  //* the base operator to iteret on
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
  virtual void Evaluate(StateSetVector& rArgs, StateSet& rRes) const;
  
private:
  //* the base operator to iteret on
  const StateSetOperator& mrOp;
};

  

  


} // namespace faudes

#endif 


