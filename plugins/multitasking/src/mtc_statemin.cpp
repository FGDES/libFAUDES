/** @file mtc_statemin.cpp

State space minimization

*/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008  Matthias Singer
   Copyright (C) 2006  Bernd Opitz
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

#include "mtc_statemin.h"  

namespace faudes {

// mtcStateMin(rGen, rResGen)
void mtcStateMin(MtcSystem& rGen, MtcSystem& rResGen) {
  std::vector<StateSet> subsets;
  std::vector<Idx> newindices;
  mtcStateMin(rGen, rResGen, subsets, newindices);
}

// mtcStateMin(rGen, rResGen, rSubSets, rNewIndices)
void mtcStateMin(MtcSystem& rGen, MtcSystem& rResGen,
    std::vector<StateSet>& rSubsets, std::vector<Idx>& rNewIndices) {
  FD_DF("statemin: *** computing state space minimization of multi-tasking generator " 
    << &rGen << " ***");
  FD_DF("statemin: making generator accessible");
#ifdef FAUDES_DEBUG_FUNCTION
  FD_WARN("mtcStateMin: debug out")
  rGen.Write("tmp_smin_m.gen");
#endif
  // make accessible
  rGen.Accessible();
#ifdef FAUDES_DEBUG_FUNCTION
  FD_WARN("mtcStateMin: debug out")
  rGen.Write("tmp_smin_a.gen");
#endif


  if (rGen.Size() < 2) {
    FD_DF("statemin: generator size < 2. returning given generator");
    //rGen.Copy(rResGen); changed !! test
    rResGen = rGen;
    rSubsets.push_back(rGen.States() );
    rNewIndices.push_back(*( rResGen.States().Begin() ) );
    return;
  }

  // ensure generator is deterministic
  if (! rGen.IsDeterministic()) {
    throw Exception("mtcStateMin", "input automaton nondeterministic", 505);
  }

  // prepare result
  rResGen.Clear();
  rResGen.Name(rGen.Name()+" [mtc minstate]");

  rResGen.ClearStateAttributes();
  rResGen.InjectAlphabet(rGen.Alphabet());

  rResGen.SetControllable(rGen.ControllableEvents());
  rResGen.SetForcible(rGen.ForcibleEvents());
  rResGen.ClrObservable(rGen.UnobservableEvents());


  // blocks B[i]
  std::vector<StateSet>& b = rSubsets; // convenience notation "b"
  Idx i, j;
  // set of active b (iterators)
  std::set<Idx> active;
  std::set<Idx>::iterator ait;
  // reverse transrel
  TransSetEvX2X1 rtransrel;
  rGen.TransRel(rtransrel);
  TransSetEvX2X1::Iterator rtit, rtit_end;
  // other stuff
  StateSet::Iterator lit; 
  TransSet::Iterator tit;

  // set up b "B[i]"
  i = 0;

#ifdef FAUDES_DEBUG_FUNCTION
  FD_WARN("mtcStateMin: debug out: states");
  rGen.States().Write();
  FD_WARN("mtcStateMin: debug out: colored states");
  rGen.ColoredStates().Write();
  FD_WARN("mtcStateMin: debug out: unolored states");
  rGen.UncoloredStates().Write();
#endif

  if (rGen.UncoloredStates().Size() > 0) {
    b.push_back(rGen.UncoloredStates());
    active.insert(i++);
    FD_DF("statemin: new block B[" << i-1 << "] = {" << 
      (rGen.UncoloredStates()).ToString() << "}");
  }
  // find states that have the same associated colors
  std::map<Idx,ColorSet> stateColorMap = rGen.StateColorMap();
  std::map<Idx,ColorSet>::const_iterator sit_end = stateColorMap.end();
  std::map<ColorSet,StateSet> SameColors;
  std::map<ColorSet,StateSet>::iterator csit;
  std::map<Idx,ColorSet>::const_iterator sit;

  for(sit = stateColorMap.begin(); sit != sit_end; ++sit){
    csit = SameColors.find(sit->second);
    FD_DF("stateColorMap, state: " << ToStringInteger(sit->first) << ", colors: " << (sit->second).ToString());
    if(csit != SameColors.end() )
        csit->second.Insert(sit->first);
    else{
        StateSet NewStateSet;
        NewStateSet.Insert(sit->first);
        SameColors[sit->second] = NewStateSet;
    }
  }

  // create the initial set of equivalence classes
  std::map<ColorSet,StateSet>::const_iterator csit_end = SameColors.end();
  for(csit = SameColors.begin(); csit != csit_end; ++csit){
      b.push_back(csit->second); // Colored StateSets
      active.insert(i++);
      FD_DF("statemin: new block B[" << i-1 << "] = {" << 
            csit->second.ToString() << "}");
  }

  // while there is an active block B
  while (! active.empty()) {
    FD_WP("mtcStateMin: blocks/active:   " << b.size() << " / " << active.size());
#ifdef FD_DF
    FD_DF("statemin: if there is an active block B...");
    std::set<Idx>::iterator _it1;
    std::stringstream str;
    for (_it1 = active.begin(); _it1 != active.end(); ++_it1) {
      str << *_it1 << " ";
    }
    FD_DF("active: "+str.str());
    std::vector<StateSet>::iterator _it2;
    str.clear();
    for (_it2 = b.begin(); _it2 != b.end(); ++_it2) {
      str << "{" << _it2->ToString() << "} ";
    }
    str << std::endl;
    FD_DF("B: "+str.str());
#endif
    // current block B[i]
    i = *(active.begin());
    // inactivate B[i]
    active.erase(active.begin());
    FD_DF("statemin: getting active block B[" << i << "] = {" <<
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
      for (lit = b_current.Begin(); lit != b_current.End(); ++lit) {
        // find predecessor states by current ev + x2
        rtit = rtransrel.BeginByEvX2(*eit, *lit);
        rtit_end = rtransrel.EndByEvX2(*eit, *lit);
        for (; rtit != rtit_end; ++rtit) {
          c.Insert(rtit->X1);
        }
      }
      // if predecessor states where found
      if (! c.Empty()) {
        // search for new block on next event or coming end of rtransrel with
        // x1 as *lit
        FD_DF("statemin: computed predecessor states C = {" << c.ToString() 
            << "} for event " << rGen.EventName(*eit));

        // foreach block D 
        for (j=0; j < b.size(); ++j) {
          FD_DF("statemin: examining block B[" << j << "] = {" << 
              b.at(j).ToString() << "}");
          // compute D' = D intersection C
          StateSet d_ = b.at(j) * c;
          // compute D'' = D - D'; remove D, add D''
          StateSet d__ = b.at(j) - d_;
          // check D split by B
          if (d_.Empty() || d__.Empty()) {
            FD_DF("statemin: -> no split");  
            continue;
          }
          FD_DF("statemin: -> split:");  
          b[j] = d_;
          FD_DF("statemin: new block B[" << j << "] = {" 
              << d_.ToString() << "}");
          b.push_back(d__);
          FD_DF("statemin: new block B[" << b.size()-1 << "] = {"
              << d__.ToString() << "}");
          // if D was active
          if (active.count(j) > 0) {
            // then mark both D', D'' active
            active.insert((Idx)b.size()- 1);
            FD_DF("statemin: mark active: " << b.size()-1);
          }
          // else mark smaller of D', D'' active
          else {
            if (d_.Size() < d__.Size()) {
              active.insert(j);
              FD_DF("statemin: mark active: " << j);
            }
            else {
              active.insert((Idx)b.size()-1);
              FD_DF("statemin: mark active: " << b.size()-1);
            }
          }
        } // foreach block D
        c.Clear();
      }
    }
  }

  FD_DF("statemin: *** building minimized generator ***");
  // build minimized generator
  std::map<Idx,Idx> minstatemap;
  Idx newstate;
  // loop over all blocks B
  for (i = 0; i < b.size(); i++) {
    // create state in new generator for every block
    newstate = rResGen.InsState();
    rNewIndices.push_back(newstate);
    FD_DF("statemin: block {" << b.at(i).ToString() 
        << "} -> new state " << newstate);
    std::ostringstream ostr; 
    Idx count = 0;
    for (lit = b.at(i).Begin(); lit != b.at(i).End(); lit++) {
      count++;
      // set minstatemap entry for every state in gen
      minstatemap[*lit] = newstate;
      if (rResGen.StateNamesEnabled()) {
        FD_DF("StateNamesEnabled: index: \""<< ToStringInteger(*lit) << "\", name: \"" << rGen.StateName(*lit) << "\"");
        if (rGen.StateName(*lit) == "") {
          ostr << ToStringInteger(*lit) << ",";
          FD_DF("StateName \"\", ostr =  " << ostr.str());
        }
        else {
          ostr << rGen.StateName(*lit) << ",";
          FD_DF("StateName exists, ostr =  " << ostr.str());
        }
      }
      // set istates
      if (rGen.ExistsInitState(*lit)) {
        rResGen.SetInitState(newstate);
        FD_DF("statemin: -> initial state");
      }
      if(count == 1){ //all states in b.at(i) have the same colors
        std::map<Idx,ColorSet>::const_iterator col_it;
        col_it = stateColorMap.find(*lit);
        // are there any colors to set for the current state?
        if (col_it != stateColorMap.end()) {
          // insert corresponding colors if they are not already set for the current state
          FD_DF("mtcStateMin: insert colors " << col_it->second.ToString() << " to state " << newstate);
          rResGen.InsColors(newstate, col_it->second);
        }
      }
    }
    if (rResGen.StateNamesEnabled()) {
      std::string statename = ostr.str();
      statename.erase(statename.length()-1);
      statename = "{" + statename + "}";
      FD_DF("Set state name: name: \"" << statename << "\", index: \"" << newstate << "\"");
      rResGen.StateName(newstate, statename);
    }
  }
  // create transition relation
  for (tit = rGen.TransRelBegin(); tit != rGen.TransRelEnd(); ++tit) {
    rResGen.SetTransition(minstatemap[tit->X1], tit->Ev, minstatemap[tit->X2]);
    FD_DF("statemin: adding transition: " 
        << minstatemap[tit->X1] << "-" << tit->Ev << "-" 
        << minstatemap[tit->X2]);
  }
#ifdef FAUDES_DEBUG_FUNCTION
  FD_WARN("mtcStateMin: debug out: result")
  rResGen.Write("tmp_smin_r.gen");
#endif
}

// RTI wrapper
void mtcStateMin(const MtcSystem& rGen, MtcSystem& rResGen) {
  MtcSystem gen(rGen);
  mtcStateMin(gen,rResGen);

  FD_WARN("mtcStateMin: debug out")
  rGen.GraphWrite("tmp_smin_w.png");
  rGen.DotWrite("tmp_smin_w.dot");

}

} // namespace faudes
