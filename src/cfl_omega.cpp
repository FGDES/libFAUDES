/** @file cfl_omega.cpp

Operations regarding omega languages.

*/

/* FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2010 Thomas Moor

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

  

#include "cfl_omega.h"

#include "cfl_parallel.h"
#include "cfl_project.h" 
#include "cfl_graphfncts.h" 


namespace faudes {


// helper class for omega compositions
class OPState {
public:
  // minimal interface
  OPState() {};
  OPState(const Idx& rq1, const Idx& rq2, const bool& rf) :
    q1(rq1), q2(rq2), m1required(rf), mresolved(false) {};
  std::string Str(void) { return ToStringInteger(q1)+"|"+
      ToStringInteger(q2)+"|"+ToStringInteger(m1required); };
  // order
  bool operator < (const OPState& other) const {
    if (q1 < other.q1) return(true);
    if (q1 > other.q1) return(false);
    if (q2 < other.q2) return(true);
    if (q2 > other.q2) return(false);
    if (!m1required && other.m1required)  return(true);
    if (m1required &&  !other.m1required) return(false);
    if (!mresolved &&  other.mresolved)  return(true);
    return(false);
  }
  // member variables
  Idx q1;
  Idx q2;
  bool m1required;
  bool mresolved;
};



// OmegaProduct for Generators, transparent for event attributes.
void aOmegaProduct(
  const Generator& rGen1,
  const Generator& rGen2,
  Generator& rResGen) 
{

  // inputs have to agree on attributes of shared events:
  bool careattr=rGen1.Alphabet().EqualAttributes(rGen2.Alphabet());

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen1 || &rResGen== &rGen2) {
    pResGen= rResGen.New();
  }

  // make product composition of inputs
  OmegaProduct(rGen1,rGen2,*pResGen);

  // copy all attributes of input alphabets
  if(careattr) {
    pResGen->EventAttributes(rGen1.Alphabet());
    pResGen->EventAttributes(rGen2.Alphabet());
  }

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

}



// OmegaProduct(rGen1, rGen2, res)
void OmegaProduct(
  const Generator& rGen1, const Generator& rGen2, 
  Generator& rResGen)
{
  FD_DF("OmegaProduct(" << &rGen1 << "," << &rGen2 << ")");

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen1 || &rResGen== &rGen2) {
    pResGen= rResGen.New();
  }
  pResGen->Clear();
  pResGen->Name(CollapsString(rGen1.Name()+".x."+rGen2.Name()));

  // create res alphabet
  pResGen->InsEvents(rGen1.Alphabet());
  pResGen->InsEvents(rGen2.Alphabet());

  // reverse composition map
  std::map< OPState, Idx> reverseCompositionMap;
  // todo stack
  std::stack< OPState > todo;
  // current pair, new pair
  OPState currentstates, newstates;
  // state
  Idx tmpstate;  
  StateSet::Iterator lit1, lit2;
  TransSet::Iterator tit1, tit1_end, tit2, tit2_end;
  std::map< OPState, Idx>::iterator rcit;
  // push all combinations of initial states on todo stack
  FD_DF("OmegaProduct: adding all combinations of initial states to todo:");
  for (lit1 = rGen1.InitStatesBegin(); lit1 != rGen1.InitStatesEnd(); ++lit1) {
    for (lit2 = rGen2.InitStatesBegin(); lit2 != rGen2.InitStatesEnd(); ++lit2) {
      currentstates = OPState(*lit1, *lit2, true);
      todo.push(currentstates);
      reverseCompositionMap[currentstates] = pResGen->InsInitState();
      FD_DF("OmegaProduct:   " << currentstates.Str() << " -> " << reverseCompositionMap[currentstates]);
    }
  }

  // start algorithm
  FD_DF("OmegaProduct: processing reachable states:");
  while (! todo.empty()) {
    // allow for user interrupt
    LoopCallback();
    // get next reachable state from todo stack
    currentstates = todo.top();
    todo.pop();
    FD_DF("OmegaProduct: processing (" << currentstates.Str() << " -> " << reverseCompositionMap[currentstates]);
    // iterate over all rGen1 transitions  
    tit1 = rGen1.TransRelBegin(currentstates.q1);
    tit1_end = rGen1.TransRelEnd(currentstates.q1);
    for(; tit1 != tit1_end; ++tit1) {
      // find transition in rGen2
      tit2 = rGen2.TransRelBegin(currentstates.q2, tit1->Ev);
      tit2_end = rGen2.TransRelEnd(currentstates.q2, tit1->Ev);
      for (; tit2 != tit2_end; ++tit2) {
        newstates = OPState(tit1->X2, tit2->X2,currentstates.m1required);
        // figure whether marking was resolved
        if(currentstates.m1required) {
  	  if(rGen1.ExistsMarkedState(currentstates.q1))
	    newstates.m1required=false;
        } else {
  	  if(rGen2.ExistsMarkedState(currentstates.q2))
	    newstates.m1required=true;
        }
        // add to todo list if composition state is new
        rcit = reverseCompositionMap.find(newstates);
        if(rcit == reverseCompositionMap.end()) {
          todo.push(newstates);
          tmpstate = pResGen->InsState();
          if(!newstates.m1required)
	    if(rGen2.ExistsMarkedState(newstates.q2))
              pResGen->SetMarkedState(tmpstate);
          reverseCompositionMap[newstates] = tmpstate;
          FD_DF("OmegaProduct:   todo push: (" << newstates.Str() << ") -> " << reverseCompositionMap[newstates]);
        }
        else {
          tmpstate = rcit->second;
        }
        pResGen->SetTransition(reverseCompositionMap[currentstates], 
            tit1->Ev, tmpstate);
        FD_DF("OmegaProduct:  add transition to new generator: " << 
	      pResGen->TStr(Transition(reverseCompositionMap[currentstates], tit1->Ev, tmpstate)));
      }
    }
  }

  FD_DF("OmegaProduct: marked states: "  << pResGen->MarkedStatesToString());


  // fix statenames ...
  if(rGen1.StateNamesEnabled() && rGen2.StateNamesEnabled() && rResGen.StateNamesEnabled()) 
  for(rcit=reverseCompositionMap.begin(); rcit!=reverseCompositionMap.end(); rcit++) {
    Idx x1=rcit->first.q1;
    Idx x2=rcit->first.q2;
    bool m1requ=rcit->first.m1required;
    Idx x12=rcit->second;
    if(!pResGen->ExistsState(x12)) continue;
    std::string name1= rGen1.StateName(x1);
    if(name1=="") name1=ToStringInteger(x1);
    std::string name2= rGen2.StateName(x2);
    if(name2=="") name1=ToStringInteger(x2);
    std::string name12 = name1 + "|" + name2;
    if(m1requ) name12 += "|r1m";
    else name12 +="|r2m";
    name12=pResGen->UniqueStateName(name12);
    pResGen->StateName(x12,name12);
  }

  // .. or clear them (?)
  if(!(rGen1.StateNamesEnabled() && rGen2.StateNamesEnabled() && rResGen.StateNamesEnabled())) 
    pResGen->StateNamesEnabled(false);

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}



// OmegaParallel for Generators, transparent for event attributes.
void aOmegaParallel(
  const Generator& rGen1,
  const Generator& rGen2,
  Generator& rResGen) 
{

  // inputs have to agree on attributes of shared events:
  bool careattr=rGen1.Alphabet().EqualAttributes(rGen2.Alphabet());

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen1 || &rResGen== &rGen2) {
    pResGen= rResGen.New();
  }

  // make product composition of inputs
  OmegaParallel(rGen1,rGen2,*pResGen);

  // copy all attributes of input alphabets
  if(careattr) {
    pResGen->EventAttributes(rGen1.Alphabet());
    pResGen->EventAttributes(rGen2.Alphabet());
  }

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

}



// OmegaParallel(rGen1, rGen2, res)
void OmegaParallel(
  const Generator& rGen1, const Generator& rGen2, 
  Generator& rResGen)
{
  FD_DF("OmegaParallel(" << &rGen1 << "," << &rGen2 << ")");
  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rGen1 || &rResGen== &rGen2) {
    pResGen= rResGen.New();
  }
  pResGen->Clear();
  pResGen->Name(CollapsString(rGen1.Name()+"|||"+rGen2.Name()));
  // create res alphabet
  pResGen->InsEvents(rGen1.Alphabet());
  pResGen->InsEvents(rGen2.Alphabet());
  // shared events
  EventSet sharedalphabet = rGen1.Alphabet() * rGen2.Alphabet();
  FD_DF("OmegaParallel: g1 events: " << rGen1.Alphabet().ToString());
  FD_DF("OmegaParallel: g2 events: " << rGen2.Alphabet().ToString());
  FD_DF("OmegaParallel: shared events: " << sharedalphabet.ToString());

  // reverse composition map
  std::map< OPState, Idx> reverseCompositionMap;
  // todo stack
  std::stack< OPState > todo;
  // current pair, new pair
  OPState currentstates, newstates;
  // state
  Idx tmpstate;  
  StateSet::Iterator lit1, lit2;
  TransSet::Iterator tit1, tit1_end, tit2, tit2_end;
  std::map< OPState, Idx>::iterator rcit;
  // push all combinations of initial states on todo stack
  FD_DF("OmegaParallel: adding all combinations of initial states to todo:");
  for (lit1 = rGen1.InitStatesBegin(); lit1 != rGen1.InitStatesEnd(); ++lit1) {
    for (lit2 = rGen2.InitStatesBegin(); lit2 != rGen2.InitStatesEnd(); ++lit2) {
      currentstates = OPState(*lit1, *lit2, true);
      tmpstate=pResGen->InsInitState();
      if(rGen2.ExistsMarkedState(*lit2)) { 
        currentstates.mresolved=true;
        pResGen->SetMarkedState(tmpstate);
      }
      todo.push(currentstates);
      reverseCompositionMap[currentstates] = tmpstate;
      FD_DF("OmegaParallel:   " << currentstates.Str() << " -> " << tmpstate);
    }
  }

  // start algorithm
  FD_DF("OmegaParallel: processing reachable states:");
  while (! todo.empty()) {
    // allow for user interrupt
    LoopCallback();
    // get next reachable state from todo stack
    currentstates = todo.top();
    todo.pop();
    FD_DF("OmegaParallel: processing (" << currentstates.Str() << " -> " 
        << reverseCompositionMap[currentstates]);
    // iterate over all rGen1 transitions 
    // (includes execution of shared events)
    tit1 = rGen1.TransRelBegin(currentstates.q1);
    tit1_end = rGen1.TransRelEnd(currentstates.q1);
    for(; tit1 != tit1_end; ++tit1) {
      // if event not shared
      if(! sharedalphabet.Exists(tit1->Ev)) {
        FD_DF("OmegaParallel:   exists only in rGen1");
        newstates = OPState(tit1->X2, currentstates.q2, currentstates.m1required);
        // figure whether marking is resolved 
        // (tmoor 201208: only m1required can be resolved, since only G1 proceeds)
        if(currentstates.m1required) {
          if(rGen1.ExistsMarkedState(newstates.q1)) {
            newstates.m1required=false;
	    //newstates.mresolved=true; // optional
	  }
	}
        // add to todo list if composition state is new
        rcit = reverseCompositionMap.find(newstates);
        if (rcit == reverseCompositionMap.end()) {
          todo.push(newstates);
          tmpstate = pResGen->InsState();
          if(newstates.mresolved) pResGen->SetMarkedState(tmpstate);                   
          reverseCompositionMap[newstates] = tmpstate;
          FD_DF("OmegaParallel:   todo push: " << newstates.Str() << "|" 
              << reverseCompositionMap[newstates]);
        }
        else {
          tmpstate = rcit->second;
        }
	// insert transition to result 
        pResGen->SetTransition(reverseCompositionMap[currentstates], tit1->Ev, 
            tmpstate);
        FD_DF("OmegaParallel:  add transition to new generator: " << 
	      pResGen->TStr(Transition(reverseCompositionMap[currentstates], tit1->Ev, tmpstate)));
      }
      // if shared event
      else {
        FD_DF("OmegaParallel:   common event");
        // find shared transitions
        tit2 = rGen2.TransRelBegin(currentstates.q2, tit1->Ev);
        tit2_end = rGen2.TransRelEnd(currentstates.q2, tit1->Ev);
        for (; tit2 != tit2_end; ++tit2) {
          newstates = OPState(tit1->X2,tit2->X2,currentstates.m1required);
          // figure whether marking was resolved
          // (tmoor 201208: both markings can be resolved, since both G1 and G2 proceed)
          if(currentstates.m1required) {
  	    if(rGen1.ExistsMarkedState(newstates.q1)) {
	      newstates.m1required=false;
	      //newstates.mresolved=true; // optional
	    }
          } else {
  	    if(rGen2.ExistsMarkedState(newstates.q2)) {
	      newstates.m1required=true;
              newstates.mresolved=true;
	    }
          }
          // add to todo list if composition state is new
          rcit = reverseCompositionMap.find(newstates);
          if (rcit == reverseCompositionMap.end()) {
            todo.push(newstates);
            tmpstate = pResGen->InsState();
            if(newstates.mresolved) pResGen->SetMarkedState(tmpstate);                   
            reverseCompositionMap[newstates] = tmpstate;
            FD_DF("OmegaParallel:   todo push: (" << newstates.Str() << ") -> " 
                << reverseCompositionMap[newstates]);
          }
          else {
            tmpstate = rcit->second;
          }
          pResGen->SetTransition(reverseCompositionMap[currentstates], 
              tit1->Ev, tmpstate);
          FD_DF("OmegaParallel:  add transition to new generator: " << 
		pResGen->TStr(Transition(reverseCompositionMap[currentstates], tit1->Ev, tmpstate)));
        }
      }
    }
    // iterate over all remaining rGen2 transitions 
    // (without execution of shared events)
    tit2 = rGen2.TransRelBegin(currentstates.q2);
    tit2_end = rGen2.TransRelEnd(currentstates.q2);
    for (; tit2 != tit2_end; ++tit2) {
      if (! sharedalphabet.Exists(tit2->Ev)) {
        FD_DF("OmegaParallel:   exists only in rGen2: " << sharedalphabet.Str(tit2->Ev));
        newstates = OPState(currentstates.q1, tit2->X2, currentstates.m1required);
        // figure whether marking was resolved
        // (tmoor 201208: only m2required=!m1required can be resolved, since only G2 proceeds)
        if(!currentstates.m1required) {
          if(rGen2.ExistsMarkedState(newstates.q2)) {
            newstates.m1required=true;
            newstates.mresolved=true;
	  }
        }
        FD_DF("OmegaParallel: set trans: " << currentstates.Str() << " " << newstates.Str()); 
        // add to todo list if composition state is new
        rcit = reverseCompositionMap.find(newstates);
        if (rcit == reverseCompositionMap.end()) {
          todo.push(newstates);
          tmpstate = pResGen->InsState();
          if(newstates.mresolved) pResGen->SetMarkedState(tmpstate);                   
          reverseCompositionMap[newstates] = tmpstate;
          FD_DF("OmegaParallel:   todo push: " << newstates.Str() << " -> " 
              << reverseCompositionMap[newstates]);
        }
        else {
          tmpstate = rcit->second;
        }
        pResGen->SetTransition(reverseCompositionMap[currentstates], 
            tit2->Ev, tmpstate);
        FD_DF("OmegaParallel:  add transition to new generator: " << 
	      pResGen->TStr(Transition(reverseCompositionMap[currentstates], tit2->Ev, tmpstate)));
      }
    }
  }

  FD_DF("OmegaParallel: marked states: " << pResGen->MarkedStatesToString());

  // fix statenames ...
  if(rGen1.StateNamesEnabled() && rGen2.StateNamesEnabled() && rResGen.StateNamesEnabled()) 
  for(rcit=reverseCompositionMap.begin(); rcit!=reverseCompositionMap.end(); rcit++) {
    Idx x1=rcit->first.q1;
    Idx x2=rcit->first.q2;
    bool m1requ=rcit->first.m1required;
    bool mres=rcit->first.mresolved;
    Idx x12=rcit->second;
    if(!pResGen->ExistsState(x12)) continue;
    std::string name1= rGen1.StateName(x1);
    if(name1=="") name1=ToStringInteger(x1);
    std::string name2= rGen2.StateName(x2);
    if(name2=="") name1=ToStringInteger(x2);
    std::string name12 = name1 + "|" + name2 ;
    if(m1requ) name12 += "|r1m";
    else name12 +="|r2m";
    if(mres) name12 += "*";
    name12=pResGen->UniqueStateName(name12);
    pResGen->StateName(x12,name12);
  }

  // .. or clear them (?)
  if(!(rGen1.StateNamesEnabled() && rGen2.StateNamesEnabled() && rResGen.StateNamesEnabled())) 
    pResGen->StateNamesEnabled(false);

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}


// OmegaClosure(rGen)
void OmegaClosure(Generator& rGen) {
  FD_DF("OmegaClosure("<< rGen.Name() << ")");

  // fix name
  std::string name=CollapsString("OmegaClosure("+ rGen.Name() + ")");
    
  // remove all states that do net represent prefixes of marked strings
  rGen.OmegaTrim();
    
  // mark all remaining states
  rGen.InjectMarkedStates(rGen.States());
   
  // set name 
  rGen.Name(name);
}


// IsOmegaClosed
bool IsOmegaClosed(const Generator& rGen) {
  
  FD_DF("IsOmegaClosed("<< rGen.Name() << ")");

  // figure irrelevant states: not coaccessible / accessible
  StateSet irrelevant = rGen.States();
  irrelevant.EraseSet(rGen.AccessibleSet()* rGen.CoaccessibleSet());
  
  // figure irrelevant states: terminal
  irrelevant.InsertSet(rGen.TerminalStates());

  // iterative search on indirect terminal states ...
  bool done;
  do {
    // ... over all states
    StateSet::Iterator sit = rGen.States().Begin();
    StateSet::Iterator sit_end = rGen.States().End();
    done=true;
    for(; sit!=sit_end; ++sit) {
      if(irrelevant.Exists(*sit)) continue;
      TransSet::Iterator tit = rGen.TransRelBegin(*sit);
      TransSet::Iterator tit_end = rGen.TransRelEnd(*sit);
      for (; tit != tit_end; ++tit) {
        if(!irrelevant.Exists(tit->X2)) break;
      }
      if(tit==tit_end) {
        irrelevant.Insert(*sit);
	done=false;
      }
    }
  } while(!done);

  // marked states are irrelevant here
  irrelevant.InsertSet(rGen.MarkedStates());

  // report
#ifdef FAUDES_DEBUG_FUNCTION
  FD_DF("IsOmegaClosed(..): irrelevant states "<< irrelevant.ToString());
#endif

  // locate unmarked SCCs
  // find all relevant SCCs
  SccFilter umfilter(SccFilter::FmIgnoreTrivial | SccFilter::FmStatesAvoid, irrelevant);
  std::list<StateSet> umsccs;
  StateSet umroots;
  ComputeScc(rGen,umfilter,umsccs,umroots); 

  // report
#ifdef FAUDES_DEBUG_FUNCTION
  std::list<StateSet>::iterator ssit=umsccs.begin();
  for(;ssit!=umsccs.end(); ++ssit) {
    FD_DF("IsOmegaClosed(): GPlant-marked scc without GCand-mark: " << ssit->ToString());
  }  
#endif

  // done
  return umsccs.empty();

}



} // namespace faudes

