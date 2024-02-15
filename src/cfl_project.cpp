/** @file cfl_project.cpp projection and subset construction */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
   Copyright (C) 2006-2014  Thomas Moor
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


#include "cfl_project.h"
#include "cfl_regular.h"
#include "cfl_graphfncts.h"
#include "cfl_localgen.h"
#include "cfl_statemin.h"
#include "cfl_determin.h"

/* turn on debugging for this file */
//#undef FD_DF
//#define FD_DF(a) FD_WARN(a);

namespace faudes {


// Project, version 2008/12, Bernd Opitz (?) --- usef for libFAUDES up to 2009
// This is believed to be the original implementation by Bend Opitz, and thus might have 
// been in use since 2006, however, the below code snipped was grabbed in 2008. The call 
// of "LocalAccessibleReach" is elegant, however, needs to look up every state twice. 
// In large automata, this is a relevant contribution to the computational cost. 
// Our re-code of this algorithm saves half of the lookups (see _ref).

// Reference, 2008/12 (original Opitz?)
void ProjectNonDet_opitz(Generator& rGen, const EventSet& rProjectAlphabet) {

  // HELPERS:
  StateSet reach; // StateSet for reachable states
  std::stack<Idx> todo; // todo stack
  StateSet done; // done set
  Idx currentstate; // the currently processed state
  StateSet::Iterator lit; 
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;

  // ALGORITHM:
  // initialize algorithm by pushing init states on todo stack
  for (lit = rGen.InitStatesBegin(); lit != rGen.InitStatesEnd(); ++lit) {
    FD_DF("ProjectNonDet: todo add: " << rGen.SStr(*lit));
    todo.push(*lit);
  }

  // process todo stack
  while (! todo.empty()) {
    currentstate = todo.top();
    todo.pop();
    done.Insert(currentstate); 
    FD_DF("ProjectNonDet: current state: " << rGen.SStr(currentstate));

    // comp accessible reach
    reach.Clear();
    LocalAccessibleReach(rGen, rProjectAlphabet, currentstate, reach);
    FD_DF("ProjectNonDet: local reach: " << reach.ToString());

    // remove all transitions that leave current state 
    // with an invisible event 
    tit = rGen.TransRelBegin(currentstate);
    tit_end = rGen.TransRelEnd(currentstate);
    while(tit != tit_end) {
      FD_DF("ProjectNonDet: current transition: " << rGen.SStr(tit->X1)
          << "-" << rGen.EStr(tit->Ev) << "-" << rGen.SStr(tit->X2));
      if (! rProjectAlphabet.Exists(tit->Ev)) {
        FD_DF("ProjectNonDet: deleting current transition");
        rGen.ClrTransition(tit++);
      } else {
        ++tit;
      }
    }
		
    // relink outgoing transitions
    FD_DF("ProjectNonDet: relinking outgoing transitions...");
    for (lit = reach.Begin(); lit != reach.End(); ++lit) {
      tit = rGen.TransRelBegin(*lit);
      tit_end = rGen.TransRelEnd(*lit);
      for (; tit != tit_end; ++tit) {
        if (rProjectAlphabet.Exists(tit->Ev)) {
          FD_DF("ProjectNonDet: relinking transition: " << rGen.TStr(*tit) << " to " << rGen.SStr(currentstate));
          rGen.SetTransition(currentstate, tit->Ev, tit->X2);
          if (! done.Exists(tit->X2)) {
            FD_DF("ProjectNonDet: todo push: " << rGen.SStr(tit->X2));
            todo.push(tit->X2);
          }
        }
      }
      // marked status test
      if (rGen.ExistsMarkedState(*lit)) {
        FD_DF("ProjectNonDet: setting marked state " << rGen.SStr(currentstate));
        rGen.SetMarkedState(currentstate);
      }
    }
  }

  // inject projection alphabet
  rGen.InjectAlphabet(rProjectAlphabet);
	
  // set name
  rGen.Name(CollapsString("Pro(" + rGen.Name() + ")"));
}


// Projection version 2014/03, Moor -- revision of original 2008 algorithm 
// This is a re-code of the original libFAUDES project implementation, as proposed
// by Bernd Opitz. The Redesign takes into account the logarithmic cost of state lockups for
// large input data and avoids to call the subroutine LocalAccessibleReach. The performance gain
// is about factor 2. We use this implementationas a reference.

void ProjectNonDet_ref(Generator& rGen, const EventSet& rProjectAlphabet) {

  // HELPERS:
  std::stack<Idx> todod, todor; // states todo 
  StateSet doned, doner;        // states done (aka have been put to todo) 
  Idx currentstate;             // the currently processed state
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;

  // NAME
  std::string name=CollapsString("ProjectNonDet(" + rGen.Name() + ")");

  // ALGORITHM:

  // initialize todo stack by adding init states to todo 
  for(sit=rGen.InitStatesBegin(); sit!=rGen.InitStatesEnd(); ++sit) {
    todod.push(*sit);
    doned.Insert(*sit); 
  }
    
  // process main todo stack
  while(!todod.empty()) {

    // loop callback
    FD_WPC(doned.Size() - todod.size(), rGen.Size(), "ProjectNonDet() [STD]: done/size: " 
	   <<  doned.Size() - todod.size() << " / " << rGen.Size());

    // get top of the stack
    currentstate = todod.top();
    todod.pop();
    FD_DF("ProjectNonDet: current state: " << rGen.SStr(currentstate));

    // local reach iteration
    todor.push(currentstate);
    doner.Clear();
    doner.Insert(currentstate); 
    bool marked=rGen.ExistsMarkedState(currentstate);
    while(!todor.empty()) {
      Idx reachstate = todor.top();
      todor.pop();
      FD_DF("ProjectNonDet: reach: " << rGen.SStr(reachstate));
      //track marking
      marked|=rGen.ExistsMarkedState(reachstate);
      // iterate successors
      tit = rGen.TransRelBegin(reachstate);
      tit_end = rGen.TransRelEnd();  
      for(; tit != tit_end; ++tit) {
        if(tit->X1!=reachstate) break;
   	// for high-level events: insert new transition, add to main todo
	if(rProjectAlphabet.Exists(tit->Ev)) {
	  rGen.SetTransition(currentstate, tit->Ev, tit->X2);
	  if(doned.Insert(tit->X2)) {
	    FD_DF("ProjectNonDet: todod insert: " << rGen.SStr(tit->X2));
	    todod.push(tit->X2);
	  }
	}
        // for low-level events: add new states to local reach todo
  	else {
          if(doner.Insert(tit->X2)) {
	    todor.push(tit->X2);
	  }
	}
      }
    } // loop: local reach

    // set marking
    if(marked) rGen.SetMarkedState(currentstate);

    // remove all silent transitions that leave current state 
    tit = rGen.TransRelBegin(currentstate);
    tit_end = rGen.TransRelEnd();
    while(tit != tit_end) {
      if(tit->X1!=currentstate) break;
      FD_DF("ProjectNonDet: current transition: " << rGen.SStr(tit->X1)
	    << "-" << rGen.EStr(tit->Ev) << "-" << rGen.SStr(tit->X2));
      if(!rProjectAlphabet.Exists(tit->Ev)) {
	FD_DF("ProjectNonDet: deleting current transition");
	rGen.ClrTransition(tit++);
      } else {
	++tit;
      }
    }

  }// outer todo loop

  // inject projection alphabet, keep Attributes 
  rGen.RestrictAlphabet(rProjectAlphabet);
  // make accessile 
  rGen.RestrictStates(doned);	
  // set name
  rGen.Name(name);
}


/**
 * Graph data structure for transitionrelation -- EXPERIMENTAL
 *
 * We have encountered situations where the set based approach implies a 
 * performace penalty for large generators. This light-weight graph class 
 * is provided to investigate the situation and to compare perfromance.
 *
 */

/**
 * Forward declaration to mimique recursive typedef by
 * templates
 *
 */
template< class VLabel, class ELabel > struct TGraph;
template< class VLabel, class ELabel > struct TNode;
template< class VLabel, class ELabel > struct graph_iterator_t;
template< class VLabel, class ELabel > struct node_iterator_t;
template< class VLabel, class ELabel > struct node_entry_t;


/**
 * A graph is modelled as a map from vertex-labels to nodes.
 * For our use, the vertex-labels are state indicees and each
 * node represents the transitions from the respective state.
 * 
 * Effectively, we implement the following structure
 * graph: 
 *    map< state-index , node >
 * node:
 *    set< node-entry >
 * node-entry:
 *    pair < event-index , graph-iterator > >
 *
 * For convenience methods addressing the intended ussage, see
 * also the below specialisation.
 */
template< class VLabel, class ELabel >
struct TGraph : std::map< VLabel , TNode< VLabel , ELabel > > {
  // convenience typedef
  typedef graph_iterator_t< VLabel , ELabel> Iterator;
  // global min, element
  static inline Iterator InfX1(void) { 
    static TGraph gInfX1;
    return gInfX1.begin();
  }
};


/**
 * A node represents the edges related to one individual vertex.
 * For our use, a node is a state and the edges are transitions to
 # successor states.
 */
template< class VLabel, class ELabel >
struct TNode : std::set< node_entry_t< VLabel , ELabel > > {
  // user data
  Idx RefCnt;
  Int UsrFlg;
  // constructors
  TNode( void )
    : std::set< node_entry_t< VLabel , ELabel > >() , RefCnt(0) , UsrFlg(0) {}
  TNode( const typename std::set< node_entry_t< VLabel , ELabel > > n )
    : std::set< node_entry_t< VLabel , ELabel > >(n) ,RefCnt(0) , UsrFlg(0) {}
  // convenience tyepdef
  typedef node_iterator_t< VLabel , ELabel> Iterator;
}; 


/**
 * A node-entry represents one edge. For our use, this corresponds
 * to one transition and the edge-label is the event index.
 */
template< class VLabel, class ELabel >
struct node_entry_t {
  // data members
  ELabel Ev;
  graph_iterator_t< VLabel , ELabel > X2It;
  // constructors
  node_entry_t( void ) {};
  node_entry_t( const node_entry_t< VLabel , ELabel >& ent) 
   : Ev(ent.Ev) , X2It(ent.X2It) {};
  node_entry_t( ELabel ev , graph_iterator_t< VLabel , ELabel > x2it)
    : Ev(ev), X2It(x2it) {}
  // sorting
  inline bool operator<(const node_entry_t< VLabel , ELabel >& ent) const {
    if(this->Ev < ent.Ev) return true;
    if(this->Ev > ent.Ev) return false;
    if(this->X2It->first == ent.X2It->first) return false;
    if(this->X2It == TGraph< VLabel , ELabel >::InfX1()) return true;
    if(ent.X2It == TGraph< VLabel , ELabel >::InfX1()) return false;
    return this->X2It->first < ent.X2It->first;
  }
};


/**
 * An iterator over the map of all nodes is interpreted
 * as a state incl. all related transition. This impelmentation
 * provides convenience methods to access the state index and to iterate 
 * over exiting transitions.
 */
template< class VLabel, class ELabel >
struct graph_iterator_t : TGraph< VLabel , ELabel >::iterator {
  // convenience const cast
  inline graph_iterator_t< VLabel , ELabel >* FakeConst(void) const 
    { return (const_cast<graph_iterator_t< VLabel , ELabel >* >(this)); }
  // constructors
  graph_iterator_t( void )
    : TGraph< VLabel , ELabel  >::iterator() {}
  graph_iterator_t( typename TGraph< VLabel , ELabel >::iterator git)
    : TGraph< VLabel , ELabel  >::iterator(git) {}
  // read access
  inline VLabel X1(void) const { return (*this)->first; }
  inline typename TNode< VLabel , ELabel >::Iterator Begin(void) const
    { return (*FakeConst())->second.begin(); }
  inline typename TNode< VLabel , ELabel >::Iterator End(void) const
    { return (*this)->second.end(); }
  inline typename TNode< VLabel , ELabel >::Iterator Begin(ELabel Ev) const
    { return (*FakeConst())->second.lower_bound(
	std::make_pair(Ev , TGraph< VLabel , ELabel >::InfX1()) ); }
  inline typename TNode< VLabel , ELabel >::Iterator End(ELabel Ev) const
    { return (*FakeConst())->second.lower_bound(
	std::make_pair(Ev+1 , TGraph< VLabel , ELabel >::InfX1()) ); }
  // write access
  inline bool Insert(const ELabel ev, const graph_iterator_t< VLabel , ELabel > x2it) {
    if(! (*this)->second.insert(node_entry_t< VLabel , ELabel >(ev,x2it)).second) return false; 
    if(x2it!=(*this)) ++(x2it->second.RefCnt);
    return true;
  }  
  inline void Erase(const node_iterator_t< VLabel , ELabel > nit) {
    if(nit->X2It != (*this)) --(nit->X2It->second.RefCnt);
    (*this)->second.erase(nit);
  }  
  inline void IncRefCnt(void) {
    ++((*this)->second.RefCnt);
  }  
  // user data
  inline Int  UsrFlg(void) const { return (*this)->second.UsrFlg; }
  inline void UsrFlg(Int f) { (*this)->second.UsrFlg=f; }
  // inspect for debugging
  std::string Str(void) const {
    std::stringstream rep;
    typename TNode< VLabel , ELabel >::Iterator nit=Begin();
    typename TNode< VLabel , ELabel >::Iterator nit_end=End();
    rep << "[" << X1() << "] ";
    for(; nit!=nit_end; ++nit) 
      rep << "(" << nit.Ev() << ")->" << nit.X2() << " ";
    return rep.str();
  }

};


/**
 * An iterator over the set of edges related to one vertex is interpreted
 * as a transition.
 */
template< class VLabel, class ELabel >
struct node_iterator_t : TNode< VLabel , ELabel >::iterator {
  node_iterator_t( void )
    : TNode< VLabel , ELabel >::iterator() {}
  node_iterator_t( const typename TNode< VLabel , ELabel >::iterator & nit)
    : TNode< VLabel , ELabel >::iterator(nit) {}
  inline ELabel Ev(void) const { return (*this)->Ev; }
  inline graph_iterator_t< VLabel , ELabel > X2It(void) const { return (*this)->X2It; }
  inline VLabel X2(void) const { return (*this)->X2It->first; }
};


/**
 * Specialisation of the graph template to provide convenience methods 
 * addressing the intended ussage.
 */
template< >
struct TGraph<Idx,Idx> : std::map< Idx , TNode< Idx , Idx > >{ 
  // convenience typedef
  typedef graph_iterator_t< Idx , Idx> Iterator;
  // conveniend const cast
  inline TGraph<Idx,Idx>*  FakeConst(void) const 
    { return (const_cast<TGraph< Idx, Idx>*>(this)); }
  // read access
  inline Iterator Begin(void) const { return FakeConst()->begin(); }
  inline Iterator End(void)   const { return FakeConst()->end(); }
  inline Iterator Begin(Idx x1) const { return FakeConst()->lower_bound(x1); }
  inline Iterator End(Idx x1)   const { return FakeConst()->lower_bound(x1+1); }
  inline Iterator Find(Idx x1) const { return FakeConst()->find(x1); }
  // write access
  inline Iterator Insert(Idx x1) { 
    return (this->insert(std::make_pair(x1,mapped_type()))).first; }
  inline bool Erase(Idx x1) {
    if(Find(x1)==End()) return false;
    Iterator git=Begin();
    Iterator git_end=End();
    for(;git!=git_end;++git) {
      TNode<Idx,Idx>::Iterator nit=git.Begin();
      TNode<Idx,Idx>::Iterator nit_end=git.End();
      while(nit!=nit_end) {
        if(nit.X2()==x1) git.Erase(nit++);
        else ++nit;
      }
    }
    return ((*this).erase(x1)) !=0; // allways true
  }
  inline void Erase(Iterator git, TNode<Idx,Idx>::Iterator nit) {
    Iterator x2it = nit.X2It();
    git.Erase(nit);
    if(x2it->second.RefCnt == 0 ) ((*this).erase(x2it)); 
  }
  // conversion from generator interface (destructive)
  void Import(vGenerator& rGen) {
    TransSet::Iterator tit = rGen.TransRelBegin();
    TransSet::Iterator tit_end = rGen.TransRelEnd();  
    for(; tit != tit_end; rGen.ClrTransition(tit++)) {
      Iterator git=find(tit->X1);
      if(git==end()) git = insert(std::make_pair(tit->X1,mapped_type())).first; 
      Iterator x2it=find(tit->X2);
      if(x2it==end()) x2it = insert(std::make_pair(tit->X2,mapped_type())).first; 
      git.Insert(tit->Ev,x2it);
    }
    StateSet::Iterator sit = rGen.InitStatesBegin();
    StateSet::Iterator sit_end = rGen.InitStatesEnd();
    for(; sit != sit_end; sit++) 
      Find(*sit).IncRefCnt();
  };
  // conversion to generator (destructive)
  void Export(vGenerator& rGen) {
    rGen.ClearTransRel();
    Iterator git=begin();
    Iterator git_end=end();
    for(; git != git_end; git++) {
      TNode<Idx,Idx>::Iterator nit =git.Begin();
      TNode<Idx,Idx>::Iterator nit_end=git.End();
      for(; nit != nit_end; ++nit) {
        rGen.InjectState(git.X1());                
        rGen.InjectState(nit.X2());                
        rGen.InjectTransition(Transition(git.X1(),nit.Ev(),nit.X2()));
      }
      git->second.clear();
    }
    this->clear();
  }
  // inspect or debugging
  std::string Str(void) const {
    std::stringstream rep;
    Iterator git=Begin();
    Iterator git_end=End();
    for(; git!=git_end; ++git) 
      rep << git.Str() << std::endl;
    return rep.str();
  }
  // global min, element
  static inline Iterator InfX1(void) { 
    static TGraph gInfX1;
    return gInfX1.begin(); }
};


// convenience typedefs
typedef TNode<Idx,Idx> Node; 
typedef TGraph<Idx,Idx> Graph;


// Projection, version 2014/03, Moor -- graph based implementation of original 2008 algorithm
// The performance gain is about +50%, demonstrating that the graph data structure is a more
// adequate representation for intensive forward reachability analysis.

void ProjectNonDet_graph(Generator& rGen, const EventSet& rProjectAlphabet) {

  std::stack<Graph::Iterator> todod, todor; // states todo 
  std::stack<Graph::Iterator> todov; // states todo 
  StateSet doned, doner;        // states done (aka have been put to todo) 
  Graph::Iterator currentstate;  // the currently processed state
  Graph::Iterator reachstate;   // inner reachability loop
  StateSet::Iterator sit;
  Graph trg;
  Graph::Iterator git, git_end;
  Node::Iterator nit, nit_end;

  // NAME
  std::string name=CollapsString("ProjectNonDet(" + rGen.Name() + ")");

  // convert to graph
  FD_DF("ProjectNonDet: convert to graph");
  trg.Import(rGen);
  FD_DF("ProjectNonDet: convert to graph: done");

  // initialize todo stack by adding init states to todo 
  for(sit=rGen.InitStatesBegin(); sit!=rGen.InitStatesEnd(); ++sit) {
    todod.push(trg.Find(*sit));
    doned.Insert(*sit); 
  }
    
  // process main todo stack
  while(!todod.empty()) {

    // loop callback
    FD_WPC(doned.Size() - todod.size(),trg.size(), "ProjectNonDet() [G1]: done/size: " 
	   <<  doned.Size() - todod.size() << "/" << trg.size());

    // get top of the stack
    currentstate = todod.top();
    todod.pop();
    FD_DF("ProjectNonDet: current state: " << rGen.SStr(currentstate.X1()));

    // local reach iteration
    todor.push(currentstate);
    doner.Clear();
    doner.Insert(currentstate.X1()); 
    bool marked=rGen.ExistsMarkedState(currentstate->first);
    while(!todor.empty()) {
      reachstate = todor.top();
      todor.pop();
      FD_DF("ProjectNonDet: reach: " << rGen.SStr(reachstate.X1()));
      //track marking
      marked|=rGen.ExistsMarkedState(reachstate.X1());
      // iterate successors
      nit=reachstate.Begin();
      nit_end=reachstate.End();
      for(; nit != nit_end; ++nit) {
   	// for high-level events: add to main todo and insert new transition, 
	if(rProjectAlphabet.Exists(nit.Ev())) {
	  if(doned.Insert(nit.X2())) {
	    FD_DF("ProjectNonDet: todod insert: " << rGen.SStr(nit.X2()));
	    todod.push(nit.X2It());
	  }
          // transition is only new if we are not on the currentstate
          if(reachstate!=currentstate) {
  	    FD_DF("ProjectNonDet: trans insert: " << rGen.SStr(currentstate.X1()) << " -> " << rGen.SStr(nit.X2()));
            currentstate.Insert(nit.Ev(),nit.X2It());
	  }
	}
        // for low-level events: add new states to reach todo
  	else {
          if(doner.Insert(nit.X2())) 
	    todor.push(nit.X2It());
          if(nit.X2It()==currentstate) 
            todov.push(reachstate);
	}
      }
    } // loop: local reach

    // set marking
    if(marked) rGen.SetMarkedState(currentstate->first);

    // remove all silent transitions that leave current state 
    nit=currentstate.Begin();
    nit_end=currentstate.End();
    while(nit != nit_end) {
      if(!rProjectAlphabet.Exists(nit.Ev())) 
	trg.Erase(currentstate,nit++);
	//currentstate.Erase(nit++);
      else 
	++nit;
    }

  }// outer todo loop

  // convert back
  FD_DF("ProjectNonDet: convert from graph");
  trg.Export(rGen);
  FD_DF("ProjectNonDet: convert from graph: done");

  // inject projection alphabet, keep Attributes 
  rGen.RestrictAlphabet(rProjectAlphabet);
  // make accessile 
  rGen.RestrictStates(doned);
  // set name
  rGen.Name(name);
}



// Projection, version 2014/03, Moor -- simple algorithm
// For large automata, the performance penalty is prohibitive. However, this
// simple variant is useful for validation of correctness for other implementations.

void ProjectNonDet_simple(Generator& rGen, const EventSet& rProjectAlphabet) {

  // HELPERS:
  std::stack<Idx> todod; // states todo 
  StateSet doned;         // states done (aka have been put to todo) 
  Idx currentstate; // the currently processed state
  StateSet::Iterator sit, sit_end;
  TransSet::Iterator tit, tit_end;
  StateSet reach;

  // NAME
  std::string name=CollapsString("ProjectNonDet(" + rGen.Name() + ")");

  // ALGORITHM:

  // initialize todo stack by adding init states to todo 
  for(sit=rGen.InitStatesBegin(); sit!=rGen.InitStatesEnd(); ++sit) {
    todod.push(*sit);
    doned.Insert(*sit); 
  }
    
  // process main todo stack
  while(!todod.empty()) {

    // loop callback
    FD_WPC(doned.Size(),rGen.Size(), "ProjectNonDet() [SIMPL]: current/size: " 
      <<  doned.Size() << " / " << rGen.Size());

    // get top of the stack
    currentstate = todod.top();
    todod.pop();
    FD_DF("ProjectNonDet: current state: " << rGen.SStr(currentstate));

    // inspect successors to obtain one-step silent reach set
    reach.Clear();
    tit = rGen.TransRelBegin(currentstate);
    tit_end = rGen.TransRelEnd(currentstate);
    for(; tit != tit_end; ++tit) {
      if(!doned.Exists(tit->X2)) { todod.push(tit->X2); doned.Insert(tit->X2); }
      if(rProjectAlphabet.Exists(tit->Ev)) continue;
      if(tit->X2==currentstate) continue;
      reach.Insert(tit->X2);
    }
	
    // copy local successor transitions/marking to this state
    bool revisit=false;
    bool marked=rGen.ExistsMarkedState(currentstate);
    sit = reach.Begin();
    sit_end = reach.End();
    for(; sit != sit_end; ++sit) {
      tit = rGen.TransRelBegin(*sit);
      tit_end = rGen.TransRelEnd(*sit);
      for(; tit != tit_end; ++tit) {
        revisit|=rGen.SetTransition(currentstate,tit->Ev,tit->X2);
        marked|=rGen.ExistsMarkedState(tit->X1);
      }
    }
    if(marked) rGen.InsMarkedState(currentstate);

    // queue again if we introdced a new transion
    if(revisit) todod.push(currentstate);

  } // todo stack

  FD_DF("ProjectNonDet: finalize");

  // restrict to projection alphabet, keep Attributes
  rGen.RestrictAlphabet(rProjectAlphabet);
  // make accessile 
  rGen.Accessible();
  // set name
  rGen.Name(name);
}


// Project, version 2009/05, Tobias Barthel -- used for libFAUDES 2.14 to 2.23 (2009 -- 2014)
// Tobias Barthel found a test case (non-deterministic? diagnoser?) in which the original implementation 
// by Bernd Opitz was believed to fail. Unfortunatly, this test case is now lost. As of 2014/03, we use 
// the above re-coded version of the original algorithm as our reference. We believe it to be correct 
// ... it passed all our test cases with identical results to the below implementation.
void ProjectNonDet_barthel(Generator& rGen, const EventSet& rProjectAlphabet) {

  // HELPERS:
  StateSet reach, reachext; // StateSet for reachable states
  StateSet todo; // states todo
  StateSet done; // states done 
  Idx currentstate; // the currently processed state
  StateSet::Iterator lit;
  StateSet::Iterator lit2;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;

  // NAME
  std::string name=CollapsString("ProjectNonDet(" + rGen.Name() + ")");

  // ALGORITHM:

  // set all locally reachable states to init states
  // tmoor 201403: is this needed to prevent the initial state to disappear?
  reachext.Clear();
  for(lit = rGen.InitStatesBegin(); lit != rGen.InitStatesEnd(); ++lit) {
    reach.Clear();
    LocalAccessibleReach(rGen, rProjectAlphabet, *lit, reach);
    reachext.InsertSet(reach);
  }   
  rGen.InsInitStates(reachext);
  FD_DF("ProjectNonDet: initial states: " << rGen.InitStates().ToString());
  // initialize algorithm by adding init states to todo 
  todo.InsertSet(rGen.InitStates());
    
  // process todo stack
  while(!todo.Empty()) {
    currentstate = *todo.Begin();
    todo.Erase(*todo.Begin());
    done.Insert(currentstate); // mark as done
    FD_DF("ProjectNonDet: current state: " << rGen.SStr(currentstate));

    // comp accessible reach
    reach.Clear();
    LocalAccessibleReach(rGen, rProjectAlphabet, currentstate, reach);
    FD_DF("ProjectNonDet: local reach: " << reach.ToString());
		
    // relink outgoing transitions
    FD_DF("ProjectNonDet: relinking outgoing transitions...");
    for(lit = reach.Begin(); lit != reach.End(); ++lit) {
      tit = rGen.TransRelBegin(*lit);
      tit_end = rGen.TransRelEnd(*lit);
      for(; tit != tit_end; ++tit) {
	if(rProjectAlphabet.Exists(tit->Ev)) {
	  FD_DF("ProjectNonDet: relinking transition: " << rGen.TStr(*tit) << " to " << rGen.SStr(currentstate));
	  rGen.SetTransition(currentstate, tit->Ev, tit->X2);
	  if (!done.Exists(tit->X2)) {
	    FD_DF("ProjectNonDet: todo insert: " << rGen.SStr(tit->X2));
	    todo.Insert(tit->X2);
	  }
	  // add transistions to all states in extended local reach
          // tmoor 201308: I dont see why we need this ...
	  reachext.Clear();
	  LocalAccessibleReach(rGen, rProjectAlphabet, tit->X2, reachext);
	  FD_DF("ProjectNonDet: local reach from state " << tit->X2 << ": " << reachext.ToString());
	  for (lit2 = reachext.Begin(); lit2 != reachext.End(); ++lit2) {
	    if (!rGen.ExistsTransition(tit->X2, tit->Ev, *lit2) && (tit->X2 != *lit2)) {
	      rGen.SetTransition(tit->X1, tit->Ev, *lit2);
	      FD_DF("ProjectNonDet: setting transition: " << rGen.SStr(tit->X1) << "-" << rGen.EStr(tit->Ev) << "-" << rGen.SStr(*lit2));
	      //if (!done.Exists(*lit2) && !todo.Exists(*lit2)) {
	      if (!done.Exists(*lit2)) {
		FD_DF("ProjectNonDet: todo insert: " << rGen.SStr(tit->X2));
		todo.Insert(*lit2);
	      }
	    }
	  }
	}
      }
      // marked status test
      if(rGen.ExistsMarkedState(*lit)) {
	FD_DF("ProjectNonDet: setting marked state " << rGen.SStr(currentstate));
	rGen.SetMarkedState(currentstate);
      }
    }
	 
    // remove all silent transitions that leave current state 
    tit = rGen.TransRelBegin(currentstate);
    tit_end = rGen.TransRelEnd(currentstate);
    while(tit != tit_end) {
      FD_DF("ProjectNonDet: current transition: " << rGen.SStr(tit->X1)
	    << "-" << rGen.EStr(tit->Ev) << "-" << rGen.SStr(tit->X2));
      if(!rProjectAlphabet.Exists(tit->Ev)) {
	FD_DF("ProjectNonDet: deleting current transition");
	rGen.ClrTransition(tit++);
      } else {
	++tit;
      }
    }

  }// todo loop

  // restrict to projection alphabet, keep Attributes (could also use Inject here)
  rGen.RestrictAlphabet(rProjectAlphabet);

  // make accessile
  rGen.Accessible();
	
  // set name
  rGen.Name(name);

}


// Project, version 2014/03, Moor -- an alternative for some large generators.
// This argorithm is derived from the reference version. Subsequent to each forward
// reachablity analysis, the states at which only non-silent events are enabled 
// are identified as must-exit-states. From those, a backward search is conducted, to
// introduce more must-exit-states. Idealy, this resolvs the entire foreward reach region.
// This is beneficial when compared to the reference algorithm, in which only the
// first state of the region is resolved. However, the backward reach will stuck
// if the forard reach region contains strongly connected components. See also the 
// ProjectNonDet_scc variant, which addresses this issue.

void ProjectNonDet_fbr(Generator& rGen, const EventSet& rProjectAlphabet) {

  // HELPERS:

  std::stack<Idx> todod, todor, todof, todox;  // states todo 
  StateSet doned, doner, donef, donex;         // states done (aka have been put to todo) 
  Idx currentstate;                     // the currently processed state
  Idx reachstate;                       // the currently processed state in reach iteration
  Idx exitstate;                        // the currently processed state in exit search iteration
  StateSet candx;
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  TransSetX2EvX1 revrel;
  TransSetX2EvX1::Iterator rit;
  TransSetX2EvX1::Iterator rit_end;

  // NAME
  std::string name=CollapsString("ProjectNonDet(" + rGen.Name() + ")");

  // removing silent selfloops (to avoid trivial sccs and special cases in their treatment)
  FD_WPD(0,1, "ProjectNonDet() [FB-REACH]: remove silent selfloops");
  tit=rGen.TransRelBegin(); 
  tit_end=rGen.TransRelEnd(); 
  while(tit!=tit_end) {
    if(tit->X1 == tit->X2)
       if(!rProjectAlphabet.Exists(tit->Ev))
	 { rGen.ClrTransition(tit++); continue;}
    ++tit;
  }

  // initialize todo stack by init states
  for(sit=rGen.InitStatesBegin(); sit!=rGen.InitStatesEnd(); ++sit) {
    todod.push(*sit);
    doned.Insert(*sit); 
  }

  // process main todo stack
  while(!todod.empty()) {

    // loop callback
    FD_WPD(donex.Size(), rGen.Size(), "ProjectNonDet() [FB-REACH]: done/size: " 
	   <<  donex.Size()  << " / " << rGen.Size());

    // get top of the stack
    currentstate = todod.top();
    todod.pop();
    FD_DF("ProjectNonDet: ---- current state: " << rGen.SStr(currentstate));

    // bail out on trivial
    if(donex.Exists(currentstate)) continue;

    // local forward reach to find exit-only states to initialize todox
    FD_DF("ProjectNonDet: running f-reach on " << rGen.SStr(currentstate));
    revrel.Clear();  // local reverse map
    todor.push(currentstate);
    doner.Clear();
    doner.Insert(currentstate); 
    Idx lastfound=currentstate;
    while(!todor.empty()) {
      reachstate = todor.top();
      todor.pop();
      bool local=false;
      // iterate successors
      tit = rGen.TransRelBegin(reachstate);
      tit_end = rGen.TransRelEnd();  
      for(; tit != tit_end; ++tit) {
        if(tit->X1 != reachstate) break;
   	// add high-level events to main todo
	if(rProjectAlphabet.Exists(tit->Ev)) {
	  if(doned.Insert(tit->X2)) 
	    todod.push(tit->X2);
	}
        // add low-level events to reach todo
        else {
          lastfound=tit->X2;
          revrel.Insert(*tit);
          local=true;
          if(doner.Insert(tit->X2)) 
	    todor.push(tit->X2);
	}
      }
      // record exit states
      if(!local) {
        donex.Insert(reachstate); // global
        if(reachstate!=currentstate) todox.push(reachstate);
      }
    }

    // bail out on trivial
    if(donex.Exists(currentstate)) continue;

    FD_DF("ProjectNonDet: f-reach-proj found #" << doner.Size() << " reachable states");
    FD_DF("ProjectNonDet: f-reach-proj found #" << (donex*doner).Size() << " exit states");

    // have fallback candidate for exit states
    candx.Clear();
    if(!donex.Exists(lastfound)) candx.Insert(lastfound);
    FD_DF("ProjectNonDet: f-reach-proj found #" << candx.Size() << " exit candidates");

    // alternate backward and forward analysis until currentstate is resolved
    while(true) {

      // backward reach on exit states to re-link predecessors and to find new exit-only states
      FD_DF("ProjectNonDet: running b-reach-proj on exitstates");
      while(!todox.empty()) {
        exitstate = todox.top();
        todox.pop();
        FD_DF("ProjectNonDet: -- b-reach on exit state: " << rGen.SStr(exitstate));
        // a) record attributes
        bool mark= rGen.ExistsMarkedState(exitstate);
        // b) figure exit transitions
        TransSet exits;
        tit=rGen.TransRelBegin(exitstate); 
        tit_end=rGen.TransRelEnd(); 
        for(;tit!=tit_end; ++tit) {
          if(tit->X1!=exitstate) break;
          exits.Inject(Transition(0,tit->Ev,tit->X2));
	}
        // c) iterate and re-link pre-decessors
        rit = revrel.BeginByX2(exitstate);
        rit_end = revrel.End();
        while(rit != rit_end) {
          if(rit->X2!=exitstate) break;
          // skip non-local event (automatic by local/silent reverse relation)
  	  //if(rProjectAlphabet.Exists(rit->Ev)) {++rit; continue;}
          // skip states outside current forward reach (automatic by local/silent reverse relation)
          //if(!doner.Exists(rit->X1)) {++rit; continue;}
          // skip states that are known exit states
          if(donex.Exists(rit->X1)) {++rit; continue;}
          FD_DF("ProjectNonDet: -- b-reach predecessor: " << rGen.SStr(rit->X1));
          // propagate exit links
          tit=exits.Begin(); 
          tit_end=exits.End(); 
          for(;tit!=tit_end; ++tit) 
            rGen.SetTransition(rit->X1,tit->Ev,tit->X2);
          // propagate attributes
          if(mark) rGen.InsMarkedState(rit->X1);
          // unlink original local transition
          FD_DF("unlink " << rit->Str());
          rGen.ClrTransition(*rit);
          // test if we found a new exit state (the if is allways true because we went back by a silent event)
          if(!donex.Exists(rit->X1)) {
            bool local=false;
            tit = rGen.TransRelBegin(rit->X1); // still in revrel
            tit_end = rGen.TransRelEnd(rit->X1);  
            while(tit != tit_end) {
              if(rProjectAlphabet.Exists(tit->Ev)) { ++tit; continue;}
  	      if(tit->X2==exitstate) {revrel.Erase(*tit); rGen.ClrTransition(tit++); continue;} //optional
              ++tit; local=true; 
            }
            // record new exit-only states
            if(!local) {
              FD_DF("ProjectNonDet: b-reach new exit state: " << rGen.SStr(currentstate));
              todox.push(rit->X1);
              donex.Insert(rit->X1);
              candx.Erase(rit->X1);
    	    } 
            // record candidates to overcome ssc  
            else {
              candx.Insert(rit->X1);
	    }
  	  }
          // unlink original in reverse transition
          revrel.Erase(rit++);
        } // examine one exits state

      } // todox


      // break f-b-alternation
      if(doner<=donex) break;  // when dealt with entire local reach
      //if(donex.Exists(currentstate)) break; // when dealt with current state

      // do one forward reach 
      FD_DF("ProjectNonDet: b-reach-proj stuck with #" << donex.Size() << " exit states");
      FD_DF("ProjectNonDet: choosing first of #" << candx.Size() << " exit candidates");
      exitstate= *candx.Begin();
     
      // this is the std forward algorthm applied to the candidate exitstate
      // (except that there is no need to feed the outer stack todod)
      FD_DF("ProjectNonDet: running f-reach-proj on: " << rGen.SStr(exitstate));
      todof.push(exitstate);
      donef.Clear();
      donef.Insert(exitstate); 
      bool marked=rGen.ExistsMarkedState(exitstate);
      while(!todof.empty()) {
        reachstate = todof.top();
        todof.pop();
        //track marking
        marked|=rGen.ExistsMarkedState(reachstate);
        // iterate successors
        tit = rGen.TransRelBegin(reachstate);
        tit_end = rGen.TransRelEnd();  
        for(; tit != tit_end; ++tit) {
          if(tit->X1!=reachstate) break;
     	  // for high-level events: insert new transition
	  if(rProjectAlphabet.Exists(tit->Ev)) 
 	    rGen.SetTransition(exitstate, tit->Ev, tit->X2);
          // for low-level events: add new states to f-reach todo
    	  else {
            if(donef.Insert(tit->X2)) 
 	      todof.push(tit->X2);
	  }
	}
      } // reach
      FD_DF("ProjectNonDet: f-reach-proj found #" << donef.Size() << " states");
   
      // set marking
      if(marked) rGen.SetMarkedState(exitstate);

      // std forward algorithm continued - remove all silent transitions that leave exitstate
      tit = rGen.TransRelBegin(exitstate);
      tit_end = rGen.TransRelEnd();
      while(tit != tit_end) {
        if(tit->X1!=exitstate) break;
        if(!rProjectAlphabet.Exists(tit->Ev)) 
	  rGen.ClrTransition(tit++);
        else 
	++tit;
      }

      // record new exit state
#ifdef FAUDES_CHECKED
      if(donex.Exists(exitstate)) 
	FD_WARN("ProjectNonDet: ERROR: new exit state " << exitstate);
#endif
      todox.push(exitstate);
      donex.Insert(exitstate);
      candx.Erase(exitstate);

    }  // backward-forward alternation

  }// outer todod 

  // inject projection alphabet, keep Attributes 
  rGen.RestrictAlphabet(rProjectAlphabet);
  // make accessile 
  rGen.RestrictStates(doned);
  // set name
  rGen.Name(name);

}


// Project, version 2014/03, Moor -- an alternative for some large generators.
// This argorithm first substitutes each "silent strictly-connected compnent" by a 
// single state. The rational is that the usual forward reachalility analysis 
// would be applied multiple times for each such component, leading to something
// close to quadratic order. Once the silent components have been eliminated,
// a local backward reachability analysis can be applied, ending up with almost
// linear order. Of course, the effective gain of efficiency depends on the
// automata at hand --  
//
void ProjectNonDet_scc(Generator& rGen, const EventSet& rProjectAlphabet) {

  // HELPERS:

  std::stack<Idx> todod, todor, todof, todox;  // states todo 
  StateSet doned, doner, donef, donex;         // states done (aka have been put to todo) 
  Idx currentstate;                     // the currently processed state
  Idx reachstate;                       // the currently processed state in reach iteration
  Idx exitstate;                        // the currently processed state in exit search iteration
  StateSet candx;
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  TransSetX2EvX1 revrel;
  TransSetX2EvX1::Iterator rit;
  TransSetX2EvX1::Iterator rit_end;

  // NAME
  std::string name=CollapsString("ProjectNonDet(" + rGen.Name() + ")");

  // removing silent selfloops (to avoid trivial sccs and special cases in their treatment)
  FD_WPD(0,1, "ProjectNonDet() [SCC]: remove silent selfloops");
  tit=rGen.TransRelBegin(); 
  tit_end=rGen.TransRelEnd(); 
  while(tit!=tit_end) {
    if(tit->X1 == tit->X2)
       if(!rProjectAlphabet.Exists(tit->Ev))
	 { rGen.ClrTransition(tit++); continue;}
    ++tit;
  }

  // indentify local sccs
  FD_WPD(0,1, "ProjectNonDet() [SCC]: compute silent sccs");
  SccFilter locreach(
     SccFilter::FmEventsAvoid|SccFilter::FmIgnoreUnaccessible|SccFilter::FmIgnoreTrivial,
     rProjectAlphabet);
  std::list<StateSet> scclist;
  StateSet sccroots;
  ComputeScc(rGen,locreach,scclist,sccroots);

  // have one substitute state for each individual scc
  FD_WPD(0,1, "ProjectNonDet() [SCC]: processing #" << scclist.size() << " sccs");
  std::list<StateSet>::iterator cit=scclist.begin();
  std::list<StateSet>::iterator cit_end=scclist.end();
  std::map<Idx,Idx> sccxmap;
  // loop individual sccs
  for(;cit!=cit_end;++cit) {
    FD_WARN("ProjectNonDet() [SCC]: processing scc " << cit->ToString() );
    // track attributes
    bool init=false;
    bool mark=false;
    // introduce substitute state
    Idx sccx = rGen.InsState();
    // iterate over all transitions that start in current scc
    sit=cit->Begin();    
    sit_end=cit->End();    
    for(;sit!=sit_end;++sit){
      init|= rGen.ExistsInitState(*sit);
      mark|= rGen.ExistsMarkedState(*sit);
      tit=rGen.TransRelBegin(*sit); 
      tit_end=rGen.TransRelEnd(*sit); 
      while(tit!=tit_end) {
	// subtle bug related to an STL detail (TM/TW 2018-12)
	// our loop inserts transitions with X1=sccx and will not invalidate tit or
	// tit_end (since they refer to X1-range strictly below sscx) ... except for the
	// specaial case where *sit happens to be is the last X1-state in the
	// transition relation: then tit_end points the universal invalid transition and
	// our insertion with X1=sscx sneaks in before tit_end ... rather than an
	// expensive update of tit_end we sense the special case to break the loop
	if(tit->X1 != *sit) break;
        // case a) exit this scc with any event
        if(!cit->Exists(tit->X2)) {
          rGen.SetTransition(sccx,tit->Ev,tit->X2);
	}
        // case b) remains in this scc with non-silent event
        else {
          if(rProjectAlphabet.Exists(tit->Ev)) {
            rGen.SetTransition(sccx,tit->Ev,sccx);
	  }
	}
        // delete transition, increment ieterator
        rGen.ClrTransition(tit++);
      }
    }
    // propagate attributes to substitute state
    if(init) rGen.InsInitState(sccx);
    if(mark) rGen.InsMarkedState(sccx);
    // record substitution in map
    sit=cit->Begin();    
    sit_end=cit->End();    
    for(;sit!=sit_end;++sit) sccxmap[*sit]=sccx;
  }

  rGen.Write();

  // apply substitution
  if(sccxmap.size()>0){
    FD_WPD(0,1, "ProjectNonDet() [SCC]: applying state substitution for #" << sccxmap.size() << " states");
    std::map<Idx,Idx>::iterator xxit;
    tit=rGen.TransRelBegin(); 
    tit_end=rGen.TransRelEnd(); 
    while(tit!=tit_end) {
      xxit=sccxmap.find(tit->X2);
      if(xxit==sccxmap.end()) {++tit;continue;}
      rGen.SetTransition(tit->X1,tit->Ev,xxit->second);
      rGen.ClrTransition(tit++);
    }
  }


  // delete scc states 
  // (they dont have any transitions attached, this is cosmetic)
  cit=scclist.begin();
  cit_end=scclist.end();
  for(;cit!=cit_end;++cit) 
    rGen.DelStates(*cit);

  // free men
  scclist.clear();
  sccxmap.clear();
 
  // initialize todo stack by init states
  for(sit=rGen.InitStatesBegin(); sit!=rGen.InitStatesEnd(); ++sit) {
    todod.push(*sit);
    doned.Insert(*sit); 
  }

  // process main todo stack
  while(!todod.empty()) {

    // loop callback
    FD_WPD(donex.Size(), rGen.Size(), "ProjectNonDet() [FB-REACH]: done/size: " 
	   <<  donex.Size()  << " / " << rGen.Size());

    // get top of the stack
    currentstate = todod.top();
    todod.pop();
    FD_DF("ProjectNonDet: ---- current state: " << rGen.SStr(currentstate));

    // bail out on trivial
    if(donex.Exists(currentstate)) continue;

    // local forward reach to find exit-only states to initialize todox
    FD_DF("ProjectNonDet: running f-reach on " << rGen.SStr(currentstate));
    revrel.Clear();  // local reverse map
    todor.push(currentstate);
    doner.Clear();
    doner.Insert(currentstate); 
    Idx lastfound=currentstate;
    while(!todor.empty()) {
      reachstate = todor.top();
      todor.pop();
      bool local=false;
      // iterate successors
      tit = rGen.TransRelBegin(reachstate);
      tit_end = rGen.TransRelEnd();  
      for(; tit != tit_end; ++tit) {
        if(tit->X1 != reachstate) break;
   	// add high-level events to main todo
	if(rProjectAlphabet.Exists(tit->Ev)) {
	  if(doned.Insert(tit->X2)) 
	    todod.push(tit->X2);
	}
        // add low-level events to reach todo
        else {
          lastfound=tit->X2;
          revrel.Insert(*tit);
          local=true;
          if(doner.Insert(tit->X2)) 
	    todor.push(tit->X2);
	}
      }
      // record exit states
      if(!local) {
        donex.Insert(reachstate); // global
        if(reachstate!=currentstate) todox.push(reachstate);
      }
    }

    // bail out on trivial
    if(donex.Exists(currentstate)) continue;

    FD_DF("ProjectNonDet: f-reach-proj found #" << doner.Size() << " reachable states");
    FD_DF("ProjectNonDet: f-reach-proj found #" << (donex*doner).Size() << " exit states");

    // have fallback candidate for exit states
    candx.Clear();
    if(!donex.Exists(lastfound)) candx.Insert(lastfound);
    FD_DF("ProjectNonDet: f-reach-proj found #" << candx.Size() << " exit candidates");

    // backward reach on exit states to re-link predecessors and to find new exit-only states
    FD_DF("ProjectNonDet: running b-reach-proj on exitstates");
    while(!todox.empty()) {
      exitstate = todox.top();
      todox.pop();
      FD_DF("ProjectNonDet: -- b-reach on exit state: " << rGen.SStr(exitstate));
      // a) record attributes
      bool mark= rGen.ExistsMarkedState(exitstate);
      // b) figure exit transitions
      TransSet exits;
      tit=rGen.TransRelBegin(exitstate); 
      tit_end=rGen.TransRelEnd(); 
      for(;tit!=tit_end; ++tit) {
        if(tit->X1!=exitstate) break;
        exits.Inject(Transition(0,tit->Ev,tit->X2));
      }
      // c) iterate and re-link pre-decessors
      rit = revrel.BeginByX2(exitstate);
      rit_end = revrel.End();
      while(rit != rit_end) {
        if(rit->X2!=exitstate) break;
        // skip non-local event (automatic by local/silent reverse relation)
        //if(rProjectAlphabet.Exists(rit->Ev)) {++rit; continue;}
        // skip states outside current forward reach (automatic by local/silent reverse relation)
        //if(!doner.Exists(rit->X1)) {++rit; continue;}
        // skip states that are known exit states
        if(donex.Exists(rit->X1)) {++rit; continue;}
        FD_DF("ProjectNonDet: -- b-reach predecessor: " << rGen.SStr(rit->X1));
        // propagate exit links
        tit=exits.Begin(); 
        tit_end=exits.End(); 
        for(;tit!=tit_end; ++tit) 
          rGen.SetTransition(rit->X1,tit->Ev,tit->X2);
        // propagate attributes
        if(mark) rGen.InsMarkedState(rit->X1);
        // unlink original local transition
        FD_DF("unlink " << rit->Str());
        rGen.ClrTransition(*rit);
        // test if we found a new exit state (the if is allways true because we went back by a silent event)
        if(!donex.Exists(rit->X1)) {
          bool local=false;
          tit = rGen.TransRelBegin(rit->X1); // still in revrel
          tit_end = rGen.TransRelEnd(rit->X1);  
          while(tit != tit_end) {
            if(rProjectAlphabet.Exists(tit->Ev)) { ++tit; continue;}
  	    if(tit->X2==exitstate) {revrel.Erase(*tit); rGen.ClrTransition(tit++); continue;} //optional
            ++tit; local=true; 
          }
          // record new exit-only states
          if(!local) {
            FD_DF("ProjectNonDet: b-reach new exit state: " << rGen.SStr(currentstate));
            todox.push(rit->X1);
            donex.Insert(rit->X1);
            candx.Erase(rit->X1);
    	  } 
          // record candidates to overcome ssc  
          else {
            candx.Insert(rit->X1);
          }
        }
        // unlink original in reverse transition
        revrel.Erase(rit++);
      } // examine one exits state

    } // todox

    // since we eliminated silent sccs, we must have doner<=donex,
    // and we dont need to run a forward in this routine
#ifdef FAUDES_CHECKED
    if(!donex.Exists(currentstate)) 
      FD_WARN("ProjectNonDet: ERROR: b-reach-proj stuck with #" << donex.Size() << " exit states");
#endif
 

  }// outer todod 

  // inject projection alphabet, keep Attributes 
  rGen.RestrictAlphabet(rProjectAlphabet);
  // make accessile 
  rGen.RestrictStates(doned);
  // set name
  rGen.Name(name);

  FD_WPD(0,1, "ProjectNonDet() [SCC]: done");
}


			
// wrapper to choose std implementation 
void ProjectNonDet(Generator& rGen, const EventSet& rProjectAlphabet) {
  ProjectNonDet_ref(rGen, rProjectAlphabet);     // (here: 2014 re-code of original -2008 algorithm)
  //ProjectNonDet_opitz(rGen, rProjectAlphabet);   // (here: original code used -2008)
  //ProjectNonDet_barthel(rGen, rProjectAlphabet); // (here: implementation used 2009-2013)
  //ProjectNonDet_fbr(rGen, rProjectAlphabet);     // (here: 2014 optimized algorithm)
  //ProjectNonDet_scc(rGen, rProjectAlphabet);     // (here: 2014 optimized algorithm)
}

// wrapper to make the scc version available externally (purely cosmetic)
void ProjectNonDetScc(Generator& rGen, const EventSet& rProjectAlphabet) {
  ProjectNonDet_scc(rGen, rProjectAlphabet); // (here: 2014 optimized algorithm)
}


// Project(rGen, rProjectAlphabet, rResGen&)
void Project(const Generator& rGen, const EventSet& rProjectAlphabet, Generator& rResGen) {
  FD_DF("Project(...): #" << rGen.TransRelSize());
  //FAUDES_TIMER_START("");
  // initialize result with argument generator
  if(&rResGen != &rGen) rResGen.Assign(rGen);
  // turn off state names
  bool se= rResGen.StateNamesEnabled();
  rResGen.StateNamesEnabled(false);
  // project non det version
  ProjectNonDet(rResGen, rProjectAlphabet);
  // make deterministic
  Generator* gd = rGen.New(); 
  gd->StateNamesEnabled(false);
  //FAUDES_TIMER_LAP("");
  FD_DF("Project(...): make det #" << rResGen.TransRelSize());
  Deterministic(rResGen, *gd);
  // minimize states  (tmoor 201308: this is cosmetic ... 
  // ... and turned out expensive when iterating on an observer 
  // stateset; hence we do it only for small generators)
  if(gd->Size() < 20)
    StateMin(*gd,rResGen);
  else
    gd->Move(rResGen);
  delete gd;
  // restore state names
  rResGen.StateNamesEnabled(se);
  // set name
  rResGen.Name("Project("+CollapsString(rGen.Name()+")")); 
  //FAUDES_TIMER_LAP("");
  FD_DF("Project(...): done #" << rResGen.TransRelSize());
  
  /*
  // compare with reference implementation
  FD_WARN("Project(...): testing #" << rGen.TransRelSize());
  FAUDES_TIMER_START("");
  Generator gref=rGen;
  gref.StateNamesEnabled(false);
  ProjectNonDet_ref(gref, rProjectAlphabet);
  Generator g3;
  g3.StateNamesEnabled(false);
  FD_WARN("Project(...): make det #" << gref.Size());
  FAUDES_TIMER_LAP("");
  Deterministic(gref, g3);
  g3.Move(gref);
  FAUDES_TIMER_LAP("");
  // compare
  FD_WARN("Project(...): compare #" << gref.Size());
  FD_WARN("Project(...): ref<=alt " << LanguageInclusion(gref,rResGen));
  FD_WARN("Project(...): alt<=ref " << LanguageInclusion(rResGen,gref));
  if(!LanguageEquality(gref,rResGen))    throw Exception("Project(Generator,EventSet)", "TEST ERROR", 506);
  Generator resgen=rResGen;
  MarkAllStates(resgen);
  MarkAllStates(gref);
  FD_WARN("Project(...): compare closed");
  FD_WARN("Project(...): ref<=alt " << LanguageInclusion(gref,resgen));
  FD_WARN("Project(...): alt<=ref " << LanguageInclusion(resgen,gref));
  if(!LanguageEquality(gref,resgen))    throw Exception("Project(Generator,EventSet)", "TEST ERROR", 506);  
  */
}

	 
// wrapper
void aProjectNonDet(Generator& rGen, const EventSet& rProjectAlphabet) {
  ProjectNonDet(rGen,rProjectAlphabet);
}


// Project(rGen, rProjectAlphabet, rResGen&)
void aProject(const Generator& rGen, const EventSet& rProjectAlphabet, Generator& rResGen) {
  // prepare result to keep original alphabet
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen) {
    pResGen= rResGen.New();
  }
  // perform op
  Project(rGen,rProjectAlphabet,*pResGen);
  // set old attributes
  pResGen->EventAttributes(rGen.Alphabet());
  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}



// Project(rGen, rProjectAlphabet, rEntryStatesMap&, rResGen&)
void Project(const Generator& rGen, const EventSet& rProjectAlphabet,
	     std::map<Idx,StateSet>& rEntryStatesMap, Generator& rResGen) {
  FD_DF("Project(...)");
  // temporary entry state map
  std::map<Idx,StateSet> tmp_entrystatemap;
  // temporarily assign rGen to rResGen
  if(&rResGen != &rGen) rResGen.Assign(rGen); 
  // project tmp with respect to palphabet
  ProjectNonDet_ref(rResGen, rProjectAlphabet);  // must use a version that does not add states
  // put deterministic result into tmp
  Generator* tmp = rGen.New();
  Deterministic(rResGen, tmp_entrystatemap, *tmp);
  // write entry state map for minimized generator
  std::vector<StateSet> subsets;
  std::vector<Idx> newindices;
  // minimize states and rewrite result to rResGen
  StateMin(*tmp, rResGen, subsets, newindices);
  // build entry state map
  std::vector<StateSet>::size_type i;
  std::map<Idx,StateSet>::iterator esmit;
  StateSet::Iterator sit;
  for (i = 0; i < subsets.size(); ++i) {	
    StateSet tmpstates;
    for (sit = subsets[i].Begin(); sit != subsets[i].End(); ++sit) {
      esmit = tmp_entrystatemap.find(*sit);
#ifdef FAUDES_DEBUG_CODE
      if (esmit == tmp_entrystatemap.end()) {
        FD_DF("project internal error");
	abort();
      }
#endif
      // insert entry states in temporary StateSet
      tmpstates.InsertSet(esmit->second);
    }

    rEntryStatesMap.insert(std::make_pair(newindices[i], tmpstates));
  }
  delete tmp;
}


// InvProject(rGen&, rProjectAlphabet)
void InvProject(Generator& rGen, const EventSet& rProjectAlphabet) {
  // test if the alphabet of the generator is included in the given alphabet
  if(! (rProjectAlphabet >= (EventSet) rGen.Alphabet() ) ){
    std::stringstream errstr;
    errstr << "Input alphabet has to contain alphabet of generator \"" << rGen.Name() << "\"";
    throw Exception("InvProject(Generator,EventSet)", errstr.str(), 506);
  }
  EventSet newevents = rProjectAlphabet - rGen.Alphabet();
  // insert events into generator
  rGen.InsEvents(newevents);
  FD_DF("InvProject: adding events \"" << newevents.ToString() 
	<< "\" at every state");
  StateSet::Iterator lit;
  EventSet::Iterator eit;
  for (lit = rGen.StatesBegin(); lit != rGen.StatesEnd(); ++lit) {
    LoopCallback(); // should only be an issue for very large generators 
    for (eit = newevents.Begin(); eit != newevents.End(); ++eit) {
      rGen.SetTransition(*lit, *eit, *lit);
    }
  }
}



// InvProject(rGen&, rProjectAlphabet)
void aInvProject(Generator& rGen, const EventSet& rProjectAlphabet) {
  FD_DF("aInvProject(..)");
  // see whether the generator can digest attribues
  if(!rGen.Alphabet().AttributeTest(*rProjectAlphabet.AttributeType())) {
    InvProject(rGen,rProjectAlphabet);
    return;
  }
  // record extra events
  EventSet newevents = rProjectAlphabet - rGen.Alphabet();
  // perform
  InvProject(rGen,rProjectAlphabet);
  // copy all attributes from input alphabets
  FD_DF("aInvProject(..): fixing attributes: source " << typeid(*rProjectAlphabet.AttributeType()).name() <<
	  " dest " << typeid(*rGen.Alphabet().AttributeType()).name());
  for(EventSet::Iterator eit=newevents.Begin(); eit!=newevents.End(); ++eit)  
    rGen.EventAttribute(*eit,rProjectAlphabet.Attribute(*eit));
}


// InvProject
void aInvProject(
  const Generator& rGen,
  const EventSet& rProjectAlphabet,
  Generator& rResGen)
{
  rResGen.Assign(rGen);
  aInvProject(rResGen, rProjectAlphabet);
}



// CreateEntryStatesMap(rRevEntryStatesMap, rEntryStatesMap)
void CreateEntryStatesMap(const std::map<StateSet,Idx>& rRevEntryStatesMap,
			  std::map<Idx,StateSet>& rEntryStatesMap) {
  std::map<StateSet,Idx>::const_iterator it;
  for (it = rRevEntryStatesMap.begin(); it != rRevEntryStatesMap.end(); ++it) {
    rEntryStatesMap.insert(std::make_pair(it->second, it->first));
  }
}



} // namespace faudes
