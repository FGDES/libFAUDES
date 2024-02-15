/** @file cfl_bisimulation.cpp Bisimulation relations

    Functions to compute bisimulation relations on dynamic systems (represented
    by non-deterministic finite automata). 

    The relevant algorithms are described in J.-C. Fernandez, "An implementation 
    of an efficient algorithm for bisimulation equivalence", Science of Computer 
    Programming, vol. 13, pp. 219-236, 1990. 

    This code was originally part of the observer plug-in (op_bisimulation.* and
    op_partition.*). It moved in revised form to corefaudes as of libFAUDES 2.26.

**/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009, Christian Breindl
   Copyright (C) 2015, Thomas Moor
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

#include <cfl_bisimulation.h>

#include <string>
#include <sstream>
#include <stack>

using namespace std;

//#undef FD_DF
//#define FD_DF(a) FD_WARN(a)

namespace faudes {

/*
********************************************* 
********************************************* 

PART 1: Partition

Objects of class faudes::Pnode represent a set of states with various administration 
features for the interpretation as a node in a binary tree. Each node state set amounts 
to the disjoint union of the two child state sets, with the root of the tree holding the 
full state set of the generator under consideration. The leaves of the tree the form 
the current partition in the bisimulation algorithm.

The code used to reside in the observer plug-in as a public class up to version 2.25.  

********************************************* 
********************************************* 
*/


struct Pnode {

  /** unique partition index */
  Idx index;

  /** Associated set of states */
  // Revision 20150714 tmoor
  // --- replaced faudes sets by plain STL sets for inner-loop variables 
  //     [test case performance 4min-->1min]
  // --- avoid copying STL sets by using pointers instead
  //     [test case performance 1min-->30sec]
  // --- avoid sets altogether, use sorted STL vectors and swap rather than copy
  //     [test case performance 30min-->20sec]
  std::vector<Idx> states;

  /** number of states in this coset */
  Idx nsize;

  /** ref or relatives */
  Pnode* pParent;
  Pnode* pFirstChild;
  Pnode* pSecondChild;

  /** indicates whether the current partition ro is stable with respect to this block */
  bool rostable;

  /** Info-map. First Idx: ev, second Idx: state q, third Idx: number q-ev-successors in this node */
  std::vector<std::map<Idx,Idx> > infoMap;

  /** flag to inicate non-empty infoMap per event (for use with alternative info-map constructs) */
  std::vector<bool> activeEv; // <> infomap nonempty

  /**
   * write info-map to console
   *
   * @param event
   *    event for which the info-map shall be plotted
   */
  void writeInfoMap(Idx event) const {
    cout << "Writing info-map for event " << event << endl;
    const std::map<Idx,Idx> & pMap = infoMap[event];
    std::map<Idx,Idx>::const_iterator mIt;
    std::map<Idx,Idx>::const_iterator mItBegin = pMap.begin();
    std::map<Idx,Idx>::const_iterator mItEnd = pMap.end();
    if(mItBegin == mItEnd)
       cout << "no entries for this event" << endl; 
    for(mIt=mItBegin; mIt != mItEnd; ++mIt)
       cout << "state: " << (*mIt).first << " : occurrences: " << (*mIt).second << endl;
    cout << endl;
  };
};


/*
********************************************* 
PART 2: Bisimulation

The class faudes::Bisimulation is used to compute the coarsest bisimulation relation
and to maintain relevant data structures required for the particular implementation.
The code used to reside in the observer plug-in as a public class up to libFAUDES 2.25. 

********************************************* 
*/


class Bisimulation {

public:

  /**
   * Contructor: 
   * keep a reference to the generator and initialize the partition and the W-Tree
   * to represent the universal equivalence relation, i.e. every two states are equivalent.
   *
   * @param g
   *   Original generator
   */
  Bisimulation(const Generator& g);

  /**
   * Destructor
   */
  ~Bisimulation(void);

  /**
   * Write W-tree to console
   */
  void writeW(void);

  /**
   * Write the set of equivalence classes to console
   */
  void writeRo(void);

  /**
   * Perform fixpoint iteration to obtain the coarset bisimulation relation
   */
  void refine(void);

  /**
   * Extract output generator that represents the resulting quotient automaton.
   * (need to invoke refine() befor)
   *
   * @param rMapStateToPartition
   *   Maps each state to its equivalence class
   * @param rGenPart
   *   Generator representing the result of the computation. Each state corresponds to an
   *   euivalence class
   */
  void partition(std::map<Idx,Idx>& rMapStateToPartition, Generator& rGenPart);


  /**
   * Extract the coarsest quasi-congruence as an STL map
   * (need to invoke refine() befor)
   *
   * @param rMapStateToPartition
   *   Maps each state to its equivalence class
   */
  void partition(std::map<Idx,Idx>& rMapStateToPartition);

  /**
   * Extract the coarsest quasi-congruence as a list of equivalence classes.
   * (need to invoke refine() befor)
   *
   * @param rPartition
   *   List of equivcalent sets ogf states
   */
  void partition(std::list< StateSet >& rPartition); 


private:

  /**
   * internal representation of transition relation with consecutive indexed states and events
   // Revision 20150724 tmoor
   // --- technically, re-indexing is equivalent to "pointers on source indicees" and
   //     buffers log-n searches [30sec -> 10sec for our testcases]
   */
  struct State {
    Idx idx;  // source state idx
    vector< vector<Idx> > suc; // successors by event
    vector< vector<Idx> > pre; // predecessors by event
    size_t iscnt;              // alternative infomap for current BSmaller [not used]
    size_t ilcnt;              // alternative infomap for current BLarger [not used]
  };
  vector<State> states;        // vector of all states  [starting with 1 -- use min-index for debugging]
  vector<Idx> events;          // vector of all events [starting with 1]


  /**
   *  Keep reference to Automaton
   */
  const Generator* gen;

  /**
   * Counter to assign a unique index to each node [debugging/cosmetic only]
   */
  Idx nxidx;

  /**
   * W-tree. Contains all blocks ever created  [required for destruction]
   */
  std::vector<Pnode*> W;

  /**
   * insert new node to W-tree (empty stateset)
   * note: this method only cares about indexing, and initialisation of node members
   * note: cross references are left to the caller
   */
  Pnode* newnode();

  /**
   * set of nodes that form current partition ro (i.e. leaves of W)
   */
  // Revision 20150714 tmoor
  // --- this used to be a vector with suboptimal performance for search/delete
  // --- todo: this could be represented by pointers per node
  std::set<Pnode*> ro;

  /** 
   * set of nodes that can possibly split classes in ro
   */
  std::set<Pnode*> roDividers;


  /**
   * compute info-maps for two cosets pSmallerPart and pLargerpart. 
   * - the current partition must be stable wrt the common parent the both parts
   * - the smaller part and larger part info maps must be initialized empty and by parent, resp.
   *
   * @param node
   *   Node to provide states of pSmallerPart (call with pSmallerPart)
   * @param pSmallerPart
   *   Child coset P1 of P with smaller number of states
   * @param pLargerPart
   *   Child coset P2 of P with larger number of states
   * @param ev
   *   Event
   */
  void computeInfoMaps(Pnode& node, Pnode* pSmallerPart, Pnode* pLargerPart, Idx ev);

  /**
   * Compute info-map for coset B
   *
   * @param B
   *   Coset for which the info-map is computed
   * @param Bstates
   *   Node to provide states of B
   * @param ev
   *   Event to use
   * @param tb
   *   sorted vector containing all the states that have an ev-transition into coset B
   */
  void computeInfoMap(Pnode& B, Pnode& Bstates, Idx ev, vector<Idx>& tb);


  /**
   * Alternative: compute and set info-map to state data
   * --- this is for testing/debugging only, it did neither buy nor cost relevant performance
   */
  void setInfoMap(Pnode& BSmaller, Pnode& BLarger, Idx ev);
  void invImage(Pnode& B, Pnode& Bstates, Idx ev, vector<Idx>& tb);

  /**
   * Test wehther a state has an ev-transitions into the specified coset 
   *
   * @param state
   *   State to be examined
   * @param node
   *   Coset
   * @param ev
   *   Event
   */
  bool stateLeadsToPartition(Idx state, Pnode& node, Idx ev);


  /**
   * Refine current partition with respect to coset B
   *
   * @param B
   *   Coset
   */
  void partitionClass(Pnode& B);

  /**
   * Refine current partition with respect to partition B and make use of the fact that
   * the current partition is stable with respect to the parent coset of B (compound splitter).
   *
   * @param B
   *   Coset
   */
  void partitionSplitter(Pnode& B);


  /**
   * Function for recursively plotting the W-tree to console [debugging]
   *
   * @param node
   *   Coset
   */
  void writeNode(Pnode& node);

};



// Constructor Bisimulation(g)
Bisimulation::Bisimulation(const Generator& g)
{
  FD_DF("Bisimulation::Bisimulation(" << g.Name() << ")");
  gen = &g;
  nxidx=1;

  // encode transition relation [effectively buffer log-n search]
  map<Idx,Idx> smap;
  map<Idx,Idx> emap;
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
    states[max].suc.resize(events.size());
    states[max].pre.resize(events.size());
  }
  TransSet::Iterator tit=gen->TransRelBegin();
  for(; tit != gen->TransRelEnd(); ++tit) {
    states[smap[tit->X1]].suc[emap[tit->Ev]].push_back(smap[tit->X2]);
    states[smap[tit->X2]].pre[emap[tit->Ev]].push_back(smap[tit->X1]);
  }

  // create universal partition holding the complete state set
  Pnode* universe = newnode(); 
  universe->pParent=NULL;

  Idx st=1;
  for(;st<states.size();++st) universe->states.push_back(st); 
  universe->nsize = universe->states.size();

  // add universal Partition to ro and roDividers (all equivalence classes are potential dividers)
  ro.insert(universe);
  roDividers.insert(universe);

  // this partition is too coarse ... it needs refinement
  FD_DF("Bisimulation::Bisimulation: leaving function");
}

// destruct
Bisimulation::~Bisimulation(void)
{
  vector<Pnode*>::iterator wit=W.begin();
  vector<Pnode*>::iterator wit_end=W.end();
  for(;wit!=wit_end;++wit) delete *wit;

}

// maintain W-tree: new node
Pnode* Bisimulation::newnode(void) {
  Pnode* nn= new Pnode();
  nn->rostable=false;
  nn->nsize=0;
  nn->pFirstChild=NULL;
  nn->pSecondChild=NULL;
  nn->index=nxidx;
  nn->infoMap.resize(events.size());
  nn->activeEv.resize(events.size());
  nxidx++;
  W.push_back(nn);
  return nn;
}


// Bisimulation::partitionSplitter(B)
// Revision tmoor 20150714 
// -- gprof indicated that most time is spend in this function; not quite sure whether this is acurate
// -- circumvented excessive set-look up "W[index]" 
// -- using stl set for current partition "ro" to improve performance when deleting a cell
void Bisimulation::partitionSplitter(Pnode& B) {

  FD_DF("Bisimulation::partitionSplitter(B)");
  FD_DF("Bisimulation::partitionSplitter: index of current coset is " << B.index);

  // it is ensured by the calling function that the passed coset B has a parent coset B' and
  // that ro is stable with respect to B'. Therefore, the info-maps for the parent coset B' already exist.

  // pointer to parent coset
  Pnode* BParent = B.pParent;

  // Choose the coset with fewer states among B and its brother coset and let pSmallerPart point to it
  Pnode* BSmallerPart=BParent->pFirstChild;
  Pnode* BLargerPart=BParent->pSecondChild;
  if(BSmallerPart->nsize > BLargerPart->nsize) { 
    BSmallerPart=BParent->pSecondChild;
    BLargerPart=BParent->pFirstChild;
  }

  FD_DF("Bisimulation::partitionSplitter: the smaller coset (chosen) has index: " << BSmallerPart->index);
  FD_DF("Bisimulation::partitionSplitter: the larger coset has index: " << BLargerPart->index);

  // Iterators for loop over states
  vector<Idx>::iterator sit;
  vector<Idx>::iterator sit_end;

  // stack of cells to process
  stack<Pnode*> toDo;

  //loop over all events
  size_t ev=1;
  for(; ev < events.size(); ++ev){

    // the current event only has to be processed if there are transitions with ev to states in the 
    // parent coset, i.e., there is an entry for ev in the infoMap of BParent
    if(!BParent->activeEv[ev]) continue;
    FD_DF("Bisimulation::partitionSplitter: partitioning for event " << gen->EventName(events[ev]));

    // initialize info-map for current event of larger child coset with 
    // a copy of the info-map of the current event of the parent coset 
    BLargerPart->infoMap[ev]= BParent->infoMap[ev];
    // initialize info-map for current event of smaller child coset with an empty info-map
    BSmallerPart->infoMap[ev].clear();
    // compute the info maps for current event for smaller child cosets
    computeInfoMaps(*(BSmallerPart), BSmallerPart, BLargerPart, ev);

  

    //debugging / performance: alternative  --- almost no difference in performance
    /*
    set<Idx> tb;
    computeInfoMap(*BSmallerPart,*BSmallerPart,ev,tb);
    computeInfoMap(*BLargerPart,*BLargerPart,ev,tb);
    */

  
    // alternative current info map
    // setInfoMap(*BSmallerPart, *BLargerPart, ev);

    // figure cosets in ro that are completely contained in T_{ev}^{-1}{B}
    // note: since ro is stable w.r.t BParent, either all or no states of a ro-class have a successor in BParent
    FD_DF("Bisimulation::partitionSplitter: computing predecessor cosets of parent coset with index " << BParent->index);
    set<Pnode*>::iterator rit;
    for(rit=ro.begin(); rit!=ro.end(); ++rit) {
      // skip singletons
      if((*rit)->nsize==1) continue;
      // choose an arbitrary state from the current coset
      Idx testState = *((*rit)->states.begin());
      FD_DF("Bisimulation::partitionSplitter: candidate coset index " << (*rit)->index << " and test state " << gen->SStr(testState));
      if(stateLeadsToPartition(testState, *BParent, ev)) {
	toDo.push(*rit);
	FD_DF("Bisimulation::partitionSplitter: coset index " << (*rit)->index << " is predecessor for parent coset with index " << (*BParent).index << " and event = " << gen->EStr(events[ev]) << " and was pushed on toDo-stack");
      }
    }

    // iterator to info map for parent and smaller coset with event (and opt x1) resolved
    map<Idx, Idx> &   imapParentEv = BParent->infoMap[ev];
    map<Idx, Idx> &   imapSmallerEv = BSmallerPart->infoMap[ev];
    map<Idx, Idx>::const_iterator imapParentEvX1;
    map<Idx, Idx>::const_iterator imapSmallerEvX1;
		
  
    // split all cosets on the toDo-stack using the info-maps. This process is described in Equation (2) of 
    // J.-C. Fernandez, “An implementation of an efficient algorithm for bisimulation equivalence,” 
    // Science of Computer Programming, vol. 13, pp. 219-236, 1990.
		
    // iteration over all cosets on todo-stack
    while(!toDo.empty()){
      // take top of the stack
      Pnode* rop=toDo.top();
      toDo.pop();
      FD_DF("Bisimulation::partitionSplitter: current coset taken from todo-stack has index " << rop->index);
			
      // prepare three empty state state sets for later insertion to W-Tree 
      vector<Idx> sX1;
      vector<Idx> sX2;
      vector<Idx> sX3;

      // iteration over all states of current candidate coset
      // apply rules for splitting cosets into subsets using info-maps 
      sit_end = rop->states.end();
      for(sit=rop->states.begin(); sit!=sit_end; ++sit) {
        if(imapSmallerEv.empty()){
	  sX2.push_back(*sit); // all sucessors in larger part
	} else {
	  imapSmallerEvX1=imapSmallerEv.find(*sit);
          if(imapSmallerEvX1==imapSmallerEv.end()) {
	    sX2.push_back(*sit); // all sucessors in larger part
	  } else {  
            imapParentEvX1=imapParentEv.find(*sit);
	    if(imapParentEvX1->second==imapSmallerEvX1->second)                
	      sX1.push_back(*sit);  // all sucessors in smaller part              
	    else { 
              sX3.push_back(*sit); // sucessors in both parts
	    }
	  }
	}
      }

      // use alternative info-map
      /*
      sit_end = rop->states.end();
      for(sit=rop->states.begin(); sit!=sit_end; ++sit) {
        State& st=states[*sit];
        if(st.iscnt==0)  { sX2->insert(*sit);  continue;}
        if(st.ilcnt==0)  { sX1->insert(*sit);  continue;}
	sX3->insert(*sIt);
      }
      */

      // sort states
      sort(sX1.begin(), sX1.end());
      sort(sX2.begin(), sX2.end());
      sort(sX3.begin(), sX3.end());
  
      // insert non empty sets to the W-Tree; W-Tree takes ownership
      Pnode* nX1 = NULL;
      Pnode* nX2 = NULL;
      Pnode* nX3 = NULL;
      if(!sX1.empty()) { nX1=newnode(); nX1->nsize=sX1.size(); nX1->states.swap(sX1); };
      if(!sX2.empty()) { nX2=newnode(); nX2->nsize=sX2.size(); nX2->states.swap(sX2); };
      if(!sX3.empty()) { nX3=newnode(); nX3->nsize=sX3.size(); nX3->states.swap(sX3); };

      // if not more than one set is empty, no split happens
      if((nX1!=NULL ? 1 : 0) + (nX2!=NULL ? 1 : 0) + (nX3!=NULL ? 1 : 0) <2) continue;

      // any non-empty set becomes a ro-class and a roDivider
      if(nX1!=NULL) {ro.insert(nX1); roDividers.insert(nX1);};
      if(nX2!=NULL) {ro.insert(nX2); roDividers.insert(nX2);};
      if(nX3!=NULL) {ro.insert(nX3); roDividers.insert(nX3);};

      // current split ro-class does not belong to ro anymore
      ro.erase(rop);
      roDividers.erase(rop);

      // states of split ro-class coset are no longer needed as they will be represented by childs
      // rop->states=vector<Idx>()

      // info-map of the split coset's parent coset can be deleted if it is no longer needed 
      if(rop->pParent!=NULL && rop->pParent != BParent) 
	if(rop->pParent->pFirstChild->pFirstChild!=NULL && rop->pParent->pSecondChild->pSecondChild!=NULL) {
	  rop->pParent->infoMap.clear();
	  rop->pParent->states=vector<Idx>();
      }	

      // update W-tree structure (set cross references)
      switch( (nX1!=NULL ? 1 : 0) + (nX2!=NULL ? 2 : 0) + (nX3!=NULL ? 4 : 0) )  {
	case 0: // not possible
	case 1: // not possible
	case 2: // not possible
	case 4: // not possible
	    break;
      
        case 3: //X1 and X2 are not empty
	   FD_DF("Bisimulation::partitionSplitter: coset " << rop->index << " has been split into cosets X1 and X2");
	   rop->pFirstChild=nX1;
	   rop->pSecondChild=nX2;
	   nX2->pParent=rop;
	   nX1->pParent=rop;		
	   break;
	     
	 case 5: //X1 and X3 are not empty
	   FD_DF("Bisimulation::partitionSplitter: coset " << rop->index << " has been split into cosets X1 and X3");
	   rop->pFirstChild=nX1;
	   rop->pSecondChild=nX3;
	   nX3->pParent=rop;
	   nX1->pParent=rop;
	   break;
				
      	 case 6: //X2 and X3 are not empty
  	   FD_DF("Bisimulation::partitionSplitter: coset " << rop->index << " has been split into cosets X2 and X3");
	   rop->pFirstChild=nX2;
	   rop->pSecondChild=nX3;
	   nX3->pParent=rop;
	   nX2->pParent=rop;
	   break;
				
    	 case 7: //X1 and X2 and X3 are not empty		
	   FD_DF("Bisimulation::partitionSplitter: coset " << rop->index << " has been split into cosets X1, X2 and X3");
	   // have one extra node parent of 2 and 3
	   Pnode* nX23=newnode();
           std::set_union(nX2->states.begin(), nX2->states.end(), nX3->states.begin(), nX3->states.end(),       
	     std::inserter(nX23->states, nX23->states.begin()));
	   // set references
	   rop->pFirstChild=nX1;
	   rop->pSecondChild=nX23;
	   nX1->pParent=rop;
	   nX23->pParent=rop;
	   nX23->pFirstChild=nX2;
	   nX23->pSecondChild=nX3;
	   nX2->pParent=nX23;
	   nX3->pParent=nX23;
	   nX23->nsize=nX2->nsize + nX3->nsize;
	   break;

      } //end switch-case
	
    } // end iteration over todo-stack
  }	

  // infoMap of parent coset is now lo longer needed and can be deleted (the information has been transferred to the children cosets)
  BParent->infoMap.clear();
  BParent->states=vector<Idx>();

  // ro is now stable with respect to the larger and smaller child cosets
  BSmallerPart->rostable=true;
  BLargerPart->rostable=true;
    
  // delete the stateSets of the smaller and larger child cosets since they are no ro-deviders anymore
  if(BSmallerPart->pFirstChild != NULL) BSmallerPart->states=vector<Idx>();
  if(BLargerPart->pFirstChild != NULL) BLargerPart->states=vector<Idx>();
    
  // if smaller and larger partitions were in roDividers, then they now have to be deleted from this vector
  roDividers.erase(BSmallerPart);
  roDividers.erase(BLargerPart);

  FD_DF("Bisimulation::partitionSplitter: leaving function");
}

// Bisimulation::computeInfoMaps(node, pSmallerPart, pLargerPart, ev)
// compute info-maps for smaller and larger part with target states provided by node, first call with node=smaller
// assume that smaller is inititialized with zero and larger is initialized with parent map
void Bisimulation::computeInfoMaps(Pnode& node, Pnode* pSmallerPart, Pnode* pLargerPart, Idx ev) {

  FD_DF("Bisimulation::computeInfoMaps(" << node.index << "," << pSmallerPart->index << "," << pLargerPart->index << "," << gen->EventName(ev) << ")");

  // if the target node does not hold states, ask children
  if(node.states.empty()){
    if(node.pFirstChild!=NULL)
      computeInfoMaps(*(node.pFirstChild), pSmallerPart, pLargerPart, ev);
    if(node.pSecondChild!=NULL)
      computeInfoMaps(*(node.pSecondChild),pSmallerPart, pLargerPart, ev);
    return;
  }

  // iterators over transitions 
  vector<Idx>::iterator tit;
  vector<Idx>::iterator tit_end;

  // info maps with resolved ev (and opt x1)
  map<Idx,Idx>& imapLEv = pLargerPart->infoMap[ev];
  map<Idx,Idx>& imapSEv = pSmallerPart->infoMap[ev];
  map<Idx,Idx>::iterator imapLEvX1;
  map<Idx,Idx>::iterator imapSEvX1;

  // loop over states of target node
  vector<Idx>::iterator xit2=node.states.begin();
  vector<Idx>::iterator xit2_end=node.states.end();
  for(; xit2!=xit2_end; ++xit2) {
    // loop over transitions of target state to find predecessors
    // Note that all states investigated are states of the smaller coset.
    tit = states[*xit2].pre[ev].begin();
    tit_end = states[*xit2].pre[ev].end();
    for(; tit!=tit_end; ++tit) {
      // record predecessor 
      Idx x1=*tit;
      // increment number of occurrences for found predecessor in smaller part map
      imapSEvX1=imapSEv.find(x1);
      if(imapSEvX1!=imapSEv.end())
        ++imapSEvX1->second;
      else
        imapSEv[x1]=1;
      //decrement number of occurrences for found predecessor in larger part map
      imapLEvX1=imapLEv.find(x1);
      --imapLEvX1->second;
      if(imapLEvX1->second==0) imapLEv.erase(imapLEvX1);
    }
  }

  // counts: larger = parent -smaller (should be outside the recursion)
  /*
  map<Idx,Idx> & imapFEv=pLargerPart->pParent->infoMap[ev];
  map<Idx,Idx>::iterator mit=imapFEv.begin();
  for(;mit!=imapFEv.end();++mit){
    Idx cnt= mit->second;
    imapSEvX1=imapLEv.find(mit->first);    
    if(imapSEvX1!=imapLEv.end()) cnt=cnt-imapSEvX1->second;
    if(cnt>0) imapLEv[mit->first]=cnt;
  }
  */

  // set active flag
  pSmallerPart->activeEv[ev]= !pSmallerPart->infoMap[ev].empty();
  pLargerPart->activeEv[ev]=  !pLargerPart->infoMap[ev].empty();
        
  FD_DF("Bisimulation::computeInfoMaps: leaving function");
}


// Bisimulation::stateLeadsToPartition(state,  node, ev)
// Revision tmoor 20150715
// --- this was in error and of low performance
bool Bisimulation::stateLeadsToPartition(Idx state, Pnode& node, Idx ev) {
  FD_DF("Bisimulation::stateLeadsToPartition(" << state << "," << node.index << "," << gen->EventName(ev) << ")");
 
  // if node does not hold states, ask children
  if(node.states.empty()) {
    if(node.pFirstChild!=NULL)
      if(stateLeadsToPartition(state,*node.pFirstChild,ev)) return true;
    if(node.pSecondChild!=NULL)
      if(stateLeadsToPartition(state,*node.pSecondChild,ev)) return true;
    return false;
  }

  // look for transition to target node
  vector<Idx>::iterator tit = states[state].suc[ev].begin();
  vector<Idx>::iterator tit_end = states[state].suc[ev].end();
  for(;tit!=tit_end; ++tit) {
    if(binary_search(node.states.begin(),node.states.end(),*tit))  return true;
    // break; --- todo: do we need to iterate here? original code did not ...
  }
  return false;
}


/*
// Bisimulation::stateLeadsToPartition(state,  node, ev)
// libFAUDES Version 2.25 for reference
bool Bisimulation::stateLeadsToPartition(Idx state, Pnode& node, Idx ev)
{
  FD_DF("Bisimulation::stateLeadsToPartition(" << state << "," << node.index << "," << gen->EventName(ev) << ")");

  bool found=false;
  // there is a direct transition from state to a state in the coset node
  if(  node.states->find(gen->TransRelBegin(state, ev)->X2)!= node.states->end() && 
       gen->TransRelBegin(state, ev)->X1 == state && 
       gen->TransRelBegin(state, ev)->Ev == ev)
    {
      found = true;
    }
  // the stateset of node is stored in its children. Then, try to find a transition to any of the children cosets of node
  if(found == false && node.pFirstChild!=NULL)
    {
      found=stateLeadsToPartition(state,*node.pFirstChild,ev);
    }
	
  if(found == false && node.pSecondChild!=NULL)
    {
      found=stateLeadsToPartition(state,*node.pSecondChild,ev); 
    }
	
  FD_DF("Bisimulation::stateLeadsToPartition: leaving function");
  return found;
}
*/



// Bisimulation::partitionClass(B)
// Revision tmoor 20150714 
// -- gprof indicated that most time is spend in this function; not quite sure whether this is acurate
// -- circumvented excessive set-look up "W[index]" (this should become a vector anyway)
// -- using stl set for current partition ro to improve performance when deleting a cell
void Bisimulation::partitionClass(Pnode& B) {

  FD_DF("Bisimulation::partitionClass(" << B.index << ")");
  FD_DF("Bisimulation::partitionClass: index of passed coset is " << B.index);

  // helpers
  vector<Pnode*> addSet;
  vector<Pnode*>::iterator addIt;
  vector<Pnode*> removeSet;
  vector<Pnode*>::iterator remIt;

  FD_DV("Bisimulation::partitionClass: loop over events");

  // Loop over all events
  size_t ev=1;
  for(; ev<events.size(); ++ev) {

    FD_DF("Bisimulation::partitionClass: partitioning for event " << gen->EventName(events[ev]));
    // compute info map for current event ev and coset B and store predecessor states in T_{ev}^{-1}(B) in tb
    vector<Idx> tb;
    computeInfoMap(B, B, ev, tb);
    //invImage(B, B, ev, tb);

    // if there are no predecessors for this event there will be no split
    // --- tmoor 20150723: this clause bought us 13sec --> 6sec for testcases
    if(!B.activeEv[ev]) continue;

    // compute the cosets in ro that are split into the subsets intersec and diff by evaluating the backward reachability from states in B
    set<Pnode*>::iterator rit;
    for(rit = ro.begin(); rit != ro.end(); ++rit) {
      FD_DF("Bisimulation::partitionClass: candidate coset to be split has index " << (*rit)->index);

      // skip singletons
      if((*rit)->nsize==1) continue;

      // compute intersection of tb with current candidate coset
      vector<Idx> sXinter;
      std::set_intersection((*rit)->states.begin(), (*rit)->states.end(), tb.begin(), tb.end(),       
	   std::inserter(sXinter, sXinter.begin()));

      // skip on trivial intersection
      if(sXinter.empty() || sXinter.size()==(*rit)->states.size()) continue;
      FD_DF("Bisimulation::partitionClass: current coset with index " << (*rit)->index << " will be split");
                
      // delete from ro and roDividers
      roDividers.erase(*rit);
      removeSet.push_back(*rit);

      // set up new node for intersection 
      Pnode* nXinter= newnode();
      nXinter->nsize=sXinter.size();
      nXinter->states.swap(sXinter);
      (*rit)->pFirstChild=nXinter;
      nXinter->pParent=*rit;

      // set up new node for difference
      Pnode* nXdiff = newnode();
      std::set_difference((*rit)->states.begin(), (*rit)->states.end(), nXinter->states.begin(), nXinter->states.end(), 
	   std::inserter(nXdiff->states,nXdiff->states.begin()));
      (*rit)->pSecondChild=nXdiff;
      nXdiff->pParent=*rit;
      nXdiff->nsize=nXdiff->states.size();
 
      // record that the new cosets have to be added to ro and roDividers
      addSet.push_back(nXinter);
      addSet.push_back(nXdiff);
      roDividers.insert(nXinter);                
      roDividers.insert(nXdiff);
   
      // report
      FD_DF("Bisimulation::partitionClass: the coset with index " << nXinter->index << " has been added to addSet and to roDividers");
      FD_DF("Bisimulation::partitionClass: the coset with index " << nXdiff->index << " has been added to addSet and to roDividers");    
      FD_DF("Bisimulation::partitionClass: the candidate coset has been split");

      // delete info-map of parent coset of split coset if no longer needed
      if((*rit)->pParent!=NULL) {
         FD_DF("Bisimulation::partitionClass: split coset has parent coset with index " << (*rit)->pParent->index);
	 if((*rit)->pParent->pFirstChild->pFirstChild!=NULL && (*rit)->pParent->pSecondChild->pSecondChild!=NULL && (*rit)->pParent->rostable) {
	   (*rit)->pParent->infoMap.clear();
	   (*rit)->pParent->states=vector<Idx>();
           FD_DF("Bisimulation::partitionClass: info map of parent coset deleted");
        }
      }
                
      // delete stateSet of split coset as the states are now stored in the child cosets
      //(*rit)->states=vector<Idx>();
      FD_DF("Bisimulation::partitionClass: states of split coset " << (*rit)->index << " have been deleted");
    
    } // loop all ro classes

    // delete split partitions from ro
    FD_DF("Bisimulation::partitionClass: deleting split cosets from ro");
    for(remIt=removeSet.begin(); remIt!=removeSet.end();++remIt) 
      ro.erase(*remIt);
    removeSet.clear();

    // insert the new cosets into ro
    FD_DF("Bisimulation::partitionClass: inserting #" << addSet.size() <<" new cosets into ro");
    for(addIt=addSet.begin();addIt!=addSet.end();++addIt) 
      ro.insert(*addIt);
    addSet.clear();

  } //end loop over events


  // ro is now stable with respect to coset B
  B.rostable = true;
    
  // delete coset B from roDividers
  roDividers.erase(&B);

  // delete stateSet of coset B if no longer needed
  if(B.pFirstChild != NULL) B.states=vector<Idx>();
            
  FD_DF("Bisimulation::partitionClass(): done");
}

// Bisimulation::computeInfoMap(B, Bstates, ev, tb)
// compute info map for B using target states provided by Bstates, i.e., first call with Bstates=B
void Bisimulation::computeInfoMap(Pnode& B, Pnode& Bstates, Idx ev, vector<Idx>& tb) {

  FD_DF("Bisimulation::computeInfoMap(" << B.index << "," << Bstates.index << "," << gen->EventName(ev) << ", Stateset&)");
  FD_DF("Bisimulation::computeInfoMap: consider stateSet of coset " << Bstates.index);

  // if Bstates does not host states, ask children
  if(Bstates.states.empty()) {
    if(Bstates.pFirstChild!=NULL)
        computeInfoMap(B, *(Bstates.pFirstChild), ev, tb);
    if(Bstates.pSecondChild!=NULL)
        computeInfoMap(B, *(Bstates.pSecondChild), ev, tb);
    return;
  }

  // iterators over predecessors
  vector< Idx >::iterator tit;
  vector< Idx >::iterator tit_end;

  // info map with ev (and opt x1) resolved
  map<Idx, Idx>& imapBEv = B.infoMap[ev];
  map<Idx, Idx>::iterator imapBEvX1;

  // loop over all states of current partition B
  vector<Idx>::iterator xit2 = Bstates.states.begin();
  vector<Idx>::iterator xit2_end = Bstates.states.end();
  for(; xit2 !=  xit2_end; ++ xit2){
    // loop over all transitions to current target state 
    tit=states[*xit2].pre[ev].begin();
    tit_end=states[*xit2].pre[ev].end();
    for(; tit < tit_end; ++tit) {
       // record precessor 		
       Idx x1=*tit;
       tb.push_back(x1);	
       // increase number of occurrences for found predecessor state in info-map
       imapBEvX1=imapBEv.find(x1);
       if(imapBEvX1!=imapBEv.end()) ++imapBEvX1->second;
       else imapBEv[x1]=1;
    }
  }
  // sort states
  sort(tb.begin(),tb.end());

  B.activeEv[ev]= !tb.empty();

  FD_DF("Bisimulation::computeInfoMap: leaving function");
}


// Bisimulation::setInfoMap(...)
// set up info map for smaller and larger part
// -- Revision 20150725: use state data structure to store current info map 
// -- this did not help performance
void Bisimulation::setInfoMap(Pnode& BSmaller, Pnode& BLarger, Idx ev) {

  FD_DF("Bisimulation::setInfoMap(" << B.index << "," << Bstates.index << "," << gen->EventName(ev) << ", Stateset&)");
  FD_DF("Bisimulation::setInfoMap: consider stateSet of coset " << Bstates.index);

  // clear info
  vector<State>::iterator sit=states.begin();
  if(sit!=states.end()) ++sit;
  for(;sit!=states.end();++sit) { 
    sit->iscnt=0;
    sit->ilcnt=0;
  }

  // iterators over predecessors
  vector< Idx >::iterator tit;
  vector< Idx >::iterator tit_end;
  vector<Idx>::iterator xit2;
  bool found;

  // loop over all states of current partition BSmaller
  found=false;
  xit2 = BSmaller.states.begin();
  for(; xit2 !=  BSmaller.states.end(); ++xit2) {
    // loop over all transitions to current target state 
    tit=states[*xit2].pre[ev].begin();
    tit_end=states[*xit2].pre[ev].end();
    for(; tit < tit_end; ++tit) {
       // record predecessor 		
       Idx x1=*tit;
       ++(states[x1].iscnt);
       found=true;
    }
  }
  if(found) BSmaller.activeEv[ev]=true;
  

  // loop over all states of current partition BLarger
  found=false;
  xit2 = BLarger.states.begin();
  for(; xit2 !=  BLarger.states.end(); ++xit2) {
    // loop over all transitions to current target state 
    tit=states[*xit2].pre[ev].begin();
    tit_end=states[*xit2].pre[ev].end();
    for(; tit < tit_end; ++tit) {
       // record predecessor 		
       Idx x1=*tit;
       ++(states[x1].ilcnt);
       found=true;
    }
  }
  if(found) BLarger.activeEv[ev]=true;

  FD_DF("Bisimulation::setInfoMap: leaving function");
}


// inverse image
void Bisimulation::invImage(Pnode& B, Pnode& Bstates, Idx ev, vector<Idx>& tb) {

  // if Bstates does not host states, ask children
  if(Bstates.states.empty()) {
    if(Bstates.pFirstChild!=NULL)
        invImage(B, *(Bstates.pFirstChild), ev, tb);
    if(Bstates.pSecondChild!=NULL)
        invImage(B, *(Bstates.pSecondChild), ev, tb);
    return;
  }

  // iterators over predecessors
  vector< Idx >::iterator tit;
  vector< Idx >::iterator tit_end;

  // loop over all states of current partition B
  vector<Idx>::iterator xit2 = Bstates.states.begin();
  for(; xit2 !=  Bstates.states.end(); ++xit2) {
    // loop over all transitions to current target state 
    tit=states[*xit2].pre[ev].begin();
    tit_end=states[*xit2].pre[ev].end();
    for(; tit < tit_end; ++tit) {
       // record precessor 		
       Idx x1=*tit;
       tb.push_back(x1);	
    }
  }
  // sort states
  sort(tb.begin(),tb.end());

  B.activeEv[ev]= !tb.empty();
}


// Bisimulation::refine()
void Bisimulation::refine() {
  FD_DF("Bisimulation::refine()");    
  //helpers
  set<Pnode*>::iterator roDivIt;
  Pnode* pParent;
  // start with first entry of roDividers and repeat the refinement steps performed by the functions partitionClass 
  // and partitionSplitter as long as there are any roDividers
  while(!roDividers.empty()) {
    // be interuptable
    FD_WPC(ro.size()-roDividers.size(), ro.size(), "Bisimulation: blocks/dividers:   " << ro.size() << " / " << roDividers.size());
    // find a coset that has a parent coset B' such that ro is stable w.r.t. B'
    // (the order in which the dividers are chosen does not matter)
    roDivIt=roDividers.begin();    
    while(roDivIt != roDividers.end()) {
      pParent=(*roDivIt)->pParent;
      if(pParent == NULL) { roDivIt++; continue; };
      if(pParent->rostable != true) { roDivIt++; continue; };
      break;
    }
    // if a splitter was found, do the split
    if(roDivIt != roDividers.end()) {
      partitionSplitter(**roDivIt);          
      continue;
    }
    // else split by class
    roDivIt=roDividers.begin();
    partitionClass(**roDivIt);   
  }
}



// Bisimulation::partition(rMapStateToPartition, rGenPart)
void Bisimulation::partition(std::map<Idx,Idx>& rMapStateToPartition, Generator& rGenPart) {

  FD_DF("Bisimulation::partition(rMapStateToPartition," << rGenPart.Name() << ")");
  
  // prepare result  
  rGenPart.Clear();
  rMapStateToPartition.clear();

  // loop over all elements of ro and create a new state for every coset in ro (ro contains the set of equivalence classes)
  set<Pnode*>::const_iterator cRoIt    = ro.begin();
  set<Pnode*>::const_iterator cRoItEnd = ro.end();
  vector<Idx>::iterator cSIt;
  vector<Idx>::iterator cSItEnd;
  for(; cRoIt != cRoItEnd; ++cRoIt) {
    // have a new state
    std::ostringstream ostr;
    Idx newstate = rGenPart.InsState();        
    FD_DF("Bisimulation::partition: new state " << newstate << " for coset " 
	  << (*cRoIt)->index );

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
      FD_DF("Bisimulation::partition: new state " << statename);
    }

  }

  // create transition relation
  // iterate over all transitions of the original generator
  TransSet::Iterator tIt = gen->TransRelBegin();
  TransSet::Iterator tItEnd = gen->TransRelEnd();
  for(; tIt != tItEnd; ++tIt) {
    rGenPart.InsEvent(tIt->Ev); // ??
    rGenPart.SetTransition(rMapStateToPartition[tIt->X1], tIt->Ev, rMapStateToPartition[tIt->X2]);
    FD_DF("Bisimulation::partition: adding transition: X1=" << rGenPart.TStr(*tIt) );
  }
  FD_DF("Bisimulation::partition: leaving function");
}



// Bisimulation::partition(rMapStateToPartition)
void Bisimulation::partition(std::map<Idx,Idx>& rMapStateToPartition) {
  FD_DF("Bisimulation::partition(rMapStateToPartition)");


  // prepare result 
  rMapStateToPartition.clear();

  // loop over all elements of ro
  set<Pnode*>::const_iterator cRoIt = ro.begin();
  set<Pnode*>::const_iterator cRoItEnd = ro.end();
  vector<Idx>::iterator cSIt;
  vector<Idx>::iterator cSItEnd;
  for (; cRoIt != cRoItEnd; ++cRoIt) {
    cSIt=(*cRoIt)->states.begin();
    cSItEnd=(*cRoIt)->states.end();
    for(; cSIt!=cSItEnd; ++ cSIt) {
      // map every state in the current coset to the index of this coset by creating an entry in the map rMapStateToPartition
      Idx st=states[*cSIt].idx;
      rMapStateToPartition[st] = (*cRoIt)->index;   // fix: tmoor 20150712
    }
  }
  FD_DF("Bisimulation::partition: leaving function");
}


// Bisimulation::partition(rPartition)
void Bisimulation::partition(std::list< StateSet >& rPartition) {
  FD_DF("Bisimulation::partition(rPartition)");

  // prepare result 
  rPartition.clear();

  // loop over all elements of ro
  set<Pnode*>::const_iterator cRoIt = ro.begin();
  for(; cRoIt != ro.end(); ++cRoIt) {
    if((*cRoIt)->states.size()<=1) continue;
    StateSet tb;
    vector<Idx>::iterator cSIt=(*cRoIt)->states.begin();
    for(; cSIt!=(*cRoIt)->states.end(); ++ cSIt) tb.Insert(states[*cSIt].idx);
    rPartition.push_back(tb);
  }
  FD_DF("Bisimulation::partition: leaving function");
}


// Bisimulation::writeW()
void Bisimulation::writeW(void)
{
  FD_DF("Bisimulation:writeW()");
  cout << "Plotting the W-tree:" << endl;
  writeNode(**W.begin());
}

// Bisimulation::writeNode(node)
void Bisimulation::writeNode(Pnode& node)
{
  FD_DF("Bisimulation::writeNode(" << node.index << ")");
  cout << "Coset with index " << node.index << " has the following states:" << endl;
  vector<Idx>::iterator cSIt;
  vector<Idx>::iterator cSItEnd;
  cSIt=node.states.begin();
  cSItEnd=node.states.end();
  for(; cSIt!=cSItEnd; ++ cSIt) cout << *cSIt << " ";
  cout << endl;
  if(node.pParent!=NULL)
    cout << "Parent coset has index: " << node.pParent->index << endl;
  else
    cout << "Coset is the root of the tree" << endl;
  if(node.pParent!=NULL)
  if(node.pParent->pFirstChild!=NULL)
  if(node.index!=node.pParent->pFirstChild->index)
    cout << "Coset has brother coset with index: " << node.pParent->pFirstChild->index << endl;
  if(node.pParent!=NULL)
  if(node.pParent->pSecondChild!=NULL)
  if(node.index!=node.pParent->pSecondChild->index)
    cout << "Coset has brother coset with index: " << node.pParent->pSecondChild->index << endl;


  if(node.pFirstChild!=NULL && node.pSecondChild!=NULL)
    cout << "Child cosets have indices : " << node.pFirstChild->index << " and " << node.pSecondChild->index << endl;
  else
    cout << "Coset has no children" << endl;
  cout << "ro is stable with respect to this coset (1=yes; 0=no): " << node.rostable << endl;
  /*
  cout << "Info-map of coset: " << endl;
  EventSet::Iterator eIt;
  for(eIt=gen->AlphabetBegin();eIt != gen->AlphabetEnd(); ++eIt)
    node.writeInfoMap(*eIt);

  if(node.pFirstChild!=NULL)
    writeNode(*(node.pFirstChild));
  if(node.pSecondChild!=NULL)
    writeNode(*(node.pSecondChild));
  */
}

// Bisimulation::writeRo()
void Bisimulation::writeRo(void)
{
  FD_DF("Bisimulation::writeRo()");
  cout << "The Cosets with the following indices are in ro: " << endl;
  set<Pnode*>::iterator roIt;
  set<Pnode*>::iterator roItBegin =ro.begin();
  set<Pnode*>::iterator roItEnd = ro.end();
  for(roIt=roItBegin; roIt!=roItEnd; roIt++)
    cout << (*roIt)->index << endl;
  cout << endl;
}




/*
********************************************* 
********************************************* 
PART 3: application interface

Minimal application interface based on plain functions.
********************************************* 
********************************************* 
*/

// calcBisimulation(rGenOrig, rMapStateToPartition)
void calcBisimulation(const Generator& rGenOrig, map<Idx,Idx>& rMapStateToPartition)
{
  FD_DF("calcBisimulation(" << rGenOrig.Name() << ", rMapStateToPartition)");
  // Construct an instance of the Bisimulation class from rGenOrig
  Bisimulation bisim = Bisimulation(rGenOrig);
  // method to compute the bisimulation on rGenOrig
  bisim.refine();
  bisim.partition(rMapStateToPartition);
#ifdef FAUDES_DEBUG_FUNCTION
  cout << "The result of the partition refinement is:" << endl;
  bisim.writeW();
  bisim.writeRo();
#endif
  FD_DF("calcBisimulation: leaving function");    
}

// calcBisimulation(rGenOrig, rMapStateToPartition, rGenPart)
void calcBisimulation(const Generator& rGenOrig, map<Idx,Idx>& rMapStateToPartition, Generator& rGenPart)
{
  FD_DF("calcBisimulation(" << rGenOrig.Name() << ", rMapStateToPartition, " << rGenPart.Name() << ")");
  // Construct an instance of the Bisimulation class from rGenOrig
  Bisimulation bisim = Bisimulation(rGenOrig);
  // method to compute the bisimulation on rGenOrig
  bisim.refine();
  bisim.partition(rMapStateToPartition, rGenPart);
#ifdef FD_DF_PLOT
  cout << "The result of the partition refinement is:" << endl;
  bisim.writeW();
  bisim.writeRo();
#endif
  FD_DF("calcBisimulation: leaving function");
}

// calcBisimulation(rGenOrig, rPartition)
void calcBisimulation(const Generator& rGenOrig, std::list< StateSet >& rPartition)
{
  FD_DF("calcBisimulation(" << rGenOrig.Name() << ", rPartition)");
  // Construct an instance of the Bisimulation class from rGenOrig
  Bisimulation bisim = Bisimulation(rGenOrig);
  // method to compute the bisimulation on rGenOrig
  bisim.refine();
  bisim.partition(rPartition);
#ifdef FD_DF_PLOT
  cout << "The result of the partition refinement is:" << endl;
  bisim.writeW();
  bisim.writeRo();
#endif
  FD_DF("calcBisimulation: leaving function");
}


 
}
