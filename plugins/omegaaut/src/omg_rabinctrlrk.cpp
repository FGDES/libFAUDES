/** @file omg_rabinctrlrk.cpp Synthesis omega languages accepted by Rabin automata */

/* FAU Discrete Event Systems Library (libFAUDES)

Copyright (C) 2025 Changming Yang, Thomas Moor

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



/*
This is an alternative implementation to compute the controllable set
aka the controllability prefix. Doublets w.r.t. the the base implementation
have been suffixed "_RK". From my petrspective, they can be safely removed
(for classs definitions they dont generate symbols in the bbject file and
the only doublet function RabinCtrlPfx is not invoked anywhere).

See also the heeder omg_rabinctrlrk.h.

CY/TM, 2025/07.
*/
  

#include "omg_rabinctrlrk.h"
#include "syn_include.h"
#include "omg_controlpattern.h"

// local degug
//#undef FD_DF
//#define FD_DF(m) FD_WARN(m)

namespace faudes {

/*
Base class for my operators to hold context
*/
class  RabinInvDynOperator_RK : public StateSetOperator {
protected:
  /** record context references */
  const vGenerator& rGen;
  const StateSet& rDomain;
  const StateSet& rMarkedStates;
  const TransSet& rTransRel;
  const TransSetX2EvX1& rRevTransRel;
  EventSet mSigmaCtrl;
public:
  /** construct to record context */
  RabinInvDynOperator_RK(
    const vGenerator& gen, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    StateSetOperator(),
    rGen(gen),
    rDomain(gen.States()),
    rMarkedStates(gen.MarkedStates()),
    rTransRel(gen.TransRel()),
    rRevTransRel(revtrans),    
    mSigmaCtrl(sigctrl)
  {
    Name("void base class operator");
    mArgCount=0;
  };
  /** overaall stateset */
  virtual const StateSet& Domain(void) const {
    return rDomain;
  }
};

/*
Extended base class for operators that track state rankings for feedback construction
*/
class  RabinInvDynOperatorWithRanking : public RabinInvDynOperator_RK {
protected:
  /** state ranking map for feedback construction */
  mutable StateRankingMap* mpStateRanking;
  /** current iteration levels */
  mutable int mCurrentMuLevel;
  mutable int mCurrentNuLevel;
  mutable int mCurrentBranchType;
  
public:
  /** construct to record context */
  RabinInvDynOperatorWithRanking(
    const vGenerator& gen, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynOperator_RK(gen, revtrans, sigctrl),
    mpStateRanking(nullptr),
    mCurrentMuLevel(0),
    mCurrentNuLevel(0),
    mCurrentBranchType(0)
  {
    Name("base class operator with ranking");
  };
  
  /** set state ranking map pointer */
  void SetStateRankingMap(StateRankingMap* pRanking) {
    mpStateRanking = pRanking;
  }
  
  /** set current iteration levels */
  void SetCurrentLevels(int muLevel, int nuLevel, int branchType) const {
    mCurrentMuLevel = muLevel;
    mCurrentNuLevel = nuLevel;
    mCurrentBranchType = branchType;
  }
  
protected:
  /** record state ranking when adding states to result */
  void RecordStateRanking(const StateSet& rNewStates) const {
    if (mpStateRanking == nullptr) return;
    
    StateRanking ranking(mCurrentMuLevel, mCurrentNuLevel, mCurrentBranchType);
    for (StateSet::Iterator it = rNewStates.Begin(); it != rNewStates.End(); ++it) {
      // Only record if this state hasn't been ranked yet (first time added)
      if (mpStateRanking->find(*it) == mpStateRanking->end()) {
        (*mpStateRanking)[*it] = ranking;
      }
    }
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
class  RabinInvDynTheta_RK : public RabinInvDynOperator_RK {
public:
  /** construct to record context */
  RabinInvDynTheta_RK(
    const vGenerator& gen, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynOperator_RK(gen,revtrans,sigctrl)
  {
    FD_DF("RabinInvDynTheta(): instantiated for " << rGen.Name());
    Name("theta([Z1,Z2])");
    mArgNames= std::vector<std::string>{"Z1","Z2"};
    mArgCount=2;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    // convenience acsesors
    const StateSet& Z1=rArgs.At(0);
    const StateSet& Z2=rArgs.At(1);
    // do operate
    rRes=rRevTransRel.PredecessorStates(Z1);
    StateSet::Iterator sit=rRes.Begin();
    StateSet::Iterator sit_end=rRes.End();
    while(sit!=sit_end){
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
    	if(!mSigmaCtrl.Exists(tit->Ev)){ exitZ12 = true; break;}
      }  
      if(!enterZ1 || exitZ12) rRes.Erase(sit++);
      else++sit;
    }
  }; 
};      
    

/*
Inverse dynamics operator theta-tilde

Cite Thistle/Wonham "Control of w-Automata, Church's Problem, and the Emptiness
Problem for Tree w-Automata", 1992, Def 8.3:

"theta_tilde(X1,X2)=nu X3 mu X4 theta( X1 + (X4 - R), (X3 - R) * X2 ) ")

We first implement the core formula without the nu/mu iteration and then apply nu/mu.
We use "Y1/Y2/Y3/Y4" and "Z1/Z2" argument names to
*/
class  RabinInvDynThetaTildeCore_RK : public RabinInvDynTheta_RK {
public:
  /** construct to record context */
  RabinInvDynThetaTildeCore_RK(
    const vGenerator& gen, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynTheta_RK(gen,revtrans,sigctrl)
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
    RabinInvDynTheta_RK::DoEvaluate(args,rRes);
  };
};      
    

/*
Inverse dynamics operator theta tilde, outer nu/mu iteration
*/
class  RabinInvDynThetaTilde_RK : public RabinInvDynOperator_RK {
protected:
  /** additional context */
  RabinInvDynThetaTildeCore_RK mThetaCore;
  MuIteration mMuThetaCore;
  NuIteration mNuMuThetaCore;
public:
  /** construct to record context */
  RabinInvDynThetaTilde_RK(
    const vGenerator& gen, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynOperator_RK(gen,revtrans,sigctrl),
    mThetaCore(gen,revtrans,sigctrl),
    mMuThetaCore(mThetaCore),
    mNuMuThetaCore(mMuThetaCore)
  {
    FD_DF("RabinInvDynThetaTilde(): instantiated for " << rGen.Name());
    Name("theta_tilde([W1,W2])");
    mArgNames= std::vector<std::string>{"W1","W2"};
    mArgCount=2;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    mNuMuThetaCore.Evaluate(rArgs, rRes);
  };
};      


/*
P-reach operator

Cite Thistle/Wonham "Control of w-Automata, Church's Problem, and the Emptiness
Problem for Tree w-Automata", 1992, Def 8.4

p-reach(X1,X2)= mu X3 . theta-tilde(X1,X) union theta-tilde((X1 u X2 u X3, I_p))

We first implement the core formular without the nu/mu iteration, and then do the
mu iteration

We use "U1/U2/U3" and "W1/W2" argument names
*/
class  RabinInvDynPReachCore_RK : public RabinInvDynThetaTilde_RK {
  /** additional context */
  RabinAcceptance::CIterator mRPit;
public:
  /** construct to record context */
  RabinInvDynPReachCore_RK(
    const RabinAutomaton& raut, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynThetaTilde_RK(raut,revtrans,sigctrl),
    mRPit(raut.RabinAcceptance().Begin())
  {
    FD_DF("RabinInvDynPReachCore(): instantiated for " << rGen.Name());
    Name("invdyn_op([U1,U2,U3)");
    mArgNames= std::vector<std::string>{"U1","U2","U3"};
    mArgCount=3;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    // convenience accessors
    const StateSet& U1=rArgs.At(0);
    const StateSet& U2=rArgs.At(1);
    const StateSet& U3=rArgs.At(2);
    StateSetVector args;
    args.Size(2);
    StateSet& W1=args.At(0);
    StateSet& W2=args.At(1);
    // do operate
    W1=U1;
    W2=Domain();
    RabinInvDynThetaTilde_RK::DoEvaluate(args,rRes);
    W1=U1+U2+U3;
    W2=mRPit->ISet();
    StateSet rhs;
    RabinInvDynThetaTilde_RK::DoEvaluate(args,rhs);
    rRes.InsertSet(rhs);
  };
};      
    
/** p-reach operator, mu iteration */
class  RabinInvDynPReach_RK : public RabinInvDynOperator_RK {
protected:
  /** have additional context */
  RabinInvDynPReachCore_RK mPReachCore;
  MuIteration mMuPReachCore;
public:
  /** construct to record context */
  RabinInvDynPReach_RK(
    const RabinAutomaton& raut, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynOperator_RK(raut,revtrans,sigctrl),
    mPReachCore(raut,revtrans,sigctrl),
    mMuPReachCore(mPReachCore)
  {
    FD_DF("RabinInvDynPReach(): instantiated for " << rGen.Name());
    Name("p_reach_op([O1,O2])");
    mArgNames= std::vector<std::string>{"O1","O2"};
    mArgCount=2;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    mMuPReachCore.Evaluate(rArgs, rRes);
  };
};      

/*
Controllable subset

Cite Thistle/Wonham "Control of w-Automata, Church's Problem, and the Emptiness
Problem for Tree w-Automata", 1992, Def 8.5

CA = mu X1 nu X2 . p-reach(X1, X2 * R)

We first implement the core formular without the mu/nu iteration, and then do the mu/nu.

*/
class  RabinInvDynCtrlCore_RK : public RabinInvDynPReach_RK {
  /** additional context */
  RabinAcceptance::CIterator mRPit;
public:
  /** construct to record context */
  RabinInvDynCtrlCore_RK(
    const RabinAutomaton& raut, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynPReach_RK(raut,revtrans,sigctrl),
    mRPit(raut.RabinAcceptance().Begin())
  {
    FD_DF("RabinInvDynCtrlCore(): instantiated for " << rGen.Name());
    Name("ctrl([X1,X2])");
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
    RabinInvDynPReach_RK::DoEvaluate(args,rRes);
  };
};      
    
/** ctrollable set, mu-nu iteration */
class  RabinInvDynCtrl_RK : public RabinInvDynOperator_RK {
protected:
  /** have additional context */
  RabinInvDynCtrlCore_RK mCtrlCore;
  NuIteration mNuCtrlCore;
  MuIteration mMuNuCtrlCore;
public:
  /** construct to record context */
  RabinInvDynCtrl_RK(
    const RabinAutomaton& raut, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynOperator_RK(raut,revtrans,sigctrl),
    mCtrlCore(raut,revtrans,sigctrl),
    mNuCtrlCore(mCtrlCore),
    mMuNuCtrlCore(mNuCtrlCore)
  {
    FD_DF("RabinInvDynCtrl(): instantiated for " << rGen.Name());
    Name("ctrl()");
    mArgCount=0;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    mMuNuCtrlCore.Evaluate(rArgs, rRes);
  };
};      



// doublet: see ong_rabinctrl.cpp  --- can be safely removed  
void RabinCtrlPfx_RK(
  const RabinAutomaton& rRAut, const EventSet& rSigmaCtrl,
  StateSet& rCtrlPfx){
  // we can only handle one Rabin pair
  if(rRAut.RabinAcceptance().Size()!=1){
    std::stringstream errstr;
    errstr << "the current implementation requires exactly one Rabin pair";
    throw Exception("RabinCtrlPfx", errstr.str(), 80);
  }
  // set up various helper
  TransSetX2EvX1 revtrans(rRAut.TransRel());
  EventSet sigctrl(rSigmaCtrl);
  // have operator
  RabinInvDynCtrl_RK ctrl(rRAut,revtrans,sigctrl);
  // run
  ctrl.Evaluate(rCtrlPfx);
};



/*
Extended controllable subset operator with state ranking tracking
*/
class RabinInvDynCtrlWithRanking : public RabinInvDynOperatorWithRanking {
protected:
  /** additional context */
  const RabinAutomaton& mRAut;
  RabinAcceptance::CIterator mRPit;
  
public:
  /** construct to record context */
  RabinInvDynCtrlWithRanking(
    const RabinAutomaton& raut, const TransSetX2EvX1& revtrans, const EventSet& sigctrl)
  :
    RabinInvDynOperatorWithRanking(raut, revtrans, sigctrl),
    mRAut(raut),
    mRPit(raut.RabinAcceptance().Begin())
  {
    FD_DF("RabinInvDynCtrlWithRanking(): instantiated for " << rGen.Name());
    Name("ctrl_with_ranking()");
    mArgCount=0;
  };
  
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) {
    // Implementation of the nested mu-nu fixpoint with ranking tracking
    rRes.Clear();
    
    // Outer mu-iteration: C^A = μX1. νX2. ρ^A_p(X1, X2∩R_p)
    StateSet X1_current;  // Start with empty set for mu-iteration
    StateSet X1_prev;
    int muLevel = 0;
    
    do {
      X1_prev = X1_current;
      ++muLevel;
      
      // Inner nu-iteration: νX2. ρ^A_p(X1_current, X2∩R_p)
      StateSet X2_current = Domain();  // Start with full domain for nu-iteration
      StateSet X2_prev;
      int nuLevel = 0;
      
      do {
        X2_prev = X2_current;
        ++nuLevel;
        
        // Compute ρ^A_p(X1_current, X2_current ∩ R_p)
        StateSet pReachResult;
        ComputePReachWithRanking(X1_current, X2_current * mRPit->RSet(), pReachResult, muLevel, nuLevel);
        
        X2_current = X2_current * pReachResult;  // Intersection for nu-iteration
        
      } while (X2_current != X2_prev);
      
      X1_current = X1_current + X2_current;  // Union for mu-iteration
      
    } while (X1_current != X1_prev);
    
    rRes = X1_current;
  };
  
private:
  /** compute p-reach operator with ranking tracking */
  void ComputePReachWithRanking(const StateSet& X1, const StateSet& X2, StateSet& rRes, 
                               int muLevel, int nuLevel) const {
    // This implements: μX3. θ̃(X1, Domain) ∪ θ̃(X1∪X2∪X3, I_p)
    rRes.Clear();
    StateSet X3_current;  // Start with empty set for mu-iteration
    StateSet X3_prev;
    int pReachLevel = 0;
    
    do {
      X3_prev = X3_current;
      ++pReachLevel;
      
      // First branch: θ̃(X1, Domain)
      StateSet branch1;
      ComputeThetaTildeWithRanking(X1, Domain(), branch1, muLevel, nuLevel, 0);
      
      // Second branch: θ̃(X1∪X2∪X3_current, I_p)  
      StateSet branch2;
      ComputeThetaTildeWithRanking(X1 + X2 + X3_current, mRPit->ISet(), branch2, muLevel, nuLevel, 1);
      
      StateSet newStates = (branch1 + branch2) - X3_current;
      if (!newStates.Empty()) {
        SetCurrentLevels(muLevel, nuLevel, 0);  // Branch type will be set in theta-tilde
        RecordStateRanking(newStates);
      }
      
      X3_current = X3_current + branch1 + branch2;  // Union for mu-iteration
      
    } while (X3_current != X3_prev);
    
    rRes = X3_current;
  }
  
  /** compute theta-tilde operator with ranking tracking */
  void ComputeThetaTildeWithRanking(const StateSet& W1, const StateSet& W2, StateSet& rRes,
                                   int muLevel, int nuLevel, int branchType) const {
    // This implements: νX3. μX4. θ(W1 ∪ (X4 - R), (X3 - R) ∩ W2)
    rRes.Clear();
    StateSet X3_current = Domain();  // Start with full domain for nu-iteration
    StateSet X3_prev;
    
    do {
      X3_prev = X3_current;
      
      // Inner mu-iteration: μX4. θ(W1 ∪ (X4 - R), (X3_current - R) ∩ W2)
      StateSet X4_current;  // Start with empty set for mu-iteration
      StateSet X4_prev;
      
      do {
        X4_prev = X4_current;
        
        // Compute θ(W1 ∪ (X4_current - R), (X3_current - R) ∩ W2)
        StateSet thetaResult;
        ComputeThetaWithRanking(W1 + (X4_current - mRPit->RSet()), 
                               (X3_current - mRPit->RSet()) * W2, 
                               thetaResult, muLevel, nuLevel, branchType);
        
        X4_current = X4_current + thetaResult;  // Union for mu-iteration
        
      } while (X4_current != X4_prev);
      
      X3_current = X3_current * X4_current;  // Intersection for nu-iteration
      
    } while (X3_current != X3_prev);
    
    rRes = X3_current;
  }
  
  /** compute basic theta operator with ranking tracking */
  void ComputeThetaWithRanking(const StateSet& Z1, const StateSet& Z2, StateSet& rRes,
                              int muLevel, int nuLevel, int branchType) const {
    // This implements the basic inverse dynamics operator θ(Z1, Z2)
    rRes = rRevTransRel.PredecessorStates(Z1);
    StateSet::Iterator sit = rRes.Begin();
    StateSet::Iterator sit_end = rRes.End();
    
    while (sit != sit_end) {
      TransSet::Iterator tit = rTransRel.Begin(*sit);
      TransSet::Iterator tit_end = rTransRel.End(*sit);
      bool enterZ1 = false;
      bool exitZ12 = false;
      
      for (; tit != tit_end; ++tit) {
        // successor is in Z1: will not disable, found evidence to enter Z1
        if (Z1.Exists(tit->X2)) {
          enterZ1 = true;
          continue;
        }
        // successor is in Z2: will not disable
        if (Z2.Exists(tit->X2)) {
          continue;
        }
        // successor is neither in Z1 nor Z2: need to disable
        if (!mSigmaCtrl.Exists(tit->Ev)) {
          exitZ12 = true;
          break;
        }
      }
      
      if (!enterZ1 || exitZ12) {
        rRes.Erase(sit++);
      } else {
        ++sit;
      }
    }
    
    // Record ranking for newly computed states
    SetCurrentLevels(muLevel, nuLevel, branchType);
  }
};


/*
State feedback constructor based on state rankings
*/
class StateFeedbackConstructor {
private:
  const RabinAutomaton& mRAut;
  const EventSet& mSigmaCtrl;
  const StateRankingMap& mStateRanking;
  
public:
  StateFeedbackConstructor(const RabinAutomaton& raut, const EventSet& sigctrl, 
                          const StateRankingMap& ranking)
    : mRAut(raut), mSigmaCtrl(sigctrl), mStateRanking(ranking) {}
  
  /** construct state feedback mapping based on Theorem 6.4 */
  void ConstructStateFeedback(const StateSet& rCtrlPfx, StateFeedbackMap& rFeedback) const {
    rFeedback.clear();
    
    // Get Rabin pair (we assume exactly one pair)
    RabinAcceptance::CIterator rpIt = mRAut.RabinAcceptance().Begin();
    const StateSet& RSet = rpIt->RSet();
    const StateSet& ISet = rpIt->ISet();
    
    // For each controllable state, construct its control pattern
    for (StateSet::Iterator sit = rCtrlPfx.Begin(); sit != rCtrlPfx.End(); ++sit) {
      Idx state = *sit;
      EventSet controlPattern;
      
      // Get state ranking
      StateRankingMap::const_iterator rankIt = mStateRanking.find(state);
      if (rankIt == mStateRanking.end()) {
        // If no ranking found, use default safe control (allow all controllable events)
        controlPattern = mSigmaCtrl;
      } else {
        const StateRanking& ranking = rankIt->second;
        
        // Construct control pattern based on ranking and Theorem 6.4
        constructControlPatternForState(state, ranking, RSet, ISet, controlPattern);
      }
      
      // Always include uncontrollable events
      EventSet uncontrollableEvents = mRAut.Alphabet() - mSigmaCtrl;
      controlPattern.InsertSet(uncontrollableEvents);
      
      rFeedback[state] = controlPattern;
    }
  }
  
private:
  /** construct control pattern for a specific state based on its ranking */
  void constructControlPatternForState(Idx state, const StateRanking& ranking,
                                      const StateSet& RSet, const StateSet& ISet,
                                      EventSet& rPattern) const {
    rPattern.Clear();
    
    // Get all outgoing transitions from this state
    TransSet::Iterator tit = mRAut.TransRelBegin(state);
    TransSet::Iterator tit_end = mRAut.TransRelEnd(state);
    
    for (; tit != tit_end; ++tit) {
      Idx event = tit->Ev;
      Idx successor = tit->X2;
      
      // Check if this event should be enabled based on control strategy
      bool enableEvent = false;
      
      // Get successor's ranking if it exists
      StateRankingMap::const_iterator succRankIt = mStateRanking.find(successor);
      
      if (succRankIt != mStateRanking.end()) {
        const StateRanking& succRanking = succRankIt->second;
        
        // Apply control rules based on Theorem 6.4:
        // 1. If successor has lower or equal ranking, enable the event
        // 2. If we're in I_p and successor is in I_p, enable to maintain invariance
        // 3. If successor is in R_p and we need to visit R_p, enable the event
        
        if (succRanking < ranking || succRanking.muLevel == ranking.muLevel) {
          enableEvent = true;
        } else if (ISet.Exists(state) && ISet.Exists(successor)) {
          enableEvent = true; // Stay within invariant set
        } else if (RSet.Exists(successor) && ranking.branchType == 0) {
          enableEvent = true; // Allow reaching recurrent set when needed
        }
      } else {
        // If successor is not controllable, be conservative
        if (ISet.Exists(successor)) {
          enableEvent = true; // Safe to go to invariant states
        }
      }
      
      // Always enable uncontrollable events
      if (!mSigmaCtrl.Exists(event)) {
        enableEvent = true;
      }
      
      if (enableEvent) {
        rPattern.Insert(event);
      }
    }
    
    // Ensure we have at least one enabled controllable event to avoid deadlock
    bool hasControllableEvent = false;
    for (EventSet::Iterator eit = rPattern.Begin(); eit != rPattern.End(); ++eit) {
      if (mSigmaCtrl.Exists(*eit)) {
        hasControllableEvent = true;
        break;
      }
    }
    
    // If no controllable events are enabled, enable all to avoid deadlock
    if (!hasControllableEvent) {
      rPattern.InsertSet(mSigmaCtrl);
    }
  }
};


/*
Compute control patterns and state feedback mapping
*/
void ComputeControlPattern(
  const RabinAutomaton& rRAut, 
  const EventSet& rSigmaCtrl,
  const StateRankingMap& stateRanking,
  const StateSet& rCtrlPfx,
  NameSet& rSigmaCtrlPattern) {
  
  // construct state feedback mapping
  StateFeedbackMap rStateFeedback;
  StateFeedbackConstructor feedbackConstructor(rRAut, rSigmaCtrl, stateRanking);
  feedbackConstructor.ConstructStateFeedback(rCtrlPfx, rStateFeedback);

  // Generate control patterns using ControlPatternGenerator
  std::vector<EventSet> controlPatterns = ControlPatternGenerator::GenerateControlPatterns(
    rRAut.Alphabet(), rSigmaCtrl);
  
  FD_DF("ComputeControlPattern(): Generated " << controlPatterns.size() << " control patterns");
  
  // Convert control patterns to symbolic representation in rSigmaCtrlPattern
  for (size_t i = 0; i < controlPatterns.size(); ++i) {
    const EventSet& pattern = controlPatterns[i];
    pattern.Write();
    
    // Create pattern name (e.g., "G1", "G2", etc.)
    std::string patternName = "G" + std::to_string(i + 1);
  }
  
  // Map each state to its corresponding control pattern
  for (const auto& pair : rStateFeedback) {
    std::string stateName = rRAut.StateName(pair.first);
    if (stateName.empty()) {
      stateName = std::to_string(pair.first); // Fallback to numeric representation
    }
    
    const EventSet& statePattern = pair.second;
    
    // Find which control pattern matches this state's feedback
    std::string matchingPattern = "Unknown";
    for (size_t i = 0; i < controlPatterns.size(); ++i) {
      const EventSet& pattern = controlPatterns[i];
      
      // Check if the state's feedback pattern matches this control pattern
      if (statePattern == pattern) {
        matchingPattern = "G" + std::to_string(i + 1);
        break;
      }
    }
    
    // Create state-to-pattern mapping: "stateName_PatternName"
    std::string statePatternMapping = stateName + "_" + matchingPattern;
    rSigmaCtrlPattern.Insert(statePatternMapping);
  }

  FD_DF("ComputeControlPattern(): computed controllable prefix with " 
        << rCtrlPfx.Size() << " states and " << rStateFeedback.size() << " feedback entries");
}

void RabinCtrlPfxWithFeedback(
  const RabinAutomaton& rRAut, const EventSet& rSigmaCtrl, NameSet& rSigmaCtrlPattern) {
  
  // we can only handle one Rabin pair
  if(rRAut.RabinAcceptance().Size()!=1){
    std::stringstream errstr;
    errstr << "the current implementation requires exactly one Rabin pair";
    throw Exception("RabinCtrlPfxWithFeedback", errstr.str(), 80);
  }
  
  // set up various helper
  TransSetX2EvX1 revtrans(rRAut.TransRel());
  EventSet sigctrl(rSigmaCtrl);
  
  // State ranking map to track fixpoint computation
  StateRankingMap stateRanking;
  
  // have operator with ranking capability
  RabinInvDynCtrlWithRanking ctrl(rRAut, revtrans, sigctrl);
  ctrl.SetStateRankingMap(&stateRanking);

  StateSet rCtrlPfx; 
  // run computation
  ctrl.Evaluate(rCtrlPfx);
  
  // construct state feedback mapping and compute control patterns
  ComputeControlPattern(rRAut, sigctrl, stateRanking, rCtrlPfx, rSigmaCtrlPattern);
};



} // namespace faudes

