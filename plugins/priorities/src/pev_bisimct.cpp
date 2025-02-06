#include "pev_bisimct.h"

namespace faudes {

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

namespace changetracking{

// *****************************************************
// *****************************************************
// (strong) bisimulation
// *****************************************************
// *****************************************************

void Bisimulation::EncodeData(){
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

void Bisimulation::FirstStepApproximation(){
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

void Bisimulation::ComputeChangedAfters(){
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

void Bisimulation::RefineChanged(){


    std::fill(mChanged.begin(),mChanged.end(),0);
    std::vector<Idx>::iterator partit = mPartition.begin();
    while(partit!=mPartition.end()){
        // treat affected node is merged into the following step

        // get class containing affected node (line 25 in cited paper)
        // iterate on mPartition, i.e. mPartition must already sorted by c
        // we simply iterate forward to get the current class. if no states in class is affected, skip
        Idx c = mStates[*partit].c;
        std::list<Idx> eqclass;
        std::vector<Idx>::iterator partit_rec = partit;
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
void Bisimulation::ComputeBisimulation(){
    BISIM_VERB2("Initializing data")
    EncodeData();
    BISIM_VERB2("Doing FirstStepApproximation")
    FirstStepApproximation();
    while (std::find(mChanged.begin(),mChanged.end(),1)!=mChanged.end()){
        BISIM_VERB2("Doing ComputeChangedAfters")
        ComputeChangedAfters();
        BISIM_VERB2("Doing RefineChanged")
        RefineChanged();
    }
}

void Bisimulation::GenerateResult(std::list<StateSet>& rResult){
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

void Bisimulation::ComputePartition(std::list<StateSet>& rResult){
    ComputeBisimulation();
    GenerateResult(rResult);
}

// *****************************************************
// *****************************************************
// delayed and weak bisimulation
// *****************************************************
// *****************************************************

void AbstractBisimulation::EncodeData(){
    mStateSize = mGen->States().Size()+1;
    mAlphSize = mGen->Alphabet().Size(); // Note the difference with Bisimulation::EncodeData(). Index 0 is for silent event

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
        throw Exception("DelayedBisimulation::EncodeData(): ", "input automaton shall not have tau loop. Please factor"
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

void AbstractBisimulation::MarkTauStarAffected(std::vector<bool> &rAffected, const Idx& rState){
    rAffected[rState] = 1;
    std::vector<Idx>::const_iterator taupredit = mStates[rState].taupre.begin();
    for(;taupredit!=mStates[rState].taupre.end();taupredit++){
        if(!rAffected[*taupredit]){
           MarkTauStarAffected(rAffected, *taupredit);
        }
    }
}

void AbstractBisimulation::ComputeChangedDelayedAfters(){
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

void AbstractBisimulation::ComputeChangedObservedAfters(){
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
void AbstractBisimulation::ComputeAbstractBisimulation(){
    BISIM_VERB2("Initializing data")
    EncodeData();
    BISIM_VERB2("Doing FirstStepApproximation")
    FirstStepApproximation();
    while (std::find(mChanged.begin(),mChanged.end(),1)!=mChanged.end()){
        if (mTaskFlag==1){
            BISIM_VERB2("Doing ComputeChangedDelayedAfters")
            ComputeChangedDelayedAfters();
        }
        else if (mTaskFlag==2){
            BISIM_VERB2("Doing ComputeChangedObservedAfters")
            ComputeChangedObservedAfters();
        }
        BISIM_VERB2("Doing RefineChanged")
        RefineChanged();
    }
}

void AbstractBisimulation::ComputePartition(std::list<StateSet>& rResult){
    ComputeAbstractBisimulation();
    GenerateResult(rResult);
}

}


// API wrappers
void ComputeBisimulationCt(const Generator& rGen, std::list<StateSet>& rResult){
    std::vector<StateSet> trivial;
    changetracking::Bisimulation bisim(rGen,trivial);
    bisim.ComputePartition(rResult);
}

void ComputeBisimulationCt(const Generator& rGen, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition){
    changetracking::Bisimulation bisim(rGen,rPrePartition);
    bisim.ComputePartition(rResult);
}

void ComputeDelayedBisimulation(const Generator& rGen, const EventSet &rSilent, std::list<StateSet>& rResult){
    if (rSilent.Empty()){
        ComputeBisimulationCt(rGen,rResult);
    }
    else if (rSilent.Size()==1){
        std::vector<StateSet> trivial;
        changetracking::AbstractBisimulation dbisim(rGen,*(rSilent.Begin()),1,trivial);
        dbisim.ComputePartition(rResult);
    }
    else throw Exception("ComputeDelayedBisimulation::","silent alphabet can contain at most one event", 100);

}

void ComputeDelayedBisimulation(const Generator& rGen, const EventSet &rSilent, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition){
    if (rSilent.Empty()){
        ComputeBisimulationCt(rGen,rResult);
    }
    else if (rSilent.Size()==1){
        changetracking::AbstractBisimulation dbisim(rGen,*(rSilent.Begin()),1,rPrePartition);
        dbisim.ComputePartition(rResult);
    }
    else throw Exception("ComputeDelayedBisimulation::","silent alphabet can contain at most one event", 100);

}

void ComputeWeakBisimulation(const Generator& rGen, const EventSet &rSilent, std::list<StateSet>& rResult){
    if (rSilent.Empty()){
        ComputeBisimulationCt(rGen,rResult);
    }
    else if (rSilent.Size()==1){
        std::vector<StateSet> trivial;
        changetracking::AbstractBisimulation wbisim(rGen,*(rSilent.Begin()),2,trivial);
        wbisim.ComputePartition(rResult);
    }
    else throw Exception("ComputeWeakBisimulation::","silent alphabet can contain at most one event", 100);
}

void ComputeWeakBisimulation(const Generator& rGen, const EventSet &rSilent, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition){
    if (rSilent.Empty()){
        ComputeBisimulationCt(rGen,rResult);
    }
    else if (rSilent.Size()==1){
        changetracking::AbstractBisimulation wbisim(rGen,*(rSilent.Begin()),2,rPrePartition);
        wbisim.ComputePartition(rResult);
    }
    else throw Exception("ComputeWeakBisimulation::","silent alphabet can contain at most one event", 100);
}

void ComputeAbstractBisimulation_Sat(
        const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult, const Idx& rFlag, const std::vector<StateSet>& rPrePartition){
    if (rSilent.Empty()){
        ComputeBisimulationCt(rGen,rResult);
    }
    else if (rSilent.Size()==1){
        Generator xg(rGen);
        ExtendTransRel(xg,rSilent,rFlag);
        InstallSelfloops(xg,rSilent);
        changetracking::Bisimulation bisim(xg, rPrePartition);
        bisim.ComputePartition(rResult);
    }
    else throw Exception("ComputeAbstractBisimulation_Sat::","silent alphabet can contain at most one event", 100);
}


void ComputeDelayedBisimulation_Sat(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult){
    std::vector<StateSet> trivial;
    ComputeAbstractBisimulation_Sat(rGen,rSilent,rResult,1,trivial);
}

void ComputeWeakBisimulation_Sat(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult){
    std::vector<StateSet> trivial;
    ComputeAbstractBisimulation_Sat(rGen,rSilent,rResult,2,trivial);
}

void ComputeDelayedBisimulation_Sat(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition){
    ComputeAbstractBisimulation_Sat(rGen,rSilent,rResult,1,rPrePartition);
}

void ComputeWeakBisimulation_Sat(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition){
    ComputeAbstractBisimulation_Sat(rGen,rSilent,rResult,2,rPrePartition);
}

}// namespace
