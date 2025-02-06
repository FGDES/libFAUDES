#include "pev_abstraction.h"
#include "pev_verify.h"
#include <chrono>

namespace faudes {
  
FAUDES_TYPE_IMPLEMENTATION(CompVerify::StateRef,CompVerify::StateRef,AttributeVoid)

void ExtendedActiveEventSet(Idx x1, const Generator& rGen,const EventSet& silent, EventSet& result){
    std::set<Idx> visited;
    std::stack<Idx> todo;
    todo.push(x1);
    while(!todo.empty()){
        Idx cstate = todo.top();
        todo.pop();
        TransSet::Iterator tit = rGen.TransRelBegin(cstate);
        for(;tit!=rGen.TransRelEnd(cstate);tit++){
            if(silent.Exists(tit->Ev)){
                if (visited.find(tit->X2)==visited.end()) {
                    todo.push(tit->X2);
                }
            }
            else {
                result.Insert(tit->Ev);
            }
        }
        visited.insert(cstate);
    }
}

void AppendOmega(Generator& rGen){
    Idx omega = rGen.InsEvent("_OMEGA_");
    StateSet::Iterator sit = rGen.MarkedStatesBegin();
    for(;sit!=rGen.MarkedStatesEnd();sit++){
        rGen.SetTransition(*sit,omega,*sit);
    }
//    rGen.SetDefaultStateNames(); // activate for  debug
}



CompVerify::CompVerify(Generator& goi){
    SynchCandidates* candidates = new SynchCandidates(goi);
    mAllCandidates.push(candidates);
}

CompVerify::CompVerify(GeneratorVector& gvoi){

    Idx git = 0;
    for(;git<gvoi.Size();git++){
        AppendOmega(gvoi.At(git));
    }
    SynchCandidates* candidates = new SynchCandidates(gvoi);
    mAllCandidates.push(candidates);
}

CompVerify::CompVerify(GeneratorVector& gvoi, const EventSet& pevs){

    // make explicit omega to terminal with a dummy selfloop on it
    // the dummy self loop is there to distinguish it from deadend states
    Idx git = 0;
    for(;git<gvoi.Size();git++){
        AppendOmega(gvoi.At(git));
    }
    SynchCandidates* candidates = new SynchCandidates(gvoi, pevs);
    mAllCandidates.push(candidates);
    mIsPreemptive = 1;
    mAllPevs = pevs;
}

CompVerify::~CompVerify(){
    mGenFinal.Clear();
    mCounterExp.Clear();
}

SynchCandidates::SynchCandidates(Generator& goi){
    Candidate* candidate = new Candidate(goi);
    mCandidates.push_back(candidate);

}

SynchCandidates::SynchCandidates(GeneratorVector& gvoi){
    Idx git = 0;
    for (;git<gvoi.Size();git++){
        Candidate* candidate = new Candidate(gvoi.At(git));
        mCandidates.push_back(candidate);
    }
}

SynchCandidates::SynchCandidates(GeneratorVector& gvoi, const EventSet& pevs){
    Idx git = 0;
    for (;git<gvoi.Size();git++){
        PCandidate* pcandidate = new PCandidate(gvoi.At(git), pevs * gvoi.At(git).Alphabet());
        mCandidates.push_back(pcandidate);
    }
}


SynchCandidates::~SynchCandidates(){
    mCandidates.clear();
}

Candidate::~Candidate(){
    mGenRaw.Clear();
    mSilentevs.Clear();
    mtau = 0;
}

void SynchCandidates::DoAssign(SynchCandidates synchcands){
    mCandidates.clear();
    SynchCandidates::Iterator scit = synchcands.CandidatesBegin();
    for (;scit!=synchcands.CandidatesEnd();scit++)
        mCandidates.push_back(*scit);
}

void Candidate::DoAssign(Candidate cand){
    mGenRaw = cand.mGenRaw;
    mGenHidden = cand.mGenHidden;
    mGenMerged = cand.mGenMerged;
    mMergeMap = cand.mMergeMap;
    mSilentevs = cand.mSilentevs;
    mtau = cand.mtau;
    mComposeMap = cand.mComposeMap;
    mDecomposedPair.first = cand.mDecomposedPair.first;
    mDecomposedPair.second = cand.mDecomposedPair.second;
}

void PCandidate::DoAssign(PCandidate cand){
    mGenRaw = cand.mGenRaw;
    mGenHidden = cand.mGenHidden;
    mGenMerged = cand.mGenMerged;
    mMergeMap = cand.mMergeMap;
    mSilentevs = cand.mSilentevs;
    mtau = cand.mtau;
    mComposeMap = cand.mComposeMap;
    mDecomposedPair.first = cand.mDecomposedPair.first;
    mDecomposedPair.second = cand.mDecomposedPair.second;
    mPevs = cand.mPevs;
    mPSilentevs = cand.mPSilentevs;
    mPtau = cand.mPtau;
}

Candidate::Candidate(Generator& goi){
    mGenRaw = goi;
    // initialize trivial merge map
    StateSet::Iterator sit = goi.StatesBegin();
    for(;sit!=goi.StatesEnd();sit++){
        mMergeMap.insert(std::pair<Idx,Idx>(*sit,*sit));
    }
}

Candidate::Candidate(Generator& goi, ProductCompositionMap map, std::pair<Candidate*, Candidate*> pair){
    mGenRaw = goi;
    mComposeMap = map;
    mDecomposedPair.first = pair.first;
    mDecomposedPair.second = pair.second;
    // initialize trivial merge map
    StateSet::Iterator sit = goi.StatesBegin();
    for(;sit!=goi.StatesEnd();sit++){
        mMergeMap.insert(std::pair<Idx,Idx>(*sit,*sit));
    }
}

std::set<Idx> Candidate::FindConcreteStates(Idx abstractState){
    std::map<Idx,Idx>::iterator mit = mMergeMap.begin();
    std::set<Idx> concreteStates;
    for(;mit!=mMergeMap.end();mit++){
        if (mit->second == abstractState) concreteStates.insert(mit->first);
    }
    if (concreteStates.empty())
        throw Exception("FindConcreteStates()", "invalid abstract state index", 599);
    return concreteStates;
}

bool Candidate::IsInMergedClass(Idx concrete, Idx abstract){
    std::map<Idx,Idx>::iterator mit = mMergeMap.begin();
    for(;mit!=mMergeMap.end();mit++){
        if (mit->first == concrete){
            if (mit->second == abstract) return true;
            else return false;
        }
    }
    throw Exception("IsInMergedClass()","the given concrete state index is not in the merge map",500);
}

void CompVerify::VerifyAll(Generator& trace){
    std::cout<<"================================================"<<std::endl;
    std::cout<<"VerifyAll(): Verifying non-conflictness via conflict-preserving abstraction"<<std::endl;
    auto start = std::chrono::steady_clock::now();

    bool isnc = CompVerify::IsNonconflicting();

    auto end = std::chrono::steady_clock::now();
    std::cout<<"VerifyAll(): done with verification. Elapsed time: "
            << std::setprecision(2) << std::fixed
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()/(double)1000
            << "sec. IsNonblocking: "<<std::boolalpha<<isnc<<std::endl;
    std::cout<<"================================================"<<std::endl;
    if (!isnc){
        std::string YN;
        std::cout<<"VerifyAll(): proceed to trace generation?";
        while (true){
            std::cin>> YN;
            if (YN=="Y"||YN=="N"||YN=="y"||YN=="n") break;
        }
        if(YN=="Y"||YN=="y"){
            std::cout<<"================================================"<<std::endl;
            std::cout<<"VerifyAll(): Generating trace to some blocking state from abstraction"<<std::endl;
            start = std::chrono::steady_clock::now();

            GenerateTrace(mGenFinal);
            CounterExampleRefinement();

            auto end = std::chrono::steady_clock::now();
            std::cout<<"VerifyAll(): done with trace generation. Elapsed time: "
                    << std::setprecision(2) << std::fixed
                    << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()/(double)1000
                    << "sec"<<std::endl;
            std::cout<<"================================================"<<std::endl;

            trace=mCounterExp;
            // install self loops on last state, convenient for debug
            EventSet::Iterator eit = trace.Alphabet().Begin();
            for(;eit!=trace.Alphabet().End();eit++){
                trace.SetTransition(*LastState(mCounterExp),*eit,*LastState(mCounterExp));
            }
        }
    }
}

bool CompVerify::IsNonconflicting() {
    SynchCandidates* mSynchCandidates = mAllCandidates.top();
    std::stack<SynchCandidates*> allCandidates = mAllCandidates;
    bool isInitial = true; //this will be set to false from the second iteration
    // flagging the task. False when ordinary composition, true when preemption considered
    if (!mIsPreemptive){
        COMPVER_VERB0("IsNonconflicting::Start Iteration. Perform ordinary non-conflictingness check");
    }
    else {
        COMPVER_VERB0("IsNonconflicting::Start Iteration. Perform non-conflictingness check w.r.t. preemption");
    }
    while (true){
        COMPVER_VERB0("IsNonconflicting::Iterating. Remaining automata: " + ToStringInteger(mSynchCandidates->Size()));

        // trivial cases
        if(mSynchCandidates->Size()==0) return true;
        if(mSynchCandidates->Size()==1) break;

        // figure silent events
        EventSet silent, all, shared;
        SynchCandidates::Iterator scit = mSynchCandidates->CandidatesBegin();
        while(true){
            all = all+(*scit)->GenRaw().Alphabet();
            SynchCandidates::Iterator scit_next = std::next(scit,1);
            if (scit_next == mSynchCandidates->CandidatesEnd()) break;
            for(;scit_next!=mSynchCandidates->CandidatesEnd();scit_next++){
            shared = shared
                  + ((*scit)->GenRaw().Alphabet())
                  * ((*scit_next)->GenRaw().Alphabet());
            }
            scit++;
        }
        silent=all-shared; // all silent events in all current candidates
        silent.Write();

        // normalize for one silent event per generator, and then abstract
        // note from the second iteration, this is only necessary for the
        // automaton composed from former candidates. This is realized by
        // the break at the end
        scit = mSynchCandidates->CandidatesBegin();
        for(;scit!=mSynchCandidates->CandidatesEnd();scit++){
            // ***************************************************
            // abstraction
            COMPVER_VERB0("====================================")
            COMPVER_VERB0("IsNonconflicting::Abstract generator " + (*scit)->GenRaw().Name() + ". State count: "
                          + ToStringInteger((*scit)->GenRaw().Size()));
            if (!mIsPreemptive){
                (*scit)->ConflictEquivalentAbstraction(silent);
            }
            else {
                PCandidate* pcand = dynamic_cast<PCandidate*>(*scit);
                pcand->ConflictEquivalentAbstraction(silent);
                COMPVER_VERB0("State Count of Generator "<<pcand->GenRaw().Name()<<": "<<pcand->GenRaw().Size())
                COMPVER_VERB0("State Count of Generator "<<pcand->GenMerged().Name()<<": "<<pcand->GenMerged().Size())

            }
            COMPVER_VERB1("IsNonconflicting::Abstraction done. State count: "
                           + ToStringInteger((*scit)->GenMerged().Size()));


#ifdef DEBUG_VF
            (*scit)->GenRaw().Write();
            (*scit)->GenHidden().Write();
            (*scit)->GenMerged().Write();
            std::cout<<"Merge map of " + (*scit)->GenRaw().Name() + ":"<<std::endl;
            std::map<Idx,Idx> map = (*scit)->MergeMap();
            std::map<Idx,Idx>::iterator mit = map.begin();
            std::map<Idx,Idx>::iterator mit_end = map.end();
            for(;mit!=mit_end;mit++){
                std::cout<<ToStringInteger(mit->first) + "   ->   " + ToStringInteger(mit->second)<<std::endl;
            }

            std::cout<<"silent events are:"<<std::endl;
            EventSet::Iterator eit = (*scit)->Silentevs().Begin();
            EventSet::Iterator eit_end = (*scit)->Silentevs().End();
            for(;eit!=eit_end;eit++){
                std::cout<< (*scit)->GenRaw().EventName(*eit)<<std::endl;
            }
            if (dynamic_cast<PCandidate*>(*scit)!=NULL){
                PCandidate* pscit = dynamic_cast<PCandidate*>(*scit);
                eit = pscit->PSilentevs().Begin();
                eit_end = pscit->PSilentevs().End();
                for (;eit!=eit_end;eit++){
                    std::cout<< "(P) " + pscit->GenRaw().EventName(*eit)<<std::endl;
                }
            }
#endif
            if (mAllCandidates.size()!=1) break;
        }

        // candidate choice heuritics. Branch by different tasks
        SynchCandidates::Iterator imin = mSynchCandidates->CandidatesBegin();
        SynchCandidates::Iterator jmin = mSynchCandidates->CandidatesBegin();
        if (mIsPreemptive){
            jmin++;
            /* heuristics which attempts to eliminate shared preempting evs.
            for(;imin!=mSynchCandidates->CandidatesEnd();imin++){
                bool breakflag = 0;
                jmin = std::next(imin,1);
                if (std::next(jmin,1)==mSynchCandidates->CandidatesEnd()) break; // no shared pevs at all
                for(;jmin!=mSynchCandidates->CandidatesEnd();jmin++){
                    PCandidate* pimin = dynamic_cast<PCandidate*>(*imin);
                    PCandidate* pjmin = dynamic_cast<PCandidate*>(*jmin);
                    if (!(pimin->Pevs()*pjmin->Pevs()).Empty()){
                        breakflag = 1;
                        break;
                    }
                }
                if (breakflag) break;
            }
            */
        }

        else {
            // candidat pairs with fewest transitions 'minT'

            scit = std::next(imin,1);
            for(;scit!=mSynchCandidates->CandidatesEnd();scit++){
                if((*scit)->GenMerged().TransRelSize()<(*imin)->GenMerged().TransRelSize())
                    imin = scit;
            }

            // candidat pairs with most local events 'maxL'

            scit = jmin;
            Int score=-1;
            for(; scit!=mSynchCandidates->CandidatesEnd(); scit++){
                if(scit==imin) continue;
                const Generator& gi=(*imin)->GenMerged();
                const Generator& gj=(*scit)->GenMerged();
                EventSet aij=gi.Alphabet()+gj.Alphabet();
                EventSet shared;
                SynchCandidates::Iterator scit2 = mSynchCandidates->CandidatesBegin();
                for(;scit2!=mSynchCandidates->CandidatesEnd();scit2++){
                    if(scit2==imin || scit2==scit) continue;
                    shared=shared + (*scit2)->GenMerged().Alphabet()*aij;
                }
                Int jscore= aij.Size()-shared.Size();
                if(jscore>score) {score=jscore; jmin=scit;}
            }
        }


        // compose candidate pair
        Generator gimin= (*imin)->GenMerged();
        Generator gjmin= (*jmin)->GenMerged();
        Generator gij;
        ProductCompositionMap newmap;
        Parallel(gimin,gjmin,newmap,gij);
        gij.SetDefaultStateNames();
        gij.Name("(" + gimin.Name() + "--" + gjmin.Name() + ")");
        std::pair<Candidate*, Candidate*> newpair;
        newpair.first = *imin;
        newpair.second = *jmin;
        COMPVER_VERB0("IsNonconflicting::" + (*imin)->GenRaw().Name() + " and " + (*jmin)->GenRaw().Name() + " will be composed.");
        // instantiate a new synchcandidate set for next iteration
        SynchCandidates* newSynchCandidates = new SynchCandidates();

        if (!mIsPreemptive){
            Candidate* newcand = new Candidate(gij,newmap,newpair);
            newSynchCandidates->Insert(newcand);
        }
        else {
            PCandidate* pimin = dynamic_cast<PCandidate*>(*imin);
            PCandidate* pjmin = dynamic_cast<PCandidate*>(*jmin);
            PCandidate* newcand = new PCandidate(gij,newmap,newpair,pimin->Pevs()+pjmin->Pevs());
            newSynchCandidates->Insert(newcand);
        }

        scit = mSynchCandidates->CandidatesBegin();
        for(;scit!=mSynchCandidates->CandidatesEnd();scit++){
            if (scit == imin || scit == jmin) continue;
            newSynchCandidates->Insert(*scit); // all other candidates are just copied to the next iteraion
        }

        mAllCandidates.push(newSynchCandidates);
        mSynchCandidates = newSynchCandidates;
        isInitial = false;
    }

    FD_DF("Abstraction done. Tesing nonblockingness of final automaton")
    mGenFinal = (*(mSynchCandidates->CandidatesBegin()))->GenRaw();
    if(mIsPreemptive){
      PCandidate* pcand = dynamic_cast<PCandidate*>(*mSynchCandidates->CandidatesBegin());
      ShapePreemption(mGenFinal,pcand->Pevs());
    }
    mGenFinal.Write("final.gen");
    return faudes::IsNonblocking(mGenFinal);
}


void CompVerify::GenerateTrace(const Generator& rGen){
    StateSet marked = rGen.CoaccessibleSet();
    SccFilter filter(SccFilter::FMode::FmStatesAvoid,marked);
    std::list<StateSet> scclist;
    StateSet root;
    COMPVER_VERB0("GenerateTrace():: Computing strongly connected components (scc)")
    ComputeScc(rGen,filter,scclist,root);

    COMPVER_VERB0("GenerateTrace():: Figuring out scc without outgoing transitions")
    // consider only scc without outgoing transitions
    std::list<StateSet>::iterator sccit = scclist.begin();
    StateSet::Iterator rit; // root iterator

    while(sccit!=scclist.end()){

        bool breakflag = false; // if true, all iteration below breaks
        StateSet::Iterator sit = sccit->Begin();
        for(;sit!=sccit->End();sit++){
            TransSet::Iterator tit = rGen.TransRelBegin(*sit);
            for(;tit!=rGen.TransRelEnd(*sit);tit++){
                if(sccit->Exists(tit->X2)) continue;
                // else, i.e. if there is a successor state not in this scc
                breakflag = true;
                rit = root.Begin();// delete the corresponding root at first...
                for (;rit!=root.End();rit++){
                    if (sccit->Exists(*rit)){
                        root.Erase(rit);
                        break;
                    }
                }
                sccit = scclist.erase(sccit);// ... then delete the scc
                break;
            }
            if (breakflag) break;
        }
        if (!breakflag) {sccit++;}
    }

    // just report the first counter-example, not necessarily the shortest
    rit = root.Begin();
    COMPVER_VERB0("GenerateTrace():: Generating trace to the first scc")
    ShortestPath(rGen,mCounterExp,rGen.InitState(),*rit);
    COMPVER_VERB0("GenerateTrace():: Generation done. Trace length: " + ToStringInteger(mCounterExp.Size()))
}

bool CompVerify::ShortestPath(const Generator& rGen, Generator& rRes, Idx begin, Idx end){
    /* This is a simplified version of Dijkstra's algorithm
     * in which each transition only has weight=1. Path length is implied
     * by state count.
     */
    /* set up map of each (reachable) state from the begin state to its shortest path.
     * path are stored in form of generator. Maybe not efficient, but convenient for
     * output.
     */
    rRes.StateNamesEnabled(true);
    std::map<Idx, Generator> paths;
    Generator dummy;
    dummy.InsInitState(begin);
    paths[begin] = dummy;
    std::map<Idx, Idx> todo; // state index set pairs to temporily shortest distance
    todo[begin]=dummy.Size();
    Idx progressCounter = 1;
    Idx progressMax = rGen.Size();
    while(!todo.empty()){
        // search for state with shortest distance in todo
        std::map<Idx,Idx>::iterator sit = todo.begin();
        Idx where = sit->first;
        sit++;
        for(;sit!=todo.end();sit++){
            if (sit->second<todo[where])
                where = sit->first;
        }
        // if reach the target, return
        if (where == end) {
            rRes = paths[where];
            return true;
        }
        todo.erase(todo.find(where));
        TransSet::Iterator tit = rGen.TransRelBegin(where);
        for(;tit!=rGen.TransRelEnd(where);tit++){
            if(paths.find(tit->X2)==paths.end()||paths[tit->X2].Size()>paths[where].Size()){
                Generator newPath = paths[where];
                newPath.InsEvents(rGen.Alphabet());
                newPath.InsState(tit->X2);
                newPath.SetTransition(*tit);
                paths[tit->X2]=newPath;
                todo[tit->X2]=newPath.Size();
            }
        }
        progressCounter++;
    }
    // if target not reachable
    return false;
}

StateSet::Iterator CompVerify::LastState (CounterExample& ce){
    StateSet::Iterator sit = ce.StatesBegin();
    for (;sit!=ce.StatesEnd();sit++){
        if (ce.TransRelBegin(*sit)==ce.TransRelEnd(*sit)) return sit;
    }
    throw Exception("LastState():","no last state found (perhaps not ordinary counter-example)", 500);
}

void CompVerify::ClearAttribute(CounterExample& rCE){
    StateSet::Iterator sit = rCE.StatesBegin();
    for(;sit!=rCE.StatesEnd();sit++){
        rCE.StateAttributep(*sit)->ClearStateRef();
    }
}

void CompVerify::ExtractParallel (Candidate* cand,CounterExample& rCE){
    StateSet::Iterator sit = rCE.StatesBegin();
    for(;sit!=rCE.StatesEnd();sit++){
        Idx x12 = rCE.StateAttributep(*sit)->FindState(cand);
        Idx x1 = cand->ComposeMap().Arg1State(x12);
        Idx x2 = cand->ComposeMap().Arg2State(x12);
        rCE.StateAttributep(*sit)->InsertStateRef(cand->DecomposedPair().first,x1);
        rCE.StateAttributep(*sit)->InsertStateRef(cand->DecomposedPair().second,x2);
        rCE.StateAttributep(*sit)->DeleteStateRef(cand);// delete the original state attribute with composed automaton
    }
}

void CompVerify::StateMergingExpansion (
            SynchCandidates* synchCands,
            Candidate* cand,
            CounterExample& rCE)
{    

    std::set<std::pair<CounterExample,CounterExample>> queue; // pair of <concrete ce C, abstract ce to be processed C~>
    std::set<std::pair<Idx, Idx>> visited; // pair of <state, index>
    // delete all tau transitions corresponds to "this" cand
    Idx cstate = *rCE.InitStatesBegin(); // we shall only have one intial state

    COMPVER_VERB2("StateMergingExpansion(): deleting tau-transitions from current candidate")
    TransSetX2EvX1 rtrel; // set up backwards transrel, as we want to delete tau-trans with its SOURCE state
    rCE.TransRel().ReSort(rtrel);
    while (true){
        if (rCE.InitStates().Exists(cstate)){
            cstate = rCE.TransRelBegin(cstate)->X2;
            continue; // initial state has no preceding trans
        }

        TransSetX2EvX1::Iterator rtit = rtrel.BeginByX2(cstate);
        // if this state has a incoming tau trans
        if (cand->Silentevs().Exists(rtit->Ev)){
            Idx prestate = rtit->X1;
            // if the predecessor state is not the initial state
            if (!rCE.InitStates().Exists(prestate)){
                TransSetX2EvX1::Iterator pretrans = rtrel.BeginByX2(prestate);
                rCE.SetTransition(pretrans->X1,pretrans->Ev,cstate);
                rCE.DelState(prestate);
            }
            // else, i.e. predecessor is the initial state
            else{
                rCE.SetInitState(cstate);
                rCE.DelState(prestate);
            }
        }
        rCE.TransRel().ReSort(rtrel); // update rtrel
        if (rCE.ActiveEventSet(cstate).Empty()) break;
        else cstate = rCE.TransRelBegin(cstate)->X2;
    }

    COMPVER_VERB2("StateMergingExpansion(): deletion done")
    // initialize CE with single state for each initial state in concrete candidate
    StateSet::Iterator sit = cand->GenRaw().InitStatesBegin();
    for (;sit != cand->GenRaw().InitStatesEnd();sit++){
        CounterExample ce;
        Idx init = ce.InsInitState();
        SynchCandidates::Iterator scit = synchCands->CandidatesBegin();
        for (;scit!=synchCands->CandidatesEnd();scit++){
            ce.InsEvents((*scit)->GenRaw().Alphabet()); //install alphabet from all raw gens
            if (*scit == cand)
                ce.StateAttributep(init)->InsertStateRef(*scit, *sit); // in case pointing to the abstracted candidate
            else
                ce.StateAttributep(init)->InsertStateRef(*scit, rCE.StateAttributep(rCE.InitState())->FindState(*scit));
        }
        queue.insert({ce,rCE});
        visited.insert({*sit,0});
    }
    Idx counter = (*cand).Silentevs().Size();
    while (!queue.empty()){
        COMPVER_VERB2("StateMergingExpansion(): ======================================== ")
        COMPVER_VERB2("StateMergingExpansion(): queue size: " + ToStringInteger(queue.size()))

        // find the shortest queue element with lenght = |ceGenerated| + |ceToProceed|
        std::set<std::pair<CounterExample,CounterExample>>::iterator queit = queue.begin();
        std::set<std::pair<CounterExample,CounterExample>>::iterator shortest = queit;
        for (;queit!=queue.end();queit++){
            if (queit->first.Size()+queit->second.Size() < shortest->first.Size()+shortest->second.Size())
                shortest = queit;
        }
        CounterExample ceGenerated = shortest->first;
        CounterExample ceToProcess = shortest->second;
        COMPVER_VERB2("StateMergingExpansion(): picked ceGenerated size: " + ToStringInteger(ceGenerated.Size()))
        COMPVER_VERB2("StateMergingExpansion(): picked ceToProcess size: " + ToStringInteger(ceToProcess.Size()))
        queue.erase(shortest); // ... and erase it

        // some preparation for the if-else-cascade later on======>
        // the i in paper, i.e. the first state index of ceToProceed
        Idx i = rCE.Size()-ceToProcess.Size();
        // state index of cand's MergedGen of the first state of ceToProceed (x~_1^i in paper)
        Idx abstractState = ceToProcess.StateAttributep(*ceToProcess.StatesBegin())->FindState(cand);
        // state index of cand's RawGen of the last state of ceGenerated (x_1^i in paper)
        Idx concreteState = ceGenerated.StateAttributep(*LastState(ceGenerated))->FindState(cand);
        // =========> preparation end

        // return when ceToProceed only one state and the final state of ceGenerated is in this single state
        if (ceToProcess.Size()==1){
            if ((*cand).IsInMergedClass(concreteState,abstractState)){
                rCE = ceGenerated;
//                rCE.Write();
                return;
            }
        }



        // else... (follow the original part of paper)
        else{
            // get the next transition of ceToProcess (easy to access next event and next state)
            TransSet::Iterator nextTrans = ceToProcess.TransRelBegin(*ceToProcess.InitStatesBegin());

            // if the next event in ceToProcess is not an preemptive event and cand can execute
            // preemptive tau at the current state, then these preemptive taus must be executed and
            // shall kill all other events. Note the check will also take preemptive tau of other candidates
            // into consideration
            if (IsPreemptive() && !AllPevs().Exists(nextTrans->Ev)){
                std::cout<< "A" <<std::endl;
                PCandidate* pcand = dynamic_cast<PCandidate*>(cand);
                EventSet activePtau = pcand->PSilentevs() * pcand->GenRaw().ActiveEventSet(concreteState);

                activePtau.Write();

                if (!activePtau.Empty()){
                    TransSet::Iterator tit = pcand->GenRaw().TransRelBegin(concreteState);

                    std::cout<< pcand->GenRaw().EventName(tit->Ev) <<std::endl;

                    for (;tit!=pcand->GenRaw().TransRelEnd(concreteState);tit++) {
                        if(pcand->PSilentevs().Exists(tit->Ev) && visited.find({tit->X2,i})==visited.end()){
                            // do the same thing as the last case, i.e. perform the silent event
                            CounterExample newCeGenerated = ceGenerated;
                            Idx oldlaststate = *LastState(newCeGenerated);
                            Idx newstate = newCeGenerated.InsState();
                            newCeGenerated.SetTransition(oldlaststate,tit->Ev,newstate);
                            // install state attribute
                            SynchCandidates::Iterator scit = synchCands->CandidatesBegin();
                            for (;scit!=synchCands->CandidatesEnd();scit++){
                                if (*scit == cand)
                                    newCeGenerated.StateAttributep(newstate)->InsertStateRef(*scit, tit->X2);
                                else{
                                    Idx currentStateRef = ceToProcess.StateAttributep(ceToProcess.InitState())->FindState(*scit);
                                    newCeGenerated.StateAttributep(newstate)->InsertStateRef(*scit, currentStateRef);
                                }
                            }
                            queue.insert({newCeGenerated,ceToProcess});
                            visited.insert({tit->X2,i});
                        }
                    }
                    continue; // break the current CE pick, as all following cases are no longer possible for this pick
                }
            }


            // if the next ev is a (non-silent... this shall be guaranteed) ev in cand
            if(((*cand).GenRaw().Alphabet()-(*cand).Silentevs()).Exists(nextTrans->Ev)){
                std::cout<<"!Case 1:  the event is " + ceToProcess.EventName(nextTrans->Ev)<<std::endl;
                TransSet::Iterator tit = (*cand).GenRaw().TransRelBegin(concreteState);
                for (;tit!=(*cand).GenRaw().TransRelEnd(concreteState);tit++){

                    if(tit->Ev == nextTrans->Ev && visited.find({tit->X2,i+1})==visited.end()){
                        std::cout<<"!Adding transition in Case 1"<<std::endl;
                        CounterExample newCeGenerated = ceGenerated;
                        Idx oldlaststate = *LastState(newCeGenerated);
                        Idx newstate = newCeGenerated.InsState();
                        newCeGenerated.SetTransition(oldlaststate,tit->Ev,newstate);
                        // install state attribute
                        SynchCandidates::Iterator scit = synchCands->CandidatesBegin();
                        for (;scit!=synchCands->CandidatesEnd();scit++){
                            if (*scit == cand)
                                newCeGenerated.StateAttributep(newstate)->InsertStateRef(*scit, tit->X2);
                            else{
                                Idx nextStateRef = ceToProcess.StateAttributep(nextTrans->X2)->FindState(*scit);
                                newCeGenerated.StateAttributep(newstate)->InsertStateRef(*scit, nextStateRef);
                            }
                        }
                        CounterExample newCeToProceed = ceToProcess;
                        TransSet::Iterator tit2 = newCeToProceed.TransRelBegin(newCeToProceed.InitState());
                        newCeToProceed.SetInitState(tit2->X2);
                        newCeToProceed.DelState(tit2->X1);
                        queue.insert({newCeGenerated,newCeToProceed});
                        visited.insert({tit->X2,i+1});
                    }
                }
            }
            // else, then it shall be an event of other candidate (raw) gens, could either be silent or not
            else if(visited.find({concreteState,i+1})==visited.end()){
                std::cout<<"!Case 2"<<std::endl;
                CounterExample newCeGenerated = ceGenerated;
                Idx oldlaststate = *LastState(newCeGenerated);
                Idx newstate = newCeGenerated.InsState();
                newCeGenerated.SetTransition(oldlaststate,nextTrans->Ev,newstate);
                // install state attribute
                SynchCandidates::Iterator scit = synchCands->CandidatesBegin();
                for (;scit!=synchCands->CandidatesEnd();scit++){
                    if (*scit == cand)
                        newCeGenerated.StateAttributep(newstate)->InsertStateRef(*scit, concreteState);
                    else{
                        Idx nextStateRef = ceToProcess.StateAttributep(nextTrans->X2)->FindState(*scit);
                        newCeGenerated.StateAttributep(newstate)->InsertStateRef(*scit, nextStateRef);
                    }
                }
                CounterExample newCeToProceed = ceToProcess;
                TransSet::Iterator tit2 = newCeToProceed.TransRelBegin(newCeToProceed.InitState());
                newCeToProceed.SetInitState(tit2->X2);
                newCeToProceed.DelState(tit2->X1);
                queue.insert({newCeGenerated,newCeToProceed});
                visited.insert({concreteState,i+1});
            }
            else std::cout<<"!no match"<<std::endl;
        }

        // finally, if ceGenerated can perform some tau in cand (raw)
        TransSet::Iterator tit = (*cand).GenRaw().TransRelBegin(concreteState);
        for (;tit!=(*cand).GenRaw().TransRelEnd(concreteState);++tit){
            if(cand->Silentevs().Exists(tit->Ev) && visited.find({tit->X2,i})==visited.end()){
                std::cout<<"Tau step active"<<std::endl;
                CounterExample newCeGenerated = ceGenerated;
                Idx oldlaststate = *LastState(newCeGenerated);
                Idx newstate = newCeGenerated.InsState();
                newCeGenerated.SetTransition(oldlaststate,tit->Ev,newstate);
                // install state attribute
                SynchCandidates::Iterator scit = synchCands->CandidatesBegin();
                for (;scit!=synchCands->CandidatesEnd();scit++){
                    if (*scit == cand)
                        newCeGenerated.StateAttributep(newstate)->InsertStateRef(*scit, tit->X2);
                    else{
                        Idx currentStateRef = ceToProcess.StateAttributep(ceToProcess.InitState())->FindState(*scit);
                        newCeGenerated.StateAttributep(newstate)->InsertStateRef(*scit, currentStateRef);
                    }
                }
                queue.insert({newCeGenerated,ceToProcess});
                visited.insert({tit->X2,i});
            }
        }
    }
    throw Exception("CompVerify::StateMergingExpansion()", "can not reach a concrete final state", 500);
}

void CompVerify::CounterExampleRefinement(){
    // install state attribute for the initial CE
    COMPVER_VERB0("CounterExampleRefinement(): Preparing")
    ClearAttribute(mCounterExp);
    StateSet::Iterator sit = mCounterExp.StatesBegin();
    SynchCandidates::Iterator scit = mAllCandidates.top()->CandidatesBegin();
    for (;sit!=mCounterExp.StatesEnd();sit++){
        // the candidate is direct the top from allcandidate
        // its state reference is direct the CE?
        mCounterExp.StateAttributep(*sit)->InsertStateRef(*scit,*sit);
    }
    ExtractParallel(*scit,mCounterExp);
    Idx itsize = mAllCandidates.size();
    mAllCandidates.pop();
    Idx currentit = 1;

    while (!mAllCandidates.empty()){
        COMPVER_VERB0("CounterExampleRefinement(): remaining iteration: " + ToStringInteger(mAllCandidates.size()));
        scit = mAllCandidates.top()->CandidatesBegin();

        // merge silent events. convenient preparation for state merging expansion
        if (mIsPreemptive){
            for (;scit!=mAllCandidates.top()->CandidatesEnd();scit++){
                PCandidate* pcand = dynamic_cast<PCandidate*>(*scit);
                EventSet mergeSilent = pcand->Silentevs() + pcand->PSilentevs();
                pcand->SetSilentevs(mergeSilent);
            }
        }

        scit = mAllCandidates.top()->CandidatesBegin();
        for (;scit!=mAllCandidates.top()->CandidatesEnd();scit++){
            COMPVER_VERB0("CounterExampleRefinement(): Extracting state merging for " + (*scit)->GenMerged().Name());
            StateMergingExpansion(mAllCandidates.top(),*scit,mCounterExp);
            if (mAllCandidates.size()!=1){
                COMPVER_VERB0("CounterExampleRefinement(): Extracting composition map for " + (*scit)->GenRaw().Name());
                ExtractParallel(*scit,mCounterExp);
                break; // if in a higher iteration, no need to concretize for other candidates
            }
        }
        mAllCandidates.pop();
        currentit++;
    }  
}

}//namespace
