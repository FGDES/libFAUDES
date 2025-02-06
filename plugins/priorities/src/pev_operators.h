#ifndef FAUDES_PEV_OPERATIONS
#define FAUDES_PEV_OPERATIONS


#include "corefaudes.h"
#include "pev_pgenerator.h"

namespace faudes {

// generate merged event name
std::string MergeEventNames (const std::set<std::string>& rNames);

// the unifying opertor for a single automaton (see also .cpp))
void Unify (pGenerator& rPGen);

// perform SFC Parallel composition
void SUParallel(
        const pGenerator& rPGen1, const pGenerator& rPGen2,
        std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap,
        EventSet& rMerge,
        const EventSet& rPrio,
        EventPriorities& rPrios,
        pGenerator& rPRes);

// perform SFC Parallel composition
void SUParallel(
        const pGenerator& rPGen1, const pGenerator& rPGen2,
        EventSet& rMerge,
        const EventSet& rPrio,
        EventPriorities& rPrios,
        pGenerator& rPRes);

// the special fairness merge for SFC
 void UParallel_MergeFairness(const pGenerator& rPGen1, const pGenerator& rPGen2, const Generator& rGen12, const EventSet& rMerge, Fairness& rFairRes);

 
}// namespace
#endif //.H
