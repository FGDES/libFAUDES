/** @file omg_rabinctrl.cpp Controller synthesis for Rabin automata */


/*
FAU Discrete Event Systems Library (libFAUDES)

Copyright (C) 2025 Thomas Moor

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
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

  

#include "omg_rabinctrl.h"
#include "omg_rabinfnct.h"
#include "syn_include.h"

// local debug via FD_DF
//#undef FD_DF
//#define FD_DF(m) FD_WARN(m)

// local debug via FAUDES_DEBUG_RABINCTRL
//#define FAUDES_DEBUG_RABINCTRL

namespace faudes {

/*  
*****************************************************************
*****************************************************************
*****************************************************************

Implementation of the fixpoint iteration proposed by Thistle/Wonham
in "Control of w-Automata, Church's Problem, and the Emptiness Problem
for Tree w-Automata", 1992.

The iteration is ran on a transitions structure with two independent
acceptance conditions, one Rabin and one Buechi. The result is the so
so called controllable set, i.e., the set of states for which there
exit individual control patterns such that in the controlled system,
the Buechi acceptance condition (think of plant liveness properties)
implies the Rabin acceptance condition (think of specification
liveness requirements).

At the current stage we only address specifications with a single
Rabin pair, as this is our current use case. 

*****************************************************************
*****************************************************************
*****************************************************************
*/
  
/*
Base class for my operators to hold context
*/
class  RabinInvDynOperator : public StateSetOperator {
protected:
  /** record context references */
  const vGenerator& rGen;
  const StateSet& rDomain;
  const StateSet& rMarkedStates;
  const TransSet& rTransRel;
  const TransSetX2EvX1& rRevTransRel;
  EventSet mSigmaCtrl;
  /** record control patterns */
  bool mRecCtrl=false;
  TaIndexSet<EventSet> mController;
public:
  /** construct to record context */
  RabinInvDynOperator(
    const vGenerator& gen, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    StateSetOperator(),
    rGen(gen),
    rDomain(gen.States()),
    rMarkedStates(gen.MarkedStates()),
    rTransRel(gen.TransRel()),
    rRevTransRel(revtrans),    
    mSigmaCtrl(sigctrl),
    mRecCtrl(false)
  {
    Name("void base class rabin-inv-dynamics operator");
    mArgCount=0;
  };
  /** overaall stateset */
  virtual const StateSet& Domain(void) const {
    return rDomain;
  }
  /** access control flag  */
  void RecCtrl(bool on) { mRecCtrl=on; }
  bool RecCtrl(void) { return mRecCtrl; }
  /** acces control pattern */
  TaIndexSet<EventSet>& Controller(void) { return mController; }
  const TaIndexSet<EventSet>& Controller(void) const { return mController; }
  void ClrCtrl(void) { mController.Clear(); }
  void InsCtrl(const RabinInvDynOperator& rOther) {
    if(!mRecCtrl) return;
    const TaIndexSet<EventSet>& otherctrl=rOther.Controller();
    IndexSet::Iterator sit=otherctrl.Begin();
    IndexSet::Iterator sit_end=otherctrl.End();
    for(;sit!=sit_end;++sit)
      if(!mController.Exists(*sit))
	mController.Insert(*sit,otherctrl.Attribute(*sit));
  }
};      


/*
Inverse dynamics operator theta
    
Cite Thistle/Wonham "Control of w-Automata, Church's Problem, and the Emptiness
Problem for Tree w-Automata", 1992, Def 8.2:

theta(X1,X2) = "the set of states, from which the automaton can be controlled to
enter X1 union X2 in a single transition without being prevented from entering X1."
    
Rephrase: X1 is the target T, X1 union X2 is the domain D; control such that immediate
successors stay within within D and there is the chance to enter the T.

We use "Z1/Z2" argument names to avoid confusion with libFAUDES naming conventions.
*/
class  RabinInvDynTheta : public RabinInvDynOperator {
public:
  /** construct to record context */
  RabinInvDynTheta(
    const vGenerator& gen, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynOperator(gen,revtrans,sigctrl)
  {
    FD_DF("RabinInvDynTheta(): instantiated for " << rGen.Name());
    Name("theta([Z1,Z2])");
    mArgNames= std::vector<std::string>{"Z1","Z2"};
    mArgCount=2;
  };
protected:
  /** loop local vars */
  EventSet mDisable;    
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    // convenience accesors
    const StateSet& Z1=rArgs.At(0);
    const StateSet& Z2=rArgs.At(1);
    // do operate
    rRes=rRevTransRel.PredecessorStates(Z1);
    StateSet::Iterator sit=rRes.Begin();
    StateSet::Iterator sit_end=rRes.End();
    while(sit!=sit_end){
      if(mRecCtrl) mDisable.Clear();
      TransSet::Iterator tit=rTransRel.Begin(*sit);
      TransSet::Iterator tit_end=rTransRel.End(*sit);
      bool enterZ1 = false;
      bool exitZ12 = false;      
      for(;tit!=tit_end;++tit){
	// successor is in Z1: will not disable, found evidence to enter Z1
        if(Z1.Exists(tit->X2)) {enterZ1=true; continue;}
        // successor is in Z2: will not disable     	
        if(Z2.Exists(tit->X2)) {continue;}
	// sucessor is neither in Z1 nor Z2: need to disable
    	if(!mSigmaCtrl.Exists(tit->Ev)){exitZ12 = true; break;}
	// record controls
	if(mRecCtrl) mDisable.Insert(tit->Ev);
      }
      // failed
      if(!enterZ1 || exitZ12) {rRes.Erase(sit++); continue;}
      // success
      if(mRecCtrl) mController.Insert(*sit,rGen.Alphabet()-mDisable);
      ++sit;
    }
    
  }; 
};      
    

/*
Inverse dynamics operator theta-tilde

Cite Thistle/Wonham "Control of w-Automata, Church's Problem, and the Emptiness
Problem for Tree w-Automata", 1992, Def 8.3:

"theta_tilde(X1,X2)=nu X3 mu X4 theta( X1 + (X4 - R), (X3 - R) * X2 ) ")

We first implement the core formula without the nu/mu iteration and then apply nu/mu.
We use "Y1/Y2/Y3/Y4" -->  "Z1/Z2" argument names 
*/
class  RabinInvDynThetaTildeCore : public RabinInvDynTheta {
public:
  /** construct to record context */
  RabinInvDynThetaTildeCore(
    const vGenerator& gen, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynTheta(gen,revtrans,sigctrl)
  {
    FD_DF("RabinInvDynThetaTildeCore(): instantiated for " << rGen.Name());
    Name("theta_tilde_core([Y1,Y2,Y3,Y4])");
    mArgNames= std::vector<std::string>{"Y1","Y2","Y3","Y4"};
    mArgCount=4;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    // convenience acsesors
    const StateSet& Y1=rArgs.At(0);
    const StateSet& Y2=rArgs.At(1);
    const StateSet& Y3=rArgs.At(2);
    const StateSet& Y4=rArgs.At(3);
    StateSetVector args;
    args.Size(2);
    StateSet& Z1=args.At(0);
    StateSet& Z2=args.At(1);
    // do operate
    Z1= Y1 + (Y4 - rMarkedStates);
    Z2= Y2 * (Y3 - rMarkedStates);
    RabinInvDynTheta::DoEvaluate(args,rRes);
  };
};      
    

/*
Inverse dynamics operator theta tilde, inner mu iteration
*/
class  RabinInvDynThetaTildeInner : public RabinInvDynOperator {
protected:
  /** additional context */
  RabinInvDynThetaTildeCore mThetaCore;
  MuIteration mMuThetaCore;
public:
  /** construct to record context */
  RabinInvDynThetaTildeInner(
    const vGenerator& gen, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynOperator(gen,revtrans,sigctrl),
    mThetaCore(gen,revtrans,sigctrl),
    mMuThetaCore(mThetaCore)
  {
    FD_DF("RabinInvDynThetaTildeInner(): instantiated for " << rGen.Name());
    Name("theta_tilde_inner_mu([Y1,Y2,Y3])");
    mArgNames= std::vector<std::string>{"Y1","Y2","Y3"};
    mArgCount=3;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    // pass on ctrl record flag
    mThetaCore.RecCtrl(mRecCtrl);
    mThetaCore.ClrCtrl();
    // run mu iteration
    mMuThetaCore.Evaluate(rArgs, rRes);
    // merge controller
    InsCtrl(mThetaCore);
  };
};      


/*
Inverse dynamics operator theta tilde, outer nu iteration
*/
class  RabinInvDynThetaTilde : public RabinInvDynOperator {
protected:
  /** additional context */
  RabinInvDynThetaTildeInner mMuThetaCore;
  NuIteration mNuMuThetaCore;
public:
  /** construct to record context */
  RabinInvDynThetaTilde(
    const vGenerator& gen, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynOperator(gen,revtrans,sigctrl),
    mMuThetaCore(gen,revtrans,sigctrl),
    mNuMuThetaCore(mMuThetaCore)
  {
    FD_DF("RabinInvDynThetaTilde(): instantiated for " << rGen.Name());
    Name("theta_tilde([Y1,Y2])");
    mArgNames= std::vector<std::string>{"Y1","Y2"};
    mArgCount=2;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    // plain fixpoint, dont record in inner mu
    mMuThetaCore.RecCtrl(false);
    mNuMuThetaCore.Evaluate(rArgs, rRes);
    // if we have been asked to record, run mu again with nu-var set to fixpoint
    if(mRecCtrl) {
      StateSetVector args;
      args.AssignByReference(rArgs);
      args.PushBack(&rRes);
      mMuThetaCore.RecCtrl(true);
      StateSet dummy;
      mMuThetaCore.Evaluate(args,dummy);
      InsCtrl(mMuThetaCore);
#ifdef FAUDES_DEBUG 
      if(!dummy.Equal(rRes)) {
	FD_ERR("RabinInvDynThetaTilde: internal errror in secondary run of mu iteration")
      }
#endif
    }
  };
};      


/*
P-reach operator

Cite Thistle/Wonham "Control of w-Automata, Church's Problem, and the Emptiness
Problem for Tree w-Automata", 1992, Def 8.4

p-reach(X1,X2)= mu X3 . theta-tilde(X1,X) union theta-tilde((X1 u X2 u X3, I_p))

We first implement the core formular without the nu/mu iteration, and then do the
mu iteration

We use "U1/U2/U3" 
*/
class  RabinInvDynPReachCore : public RabinInvDynOperator {
  /** additional context */
  RabinInvDynThetaTilde mThetaTilde;
  RabinAcceptance::CIterator mRPit;
public:
  /** construct to record context */
  RabinInvDynPReachCore(
    const RabinAutomaton& raut, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynOperator(raut,revtrans,sigctrl),
    mThetaTilde(raut,revtrans,sigctrl),
    mRPit(raut.RabinAcceptance().Begin())
  {
    FD_DF("RabinInvDynPReachCore(): instantiated for " << rGen.Name());
    Name("p_reach_core([U1,U2,U3)");
    mArgNames= std::vector<std::string>{"U1","U2","U3"};
    mArgCount=3;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    // pass on rec flag
    mThetaTilde.RecCtrl(mRecCtrl);
    mThetaTilde.ClrCtrl();
    // convenience accessors
    const StateSet& U1=rArgs.At(0);
    const StateSet& U2=rArgs.At(1);
    const StateSet& U3=rArgs.At(2);
    StateSetVector args;
    args.Size(2);
    StateSet& Y1=args.At(0);
    StateSet& Y2=args.At(1);
    // do operate a
    Y1=U1;
    Y2=Domain();
    mThetaTilde.Evaluate(args,rRes);
    InsCtrl(mThetaTilde);
    // do operate b
    Y1=U1+U2+U3;
    Y2=mRPit->ISet();
    StateSet rhs;
    mThetaTilde.Evaluate(args,rhs);
    InsCtrl(mThetaTilde);
    rRes.InsertSet(rhs);
    //std::cout << "p_reach_core: ctrl #" << mController.Size() << std::endl;
  };
};      
    
/** p-reach operator, mu iteration */
class  RabinInvDynPReach : public RabinInvDynOperator {
protected:
  /** have additional context */
  RabinInvDynPReachCore mPReachCore;
  MuIteration mMuPReachCore;
public:
  /** construct to record context */
  RabinInvDynPReach(
    const RabinAutomaton& raut, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynOperator(raut,revtrans,sigctrl),
    mPReachCore(raut,revtrans,sigctrl),
    mMuPReachCore(mPReachCore)
  {
    FD_DF("RabinInvDynPReach(): instantiated for " << rGen.Name());
    Name("p_reach([O1,O2])");
    mArgNames= std::vector<std::string>{"O1","O2"};
    mArgCount=2;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    // pass on ctrl record flag
    mPReachCore.RecCtrl(mRecCtrl);
    mPReachCore.ClrCtrl();
    // run mu iteration
    mMuPReachCore.Evaluate(rArgs, rRes);
    // merge controller
    InsCtrl(mPReachCore);
    //std::cout << "p_reach: ctrl #" << mController.Size() << std::endl;
  };
};      

/*
Controllable subset

Cite Thistle/Wonham "Control of w-Automata, Church's Problem, and the Emptiness
Problem for Tree w-Automata", 1992, Def 8.5

CA = mu X1 nu X2 . p-reach(X1, X2 * Rp)

We first implement the core formula without the mu/nu iteration, and then do the mu/nu.

*/
class  RabinInvDynCtrlCore : public RabinInvDynPReach {
  /** additional context */
  RabinAcceptance::CIterator mRPit;
public:
  /** construct to record context */
  RabinInvDynCtrlCore(
    const RabinAutomaton& raut, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynPReach(raut,revtrans,sigctrl),
    mRPit(raut.RabinAcceptance().Begin())
  {
    FD_DF("RabinInvDynCtrlCore(): instantiated for " << rGen.Name());
    Name("ctrl_core([X1,X2])");
    mArgNames= std::vector<std::string>{"X1","X2"};
    mArgCount=2;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    // convenience accessors
    const StateSet& X1=rArgs.At(0);
    const StateSet& X2=rArgs.At(1);
    StateSetVector args;
    args.Size(2);
    StateSet& O1=args.At(0);
    StateSet& O2=args.At(1);
    // do operate
    O1=X1;
    O2=X2 * mRPit->RSet();
    RabinInvDynPReach::DoEvaluate(args,rRes);
  };
};      
    
/** controllable set, inner nu iteration */
class  RabinInvDynCtrlInner : public RabinInvDynOperator {
protected:
  /** have additional context */
  RabinInvDynCtrlCore mCtrlCore;
  NuIteration mNuCtrlCore;
public:
  /** construct to record context */
  RabinInvDynCtrlInner(
    const RabinAutomaton& raut, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynOperator(raut,revtrans,sigctrl),
    mCtrlCore(raut,revtrans,sigctrl),
    mNuCtrlCore(mCtrlCore)
  {
    FD_DF("RabinInvDynCtrlInner(): instantiated for " << rGen.Name());
    Name("ctrl_inner_nu([X1])");
    mArgNames= std::vector<std::string>{"X1"};
    mArgCount=1;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    // plain fixpoint, dont record 
    mCtrlCore.RecCtrl(false);
    mNuCtrlCore.Evaluate(rArgs, rRes);
    // if we have been asked to record, nu-var set to fixpoint
    if(mRecCtrl) {
      StateSetVector args;
      args.AssignByReference(rArgs);
      args.PushBack(&rRes);
      mCtrlCore.RecCtrl(true);
      StateSet dummy;
      mCtrlCore.Evaluate(args,dummy);
      InsCtrl(mCtrlCore);
#ifdef FAUDES_DEBUG 
      if(!dummy.Equal(rRes)) {
	FD_ERR("RabinInvDynCtrlInner: internal errror in secondary run of mu iteration")
      }
#endif      
      std::cout << "ctrl_inner: ctrl #" << mController.Size() << std::endl;
    }
  };
};      

/** controllable set, outer mu iteration */
class  RabinInvDynCtrl : public RabinInvDynOperator {
protected:
  /** have additional context */
  RabinInvDynCtrlInner mNuCtrlCore;
  MuIteration mMuNuCtrlCore;
public:
  /** construct to record context */
  RabinInvDynCtrl(
    const RabinAutomaton& raut, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynOperator(raut,revtrans,sigctrl),
    mNuCtrlCore(raut,revtrans,sigctrl),
    mMuNuCtrlCore(mNuCtrlCore)
  {
    FD_DF("RabinInvDynCtrl(): instantiated for " << rGen.Name());
    Name("ctrl()");
    mArgCount=0;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    // pass on ctrl record flag
    mNuCtrlCore.RecCtrl(mRecCtrl);
    mNuCtrlCore.ClrCtrl();
    // run mu iteration
    mMuNuCtrlCore.Evaluate(rArgs, rRes);
    // merge controller
    InsCtrl(mNuCtrlCore);
    std::cout << "ctrl: ctrl #" << mController.Size() << std::endl;
  };
};      


/*  
*****************************************************************
*****************************************************************
*****************************************************************

API wrappers to compute the controllability prefix. The latter is
defined by Thistle/Wonham in "Supervision of Infinite Behavior of
Discrete-Event Systems, 1994. It is the star-language which consists
of all words such that thereafter a controller exits to enforce
the specification. The controllability prefix relates to the
controllable set as follows:

Given a plant L and a specification E subseteq L, assume we have
a transition systems such that L is accepted by a Buechi acceptance
condition and E is appected by a Rabin aceptance condition. We refer
to this as the candidate. The controllable set of the candidate then
marks the controllability prefix on the same transition structur.


*****************************************************************
*****************************************************************
*****************************************************************
*/

// API  
void RabinCtrlPfx(
  const RabinAutomaton& rRAut, const EventSet& rSigmaCtrl,
  StateSet& rCtrlPfx)
{
  // can only handle one Rabin pair
  if(rRAut.RabinAcceptance().Size()!=1){
    std::stringstream errstr;
    errstr << "the current implementation requires exactly one Rabin pair";
    throw Exception("RabinCtrlPfx", errstr.str(), 80);
  }
  // set up various helper
  TransSetX2EvX1 revtrans(rRAut.TransRel());
  EventSet sigctrl(rSigmaCtrl);
  // have operator
  RabinInvDynCtrl ctrl(rRAut,revtrans,sigctrl);
  // run
  ctrl.Evaluate(rCtrlPfx);
};


// API  
void RabinCtrlPfx(
  const RabinAutomaton& rRAut, const EventSet& rSigmaCtrl,
  TaIndexSet<EventSet>& rController)
{
  // can only handle one Rabin pair
  if(rRAut.RabinAcceptance().Size()!=1){
    std::stringstream errstr;
    errstr << "the current implementation requires exactly one Rabin pair";
    throw Exception("RabinCtrlPfx", errstr.str(), 80);
  }
  // set up various helper
  TransSetX2EvX1 revtrans(rRAut.TransRel());
  EventSet sigctrl(rSigmaCtrl);
  // have operator
  RabinInvDynCtrl ctrl(rRAut,revtrans,sigctrl);
  ctrl.RecCtrl(true);
  // run
  StateSet ctrlpfx;
  ctrl.Evaluate(ctrlpfx);
  rController.Assign(ctrl.Controller());
};


// API void RabinCtrlPfx(
void RabinCtrlPfx(
  const RabinAutomaton& rRAut, const EventSet& rSigmaCtrl,
  Generator& rResGen)
{
  // prepare result
  Generator* pResGen = &rResGen;
  if(dynamic_cast<RabinAutomaton*>(&rResGen)== &rRAut) {
    pResGen= rResGen.New();
  }
  pResGen->Clear();
  pResGen->Name(CollapsString("CtrlPfx("+rRAut.Name()+")"));

  // do run
  StateSet ctrlpfx;
  RabinCtrlPfx(rRAut,rSigmaCtrl,ctrlpfx);
  pResGen->Assign(rRAut);
  pResGen->InjectMarkedStates(ctrlpfx);
  pResGen->Trim();

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}

/*  
*****************************************************************
*****************************************************************
*****************************************************************

API wrappers for the supremal omega-controllable sublanguage.

The supremal controllable sublanguage relates to the controllability
prefix as follows.

Given a plant L and a specification E, set up the candidate and
compute the controllability prexis CFX. Then

K := lim sup-closed-sublanguage(CFX) cap E

is the supremal omega-controllable sublanguage of E.

Thistle/Wonham give in " Supervision of Infinite Behavior of Discrete-Event",
1994, the slightly different formula

K := lim sup-closed-and-controllabe--sublanguage(CFX) cap E,

but since only controllable envents can exit CFX to pre L both
formulae should yield the same result.

*****************************************************************
*****************************************************************
*****************************************************************
*/

// API warpper
void SupRabinCon(
  const Generator& rBPlant, 
  const EventSet& rCAlph, 
  const RabinAutomaton& rRSpec, 
  RabinAutomaton& rRes) 
{
  // consitenct check
  ControlProblemConsistencyCheck(rBPlant, rCAlph, rRSpec);  
  // prepare result
  bool snames= rBPlant.StateNamesEnabled() &&  rRSpec.StateNamesEnabled() &&  rRes.StateNamesEnabled();    
  RabinAutomaton* pRes = &rRes;
  if(dynamic_cast<Generator*>(pRes)== &rBPlant || pRes== &rRSpec) {
    pRes= rRes.New();
  }
  // execute: set up closed loop candidate
  pRes->Assign(rRSpec);
  pRes->ClearMarkedStates();
  Automaton(*pRes);
  RabinBuechiProduct(*pRes,rBPlant,*pRes);
  // execute: compute controllability prefix
  StateSet ctrlpfx;
  RabinCtrlPfx(*pRes,rCAlph,ctrlpfx);
  // execute: trim
  pRes->ClearMarkedStates();
  pRes->InsMarkedStates(ctrlpfx);
  SupClosed(*pRes,*pRes);
  pRes->ClearMarkedStates();
  pRes->RestrictStates(pRes->States()); // fix Rabin pairs
  // record name
  pRes->Name(CollapsString("SupRabinCon(("+rBPlant.Name()+"),("+rRSpec.Name()+"))"));
  // copy result
  if(pRes != &rRes) {
    pRes->Move(rRes);
    delete pRes;
  }
}


// SupRabinCon for Systems:
// uses and maintains controllablity from plant 
void SupRabinCon(
  const System& rBPlant, 
  const RabinAutomaton& rRSpec, 
  RabinAutomaton& rRes)
{
  // prepare result
  RabinAutomaton* pRes = &rRes;
  if(dynamic_cast<Generator*>(pRes)== &rBPlant || pRes== &rRSpec) {
    pRes= rRes.New();
  }
  pRes->StateNamesEnabled(rRes.StateNamesEnabled());
  // execute 
  SupRabinCon(rBPlant, rBPlant.ControllableEvents(),rRSpec,*pRes);
  // copy all attributes of input alphabet
  pRes->EventAttributes(rBPlant.Alphabet());
  // copy result
  if(pRes != &rRes) {
    pRes->Move(rRes);
    delete pRes;
  }
}


/*  
*****************************************************************
*****************************************************************
*****************************************************************

API wrappers for controller syntheis.

Technically, neither the supremal omega-controllable sublanguage
K nor the controllility prefix CFX are controllers. But CFX lends
itself as a basis to extract a controller. We provide two variants.

a)
The greedy controller will attempts to achieve yat another R-state
as fast as possible. This is a rather restrictive policy but it does
the job.

b)
Given a lower bound specification A, let the plant run freely
until it exits pre A. Then apply the greedy controller. This
requires the relative closure of A w.r.t the plant to be within
K

*****************************************************************
*****************************************************************
*****************************************************************
*/


// API warpper
void RabinCtrl(
  const Generator& rBPlant, 
  const EventSet& rCAlph, 
  const RabinAutomaton& rRSpec, 
  Generator& rRes) 
{
  // consitency check
  ControlProblemConsistencyCheck(rBPlant, rCAlph, rRSpec);  

  // execute: set up closed loop candidate
  bool snames= rBPlant.StateNamesEnabled() &&  rRSpec.StateNamesEnabled() &&  rRes.StateNamesEnabled();    
  RabinAutomaton cand;
  cand.Assign(rRSpec);
  cand.StateNamesEnabled(snames);
  cand.ClearMarkedStates();
  Automaton(cand);
  RabinBuechiProduct(cand,rBPlant,cand);

  // execute: compute controllability prefix incl greedy controller
  TaIndexSet<EventSet> controller;
  RabinCtrlPfx(cand,rCAlph,controller);

  // execute: trim to generate pre supcon
  Generator gcand=cand;
  gcand.ClearMarkedStates();
  gcand.InsMarkedStates(controller);
  SupClosed(gcand,gcand);
  gcand.ClearMarkedStates();

  // cosmetic: recover plant Buechi acceptance
  StateSet pbuechi = cand.MarkedStates() * gcand.States();
  gcand.InsMarkedStates(pbuechi);
  
  // execute: apply control patterns
  TransSet::Iterator tit=gcand.TransRelBegin();
  Idx cx=0;
  const EventSet* pctrlpat=nullptr;
  while(tit!=gcand.TransRelEnd()) {
    if(cx!=tit->X1) {
      cx=tit->X1;
      pctrlpat=nullptr;
      if(controller.Exists(cx))
	pctrlpat=&controller.Attribute(cx);
    }
    if(pctrlpat==nullptr) {
      FD_ERR("RabinCtrl: greedy: controller incomplete");
    }
    if(pctrlpat->Exists(tit->Ev)) {
      ++tit;
    } else {
      gcand.ClrTransition(tit++);
    }
  }

  // execute: polish
  gcand.Accessible();
  gcand.Move(rRes);
  rRes.Name(CollapsString("RabinCtrl(("+rBPlant.Name()+"),("+rRSpec.Name()+"))"));
}

// API warpper
void RabinCtrl(
  const Generator& rBPlant, 
  const EventSet& rCAlph, 
  const Generator& rGLSpec, 
  const RabinAutomaton& rRUSpec, 
  Generator& rRes) 
{
  // consitency check
  ControlProblemConsistencyCheck(rBPlant, rCAlph, rRUSpec);  
  ControlProblemConsistencyCheck(rBPlant, rCAlph, rGLSpec);

  // have alphabet
  const EventSet& sigma=rBPlant.Alphabet();
  
  // execute: set up ctrlpfx
  bool snames= false;
  RabinAutomaton cand;
  cand.Assign(rRUSpec);
  cand.StateNamesEnabled(snames);
  cand.ClearMarkedStates();
  Automaton(cand);
  RabinBuechiProduct(cand,rBPlant,cand);

  // execute: compute controllability prefix incl greedy controller
  TaIndexSet<EventSet> controller;
  RabinCtrlPfx(cand,rCAlph,controller);
#ifdef FAUDES_DEBUG_RABINCTRL
  controller.Write("tmp_rbc_controller.xml");
#endif  

  // execute: trim to obtain pre SupCon
  Generator gcand=cand;
  gcand.ClearMarkedStates();
  gcand.InsMarkedStates(controller);
  SupClosed(gcand,gcand);
  gcand.ClearMarkedStates();
#ifdef FAUDES_DEBUG_RABINCTRL
  gcand.Write("tmp_rbc_supcon.gen");
#endif  

  // todo: test lim LSpec cap L subseteq SupCon
  // (currently we do this implicitly when applyiny controlpatterns
  // so we should be fine ?)
  
  // excute: compose with LSpec given as generated language
  Generator lspec=rGLSpec;
  lspec.ClearMarkedStates();
  Idx ds=Automaton(lspec);
  ProductCompositionMap cmap;
  aProduct(lspec,gcand,cmap,rRes);
#ifdef FAUDES_DEBUG_RABINCTRL
  rRes.Write("tmp_rbc_product.gen");
#endif  

  // execute: apply control patterns on exit of lspec;
  TransSet::Iterator tit=rRes.TransRelBegin();
  Idx cx=0;
  Idx csx=0;
  Idx ccx=0;
  bool doctrl=false;
  const EventSet* pctrlpat=nullptr;
  while(tit!=rRes.TransRelEnd()) {
    // new state, update control pattern
    if(cx!=tit->X1) {
      cx=tit->X1; 
      pctrlpat=nullptr;
      csx = cmap.Arg1State(tit->X1);
      ccx = cmap.Arg2State(tit->X1);
      if(controller.Exists(ccx))
  	pctrlpat=&controller.Attribute(ccx);
      // record: if we are outside lspec, the greedy controller takes action
      doctrl = (csx==ds);
      // figure issue: we shall apply control but have no control-pattern
      if(doctrl && (pctrlpat==nullptr)) {
        FD_ERR("RabinCtrl: with rGLSpec: controller incomplete");
      }
      // figure issue: if we are in lspec, we shall be in ctrlpfx
      if( (csx!=ds) && (!controller.Exists(ccx)) ) {
        EmptyLanguage(sigma,rRes);
      }
    }
    // wont apply control: fine, let run
    if(!doctrl) {
      ++tit;
      continue;
    }
    // apply that control
    if(!pctrlpat->Exists(tit->Ev)) {
      rRes.ClrTransition(tit++);
    } else {
      ++tit;
    }
  }

  cand.WriteStateSet(err);

  // cosmetic: install plant Buechi marking
  StateSet::Iterator sit=rRes.StatesBegin();
  for(; sit!=rRes.StatesEnd(); ++sit) {
    Idx ccx = cmap.Arg2State(*sit);
    if(cand.ExistsMarkedState(ccx))
       rRes.InsMarkedState(*sit);
  }
  
  // execute: polish
  rRes.Accessible();
  rRes.Name(CollapsString("RabinCtrl(("+rBPlant.Name()+"),("+rRUSpec.Name()+"))"));
}


// API warpper
void RabinCtrl(
  const System& rBPlant, 
  const RabinAutomaton& rRSpec, 
  Generator& rRes) 
{
  // prepare result
  Generator* pRes = &rRes;
  if(dynamic_cast<System*>(pRes)== &rBPlant || dynamic_cast<RabinAutomaton*>(pRes)== &rRSpec) {
    pRes= rRes.New();
  }
  // execute 
  RabinCtrl(rBPlant, rBPlant.ControllableEvents(),rRSpec,*pRes);
  // copy all attributes of input alphabet
  pRes->EventAttributes(rBPlant.Alphabet());
  // copy result
  if(pRes != &rRes) {
    pRes->Move(rRes);
    delete pRes;
  }
}

// API warpper
void RabinCtrl(
  const System& rBPlant, 
  const Generator& rBLSpec, 
  const RabinAutomaton& rRUSpec, 
  Generator& rRes) 
{
  // prepare result
  Generator* pRes = &rRes;
  bool copy=false;
  if(dynamic_cast<System*>(pRes)== &rBPlant) copy=true;
  if(dynamic_cast<RabinAutomaton*>(pRes)== &rRUSpec) copy=true;
  if(dynamic_cast<Generator*>(pRes)== &rBLSpec) copy=true;
  if(copy) pRes= rRes.New();
  // execute 
  RabinCtrl(rBPlant, rBPlant.ControllableEvents(),rBLSpec,rRUSpec,*pRes);
  // copy all attributes of input alphabet
  pRes->EventAttributes(rBPlant.Alphabet());
  // copy result
  if(pRes != &rRes) {
    pRes->Move(rRes);
    delete pRes;
  }
}
  
} // namespace faudes

