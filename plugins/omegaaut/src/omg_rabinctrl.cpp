/** @file omg_rabinctrl.cpp

Operations regarding omega languages accepted by Rabin automata

*/

/* FAU Discrete Event Systems Library (libFAUDES)

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
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

  

#include "omg_rabinctrl.h"
#include "syn_include.h"

namespace faudes {


/*
Helper: plain inverse dynamics operator theta
*/
class  RabinInvDynamicsTheta2 : public StateSetOperator {
public:
  /** construct to record context */
  RabinInvDynamicsTheta2(
    const vGenerator& rGenerator, const TransSetX2EvX1& rReverseTransRel, const EventSet& rSigmaCtrl)
  :
    StateSetOperator(),
    rGen(rGenerator),
    rMarkedStates(rGen.MarkedStates()),
    rTransRel(rGen.TransRel()),
    rRevTransRel(rReverseTransRel),    
    mSigmaCtrl(rSigmaCtrl)
  {
    FD_DF("RabinInvDynamicsTheta2(): instantiated from " << mrGen.Name());
    rGen.SWrite();
    Name("invdyn_op([Z1,Z2])");
    mArgNames= std::vector<std::string>{"Z1","Z2"};
    mArgCount=2;
  };
  /** overaall stateset */
  virtual const StateSet& Domain(void) const {
    return rGen.States();
  }
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) const {
    // convenience acsesors
    const StateSet& Z1=rArgs.At(0);
    const StateSet& Z2=rArgs.At(1);
    // do operate
    //
    // Cite Thistle/Wonham "Control of w-Automata, Church's Problem, and the Emptiness
    // Problem for Tree w-Automata", 1992, Def 8.2:
    //
    // theta(X1,X2) = "the set of states, from which the automaton can be controlled to
    //    enter X1 union X2 in a single transition without being prevented from entering X1."
    //
    // Rephrase: X1 is the target T, X1 union X2 is the domain D; control such that successors stay
    // within within D and there is the chance to enter the T.
    //
    // We use "Z1/Z2" argument names to avoid confusion with libFAUDES naming conventions.
    //
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
  /** record context references */
  const vGenerator& rGen;
  const StateSet& rMarkedStates;
  const TransSet& rTransRel;
  const TransSetX2EvX1& rRevTransRel;
  EventSet mSigmaCtrl;
};      
    

/*
Helper: inverse dynamics operator theta, call with 4 Args as needed by theta-tilde
*/
class  RabinInvDynamicsTheta4 : public RabinInvDynamicsTheta2 {
public:
  /** construct to record context */
  RabinInvDynamicsTheta4(
    const vGenerator& rGenerator, const TransSetX2EvX1& rReverseTransRel, const EventSet& rSigmaCtrl)
  :
    RabinInvDynamicsTheta2(rGenerator,rReverseTransRel,rSigmaCtrl)
  {
    FD_DF("RabinInvDyanmicsTheta4(): instantiated from " << mrGen.Name());
    Name("invdyn_op([Y1,Y2,Y3,Y4])");
    mArgNames= std::vector<std::string>{"Y1","Y2","Y3","Y4"};
    mArgCount=4;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) const {
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
    //
    // Cite Thistle/Wonham "Control of w-Automata, Church's Problem, and the Emptiness
    // Problem for Tree w-Automata", 1992, Def 8.3:
    //
    // "theta_tilde(X1,X2)=nu X3 mu X4 theta( X1 union (X4 - R), (X3 - R) intersect X2 ) ")
    //
    // We implement the core formular  (i.e. without the nu/mu iteration).
    // We use "Y1/Y2/Y3/Y4" and "Z1/Z2" argument names to avoid confusion with libFAUDES naming
    // conventions.
    //
    Z1= Y1 + (Y4 - rMarkedStates);
    Z2= Y2 * (Y3 - rMarkedStates);
    RabinInvDynamicsTheta2::DoEvaluate(args,rRes);
  };
};      
    

/*
Helper: plain inverse dynamics operator theta tilde
*/
class  RabinInvDynamicsThetaTilde2 : public RabinInvDynamicsTheta4 {
public:
  /** construct to record context */
  RabinInvDynamicsThetaTilde2(
    const vGenerator& rGenerator, const TransSetX2EvX1& rReverseTransRel, const EventSet& rSigmaCtrl)
  :
    RabinInvDynamicsTheta4(rGenerator,rReverseTransRel,rSigmaCtrl),
    mTheta4(rGenerator,rReverseTransRel,rSigmaCtrl),
    mMuTheta4(mTheta4),
    mNuMuTheta4(mMuTheta4)
  {
    FD_DF("RabinInvDynamicsThetaTilde2(): instantiated from " << mrGen.Name());
    Name("theta_tilde_op([W1,W2])");
    mArgNames= std::vector<std::string>{"W1","W2"};
    mArgCount=2;
  };
protected:  
  /** actual operator implementation */
  virtual void DoEvaluate(StateSetVector& rArgs, StateSet& rRes) const {
    // do operate
    //
    // Cite Thistle/Wonham "Control of w-Automata, Church's Problem, and the Emptiness
    // Problem for Tree w-Automata", 1992, Def 8.3:
    //
    // "theta_tilde(X1,X2)=nu X3 mu X4 theta( X1 union (X4 - R), (X3 - R) intersect X2 ) ")
    //
    // We implement the nu/mu iteration).
    // We use "W1/W2"
    mNuMuTheta4.Evaluate(rArgs, rRes);
  };
  // have context
  RabinInvDynamicsTheta4 mTheta4;
  MuIteration mMuTheta4;
  NuIteration mNuMuTheta4;
};      

  
void RabinCtrlPfx(
  const RabinAutomaton& rRAut, const EventSet& rSigmaCtrl,
  StateSet& rCtrlPfx){
  // set up various helper
  TransSetX2EvX1 revtrans(rRAut.TransRel());
  EventSet sigctrl(rSigmaCtrl);
  // have inv dynamics operator
  RabinInvDynamicsTheta4 theta4(rRAut,revtrans,sigctrl);
  // have theta tilde by inner nu-mu-iterationr
  MuIteration mu_theta4(theta4);
  NuIteration nu_mu_theta4(mu_theta4); 
};



} // namespace faudes

