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

Implementation of operator on state sets

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
void CtrlPfxOperator::Evaluate(StateSetVector& rArgs, StateSet& rRes) const {
  FD_WARN("CtrlPfxOperator::Evaluate(): " << Name());
  if(rArgs.Size()!=mArgCount) {
    // throw
  }
  // prepare result
  rRes.Clear();
  // actual implementation comes here, aka
  // eval([Y,X]) =
  //   (pre_exisential(X) union marked_states) intersectted with  pre_universal(Y)
  StateSet& Y=rArgs.At(0);
  StateSet& X=rArgs.At(1);
  FD_WARN("CtrlPfxOperator::Evaluate(): Y " << Y.ToString());
  FD_WARN("CtrlPfxOperator::Evaluate(): X " << X.ToString());
  StateSet lhs;
  lhs.Assign(mRevTransRel.PredecessorStates(X));
  lhs.InsertSet(mrGen.MarkedStates());
  StateSet rhs;
  StateSet Ycmp= mrGen.States() - Y;
  rhs.Assign(mrGen.States());
  rhs.EraseSet(mRevTransRel.PredecessorStates(Ycmp,mrSigmaUc) );
  rRes=lhs * rhs;
  FD_WARN("CtrlPfxOperator::Evaluate(): R " << rRes.ToString());
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
  for(StateSetVector::Position pos=0; pos<mArgCount; ++pos)
    mArgNames.push_back(rOp.ArgName(pos));
  FD_WARN("MuIteration(): name " << Name());
};
    

// inherit domain
const StateSet& MuIteration::Domain(void) const {
  return mrOp.Domain();
}  

// evaluation
void MuIteration::Evaluate(StateSetVector& rArgs, StateSet& rRes) const {
  FD_WARN("MuIteration::Evaluate(): " << Name());
  // prepare result
  rRes.Clear();
  // actual implementation comes here
  StateSetVector xargs;
  xargs.AssignByReference(rArgs);  
  StateSet X;
  xargs.PushBack(&X);
  FD_WARN("MuIterationEvaluste(): args: " << xargs.ToString());
  StateSet R;
  while(true) {
    Idx xsz=X.Size();
    mrOp.Evaluate(xargs,R);
    X.InsertSet(R);
    FD_WARN("MuIteration::Evaluate(): iterate: " << X.ToString());
    if(X.Size()==xsz) break;  
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
  for(StateSetVector::Position pos=0; pos<mArgCount; ++pos)
    mArgNames.push_back(rOp.ArgName(pos));
  FD_WARN("NuIteration(): name " << Name());
};
    

// inherit domain
const StateSet& NuIteration::Domain(void) const {
  return mrOp.Domain();
}  

// evaluation
void NuIteration::Evaluate(StateSetVector& rArgs, StateSet& rRes) const {
  FD_WARN("NuIteration::Evaluate(): " << Name());
  // prepare result
  rRes.Clear();
  // actual implementation comes here
  StateSetVector xargs;
  xargs.AssignByReference(rArgs);  
  StateSet X;
  xargs.PushBack(&X);
  X=Domain();
  FD_WARN("NuIterationEvaluste(): args: " << xargs.ToString());
  StateSet R;
  while(true) {
    Idx xsz=X.Size();
    mrOp.Evaluate(xargs,R);
    X.RestrictSet(R);
    FD_WARN("NuIteration::Evaluate(): iterate: " << X.ToString());
    if(X.Size()==xsz) break;  
  }
  
};

} // namespace faudes


