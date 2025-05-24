/** @file omg_buechictrl.cpp Supremal controllable sublanguage w.r.t. Buechi acceptance */

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
   

//#define FAUDES_DEBUG_FUNCTION

#include "omg_include.h"
#include "syn_include.h"


namespace faudes {


/*
***************************************************************************************
***************************************************************************************
 Implementation IsBuechiControllabe
***************************************************************************************
***************************************************************************************
*/

// IsBuechiControllable()
bool IsBuechiControllable(
  const Generator& rGenPlant, 
  const EventSet&  rCAlph, 
  const Generator& rGenCand) 
{
  FD_DF("IsBuechiControllable(\"" <<  rGenPlant.Name() << "\", \"" << rGenCand.Name() << "\")");

  // alphabets must match
  if ( rGenPlant.Alphabet() != rGenCand.Alphabet()) {
    std::stringstream errstr;
    errstr << "Alphabets of generators do not match.";
    throw Exception("IsBuechiControllable(..)", errstr.str(), 100);
  }

#ifdef FAUDES_CHECKED
  // generators are meant to be nonblocking
  if( !IsBuechiTrim(rGenCand) ) {
    std::stringstream errstr;
    errstr << "Argument \"" << rGenCand.Name() << "\" is not omega-trim.";
    throw Exception("IsBuechiControllable(..)", errstr.str(), 201);
  }
  if( !IsBuechiTrim(rGenPlant) ) {
    std::stringstream errstr;
    errstr << "Argument \"" << rGenPlant.Name() << "\" is not omega-trim.";
    throw Exception("IsBuechiControllable(..)", errstr.str(), 201);
  }
#endif

  // the trivial case: empty cand is fine
  // (we must treat this case because empty generators are not regarded deterministic)
  if(rGenCand.Empty()) {
    FD_DF("IsBuechiControllable(..): empty candidate, pass");
    return true;
  }

  // the trivial case: empty plant is fails
  // (we must treat this case because empty generators are not regarded deterministic)
  if(rGenPlant.Empty()) {
    FD_DF("IsBuechiControllable(..): empty plant, fail");
    return false;
  }

#ifdef FAUDES_CHECKED
  // generators are meant to be deterministic
  if ( !IsDeterministic(rGenCand) ||  !IsDeterministic(rGenPlant)) {
    std::stringstream errstr;
    errstr << "Arguments are expected to be deterministic.";
    throw Exception("IsBuechiControllable", errstr.str(), 202);
  }
#endif

  // test controllability
  StateSet dummy;
  if(!IsControllableUnchecked(rGenPlant,rCAlph,rGenCand,dummy)) return false;

  // test relative closedness
  if(!IsBuechiRelativelyClosedUnchecked(rGenPlant,rGenCand)) return false;

  // pass
  FD_DF("IsBuechiControllable(...): passed");
  return true;
}


// IsBuechiControllable() wrapper
bool IsBuechiControllable(
  const System& rPlantGen, 
  const Generator& rCandGen) 
{
  return IsBuechiControllable(rPlantGen,rPlantGen.ControllableEvents(),rCandGen);
}

/*
***************************************************************************************
***************************************************************************************
 Computation of the controllable prefix / enforcing omega-controlled liveness
***************************************************************************************
***************************************************************************************
*/



/*
Initial libFAUDES implementation of omega-controlled liveness, tmoor 2011, last used 
for libFAUDES 2.22p. The function retsricts the candidate to states for which a
prefix-closed controller exists such that in closed-loop configuration the acceptance condition
is implied by the plant acceptance condition. This corresponds to the controllable prefix,
originally proposed by J. Thistle et al. The actual implementation, however, is based on a sufficient 
but not necessary test. It therefore identifies a subset of the controllable prefix, which may 
turn out empty even when a prefix-closed controller exists. The below code is obsolete and will 
be removed eventually.
*/

/*

// ControlledBuechiLiveness(...)
bool ControlledBuechiLiveness(
  Generator& rSupCandGen, 
  const EventSet& rCAlph,
  StateSet& rMarkedPlantStates, 
  StateSet& rMarkedSpecStates)
{
  FD_DF("ControlledBuechiLiveness(...)");

  // set of critical states
  StateSet critical;
   
  // return true if parallelcomp contains no initial states
  if(rSupCandGen.InitStatesEmpty()) {
    return true;
  }

#ifdef FAUDES_DEBUG_FUNCTION
  FD_DF("ControlledBuechiLiveness(): marked states: ");
  StateSet ssd= rMarkedPlantStates + rMarkedSpecStates;
  ssd.Write();
#endif

  // find un-marked sccs
  StateSet astates = rMarkedPlantStates + rMarkedSpecStates;
  SccFilter umfilter(SccFilter::FmIgnoreTrivial | SccFilter::FmStatesAvoid, astates);
  std::list<StateSet> umsccs;
  StateSet umroots;
  ComputeScc(rSupCandGen,umfilter,umsccs,umroots); 

  // report
  std::list<StateSet>::iterator ssit=umsccs.begin();
  for(;ssit!=umsccs.end(); ++ssit) {
    FD_DF("ControlledBuechiLiveness(): unmarked scc: " << ssit->ToString());
  }  

  // good-states iteration
  StateSet goodstates = rMarkedSpecStates;
  
  // drive states to good states
  while(true) {
    // LoopCallback();
    // allow for user interrupt, incl progress report
    FD_WPC(1,2,"ControlledBuechiLiveness(): iterating states"); 
    // test individual states
    FD_DF("ControlledBuechiLiveness(): iterate over states (#" << rSupCandGen.Size() << ")");
    bool found=false;

    StateSet::Iterator sit = rSupCandGen.StatesBegin();
    StateSet::Iterator sit_end = rSupCandGen.StatesEnd();
    for(; sit!=sit_end; ++sit) {
      bool fail = false;  // cbaier 20121011
      bool positive=false; // cbaier 20121011
      // goodstate anyway
      if(goodstates.Exists(*sit)) continue;
      // test transitions
      TransSet::Iterator tit = rSupCandGen.TransRelBegin(*sit);
      TransSet::Iterator tit_end = rSupCandGen.TransRelEnd(*sit);
      // no transitions at all
      if(tit==tit_end) continue;
      // loop over successors
      for(; tit!=tit_end; ++tit) {
        if(goodstates.Exists(tit->X2)) 
	{positive=true; // cbaier 20121011: added 
	continue;}
        if(rCAlph.Exists(tit->Ev)) continue;
        
      // good states survive the loop // cbaier 20121011: added
      if(tit!=tit_end) { 
	fail=true;
        break;
        }                            
      }
      
      // good states survive
      if(!fail && positive) {    // cbaier 20121011: changed
        FD_DF("ControlledBuechiLiveness(): good state " << rSupCandGen.SStr(*sit));
        goodstates.Insert(*sit);
        found=true;
      }
    }  
      
    // test individual unmarked sccs
    FD_DF("ControlledBuechiLiveness(): iterate over unmarked sccs (#" << umsccs.size() <<")");
    std::list<StateSet>::iterator ssit=umsccs.begin();
    while(ssit!=umsccs.end()) { 
      bool fail=false;
      bool positive=false;
      sit=ssit->Begin();
      sit_end=ssit->End();
      for(; sit!=sit_end; ++sit) {
        // goodstate anyway
        if(goodstates.Exists(*sit)) continue;
        // test transitions
        TransSet::Iterator tit = rSupCandGen.TransRelBegin(*sit);
        TransSet::Iterator tit_end = rSupCandGen.TransRelEnd(*sit);
        // no transitions at all
        if(tit==tit_end) continue;        
        // loop over successors
        for(; tit!=tit_end; ++tit) {
          if(goodstates.Exists(tit->X2)) { positive=true; continue;}
          if(rCAlph.Exists(tit->Ev)) continue;  // tmoor 20110202: fixed typo
          if(ssit->Exists(tit->X2)) continue;
          break;
        }
        // good states survive the loop
        if(tit!=tit_end) {
          fail=true;
          break;
        }
      }
      
      // prepare next scc iterator
      std::list<StateSet>::iterator ssitnext=ssit;
      ++ssitnext;
      bool lend= (ssitnext==umsccs.end());
      // all states passed, then they are all good
      if(!fail && positive) {
        FD_DF("ControlledBuechiLiveness(): good scc " << ssit->ToString());
        goodstates.InsertSet(*ssit);
        umsccs.erase(ssit);
        found=true;
      }
      // break on end of list
      if(lend) break;
      // progress
      ssit=ssitnext;
    }
    
    // exit
    if(!found) break;
  };

  // delete critical states
  FD_DF("ControlledBuechiLiveness(): good states: " << goodstates.ToString())
  StateSet::Iterator sit = rSupCandGen.StatesBegin();
  StateSet::Iterator sit_end = rSupCandGen.StatesEnd();
  for(; sit!=sit_end; ++sit) {
    if(!goodstates.Exists(*sit))
      rSupCandGen.DelState(*sit);
  }

  // return true
  FD_DF("ControlledBuechiLiveness(): done");
  return true;
}

*/


/*
This implementation is a direct transscript of the mu-calculus formulas stated in "Control of 
w-Automata, Church's Problem, and the Emptiness Problem for Tree w-Automata", by J. Thistle and 
W.M. Wonham, 1992, adapted for the special case of deterministic Buchi-automata. Referring to the 
referenced literature, the corresponding iteration is "essentially optimal" in terms of performance. 
In the below implementation, we unwind each individual mu/nu quantifiation literally and resist 
to apply strategic substitutions in order to obtain a reference implementation. This is suspected 
to introduce (linear) penalty for avoidable boolean operations on sets of states. In a recent study, 
Christian Wamser proposes a fine-tuned alternative implementation which will be integrated in 
libFAUDES in due course.
*/

bool ControlledBuechiLiveness(
  Generator& rSupCandGen, 
  const EventSet& rCAlph,
  const StateSet& rPlantMarking) 
{
	
  FD_DF("ControlledBuechiLiveness()");

  // declare iterate sets
  StateSet resolved, initialK, targetLstar;
  StateSet initialL, targetL;
  StateSet domainL, target1;
  StateSet target, domain, theta;
 
  // convenience
  const StateSet& full = rSupCandGen.States();
  Idx fsz=full.Size();

  // evaluate mu(resolved).nu(initialK)[ p(initialK * markedK + resolved )) ];
  // here, p(T) denotes the set of states that can be driven to enter T under liveness assumption inf-markL;
  // use std. mu-nu-iteration
  resolved.Clear(); 	  
  while(true) {
    Idx rsz = resolved.Size();
    initialK = full;
    while(true) {
      Idx iKsz = initialK.Size();

      // prepare target for P(): targetLstar = initialK * markedK + resolved 
      targetLstar = (initialK * rSupCandGen.MarkedStates()) + resolved;
      FD_DF("ControlledBuechiLiveness(): [STD] iterate resolved/targetLstar #" << rsz << "/" << targetLstar.Size());

      // evaluate p(targetLstar) = mu(initialL).[ thetaTilde(targetLstar+initialL) ] 
      // here, thetaTilde(T) denotes the set of states that can be controlled such that T is persistently
      // reachable and markL is not passed before T is entered;
      // start with initialL:=0 and iterate initialL += thetaTilde(targetLstar+initialL); 
      initialL.Clear();
      while(true) {
        Idx iLsz = initialL.Size();

        // prepare targetL=targetLstar+initialL
        targetL=targetLstar+initialL; 
        FD_DF("ControlledBuechiLiveness(): [STD] ---- iterate targetL #" << targetL.Size());

        // evaluate thetaTilde(targetL)=nu(domainL).mu(target1)[ theta(targetL+(target1-markL), domainL-markL) ];
        // here, theta(T,D) is the set of states that, by one transition, can enter T and can be controlled not to exit D+T;
        // start with domainL:=full and iterate domainL -= mu(target1)[theta(targetL+(target1-markL), domainL-markL)];
        domainL=full;
        targetL = targetLstar + initialL;
        while(true) {
          Idx dLsz = domainL.Size();
          FD_DF("ControlledBuechiLiveness(): [STD] --- iterate domainL #" << domainL.Size());
            
          // user interrupt/ progress
          FD_WPC(1,2,"ControlledBuechiLiveness(): [STD] iterating reverse dynamics"); 
   
          // prepare domain = domainL - marL
	  domain=domainL-rPlantMarking;

          // evaluate mu(target1)[ theta(targetL+(target1-markL), domain) ];
          // start with target1:=0 and iterate target+=theta(targetL+(target1-markL), domainL-markL) ]; 
          target1.Clear();
          while(true) {
            Idx t1sz = target1.Size();
       
            // prepare target= targetL+(target1-markL)
            target = targetL + (target1-rPlantMarking);

            // evaluate theta(target,domain) 
            FD_DF("ControlledBuechiLiveness(): [STD] -- evaluate theta for target/domain # " 
               << target.Size() << "/" << domain.Size());
            theta.Clear();
            StateSet::Iterator sit = full.Begin(); 
            StateSet::Iterator sit_end = full.End(); 
            for(;sit!=sit_end;++sit) {
              bool pass = false;
              bool fail = false;      
              TransSet::Iterator tit = rSupCandGen.TransRelBegin(*sit);
              TransSet::Iterator tit_end = rSupCandGen.TransRelEnd(*sit);
              for(;tit!=tit_end; ++tit) {
                if(target.Exists(tit->X2)) {pass = true; continue;}
                if(domain.Exists(tit->X2)) {continue;}
    	        if(!rCAlph.Exists(tit->Ev)){ fail = true; break;}
              }
              if(pass && !fail) {
                theta.Insert(*sit);
                FD_DF("ControlledBuechiLiveness(): [STD] theta found state " << rSupCandGen.SStr(*sit));
	      }
            } // end: theta   
  
            // mu-loop on target1
            target1.InsertSet(theta);
            if(target1.Size()==t1sz) break;
            if(target1.Size()==fsz) break;
          } // end: mu

          FD_DF("ControlledBuechiLiveness(): [STD] -- mu-target1 # " << target1.Size());
  
          // nu-loop on domainL
          domainL.RestrictSet(target1);
          if(domainL.Size()==dLsz) break;
          if(domainL.Size()==0) break;
        } // end: nu
      
        FD_DF("ControlledBuechiLiveness(): [STD] --- nu-domainL-mu-target1 # " << domainL.Size());

        // mu-loop on initialL
        initialL.InsertSet(domainL);
        if(initialL.Size()==iLsz) break;
        if(initialL.Size()==fsz) break;
      } // end: mu

      // nu-loop on initialK
      initialK.RestrictSet(initialL);
      if(initialK.Size()==iKsz) break;
      if(initialK.Size()==0) break;
    } // end: nu

    // mu-loop on resolved
    resolved.InsertSet(initialK);
    if(resolved.Size()==rsz) break;
    if(resolved.Size()==fsz) break;
  } // end: mu
 
  // restrict candidate to resolved states
  rSupCandGen.DelStates(full - resolved);

  // done
  return true;
}

	
/*
Variant of the controllable prefix to extract a particular feedback map. Again, this is 
very close to the relevant literature and meant as a reference. A more efficient 
implementation proposed by Christian Wamser will be integrated in due course.
*/

bool ControlledBuechiLiveness(
  Generator& rSupCandGen, 
  const EventSet& rCAlph,
  const StateSet& rPlantMarking, 
  std::map< Idx , EventSet>& rFeedbackMap)
{
	
  FD_WARN("ControlledBuechiLiveness()");

  // declare iterate sets
  StateSet resolved, initialK, targetLstar;
  StateSet initialL, targetL;
  StateSet domainL, target1;
  StateSet target, domain, theta;
 
  // record controls per state
  EventSet disable;   
  std::map< Idx , EventSet> controls1;
  std::map< Idx , EventSet> controls1L;
  std::map< Idx , EventSet> controls1X;

  // convenience
  const StateSet& full = rSupCandGen.States();
  Idx fsz=full.Size();

  // evaluate mu(resolved).nu(initialK)[ p(initialK * markedK + resolved )) ];
  resolved.Clear(); 	  
  while(true) {
    Idx xsz = resolved.Size();
    initialK = full;
    while(true) {
      Idx rsz = initialK.Size();
      targetLstar = (initialK * rSupCandGen.MarkedStates()) + resolved;
      FD_WARN("ControlledBuechiLiveness(): [FBM] iterate resolved/targetLstar #" << xsz << "/" << targetLstar.Size());

      // reset controls
      controls1L.clear();

      // evaluate p(targetLstar) = mu(initialL).[ thetaTilde(targetLstar+initialL) ] 
      initialL.Clear();
      while(true) {
        Idx t1Lsz = initialL.Size();
        targetL = targetLstar + initialL;
        FD_WARN("ControlledBuechiLiveness(): [FBM] ---- iterate targetL #" << targetL.Size());

        // evaluate thetaTilde(targetL)=nu(domainL).mu(target1)[ theta(targetL+(target1-markL), domainL-markL) ];
        domainL=full;
        while(true) {
          Idx dsz = domainL.Size();
	  domain=domainL-rPlantMarking;
          FD_WARN("ControlledBuechiLiveness(): [FBM] --- iterate domain #" << domain.Size());
            
          // reset controls
          controls1.clear();

          // user interrupt/ progress
          FD_WPC(1,2,"ControlledBuechiLiveness(): [FBM] iterating reverse dynamics"); 
   
          // evaluate mu(target1)[ theta(targetL+(target1-markL), domain) ];
          target1.Clear();
          while(true) {
            Idx t1sz = target1.Size();
            target = targetL + (target1-rPlantMarking);

            // evaluate theta(target,domain) 
            FD_WARN("ControlledBuechiLiveness(): [FBM] -- evaluate theta for target/domain # " 
               << target.Size() << "/" << domain.Size());
            theta.Clear();
            StateSet::Iterator sit = full.Begin(); 
            StateSet::Iterator sit_end = full.End(); 
            for(;sit!=sit_end;++sit) {
              bool pass = false;
              bool fail = false;      
              disable.Clear();
              TransSet::Iterator tit = rSupCandGen.TransRelBegin(*sit);
              TransSet::Iterator tit_end = rSupCandGen.TransRelEnd(*sit);
              for(;tit!=tit_end; ++tit) {
                if(target.Exists(tit->X2)) { pass = true; continue; }
                if(domain.Exists(tit->X2)) { continue; }
    	        if(!rCAlph.Exists(tit->Ev)){ fail = true; break; }
                disable.Insert(tit->Ev); 
              }
              if(pass && !fail) {
                theta.Insert(*sit);
                if(controls1.find(*sit)==controls1.end()) controls1[*sit]=disable;
                FD_WARN("ControlledBuechiLiveness(): [FBM] theta found state " << rSupCandGen.SStr(*sit));
                if(!disable.Empty())disable.Write();
	      }
            } // end: theta   
  
            // mu-loop on target1
            target1.InsertSet(theta);
            if(target1.Size()==t1sz) break;
            if(target1.Size()==fsz) break;
          } // end: mu

          FD_WARN("ControlledBuechiLiveness(): [FBM] -- mu-target1 # " << target1.Size());
  
          // nu-loop on domainL
          domainL.RestrictSet(target1);
          if(domainL.Size()==dsz) break;
          if(domainL.Size()==0) break;
        } // end: nu
      
        FD_WARN("ControlledBuechiLiveness(): [FBM] --- nu-domainL-mu-target1 # " << domainL.Size());

        // merge controls
	std::map< Idx , EventSet>::iterator cit=controls1.begin();
	std::map< Idx , EventSet>::iterator cit_end=controls1.end();
        for(;cit!=cit_end;++cit) {
          if(controls1L.find(cit->first)!=controls1L.end()) continue;
          controls1L[cit->first]=cit->second;
        } 
  
        // mu-loop on initialL
        initialL.InsertSet(domainL);
        if(initialL.Size()==t1Lsz) break;
        if(initialL.Size()==fsz) break;
      } // end: mu

      // nu-loop on initialK
      initialK.RestrictSet(initialL);
      if(initialK.Size()==rsz) break;
      if(initialK.Size()==0) break;
    } // end: nu

    // merge controls
    std::map< Idx , EventSet>::iterator cit=controls1L.begin();
    std::map< Idx , EventSet>::iterator cit_end=controls1L.end();
    for(;cit!=cit_end;++cit) {
      if(controls1X.find(cit->first)!=controls1X.end()) continue;
      controls1X[cit->first]=cit->second;
    } 

    // mu-loop on resolved
    resolved.InsertSet(initialK);
    if(resolved.Size()==xsz) break;
    if(resolved.Size()==fsz) break;
  } // end: mu
 
  // restrict candidate to valid restrict
  rSupCandGen.DelStates(full - resolved);

  // re-write controls as feedback map
  EventSet ucalph = rSupCandGen.Alphabet() - rCAlph;
  StateSet::Iterator sit = resolved.Begin();
  StateSet::Iterator sit_end = resolved.End();
  for(;sit!=sit_end;++sit) {
    FD_WARN("ControlledBuechiLiveness(): [FBM] controls " << rSupCandGen.SStr(*sit) << " " << controls1X[*sit].ToString());
    rFeedbackMap[*sit]= (rSupCandGen.ActiveEventSet(*sit) + ucalph) - controls1X[*sit];
  }

  // done
  return true;
}
	

	

// helper class: mergable control pattern
class MCtrlPattern {
public:
  // merge with other
  void merge(const MCtrlPattern& other) {
    disable_all.InsertSet(other.disable_all);
    enable_one.insert(other.enable_one.begin(),other.enable_one.end());
  }
  // test conflict
  bool conflict() {
    std::set< EventSet >::iterator eit =enable_one.begin();
    std::set< EventSet >::iterator eit_end =enable_one.end();
    for(; eit!=eit_end ; ++eit) 
      if( (*eit - disable_all).Empty() ) return true;
    return false;
  }
  // member variables
  EventSet disable_all;
  std::set< EventSet > enable_one;
};



/* 
Variant of a controllable prefix under partial observation. This is **experimantal**.
Based on the situation of full observation, this variant records control patterns
used to establish controlled liveness and only accepts a new state if the corresponding
pattern complies all recorded patterns that correspond to the same observation.
The result is a (possibly strict) subset of the controllable prefix for the purpose.
Theoretical background will be provided in due course.
*/

bool ControlledBuechiLiveness(
  Generator& rSupCandGen, 
  const EventSet& rCAlph,
  const StateSet& rPlantMarking, 
  std::map< Idx , Idx>& rControllerStatesMap,
  std::map< Idx , EventSet>& rFeedbackMap)
{
	
  FD_WARN("ControlledBuechiLiveness(): [POBS]: cand #" << rSupCandGen.Size());

  // debugging: interpret empty controllermap as identity
  StateSet::Iterator xit = rSupCandGen.StatesBegin(); 
  StateSet::Iterator xit_end = rSupCandGen.StatesEnd(); 
  for(;xit!=xit_end;++xit) {
    std::map< Idx , Idx >::const_iterator cxit=rControllerStatesMap.find(*xit);
    if(cxit==rControllerStatesMap.end()) rControllerStatesMap[*xit]=*xit;
  }

  // informative
  std::map< Idx , Idx>::const_iterator oit = rControllerStatesMap.begin();
  std::set< Idx > ostates;  
  for(; oit != rControllerStatesMap.end(); ++oit)
    ostates.insert(oit->second);
  FD_WARN("ControlledBuechiLiveness(): [POBS]: " << "obs #" << ostates.size());

   // declare iterate sets
  StateSet resolved, initialK, targetLstar;
  StateSet initialL, targetL;
  StateSet domainL, target1;
  StateSet target, domain, theta;
 
  // record controls per state
  EventSet disable;   
  EventSet enable;   
  std::map< Idx , MCtrlPattern> controlsT;
  std::map< Idx , MCtrlPattern> controls1;
  std::map< Idx , MCtrlPattern> controls1L;
  std::map< Idx , MCtrlPattern> controls1X;
 
  // convenience
  const StateSet& full = rSupCandGen.States();
  Idx fsz=full.Size();

  // evaluate mu(resolved).nu(initialK)[ p(initialK * markedK + resolved )) ];
  resolved.Clear(); 	  
  while(true) {
    Idx xsz = resolved.Size();
    initialK = full;
    while(true) {
      Idx rsz = initialK.Size();
      targetLstar = (initialK * rSupCandGen.MarkedStates()) + resolved;
      FD_WARN("ControlledBuechiLiveness(): [POBS] iterate resolved/targetLstar #" << xsz << "/" << targetLstar.Size());

      // reset controls
      controls1L.clear();

      // evaluate p(targetLstar) = mu(initialL).[ thetaTilde(targetLstar+initialL) ] 
      initialL.Clear();
      while(true) {
        Idx t1Lsz = initialL.Size();
        targetL = targetLstar + initialL;
        FD_WARN("ControlledBuechiLiveness(): [POBS] ---- iterate targetL #" << targetL.Size());

        // evaluate thetaTilde(targetL)=nu(domainL).mu(target1)[ theta(targetL+(target1-markL), domainL-markL) ];
        domainL=full;
        while(true) {
          Idx dsz = domainL.Size();
	  domain=domainL-rPlantMarking;
          FD_WARN("ControlledBuechiLiveness(): [POBS] --- iterate domain #" << domain.Size());
            
          // reset controls
          controls1.clear();

          // user interrupt/ progress
          FD_WPC(1,2,"ControlledBuechiLiveness(): [POBS] iterating reverse dynamics"); 
   
          // evaluate mu(target1)[ theta(targetL+(target1-markL), domain) ];
          target1.Clear();
          while(true) {
            Idx t1sz = target1.Size();
            target = targetL + (target1-rPlantMarking);

            // evaluate theta(target,domain) in three passes
            FD_WARN("ControlledBuechiLiveness(): [POBS] -- evaluate theta for target/domain # " 
               << target.Size() << "/" << domain.Size());
            theta.Clear();
            controlsT.clear();

            // pass 1: find new candidate states and acumulate required controls
            StateSet::Iterator sit = full.Begin(); 
            StateSet::Iterator sit_end = full.End(); 
            for(;sit!=sit_end;++sit) {
              Idx cx=rControllerStatesMap[*sit];
              bool pass = false;
              bool fail = false;      
              disable.Clear();
              enable.Clear();
              TransSet::Iterator tit = rSupCandGen.TransRelBegin(*sit);
              TransSet::Iterator tit_end = rSupCandGen.TransRelEnd(*sit);
              for(;tit!=tit_end; ++tit) {
                if(disable.Exists(tit->Ev)) continue;
                if(target.Exists(tit->X2)) {enable.Insert(tit->Ev); pass = true; continue;}
                if(domain.Exists(tit->X2)) {continue;}
    	        if(!rCAlph.Exists(tit->Ev)){ fail = true; break;}
                disable.Insert(tit->Ev); 
              }
              if(pass && !fail) {
                // initialize control with existing patterns
	        if(controlsT.find(cx)==controlsT.end()) {
  	          if(controls1.find(cx)!=controls1.end()) 
                    controlsT[cx].merge(controls1[cx]);
  	          if(controls1L.find(cx)!=controls1L.end()) 
                    controlsT[cx].merge(controls1L[cx]);
  	          if(controls1X.find(cx)!=controls1X.end()) 
                    controlsT[cx].merge(controls1X[cx]);
		}
                // apply additional pattern
                controlsT[cx].disable_all.InsertSet(disable);
                controlsT[cx].enable_one.insert(enable);
	      }
            } // end: theta pass 1

            // pass 2: merge new controls to controls1, reject conflicting 
	    std::map< Idx , MCtrlPattern >::iterator cpit=controlsT.begin();
	    std::map< Idx , MCtrlPattern >::iterator cpit_end=controlsT.end();
	    while(cpit!=cpit_end) {
              if(cpit->second.conflict()) {controlsT.erase(cpit++); continue;}
              controls1[cpit->first].merge(cpit->second);
	      ++cpit;
	    }

            // pass 3: evaluate theta by accumulated controls control
            sit = rSupCandGen.StatesBegin(); 
            sit_end = rSupCandGen.StatesEnd(); 
            for(;sit!=sit_end;++sit) {
              bool pass = false;
              bool fail = false;      
              Idx cx=rControllerStatesMap[*sit];
              if(controls1.find(cx)==controls1.end()) continue;
              disable=controls1[cx].disable_all;
              TransSet::Iterator tit = rSupCandGen.TransRelBegin(*sit);
              TransSet::Iterator tit_end = rSupCandGen.TransRelEnd(*sit);
              for(;tit!=tit_end; ++tit) {
                if(disable.Exists(tit->Ev)) continue;
                if(target.Exists(tit->X2)) {pass = true; continue;}
                if(domain.Exists(tit->X2)) {continue;}
    	        fail = true;
              }
              if(pass && !fail) {
                theta.Insert(*sit);
                //FD_WARN("ControlledBuechiLiveness(): [POBS] theta candidate verified " << rSupCandGen.SStr(*sit));
	      }
            } // end: theta  pass 3

            // mu-loop on target1
            target1.InsertSet(theta);
            if(target1.Size()==t1sz) break;
            if(target1.Size()==fsz) break;
          } // end: mu

          FD_WARN("ControlledBuechiLiveness(): [POBS] -- mu-target1 # " << target1.Size());
  
          // nu-loop on domainL
          domainL.RestrictSet(target1);
          if(domainL.Size()==dsz) break;
          if(domainL.Size()==0) break;
        } // end: nu
      
        FD_WARN("ControlledBuechiLiveness(): [POBS] --- nu-domainL-mu-target1 # " << domainL.Size());

        // merge controls
	std::map< Idx , MCtrlPattern>::iterator cit = controls1.begin();
	std::map< Idx , MCtrlPattern>::iterator cit_end = controls1.end();
        for(;cit!=cit_end;++cit) 
          controls1L[cit->first].merge(cit->second);
  
        // mu-loop on initialL
        initialL.InsertSet(domainL);
        if(initialL.Size()==t1Lsz) break;
        if(initialL.Size()==fsz) break;
      } // end: mu

      // nu-loop on initialK
      initialK.RestrictSet(initialL);
      if(initialK.Size()==rsz) break;
      if(initialK.Size()==0) break;
    } // end: nu

    // merge controls
    std::map< Idx , MCtrlPattern>::iterator cit = controls1L.begin();
    std::map< Idx , MCtrlPattern>::iterator cit_end = controls1L.end();
    for(;cit!=cit_end;++cit) 
      controls1X[cit->first].merge(cit->second);

    // mu-loop on resolved
    resolved.InsertSet(initialK);
    if(resolved.Size()==xsz) break;
    if(resolved.Size()==fsz) break;
  } // end: mu
 
  // restrict candidate to valid restrict
  rSupCandGen.DelStates(full - resolved);

  // debugging
  if(rSupCandGen.IsCoaccessible())
    FD_WARN("ControlledBuechiLiveness(): [POBS] ---- coaccessible ok");
  if(rSupCandGen.IsComplete())
    FD_WARN("ControlledBuechiLiveness(): [POBS] ---- complete ok");
  if(!rSupCandGen.InitStates().Empty())
    FD_WARN("ControlledBuechiLiveness(): [POBS] ---- init state ok");

  // re-write controls as feedback map (w.r.t. candidate states)
  StateSet::Iterator sit = resolved.Begin();
  StateSet::Iterator sit_end = resolved.End();
  for(;sit!=sit_end;++sit) {
    Idx cx=rControllerStatesMap[*sit];
    //FD_WARN("ControlledBuechiLiveness(): [POBS] controls at cx=" << cx << " for plant state " << rSupCandGen.SStr(*sit) << " " << controls1X[cx].disable_all.ToString());
    rFeedbackMap[*sit]= rSupCandGen.Alphabet() - controls1X[cx].disable_all;
  }

  // done
  return true;
}


/*
***************************************************************************************
***************************************************************************************
 Implementation of SupBuechiCon and friends
***************************************************************************************
***************************************************************************************
*/

// helper class: omega-product state
class OPSState {
public:
  // minimal interface
  OPSState() {};
  OPSState(const Idx& rq1, const Idx& rq2, const bool& rf) :
    q1(rq1), q2(rq2), m1required(rf) {};
  std::string Str(void) { return ToStringInteger(q1)+"|"+
      ToStringInteger(q2)+"|"+ToStringInteger(m1required); };
  // order
  bool operator < (const OPSState& other) const {
    if (q1 < other.q1) return(true);
    if (q1 > other.q1) return(false);
    if (q2 < other.q2) return(true);
    if (q2 > other.q2) return(false);
    if (m1required && !other.m1required) return(true);
    return(false);
  }
  // member variables
  Idx q1;
  Idx q2;
  bool m1required;
};


// SupBuechiConProduct(rPlantGen, rCAlph, rSpecGen, rReverseCompositionMap, rResGen)
void SupBuechiConProduct(
  const Generator& rPlantGen, 
  const EventSet& rCAlph,
  const Generator& rSpecGen,
  std::map< OPSState , Idx>& rProductCompositionMap, 
  Generator& rResGen) 
{
  FD_DF("SupBuechiConProduct(" << &rPlantGen << "," << &rSpecGen << ")");
	
  // prepare result
  rResGen.Clear();
  rResGen.InjectAlphabet(rPlantGen.Alphabet());

  // trivial case
  if (rPlantGen.InitStatesEmpty()) {
    FD_DF("SupBuechiConProduct: plant got no initial states. "
            << "parallel composition contains empty language.");
    return;
  }
  if (rSpecGen.InitStatesEmpty()) {
    FD_DF("SupBuechiConProduct: spec got no initial states. "
              << "parallel composition contains empty language.");
    return;
  }

  // todo stack
  std::stack< OPSState > todo;
  // current/next state as pair with flag
  OPSState currentp, nextp;
  // current/next state as target index
  Idx currentt, nextt;  
  TransSet::Iterator ptit, ptit_end, stit, stit_end;
  std::map< OPSState, Idx>::iterator rcit;
  // critical states
  StateSet critical;
  // disabled events
  EventSet disable;


  // push initial state on todo stack
  currentp = OPSState(*rPlantGen.InitStatesBegin(), *rSpecGen.InitStatesBegin(), true);
  todo.push(currentp);
  currentt=rResGen.InsInitState();
  rProductCompositionMap[currentp] = currentt;

  // process/feed todo stack
  FD_DF("SupBuechiConProduct: processing reachable states:");
  while(!todo.empty()) {
    // allow for user interrupt
    // LoopCallback();
    // allow for user interrupt, incl progress report
    FD_WPC(rProductCompositionMap.size(),rProductCompositionMap.size()+todo.size(),"SupBuechiConProduct(): processing"); 
    FD_DF("SupBuechiConProduct(): processing" << rProductCompositionMap.size() << " " << todo.size()); 
    // get next reachable state from todo stack
    currentp = todo.top();
    currentt = rProductCompositionMap[currentp];
    todo.pop();
    // skip critical (tmoor 201308)
    if(critical.Exists(currentt)) continue; 
    // report
    FD_DF("SupBuechiConProduct: processing (" << currentp.Str() << " -> " << currentt <<")");

    // iterate over transitions, pass1: figure whether current state becomes critical (tmoor 201308)
    ptit = rPlantGen.TransRelBegin(currentp.q1);
    ptit_end = rPlantGen.TransRelEnd(currentp.q1);
    stit = rSpecGen.TransRelBegin(currentp.q2);
    stit_end = rSpecGen.TransRelEnd(currentp.q2);
    disable.Clear();
    // process all transitions and increment iterators strategically
    while((ptit != ptit_end) && (stit != stit_end)) {
      FD_DF("SupBuechiConProduct: current plant-transition: " << rPlantGen.TStr(*ptit) );
      FD_DF("SupBuechiConProduct: current spec-transition: " << rSpecGen.TStr(*stit));
      // case A: execute common event
      if(ptit->Ev == stit->Ev) {
        nextp = OPSState(ptit->X2, stit->X2,currentp.m1required);
        if(currentp.m1required) {
  	  if(rPlantGen.ExistsMarkedState(currentp.q1)) nextp.m1required=false;
        } else {
  	  if(rSpecGen.ExistsMarkedState(currentp.q2)) nextp.m1required=true;
        }
        // figure whether successor state is known to be critical
  	rcit = rProductCompositionMap.find(nextp);
        if(rcit != rProductCompositionMap.end()) { 
	  if(critical.Exists(rcit->second)) {
 	    FD_DF("SupBuechiConParallel: common event " << rSpecGen.EStr(stit->Ev) << " to a critical states");
            // if event is uncontrollable, current state becomes critical
	    if(!rCAlph.Exists(ptit->Ev)) {
	      FD_DF("SupBuechiConProduct: critical insert" << currentt);
	      critical.Insert(currentt); // tmoor 201308
	      // exit all loops
	      ptit = ptit_end;
	      stit = stit_end;
              break;
	    } 
            // else, event is disabled 
            disable.Insert(stit->Ev);	    
	  }
	}
        // increment transition
        ++ptit;
        ++stit;
      }
      // case B: process plant event that is disabled by the specification
      else if (ptit->Ev < stit->Ev) {
	FD_DF("SupConProduct:  "  << rPlantGen.EStr(ptit->Ev) << " is enabled in the plant but disabled in the specification");
	// if the event is uncontrollable, this makes the current state critical
	if (!rCAlph.Exists(ptit->Ev)) {
	  FD_DF("SupBuechiConProduct: disabled event " << rPlantGen.EStr(ptit->Ev)  << " is uncontrollable");
	  FD_DF("SupBuechiConProduct: critical insert" << currentt);
	  critical.Insert(currentt);
	  // exit all loops
	  ptit = ptit_end;
	  stit = stit_end;
	  break;
	}
	FD_DF("SupBuechiConProduct: incrementing plant transrel");
	++ptit;
      } 
      // increment titg and titg, case C: process h event that is not enabled for g
      else {
	FD_DF("SupBuechiConProduct: incrementing spec transrel");
	++stit;
      }
    } // end while incrementing 
    // increment iterators case D: process leftover events of plant
    while (ptit != ptit_end) {
      FD_DF("SupBuechiConProduct: current g-transition: " << rPlantGen.TStr(*ptit));
      FD_DF("SupBuechiConProduct: current h-transition: end");
      // if uncontrollable event leaves candidate
      if (!rCAlph.Exists(ptit->Ev)) {
	FD_DF("SupBuechiConProduct: asynchron executed uncontrollable "
	         << "event " << rPlantGen.EStr(ptit->Ev) << " leaves specification:");
	FD_DF("SupConProduct: critical insert" << rPlantGen.SStr(currentt));
	critical.Insert(currentt);
	// exit this loop 
	break;
      }
      FD_DF("SupBuechiConProduct: incrementing g transrel");
      ++ptit;
    } // end iterating pass1

    // skip critical (tmoor 201308)
    if(critical.Exists(currentt)) continue; 

    // iterate over transitions, pass2: execute shared events (tmoor 201308)
    FD_DF("SupBuechiConProduct(): processing pass2"); 
    ptit = rPlantGen.TransRelBegin(currentp.q1);
    ptit_end = rPlantGen.TransRelEnd(currentp.q1);
    stit = rSpecGen.TransRelBegin(currentp.q2);
    stit_end = rSpecGen.TransRelEnd(currentp.q2);
    // process all transitions and increment iterators strategically
    while((ptit != ptit_end) && (stit != stit_end)) {
      FD_DF("SupBuechiConProduct: current plant-transition: " << rPlantGen.TStr(*ptit) );
      FD_DF("SupBuechiConProduct: current spec-transition: " << rSpecGen.TStr(*stit));
      // case A: execute common event
      if(ptit->Ev == stit->Ev) {
        if(!disable.Exists(stit->Ev)) { 
  	  FD_DF("SupBuechiConProduct: executing common event "  << rPlantGen.EStr(ptit->Ev));
          nextp = OPSState(ptit->X2, stit->X2,currentp.m1required);
          if(currentp.m1required) {
  	    if(rPlantGen.ExistsMarkedState(currentp.q1)) nextp.m1required=false;
          } else {
  	    if(rSpecGen.ExistsMarkedState(currentp.q2)) nextp.m1required=true;
          }
          // figure target index
	  rcit = rProductCompositionMap.find(nextp);
	  // if state is new: add to todo list and result
	  if(rcit == rProductCompositionMap.end()) {
            todo.push(nextp);
            nextt = rResGen.InsState();
            rProductCompositionMap[nextp] = nextt;
            if(!nextp.m1required) 
              if(rSpecGen.ExistsMarkedState(nextp.q2)) 
                rResGen.SetMarkedState(nextt);                   
            FD_DF("SupBuechiConProduct:   todo push: (" << nextp.Str() << ") -> " << nextt);
          } else {
            nextt = rcit->second;
          }
	  // set the transition and increment iterators
          FD_DF("SupBuechierParallel:  add transition to new generator: " << rResGen.TStr(Transition(currentt, ptit->Ev, nextt)));
          rResGen.SetTransition(currentt, ptit->Ev, nextt);
	}
        // increment transition
        ++ptit;
        ++stit;
      } // end: if processing common event 
      // case B: process plant event that is disabled by the specification
      else if (ptit->Ev < stit->Ev) {
	++ptit;
      } 
      // increment titg and titg, case C: process h event that is not enabled for g
      else {
	++stit;
      }
    } // end while incrementing for pass2

  } // while todo

  FD_DF("SupBuechiConProduct: deleting critical states: " << critical.ToString());
  rResGen.DelStates(critical);

  // restrict composition map
  std::map< OPSState , Idx>::iterator cit = rProductCompositionMap.begin();
  while(cit != rProductCompositionMap.end())
    if(!rResGen.ExistsState(cit->second)) rProductCompositionMap.erase(cit++);
    else cit++;

}




// SupBuechiConUnchecked(rPlantGen, rCAlph, rSpecGen,rResGen)
void SupBuechiConUnchecked(
  const Generator& rPlantGen,
  const EventSet& rCAlph,  
  const Generator& rSpecGen,
  StateSet& rPlantMarking,
  Generator& rResGen) 
{
  FD_DF("SupBuechiConUnchecked(\"" <<  rPlantGen.Name() << "\", \"" << rSpecGen.Name() << "\")");

  // PREPARE RESULT:	
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  pResGen->Clear();
  pResGen->InjectAlphabet(rPlantGen.Alphabet());

  // controllable events
  FD_DF("SupBuechiCon: controllable events: "   << rCAlph.ToString());

  // perform product
  std::map< OPSState , Idx> cmap; 
  SupBuechiConProduct(rPlantGen, rCAlph, rSpecGen, cmap, *pResGen);

  // figure plant marking
  rPlantMarking.Clear();
  std::map< OPSState, Idx>::iterator pcit;
  for(pcit=cmap.begin(); pcit!=cmap.end(); ++pcit) 
    if(rPlantGen.ExistsMarkedState(pcit->first.q1))
       rPlantMarking.Insert(pcit->second);

  // fix statenames (debugging)
#ifdef FAUDES_DEBUG_FUNCTION
  std::map< OPSState, Idx>::iterator dcit;
  for(dcit=cmap.begin(); dcit!=cmap.end(); ++dcit) {
    Idx x1=dcit->first.q1;
    Idx x2=dcit->first.q2;
    bool m1requ=dcit->first.m1required;
    Idx x12=dcit->second;
    if(!pResGen->ExistsState(x12)) continue;
    std::string name1= rPlantGen.StateName(x1);
    if(name1=="") name1=ToStringInteger(x1);
    std::string name2= rSpecGen.StateName(x2);
    if(name2=="") name1=ToStringInteger(x2);
    std::string name12;
    if(m1requ) name12= name1 + "|" + name2 + "|r1m";
    else name12= name1 + "|" + name2 + "|r2m";
    name12=pResGen->UniqueStateName(name12);
    pResGen->StateName(x12,name12);
  }
#endif

#ifdef FAUDES_DEBUG_FUNCTION
  pResGen->DWrite();
  pResGen->Write("tmp_syn_xxx_"+pResGen->Name()+".gen");
  pResGen->WriteStateSet(rPlantMarking);
#endif

  // iterate for required properties
  while(true) {
    // catch trivial
    if(pResGen->Empty()) break;
    // fast inner loop: prefix controllability and omega-trim
    while(true) {
      Idx count1 = pResGen->Size();
      FD_WARN("SupBuechiCon: iterate: do prefix con on #"   << pResGen->Size());
      SupConClosedUnchecked(rPlantGen, rCAlph, *pResGen);
      FD_WARN("SupBuechiCon: iterate: do coaccessible on #"   << pResGen->Size());
      pResGen->Coaccessible();
      FD_WARN("SupBuechiCon: iterate: do accessible on #"   << pResGen->Size());
      pResGen->Accessible();
      FD_WARN("SupBuechiCon: iterate: do complete on #"   << pResGen->Size());
      pResGen->Complete();
      if(pResGen->Size() == count1) break;
      if(pResGen->Size() == 0) break;
    }
    // slow outer loop: controlled liveness aka restrict to controllable prefix
    Idx count2 = pResGen->Size();
    FD_WARN("SupBuechiCon: iterate: do controlled liveness  on #"   << pResGen->Size());
    ControlledBuechiLiveness(*pResGen,rCAlph,rPlantMarking);
    if(pResGen->Size() == count2) break;    
  }

  // fix statenames ...
  std::map< OPSState, Idx>::iterator rcit;
  if(rPlantGen.StateNamesEnabled() && rSpecGen.StateNamesEnabled() && pResGen->StateNamesEnabled()) 
  for(rcit=cmap.begin(); rcit!=cmap.end(); rcit++) {
    Idx x1=rcit->first.q1;
    Idx x2=rcit->first.q2;
    bool m1requ=rcit->first.m1required;
    Idx x12=rcit->second;
    if(!pResGen->ExistsState(x12)) continue;
    std::string name1= rPlantGen.StateName(x1);
    if(name1=="") name1=ToStringInteger(x1);
    std::string name2= rSpecGen.StateName(x2);
    if(name2=="") name1=ToStringInteger(x2);
    std::string name12;
    if(m1requ) name12= name1 + "|" + name2 + "|r1m";
    else name12= name1 + "|" + name2 + "|r2m";
    name12=pResGen->UniqueStateName(name12);
    pResGen->StateName(x12,name12);
  }

  // .. or clear them (?)
  if(!(rPlantGen.StateNamesEnabled() && rSpecGen.StateNamesEnabled() && pResGen->StateNamesEnabled())) 
    pResGen->StateNamesEnabled(false);

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

  FD_WARN("SupBuechiCon: return #"   << rResGen.Size());
}


// SupConBuechiNormUnchecked(rPlantGen, rCAlph, rOAlph, rSpecGen, rFeedbackMap, rResGen)
void SupBuechiConNormUnchecked(
  const Generator& rPlantGen,
  const EventSet& rCAlph,  
  const EventSet& rOAlph,  
  const Generator& rSpecGen,
  StateSet& rPlantMarking,
  std::map< Idx , Idx >& rObserverStateMap,
  std::map< Idx , EventSet>& rFeedbackMap, 
  Generator& rResGen) 
{
  FD_WARN("SupBuechiConNorm(\"" <<  rPlantGen.Name() << "\", \"" << rSpecGen.Name() << "\")");

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  pResGen->Clear();
  pResGen->InjectAlphabet(rPlantGen.Alphabet());

  // report events
  FD_WARN("SupBuechiConNorm: controllable events: "   << rCAlph.ToString());
  FD_WARN("SupBuechiConNorm: un-observabel events: "   << (rPlantGen.Alphabet()-rOAlph).ToString());

  // perform product
  std::map< OPSState , Idx> cmap; 
  SupBuechiConProduct(rPlantGen, rCAlph, rSpecGen, cmap, *pResGen);
  BuechiTrim(*pResGen);
  
  // no statenames impülemented
  pResGen->StateNamesEnabled(false);

  // figure plant marking
  rPlantMarking.Clear();
  std::map< OPSState, Idx>::iterator pcit;
  for(pcit=cmap.begin(); pcit!=cmap.end(); ++pcit) 
    if(rPlantGen.ExistsMarkedState(pcit->first.q1))
       rPlantMarking.Insert(pcit->second);

  // extend by observer states
  Generator obsG= *pResGen;
  MarkAllStates(obsG);
  Project(obsG,rOAlph,obsG);
  InvProject(obsG,pResGen->Alphabet());
  std::map< std::pair<Idx,Idx>, Idx> rObserverCompositionMap;
  Product(*pResGen, obsG, rObserverCompositionMap, *pResGen);
  // track plantmarking
  StateSet fixmarking;
  std::map< std::pair<Idx,Idx>, Idx>::iterator cit;
  for(cit=rObserverCompositionMap.begin(); cit!=rObserverCompositionMap.end();++cit) 
     if(rPlantMarking.Exists(cit->first.first)) fixmarking.Insert(cit->second);
  rPlantMarking=fixmarking;
  // construct reverse map "pResGen.State --> obsG.State"
  std::map< std::pair<Idx,Idx>, Idx>::iterator sit;
  for(sit=rObserverCompositionMap.begin(); sit!=rObserverCompositionMap.end();++sit) 
    rObserverStateMap[sit->second]=sit->first.second;
  
#ifdef FAUDES_DEBUG_FUNCTION
  pResGen->DWrite();
  pResGen->Write("tmp_syn_xxx_"+pResGen->Name()+".gen");
  pResGen->WriteStateSet(rPlantMarking);
#endif

  FD_WARN("SupBuechiConNorm(): cand #" << pResGen->Size() << " obs #" << obsG.Size());
  // make resulting generator trim until it's fully controllable
  while(true) {
    // catch trivial
    if(pResGen->Empty()) break;
    // fast inner loop: prefix controllability and omega-trim
    while(true) {
      Idx count1 = pResGen->Size();
      FD_WARN("SupBuechiConNorm: iterate: do prefix-contr  on #"   << pResGen->Size());
      SupConClosedUnchecked(rPlantGen, rCAlph, *pResGen);
      FD_WARN("SupBuechiConNorm: iterate: do accessible on #"   << pResGen->Size());
      pResGen->Accessible();
      FD_WARN("SupBuechiConNorm: iterate: do coaccessible on #"   << pResGen->Size());
      pResGen->Coaccessible();
      FD_WARN("SupBuechiConNorm: iterate: do prefix-norm  on #"   << pResGen->Size());
      SupConNormClosedUnchecked(rPlantGen, rOAlph, rOAlph, obsG, *pResGen);
      FD_WARN("SupBuechiConNorm: iterate: do coaccessible on #"   << pResGen->Size());
      pResGen->Coaccessible();
      FD_WARN("SupBuechiConNorm: iterate: do accessible on #"   << pResGen->Size());
      pResGen->Accessible();
      FD_WARN("SupBuechiConNorm: iterate: do complete on #"   << pResGen->Size());
      pResGen->Complete();
      if(pResGen->Size() == count1) break;
      if(pResGen->Size() == 0) break;
    }
    // slow outer loop: controlled liveness
    Idx count2 = pResGen->Size();
    FD_WARN("SupBuechiConNorm: iterate: do controlled liveness  on #"   << pResGen->Size());

    // cosmetic: restrict observer map and count effective states
    std::map< Idx , Idx>::iterator oit = rObserverStateMap.begin();
    while(oit != rObserverStateMap.end())
      if(!pResGen->ExistsState(oit->first)) rObserverStateMap.erase(oit++);
      else oit++;
    std::set< Idx > ostates;  
    for(oit = rObserverStateMap.begin(); oit != rObserverStateMap.end(); ++oit)
      ostates.insert(oit->second);

    //omega controlled liveness aka controllabe prefix
    FD_WARN("SupBuechiConNorm(): cand #" << pResGen->Size() << " obs #" << ostates.size());
    std::map< Idx , EventSet> feedback;
    ControlledBuechiLiveness(*pResGen,rCAlph,rPlantMarking,rObserverStateMap,feedback);
    //ControlledBuechiLiveness(*pResGen,rCAlph,rPlantMarking);
    if(pResGen->Size() == count2) break;    
  }

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}


/*
****************************************
* SUPCON: WRAPPER / USER FUNCTIONS     *
****************************************
*/


// SupBuechiCon(rPlantGen, rCAlph, rSpecGen, rResGen)
void SupBuechiCon(
  const Generator& rPlantGen, 
  const EventSet& rCAlph, 
  const Generator& rSpecGen, 
  Generator& rResGen) 
{
  // consitenct check
  ControlProblemConsistencyCheck(rPlantGen, rCAlph, rSpecGen);  
  // execute
  StateSet plantmarking;
  SupBuechiConUnchecked(rPlantGen, rCAlph, rSpecGen, plantmarking, rResGen);
  // record name
  rResGen.Name(CollapsString("SupBuechiCon(("+rPlantGen.Name()+"),("+rSpecGen.Name()+"))"));
}


// SupBuechiCon for Systems:
// uses and maintains controllablity from plant 
void SupBuechiCon(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) {
  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  // execute 
  SupBuechiCon(rPlantGen, rPlantGen.ControllableEvents(),rSpecGen,*pResGen);
  // copy all attributes of input alphabet
  pResGen->EventAttributes(rPlantGen.Alphabet());
  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}

// BuechiCon(rPlantGen, rCAlph, rSpecGen, rResGen)
void BuechiCon(
  const Generator& rPlantGen, 
  const EventSet& rCAlph, 
  const Generator& rSpecGen, 
  Generator& rResGen) 
{

  // consitency check
  ControlProblemConsistencyCheck(rPlantGen, rCAlph, rSpecGen);  
  // compute supremal closed-loop behaviour
  StateSet plantmarking;
  SupBuechiConUnchecked(rPlantGen, rCAlph, rSpecGen, plantmarking, rResGen);
  // compute restrictive feedback
  std::map< Idx , EventSet> feedback;
  ControlledBuechiLiveness(rResGen, rCAlph, plantmarking, feedback);
  // apply restrictive feedback
  StateSet::Iterator sit = rResGen.StatesBegin();
  StateSet::Iterator sit_end = rResGen.StatesEnd();
  for(;sit!=sit_end;++sit) {
    const EventSet& pattern = feedback[*sit];
    TransSet::Iterator tit = rResGen.TransRelBegin(*sit);
    TransSet::Iterator tit_end = rResGen.TransRelEnd(*sit);
    while(tit!=tit_end) {
      if(pattern.Exists(tit->Ev)) { tit++; continue;};
      rResGen.ClrTransition(tit++);
    }
  }
  // cosmetic trim
  rResGen.Trim();

#ifdef FAUDES_CODE
  // during development: test controllability and livenes  
  if(!IsControllable(rPlantGen,rCAlph,rResGen)) {
    throw Exception("BuechiCon(..)", "ERROR: controllability test failed", 500);
  }
  if(!IsBuechiRelativelyClosed(rPlantGen,rResGen)) {
    throw Exception("BuechiCon(..)", "ERROR: rel. top. closedness test failed", 500);
  }
#endif

  // record name
  rResGen.Name(CollapsString("BuechiCon(("+rPlantGen.Name()+"),("+rSpecGen.Name()+"))"));
}

// BuechiCon for Systems:
// uses and maintains controllablity from plant 
void BuechiCon(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) {
  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  // execute 
  BuechiCon(rPlantGen, rPlantGen.ControllableEvents(),rSpecGen,*pResGen);
  // copy all attributes of input alphabet
  pResGen->EventAttributes(rPlantGen.Alphabet());
  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}


// SupBuechiConNorm(rPlantGen, rCAlph, rOAlph, rSpecGen, rResGen)
void SupBuechiConNorm(
  const Generator& rPlantGen, 
  const EventSet& rCAlph, 
  const EventSet& rOAlph, 
  const Generator& rSpecGen, 
  Generator& rResGen) 
{
  // consitenct check
  ControlProblemConsistencyCheck(rPlantGen, rCAlph, rOAlph, rSpecGen);  
  // execute
  std::map< Idx , Idx> constates;
  std::map< Idx , EventSet> feedback;
  StateSet plantmarking;
  SupBuechiConNormUnchecked(rPlantGen, rCAlph, rOAlph, rSpecGen, plantmarking, constates, feedback, rResGen);
  // record name
  rResGen.Name(CollapsString("SupBuechiConNorm(("+rPlantGen.Name()+"),("+rSpecGen.Name()+"))"));
}


// SupBuechiConNorm for Systems:
// uses and maintains controllablity from plant 
void SupBuechiConNorm(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) {
  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  // execute 
  SupBuechiConNorm(rPlantGen, rPlantGen.ControllableEvents(),rPlantGen.ObservableEvents(),rSpecGen,*pResGen);
  // copy all attributes of input alphabet
  pResGen->EventAttributes(rPlantGen.Alphabet());
  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}

// BuechiConNrm(rPlantGen, rCAlph, rOAlph, rSpecGen, rResGen)
void BuechiConNorm(
  const Generator& rPlantGen, 
  const EventSet& rCAlph, 
  const EventSet& rOAlph, 
  const Generator& rSpecGen, 
  Generator& rResGen) 
{

  // consitency check
  ControlProblemConsistencyCheck(rPlantGen, rCAlph, rOAlph, rSpecGen);  
  // compute supremal closed-loop behaviour
  StateSet plantmarking;
  std::map< Idx , Idx> cxmap;
  std::map< Idx , EventSet> feedback;
  SupBuechiConNormUnchecked(rPlantGen, rCAlph, rOAlph, rSpecGen, plantmarking, cxmap, feedback, rResGen);
  // compute restrictive feedback
  ControlledBuechiLiveness(rResGen, rCAlph, plantmarking, cxmap, feedback);
  // apply restrictive feedback
  StateSet::Iterator sit = rResGen.StatesBegin();
  StateSet::Iterator sit_end = rResGen.StatesEnd();
  for(;sit!=sit_end;++sit) {
    // Idx cx = cxmap[*sit]; etc ...
    const EventSet& pattern = feedback[*sit];
    TransSet::Iterator tit = rResGen.TransRelBegin(*sit);
    TransSet::Iterator tit_end = rResGen.TransRelEnd(*sit);
    while(tit!=tit_end) {
      if(pattern.Exists(tit->Ev)) { tit++; continue;};
      rResGen.ClrTransition(tit++);
    }
  }
  // trim (?)
  BuechiTrim(rResGen);
//#ifdef FAUDES_CODE
  // during development: test controllability, normality and livenes  
  if(!IsControllable(rPlantGen,rCAlph,rResGen)) {
    throw Exception("BuechiConNorm(..)", "ERROR: controllability test failed", 500);
  }
  if(!IsBuechiRelativelyClosed(rPlantGen,rResGen)) {
    throw Exception("BuechiConNorm(..)", "ERROR: rel. top. closedness test failed", 500);
  }
  Generator prK=rResGen;
  MarkAllStates(prK);
  Generator prL=rPlantGen;
  prL.Trim();
  MarkAllStates(prL);
  if(!IsNormal(prL,rOAlph,prK)){
    throw Exception("BuechiConNorm(..)", "ERROR: prefix normality test failed", 500);
  }
//#endif

  // record name
  rResGen.Name(CollapsString("BuechiConNorm(("+rPlantGen.Name()+"),("+rSpecGen.Name()+"))"));
}

// BuechiConNorm for Systems
// uses and maintains controllablity from plant 
void BuechiConNorm(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) {
  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  // execute 
  BuechiConNorm(rPlantGen, rPlantGen.ControllableEvents(),rPlantGen.ObservableEvents(), rSpecGen,*pResGen);
  // copy all attributes of input alphabet
  pResGen->EventAttributes(rPlantGen.Alphabet());
  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}



} // name space 
