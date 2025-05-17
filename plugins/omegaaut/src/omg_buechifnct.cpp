/** @file omg_buechifnct.cpp

Operations regarding omega languages accepted by Buechi automata

*/

/* FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2010, 2025 Thomas Moor

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

  

#include "omg_buechifnct.h"


namespace faudes {

// BuechiTrim
// (return  True if resulting contains at least one initial state and at least one mark)
bool BuechiTrim(vGenerator& rGen) {

  // note: this really is inefficient; at least we should
  // record the inverse transition relation for the coaccessile
  // iteration

  // first we make the generator accessible
  rGen.Accessible();
  // iterate coaccessible and complete until convergence in oserved
  while(1) {
    Idx osize=rGen.States().Size();
    rGen.Coaccessible();
    rGen.Complete();
    if(rGen.States().Size()==osize) break;
  }    
  // done
  return !rGen.InitStates().Empty() && !rGen.MarkedStates().Empty();
}

// rti wrapper
bool BuechiTrim(const vGenerator& rGen, vGenerator& rRes) {
  rRes=rGen;
  return BuechiTrim(rRes);
}  

// IsBuechiTrim()
bool IsBuechiTrim(const vGenerator& rGen) {
  bool res=true;
  if(!rGen.IsAccessible()) res=false;
  else if(!rGen.IsCoaccessible()) res=false;
  else if(!rGen.IsComplete()) res=false;
  FD_DF("IsBuechiTrim(): result " << res);
  return res;
}

// BuechiClosure(rGen)
void BuechiClosure(Generator& rGen) {
  FD_DF("BuechiClosure("<< rGen.Name() << ")");
  // fix name
  std::string name=CollapsString("BuechiClosure("+ rGen.Name() + ")");    
  // remove all states that do net represent prefixes of marked strings
  BuechiTrim(rGen);
  // mark all remaining states
  rGen.InjectMarkedStates(rGen.States());   
  // set name 
  rGen.Name(name);
}


// IsBuechiClosed
bool IsBuechiClosed(const Generator& rGen) {  
  FD_DF("IsBuechiClosed("<< rGen.Name() << ")");
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
  FD_DF("IsBuechiClosed(..): irrelevant states "<< irrelevant.ToString());
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
    FD_DF("IsBuechiClosed(): GPlant-marked scc without GCand-mark: " << ssit->ToString());
  }  
#endif
  // done
  return umsccs.empty();
}

  
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



// BuechiProduct(rGen1, rGen2, res)
void BuechiProduct(
  const Generator& rGen1, const Generator& rGen2, 
  Generator& rResGen)
{
  FD_DF("BuechiProduct(" << &rGen1 << "," << &rGen2 << ")");

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
  FD_DF("BuechiProduct: adding all combinations of initial states to todo:");
  for (lit1 = rGen1.InitStatesBegin(); lit1 != rGen1.InitStatesEnd(); ++lit1) {
    for (lit2 = rGen2.InitStatesBegin(); lit2 != rGen2.InitStatesEnd(); ++lit2) {
      currentstates = OPState(*lit1, *lit2, true);
      todo.push(currentstates);
      reverseCompositionMap[currentstates] = pResGen->InsInitState();
      FD_DF("BuechiProduct:   " << currentstates.Str() << " -> " << reverseCompositionMap[currentstates]);
    }
  }

  // start algorithm
  FD_DF("BuechiProduct: processing reachable states:");
  while (! todo.empty()) {
    // allow for user interrupt
    LoopCallback();
    // get next reachable state from todo stack
    currentstates = todo.top();
    todo.pop();
    FD_DF("BuechiProduct: processing (" << currentstates.Str() << " -> " << reverseCompositionMap[currentstates]);
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
          FD_DF("BuechiProduct:   todo push: (" << newstates.Str() << ") -> " << reverseCompositionMap[newstates]);
        }
        else {
          tmpstate = rcit->second;
        }
        pResGen->SetTransition(reverseCompositionMap[currentstates], 
            tit1->Ev, tmpstate);
        FD_DF("BuechiProduct:  add transition to new generator: " << 
	      pResGen->TStr(Transition(reverseCompositionMap[currentstates], tit1->Ev, tmpstate)));
      }
    }
  }

  FD_DF("BuechiProduct: marked states: "  << pResGen->MarkedStatesToString());


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

// BuechiProduct for Generators, transparent for event attributes.
void aBuechiProduct(
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
  BuechiProduct(rGen1,rGen2,*pResGen);
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


// BuechiParallel(rGen1, rGen2, res)
void BuechiParallel(
  const Generator& rGen1, const Generator& rGen2, 
  Generator& rResGen)
{
  FD_DF("BuechiParallel(" << &rGen1 << "," << &rGen2 << ")");
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
  FD_DF("BuechiParallel: g1 events: " << rGen1.Alphabet().ToString());
  FD_DF("BuechiParallel: g2 events: " << rGen2.Alphabet().ToString());
  FD_DF("BuechiParallel: shared events: " << sharedalphabet.ToString());

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
  FD_DF("BuechiParallel: adding all combinations of initial states to todo:");
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
      FD_DF("BuechiParallel:   " << currentstates.Str() << " -> " << tmpstate);
    }
  }

  // start algorithm
  FD_DF("BuechiParallel: processing reachable states:");
  while (! todo.empty()) {
    // allow for user interrupt
    LoopCallback();
    // get next reachable state from todo stack
    currentstates = todo.top();
    todo.pop();
    FD_DF("BuechiParallel: processing (" << currentstates.Str() << " -> " 
        << reverseCompositionMap[currentstates]);
    // iterate over all rGen1 transitions 
    // (includes execution of shared events)
    tit1 = rGen1.TransRelBegin(currentstates.q1);
    tit1_end = rGen1.TransRelEnd(currentstates.q1);
    for(; tit1 != tit1_end; ++tit1) {
      // if event not shared
      if(! sharedalphabet.Exists(tit1->Ev)) {
        FD_DF("BuechiParallel:   exists only in rGen1");
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
          FD_DF("BuechiParallel:   todo push: " << newstates.Str() << "|" 
              << reverseCompositionMap[newstates]);
        }
        else {
          tmpstate = rcit->second;
        }
	// insert transition to result 
        pResGen->SetTransition(reverseCompositionMap[currentstates], tit1->Ev, 
            tmpstate);
        FD_DF("BuechiParallel:  add transition to new generator: " << 
	      pResGen->TStr(Transition(reverseCompositionMap[currentstates], tit1->Ev, tmpstate)));
      }
      // if shared event
      else {
        FD_DF("BuechiParallel:   common event");
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
            FD_DF("BuechiParallel:   todo push: (" << newstates.Str() << ") -> " 
                << reverseCompositionMap[newstates]);
          }
          else {
            tmpstate = rcit->second;
          }
          pResGen->SetTransition(reverseCompositionMap[currentstates], 
              tit1->Ev, tmpstate);
          FD_DF("BuechiParallel:  add transition to new generator: " << 
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
        FD_DF("BuechiParallel:   exists only in rGen2: " << sharedalphabet.Str(tit2->Ev));
        newstates = OPState(currentstates.q1, tit2->X2, currentstates.m1required);
        // figure whether marking was resolved
        // (tmoor 201208: only m2required=!m1required can be resolved, since only G2 proceeds)
        if(!currentstates.m1required) {
          if(rGen2.ExistsMarkedState(newstates.q2)) {
            newstates.m1required=true;
            newstates.mresolved=true;
	  }
        }
        FD_DF("BuechiParallel: set trans: " << currentstates.Str() << " " << newstates.Str()); 
        // add to todo list if composition state is new
        rcit = reverseCompositionMap.find(newstates);
        if (rcit == reverseCompositionMap.end()) {
          todo.push(newstates);
          tmpstate = pResGen->InsState();
          if(newstates.mresolved) pResGen->SetMarkedState(tmpstate);                   
          reverseCompositionMap[newstates] = tmpstate;
          FD_DF("BuechiParallel:   todo push: " << newstates.Str() << " -> " 
              << reverseCompositionMap[newstates]);
        }
        else {
          tmpstate = rcit->second;
        }
        pResGen->SetTransition(reverseCompositionMap[currentstates], 
            tit2->Ev, tmpstate);
        FD_DF("BuechiParallel:  add transition to new generator: " << 
	      pResGen->TStr(Transition(reverseCompositionMap[currentstates], tit2->Ev, tmpstate)));
      }
    }
  }

  FD_DF("BuechiParallel: marked states: " << pResGen->MarkedStatesToString());

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


// BuechiParallel for Generators, transparent for event attributes.
void aBuechiParallel(
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
  BuechiParallel(rGen1,rGen2,*pResGen);
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


// IsOmegaRelativelyMarked(rGenPlant,rGenCand)
bool IsBuechiRelativelyMarked(const Generator& rGenPlant, const Generator& rGenCand) {


  FD_DF("IsBuechiRelativelyMarked(\"" <<  rGenPlant.Name() << "\", \"" << rGenCand.Name() << "\")");

  // alphabets must match
  if ( rGenPlant.Alphabet() != rGenCand.Alphabet()) {
    std::stringstream errstr;
    errstr << "Alphabets of generators do not match.";
    throw Exception("BuechiRelativelyMarked", errstr.str(), 100);
  }

#ifdef FAUDES_CHECKED
  // generators are meant to be nonblocking
  if ( !IsBuechiTrim(rGenCand) ||  !IsBuechiTrim(rGenPlant)) {
    std::stringstream errstr;
    errstr << "Arguments are expected to be nonblocking.";
    throw Exception("IsBuechiRelativelyMarked", errstr.str(), 201);
  }
#endif

#ifdef FAUDES_CHECKED
  // generators are meant to be deterministic
  if ( !IsDeterministic(rGenCand) ||  !IsDeterministic(rGenPlant)) {
    std::stringstream errstr;
    errstr << "Arguments are expected to be deterministic.";
    throw Exception("IsBuechiRelativelyMarked", errstr.str(), 202);
  }
#endif

  
  // perform composition
  std::map< std::pair<Idx,Idx> , Idx> revmap;
  Generator product;
  product.StateNamesEnabled(false);
  StateSet markCand;
  StateSet markPlant;
  Product(rGenPlant,rGenCand,revmap,markPlant,markCand,product);

  // find all relevant SCCs
  SccFilter umfilter(SccFilter::FmIgnoreTrivial | SccFilter::FmStatesAvoid| SccFilter::FmStatesRequire, 
    markCand,markPlant);
  std::list<StateSet> umsccs;
  StateSet umroots;
  ComputeScc(product,umfilter,umsccs,umroots); 

  // report
  std::list<StateSet>::iterator ssit=umsccs.begin();
  for(;ssit!=umsccs.end(); ++ssit) {
    FD_DF("IsBuechiRelativelyMarked(): GPlant-marked scc without GCand-mark: " << ssit->ToString());
  }  

  // result is true if no problematic SCCs exist
  return umsccs.size()==0;


}




// IsOmegaRelativelyClosed(rGenPlant,rGenCand)
bool IsBuechiRelativelyClosed(const Generator& rGenPlant, const Generator& rGenCand) {


  FD_DF("IsBuechiRelativelyClosed(\"" <<  rGenPlant.Name() << "\", \"" << rGenCand.Name() << "\")");

  // alphabets must match
  if ( rGenPlant.Alphabet() != rGenCand.Alphabet()) {
    std::stringstream errstr;
    errstr << "Alphabets of generators do not match.";
    throw Exception("BuechiRelativelyClosed", errstr.str(), 100);
  }

#ifdef FAUDES_CHECKED
  // generators are meant to be nonblocking
  if( !IsBuechiTrim(rGenCand) ) {
    std::stringstream errstr;
    errstr << "Argument \"" << rGenCand.Name() << "\" is not omegatrim.";
    throw Exception("IsBuechiRelativelyClosed", errstr.str(), 201);
  }
  if( !IsBuechiTrim(rGenPlant) ) {
    std::stringstream errstr;
    errstr << "Argument \"" << rGenPlant.Name() << "\" is not omega-trim.";
    throw Exception("IsBuechiRelativelyClosed", errstr.str(), 201);
  }
#endif


  // the trivial case: if B1 is empty it is relatively closed
  // (we must treat this case because empty generators are not regarded deterministic)
  if(rGenCand.Empty()) {
    FD_DF("IsBuechiRelativelyClosed(..): empty candidate: pass");
    return true;
  }

  // the trivial case: if B2 is empty but B1 is not empty, the test failed
  // (we must treat this case because empty generators are not regarded deterministic)
  if(rGenPlant.Empty()) {
    FD_DF("IsBuechiRelativelyClosed(..): non-empty candidate. empty plant: fail");
    return false;
  }

#ifdef FAUDES_CHECKED
  // generators are meant to be deterministic
  if ( !IsDeterministic(rGenCand) ||  !IsDeterministic(rGenPlant)) {
    std::stringstream errstr;
    errstr << "Arguments are expected to be deterministic.";
    throw Exception("IsBuechiRelativelyClosed", errstr.str(), 202);
  }
#endif

  // doit
  return IsBuechiRelativelyClosedUnchecked(rGenPlant,rGenCand);
}

  
// IsOmegaRelativelyClosed(rGenPlant,rGenCand)
bool IsBuechiRelativelyClosedUnchecked(const Generator& rGenPlant, const Generator& rGenCand) {

  // perform composition (variant of cfl_parallel.cpp)
  std::map< std::pair<Idx,Idx> , Idx> revmap;
  Generator product;
  product.StateNamesEnabled(false);
  StateSet mark1;
  StateSet mark2;

  // shared alphabet
  product.InjectAlphabet(rGenCand.Alphabet());

  // todo stack
  std::stack< std::pair<Idx,Idx> > todo;
  // current pair, new pair
  std::pair<Idx,Idx> currentstates, newstates;
  // state
  Idx tmpstate;
  // iterators  
  StateSet::Iterator lit1, lit2;
  TransSet::Iterator tit1, tit1_end, tit2, tit2_end;
  std::map< std::pair<Idx,Idx>, Idx>::iterator rcit;
  // sense violation of L(G1) <= L(G2)
  bool inclusion12=true;

  // push all combinations of initial states on todo stack
  FD_DF("IsBuechiRelativelyClosed(): Product composition A");
  for (lit1 = rGenCand.InitStatesBegin(); 
      lit1 != rGenCand.InitStatesEnd(); ++lit1) {
    for (lit2 = rGenPlant.InitStatesBegin(); 
        lit2 != rGenPlant.InitStatesEnd(); ++lit2) {
      currentstates = std::make_pair(*lit1, *lit2);
      todo.push(currentstates);
      tmpstate = product.InsInitState();
      revmap[currentstates] = tmpstate;
      FD_DF("IsBuechiRelativelyClosed(): Product composition A:   (" << *lit1 << "|" << *lit2 << ") -> " 
          << revmap[currentstates]);
    }
  }

  // start algorithm
  while (! todo.empty() && inclusion12) {
    // allow for user interrupt
    LoopCallback();
    // get next reachable state from todo stack
    currentstates = todo.top();
    todo.pop();
    FD_DF("IsBuechiRelativelyClosed(): Product composition B: (" << currentstates.first << "|" 
        << currentstates.second << ") -> " << revmap[currentstates]);
    // iterate over all rGenCand transitions
    tit1 = rGenCand.TransRelBegin(currentstates.first);
    tit1_end = rGenCand.TransRelEnd(currentstates.first);
    tit2 = rGenPlant.TransRelBegin(currentstates.second);
    tit2_end = rGenPlant.TransRelEnd(currentstates.second);
    Idx curev1=0;
    bool resolved12=true;
    while((tit1 != tit1_end) && (tit2 != tit2_end)) {
      // sense new event
      if(tit1->Ev != curev1) {
        if(!resolved12) inclusion12=false;
	curev1=tit1->Ev;
        resolved12=false;
      }
      // shared event
      if (tit1->Ev == tit2->Ev) {
        resolved12=true;
        newstates = std::make_pair(tit1->X2, tit2->X2);
        // add to todo list if composition state is new
        rcit = revmap.find(newstates);
        if (rcit == revmap.end()) {
          todo.push(newstates);
          tmpstate = product.InsState();
          revmap[newstates] = tmpstate;
          FD_DF("IsBuechiRelativelyClosed(): Product composition C: (" << newstates.first << "|" 
             << newstates.second << ") -> " << revmap[newstates]);
        }
        else {
          tmpstate = rcit->second;
        }
        product.SetTransition(revmap[currentstates], tit1->Ev, tmpstate);
        ++tit1;
        ++tit2;
      }
      // try resync tit1
      else if (tit1->Ev < tit2->Ev) {
        ++tit1;
      }
      // try resync tit2
      else if (tit1->Ev > tit2->Ev) {
        ++tit2;
      }
    }
    // last event was not resolved in the product
    if(!resolved12) inclusion12=false;
  }
  // report
#ifdef FAUDES_DEBUG_FUNCTION
  FD_DF("IsBuechiRelativelyClosed(): Product: done"); 
  if(!inclusion12) {
    FD_DF("IsBuechiRelativelyClosed(): Product: inclusion L(G1) <= L(G2) not satisfied"); 
  }
#endif

  // bail out
  if(!inclusion12) return false;

  // retrieve marking from reverse composition map
  std::map< std::pair<Idx,Idx>, Idx>::iterator rit;
  for(rit=revmap.begin(); rit!=revmap.end(); ++rit){
    if(rGenCand.ExistsMarkedState(rit->first.first)) mark1.Insert(rit->second);
    if(rGenPlant.ExistsMarkedState(rit->first.second)) mark2.Insert(rit->second);
  }

  // find all relevant SCCs 1
  SccFilter umfilter12(SccFilter::FmIgnoreTrivial | SccFilter::FmStatesAvoid| SccFilter::FmStatesRequire, 
    mark1,mark2);
  std::list<StateSet> umsccs12;
  StateSet umroots12;
  ComputeScc(product,umfilter12,umsccs12,umroots12); 

  // report
  std::list<StateSet>::iterator ssit=umsccs12.begin();
  for(;ssit!=umsccs12.end(); ++ssit) {
    FD_DF("IsBuechiRelativelyClosed(): G2-marked scc without G1-mark: " << ssit->ToString());
  }  

  // result is false if we found problematic SCCs to exist
  if(umsccs12.size()!=0) return false;

  // find all relevant SCCs 2
  SccFilter umfilter21(SccFilter::FmIgnoreTrivial | SccFilter::FmStatesAvoid| SccFilter::FmStatesRequire, 
    mark2,mark1);
  std::list<StateSet> umsccs21;
  StateSet umroots21;
  ComputeScc(product,umfilter21,umsccs21,umroots21); 

  // report
  ssit=umsccs21.begin();
  for(;ssit!=umsccs21.end(); ++ssit) {
    FD_DF("IsBuechiRelativelyClosed(): G1-marked scc without G2-mark: " << ssit->ToString());
  }  

  // result is false if we found problematic SCCs to exist
  if(umsccs21.size()!=0) return false;

  // done, all tests passed
  FD_DF("IsBuechiRelativelyClosed(): pass");
  return true;
}



} // namespace faudes

