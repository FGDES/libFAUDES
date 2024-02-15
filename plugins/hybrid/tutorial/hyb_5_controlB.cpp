/** @file hyb_5_controlB.cpp  

Tutorial, hybrid systems plugin. This tutorial demonstrates
how to compute a finite abstraction for the purpose of
control. 

@ingroup Tutorials 

@include hyb_5_ControlB.cpp

*/

#include "libfaudes.h"

// make the faudes namespace available to our program
using namespace faudes;

/**
 * In this example, dynamics are given programtically, i.e. by
 * a customised instance of CompatibleStates as opposed to the
 * ready-to-use versions for DES or LHA.
 *
 * The plant we implement consits simple tank with two input
 * symbols for fill '+' and drain '-' and two output symbols for 
 * full 'F' and and empty 'E'. Inputs symbols and output symbols
 * alternate.
 */

class FAUDES_API ExbCompatibleStates : public CompatibleStates {

protected:
  /** one single "real" interval of compatible continuous states */
  double mXmin;
  double mXmax;
  bool mLopen;
  bool mHopen;
  /** recent input as discrete state, values "+", "-", none "~", init "0" */
  char mU;
  /** transition result */
  std::map<Idx, ExbCompatibleStates*> mReachSets;
  /** event symbol table (cosmetic) */
  const EventSet& rAlphabet;

public:

  /** construct/destruct */
  ExbCompatibleStates(const EventSet& alph) : 
   CompatibleStates(), 
   rAlphabet(alph) {
  };
  ~ExbCompatibleStates(void)  { 
   Clear(); 
  };

  /** allway initialisation with no constraints, i.e., time invariant system */
  void InitialiseFull(void) {
    mXmin=0; mXmax=30; mLopen=false; mHopen=false; mU='0'; 
  };
  void InitialiseConstraint() { 
    InitialiseFull(); 
  }

  /** debug dump */
  void DWrite(void) const {
    std::cout << "continuous state set: ";
    if(mLopen) std::cout << "(";
    else std::cout << "[";
    std::cout << mXmin << ", " << mXmax;
    if(mHopen) std::cout << ")";
    else std::cout << "]";
    std::cout << "  last input " << mU << std::endl;    
  }

  /** implement dynamics */
  virtual void ExecuteTransitions(void) {
    FD_DF("ExampleB: execute dynamics");
    Clear();
    ExbCompatibleStates* ncs;
    switch(mU) {
    // last event was an input symbol, thus we generate an output sybol
    case '+':
    case '-':
    case '0':
      // can we generate 'E'?
      if( mXmin < 15 || ((mXmin == 15) && (mLopen == false)) ) {
	ncs=new ExbCompatibleStates(rAlphabet);
	ncs->mXmin=mXmin;
	ncs->mLopen = mLopen;
	ncs->mXmax =  (mXmax <= 15 ? mXmax : 15);
	ncs->mHopen = (mXmax <= 15 ? mHopen : false);
	ncs->mU='~';
	mReachSets[rAlphabet.Index("E")]=ncs;
      }	
      // can we generate 'F'?
      if(mXmax > 15) {
	ncs=new ExbCompatibleStates(rAlphabet);
	ncs->mXmax=mXmax;
	ncs->mHopen = mHopen;
	ncs->mXmin =  (mXmin > 15 ? mXmin : 15);
	ncs->mLopen = (mXmin > 15 ? mLopen : true);
	mReachSets[rAlphabet.Index("F")]=ncs;
	ncs->mU='~';
      }
      break;
    // last event was an output symbol, this we accept and execute any input symbol 
    case '~':
      // perform '+'
      ncs=new ExbCompatibleStates(rAlphabet);
      ncs->mXmax =mXmax+10;
      ncs->mHopen = mHopen;
      ncs->mXmin = mXmin+10;
      ncs->mLopen = mLopen;
      if(ncs->mXmax>=30) {ncs->mXmax=30; ncs->mHopen=false;}
      if(ncs->mXmin>=30) {ncs->mXmin=30; ncs->mLopen=false;}
      mReachSets[rAlphabet.Index("+")]=ncs;
      ncs->mU='+';
      // perform '-'
      ncs=new ExbCompatibleStates(rAlphabet);
      ncs->mXmax =mXmax-10;
      ncs->mHopen = mHopen;
      ncs->mXmin = mXmin-10;
      ncs->mLopen = mLopen;
      if(ncs->mXmax<=0) {ncs->mXmax=0; ncs->mHopen=false;}
      if(ncs->mXmin<=0) {ncs->mXmin=0; ncs->mLopen=false;}
      mReachSets[rAlphabet.Index("-")]=ncs;
      ncs->mU='-';
    }
    FD_DF("ExampleB: execute dynamics: ok.");
  }

  // read out
  ExbCompatibleStates* TakeByEvent(Idx ev) {
    std::map<Idx, ExbCompatibleStates*>::iterator sit=mReachSets.find(ev);
    if(sit==mReachSets.end()) return 0;
    ExbCompatibleStates* res= sit->second;
    sit->second=0;
    return res;
  }  

  // support
  void Clear(void) {
    std::map<Idx, ExbCompatibleStates*>::iterator sit=mReachSets.begin();
    std::map<Idx, ExbCompatibleStates*>::iterator sit_end=mReachSets.end();
    for(;sit!=sit_end;++sit) {
      if(sit->second) delete sit->second;
    }
    mReachSets.clear();
  }

};  



/** Run the tutorial */
int main() {

  // have alphabet
  EventSet alph;
  alph.FromString("<A> F E \"+\" \"-\" </A>");

  // compile l-complete abstraction
  ExbCompatibleStates* comp = new ExbCompatibleStates(alph);
  comp->InitialiseFull();
  Experiment* exp = new Experiment(alph);
  exp->InitialStates(comp);
  LbdAbstraction tivabs;
  tivabs.Experiment(exp);
  tivabs.RefineUniformly(4);
  tivabs.Experiment().DWrite();
  Generator lcabs = tivabs.TivAbstraction();
  lcabs.StateNamesEnabled(false);
  //StateMin(lcabs,lcabs);
  lcabs.GraphWrite("tmp_hyb_5tiv2.png");
 
  // have a specification
  Generator spec;
  spec.FromString(
    "<Generator> <T> 1 E 2 1 F 2 2 \"+\" 3 2 \"-\" 3 3 E 4 3 F 4 4 - 5 4 \"+\" 5 5 F 5 5 \"+\" 5 5 \"-\" 5 </T> <I> 1 </I></Generator>");

  // set up uncontrollable events
  EventSet ucevents;
  ucevents.FromString("<A> E F </A>");

  // learning by doing
  while(1) {

   // synthesise controller
  Generator loop;
  loop.StateNamesEnabled(true);
  spec.StateNamesEnabled(true);
  lcabs.StateNamesEnabled(true);
  ProductCompositionMap psmap;
  Parallel(lcabs,spec,psmap,loop);
  while(1) {
    Idx sz =loop.Size();
    loop.Complete();
    loop.Accessible();
    StateSet fail;
    StateSet::Iterator sit=loop.StatesBegin();
    StateSet::Iterator sit_end=loop.StatesEnd();
    for(;sit!=sit_end;++sit) {
      Idx pstate = psmap.Arg1State(*sit);
      EventSet penabled=lcabs.ActiveEventSet(pstate);
      EventSet lenabled=loop.ActiveEventSet(*sit);
      if(penabled * ucevents <= lenabled) continue;
      fail.Insert(*sit);
    }
    loop.DelStates(fail);
    if(loop.Size()==sz) break;
  }
  if(loop.Size()>0) {
    std::cout << " Synthesis succeeded ";
    loop.GraphWrite("tmp_hyb_5sup.png");
    break;
  }  

  
  // refinement candidates
  StateSet leaves = tivabs.Experiment().Leaves();
  std::cout << "refinement candidates (leaves)" << std::endl;
  leaves.Write();
    
  // search for refinement candidates, "Step 1/2"
  Generator loop12;
  Generator spec12;
  spec12.StateNamesEnabled(false);
  Parallel(lcabs,spec,psmap,spec12);
  StateSet::Iterator sit=spec12.StatesBegin();
  StateSet::Iterator sit_end=spec12.StatesEnd();
  for(;sit!=sit_end;++sit) {
    if(!leaves.Exists(psmap.Arg1State(*sit))) continue;
    spec12.ClrTransitions(*sit);
    EventSet::Iterator eit=spec12.AlphabetBegin();
    EventSet::Iterator eit_end=spec12.AlphabetEnd();
    for(;eit!=eit_end;++eit) spec12.SetTransition(*sit,*eit,*sit);
  }
  spec12.Accessible();
  spec12.GraphWrite("tmp_hyb_5spec12.png");
  loop12.StateNamesEnabled(true);
  spec12.StateNamesEnabled(true);
  lcabs.StateNamesEnabled(true);
  Parallel(lcabs,spec12,psmap,loop12);
  //loop12.GraphWrite("tmp_hyb_5ctrllp.png");
  while(1) {
    Idx sz =loop12.Size();
    loop12.Complete();
    loop12.Accessible();
    StateSet fail;
    sit=loop12.StatesBegin();
    sit_end=loop12.StatesEnd();
    for(;sit!=sit_end;++sit) {
      Idx pstate = psmap.Arg1State(*sit);
      EventSet penabled=lcabs.ActiveEventSet(pstate);
      EventSet lenabled=loop12.ActiveEventSet(*sit);
      if(penabled * ucevents <= lenabled) continue;
      fail.Insert(*sit);
    }
    loop12.DelStates(fail);
    if(loop12.Size()==sz) break;
  }
  loop12.GraphWrite("tmp_hyb_5ctrl12.png");
  StateSet step12;
  sit=loop12.StatesBegin();
  sit_end=loop12.StatesEnd();
  for(;sit!=sit_end;++sit)
    if(leaves.Exists(psmap.Arg1State(*sit))) step12.Insert(psmap.Arg1State(*sit));
  std::cout << "refinement candidates (step12)" << std::endl;
  step12.Write();
  
  
  // search for refinement candidates, "cpx"
  Generator loopcpx;
  loopcpx.StateNamesEnabled(false);
  Parallel(lcabs,spec,psmap,loopcpx);
  while(1) {
    Idx sz =loopcpx.Size();
    loopcpx.Complete();
    StateSet fail;
    StateSet::Iterator sit=loopcpx.StatesBegin();
    StateSet::Iterator sit_end=loopcpx.StatesEnd();
    for(;sit!=sit_end;++sit) {
      Idx pstate = psmap.Arg1State(*sit);
      EventSet penabled=lcabs.ActiveEventSet(pstate);
      EventSet lenabled=loopcpx.ActiveEventSet(*sit);
      if(penabled * ucevents <= lenabled) continue;
      fail.Insert(*sit);
    }
    loopcpx.DelStates(fail);
    if(loopcpx.Size()==sz) break;
  }
  loopcpx.GraphWrite("tmp_hyb_5ctrlcpx.png");
  StateSet qcpx;
  sit=loopcpx.StatesBegin();
  sit_end=loopcpx.StatesEnd();
  for(;sit!=sit_end;++sit)
    qcpx.Insert(psmap.Arg1State(*sit));
  StateSet stepcpx = leaves - qcpx;
  std::cout << "refinement candidates (cpx)" << std::endl;
  stepcpx.Write();

  

  // do refinement
  StateSet refine = stepcpx * step12;
  if(refine.Size()==0) break;
  sit=refine.Begin();
  sit_end=refine.End();
  for(;sit!=sit_end;++sit)
    tivabs.RefineAt(*sit);    
  lcabs = tivabs.TivAbstraction();

  
  }


  // done
  return 0;
}



