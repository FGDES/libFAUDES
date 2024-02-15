/** @file hyb_experiment.cpp  Exhaustive experments on linear hybrid automata */

/* 
   Hybrid systems plug-in  for FAU Discrete Event Systems Library 

   Copyright (C) 2017  Thomas Moor, Stefan Goetz

*/

#include "hyb_experiment.h"
#include "hyb_compute.h"
#include "hyb_reachability.h"

using namespace faudes;

// consructor
Experiment::Experiment(const faudes::EventSet& alph) :
  mpRoot(0),
  rAlphabet(alph)
{
  Clear();
}

// destructor
Experiment::~Experiment(void) {
  Clear();
  delete mpRoot;
}

// clear all
void Experiment::Clear(void) {
  // free nodes
  Iterator nit=mNodeMap.begin();
  for(;nit!=mNodeMap.end();++nit) {
    delete nit->second->mpStates;
    delete nit->second;
  }
  mNodeMap.clear();
  // setup root
  mpRoot = new Node;
  Index(mpRoot)=1;
  mNodeMap[1]=mpRoot;
  mLeaves.Clear();
  mLeaves.Insert(1);
  // aux data
  mCost=0;
}

// must set initialstates (we take ownership)
void Experiment::InitialStates(CompatibleStates* istates) {
  if(mpRoot->mpStates) Clear();
  mpRoot->mpStates = istates;
}  


// size
Idx Experiment::Size(void) const {
  return mNodeMap.size();
}

const EventSet& Experiment::Alphabet(void) const{
  return rAlphabet;
}


// navigate tree, external interface
Idx Experiment::Root(void) const {
  return Index(mpRoot);
}
EventSet Experiment::EnabledEvents(Idx nid) const {
  const Node* node = NodePtr(nid);
  if(!node) {
    std::stringstream errstr;
    errstr << "Node #" << nid << " unknown.";
    throw Exception("Experiment::EnabledEvents(..)", errstr.str(), 90);
  }
  EventSet res; 
  Node::Iterator cit = Children(node).begin(); 
  Node::Iterator cit_end = Children(node).end(); 
  for(;cit!=cit_end;++cit) res.Insert(cit->first);
  return res;
}
Idx Experiment::SuccessorNode(Idx nid, Idx ev) const {
  const Node* node = NodePtr(nid);
  if(!node) return 0;
  Node::Iterator cit = Children(node).find(ev);
  if(cit==Children(node).end()) return 0;
  return Index(cit->second);
}
bool Experiment::IsLeave(Idx nid) const {
  const Node* node = NodePtr(nid);
  if(!node) return false;
  /*
{
    std::stringstream errstr;
    errstr << "Node #" << nid << " unknown.";
    throw Exception("Experiment::IsLeave(..)", errstr.str(), 90);
  }
  */
  return IsLeave(node);
}
bool Experiment::IsLock(Idx nid) const {
  const Node* node = NodePtr(nid);
  if(!node) {
    std::stringstream errstr;
    errstr << "Node #" << nid << " unknown.";
    throw Exception("Experiment::IsLock(..)", errstr.str(), 90);
  }
  return IsLock(node);
}
const CompatibleStates* Experiment::States(Idx nid) const {
  const Node* node = NodePtr(nid);
  if(!node) {
    std::stringstream errstr;
    errstr << "Node #" << nid << " unknown.";
    throw Exception("Experiment::CStates(..)", errstr.str(), 90);
  }
  if(!node->mpStates) {
    std::stringstream errstr;
    errstr << "Node #" << nid << " no compatible states set.";
    throw Exception("Experiment::CStates(..)", errstr.str(), 90);
  }
  return node->mpStates;
}


// access by sequence
Idx Experiment::Find(const std::deque< Idx >& seq) const {
  Node* node = mpRoot;
  std::deque< Idx >::const_iterator eit= seq.begin();
  std::deque< Idx >::const_iterator eit_end= seq.end();
  for(;eit!=eit_end;++eit) {
    Node::Iterator cit = Children(node).find(*eit);
    if(cit==Children(node).end()) return 0;
    node=cit->second;
  }
  return Index(node);
}  
std::deque< Idx > Experiment::Sequence(Idx nid) const {
  const Node* node=NodePtr(nid);
  if(!node) {
    std::stringstream errstr;
    errstr << "Node #" << nid << " unknown.";
    throw Exception("Experiment::Sequence(..)", errstr.str(), 90);
  }
  std::deque< Idx > res;
  while(node!=mpRoot) {
    res.push_front(RecEvent(node));
    node=Parent(node);
  }
  return res;
}



// inspect for debugging
void Experiment::DWrite(Idx nid) const{
  FAUDES_WRITE_CONSOLE("% -------------------------------------------------------------------- ");
  Node* node= NodePtr(nid);
  if(!node) {
      FAUDES_WRITE_CONSOLE("Node Idx " << nid << " (no such node) ");
      return;
  }    
  if(node==mpRoot) {
    FAUDES_WRITE_CONSOLE("Node Idx " << Index(node) << " (Root) ");
  } else {
    if(!Parent(node)) {
      FAUDES_WRITE_CONSOLE("Node Idx " << Index(node) << " No Parent (!) ");
    } else { 	   
      FAUDES_WRITE_CONSOLE("Node Idx " << Index(node) << " Parent " << Index(Parent(node)));
    }
  }
  std::deque<Idx> seq = Sequence(Index(node));
  if(!seq.empty()) {
    std::string msg;
    std::deque<Idx>::const_iterator eit = seq.begin();
    std::deque<Idx>::const_iterator eit_end = seq.end();
    for(; eit!=eit_end;++eit){
      msg.append(rAlphabet.SymbolicName(*eit));
      msg.append(" ");
    }
    FAUDES_WRITE_CONSOLE("Event-sequence: " << msg);
  }
  if(States(node)) States(node)->DWrite();    
  if(Children(node).size()>0) {
    std::stringstream msg;
    Node::Iterator cit=Children(node).begin();
    for(; cit!=Children(node).end(); ++cit){
       msg << "(" << rAlphabet.SymbolicName(cit->first) << ")->(" << Index(cit->second) << ")  ";
    }
    FAUDES_WRITE_CONSOLE(msg.str());
  }
}
void Experiment::DWrite(void) const{
  FAUDES_WRITE_CONSOLE("% ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ ");
  FAUDES_WRITE_CONSOLE("% ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ ");
  FAUDES_WRITE_CONSOLE("% Dumping experiment ... ");
  Iterator nit=mNodeMap.begin();
  for(;nit!=mNodeMap.end();++nit)
    DWrite(nit->first);
  FAUDES_WRITE_CONSOLE("% ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ ");
  FAUDES_WRITE_CONSOLE("% ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ ");
}

void Experiment::SWrite(void) const{
  FAUDES_WRITE_CONSOLE("%  +++++++++++++++++++++++++++ ");
  FAUDES_WRITE_CONSOLE("%  Experiment Statistics");
  std::vector<int> seqcnt;
  std::vector<int> seqlck;
  int lockcnt=0;
  Iterator nit=mNodeMap.begin();
  for(;nit!=mNodeMap.end();++nit) {
    Node* node=nit->second;
    if(seqcnt.size()<Depth(node)+1) {
      seqcnt.resize(Depth(node)+1);
      seqlck.resize(Depth(node)+1);
    }
    if(IsLeave(node)){
      seqcnt[Depth(node)]++;
    }
    if(IsLock(node)) {
      seqlck[Depth(node)]++;
      lockcnt++;
    }
  }
  std::stringstream rep;
  for(unsigned int i=0; i<seqcnt.size(); ++i) {
    if(seqcnt[i]==0) continue;
    rep << "#" << seqcnt[i];
    if(seqlck[i]>0) rep << "[locks #" << seqlck[i] << "]";
    rep << "[@" << i << "]  ";
  }
  FAUDES_WRITE_CONSOLE("%  Nodes #" << Size() << " Leaves #" << Leaves().Size() << " Locks #" << lockcnt << " Cost #" << Cost());
  if(mLeaves.Size()>0)
    FAUDES_WRITE_CONSOLE("%  " << rep.str());
  FAUDES_WRITE_CONSOLE("%  +++++++++++++++++++++++++++ ");  
}

// reprot leaves
const IndexSet& Experiment::Leaves(void) const {
  /*
  const_cast<Experiment*>(this)->mLeaves.Clear();
  Iterator nit=mNodeMap.begin();
  for(;nit!=mNodeMap.end();++nit)
    if(IsLeave(nit->second)) const_cast<Experiment*>(this)->mLeaves.Insert(nit->first);
  */
  return mLeaves;
}  

// do the job: refine at specified node
void Experiment::RefineAt(Idx idx) {
  Node* node=NodePtr(idx);
  if(!node) return; // should throw
  if(!IsLeave(node)) return;
  FAUDES_WRITE_CONSOLE("Experiment::RefineAt(..): node=#" << idx << " at depth=#" << Depth(node));
  //DWrite(Index(node));
  CompatibleStates* cstates = States(node);
  if(!cstates) return; // should throw
  // nothing to do
  if(!IsLeave(node)) return;
  if(IsLock(node)) return;
  // do the job
  cstates->ExecuteTransitions();
  IsLock(node)=true;
  mCost+=cstates->Cost();
  // distribute results
  EventSet::Iterator eit=rAlphabet.Begin();
  EventSet::Iterator eit_end=rAlphabet.End();
  for(; eit!=eit_end;++eit) {
    CompatibleStates* nstates = cstates->TakeByEvent(*eit);
    // null: emptyset
    if(!nstates) continue;
    // non-null: we take ownership; null: emptyset
    Node* child=Insert(node,*eit);
    States(child) = nstates;
    IsLock(node)=false;
  }
  /*
  if(IsLock(node)) {
    FAUDES_WRITE_CONSOLE("no successor states --- record lock");
  } else {    
    std::stringstream msg;
    msg << "child nodes inserted at d=#" << Depth(node) << ": ";
    Node::Iterator cit=Children(node).begin();
    for(; cit!=Children(node).end(); ++cit){
       msg << "(" << rAlphabet.SymbolicName(cit->first) << ")->(" << Index(cit->second) << ")  ";
    }
    FAUDES_WRITE_CONSOLE(msg.str());
  }
  FAUDES_WRITE_CONSOLE("Experiment::RefineAt(..): done");
  */
}

void Experiment::RefineUniformly(unsigned int depth) {
  std::stack<Node*> todo;
  todo.push(mpRoot);
  while(!todo.empty()) {
    Node* node=todo.top();
    todo.pop();
    if(Depth(node)>=depth) continue;
    if(IsLock(node)) continue;
    if(!IsLeave(node)) continue;  
    RefineAt(Index(node));
    Node::Iterator cit=Children(node).begin();
    Node::Iterator cit_end=Children(node).end();
    for(;cit!=cit_end;++cit)
      if(cit->second)
        todo.push(cit->second);
  }
}

// do the job: refine for a sequence (return final node or 0)
Idx Experiment::RefineSequence(const std::deque< Idx >& seq) {
  Node* node=mpRoot;
  unsigned int cnt=0;
  while(cnt<seq.size()) {
    if(IsLock(node)) break;
    if(IsLeave(node)) RefineAt(Index(node));
    Node::Iterator cit = Children(node).find(seq.at(cnt));
    if(cit == Children(node).end()) break;
    node=cit->second;
    cnt++;
  }
  if(cnt==seq.size()) return Index(node);
  return 0;
}  



/*
********************************************************************************************
********************************************************************************************

Reachability analysis for DES

********************************************************************************************
********************************************************************************************
*/

// DES dynamics operator construct
DesCompatibleStates::DesCompatibleStates(const Generator& gen) : CompatibleStates(), rGen(gen) {
  mpStates= new StateSet();
}

// DES dynamics operator destruct
DesCompatibleStates::~DesCompatibleStates(void) {
  std::map<Idx, StateSet*>::iterator sit=mReachSets.begin();
  std::map<Idx, StateSet*>::iterator sit_end=mReachSets.end();
  for(;sit!=sit_end;++sit) {
    if(sit->second) delete sit->second;
  }
  delete mpStates;
}

// Optional initialisation
void DesCompatibleStates::InitialiseFull(void) {
  mpStates->Assign(rGen.States());
}
void DesCompatibleStates::InitialiseConstraint(void) {
  mpStates->Assign(rGen.InitStates());
}

// do the job
void DesCompatibleStates::ExecuteTransitions(void) {
  // lopp all events
  EventSet::Iterator eit=rGen.AlphabetBegin();
  EventSet::Iterator eit_end=rGen.AlphabetEnd();
  for(; eit!=eit_end;++eit) {
     StateSet* nstates=new StateSet; 
     nstates->Name("CompatibleStates");
     // loop over all current states
     StateSet::Iterator sit=mpStates->Begin();                                    
     StateSet::Iterator sit_end=mpStates->End();
     for(; sit!=sit_end; ++sit ) {
        TransSet::Iterator tit= rGen.TransRelBegin(*sit,*eit); 
        TransSet::Iterator tit_end= rGen.TransRelEnd(*sit,*eit);  
        for(; tit!=tit_end;++tit) nstates->Insert(tit->X2);
     }
     // continue in disabled event
     if(nstates->Empty()) {delete nstates; continue;} 
     mReachSets[*eit]=nstates;
  }
}

// pass on result (give away ownership)
DesCompatibleStates* DesCompatibleStates::TakeByEvent(Idx ev) {
  std::map<Idx, StateSet*>::iterator sit=mReachSets.find(ev);
  if(sit==mReachSets.end()) return 0;
  if(sit->second==0) return 0;
  DesCompatibleStates* res = new DesCompatibleStates(rGen);
  delete res->mpStates;
  res->mpStates= sit->second;
  sit->second=0;
  return res;
}  


// dump
void DesCompatibleStates::DWrite(void) const {
  mpStates->DWrite();
}


/*
********************************************************************************************
********************************************************************************************

Reachability analysis for LHA

********************************************************************************************
********************************************************************************************
*/


 // LHA dynamics operator construct
LhaCompatibleStates::LhaCompatibleStates(const LinearHybridAutomaton& lha) : CompatibleStates(), rLha(lha) {
  mpStates= new HybridStateSet();
  mCnt=0;
}

// LHA dynamics operator destruct
LhaCompatibleStates::~LhaCompatibleStates(void) {
  std::map<Idx, HybridStateSet*>::iterator sit=mHybridReachSets.begin();
  std::map<Idx, HybridStateSet*>::iterator sit_end=mHybridReachSets.end();
  for(;sit!=sit_end;++sit) {
    if(sit->second) delete sit->second;
  }
  delete mpStates;
}

// Optional initialisation
void LhaCompatibleStates::InitialiseFull(void) {
  mpStates->Clear();
  StateSet::Iterator qit=rLha.StatesBegin();
  StateSet::Iterator qit_end=rLha.StatesEnd();
  for(;qit!=qit_end;++qit){
    Polyhedron* poly = new Polyhedron(rLha.StateSpace());
    PolyFinalise(*poly);
    mpStates->Insert(*qit,poly);
  }
}
void LhaCompatibleStates::InitialiseConstraint(void) {
  FD_DF("LhaCompatibleStates::InitialiseConstraint()");
  mpStates->Clear();
  StateSet::Iterator qit=rLha.InitStatesBegin();
  StateSet::Iterator qit_end=rLha.InitStatesEnd();
  for(;qit!=qit_end;++qit){
    Polyhedron* poly = new Polyhedron(rLha.InitialConstraint(*qit));
    PolyIntersection(rLha.StateSpace(),*poly);
    PolyFinalise(*poly);
    mpStates->Insert(*qit,poly);
  }
  FD_DF("LhaCompatibleStates::InitialiseConstraint(): done");
}

// do the job
void LhaCompatibleStates::ExecuteTransitions(void) {
  LhaReach(rLha,*mpStates,mHybridReachSets, &mCnt);
  FD_DF("ExecuteTransitions():: found successors #" << mHybridReachSets.size());
}

// pass on result (give away ownership)
LhaCompatibleStates* LhaCompatibleStates::TakeByEvent(Idx ev) {
  std::map<Idx, HybridStateSet*>::iterator sit=mHybridReachSets.find(ev);
  if(sit==mHybridReachSets.end()) return 0;
  if(sit->second==0) return 0;
  LhaCompatibleStates* res = new LhaCompatibleStates(rLha);
  delete res->mpStates;
  res->mpStates= sit->second;
  sit->second=0;
  return res;
}  

// dump
void LhaCompatibleStates::DWrite(void) const {
  mpStates->DWrite(rLha);
}





