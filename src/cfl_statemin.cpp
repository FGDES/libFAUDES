/** @file cfl_statemin.cpp state space minimization */

/* FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2006  Bernd Opitz
Copyright (C) 2015  Thomas Moor
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

  
#include "cfl_statemin.h"
#include "cfl_exception.h"
#include "cfl_project.h"

#include <stack>

namespace faudes {

/*
*********************************************************
Part 1: Stages of Hopcroft algorithm wrapped in a class 
- construct with generator to minimize
- invoke Minimize() to run Hopcroft iteration
- invoke Partition() to construct quotient automaton

[Code revision 201508, tmoor]
*********************************************************
*/

class Hopcroft {

public:

  /**
   * Internal representation of reverse transition relation with consecutive indexed states and events.
   * Technically, re-indexing is equivalent to "pointers on original indicees" and
   * buffers log-n searches for transitions
   */
  struct State {
    Idx idx;      // original state idx
    std::vector< std::vector<Idx> > pre; // predecessors by event
  };
  std::vector<State> states;        // vector of all states [starting with 1 -- use min-index for debugging]
  std::vector<Idx> events;          // vector of all events [starting with 1]

  /**
   * plain stl set debugging output
   */
  std::string setstr(const std::set<Idx>& sset) {
     std::set<Idx>::iterator ssit;
     std::stringstream str;
     str << "{";
     for(ssit = sset.begin(); ssit != sset.end(); ++ssit) 
       str << " " << *ssit;
     str << " }";
     return str.str();
  }

  /**
   * Hopcroft algorithm data structure: vector of blocks
   * [revision 201508 tmoor: use plain stl vectors and maintain sorting manually]
   */
  std::vector< std::vector<Idx> > blocks;

  /**
   * Keep reference to argument (symbolic names etc)
   */
  const Generator* gen;

  /**
   * Initialize from specified generator
   */
  Hopcroft(const Generator& rGen){

    //FAUDES_TIMER_START("");

    // ensure generator is deterministic
#ifdef FAUDES_CHECKED
    if(!rGen.IsDeterministic()) 
     throw Exception("StateMin", "input automaton non-deterministic", 101);
#endif

    //keep ref
    gen=&rGen;

    // convert accessible part to internal data structure
    StateSet acc = gen->AccessibleSet();
    std::map<Idx,Idx> smap; // original->internal
    std::map<Idx,Idx> emap; // original->internal
    Idx max=0;
    events.resize(gen->Alphabet().Size()+1);
    EventSet::Iterator eit=gen->AlphabetBegin();
    for(; eit != gen->AlphabetEnd(); ++eit) {
      emap[*eit]=++max;
      events[max]=*eit;
    }
    max=0;
    states.resize(acc.Size()+1);
    StateSet::Iterator sit=acc.Begin();
    for(; sit != acc.End(); ++sit) {
      smap[*sit]=++max;
      states[max].idx=*sit;
      states[max].pre.resize(events.size());
    }
    TransSet::Iterator tit=gen->TransRelBegin();
    for(; tit != gen->TransRelEnd(); ++tit) {
      if(!acc.Exists(tit->X1)) continue;
      if(!acc.Exists(tit->X2)) continue;
      states[smap[tit->X2]].pre[emap[tit->Ev]].push_back(smap[tit->X1]);
    }

    FD_DF("Hopcroft::Initialize(): states #" << states.size()-1);

    //FAUDES_TIMER_LAP("reindexing: done");

  }


  /**
   * Destruct
   */
  ~Hopcroft(void){
    // nothing to do
  }


  /**
   * Hopcroft iteration (invoke this only once, needs empty blocks vector)
   */
  void Minimize(void) {

    // no accessible states: return no blocks 
    if(states.size()-1 == 0) {
      FD_DF("Hopcroft::Minimize(): generator size 0");
      return;
    }
 
    // one accessible state: return that one block
    if(states.size()-1 == 1) {
      FD_DF("Hopcroft::Minimize(): generator size 1");
      std::vector<Idx>  b;
      b.push_back(1);
      blocks.push_back(b);
      return;
    }

    // loop variables
    std::stack<Idx> active; // active blocks
    std::vector<Idx>::iterator ssit;
    std::vector<Idx>::iterator ssit_end;
    std::vector<Idx> dp;
    std::vector<Idx> dpp;
    std::vector<Idx> c;
      
    // set up blocks B[0] and B[1] as Xm and X-Xm, resp.
    // [revision tmoor 200909: prevent empty blocks]
    std::vector<Idx> bm;
    std::vector<Idx> bnm;
    for(Idx q=1; q<states.size();++q) {
      if(gen->ExistsMarkedState(states[q].idx)) bm.push_back(q);
      else bnm.push_back(q);
    }
    if(bm.size()>0) {
      blocks.push_back(std::vector<Idx>());
      blocks.back().swap(bm);
      active.push(blocks.size()-1);
      FD_DF("Hopcroft::Minimize(): initial partition not-marked  #" << blocks.back().size());
    }
    if(bnm.size()>0) {
      blocks.push_back(std::vector<Idx>());
      blocks.back().swap(bnm);
      active.push(blocks.size()-1);
      FD_DF("Hopcroft::Minimize(): initial partition not-marked  #" << blocks.back().size());
    }

    // while there is an active block B[i]
    while(!active.empty()) {
      FD_WPC(blocks.size()-active.size(), blocks.size(), "StateMin: blocks/active:   " << blocks.size() << " / " << active.size());
      FD_DF("Hopcroft::Minimize(): active blocks #" << active.size());

      // pick an arbitrary active block B[i]  (need a copy for splitting B[i] by itself)
      std::vector<Idx> b_current(blocks.at(active.top()));
      FD_DF("StateMin: getting active block B[" << active.top() << "] = { #" << b_current.size() << "}");
      active.pop();
 
      // compute C = f^-1(B[i])  per event with B[i] as b_current
      Idx ev=1;
      for(; ev!= events.size(); ++ev){
        // iteration over states in current block
        c.clear();
        ssit = b_current.begin();
        ssit_end = b_current.end();
        for(; ssit != ssit_end; ++ssit) {
  	  // find predecessor states by current ev + x2
	  std::vector<Idx>& preevx2=states[*ssit].pre[ev];
	  std::vector<Idx>::iterator rit = preevx2.begin();
	  std::vector<Idx>::iterator rit_end = preevx2.end();
	  for (; rit != rit_end; ++rit) c.push_back(*rit);
        }
	std::sort(c.begin(),c.end());
        // if no predecessor states where found current block wont split anyway
        if(c.empty()) continue;
        // search for block to be split 
        FD_DF("StateMin: computed predecessor states C = {#" << c.size() << "} for event " << gen->EventName(events[ev]));
        // foreach block D 
        for(Idx j=0; j < blocks.size(); ++j) {
          // d_current <- B[j]
          const std::vector<Idx>& d_current = blocks[j];
          // singletons wont split anyway
          if(d_current.size()==1) continue;
	  FD_DF("StateMin: examining block D=B[" << j << "] = {#" << d_current.size() << "}");
	  // compute D' = D intersection C
          dp.clear();
          std::set_intersection(d_current.begin(), d_current.end(), c.begin(), c.end(),       
	     std::inserter(dp, dp.begin()));
	  // check whether D splits by B
	  if(dp.empty() || (dp.size()==d_current.size())) continue; 
	  FD_DF("StateMin: -> split:");  
    	  // compute split block D'' = D - D'
      	  dpp.clear();
          std::set_difference(d_current.begin(), d_current.end(), dp.begin(), dp.end(), 
	     std::inserter(dpp,dpp.begin()));
          // make D'' the smaller part
	  if(dp.size() < dpp.size()) dp.swap(dpp); 
     	  // record split blocks D' and D'' (invalidates references)
          // [performance: using swap (constant) as opposed to assignment (linear) did not make a difference]
	  blocks[j].swap(dp);
          blocks.push_back(std::vector<Idx>());
          blocks.back().swap(dpp);
	  FD_DF("StateMin: new block D' as B[" << j << "] = " << setstr(dp));
	  FD_DF("StateMin: new block D'' as B[" << blocks.size()-1 << "] = " << setstr(dpp));
  	  // if D was active then mark both D', D'' active, else mark smaller part D'' as active
          // [... and both cases effectively amount to mark D'' active]
	  active.push((Idx)blocks.size()- 1);
	} // foreach block D
      } // foreach event
    } // while active blocks exist
    //FAUDES_TIMER_LAP("minimzing: done");

   };


  /***
   * Extract result 
   * By convention: use consecutive state idx matching respective blocks starting with 1
   */
 
  void Partition(Generator& rResGen) {

    FD_DF("Hopcroft:Partition: building minimized generator #" << blocks.size());

    // buffered result (lazy)
    Generator* pResGen= &rResGen;
    if(pResGen == gen) pResGen= gen->New();

    // prepare result
    pResGen->Clear();
    pResGen->Name(gen->Name()+" [minstate]");
    pResGen->InjectAlphabet(gen->Alphabet());
    bool stateNames= pResGen->StateNamesEnabled() && gen->StateNamesEnabled();

    // build minimized generator
    std::map<Idx,Idx> minstatemap; // original states index -> new state index
    // loop over all blocks B
    for(Idx i = 0; i < blocks.size(); ++i) {
      // create state in new generator for every block
      Idx newstate = i+1;
      pResGen->InsState(newstate);
      FD_DF("StateMin: block " << setstr(blocks.at(i)) << " -> new state " << newstate);
      std::ostringstream ostr; 
      std::vector<Idx>::iterator ssit = blocks[i].begin();
      std::vector<Idx>::iterator ssit_end = blocks[i].end();
      for(; ssit != ssit_end; ++ssit) {
        Idx orgstate = states[*ssit].idx;
        // set minstatemap entry for every state in gen
        minstatemap[orgstate] = newstate;
        if(stateNames) {
  	  if (gen->StateName(orgstate) == "") ostr << ToStringInteger(orgstate) << ",";
	  else ostr << gen->StateName(orgstate) << ",";
        }
        // set istates
        if(gen->ExistsInitState(orgstate)) {
	  pResGen->SetInitState(newstate);
	  FD_DF("StateMin: -> initial state");
        }
        // set mstates
        if(gen->ExistsMarkedState(orgstate)) {
	  pResGen->SetMarkedState(newstate);
	  FD_DF("StateMmin: -> marked state");
        }
      }
      if(stateNames) {
        std::string statename = ostr.str();
        if(statename!="") statename.erase(statename.length()-1);
        statename = "{" + statename + "}";
        pResGen->StateName(newstate, statename);
      }
    }
    // create transition relation
    TransSet::Iterator tit = gen->TransRelBegin();
    for(; tit != gen->TransRelEnd(); ++tit) {
      std::map<Idx,Idx>::iterator xit1=minstatemap.find(tit->X1);
      std::map<Idx,Idx>::iterator xit2=minstatemap.find(tit->X2);
      if(xit1==minstatemap.end()) continue;
      if(xit2==minstatemap.end()) continue;
      pResGen->SetTransition(xit1->second, tit->Ev, xit2->second);
    }
    
    // resolve buffered result
    if(pResGen != &rResGen) {
      pResGen->Move(rResGen);
      delete pResGen;
    }

    //FAUDES_TIMER_LAP("quotient-generator: done");
  };
  

  /***
   * Extract result 
   * support original 2006 interface --- depreciated
   */
 
  void Partition(std::vector< StateSet >& rSubsets, std::vector<Idx>& rNewIndices){

    FD_DF("Hopcroft:Partition: returning blocks #" << blocks.size());

    // loop over all blocks B
    StateSet block;
    for(Idx i = 0; i < blocks.size(); ++i) {
      Idx newstate = i+1;
      rNewIndices.push_back(newstate);
      FD_DF("StateMin: block " << setstr(*blocks[i]) << " -> new state " << newstate);
      std::vector<Idx>::iterator ssit = blocks[i].begin();
      std::vector<Idx>::iterator ssit_end = blocks[i].end();
      block.Clear();
      for(; ssit != ssit_end; ++ssit) {
        Idx orgstate = states[*ssit].idx;
        block.Insert(orgstate);
      }
      rSubsets.push_back(block);
    }
    //FAUDES_TIMER_LAP("partition: done");
  };
  


}; // end class Hopcroft



/*
*********************************************************
Part 2: Original 2006 code basis for reference.
-- with 2009 minor revision 
-- with 2015 minor revision for a const-interface
*********************************************************
*/


// StateMin(rGen, rResGen, rSubSets, rNewIndices)
void StateMin_org(const Generator& rGen, Generator& rResGen, 
	      std::vector<StateSet>& rSubsets, std::vector<Idx>& rNewIndices) {
  FD_DF("StateMin: *** computing state space minimization of generator " 
	<< &rGen << " ***");

  FD_DF("StateMin: restrict to accessible states");
  StateSet acc=rGen.AccessibleSet();

  // figure special cases
  if(acc.Size() == 0) {
    FD_DF("StateMin: generator size 0. returning given generator");
    rResGen = rGen;
    rResGen.RestrictStates(acc);
    return;
  }
  if(rGen.Size() == 1) {
    FD_DF("StateMin: generator size 1. returning given generator");
    rResGen = rGen;
    rResGen.RestrictStates(acc);
    rSubsets.push_back(rResGen.States() );
    rNewIndices.push_back(*( rResGen.States().Begin() ) );
    return;
  }

  // ensure generator is deterministic
#ifdef FAUDES_CHECKED
  if(!rGen.IsDeterministic()) {
    throw Exception("StateMin", "input automaton nondeterministic", 101);
  }
#endif
    
  // use pointer pResGen to result rResGen; if rResGen is identical to
  // one of the parameters, allocate temporary object and copy back later
  Generator* pResGen = &rResGen;
  if(&rResGen==&rGen) {
    pResGen= pResGen->New();
  }

  // prepare result
  pResGen->Clear();
  pResGen->Name(rGen.Name()+" [minstate]");
  pResGen->InjectAlphabet(rGen.Alphabet());
  bool stateNames= pResGen->StateNamesEnabled() && rGen.StateNamesEnabled();
  // blocks B[i]
  std::vector<StateSet>& b = rSubsets; // convenience notation "b"
  Idx i, j;
  // set of active b (iterators)
  std::set<Idx> active;
  std::set<Idx>::iterator ait;
  // reverse gen transrel
  TransSetEvX2X1 rtransrel;
  rGen.TransRel(rtransrel);
  TransSetEvX2X1::Iterator rtit, rtit_end;
  // other stuff
  StateSet::Iterator sit; 
  TransSet::Iterator tit, tit_end;

  // set up b "B[i]"
  b.clear();
  i = 0;
  StateSet marked=rGen.MarkedStates()*acc;
  if(marked.Size() > 0) { // tmoor 200909: conditional to prevent emty block 
    FD_DF("StateMin: new block B[" << i << "] = {" << marked.ToString() << "}");
    b.push_back(marked); // Xm
    active.insert(i++);
  }
  StateSet notmarked= acc - marked;
  if(notmarked.Size() > 0) {
    notmarked.Name("B[i]"); 
    FD_DF("StateMin: new block B[" << i << "] = {" << notmarked.ToString() << "}");
    b.push_back(notmarked);  //X -Xm
    active.insert(i++);
  }
  marked.Clear();
  notmarked.Clear();

  // while there is an active block B
  while(! active.empty()) {
    FD_WPC(b.size()-active.size(), b.size(), "StateMin: blocks/active:   " << b.size() << " / " << active.size());
#ifdef FAUDES_DEBUG_FUNCTION
    FD_DF("StateMin: while there is an active block B...");
    std::set<Idx>::iterator _it1;
    std::stringstream str;
    for (_it1 = active.begin(); _it1 != active.end(); ++_it1) {
      str << *_it1 << " ";
    }
    FD_DF("StateMin: active: "+str.str());
    std::vector<StateSet>::iterator _it2;
    str.clear();
    str.str("");
    for (_it2 = b.begin(); _it2 != b.end(); ++_it2) {
      str << "{" << _it2->ToString() << "} "<<std::endl;
    }
    str << std::endl;
    FD_DF("B: "+str.str());
#endif
    // current block B[i]
    i = *(active.begin());
    // inactivate B[i]
    active.erase(active.begin());
    FD_DF("StateMin: getting active block B[" << i << "] = {" <<
	  b.at(i).ToString() << "}");
    // b_current <- B[i]
    StateSet b_current = b.at(i);
 
    // compute C = f^-1(B[i]) for every event in B[i] (as b_current)
    StateSet c;
    EventSet::Iterator eit;
    // iteration over alphabet
    for (eit = rGen.AlphabetBegin(); eit != rGen.AlphabetEnd(); ++eit) {
      c.Clear();
      // iteration over states in current block
      for (sit = b_current.Begin(); sit != b_current.End(); ++sit) {
	// find predecessor states by current ev + x2
	rtit = rtransrel.BeginByEvX2(*eit, *sit);
	rtit_end = rtransrel.EndByEvX2(*eit, *sit);
	for (; rtit != rtit_end; ++rtit) {
	  c.Insert(rtit->X1);
	}
      }
      // if no predecessor states where found, try next event
      if(c.Empty()) continue;
      // search for block to be split 
      FD_DF("StateMin: computed predecessor states C = {" << c.ToString() 
	      << "} for event " << rGen.EventName(*eit));
      // foreach block D 
      for (j=0; j < b.size(); ++j) {
        // d_current <- B[j]
        const StateSet& d_current = b.at(j);
	FD_DF("StateMin: examining block B[" << j << "] = {" << 
		d_current.ToString() << "}");
	// compute D' = D intersection C
	StateSet d_ = d_current * c;
	d_.Name("D'");
	// check D split by B
	if(d_.Empty() || (d_.Size()==d_current.Size())) {
	  FD_DF("StateMin: -> no split");  
	  continue;
	}
	FD_DF("StateMin: -> split:");  
	// compute D'' = D intersected not C; 
	StateSet d__ = d_current - d_;
	d__.Name("D''");
     	// record split block
	b[j] = d_;
	b.push_back(d__);
	FD_DF("StateMin: new block B[" << j << "] = {" << d_.ToString() << "}");
	FD_DF("StateMin: new block B[" << b.size()-1 << "] = {" << d__.ToString() << "}");
  	// if B[j] was active then mark both D', D'' active
	if(active.find(j) != active.end()) {
	  active.insert((Idx)b.size()- 1);
	  FD_DF("StateMin: mark active: " << b.size()-1);
	}
	// else mark smaller of D', D'' active
	else {
	  if (d_.Size() < d__.Size()) {
	    active.insert(j);
	    FD_DF("StateMin: mark active: " << j);
	  } else {
	    active.insert((Idx)b.size()-1);
	    FD_DF("StateMin: mark active: " << b.size()-1);
	  }
	}
      } // foreach block D
    } // foreach event
  } // while active blocks exist

  FD_DF("StateMin: *** building minimized generator ***");
  // build minimized generator
  std::map<Idx,Idx> minstatemap;
  Idx newstate;
  // loop over all blocks B
  for (i = 0; i < b.size(); ++i) {
    // create state in new generator for every block
    newstate = pResGen->InsState();
    rNewIndices.push_back(newstate);
    FD_DF("StateMin: block {" << b.at(i).ToString() 
	  << "} -> new state " << newstate);
    std::ostringstream ostr; 
    for (sit = b.at(i).Begin(); sit != b.at(i).End(); ++sit) {
      // set minstatemap entry for every state in gen
      minstatemap[*sit] = newstate;
      if(stateNames) {
	if (rGen.StateName(*sit) == "") {
	  ostr << ToStringInteger(*sit) << ",";
	}
	else {
	  ostr << rGen.StateName(*sit) << ",";
	}
      }
      // set istates
      if(rGen.ExistsInitState(*sit)) {
	pResGen->SetInitState(newstate);
	FD_DF("StateMin: -> initial state");
      }
      // set mstates
      if(rGen.ExistsMarkedState(*sit)) {
	pResGen->SetMarkedState(newstate);
	FD_DF("StatMmin: -> marked state");
      }
    }
    if(stateNames) {
      std::string statename = ostr.str();
      if(statename!="") statename.erase(statename.length()-1);
      statename = "{" + statename + "}";
      pResGen->StateName(newstate, statename);
    }
  }
  // create transition relation
  for (tit = rGen.TransRelBegin(); tit != rGen.TransRelEnd(); ++tit) {
    if(!acc.Exists(tit->X1)) continue;
    if(!acc.Exists(tit->X2)) continue;
    pResGen->SetTransition(minstatemap[tit->X1], tit->Ev, minstatemap[tit->X2]);
    FD_DF("statemin: adding transition: " 
	  << minstatemap[tit->X1] << "-" << tit->Ev << "-" 
	  << minstatemap[tit->X2]);
  }
    
  // if necessary, move pResGen to rResGen
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}





/*
*********************************************************
Part 3: provide std function api, incl 2006 interface
[using revision 201508 tmoor]
*********************************************************
*/


// StateMin(rGen, rResGen)
void StateMin(const Generator& rGen, Generator& rResGen) {
  Hopcroft hc(rGen);
  hc.Minimize();
  hc.Partition(rResGen);
}

// StateMin(rGen, rResGen, maps ...) ---- original interface except for "const"
void StateMin(const Generator& rGen, Generator& rResGen, 
  std::vector< StateSet >& rSubsets, std::vector<Idx>& rNewIndices){
  Hopcroft hc(rGen);
  hc.Minimize();
  hc.Partition(rSubsets,rNewIndices);
  hc.Partition(rResGen);
} 

// StateMin(rGen, rResGen)
void aStateMin(const Generator& rGen, Generator& rResGen) {
  StateMin(rGen, rResGen);
  rResGen.EventAttributes(rGen.Alphabet());
}

// StateMin(rGen)
void aStateMin(Generator& rGen) {
  aStateMin(rGen,rGen);
}





  
} // namespace faudes
