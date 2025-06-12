/** @file pev_abstraction.h Conflict preserving abstractions */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2023 Yiheng Tang
   Copyright (C) 2025 Thomas Moor
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

#ifndef PEV_ABSTRACTION_H
#define PEV_ABSTRACTION_H

#define PCOMPVER_VERB2(msg) \
  { if(faudes::ConsoleOut::G()->Verb() >=2 ) { \
      std::ostringstream cfl_line; cfl_line << msg << std::endl; faudes::ConsoleOut::G()->Write(cfl_line.str(),0,0,0);} }
#define PCOMPVER_VERB1(msg) \
  { if(faudes::ConsoleOut::G()->Verb() >=1 ) { \
      std::ostringstream cfl_line; cfl_line << msg << std::endl; faudes::ConsoleOut::G()->Write(cfl_line.str(),0,0,0);} }
#define PCOMPVER_VERB0(msg) \
  { if(faudes::ConsoleOut::G()->Verb() >=0 ) { \
      std::ostringstream cfl_line; cfl_line << msg << std::endl; faudes::ConsoleOut::G()->Write(cfl_line.str(),0,0,0);} }

#include "pev_pgenerator.h"

namespace faudes {

// manually install omega events
void AppendOmega(Generator& rGen);

/**
 * Shape generator by removing transitions that are preempted by higher priority altyernatives.
 *
 * @param rGen generator to shape
 * @param rPrios event priorities
 *
 * @ingroup PrioritiesPlugin
 */
FAUDES_API void ShapePriorities(vGenerator& rGen, const EventPriorities& rPrios);

/**
 * Shape generator by removing transitions that are preempted by higher priority altyernatives.
 *
 * @param rPGen generator to shape incl. event priorities
 *
 * @ingroup PrioritiesPlugin
 */  
FAUDES_API void ShapePriorities(pGenerator& rPGen);
  
/**
 * Shape generator by removing transitions that are preempted by higher priority alternatives.
 *
 * @param rPGen generator to shape incl. event priorities
 * @param rUpsilon only consider events in Upsilon for shaping 
 *
 * @ingroup PrioritiesPlugin
 */  
FAUDES_API void ShapeUpsilon(pGenerator& rPGen, const EventSet& rUpsilon);


/**
 * Shape generator by removing preempted transitions
 *
 * @param rGen generator to shape
 * @param rPrevs set of preempting events
 *
 * @ingroup PrioritiesPlugin
 */  
FAUDES_API void ShapePreemption(Generator& rGen, const EventSet& rPrevs);


FAUDES_API bool IsShaped(const pGenerator& rPGen, const EventSet& pevs);


/**
 * Nonconflicting Test
 *
 * Given a family of generators and global event priorities, decide whether or not the
 * synchronous composition is non-conflicting under event priorisation. This function
 * is a variant of faudes::IsPFNonblocking that addresses a generalised concept of fairness.
 *
 * @param rGvec generators to consider
 * @param rPrevs global event priosities
 * @param rFairVec one fairness constraint per generator
 * @return true for nonconflicting
 *
 * @ingroup PrioritiesPlugin
 */  
FAUDES_API bool IsPFNonblocking(
    const GeneratorVector& rGvec,
    const EventPriorities& rPrios,
    const std::vector<FairnessConstraints>& rFairVec);

/**
 * Nonconflicting Test
 *
 * Given a family of generators and global event priorities, decide whether or not the
 * synchronous composition is non-conflicting under event priorisation. This functions
 * implements a compositional approach to avoid an explicit representation of the composed
 * overall system.
 *
 * @param rGvec generators to consider
 * @param rPrevs global event priosities
 * @return true for nonconflicting
 *
 * @ingroup PrioritiesPlugin
 */  
FAUDES_API bool IsPNonblocking(
    const GeneratorVector& rGvec,
    const EventPriorities& rPrios);

  
  
  
class CompVerify;

class Candidate{
public:
    Candidate(void){}
    Candidate(Generator& goi);
    Candidate(Generator& goi, ProductCompositionMap map,std::pair<Candidate*, Candidate*> pair);
    virtual ~Candidate();
    Generator GenRaw(){return mGenRaw;}
    Generator GenHidden(){return mGenHidden;}
    Generator GenMerged(){return mGenMerged;}
    std::map<Idx,Idx> MergeMap(){return mMergeMap;}
    std::set<Idx> FindConcreteStates(Idx abstract);
    bool IsInMergedClass(Idx concrete,Idx abstract);
    ProductCompositionMap ComposeMap(){return mComposeMap;}
    std::pair<Candidate*,Candidate*> DecomposedPair(){return mDecomposedPair;}
    EventSet Silentevs() {return mSilentevs;}   
    void SetSilentevs(EventSet silentevs) {mSilentevs.Clear(); mSilentevs.InsertSet(silentevs);}
    void ClearComposition(){mComposeMap.Clear();mDecomposedPair.~pair();}
    Idx Tau(){return mtau;}
    void SetTau(Idx tau){mtau = tau;}


    /*! re-imp conflict-eq abstraction below */
    virtual void HidePrivateEvs(EventSet& silent); // derived class has variance

    void MergeEquivalenceClasses(
            Generator& rGen,
            TransSetX2EvX1& rRevTrans,
            const std::list< StateSet >& rClasses,
            const EventSet& silent);

    void ExtendedTransRel(
            const Generator& rGen,
            const EventSet& rSilentAlphabet,
            TransSet& rXTrans);

    void IncomingTransSet(
            const Generator& rGen,
            const EventSet& silent,
            const Idx& state,
            std::set<std::pair<Idx, Idx>>& result);

    void ActiveNonTauEvs(const Generator& rGen,
            const EventSet& silent,
            const Idx& state,
            EventSet &result);

    void ObservationEquivalentQuotient(
            Generator& g,
            const EventSet& silent);

    void ReverseObservationEquivalentQuotient(
            Generator& g,
            const EventSet& silent);


    void WeakObservationEquivalentQuotient(
            Generator& g,
            const EventSet& silent);


    void ActiveEventsANDEnabledContinuationRule(
            Generator& g,
            const EventSet& silent);

    void RemoveTauSelfloops(
            Generator& g,
            const EventSet& silent);

    virtual void MergeSilentLoops(
            Generator& g,
            const EventSet& silent);

    void RemoveNonCoaccessibleOut(Generator& g);

    void BlockingSilentEvent(
            Generator& g,
            const EventSet& silent);

    void MergeNonCoaccessible(Generator& g);

    void BlockingEvent(
            Generator& g,
            const EventSet& silent);

    void OnlySilentIncoming(
            Generator& g,
            const EventSet& silent);

    void OnlySilentOutgoing(
            Generator& g,
            const EventSet& silent);

    virtual void ConflictEquivalentAbstraction(EventSet &silent);

protected:
    void DoAssign(Candidate cand);
    Generator mGenRaw; // input generator, not abstracted yet
    Generator mGenHidden; // generator after hiding private evs
    Generator mGenMerged; // generator after state merging abstraction
    std::map<Idx,Idx> mMergeMap; // map of states between mGenMerged and mGenRaw
    EventSet mSilentevs; // original silent events before hiding.

    Idx mtau = 0;
    // the composition map from the last iteration.
    // Empty when not composed in the last iteration
    // the compmap matches mGenRaw to some candidate pair
    // in the latest iteration
    ProductCompositionMap mComposeMap;
    std::pair<Candidate*, Candidate*> mDecomposedPair;
};

class PCandidate : public Candidate{
public:
    PCandidate (void){}
    PCandidate (Generator &goi, EventSet pevs) : Candidate(goi), mPevs(pevs){}
    PCandidate(Generator& goi, ProductCompositionMap map,std::pair<Candidate*, Candidate*> pair, EventSet pevs)
        : Candidate(goi, map, pair), mPevs(pevs){}
    EventSet PSilentevs() {return mPSilentevs;}
    EventSet Pevs() {return mPevs;}
    void SetPSilentevs(EventSet psilentevs) {mPSilentevs.Clear(); mPSilentevs.InsertSet(psilentevs);}

    Idx Ptau(){return mPtau;}
    void SetPtau(Idx ptau){mPtau = ptau;}

    /*!
     * \brief HidePrivateEvs
     * replace all private events
     *
     * \param silent
     * private events which can be hidden
     */
    virtual void HidePrivateEvs(EventSet &silent);

    void ObservationEquivalenceQuotient_NonPreemptive(
            Generator& g,
            const EventSet& silent);

    void ObservationEquivalenceQuotient_Preemptive(
            Generator& g,
            const EventSet& silent,
            const bool& flag);

    virtual void MergeSilentLoops(
            Generator &g,
            const EventSet &silent);


    virtual void ConflictEquivalentAbstraction(EventSet &silent);

private:
    void DoAssign(PCandidate cand);
    EventSet mPevs;
    EventSet mPSilentevs;

    Idx mPtau = 0;

};


class SynchCandidates{
public:
    SynchCandidates(void){}
    SynchCandidates(Generator& gvoi);
    SynchCandidates(GeneratorVector& gvoi);
    SynchCandidates(GeneratorVector& gvoi, const EventSet& pevs);
    virtual ~SynchCandidates();
    typedef std::list<Candidate*>::iterator Iterator;

    Iterator CandidatesBegin(){return mCandidates.begin();}
    Iterator CandidatesEnd(){return mCandidates.end();}
    void Insert(Candidate* cand) {mCandidates.push_back(cand);}
    Idx Size(){return mCandidates.size();}

protected:
    void DoAssign(SynchCandidates synchcands);
    std::list<Candidate*> mCandidates;
};



} // namespace faudes
#endif // PEV_ABSTRACTION_H
