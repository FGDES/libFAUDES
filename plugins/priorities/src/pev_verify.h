#ifndef FAUDES_PEV_VERIFY_PRIORITY_H
#define FAUDES_PEV_VERIFY_PRIORITY_H


#include "corefaudes.h"

namespace faudes {

class CompVerify{
public:
    CompVerify(void){}
    CompVerify(Generator& goi);
    CompVerify(GeneratorVector& gvoi);
    CompVerify(GeneratorVector& gvoi, const EventSet& pevs);
    virtual ~CompVerify(void);

    // counter example owns state attribute of the set of state indices matching the corresponding automata
    class StateRef : public AttributeVoid{
        FAUDES_TYPE_DECLARATION(Void,StateRef,AttribiteVoid)
    public:
        StateRef(void) : AttributeVoid() {}
        virtual ~StateRef(void) {}

        // find state idx set begin via candidate
        Idx FindState(Candidate* cand){return mref.find(cand)->second;}
        void InsertStateRef(Candidate* cand,Idx state) {mref.insert({cand,state});}
        void DeleteStateRef(Candidate* cand){mref.erase(mref.find(cand));}
        void ClearStateRef(){mref.clear();}
        std::map<Candidate*, Idx>::iterator StateRefBegin(){return mref.begin();}
        std::map<Candidate*, Idx>::iterator StateRefEnd(){return mref.end();}
    protected:
        /*!
         * \brief mref
         * This map links a candidate to a state index.
         * Generally, we do not really know to which candidate automaton (i.e. raw, hidden, merged, etc)
         * does the state index refer. However, as each step of the ce refinement do have a certain map
         * from which type of candiate automaton to which, thus it shall not cause ambiguity.
         * For Example, after each refinement iteration, state indices are referred to raw automata,
         * which are further extracted to result (e.g. merged) automaton of the successive iteration
         */
        std::map<Candidate*,Idx> mref;
    };

    typedef TaGenerator<AttributeVoid, StateRef, AttributeVoid,AttributeVoid> CounterExample;

    // convenient func for last state of a ce
    static StateSet::Iterator LastState (CounterExample& ce);
    EventSet AllPevs() {return mAllPevs;}
    virtual void VerifyAll(Generator& trace);
    virtual bool IsNonconflicting();
    bool IsPreemptive(){return mIsPreemptive;}

    /*!
     * Generate trace to bad states, i.e. some root in a blocking scc.
     *
     * @param rGen
     *   the blocking automaton
     */
    virtual void GenerateTrace(const Generator& rGen);

    /*!
     * Generate the shortest path between given start and end state
     * in a given automaton
     *
     * @param rGen
     *   the given automaton
     * @param rRes
     *   the shortest path
     * @param begin
     *   index of begin state
     * @param end
     *   index of end state
     * @return
     *   true if there exists a path between begin and end state
     */
    virtual bool ShortestPath(const Generator& rGen, Generator& rRes, Idx begin, Idx end);



    /*! counter example re-generation algorithm */

    /*!
     * Clear all state attributes of a counter example. Intended for initialization
     * @param rCE
     *   counter example
     */
    virtual void ClearAttribute (CounterExample& rCE);

    /*!
     * Extract state attributes from composed automaton to its original automata
     * @param cand
     *   the composed camp, i.e. with mMap
     * @param rCE
     *   the counter example
     * @exception
     *   599: candidate not composed
     */
    virtual void ExtractParallel (Candidate* cand,CounterExample& rCE);

    /*!
     * Expand counter example from state merging abstraction
     *
     * @param synchCands
     *   candidates (i.e. generators with their silentevs)
     * @param cand
     *   the candidate which is abstracted
     * @param rCE
     *   counter example, which is abstract at begin and shall be
     *   expanded to concrete finally w.r.t. cand
     * @param isPreemptive
     *   flag, true when consider event preemption
     * @exception
     *   500: concrete bad state can not be reached
     *   (this is theoretically impossible. Will only occur
     *   in case of implementation bugs)
     */
    virtual void StateMergingExpansion (
            SynchCandidates*  synchCands,
            Candidate*  cand,
            CounterExample& rCE
    );


    /*!
     * wrapper for counter example generation
     */
    virtual void CounterExampleRefinement();

protected:
    Generator mGenFinal; //result of final abstraction
    CounterExample mCounterExp; // resulting trace if conflicting
    std::stack<SynchCandidates*> mAllCandidates;
    EventSet mAllPevs;
    bool mIsPreemptive = 0;
};




}  // namespace

#endif // VERIFY_PRIORITY_H
