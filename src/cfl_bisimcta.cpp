/** @file cfl_bisimcta.cpp  Bisimulation relations (CTA)

    Functions to compute bisimulation relations on dynamic systems (represented
    by non-deterministic finite automata). 

    More specifically, we we implement algorithms to obtain ordinary/delayed/weak 
    bisimulations partitions based on change-tracking. In large, these implementations
    are expected to perform better than the classical variants found in cfl_bisimulation.h".

    This code was originally developed by Yiheng Tang in the context of compositional
    verification in 2020/21.

**/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2020/21, Yiheng Tang
   Copyright (C) 2021,    Thomas Moor
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

#include "cfl_bisimcta.h"


// *******************************
// *******************************
// Preliminaries
// *******************************
// *******************************

#define BISIM_VERB1(msg)						\
  { if(faudes::ConsoleOut::G()->Verb() >=1 ) { \
      std::ostringstream cfl_line; cfl_line << msg << std::endl; faudes::ConsoleOut::G()->Write(cfl_line.str(),0,0,0);} }
#define BISIM_VERB0(msg) \
  { if(faudes::ConsoleOut::G()->Verb() >=0 ) { \
      std::ostringstream cfl_line; cfl_line << msg << std::endl; faudes::ConsoleOut::G()->Write(cfl_line.str(),0,0,0);} }

namespace faudes {


// ****************************************************************************************************************************
// ****************************************************************************************************************************
// Private part of header
// ****************************************************************************************************************************
// ****************************************************************************************************************************


/*!
 * \brief The Bisimulation class
 * The following class implements a basic normal bisimulation partition algorithms and derives a class
 * for partitioning w.r.t. delayed bisimulation and weak bisimulation . All these algorithms are introduced
 * in "Computing Maximal Weak and Other Bisimulations" from Alexandre Boulgakov et. al. (2016).
 * The utilised normal bisimulation algorithm originates from the "change tracking algorithm"
 * from Stefan Blom and Simona Orzan (see ref. of cited paper). Besides, the current paper uses topological
 * sorted states to avoid computing saturated transitions explicitly when computing delayed and weak bisimulation.
 *
 * IMPORTANT NOTE: both algorithms for delayed and weak bisimulation requires tau-loop free automaton. This shall
 * be done extern beforehand by using e.g. FactorTauLoops(Generator &rGen, const Idx &rSilent).
 */

class BisimulationCTA{
public:
    BisimulationCTA(const Generator &rGen, const std::vector<StateSet> &rPrePartition) : mGen(&rGen), mPrePartition(rPrePartition){}
    virtual ~BisimulationCTA() = default;

    /*!
     * \brief BisimulationPartition
     * wrapper
     */
    virtual void ComputePartition(std::list<StateSet>& rResult);

protected:

    // encoded data structure
    struct State {
        Idx id;  // source state idx
        std::vector< std::vector<Idx>> suc; // successors by event
        std::vector< Idx > pre; // predecessors (neglect event, only for figuring affected)
        std::vector< std::set<Idx> > cafter; // cafter by event (the std::set<Idx> is the set of c-values)
        std::vector< Idx > evs; // active event set, only for pre-partition. (indicates "delayed" active ev in case of daleyd- or weak-bisim)
        Idx c = 0; // this shall be replaced by 1 for each state (except #0 state)

        // only for derived classes, recored predececssors with silent event.
        // in such cases, "pre" records only non-tau predecessors
        std::vector< Idx > taupre;
    };
    std::vector<State> mStates;        // vector of all states  [starting with 1 -- use min-index for debugging]
    std::vector<Idx> mEvents;          // vector of all events [starting with 1]

    /*!
     * \brief EncodeData
     * encode source generator into vector-form data structures to accelerate the iteration
     *
     * NOTE: this function is virtual since derived classes, in the context of
     * abstraction, have different steps
     */
    virtual void EncodeData(void);    

    /*!
     * \brief FirstStepApproximation (see Fig. 2 (b) in cited paper)
     * Prepartition states according to their active events.
     * If a prepartition already exists, this function will refine the prepartition
     *
     * NOTE: this function is also utilised by delayed- and weak-bisimulation since in both
     * cases, State.evs neglect tau (as tau is always active)
     *
     */
    void FirstStepApproximation(void);

    /*!
     * \brief RefineChanged
     * refine equivalence classes contained affected nodes in the last iteration
     * (see Fig. 5 in cited paper)
     */
    void RefineChanged(void);   

    /*!
     * \brief GenerateResult
     * generate partition result w.r.t. original state indices (without trivial classes)
     * \param rResult partition w.r.t. original state indices without trivial classes
     */
    void GenerateResult(std::list<StateSet>& rResult);

    /*! keep the original generator */
    const Generator* mGen;

    /*! state prepartition with original state idx (the same as state label as in cited paper). Empty for trivial prepartition. */
    const std::vector<StateSet> mPrePartition;

    /*! persisted data structures, see cited paper. */
    std::vector<bool> mAffected;
    std::vector<bool> mChanged;
    Idx mCmax; // maximal value of c in the current partition

    /*! constant parameters for encoding*/
    Idx mStateSize; // state count, for vector resizing and iteration
    Idx mAlphSize; // event count, for vector resizing and iteration

    /*! the current (and also final) partition. Partition is represented
     *  by states sorted by c_value. Access mStates for c_value to get the exact partition.
     */
    std::vector<Idx> mPartition;

private:

    /*!
     * \brief ComputeBisimulation
     * perform the overall iteration
     */
    void ComputeBisimulation(void);

    /*!
     * \brief ComputeChangedAfters
     * Compute changed afters of each affected state. (see Fig 5. in cited paper)
     * Affected states are those having some successors that have changed class (namely c_value)
     * in the last iteration
     * NOTE: this function is made private in that derived classes compute cafters
     * w.r.t. silent event and these are accomplished by ComputeChangedDelayedAfters
     * and ComputeChangedObservedAfters, respectively
     */
    void ComputeChangedAfters(void);


};

/*!
 * \brief The DelayedBisimulation class
 * Derived from Bisimulation class. We implement the "two-pass change-tracking" algorithm from the cited paper.
 */
class AbstractBisimulationCTA : BisimulationCTA{
public:
    AbstractBisimulationCTA (const Generator& rGen, const Idx& rSilent, const Idx& rTaskFlag, const std::vector<StateSet>& rPrePartition)
        : BisimulationCTA(rGen,rPrePartition), mTau(rSilent), mTaskFlag(rTaskFlag){}

    virtual void ComputePartition(std::list<StateSet>& rResult);

private:
    /*!
     * \brief tau
     * persist index of original silent event
     */
    const Idx mTau;

    /*!
     * \brief mTaskFlag
     * flag for task:
     *      mTaskFlag == 1: delayed bisimulation
     *      mTaskFlag == 2: weak bisimulation
     */
    const Idx mTaskFlag;

    /*!
     * \brief EncodeData
     * basic preparation for bisimulation
     * NOTE: this function is virtual since derived classes, in the context of
     * abstraction, have different steps
     */
    virtual void EncodeData(void);

    /*!
     * \brief MarkTauAffected
     * perform a recursive Depth-First-Search to mark all tau* predecessors as affected
     * based on given state index rState.
     * NOTE: rState is also a tau* predecessor of itself, thus also affected
     * NOTE: as tau-loop-free is guaranteed, no need to use visited list to avoid duplet
     * NOTE: in most cases, argument rAffected directly takes mAffected. The only
     *       exception is that in ComputeChangedObservedAfters, we need to buffer an intermediate
     *       affected-vector.
     */
    void MarkTauStarAffected(std::vector<bool>& rAffected, const Idx& rState);

    /*!
     * \brief ComputeDelayedBisimulation
     * perform the overall iteration based on different task flag value, see mTaskFlag
     */
    void ComputeAbstractBisimulation();


    /*!
     * \brief ComputeChangedDelayedAfters
     * see Fig. 10 of cited paper
     */
    void ComputeChangedDelayedAfters(void);

    /*!
     * \brief ComputeChangedObservedAfters
     * see Fig. 12 of cited paper
     */
    void ComputeChangedObservedAfters(void);

};



// *******************************
// *******************************
// topological sort
// *******************************
// *******************************


/*!
 * \brief The TopoSort class
 * perform a topological sort on states of a given automaton. if s1 is before s2 in the sort
 * then there is no path from s2 to s1. The algorithm can be found in
 * https://en.wikipedia.org/wiki/Topological_sorting
 * under depth-first search, which is considered as first invented by R. Tarjan in 1976.
 */
class TopoSort{
public:
    struct State{
        Idx id;
        bool permanent = 0;
        bool temporary = 0;
        std::vector<Idx> succs;
    };
    TopoSort (const Generator& rGen, const EventSet& rEvs);

    bool Sort(std::list<Idx>& result);

    bool Visit(State& rNode);

private:
    const Generator* mGen;
    Idx nxidx;
    std::vector<State> mStates;
    std::list<Idx> mResult;
};

/*!
 * \brief topoSort
 * wrapper for topological sortation
 * rEvs is the set of events which will be considered while sorting
 */
bool topoSort (const Generator& rGen, const EventSet& rEvs, std::list<Idx>& result);




// ****************************************************************************************************************************
// ****************************************************************************************************************************
// Implementation part
// ****************************************************************************************************************************
// ****************************************************************************************************************************


// *******************************
// *******************************
// Transition saturation
// *******************************
// *******************************


// YT: note flag: 1 := delayed bisim;   2 := weak bisim
void ExtendTransRel(Generator& rGen, const EventSet& rSilent, const Idx& rFlag) {

  if (rSilent.Empty()) return;
  // HELPERS:
  TransSet::Iterator tit1;
  TransSet::Iterator tit1_end;
  TransSet::Iterator tit2;
  TransSet::Iterator tit2_end;
  TransSet newtrans;

  // initialize result with original transitionrelation
  TransSet xTrans=rGen.TransRel();
  newtrans = rGen.TransRel(); // initialize iteration
  // loop for fixpoint
  while(!newtrans.Empty()) {
    // store new transitions for next iteration
    TransSet nextnewtrans;
    // loop over all transitions in newtrans
    tit1=newtrans.Begin();
    tit1_end=newtrans.End();
    for(;tit1!=tit1_end; ++tit1) {
      // if it is silent add transition to non silent successor directly
      if(rSilent.Exists(tit1->Ev)) {
         tit2=xTrans.Begin(tit1->X2);
         tit2_end=xTrans.End(tit1->X2);
         for(;tit2!=tit2_end; ++tit2) {
//         if(!rSilentAlphabet.Exists(tit2->Ev)) // tmoor 18-09-2019
           if (!xTrans.Exists(tit1->X1,tit2->Ev,tit2->X2))
             nextnewtrans.Insert(tit1->X1,tit2->Ev,tit2->X2);
        }
      }
      else if (rFlag == 2){ // in case for observed transition (flag == 2), add non-silent transition to silent successor
          tit2=xTrans.Begin(tit1->X2);
          tit2_end=xTrans.End(tit1->X2);
          for(;tit2!=tit2_end; ++tit2) {
            if(rSilent.Exists(tit2->Ev)) // silent successor
              if (!xTrans.Exists(tit1->X1,tit1->Ev,tit2->X2))
                nextnewtrans.Insert(tit1->X1,tit1->Ev,tit2->X2);
         }
      }
    }
    // insert new transitions
    xTrans.InsertSet(nextnewtrans);
    // update trans for next iteration.
    newtrans = nextnewtrans;
  }
  rGen.InjectTransRel(xTrans);
}

void InstallSelfloops(Generator &rGen, const EventSet& rEvs){
    if (rEvs.Empty()) return;
    StateSet::Iterator sit = rGen.StatesBegin();
    for(;sit!=rGen.StatesEnd();sit++){
      EventSet::Iterator eit = rEvs.Begin();
      for(;eit!=rEvs.End();eit++)
        rGen.SetTransition(*sit,*eit,*sit);
    }
}


// *******************************
// *******************************
// topological sort
// *******************************
// *******************************

TopoSort::TopoSort (const Generator& rGen, const EventSet &rEvs) {
    mGen = &rGen;
    nxidx=1;

    // encode transition relation [effectively buffer log-n search]
    Idx max=0;
    std::map<Idx,Idx> smap;
    mStates.resize(mGen->States().Size()+1);
    StateSet::Iterator sit=mGen->StatesBegin();
    for(; sit != mGen->StatesEnd(); ++sit) {
        smap[*sit]=++max;
        mStates[max].id=*sit;
    }
    TransSet::Iterator tit=mGen->TransRelBegin();
    for(; tit != mGen->TransRelEnd(); ++tit) {
        if (rEvs.Exists(tit->Ev))
            mStates[smap[tit->X1]].succs.push_back(smap[tit->X2]);
    }
}

bool TopoSort::Sort (std::list<Idx> &result){
    std::vector<State>::iterator sit = mStates.begin();
    sit++;
    for(;sit!=mStates.end();sit++){
        if ((*sit).permanent) continue;
        if (!Visit(*sit)) return false;
    }
    result = mResult;
    return true;
}

bool TopoSort::Visit(State &rState){
    if (rState.permanent) return true;
    if (rState.temporary) return false;
    rState.temporary = 1;
    std::vector<Idx>::iterator sit = rState.succs.begin();
    for(;sit!=rState.succs.end();sit++){
        if(!Visit(mStates[*sit])) return false;
    }
    rState.temporary = 0;
    rState.permanent = 1;
    mResult.push_front(rState.id);
    return true;
}

bool topoSort(const Generator& rGen, const EventSet &rEvs, std::list<Idx>& result){
    TopoSort topo(rGen, rEvs);
    return topo.Sort(result);
}



// *****************************************************
// *****************************************************
// (strong) bisimulation
// *****************************************************
// *****************************************************

void BisimulationCTA::EncodeData(){
    mStateSize = mGen->States().Size()+1;
    mAlphSize = mGen->Alphabet().Size()+1;

    // encode transition relation [effectively buffer log-n search]
    std::map<Idx,Idx> smap;
    std::map<Idx,Idx> emap;
    Idx max=0;
    mEvents.resize(mAlphSize);
    EventSet::Iterator eit= mGen->AlphabetBegin();
    for(; eit != mGen->AlphabetEnd(); ++eit) {
      emap[*eit]=++max;
      mEvents[max]=*eit;
    }
    if (mPrePartition.empty())
        mCmax = 1;
    else
        mCmax = mPrePartition.size();
    max=0;
    mStates.resize(mStateSize);
    mPartition.resize(mStateSize-1);
    StateSet::Iterator sit=mGen->StatesBegin();
    for(; sit != mGen->StatesEnd(); ++sit) {
      mPartition[max] = max+1; // trivial partition
      smap[*sit]=++max;
      mStates[max].id=*sit;
      mStates[max].suc.resize(mAlphSize);
      mStates[max].cafter.resize(mAlphSize);
      // initialize cafter
      const EventSet& activeevs = mGen->ActiveEventSet(*sit);
      EventSet::Iterator eit = activeevs.Begin();
      for(;eit!=activeevs.End();eit++)
          mStates[max].evs.push_back(emap[*eit]);
      // initialize c value. c = 1 if no prepartition; c = (index of prepartion) + 1 if prepartition defined
      if (mPrePartition.empty()) mStates[max].c = mCmax;
      else{
          Idx prepit = 0;
          for(;prepit<mPrePartition.size();prepit++){
              if (mPrePartition[prepit].Exists(*sit)) break;
          }
          if (prepit == mPrePartition.size())
              throw Exception("EncodeData:: ", "invalide prepartition. State "+ ToStringInteger(*sit) + "is not allocated.", 100);
          mStates[max].c = prepit+1;
      }
    }
    TransSet::Iterator tit=mGen->TransRelBegin();
    for(; tit != mGen->TransRelEnd(); ++tit) {
      mStates[smap[tit->X1]].suc[emap[tit->Ev]].push_back(smap[tit->X2]);
      // since predecessors are recorded neglecting events, we (shall) check for
      // duplication before inserting. This is actually optional.
      std::vector<Idx>::const_iterator preit = mStates[smap[tit->X2]].pre.begin();
      for (;preit!=mStates[smap[tit->X2]].pre.end();preit++){
          if (*preit==smap[tit->X1]) break;
      }
      if (preit==mStates[smap[tit->X2]].pre.end())
          mStates[smap[tit->X2]].pre.push_back(smap[tit->X1]);
    }

    // initialize affected and changed vector
    mAffected.resize(mStateSize);
    mAffected[0] = 0;
    mChanged.resize(mStateSize);
    mChanged[0] = 0;
    Idx iit = 1;
    for(;iit<mStateSize;iit++){
        mAffected[iit] = 0;
        mChanged[iit] = 1;
    }
}

void BisimulationCTA::FirstStepApproximation(){
    // set up (modified) Phi as in the cited paper. pairs (state, (activeEvs,c-value)) by lex-sort
    std::sort(mPartition.begin(),mPartition.end(), [this](const Idx& state1, const Idx& state2){
        if (this->mStates[state1].evs < this->mStates[state2].evs) return 1;
        if (this->mStates[state1].evs > this->mStates[state2].evs) return 0;
        if (this->mStates[state1].c < this->mStates[state2].c) return 1;
        return 0;
    });
    // assign new c_values
    std::vector<Idx> evs(1);
    evs[0] = 0; //initialize invalid active event set
    Idx c = 0; // initialize invalide c value
    std::vector<Idx>::const_iterator partit = mPartition.begin();
    for(;partit!=mPartition.end();partit++){
        if(mStates[*partit].evs != evs || mStates[*partit].c != c){
            evs = mStates[*partit].evs;
            c = mStates[*partit].c;
            mCmax++;
        }
        mStates[*partit].c = mCmax;
    }
}

void BisimulationCTA::ComputeChangedAfters(){
    // recompute mAffected
    std::fill(mAffected.begin(),mAffected.end(),0);
    Idx stateit = 1;
    for(;stateit<mStateSize;stateit++){
        if (!mChanged[stateit]) continue;
        std::vector<Idx>::iterator predit = mStates[stateit].pre.begin();
        for(;predit!=mStates[stateit].pre.end();predit++) mAffected[*predit] = 1;
    }
    // compute cafters
    stateit = 1;
    for (;stateit<mStateSize;stateit++){
        if (!mAffected[stateit]) continue;
        // clear cafter of this node
        std::set<Idx> emptyset;
        std::fill(mStates[stateit].cafter.begin(),mStates[stateit].cafter.end(),emptyset);
        // compute new cafter
        Idx sucevit = 1;
        for( ; sucevit<mAlphSize; sucevit++){
            std::vector<Idx>::const_iterator sucstateit = mStates[stateit].suc[sucevit].begin();
            for(;sucstateit!=mStates[stateit].suc[sucevit].end();sucstateit++)
                mStates[stateit].cafter[sucevit].insert(mStates[*sucstateit].c);
        }        
    }
}

void BisimulationCTA::RefineChanged(){


    std::fill(mChanged.begin(),mChanged.end(),0);
    std::vector<Idx>::iterator partit = mPartition.begin();
    while(partit!=mPartition.end()){
        // treat affected node is merged into the following step

        // get class containing affected node (line 25 in cited paper)
        // iterate on mPartition, i.e. mPartition must already sorted by c
        // we simply iterate forward to get the current class. if no states in class is affected, skip
        Idx c = mStates[*partit].c;
        std::list<Idx> eqclass;
        //std::vector<Idx>::iterator partit_rec = partit;
        bool affectedflag = false;
        while(mStates[*partit].c == c){
            eqclass.push_back(*partit);
            if (mAffected[*partit]) affectedflag = true;
            partit++;
            if (partit==mPartition.end()) break;
        }
        if (eqclass.size()==1 || !affectedflag) continue; // skip trivial class or non-affected class

        // reorder nodes in eqclasses by moving those in affected to the front (line 26 in cited paper)
        std::list<Idx>::iterator bound = std::partition(eqclass.begin(), eqclass.end(),
                    [this](const Idx& state){return this->mAffected[state];});

        // sort affected nodes by cafter (line 27. Note in cited paper line 27, it says sort "NOT" affected nodes.
        // this shall be a typo since all unaffected nodes in this class shall have the same cafter, as their
        // cafters are not changed from the last iteration)
        std::list<Idx> eqclass_aff; // unhook elements before bound
        eqclass_aff.splice(eqclass_aff.begin(),eqclass,eqclass.begin(),bound);
        eqclass_aff.sort([this](const Idx& state1, const Idx& state2){
            return this->mStates[state1].cafter < this->mStates[state2].cafter;});
        eqclass.splice(eqclass.begin(),eqclass_aff);// hook the sorted eqclass_aff again to the front of eqclass        

        // delete the largest set of states with the same cafter (line 28). c_value of these states are preserved
        std::list<Idx>::iterator eqclassit = eqclass.begin();
        std::vector<std::set<Idx>> maxCafter; // cafter corresponding to most states in the current class
        std::vector<std::set<Idx>> currentCafter; // initialize an invalid cafter for comparison
        Idx maxSize = 0;
        Idx currentSize = 0;
        for(;eqclassit!=eqclass.end();eqclassit++){
            if (mStates[*eqclassit].cafter!=currentCafter){
                if (currentSize > maxSize){ // update max if the latest record is greater
                    maxSize = currentSize;
                    maxCafter = currentCafter;
                }
                currentSize = 1;
                currentCafter = mStates[*eqclassit].cafter;
            }
            else{
                currentSize++;
            }
        }
        if (maxCafter.empty()) continue; // namely all states has the same cafter. no need to refine.

        // refine this class. The greatest refined class is removed and all other refined classes
        // will have new c_value (line 29-37)
        eqclass.remove_if([this, maxCafter](const Idx& state){
            return this->mStates[state].cafter==maxCafter;});
        currentCafter.clear();
        eqclassit = eqclass.begin();
        for(;eqclassit!=eqclass.end();eqclassit++){
            if(mStates[*eqclassit].cafter!=currentCafter){
                currentCafter = mStates[*eqclassit].cafter;
                mCmax++;
            }
            mStates[*eqclassit].c = mCmax;
            mChanged[*eqclassit] = 1;
        }
    }

    // sort by c
    std::sort(mPartition.begin(),mPartition.end(), [this](const Idx& state1, const Idx& state2){
        return this->mStates[state1].c<this->mStates[state2].c;
    });
}

// the wrapper
void BisimulationCTA::ComputeBisimulation(){
    BISIM_VERB1("Initializing data")
    EncodeData();
    BISIM_VERB1("Doing FirstStepApproximation")
    FirstStepApproximation();
    while (std::find(mChanged.begin(),mChanged.end(),1)!=mChanged.end()){
        BISIM_VERB1("Doing ComputeChangedAfters")
        ComputeChangedAfters();
        BISIM_VERB1("Doing RefineChanged")
        RefineChanged();
    }
}

void BisimulationCTA::GenerateResult(std::list<StateSet>& rResult){
    rResult.clear();
    Idx c = 0;
    std::vector<Idx>::const_iterator partit = mPartition.begin();
    StateSet eqclass;
    for(;partit!=mPartition.end();partit++){
        if (mStates[*partit].c != c){
            if (eqclass.Size()>1) rResult.push_back(eqclass);
            eqclass.Clear();
            eqclass.Insert(mStates[*partit].id);
            c = mStates[*partit].c;
        }
        else eqclass.Insert(mStates[*partit].id);
    }
    if (eqclass.Size()>1) rResult.push_back(eqclass); // insert the last partitio
}

void BisimulationCTA::ComputePartition(std::list<StateSet>& rResult){
    ComputeBisimulation();
    GenerateResult(rResult);
}

// *****************************************************
// *****************************************************
// delayed and weak bisimulation
// *****************************************************
// *****************************************************

void AbstractBisimulationCTA::EncodeData(){
    mStateSize = mGen->States().Size()+1;
    mAlphSize = mGen->Alphabet().Size(); // Note the difference with BisimulationCTA::EncodeData(). Index 0 is for silent event

    // encode transition relation [effectively buffer log-n search]
    std::map<Idx,Idx> smap;
    std::map<Idx,Idx> emap;

    Idx max=0;
    mEvents.resize(mAlphSize);
    EventSet::Iterator eit= mGen->AlphabetBegin();
    for(; eit != mGen->AlphabetEnd(); ++eit) {
        if (*eit != mTau){
            emap[*eit]=++max;
            mEvents[max]=*eit;
        }
        else{
            emap[*eit]=0;
            mEvents[0]=*eit;
        }
    }

    // perform topological sort, throw if with tau-loops
    std::list<Idx> topo;
    EventSet tau;
    tau.Insert(mTau);
    if (!topoSort(*mGen,tau,topo))
        throw Exception("DelayedBisimulationCTA::EncodeData(): ", "input automaton shall not have tau loop. Please factor"
                                                               "tau-loops before partition", 500);

    if (mPrePartition.empty())
        mCmax = 1;
    else
        mCmax = mPrePartition.size();
    max=0;
    mStates.resize(mStateSize);
    mPartition.resize(mStateSize-1);
    // iterate over topo sorted states BACKWARDS and install into mStates
    // mStates shall have the upstream order. this is a pure design feature
    // as all iterations will start from the downstream-most state in the topo sort.
    // By installing them backwards, we always start the iteration from the first state
    std::list<Idx>::const_reverse_iterator sit=topo.rbegin();
    for(; sit != topo.rend(); ++sit) {
        mPartition[max] = max+1; // trivial partition
        smap[*sit]=++max;
        mStates[max].id=*sit;        
        mStates[max].suc.resize(mAlphSize);
        mStates[max].cafter.resize(mAlphSize);
        // install DELAYED active events
        EventSet activeevs = mGen->ActiveEventSet(*sit); // direct active events
        TransSet::Iterator tit = mGen->TransRelBegin(*sit); // active events of one-step tau successors
        for(;tit!=mGen->TransRelEnd(*sit);tit++){
            if (tit->Ev!=mTau) continue;
            // since active events are encoded in vectors, iterate over active event vector of successor
            std::vector<Idx>::const_iterator eit = mStates[smap[tit->X2]].evs.begin(); // the state smap[tit->X2] must have been encoded
            for(;eit!=mStates[smap[tit->X2]].evs.end();eit++){
                activeevs.Insert(mEvents[*eit]); // set operation to avoid duplication
            }
        }
        EventSet::Iterator eit = activeevs.Begin();
        for(;eit!=activeevs.End();eit++){
            if (*eit == mTau) continue; // tau is always active, no need to take into consideration
            mStates[max].evs.push_back(emap[*eit]);
        }
        // initialize c value. c = 1 if no prepartition; c = (index of prepartion) + 1 if prepartition defined
        if (mPrePartition.empty()) mStates[max].c = mCmax;
        else{
            Idx prepit = 0;
            for(;prepit<mPrePartition.size();prepit++){
                if (mPrePartition[prepit].Exists(*sit)) break;
            }
            if (prepit == mPrePartition.size())
                throw Exception("EncodeData:: ", "invalide prepartition. State "+ ToStringInteger(*sit) + "is not allocated.", 100);
            mStates[max].c = prepit+1;
        }
    }
    TransSet::Iterator tit=mGen->TransRelBegin();
    for(; tit != mGen->TransRelEnd(); ++tit) {
        mStates[smap[tit->X1]].suc[emap[tit->Ev]].push_back(smap[tit->X2]);
        // distinguish tau-predecessor and non-tau-predecessor
        // since predecessors are recorded neglecting events, we (shall) check for
        // duplication before inserting. This is actually optional.
        if (emap[tit->Ev]!=0){

            std::vector<Idx>::const_iterator preit = mStates[smap[tit->X2]].pre.begin();
            for (;preit!=mStates[smap[tit->X2]].pre.end();preit++){
                if (*preit==smap[tit->X1])
                    break;
            }
            if (preit==mStates[smap[tit->X2]].pre.end())
                mStates[smap[tit->X2]].pre.push_back(smap[tit->X1]);
        }
        else{
            std::vector<Idx>::const_iterator preit = mStates[smap[tit->X2]].taupre.begin();
            for (;preit!=mStates[smap[tit->X2]].taupre.end();preit++){
                if (*preit==smap[tit->X1])
                    break;
            }
            if (preit==mStates[smap[tit->X2]].taupre.end())
                mStates[smap[tit->X2]].taupre.push_back(smap[tit->X1]);
        }
    }

    // initialize affected and changed vector
    mAffected.resize(mStateSize);
    std::fill(mAffected.begin(),mAffected.end(),0);
    mChanged.resize(mStateSize);
    std::fill(mChanged.begin(),mChanged.end(),1);
    mChanged[0] = 0;    
}

void AbstractBisimulationCTA::MarkTauStarAffected(std::vector<bool> &rAffected, const Idx& rState){
    rAffected[rState] = 1;
    std::vector<Idx>::const_iterator taupredit = mStates[rState].taupre.begin();
    for(;taupredit!=mStates[rState].taupre.end();taupredit++){
        if(!rAffected[*taupredit]){
           MarkTauStarAffected(rAffected, *taupredit);
        }
    }
}

void AbstractBisimulationCTA::ComputeChangedDelayedAfters(){
    std::fill(mAffected.begin(),mAffected.end(),0);
    Idx stateit = 1;

    // figure out all affected
    for(;stateit<mStateSize;stateit++){
        if (!mChanged[stateit]) continue;
        // all non-tau predecessors and their tau* predecessors
        std::vector<Idx>::iterator predit = mStates[stateit].pre.begin();
        for(;predit!=mStates[stateit].pre.end();predit++){
            MarkTauStarAffected(mAffected,*predit);
        }
        // this state and its tau* predecessors
        MarkTauStarAffected(mAffected, stateit);
    }
    stateit = 1;
    for(;stateit<mStateSize;stateit++){
        if(!mAffected[stateit]) continue;
        std::set<Idx> emptyset;
        std::fill(mStates[stateit].cafter.begin(),mStates[stateit].cafter.end(),emptyset);
        // compute new cafter
        // implicit selfloop (line 18)
        mStates[stateit].cafter[0].insert(mStates[stateit].c);
        // visible afters (line 19-21)
        Idx sucevit = 0; // handle tau successor as well
        for( ; sucevit<mAlphSize; sucevit++){
            std::vector<Idx>::const_iterator sucstateit = mStates[stateit].suc[sucevit].begin();
            for(;sucstateit!=mStates[stateit].suc[sucevit].end();sucstateit++){
                mStates[stateit].cafter[sucevit].insert(mStates[*sucstateit].c);
            }
        }
        // delayed afters (line 22-26)
        std::vector<Idx>::const_iterator suctauit = mStates[stateit].suc[0].begin(); // iterate over tau successors
        for(;suctauit!=mStates[stateit].suc[0].end();suctauit++){
            // append all cafters of suctauit to stateit
            Idx sucsucevit = 0;
            for( ; sucsucevit<mAlphSize; sucsucevit++){
                mStates[stateit].cafter[sucsucevit].insert(
                        mStates[*suctauit].cafter[sucsucevit].begin(),
                        mStates[*suctauit].cafter[sucsucevit].end());
            }
        }
    }
}

void AbstractBisimulationCTA::ComputeChangedObservedAfters(){
    std::fill(mAffected.begin(),mAffected.end(),0);
    Idx stateit = 1;

    // figure out all affected
    for(;stateit<mStateSize;stateit++){
        if (!mChanged[stateit]) continue;
        // buffer a vector of all tau* predecessors, including this state itself
        std::vector<bool> taustarpred(mStateSize);
        MarkTauStarAffected(taustarpred,stateit);
        // for each taustarpred, mark itself and all its non-tau preds and their tau* preds as affected
        Idx affectedit = 1;
        for (;affectedit<mStateSize;affectedit++){
            if (!taustarpred[affectedit]) continue;
            std::vector<Idx>::iterator predit = mStates[affectedit].pre.begin();
            for(;predit!=mStates[affectedit].pre.end();predit++){
                MarkTauStarAffected(mAffected,*predit);
            }
            mAffected[affectedit] = 1; // no need to mark all taupred of affectedit as they are all in taustarpred
        }
    }

    stateit = 1;
    for(;stateit<mStateSize;stateit++){
        if(!mAffected[stateit]) continue;
        std::set<Idx> emptyset;
        std::fill(mStates[stateit].cafter.begin(),mStates[stateit].cafter.end(),emptyset);
        // implicit selfloop (line 16)
        mStates[stateit].cafter[0].insert(mStates[stateit].c);
        // merge (tau-)cafter of tau successors
        std::vector<Idx>::const_iterator tausucit = mStates[stateit].suc[0].begin();
        for (;tausucit!=mStates[stateit].suc[0].end();tausucit++){
            mStates[stateit].cafter[0].insert(mStates[*tausucit].cafter[0].begin(),mStates[*tausucit].cafter[0].end());
        }
    }

    stateit = 1;
    for(;stateit<mStateSize;stateit++){
        if(!mAffected[stateit]) continue;
        // merge (tau-)cafter of non-tau successor (line 23-25)
        Idx sucevit = 1; // skip tau (0)
        for (;sucevit<mAlphSize;sucevit++){
            std::vector<Idx>::const_iterator sucstateit = mStates[stateit].suc[sucevit].begin();
            for(;sucstateit!=mStates[stateit].suc[sucevit].end();sucstateit++){
                mStates[stateit].cafter[sucevit].insert(mStates[*sucstateit].cafter[0].begin(),mStates[*sucstateit].cafter[0].end());
            }
        }
        // merge (nontau-) cafter of tau sucessros (line 26-30)
        Idx sucsucevit = 1; // non-tau suc-successors
        for(;sucsucevit<mAlphSize;sucsucevit++){
            std::vector<Idx>::const_iterator sucstateit = mStates[stateit].suc[0].begin(); // iterate over tau-succesor states
            for(;sucstateit!=mStates[stateit].suc[0].end();sucstateit++){
                mStates[stateit].cafter[sucsucevit].insert(mStates[*sucstateit].cafter[sucsucevit].begin(),mStates[*sucstateit].cafter[sucsucevit].end());
            }
        }
    }
}

// the internal wrapper
void AbstractBisimulationCTA::ComputeAbstractBisimulation(){
    BISIM_VERB1("Initializing data")
    EncodeData();
    BISIM_VERB1("Doing FirstStepApproximation")
    FirstStepApproximation();
    while (std::find(mChanged.begin(),mChanged.end(),1)!=mChanged.end()){
        if (mTaskFlag==1){
            BISIM_VERB1("Doing ComputeChangedDelayedAfters")
            ComputeChangedDelayedAfters();
        }
        else if (mTaskFlag==2){
            BISIM_VERB1("Doing ComputeChangedObservedAfters")
            ComputeChangedObservedAfters();
        }
        BISIM_VERB1("Doing RefineChanged")
        RefineChanged();
    }
}

void AbstractBisimulationCTA::ComputePartition(std::list<StateSet>& rResult){
    ComputeAbstractBisimulation();
    GenerateResult(rResult);
}



// wrappers
void ComputeBisimulationCTA(const Generator& rGen, std::list<StateSet>& rResult){
    std::vector<StateSet> trivial;
    BisimulationCTA bisim(rGen,trivial);
    bisim.ComputePartition(rResult);
}

void ComputeBisimulationCTA(const Generator& rGen, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition){
    BisimulationCTA bisim(rGen,rPrePartition);
    bisim.ComputePartition(rResult);
}

void ComputeDelayedBisimulationCTA(const Generator& rGen, const EventSet &rSilent, std::list<StateSet>& rResult){
    if (rSilent.Empty()){
        ComputeBisimulationCTA(rGen,rResult);
    }
    else if (rSilent.Size()==1){
        std::vector<StateSet> trivial;
        AbstractBisimulationCTA dbisim(rGen,*(rSilent.Begin()),1,trivial);
        dbisim.ComputePartition(rResult);
    }
    else throw Exception("ComputeDelayedBisimulationCTA:","silent alphabet can contain at most one event", 100);

}

void ComputeBisimulationCTA(const Generator& rGen, const EventSet &rSilent, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition){
    if (rSilent.Empty()){
        ComputeBisimulationCTA(rGen,rResult);
    }
    else if (rSilent.Size()==1){
        AbstractBisimulationCTA dbisim(rGen,*(rSilent.Begin()),1,rPrePartition);
        dbisim.ComputePartition(rResult);
    }
    else throw Exception("ComputeDelayedBisimulationCTA:","silent alphabet can contain at most one event", 100);

}

void ComputeWeakBisimulationCTA(const Generator& rGen, const EventSet &rSilent, std::list<StateSet>& rResult){
    if (rSilent.Empty()){
        ComputeBisimulationCTA(rGen,rResult);
    }
    else if (rSilent.Size()==1){
        std::vector<StateSet> trivial;
        AbstractBisimulationCTA wbisim(rGen,*(rSilent.Begin()),2,trivial);
        wbisim.ComputePartition(rResult);
    }
    else throw Exception("ComputeWeakBisimulation::","silent alphabet can contain at most one event", 100);
}

void ComputeWeakBisimulation(const Generator& rGen, const EventSet &rSilent, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition){
    if (rSilent.Empty()){
        ComputeBisimulationCTA(rGen,rResult);
    }
    else if (rSilent.Size()==1){
        AbstractBisimulationCTA wbisim(rGen,*(rSilent.Begin()),2,rPrePartition);
        wbisim.ComputePartition(rResult);
    }
    else throw Exception("ComputeWeakBisimulationCTA::","silent alphabet can contain at most one event", 100);
}

void ComputeAbstractBisimulationSatCTA(
        const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult, const Idx& rFlag, const std::vector<StateSet>& rPrePartition){
    if (rSilent.Empty()){
        ComputeBisimulationCTA(rGen,rResult);
    }
    else if (rSilent.Size()==1){
        Generator xg(rGen);
        ExtendTransRel(xg,rSilent,rFlag);
        InstallSelfloops(xg,rSilent);
        BisimulationCTA bisim(xg, rPrePartition);
        bisim.ComputePartition(rResult);
    }
    else throw Exception("ComputeAbstractBisimulationSatCTA::","silent alphabet can contain at most one event", 100);
}


void ComputeDelayedBisimulationSatCTA(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult){
    std::vector<StateSet> trivial;
    ComputeAbstractBisimulationSatCTA(rGen,rSilent,rResult,1,trivial);
}

void ComputeWeakBisimulationSatCTA(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult){
    std::vector<StateSet> trivial;
    ComputeAbstractBisimulationSatCTA(rGen,rSilent,rResult,2,trivial);
}

void ComputeDelayedBisimulationSatCTA(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition){
    ComputeAbstractBisimulationSatCTA(rGen,rSilent,rResult,1,rPrePartition);
}

void ComputeWeakBisimulationSatCTA(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition){
    ComputeAbstractBisimulationSatCTA(rGen,rSilent,rResult,2,rPrePartition);
}

} //namespace faudes
