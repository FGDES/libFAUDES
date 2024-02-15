/** @file syn_synthequiv.cpp Synthesis-observation equivalence **/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2015  Hao Zhou, Thomas Moor
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


#include "syn_synthequiv.h"

/*
Notes:

This file is extracted from "libzhou.h" and "libzhou.cpp", 
originally written by Hao Zhou in course of his Master Thesis. 
The merge is meant to minimise the public interface and to decouple 
side effects of further development of libFAUDES. 

*/

// pragmatic coding ... ok its not a header
using namespace std;

namespace faudes {

/*
****************************************************************
****************************************************************
PART 1: Partition
****************************************************************
****************************************************************
*/

struct Pnode {

  /** unique partition index */
  Idx index;

  /** Associated set of states */
  std::set<Idx> states;

  /** number of states in this coset */
  Idx nsize;

};

/*
****************************************************************
****************************************************************
PART 2: SOE

The class SOE is used to compute the coarsest synthesis-observation-equivalence
relation and to maintain relevant data structures required for the particular implementation.

****************************************************************
****************************************************************
*/

class SOE {

public:

  /**
   * Contructor: 
   * keep a reference to the generator and initialize the partition and the W-Tree
   * to represent the universal equivalence relation, i.e. every two states are equivalent.
   *
   * @param rGenOrig
   *   Original generator
   * @param rConAlph
   *   controllable events
   * @param rLocAlph
   *   local events  
   */
  SOE(const Generator& rGenOrig, const EventSet& rConAlph, const EventSet& rLocAlph);

  /**
   * Destructor
   */
  ~SOE(void);

  /**
   * Perform fixpoint iteration to obtain the coarset synthesis-observation-equivalence
   */
  void refine(void);

  /**
   * Extract output generator that represents the resulting quotient automaton.
   * (need to invoke refine() befor)
   *
   * @param rMapStateToPartition
   *   Maps each state to its equivalence class
   * @param rGenPart
   *   Generator representing the result of the computation.
   *   Each state corresponds to an euivalence class
   */
  void partition(std::map<Idx,Idx>& rMapStateToPartition, Generator& rGenPart);

private:

  /** 
   * internal representation of transition relation with consecutive indexed states and events
   */
  struct State {
    /** Part 1 */
    Idx idx;
    vector< vector<Idx> > suc; // only for Initialization
    vector< vector<Idx> > pre; // only for Initialization

    /** Part 2 (only via an transition) */
    vector< vector<Idx> > shaconPre;  // precessors with a shared controllable event
    vector< vector<Idx> > shauncPre;  // precessors with a shared uncontrollable event
    vector<Idx> locconPre;  // precessors with a local controllable event       
    vector<Idx> locuncPre;  // precessors with a local uncontrollable event

    /** Part 3: */
    // the primal precessors which continuously via local uncontrollable events to this state 
    vector<Idx> locuncPres; 

    // the precessors which firstly via a shared uncontrollable event and then 
    // continuously via local uncontrollable events to this state    
    vector< vector<Idx> > shauncPres;  
  };

  /**
   * a relation according to SOE in one block
   */
  struct Relation {
    Idx ev; // Event
    std::set<Idx> pre;  // Precessors
  };

  /** 
   * vector of all states [starting with 1]
   */
  std::vector<State> states;

  /**
   * vector of all events [starting with 1]
   */
  std::vector<Idx> events;

  /**
   * keep a reference to automaton
   */
  const Generator* gen;

  /**
   * vorious eventsets of interest
   */
  std::set<Idx> uncalph;  // uncontrollable events
  std::set<Idx> conalph;  // controllable events
  std::set<Idx> localph;  // local events
  std::set<Idx> shaalph;  // shared events

  std::set<Idx> locuncalph;  // local uncontrollable events
  std::set<Idx> locconalph;  // local controllable events
  std::set<Idx> shauncalph;  // shared uncontrollable events
  std::set<Idx> shaconalph;  // shared controllable events

  /**
   * Counter to assign a unique index to each node [debugging/cosmetic only] 
   */
  Idx nxidx;

  /**
   * W-tree contains all blocks ever created [required for destruction]
   */
  std::vector<Pnode*> W;

  /**
   * set of nodes that form current partition, i.e. leaves of W.
   */
  std::set<Pnode*> ro;

  /**
   * set of nodes that can possibly split classes in ro
   */
  std::set<Pnode*> roDividers;

  /**
   * construct 2st Part of Struct "State"
   */
  void initStateMember_Pre();

  /**
   * construct 3st Part of Struct "State"
   */
  void initStateMember_Pres();

  /**
   * insert new node in W-tree with empty stateset
   */
  Pnode* newnode();

  /**
   * refine current partition with respect to coset B
   *
   * @param B
   *   coset
   */
  void partitionClass(Pnode& B);

  /**
   * construct all relations with respect to coset B
   *
   * @param B
   *   coset
   * @param relations
   *   vector of all relations
   */
  void computeRel(Pnode& B, std::vector<Relation>& relations);

  /**
   * collect all states from coset B which are equivalent to the speicified state
   * and determine the related nodes which is possibly to be splitted
   *
   * @param B
   *   coset
   * @param rel
   *   a specified relation
   * @param tb
   *   all equivalent states with respect to rel
   * @param todo
   *   all related nodes which is possibly to be splitted
   */
  void computeEquStates(Pnode& B, Relation& rel, std::set<Idx>& tb, std::stack<Pnode*>& todo);

  /**
   * implementation part for function "computeEquStates"
   * it is organized as follows with respect to various cases:
   */ 
  void relCase_1(Pnode& B, Relation& rel, set<Idx>& tb, stack<Pnode*>& todo);
  void relCase_2(Pnode& B, Relation& rel, set<Idx>& tb, stack<Pnode*>& todo);
  void relCase_3(Pnode& B, Relation& rel, set<Idx>& tb, stack<Pnode*>& todo);
  void relCase_4(Pnode& B, Relation& rel, set<Idx>& tb, stack<Pnode*>& todo);

};


// maintain W-tree: new node
Pnode* SOE::newnode(void)
{
  Pnode* nn= new Pnode();
  nn->nsize=0;
  nn->index=nxidx;
  nxidx++;
  W.push_back(nn);
  return nn;
}


// destruct
SOE::~SOE(void) 
{
  std::vector<Pnode*>::iterator wit=W.begin();
  for(; wit!=W.end(); ++wit) delete *wit;
}


// Constructor SOE(rGenOrig, rConAlph, rLocAlph)
SOE::SOE(const Generator& rGenOrig, const EventSet& rConAlph, const EventSet& rLocAlph)
{
  gen = &rGenOrig;
  nxidx=1;
  
  // encode transition relation [effectively buffer log-n search]
  std::map<Idx,Idx> smap;
  std::map<Idx,Idx> emap;
  Idx max=0;

  events.resize(gen->Alphabet().Size()+1);
  EventSet::Iterator eit=gen->AlphabetBegin();
  for(; eit != gen->AlphabetEnd(); ++eit) {
    emap[*eit]=++max;
    events[max]=*eit;
  }

  max=0;
  states.resize(gen->States().Size()+1);
  StateSet::Iterator sit=gen->StatesBegin();
  for(; sit != gen->StatesEnd(); ++sit) {
    smap[*sit]=++max;
    states[max].idx=*sit;
    states[max].pre.resize(events.size());
    states[max].suc.resize(events.size());
    states[max].shaconPre.resize(events.size());
    states[max].shauncPre.resize(events.size());
    states[max].shauncPres.resize(events.size());
  }

  TransSet::Iterator tit=gen->TransRelBegin();
  for(; tit != gen->TransRelEnd(); ++tit) {
    states[smap[tit->X2]].pre[emap[tit->Ev]].push_back(smap[tit->X1]);
    states[smap[tit->X1]].suc[emap[tit->Ev]].push_back(smap[tit->X2]);
  }

  // construct various alphabets
  eit=rConAlph.Begin();
  for(; eit != rConAlph.End(); ++eit) conalph.insert(emap[*eit]);   // controllable
  eit=rLocAlph.Begin();
  for(; eit != rLocAlph.End(); ++eit) localph.insert(emap[*eit]);   // local
  Idx ev=1;
  for(; ev < events.size(); ++ev) 
    if(conalph.find(ev) == conalph.end()) uncalph.insert(ev);       // uncontrollable
  ev=1;
  for(; ev < events.size(); ++ev) 
    if(localph.find(ev) == localph.end()) shaalph.insert(ev);       // shared

  std::set_intersection(localph.begin(), localph.end(), uncalph.begin(), uncalph.end(),
			std::inserter(locuncalph, locuncalph.begin())); // locunc
  std::set_intersection(localph.begin(), localph.end(), conalph.begin(), conalph.end(),
			std::inserter(locconalph, locconalph.begin())); // loccon
  std::set_intersection(shaalph.begin(), shaalph.end(), uncalph.begin(), uncalph.end(),
			std::inserter(shauncalph, shauncalph.begin())); // shaunc
  std::set_intersection(shaalph.begin(), shaalph.end(), conalph.begin(), conalph.end(),
			std::inserter(shaconalph, shaconalph.begin())); // shacon

  // for Pre and Pres
  initStateMember_Pre();
  initStateMember_Pres();

  // create universal partition holding the complete state set
  Pnode* universe = newnode(); 
  Idx st=1;
  for(;st<states.size();++st) universe->states.insert(st); 
  universe->nsize = universe->states.size();

  ro.insert(universe);
  roDividers.insert(universe);

}

// continue to add other member in State
void SOE::initStateMember_Pre()
{
  // helpers
  std::set<Idx>::iterator sit;
  std::vector<Idx>::iterator vit;

  // construct State.shaconPre
  sit=shaconalph.begin();
  for(; sit!=shaconalph.end(); ++sit) {
    Idx idx=1;
    for(; idx < states.size(); ++idx) {
      vit=states[idx].pre[*sit].begin();
      for(;vit!=states[idx].pre[*sit].end();++vit)
	states[idx].shaconPre[*sit].push_back(*vit);
    }
  }

  // construct State.shauncPre
  sit=shauncalph.begin();
  for(;sit!=shauncalph.end();++sit) {
    Idx idx=1;
    for(;idx<states.size();++idx) {
      vit=states[idx].pre[*sit].begin();
      for(;vit!=states[idx].pre[*sit].end();++vit)
	states[idx].shauncPre[*sit].push_back(*vit);
    }
  }

  // construct State.locconPre
  sit=locconalph.begin();
  for(;sit!=locconalph.end();++sit) {
    Idx idx=1;
    for(;idx<states.size();++idx) {
      vit=states[idx].pre[*sit].begin();
      for(;vit!=states[idx].pre[*sit].end();++vit)
	states[idx].locconPre.push_back(*vit);
    }
  }

  // construct State.locuncPre
  sit=locuncalph.begin();
  for(;sit!=locuncalph.end();++sit) {
    Idx idx=1;
    for(;idx<states.size();++idx) {
      vit=states[idx].pre[*sit].begin();
      for(;vit!=states[idx].pre[*sit].end();++vit)
	states[idx].locuncPre.push_back(*vit);
    }
  }

}

void SOE::initStateMember_Pres()
{
  // helpers
  std::stack<Idx> todo;
  std::vector<Idx>::iterator vit;
  std::vector<Idx>::iterator Ivit;

  // construct locuncPres 
  Idx idx=1;
  for(;idx<states.size();++idx) {
    vit=states[idx].locuncPre.begin();
    for(;vit!=states[idx].locuncPre.end();++vit) {
      todo.push(*vit);
      while(!todo.empty()) {
	Idx st=todo.top();
	todo.pop();
	if(std::find(states[idx].locuncPres.begin(),states[idx].locuncPres.end(),st)
	   !=states[idx].locuncPres.end()) continue;
	states[idx].locuncPres.push_back(st);
	Ivit=states[st].locuncPre.begin();
	for(;Ivit!=states[st].locuncPre.end();++Ivit)
	  todo.push(*Ivit);
      }
    }
  }

  std::set<Idx> tempset;
  std::set<Idx>::iterator sit;
  std::vector<Idx>::iterator IIvit;

  // construct shauncPres ( without near, i.e. without shauncPre )
  sit=shauncalph.begin();
  for(;sit!=shauncalph.end();++sit) {
    Idx idx=1;
    for(;idx<states.size();++idx) {
      vit=states[idx].locuncPre.begin();
      for(;vit!=states[idx].locuncPre.end();++vit) {
	todo.push(*vit);
	while(!todo.empty()){
	  Idx st=todo.top();
	  todo.pop();
	  if(tempset.find(st)!=tempset.end()) continue;
	  tempset.insert(st);
	  Ivit=states[st].locuncPre.begin();
	  for(;Ivit!=states[st].locuncPre.end();++Ivit)
	    todo.push(*vit);
	  IIvit=states[st].shauncPre[*sit].begin();
	  for(;IIvit!=states[st].shauncPre[*sit].end();++IIvit)
	    states[st].shauncPres[*sit].push_back(*IIvit);
	}
      }
    }
  }

} 
// SOE::partitionClass(B)
void SOE::partitionClass(Pnode& B)
{

  // helpers
  vector<Relation> relations;
  std::stack<Pnode*> todo;  // the related node in a particular relation
  std::vector<Pnode*> addSet;
  std::vector<Pnode*> removeSet;
  // iterators
  vector<Relation>::iterator relIt;
  std::vector<Pnode*>::iterator addIt;
  std::vector<Pnode*>::iterator remIt;

  // collect all relations for this block
  computeRel(B, relations);  // *extra funktion:1

  // loop over every relation
  relIt=relations.begin();
  for(; relIt!=relations.end(); ++relIt) {
    //std::cout<<"        now coming to one relation"<<std::endl;
    // compute all equivalent states for the relation
    std::set<Idx> tb;
    computeEquStates(B, *relIt, tb, todo); // *extra funktion:2
    //std::cout<<" todo is empty ->"<<(true==todo.empty()?"yes":"no")<<std::endl;

    // iteration over all cosets on todo-stack
    while(!todo.empty()) {
      Pnode* rop=todo.top();
      todo.pop();
      
      // skip Nr.1:  singletons
      if(rop->nsize==1) continue;
      // skip Nr.2:  if it is the last node (repeated), then now is empty
      if(rop->states.empty()) continue;

      // compute intersection of relstates with current candidate coset
      std::set<Idx> sXinter;
      std::set_intersection(rop->states.begin(), rop->states.end(),
			    tb.begin(), tb.end(),
			    std::inserter(sXinter, sXinter.begin()));
      
      // skip Nr.3 : the current coset is a subset of relstates
      if(sXinter.size()==rop->states.size()) continue;

      // delete from ro and roDividers
      roDividers.erase(rop);
      removeSet.push_back(rop);
      
      // set up new node for intersection 
      Pnode* nXinter= newnode();
      nXinter->states.swap(sXinter);
      nXinter->nsize=sXinter.size();
      
      // set up new node for difference
      Pnode* nXdiff = newnode();
      std::set_difference(rop->states.begin(), rop->states.end(), nXinter->states.begin(),
			  nXinter->states.end(), 
			  std::inserter(nXdiff->states, nXdiff->states.begin()));
      nXdiff->nsize=nXdiff->states.size();
      
      // record that the new cosets have to be added to ro and roDividers
      addSet.push_back(nXinter);
      addSet.push_back(nXdiff);
      roDividers.insert(nXinter);                
      roDividers.insert(nXdiff);

      // delete stateSet of split coset as the states are now stored in the child cosets
      rop->states.clear();

    } // end loop over all related nodes with the particular relation in todo
    
    // delete split partitions from ro
    for(remIt=removeSet.begin(); remIt!=removeSet.end(); ++remIt) ro.erase(*remIt);
    removeSet.clear();
    
    // insert the new cosets into ro
    for(addIt=addSet.begin(); addIt!=addSet.end(); ++addIt) ro.insert(*addIt);
    addSet.clear();

    // if dividers has been split, then no more continue
    if(B.states.empty()) return;

  } // end loop over all relations
  
  // delete coset B from roDividers
  roDividers.erase(&B);

}

// computeRel(B, relations)
// collect complete relations in this block and save in relations
void SOE::computeRel(Pnode& B, std::vector<Relation>& relations)
{

  // helpers
  Relation rel;
  // iterators
  std::set<Idx>::iterator sit;   // states in node
  std::set<Idx>::iterator evit;  // event iterator
  std::vector<Idx>::iterator vit; // pre states 

  // *relation cases keep in the sequence (wichtig or a better sequence)

  // relation case 1: shared controllable
  for(evit=shaconalph.begin(); evit!=shaconalph.end(); ++evit) {
    // loop all states in B
    for(sit=B.states.begin(); sit!=B.states.end(); ++sit) {
      // collect ev and pre if possible
      vit=states[*sit].shaconPre[*evit].begin();
      for(;vit!=states[*sit].shaconPre[*evit].end();++vit) {
	rel.pre.clear();
	rel.ev=*evit;
	rel.pre.insert(*vit);
	relations.push_back(rel);
      }
    }
  }

  // relation case 2: shared uncontrollable
  for(evit=shauncalph.begin();evit!=shauncalph.end();++evit) {
    rel.pre.clear();
    rel.ev=*evit;
    // loop all states in B
    for(sit=B.states.begin(); sit!=B.states.end(); ++sit) {
      // 
      vit=states[*sit].shauncPre[*evit].begin();
      for(;vit!=states[*sit].shauncPre[*evit].end();++vit) {
	rel.pre.insert(*vit);
      }
    }
    if(!rel.pre.empty()) relations.push_back(rel);
  }
  
  // relation case 3: local controllable 
  for(evit=locconalph.begin(); evit!=locconalph.end(); ++evit) {
    // loop all states in B
    for(sit=B.states.begin(); sit!=B.states.end(); ++sit) {
      // collect ev and pre if possible
      vit=states[*sit].locconPre.begin();
      for(;vit!=states[*sit].locconPre.end();++vit) {
	rel.pre.clear();
	rel.ev=*evit;
	rel.pre.insert(*vit);
	relations.push_back(rel);
      }
    }
  }
  
  // relation case 4: local uncontrollable
  rel.pre.clear();
  for(evit=locuncalph.begin();evit!=locuncalph.end();++evit) {
    rel.ev=*evit;
    // loop all states in B
    for(sit=B.states.begin(); sit!=B.states.end(); ++sit) {
      vit=states[*sit].locuncPre.begin();
      for(;vit!=states[*sit].locuncPre.end();++vit) 
	rel.pre.insert(*vit);
    }
  }
  if(!rel.pre.empty()) relations.push_back(rel);

}

// computeEquStates(B, rel, tb, todo)
// add info to "tb" and "todo"
void SOE::computeEquStates(Pnode& B, Relation& rel, std::set<Idx>& tb,
			    std::stack<Pnode*>& todo)
{
  // select one through "rel.ev" from 4 cases 

  // case 1: shared controllable event
  if(shaconalph.find(rel.ev)!=shaconalph.end()) {
    relCase_1(B, rel, tb, todo); return;}
  // case 2: shared uncontrollable event
  else if(shauncalph.find(rel.ev)!=shauncalph.end()) {
    relCase_2(B, rel, tb, todo); return;}
  // case 3: local controllable event
  else if(locconalph.find(rel.ev)!=locconalph.end()) {
    relCase_3(B, rel, tb, todo); return;}
  // case 4: local uncontrollable event
  else if(locuncalph.find(rel.ev)!=locuncalph.end()) {
    relCase_4(B, rel, tb, todo); return;}
  else 
    std::cout<<"####[SOE]this can not happen[SOE]####\n";

}

// case1: shared controllable
//relCase_1(B, rel, tb, todo)
void SOE::relCase_1(Pnode& B, Relation& rel, set<Idx>& tb, stack<Pnode*>& todo) 
{

  //helpers
  std::stack<Idx> stodo;
  std::set<Idx>::iterator sit;
  std::vector<Idx>::iterator vit;
  std::set<Pnode*>::iterator roIt;

  // find the related pnode which include the states in "rel"
  sit=rel.pre.begin();
  for(; sit!=rel.pre.end(); ++sit) {
    roIt=ro.begin();
    for(; roIt!=ro.end(); ++roIt) {
      if((*roIt)->states.find(*sit)!=(*roIt)->states.end()) {
	todo.push(*roIt);
	break;
      }
    }
  }

  // part1: near
  sit=B.states.begin();
  for(;sit!=B.states.end();++sit) {
    vit=states[*sit].shaconPre[rel.ev].begin();
    for(;vit!=states[*sit].shaconPre[rel.ev].end();++vit) 
      stodo.push(*vit);
  }
  
  // Part2: far 
  while(!stodo.empty()) {
    Idx idx=stodo.top();
    stodo.pop();
    if(tb.find(idx)!=tb.end()) continue;
    tb.insert(idx);
    // local uncontrollable
    vit=states[idx].locuncPre.begin();
    for(;vit!=states[idx].locuncPre.end();++vit)
      stodo.push(*vit);
    // local controllable + condition
    vit=states[idx].locconPre.begin();
    for(;vit!=states[idx].locconPre.end();++vit)
      if(todo.top()->states.find(*vit)!=todo.top()->states.end())
	stodo.push(*vit);
  }

}

// case2: shared uncontrollable
// relCase_2(B, rel, tb, todo)
void SOE::relCase_2(Pnode& B, Relation& rel, set<Idx>& tb, stack<Pnode*>& todo) 
{

  //helpers
  std::set<Idx>::iterator sit;
  std::vector<Idx>::iterator vit;
  std::set<Pnode*>::iterator roIt;

  // find the related pnode which include the states in "rel"
  sit=rel.pre.begin();
  for(; sit!=rel.pre.end(); ++sit) {
    roIt=ro.begin();
    for(; roIt!=ro.end(); ++roIt) {
      if((*roIt)->states.find(*sit)!=(*roIt)->states.end()) {
	todo.push(*roIt);
	break;
      }
    }
  }

  // part 1.1 (near)
  tb=rel.pre;

  // part 1.2 (far)
  sit=B.states.begin();
  for(; sit != B.states.end(); ++sit) {
    vit=states[*sit].shauncPres[rel.ev].begin();
    for(; vit!=states[*sit].shauncPres[rel.ev].end(); ++vit) tb.insert(*vit);
  }

  // part 2 
  sit=tb.begin();
  for(; sit != tb.end(); ++sit) {
    vit=states[*sit].locuncPres.begin();
    for(; vit != states[*sit].locuncPres.end(); ++vit) tb.insert(*vit);
  }

}

// case3: local controllable
//relCase_3(B, rel, tb, todo)
void SOE::relCase_3(Pnode& B, Relation& rel, set<Idx>& tb, stack<Pnode*>& todo) 
{

  //helpers
  std::stack<Idx> stodo;
  std::set<Idx>::iterator sit;
  std::vector<Idx>::iterator vit;
  std::set<Pnode*>::iterator roIt;

  // find the related pnode which include the states in "rel"
  sit=rel.pre.begin();
  for(; sit!=rel.pre.end(); ++sit) {
    roIt=ro.begin();
    for(; roIt!=ro.end(); ++roIt) {
      if((*roIt)->states.find(*sit)!=(*roIt)->states.end()) {
	todo.push(*roIt);
	break;
      }
    }
  }

  // all states in B belongs to "tb"
  tb=B.states;

  // Part1; near 
  sit=B.states.begin();
  for(; sit!=B.states.end(); ++sit) {
    vit=states[*sit].locconPre.begin();
    for(; vit != states[*sit].locconPre.end(); ++vit) 
      stodo.push(*vit);
  }

  // Part2: far 
  while(!stodo.empty()) {
    Idx idx=stodo.top();
    stodo.pop();
    if(tb.find(idx)!=tb.end()) continue;
    tb.insert(idx);
    // local uncontrollable
    vit=states[idx].locuncPre.begin();
    for(;vit!=states[idx].locuncPre.end();++vit)
      stodo.push(*vit);
    // local controllable + condition
    vit=states[idx].locconPre.begin();
    for(;vit!=states[idx].locconPre.end();++vit)
      if(todo.top()->states.find(*vit)!=todo.top()->states.end())
	stodo.push(*vit);
  }

}

// case 4: local uncontrollable 
// relCase_4(B, rel, tb, todo)
void SOE::relCase_4(Pnode& B, Relation& rel, set<Idx>& tb, stack<Pnode*>& todo)
{

  // helpers
  std::vector<Idx>::iterator vit;
  std::set<Idx>::iterator sit;
  std::set<Pnode*>::iterator roIt;

  // find the related pnode which include the states in "rel"
  sit=rel.pre.begin();
  for(; sit!=rel.pre.end(); ++sit) {
    roIt=ro.begin();
    for(; roIt!=ro.end(); ++roIt) {
      if((*roIt)->states.find(*sit)!=(*roIt)->states.end()) {
	todo.push(*roIt);
	break;
      }
    }
  }

  // part1: B himself
  tb=B.states;

  // Part2:collect equivalent states to tb w.r.t. this case
  sit=B.states.begin();
  for(; sit != B.states.end(); ++sit) {
    vit=states[*sit].locuncPres.begin();
    for(; vit != states[*sit].locuncPres.end(); ++vit) 
      tb.insert(*vit);
  }

}

// SOE::refine()
void SOE::refine() 
{
  FD_DF("SOE::refine()");    

  //helpers
  std::set<Pnode*>::iterator roDivIt;

  while(!roDividers.empty()) {
    roDivIt=roDividers.begin();
    partitionClass(**roDivIt);      
  }
}


// SOE::partition(rMapStateToPartition, rGenPart)
void SOE::partition(std::map<Idx,Idx>& rMapStateToPartition, Generator& rGenPart) {

  FD_DF("SOE::partition(rMapStateToPartition," << rGenPart.Name() << ")");
  
  // prepare result  
  rGenPart.Clear();
  rGenPart.Name(gen->Name());
  rMapStateToPartition.clear();

  // loop over all elements of ro and create a new state for every coset in ro (ro contains the set of equivalence classes)
  std::set<Pnode*>::iterator cRoIt    = ro.begin();
  std::set<Pnode*>::iterator cRoItEnd = ro.end();
  std::set<Idx>::iterator cSIt;
  std::set<Idx>::iterator cSItEnd;


  for(; cRoIt != cRoItEnd; ++cRoIt) {
    // have a new state
    std::ostringstream ostr;
 
    Idx newstate = rGenPart.InsState();
 
    // loop over all original states in current coset
    cSIt=(*cRoIt)->states.begin();
    cSItEnd=(*cRoIt)->states.end();        
    for(; cSIt != cSItEnd; ++cSIt)  {
      // retrieve source state idx
      Idx st=states[*cSIt].idx;
      // map every state of the original generator to its equivalence class (= state in rGenPart) 
      // by creating an entry in the map rMapStateToPartition 
      rMapStateToPartition[st] = newstate;       
      // set state names for resulting generator
      if(rGenPart.StateNamesEnabled()) { 
        if(gen->StateName(st)!="") ostr << gen->StateName(st) << ",";
        else ostr << st << ",";
      }
      // set init states
      if(gen->ExistsInitState(st)) 
  	rGenPart.SetInitState(newstate);
      // set marked states
      if(gen->ExistsMarkedState(st)) 
 	rGenPart.SetMarkedState(newstate);
    }
 
    // set state name       
    if(rGenPart.StateNamesEnabled()) {
      std::string statename = ostr.str();
      if(statename.length()>=1) statename.erase(statename.length()-1);
      statename = "{" + statename + "}";
      rGenPart.StateName(newstate, statename); 
    }
  }

  // create transition relation
  // iterate over all transitions of the original generator
  TransSet::Iterator tIt = gen->TransRelBegin();
  TransSet::Iterator tItEnd = gen->TransRelEnd();
  for(; tIt != tItEnd; ++tIt) {
    rGenPart.InsEvent(tIt->Ev);
    rGenPart.SetTransition(rMapStateToPartition[tIt->X1], tIt->Ev,
			   rMapStateToPartition[tIt->X2]);
  }


}

/*
********************************************* 
********************************************* 
PART 3: application interface

Minimal application interface based on plain functions.
********************************************* 
********************************************* 
*/

// ComputeSOE(rGenOrig, rLocAlph, rLocAlph, rMapStateToPartition, rGenPart)
void ComputeSynthObsEquiv(const Generator& rGenOrig, 
	      const EventSet& rConAlph, 
	      const EventSet& rLocAlph, 
	      std::map<Idx,Idx>& rMapStateToPartition, 
	      Generator& rResGen) {

  // test condition: rConAlph,rLocAlph \subseteq rGenOrig.Alphabet()

  // Construct an instance of the SOE class from rGenOrig
  SOE soe = SOE(rGenOrig, rConAlph, rLocAlph);
  // method to compute the SOE on rGenOrig
  soe.refine();
  soe.partition(rMapStateToPartition, rResGen);

}

} //namespace
