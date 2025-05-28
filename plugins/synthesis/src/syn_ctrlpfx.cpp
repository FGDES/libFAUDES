/** @file syn_ctrlpfx.cpp Controllability prefix */

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

 

#include "syn_ctrlpfx.h"

namespace faudes {

/*  
*********************************************************************

Implementation of operator on state sets: virtual base

*********************************************************************    
*/

// ge dummy domain  
const StateSet&  StateSetOperator::Domain(void) const {
  static StateSet empty;
  return empty;
}

// API wrapper, multiple arguments  
void StateSetOperator::Evaluate(StateSetVector& rArgs, StateSet& rRes) const {
  if(rArgs.Size()!=mArgCount) {
    std::stringstream errstr;
    errstr << "signature mismatch: expected arguments #" << mArgCount <<
      " provided argumenst #" << rArgs.Size();
    throw Exception("StateSetOperator::Evaluate", errstr.str(), 80);
  }
  DoEvaluate(rArgs,rRes); 
}

// API wrapper, single argument  
void StateSetOperator::Evaluate(StateSet& rArg, StateSet& rRes) const {
  if(mArgCount!=1) {
    std::stringstream errstr;
    errstr << "signature mismatch: expected arguments #" << mArgCount <<
      " provided argumenst #1";
    throw Exception("StateSetOperator::Evaluate", errstr.str(), 80);
  }
  StateSetVector args;
  args.PushBack(&rArg);
  DoEvaluate(args,rRes); 
}

// API wrapper, no arguments 
void StateSetOperator::Evaluate(StateSet& rRes) const {
  if(mArgCount!=0) {
    std::stringstream errstr;
    errstr << "signature mismatch: expected arguments #" << mArgCount <<
      " provided argumenst #0";
    throw Exception("StateSetOperator::Evaluate", errstr.str(), 80);
  }
  static StateSetVector args;
  DoEvaluate(args,rRes); 
}

// signature, i.e., the number of arguments */
StateSetVector::Position StateSetOperator::ArgCount(void) const {
  return mArgCount;
};

// signature, i.e., argument names (cosmetic) */
const std::string& StateSetOperator::ArgName(StateSetVector::Position pos) const {
  static std::string range="?";
  if(pos>= mArgNames.size()) return range;
  return mArgNames.at(pos);
}

// stats  
std::string StateSetOperator::ArgStatistics(const StateSetVector& rArgs) const {
  std::stringstream res;
  for(StateSetVector::Position pos=0; pos< rArgs.Size(); ++pos) {
    res << " " << ArgName(pos) << " #" << rArgs.At(pos).Size();
  }
  return res.str();
}

/*  
*********************************************************************

Implementation of operator on state sets: controllability prefix

*********************************************************************    
*/

// constructor  
CtrlPfxOperator::CtrlPfxOperator(const vGenerator& rGen, const EventSet& rSigmaUc) :
  StateSetOperator(),
  mrGen(rGen),
  mrSigmaUc(rSigmaUc),
  mrTransRel(rGen.TransRel()),
  mRevTransRel(rGen.TransRel())
{
  FD_DF("CtrlPfxOperator(): instantiated from " << rGen.Name());
  Name("cpx_op([Y,X])");
  mArgNames= std::vector<std::string>{"Y","X"};
  mArgCount=2;
  (void) mrGen;
};

// domain
const StateSet& CtrlPfxOperator::Domain(void) const {
  return mrGen.States();
}  

// evaluation
void CtrlPfxOperator::DoEvaluate(StateSetVector& rArgs, StateSet& rRes) const {
  //FD_WARN("CtrlPfxOperator::DoEvaluate(): " << Name());
  // prepare result
  rRes.Clear();
  // actual implementation comes here, aka
  // eval([Y,X]) =
  //   (pre_exisential(X) union marked_states) intersectted with  pre_universal(Y)
  StateSet& Y=rArgs.At(0);
  StateSet& X=rArgs.At(1);
  //FD_WARN("CtrlPfxOperator::DoEvaluate(): Y " << mrGen.StateSetToString(Y));
  //FD_WARN("CtrlPfxOperator::DoEvaluate(): X " << mrGen.StateSetToString(X));
  StateSet lhs;
  lhs.Assign(mRevTransRel.PredecessorStates(X));
  lhs.InsertSet(mrGen.MarkedStates());
  StateSet rhs;
  StateSet Ycmp= mrGen.States() - Y;
  rhs.Assign(mrGen.States());
  rhs.EraseSet(mRevTransRel.PredecessorStates(Ycmp,mrSigmaUc) );
  rRes=lhs * rhs;
  //FD_WARN("CtrlPfxOperator::DoEvaluate(): R " << mrGen.StateSetToString(rRes));
};
  

/*  
*********************************************************************

Implementation of fixpoint iterations: mu

*********************************************************************    
*/

MuIteration::MuIteration(const StateSetOperator& rOp) :
  StateSetOperator(),
  mrOp(rOp)
{
  FD_WARN("MuIteration(): instantiated to run on " << rOp.Name());
  if(rOp.ArgCount()<1) {
    // throw
  }
  Name("mu " + rOp.ArgName(rOp.ArgCount()-1) + " . " + rOp.Name());
  mArgCount=rOp.ArgCount()-1;
  for(StateSetVector::Position pos=0; pos<rOp.ArgCount(); ++pos)
    mArgNames.push_back(rOp.ArgName(pos));
  FD_WARN("MuIteration(): name " << Name());
};
    

// inherit domain
const StateSet& MuIteration::Domain(void) const {
  return mrOp.Domain();
}  

// evaluation
void MuIteration::DoEvaluate(StateSetVector& rArgs, StateSet& rRes) const {
  FD_WARN("MuIteration::DoEvaluate(): " << Name());
  // prepare result
  rRes.Clear();
  // actual implementation comes here
  StateSetVector xargs;
  xargs.AssignByReference(rArgs);  
  xargs.PushBack(&rRes);
  StateSet R;
  while(true) {
    Idx xsz=rRes.Size();
    mrOp.Evaluate(xargs,R);
    FD_WARN("MuIteration::DoEvaluate(): " << ArgStatistics(xargs) << " -> #" << R.Size());
    rRes.InsertSet(R);
    if(rRes.Size()==xsz) break;  
  }
  
};



/*  
*********************************************************************

Implementation of fixpoint iterations: nu

*********************************************************************    
*/

NuIteration::NuIteration(const StateSetOperator& rOp) :
  StateSetOperator(),
  mrOp(rOp)
{
  FD_WARN("NuIteration(): instantiated to run on " << rOp.Name());
  if(rOp.ArgCount()<1) {
    // throw
  }
  Name("nu " + rOp.ArgName(rOp.ArgCount()-1) + " . " + rOp.Name());
  mArgCount=rOp.ArgCount()-1;
  for(StateSetVector::Position pos=0; pos<rOp.ArgCount(); ++pos)
    mArgNames.push_back(rOp.ArgName(pos));
  FD_WARN("NuIteration(): name " << Name());
};
    

// inherit domain
const StateSet& NuIteration::Domain(void) const {
  return mrOp.Domain();
}  

// evaluation
void NuIteration::DoEvaluate(StateSetVector& rArgs, StateSet& rRes) const {
  FD_WARN("NuIteration::DoEvaluate(): " << Name());
  // prepare result
  rRes.Clear();
  // actual implementation comes here
  StateSetVector xargs;
  xargs.AssignByReference(rArgs);  
  xargs.PushBack(&rRes);
  rRes=Domain();
  StateSet R;
  while(true) {
    Idx xsz=rRes.Size();
    mrOp.Evaluate(xargs,R);
    FD_WARN("NuIteration::DoEvaluate(): " << ArgStatistics(xargs) << " -> #" << R.Size());
    rRes.RestrictSet(R);
    if(rRes.Size()==xsz) break;  
  }
  
};

} // namespace faudes


