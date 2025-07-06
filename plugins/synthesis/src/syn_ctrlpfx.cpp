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

// get dummy domain  
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
  const_cast<StateSetOperator*>(this)->DoEvaluate(rArgs,rRes); 
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
  const_cast<StateSetOperator*>(this)->DoEvaluate(args,rRes); 
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
  const_cast<StateSetOperator*>(this)->DoEvaluate(args,rRes); 
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

// indentation for nested iterations
const std::string& StateSetOperator::Indent(void) const {
  return mIndent;
}

// indentation for nested iterations (fake const)
void StateSetOperator::Indent(const std::string& indent) const {
  StateSetOperator* rwp = const_cast<StateSetOperator*>(this);
  rwp->mIndent = indent;
}


/*  
*********************************************************************

Implementation of operator on state sets: controllability prefix

*********************************************************************    
*/

// constructor  
CtrlPfxOperator::CtrlPfxOperator(const vGenerator& rGenerator, const EventSet& rSigmaCtrl) :
  StateSetOperator(),
  rGen(rGenerator),
  mSigmaCtrl(rSigmaCtrl),
  rTransRel(rGen.TransRel()),
  mRevTransRel(rGen.TransRel())
{
  FD_DF("CtrlPfxOperator(): instantiated from " << mrGen.Name());
  rGen.SWrite();
  Name("cpx_op([Y,X])");
  mArgNames= std::vector<std::string>{"Y","X"};
  mArgCount=2;
};

// domain
const StateSet& CtrlPfxOperator::Domain(void) const {
  return rGen.States();
}  

// evaluation
void CtrlPfxOperator::DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
  FD_DF("CtrlPfxOperator::DoEvaluate(): " << Name());
  // prepare result
  rRes.Clear();
  // have neat accessors
  StateSet& Y=rArgs.At(0);
  StateSet& X=rArgs.At(1);
  //FD_DF("CtrlPfxOperator::DoEvaluate(): Y " << rGen.StateSetToString(Y));
  //FD_DF("CtrlPfxOperator::DoEvaluate(): X " << rGen.StateSetToString(X));

  // actual implementation comes here, aka
  // eval([Y,X]) =
  //   (pre_exisntial(X) union marked_states) intersectted with  pre_universal(Y)

  /*
  // variant 1: by the book  
  StateSet lhs;
  lhs.Assign(mRevTransRel.PredecessorStates(X));
  lhs.InsertSet(rGen.MarkedStates());
  StateSet rhs;
  StateSet Ycmp= rGen.States() - Y;
  rhs.Assign(rGen.States());
  rhs.EraseSet(mRevTransRel.PredecessorStates(Ycmp,mSigmaUc) );
  rRes=lhs * rhs;
  */
  
  // variant 2: perhaps gain some performance
  rRes.Assign(mRevTransRel.PredecessorStates(X));
  rRes.InsertSet(rGen.MarkedStates());
  StateSet::Iterator sit=rRes.Begin();
  StateSet::Iterator sit_end=rRes.End();
  while(sit!=sit_end){
    TransSet::Iterator tit=rTransRel.Begin(*sit);
    TransSet::Iterator tit_end=rTransRel.End(*sit);
    for(;tit!=tit_end;++tit){
      if(mSigmaCtrl.Exists(tit->Ev)) continue;
      if(Y.Exists(tit->X2)) continue;
      break;
    }  
    if(tit!=tit_end) rRes.Erase(sit++);
    else++sit;
  }
  
  //FD_DF("CtrlPfxOperator::DoEvaluate(): R " << rGen.StateSetToString(rRes));
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
  if(rOp.ArgCount()<1) {
    std::stringstream errstr;
    errstr << "operator \"" << rOp.Name() << "\"  takes no arguments";
    throw Exception("MuIteration", errstr.str(), 80);
  }
  Name("mu " + rOp.ArgName(rOp.ArgCount()-1) + " . " + rOp.Name());
  mArgCount=rOp.ArgCount()-1;
  for(StateSetVector::Position pos=0; pos<rOp.ArgCount(); ++pos)
    mArgNames.push_back(rOp.ArgName(pos));
  mIndent=mrOp.Indent();
  mrOp.Indent(mIndent+"  ");
  FD_DF("MuIteration(): instantiated to evaluate " << Name());
};


// pass on indent to inner loops
void MuIteration::Indent(const std::string& indent) const {
  StateSetOperator::Indent(indent);
  mrOp.Indent(indent+"  ");
}

    
// inherit domain
const StateSet& MuIteration::Domain(void) const {
  return mrOp.Domain();
}  

// evaluation
void MuIteration::DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
  // prepare progress message
  std::string prog;
  if(Verbosity()>=10) {
    prog="MuIteration::DoEvaluate(): " + Indent() + Name() + ": " + ArgStatistics(rArgs);
    FAUDES_WRITE_CONSOLE("FAUDES_MUNU:  " << prog);
  }
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
    FD_DF("MuIteration::DoEvaluate(): " << Indent() << xsz << "# -> #" << R.Size());
    rRes.Assign(R);
    if(rRes.Size()==xsz) break;  
    FD_WPC(1,2,prog);
  }
  // say goodby
  if(Verbosity()>=10) {
    prog=prog + " -> " + mrOp.ArgName(mrOp.ArgCount()-1) + " #" + faudes::ToStringInteger(rRes.Size());
    FAUDES_WRITE_CONSOLE("FAUDES_MUNU:  " << prog);
  }
};


// evaluation incl rank (there are more clever wayz to implement this ...)
void MuIteration::Rank(StateSetVector& rArgs, std::map<Idx,int>& rRMap) const {
  // consistency check
  if(rArgs.Size()!=mArgCount) {
    std::stringstream errstr;
    errstr << "signature mismatch: expected arguments #" << mArgCount <<
      " provided argumenst #" << rArgs.Size();
    throw Exception("StateSetOperator::Evaluate", errstr.str(), 80);
  }
  // prepare progress message
  std::string prog;
  if(Verbosity()>=10) {
    prog="MuIteration::Rank(): " + Indent() + Name() + ": " + ArgStatistics(rArgs);
    FAUDES_WRITE_CONSOLE("FAUDES_MUNU:  " << prog);
  }
  // prepare result
  rRMap.clear();
  StateSet res;
  // actual implementation comes here
  StateSetVector xargs;
  xargs.AssignByReference(rArgs);  
  xargs.PushBack(&res);
  StateSet R;
  StateSet N;
  StateSet::Iterator sit;
  int rank=0;
  while(true) {
    mrOp.Evaluate(xargs,R);
    N=R-res;
    FD_DF("MuIteration::DoEvaluate(): " << Indent() << res.size() << "# -> #" << R.Size());
    res.Assign(R);
    if(N.Empty()) break;
    for(sit=N.Begin();sit!=N.End();++sit)
      rRMap[*sit]=rank;
    ++rank;
    FD_WPC(1,2,prog);
  }
  // say goodby
  if(Verbosity()>=10) {
    prog=prog + " -> " + mrOp.ArgName(mrOp.ArgCount()-1) + " #" + faudes::ToStringInteger(res.Size());
    FAUDES_WRITE_CONSOLE("FAUDES_MUNU:  " << prog);
  }
};

// API wrapper, single argument  
void MuIteration::Rank(StateSet& rArg, std::map<Idx,int>& rRMap) const {
  StateSetVector args;
  args.PushBack(&rArg);
  Rank(args,rRMap); 
}

// API wrapper, no arguments 
void MuIteration::Rank(std::map<Idx,int>& rRMap) const {
  static StateSetVector args;
  Rank(args,rRMap); 
}
  

/*  
*********************************************************************

Implementation of fixpoint iterations: nu

*********************************************************************    
*/

NuIteration::NuIteration(const StateSetOperator& rOp) :
  StateSetOperator(),
  mrOp(rOp)
{
  if(rOp.ArgCount()<1) {
    std::stringstream errstr;
    errstr << "operator \"" << rOp.Name() << "\"  takes no arguments";
    throw Exception("MuIteration", errstr.str(), 80);
  }
  Name("nu " + rOp.ArgName(rOp.ArgCount()-1) + " . " + rOp.Name());
  mArgCount=rOp.ArgCount()-1;
  for(StateSetVector::Position pos=0; pos<rOp.ArgCount(); ++pos)
    mArgNames.push_back(rOp.ArgName(pos));
  mIndent=mrOp.Indent();
  mrOp.Indent(mIndent+"  ");
  FD_DF("NuIteration(): instantiated to evaluate " << Name());
}

// pass on indent to inner loops
void NuIteration::Indent(const std::string& indent) const {
  StateSetOperator::Indent(indent);
  mrOp.Indent(indent+"  ");
}

// inherit domain
const StateSet& NuIteration::Domain(void) const {
  return mrOp.Domain();
}  

// evaluation
void NuIteration::DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
  // prepare progress message
  std::string prog;
  if(Verbosity()>=10) {
    prog="NuIteration::DoEvaluate(): " + Indent() + Name() + ": " + ArgStatistics(rArgs);
    FAUDES_WRITE_CONSOLE("FAUDES_MUNU:  " << prog);
  }
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
    FD_DF("NuIteration::DoEvaluate(): " << Indent() << xsz << "# -> #" << R.Size());
    rRes.Assign(R);
    if(rRes.Size()==xsz) break;  
    FD_WPC(1,2,prog);
  }
  // say goodby
  if(Verbosity()>=10) {
    prog=prog + " -> " + mrOp.ArgName(mrOp.ArgCount()-1) + " #" + faudes::ToStringInteger(rRes.Size());
    FAUDES_WRITE_CONSOLE("FAUDES_MUNU:  " << prog);
  }
};

} // namespace faudes


