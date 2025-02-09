#include "pev_operators.h"

namespace faudes {

// generate merged event name
std::string MergeEventNames (const std::set<std::string>& rNames){
    std::set<std::string>::const_iterator nameit = rNames.begin();
    if (nameit==rNames.end()){
        throw Exception("MergeEventNames()::","Empty set of events",0);
    }
    std::string result;
    for(;nameit!=rNames.end();nameit++){
        if (nameit!=rNames.begin()) result += "-";
        result += *nameit;
    }
    return result;
}

// the unifying opertor for a single automaton.
// since we only conside the use-case for SFC verification, we consider the set of
// unifiable events as exactly the set of events at priority = 1.
// also handle fairness and alphabet
void Unify (pGenerator& rPGen){
    EventSet uevs;
    EventSet::Iterator eit = rPGen.AlphabetBegin();
    for(;eit!=rPGen.AlphabetEnd();eit++){
        if (rPGen.EventAttribute(*eit).Priority()==1) uevs.Insert(*eit);
    }
    TransSet toremove; // buffer the set of transitions to remove, i.e. the interleaving trans
    EventSet newevs;
    std::map<Idx,EventSet> newevsmap; // map an old u ev to a set of newevs
    StateSet::Iterator sit = rPGen.StatesBegin();
    for(;sit!=rPGen.StatesEnd();sit++){
        EventSet uactive = rPGen.ActiveEventSet(*sit) * uevs;
        if (uactive.Size()<2) continue; // skip for 0 or 1 active u event
        const EventSet uactive_copy = uactive; // buffer a copy for fairness
        // push active u-ev transitions to toremove
        TransSet::Iterator tit = rPGen.TransRelBegin(*sit);
        for(;tit!=rPGen.TransRelEnd(*sit);tit++){
            if (uevs.Exists(tit->Ev)){
                toremove.Insert(*tit);
            }
        }
        // find target state of unification
        // ************************CAUTION*********
        // the current implementation is rather lasy -- only works for SFC,
        // i.e. all simulatanously active unifiable events must form a "diamond" structure
        // and conflicting transitions must have been resolved before
        // ****************************************
        Idx front = *sit;
        std::set<std::string> evs; // record event names to unify
        while (!uactive.Empty()){
            tit = rPGen.TransRelBegin(front);
            for (;tit!=rPGen.TransRelEnd(front);tit++){
                if (uactive.Exists(tit->Ev)) break;
            }
            if (tit == rPGen.TransRelEnd(front)){
                throw Exception("Unify()","Ill-formed structure for transition unification. "
                                "Interleaving transitions do not form diamond near state "+ToStringInteger(front),0);
            }
            front = tit->X2;
            evs.insert(rPGen.EventName(tit->Ev));
            uactive.Erase(tit->Ev);
        }
        std::string newevname = MergeEventNames(evs);
        Idx newev;
        EventSet::Iterator neweit = newevs.Find(newevname);
        if (neweit==newevs.End()){ // this is really a new event for the alphabet
            newevs.Insert(newevname);
            newev = rPGen.InsEvent(newevname);
            EventSet::Iterator eit = uactive_copy.Begin();
            for(;eit!=uactive_copy.End();eit++){
                newevsmap[*eit].Insert(newev);
            }
        }
        else{
            newev = *rPGen.FindEvent(newevname);
        }
        rPGen.SetTransition(*sit,newev,front);
    }
    // remove transitions
    TransSet::Iterator tit = toremove.Begin();
    while (tit!=toremove.End()){
        Transition trans = *tit++;
        rPGen.ClrTransition(trans);
    }
    FairnessConstraints fair = rPGen.GlobalAttribute().Fairness();
    rPGen.Accessible();
    // handle fairness (neglect new evs not appear as transition)
    EventSet realnewevs;
    tit = rPGen.TransRelBegin();
    for(;tit!=rPGen.TransRelEnd();tit++){
        // only preserve "real new events"
        if (newevs.Exists(tit->Ev)) realnewevs.Insert(tit->Ev);
    }
    FairnessConstraints newfair;
    FairnessConstraints::Position fpos = 0;
    for(;fpos<rPGen.GlobalAttributep()->Fairness().Size();++fpos){
        EventSet cfair = rPGen.GlobalAttributep()->Fairness().At(fpos);
        EventSet::Iterator eit = rPGen.GlobalAttributep()->Fairness().At(fpos).Begin();
        EventSet::Iterator eit_end = rPGen.GlobalAttributep()->Fairness().At(fpos).End();
        for(;eit!=eit_end;eit++){
            std::map<Idx,EventSet>::const_iterator mit = newevsmap.find(*eit);
            if (mit!=newevsmap.end()){
                cfair.InsertSet(mit->second*realnewevs);
            }
        }
        newfair.Append(cfair);
    }
    rPGen.GlobalAttributep()->Fairness(newfair);
}


// perform SFC Parallel composition
void SUParallel(
        const pGenerator& rPGen1, const pGenerator& rPGen2,
        std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap,
        EventSet& rMerge,
        const EventSet& rPrio,
        EventPriorities& rPrios,
        pGenerator& rPRes)
{
    FD_DF("Parallel(" << &rGen1 << "," << &rGen2 << ")");

    // prepare result
    Generator* pResGen = &rPRes;
    if(&rPRes== &rPGen1 || &rPRes== &rPGen2) {
        pResGen= rPRes.New();
    }
    pResGen->Clear();
    pResGen->Name(CollapsString(rPGen1.Name()+"||"+rPGen2.Name()));
    rCompositionMap.clear();

    // check if merging events are disjunct
    if(!(rPGen1.Alphabet()*rPGen2.Alphabet()*rMerge).Empty()){
        Exception("Parallel()","invalid input automata (they share some mergning events)",100);
        throw;
    }
    // check if any merged event name is already utilised (consistent with the naming of merged event)
    EventSet::Iterator eit = rPGen1.AlphabetBegin();
    for (;eit!=rPGen1.AlphabetEnd();eit++){
        if (!rMerge.Exists(*eit)) continue;
        EventSet::Iterator eit2 = rPGen2.AlphabetBegin();
        for(;eit2!=rPGen2.AlphabetEnd();eit2++){
            if(!rMerge.Exists(*eit2)) continue;
            std::set<std::string> evs;
            evs.insert(rPGen1.EventName(*eit));
            evs.insert(rPGen2.EventName(*eit2));
            std::string newevname = MergeEventNames(evs);
            if (rPGen1.FindEvent(newevname)!=rPGen1.AlphabetEnd()
                    ||
                rPGen2.FindEvent(newevname)!=rPGen2.AlphabetEnd()){
                Exception("Parallel()","invalid input automata (event name"+newevname+"is in the form of a merged event)",101);
                throw;
            }
        }
    }

    // create res alphabet (no mergning)
    for (eit = rPGen1.AlphabetBegin(); eit != rPGen1.AlphabetEnd(); ++eit) {
        pResGen->InsEvent(*eit);
    }
    for (eit = rPGen2.AlphabetBegin(); eit != rPGen2.AlphabetEnd(); ++eit) {
        pResGen->InsEvent(*eit);
    }
    FD_DF("Parallel: inserted indices in rResGen.alphabet( "
          << pResGen->AlphabetToString() << ")");

    const Idx lowest = rPGen1.GlobalAttribute().LowestPriority();

    // shared events
    EventSet sharedalphabet = rPGen1.Alphabet() * rPGen2.Alphabet();
    EventSet mergingalphabet = (rPGen1.Alphabet()+rPGen2.Alphabet())*rMerge;
    FD_DF("Parallel: shared events: " << sharedalphabet.ToString());

    // todo stack
    std::stack< std::pair<Idx,Idx> > todo;
    // current pair, new pair
    std::pair<Idx,Idx> currentstates, newstates;
    // state
    Idx tmpstate;
    StateSet::Iterator lit1, lit2;
    TransSet::Iterator tit1, tit1_end, tit2, tit2_end;
    std::map< std::pair<Idx,Idx>, Idx>::iterator rcit;

    // push all combinations of initial states on todo stack
    FD_DF("Parallel: adding all combinations of initial states to todo:");
    for (lit1 = rPGen1.InitStatesBegin(); lit1 != rPGen1.InitStatesEnd(); ++lit1) {
        for (lit2 = rPGen2.InitStatesBegin(); lit2 != rPGen2.InitStatesEnd(); ++lit2) {
            currentstates = std::make_pair(*lit1, *lit2);
            todo.push(currentstates);
            tmpstate = pResGen->InsInitState();
            rCompositionMap[currentstates] = tmpstate;
            FD_DF("Parallel:   (" << *lit1 << "|" << *lit2 << ") -> "
                  << rCompositionMap[currentstates]);
        }
    }

    // start algorithm
    EventSet newMerge; // buffer a set for newly emerging merging events
    FD_DF("Parallel: processing reachable states:");
    while (! todo.empty()) {

        // allow for user interrupt
        // LoopCallback();
        // allow for user interrupt, incl progress report
        FD_WPC(rCompositionMap.size(),rCompositionMap.size()+todo.size(),"Parallel(): processing");
        // get next reachable state from todo stack
        currentstates = todo.top();
        todo.pop();

        // get current highest priority
        Idx highest = lowest;
        EventSet active1 = rPGen1.ActiveEventSet(currentstates.first);
        EventSet active2 = rPGen2.ActiveEventSet(currentstates.second);
        EventSet active = (active1 * active2) + (active1-rPGen2.Alphabet()); // iterate rPGen1 first
        EventSet::Iterator eit = active.Begin();
        for(;eit!=active.End();eit++){
            if (!rPrio.Exists(*eit)) continue;
            if (rPGen1.EventAttribute(*eit).Priority()<highest)
                highest = rPGen1.EventAttribute(*eit).Priority();
        }
        active = active2-rPGen1.Alphabet();
        eit = active.Begin();
        for(;eit!=active.End();eit++){
            if (!rPrio.Exists(*eit)) continue;
            if (rPGen2.EventAttribute(*eit).Priority()<highest)
                highest = rPGen2.EventAttribute(*eit).Priority();
        }


        FD_DF("Parallel: processing (" << currentstates.first << "|"
              << currentstates.second << ") -> "
              << rCompositionMap[currentstates]);
        // iterate over all rGen1 transitions
        // (includes execution of shared events)
        tit1 = rPGen1.TransRelBegin(currentstates.first);
        tit1_end = rPGen1.TransRelEnd(currentstates.first);
        for (; tit1 != tit1_end; ++tit1) {
            // skip if priority lower than highest
            if (rPGen1.EventAttribute(tit1->Ev).Priority()>highest) continue;
            // if asynch
            if ((!sharedalphabet.Exists(tit1->Ev) && !mergingalphabet.Exists(tit1->Ev))
                    ||
                (mergingalphabet.Exists(tit1->Ev) &&  (rPGen2.ActiveEventSet(currentstates.second) * rMerge).Empty())) {
                FD_DF("Parallel:   asynch step (private non-merging or merging with cstate.second cannot merge)");
                newstates = std::make_pair(tit1->X2, currentstates.second);
                // add to todo list if composition state is new
                rcit = rCompositionMap.find(newstates);
                if (rcit == rCompositionMap.end()) {
                    todo.push(newstates);
                    tmpstate = pResGen->InsState();
                    rCompositionMap[newstates] = tmpstate;
                    FD_DF("Parallel:   todo push: (" << newstates.first << "|"
                          << newstates.second << ") -> "
                          << rCompositionMap[newstates]);
                }
                else {
                    tmpstate = rcit->second;
                }
                pResGen->SetTransition(rCompositionMap[currentstates], tit1->Ev, tmpstate);
                FD_DF("Parallel:   add transition to new generator: "
                      << rCompositionMap[currentstates] << "-" << tit1->Ev << "-"
                      << tmpstate);
            }
            // if synch
            else {
                FD_DF("Parallel: synch (common event or synch through merging)");
                // find shared transitions
                if (sharedalphabet.Exists(tit1->Ev)){
                    FD_DF("Parallel: synch through common event");
                    tit2 = rPGen2.TransRelBegin(currentstates.second, tit1->Ev);
                    tit2_end = rPGen2.TransRelEnd(currentstates.second, tit1->Ev);
                    for (; tit2 != tit2_end; ++tit2) {
                        newstates = std::make_pair(tit1->X2, tit2->X2);
                        // add to todo list if composition state is new
                        rcit = rCompositionMap.find(newstates);
                        if (rcit == rCompositionMap.end()) {
                            todo.push(newstates);
                            tmpstate = pResGen->InsState();
                            rCompositionMap[newstates] = tmpstate;
                            FD_DF("Parallel:   todo push: (" << newstates.first << "|"
                                  << newstates.second << ") -> "
                                  << rCompositionMap[newstates]);
                        }
                        else {
                            tmpstate = rcit->second;
                        }
                        pResGen->SetTransition(rCompositionMap[currentstates],
                                               tit1->Ev, tmpstate);
                        FD_DF("Parallel:   add transition to new generator: "
                              << rCompositionMap[currentstates] << "-"
                              << tit1->Ev << "-" << tmpstate);
                    }
                }
                else{
                    FD_DF("Parallel: synch through merging");
                    tit2 = rPGen2.TransRelBegin(currentstates.second);
                    tit2_end = rPGen2.TransRelEnd(currentstates.second);
                    for (; tit2 != tit2_end; ++tit2) {
                        if (!rMerge.Exists(tit2->Ev)) continue; // only interested in merging events
                        std::set<std::string> evs;
                        evs.insert(rPGen1.EventName(tit1->Ev));
                        evs.insert(rPGen2.EventName(tit2->Ev));
                        std::string newevname = MergeEventNames(evs); // event name of merged event
                        Idx newev;
                        if (!rPRes.ExistsEvent(newevname)){
                            newev = rPRes.InsEvent(newevname);
                            newMerge.Insert(newevname);
			    rPrios.InsPriority(newevname,1);
                        }
                        else{
                            newev = *rPRes.FindEvent(newevname);
                        }
                        newstates = std::make_pair(tit1->X2, tit2->X2);
                        // add to todo list if composition state is new
                        rcit = rCompositionMap.find(newstates);
                        if (rcit == rCompositionMap.end()) {
                            todo.push(newstates);
                            tmpstate = pResGen->InsState();
                            rCompositionMap[newstates] = tmpstate;
                            FD_DF("Parallel:   todo push: (" << newstates.first << "|"
                                  << newstates.second << ") -> "
                                  << rCompositionMap[newstates]);
                        }
                        else {
                            tmpstate = rcit->second;
                        }
                        pResGen->SetTransition(rCompositionMap[currentstates],
                                               newev, tmpstate);
                        FD_DF("Parallel:   add transition to new generator: "
                              << rCompositionMap[currentstates] << "-"
                              << tit1->Ev << "-" << tmpstate);
                    }

                }
            }
        }
        // iterate over all rGen2 transitions
        // (without execution of synch step)
        tit2 = rPGen2.TransRelBegin(currentstates.second);
        tit2_end = rPGen2.TransRelEnd(currentstates.second);
        for (; tit2 != tit2_end; ++tit2) {
            if (rPGen2.EventAttribute(tit2->Ev).Priority()>highest) continue;
            if ((!sharedalphabet.Exists(tit2->Ev) && !mergingalphabet.Exists(tit2->Ev))
                    ||
                (mergingalphabet.Exists(tit2->Ev) &&  (rPGen1.ActiveEventSet(currentstates.first) * rMerge).Empty())) {
                FD_DF("Parallel:   exists only in rGen2");
                newstates = std::make_pair(currentstates.first, tit2->X2);
                // add to todo list if composition state is new
                rcit = rCompositionMap.find(newstates);
                if (rcit == rCompositionMap.end()) {
                    todo.push(newstates);
                    tmpstate = pResGen->InsState();
                    rCompositionMap[newstates] = tmpstate;
                    FD_DF("Parallel:   todo push: (" << newstates.first << "|"
                          << newstates.second << ") -> "
                          << rCompositionMap[newstates]);
                }
                else {
                    tmpstate = rcit->second;
                }
                pResGen->SetTransition(rCompositionMap[currentstates],
                                       tit2->Ev, tmpstate);
                FD_DF("Parallel:   add transition to new generator: "
                      << rCompositionMap[currentstates] << "-"
                      << tit2->Ev << "-" << tmpstate);
            }
        }
    }

    // set marked states
    for (lit1 = rPGen1.MarkedStatesBegin();
         lit1 != rPGen1.MarkedStatesEnd(); ++lit1) {
        for (lit2 = rPGen2.MarkedStatesBegin();
             lit2 != rPGen2.MarkedStatesEnd(); ++lit2) {
            currentstates = std::make_pair(*lit1, *lit2);
            rcit = rCompositionMap.find(currentstates);
            if (rcit != rCompositionMap.end()) {
                pResGen->SetMarkedState(rcit->second);
            }
        }
    }
    FD_DF("Parallel: marked states: "
          << pResGen->MarkedStatesToString());
    // copy result
    if(pResGen != &rPRes) {
        rPRes = *pResGen;
        delete pResGen;
    }
    // set statenames
    if(rPGen1.StateNamesEnabled() && rPGen2.StateNamesEnabled() && rPRes.StateNamesEnabled())
        SetComposedStateNames(rPGen1, rPGen2, rCompositionMap, rPRes);
    else
        rPRes.StateNamesEnabled(false);

    // push new merging events
    rMerge.InsertSet(newMerge);
}

void SUParallel(
        const pGenerator& rPGen1, const pGenerator& rPGen2,
        EventSet& rMerge,
        const EventSet& rPrio,
        EventPriorities& rPrios,
        pGenerator& rPRes){
    std::map< std::pair<Idx,Idx>, Idx> dummy;
    SUParallel(rPGen1,rPGen2,dummy,rMerge,rPrio,rPrios,rPRes);
}

// the special fairness merge for SFC
// does not do consistency check (should have been done before in SFC_Parallel)
// rGen12 is the parallel composition of rPGen1 and rPGen2, which should have been computed before
void UParallel_MergeFairness(const pGenerator& rPGen1, const pGenerator& rPGen2, const Generator& rGen12, const EventSet& rMerge, FairnessConstraints& rFairRes){
    rFairRes.Clear();
    // iterate over fairness of rPGen1
    FairnessConstraints::Position fpos = 0;
    for(;fpos<rPGen1.GlobalAttribute().Fairness().Size();++fpos){
      EventSet fair = rPGen1.GlobalAttribute().Fairness().At(fpos);
        EventSet fairm1 = fair * rMerge; // merging events in the current fairness (of rPGen1)
        EventSet::Iterator eit1 = fairm1.Begin();
        for(;eit1!=fairm1.End();eit1++){
            EventSet m2 = rPGen2.Alphabet() * rMerge; // merging events in rPgen2
            EventSet::Iterator eit2 = m2.Begin();
            for(;eit2!=m2.End();eit2++){
                std::set<std::string> evs;
                evs.insert(rPGen1.EventName(*eit1));
                evs.insert(rPGen2.EventName(*eit2));
                std::string newevname = MergeEventNames(evs);
                if (!rGen12.Alphabet().Exists(newevname)) continue; // skip if this event is actually not used in the parallel composition
                fair.Insert(newevname);
            }
        }
        rFairRes.Append(fair);
    }
    // iterate over fairness of rPGen2
    fpos = 0;
    for(;fpos<rPGen2.GlobalAttribute().Fairness().Size();++fpos){
        EventSet fair = rPGen2.GlobalAttribute().Fairness().At(fpos);
        EventSet fairm2 = fair * rMerge; // merging events in the current fairness (of rPGen2)
        EventSet::Iterator eit2 = fairm2.Begin();
        for(;eit2!=fairm2.End();eit2++){
            EventSet m1 = rPGen1.Alphabet() * rMerge; // merging events in rPgen1
            EventSet::Iterator eit1 = m1.Begin();
            for(;eit1!=m1.End();eit1++){
                std::set<std::string> evs;
                evs.insert(rPGen1.EventName(*eit1));
                evs.insert(rPGen2.EventName(*eit2));
                std::string newevname = MergeEventNames(evs);
                if (!rGen12.Alphabet().Exists(newevname)) continue; // skip if this event is actually not used in the parallel composition
                fair.Insert(newevname);
            }
        }
        rFairRes.Append(fair);
    }
}

}// namspace  
