/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2025  Yiheng Tang and Thomnas Moor.
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

#ifndef BISIM_CHANGETRACKING_H
#define BISIM_CHANGETRACKING_H


#include "corefaudes.h"

#define BISIM_VERB2(msg) \
  { if((!faudes::ConsoleOut::G()->Mute()) && (faudes::ConsoleOut::G()->Verb() >=2 )) { \
      std::ostringstream cfl_line; cfl_line << msg << std::endl; faudes::ConsoleOut::G()->Write(cfl_line.str(),0,0,0);} }
#define BISIM_VERB0(msg) \
  { if((!faudes::ConsoleOut::G()->Mute()) && (faudes::ConsoleOut::G()->Verb() >=0 )) { \
      std::ostringstream cfl_line; cfl_line << msg << std::endl; faudes::ConsoleOut::G()->Write(cfl_line.str(),0,0,0);} }


namespace faudes {


extern FAUDES_API void FactorTauLoops(Generator &rGen, const Idx &rSilent); // call this (merge silent loop and then remove silent self loops)

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

// *******************************
// *******************************
// change tracking
// *******************************
// *******************************

namespace changetracking {

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
 * be done beforehand by using e.g. FactorTauLoops(Generator &rGen, const Idx &rSilent).
 */
class Bisimulation{
public:
    Bisimulation(const Generator &rGen, const std::vector<StateSet> &rPrePartition) : mGen(&rGen), mPrePartition(rPrePartition){}
    virtual ~Bisimulation() = default;

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
class AbstractBisimulation : Bisimulation{
public:
    AbstractBisimulation (const Generator& rGen, const Idx& rSilent, const Idx& rTaskFlag, const std::vector<StateSet>& rPrePartition)
        : Bisimulation(rGen,rPrePartition), mTau(rSilent), mTaskFlag(rTaskFlag){}

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



} // namespace changetracking

// wrappers

/*!
 * \brief ComputeBisimulation
 * basic bisimulation partition algorithm based on change-tracking algorithm
 * \param rGen
 *      input gen
 * \param rResult
 *      state partition without trivial classes
 */
extern FAUDES_API void ComputeBisimulationCt(const Generator& rGen, std::list<StateSet>& rResult);

/*!
 * \brief ComputeDelayedBisimulation
 * delayed bisimulation partition based on change-tracking algorithm and topo sort
 * \param rGen
 *      input gen
 * \param rSilent
 *      silent event set (contains either 0 or 1 event)
 * \param rResult
 *      state partition without trivial classes
 */
extern FAUDES_API void ComputeDelayedBisimulation(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult);

/*!
 * \brief ComputeWeakBisimulation
 * weak bisimulation (aka observation eq) partition based on change-tracking algorithm and topo sort
 * \param rGen
 *      input gen
 * \param rSilent
 *      silent event set (contains either 0 or 1 event)
 * \param rResult
 *      state partition without trivial classes
 */
extern FAUDES_API void ComputeWeakBisimulation(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult);

/*!
 * \brief ComputeBisimulation
 * basic bisimulation partition algorithm under prepartition based on change-tracking algorithm
 * \param rGen
 *      input gen
 * \param rResult
 *      state partition without trivial classes
 * \param rPrePartition
 *      prepartition (trivial classes MUST be included)
 */
extern FAUDES_API void ComputeBisimulationCt(const Generator& rGen, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition);

/*!
 * \brief ComputeDelayedBisimulation
 * delayed bisimulation partition under prepartition based on change-tracking algorithm and topo sort
 * \param rGen
 *      input gen
 * \param rSilent
 *      silent event set (contains either 0 or 1 event)
 * \param rResult
 *      state partition without trivial classes
 * \param rPrePartition
 *      prepartition (trivial classes MUST be included)
 */
extern FAUDES_API void ComputeDelayedBisimulation(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition);

/*!
 * \brief ComputeWeakBisimulation
 * weak bisimulation (aka observation eq) partition under prepartition based on change-tracking algorithm and topo sort
 * \param rGen
 *      input gen
 * \param rSilent
 *      silent event set (contains either 0 or 1 event)
 * \param rResult
 *      state partition without trivial classes
 * \param rPrePartition
 *      prepartition (trivial classes MUST be included)
 */
extern FAUDES_API void ComputeWeakBisimulation(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition);



// wrappers for satuaration-based abstract bisimulations
/*!
 * \brief ExtendTransRel
 * perform transition saturation w.r.t. silent evs. Two different saturation modes are set depending on flag value
 * \param rGen
 *      input gen
 * \param rSilent
 *      silent event set (contains either 0 or 1 event)
 * \param rFlag
 *      flag for saturation mode -- rFlag == 1: delayed transition (half-saturated); rFlag == 2: observed transition (full-saturated)
 */
void ExtendTransRel(Generator &rGen, const EventSet& rSilent, const Idx& rFlag);

/*!
 * \brief InstallSelfloops
 * install selfloops on all states of given event set. intended to install silent event selfloops for saturation
 * \param rGen
 *      input gen
 * \param rEvs
 *      events which will be installed as selfloops
 */
void InstallSelfloops(Generator &rGen, const EventSet& rEvs);

/*!
 * \brief ComputeAbstractBisimulation_Sat
 * saturation and change-tracking based partition algorithm for either delayed or weak bisimulation. This function is intended to be
 * invoked by ComputeDelayedBisimulation_Sat or ComputeWeakBisimulation_Sat, not for direct external usage
 * \param rGen
 *      input gen
 * \param rSilent
 *      silent event set (contains either 0 or 1 event)
 * \param rResult
 *      state partition without trivial classes
 * \param rFlag
 *      rFlag == 1: dalayed bisimulation; rFlag == 2: weak bisimulation
 * \param rPrePartition
 *      prepartition (trivial classes MUST be included)
 */
void ComputeAbstractBisimulation_Sat(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult, const Idx& rFlag, const std::vector<StateSet>& rPrePartition);

/*!
 * \brief ComputeDelayedBisimulation_Sat
 * delayed bisimulation partition based on change-tracking algorithm and saturation
 * \param rGen
 *      input gen
 * \param rSilent
 *      silent event set (contains either 0 or 1 event)
 * \param rResult
 *      state partition without trivial classes
 */
extern FAUDES_API void ComputeDelayedBisimulation_Sat(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult);

/*!
 * \brief ComputeWeakBisimulation_Sat
 * weak bisimulation (aka observation eq) partition based on change-tracking algorithm and saturation
 * \param rGen
 *      input gen
 * \param rSilent
 *      silent event set (contains either 0 or 1 event)
 * \param rResult
 *      state partition without trivial classes
 */
extern FAUDES_API void ComputeWeakBisimulation_Sat(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult);


/*!
 * \brief ComputeDelayedBisimulation_Sat
 * delayed bisimulation partition under prepartition based on change-tracking algorithm and saturation
 * \param rGen
 *      input gen
 * \param rSilent
 *      silent event set (contains either 0 or 1 event)
 * \param rResult
 *      state partition without trivial classes
 * \param rPrePartition
 *      prepartition (trivial classes MUST be included)
 */
extern FAUDES_API void ComputeDelayedBisimulation_Sat(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition);

/*!
 * \brief ComputeWeakBisimulation_Sat
 * weak bisimulation partition under prepartition based on change-tracking algorithm and saturation
 * \param rGen
 *      input gen
 * \param rSilent
 *      silent event set (contains either 0 or 1 event)
 * \param rResult
 *      state partition without trivial classes
 * \param rPrePartition
 *      prepartition (trivial classes MUST be included)
 */
extern FAUDES_API void ComputeWeakBisimulation_Sat(const Generator& rGen, const EventSet& rSilent, std::list<StateSet>& rResult, const std::vector<StateSet>& rPrePartition);


} // namespace faudes

#endif // BISIM_CHANGETRACKING_H
