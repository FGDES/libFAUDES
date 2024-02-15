/** @file mtc_parallel.cpp

Methods for parallel composition of multitasking generators

*/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008  Matthias Singer
   Copyright (C) 2007  Thomas Moor
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


#include "mtc_parallel.h"

namespace faudes {

void mtcParallel(const MtcSystem& rGen1, const MtcSystem& rGen2, MtcSystem& rResGen) {
  // inputs have to agree on controllability of shared events:
#ifdef FAUDES_CHECKED
  EventSet inconsistent = (rGen1.ControllableEvents()*rGen2.UncontrollableEvents())
      +(rGen1.UncontrollableEvents()*rGen2.ControllableEvents());
  if(!inconsistent.Empty()) {
    std::stringstream errstr;
    errstr << "mtcParallel: inconsistent controllability flags";
    throw Exception("mtcParallel::inconsistent controllability flags", errstr.str(), 500);
  }
#endif
  // prepare result
  rResGen.Clear();

  std::map< std::pair<Idx,Idx>, Idx> rcmap;

  // make parallel composition of inputs
  mtcParallel(rGen1, rGen2, rcmap, rResGen);

  // copy controllability of input alphabets (TODO: copy all event attributes as in a Parallel)
  rResGen.SetControllable(rGen1.ControllableEvents());
  rResGen.SetControllable(rGen2.ControllableEvents());
}


// Parallel(rGen1, rGen2, rReverseCompositionMap, res)
void mtcParallel(const MtcSystem& rGen1, const MtcSystem& rGen2,
    std::map< std::pair<Idx,Idx>, Idx>& rReverseCompositionMap, 
    MtcSystem& rResGen) {
  FD_DF("mtcParallel(" << &rGen1 << "," << &rGen2 << ")");
  // prepare result
  rResGen.Clear();
  rResGen.Name(rGen1.Name()+"||"+rGen2.Name());
  // rGen1.events() \ rGen2.events()
  EventSet sharedalphabet = rGen1.Alphabet() * rGen2.Alphabet();
  FD_DF("mtcParallel: shared events: " << sharedalphabet.ToString());

  // create res alphabet
  EventSet::Iterator eit;
  for (eit = rGen1.AlphabetBegin(); eit != rGen1.AlphabetEnd(); ++eit) {
    rResGen.InsEvent(*eit);
  }
  for (eit = rGen2.AlphabetBegin(); eit != rGen2.AlphabetEnd(); ++eit) {
    rResGen.InsEvent(*eit);
  }
  FD_DF("mtcParallel: inserted indices in rResGen.alphabet( "
      << rResGen.AlphabetToString() << ")");

  // todo stack
  std::stack< std::pair<Idx,Idx> > todo;
  // actual pair, new pair
  std::pair<Idx,Idx> currentstates, newstates;
  // state
  Idx tmpstate;

  StateSet::Iterator lit1, lit2;
  TransSet::Iterator tit1, tit1_end, tit2, tit2_end;
  std::map< std::pair<Idx,Idx>, Idx>::iterator rcit;

  ColorSet colors1, colors2, composedSet;
  rGen1.Colors(colors1);
  rGen2.Colors(colors2);

  // push all combinations of initial states on todo stack
  FD_DF("mtcParallel: adding all combinations of initial states to todo:");
  for (lit1 = rGen1.InitStatesBegin(); lit1 != rGen1.InitStatesEnd(); ++lit1) {
    for (lit2 = rGen2.InitStatesBegin(); lit2 != rGen2.InitStatesEnd(); ++lit2) {
      currentstates = std::make_pair(*lit1, *lit2);
      todo.push(currentstates);
      tmpstate = rReverseCompositionMap[currentstates] = rResGen.InsInitState();
      ComposedColorSet(rGen1, *lit1, colors1, rGen2, *lit2, colors2, composedSet);
      rResGen.InsColors(tmpstate, composedSet);
      FD_DF("mtcParallel:   (" << *lit1 << "|" << *lit2 << ") -> " 
          << rReverseCompositionMap[currentstates]);
    }
  }

  // start algorithm
  FD_DF("mtcParallel: processing reachable states:");
  while (! todo.empty()) {
    // get next reachable state from todo stack
    currentstates = todo.top();
    todo.pop();
    FD_DF("mtcParallel: processing (" << currentstates.first << "|" 
        << currentstates.second << ") -> " 
        << rReverseCompositionMap[currentstates]);
    // iterate over all rGen1 transitions 
    // (includes execution of shared events)
    tit1 = rGen1.TransRelBegin(currentstates.first);
    tit1_end = rGen1.TransRelEnd(currentstates.first);
    for (; tit1 != tit1_end; ++tit1) {
      // if event not shared
      if (! sharedalphabet.Exists(tit1->Ev)) {
        FD_DF("mtcParallel:   exists only in rGen1");
        newstates = std::make_pair(tit1->X2, currentstates.second);
        // add to todo list if composition state is new
        rcit = rReverseCompositionMap.find(newstates);
        if (rcit == rReverseCompositionMap.end()) {
          todo.push(newstates);
          tmpstate = rResGen.InsState();
          ComposedColorSet(rGen1, tit1->X2, colors1, rGen2, currentstates.second, colors2, composedSet);
          rResGen.InsColors(tmpstate, composedSet);
          rReverseCompositionMap[newstates] = tmpstate;
          FD_DF("mtcParallel:   todo push: (" << newstates.first << "|" 
              << newstates.second << ") -> " 
              << rReverseCompositionMap[newstates]);
        }
        else {
          tmpstate = rcit->second;
        }
        rResGen.SetTransition(rReverseCompositionMap[currentstates], tit1->Ev, 
            tmpstate);
        FD_DF("mtcParallel:   add transition to new generator: " 
            << rReverseCompositionMap[currentstates] << "-" << tit1->Ev << "-" 
            << tmpstate);
      }
      // if shared event
      else {
        FD_DF("mtcParallel:   common event");
        // find shared transitions
        tit2 = rGen2.TransRelBegin(currentstates.second, tit1->Ev);
        tit2_end = rGen2.TransRelEnd(currentstates.second, tit1->Ev);
        for (; tit2 != tit2_end; ++tit2) {
          newstates = std::make_pair(tit1->X2, tit2->X2);
          // add to todo list if composition state is new
          rcit = rReverseCompositionMap.find(newstates);
          if (rcit == rReverseCompositionMap.end()) {
            todo.push(newstates);
            tmpstate = rResGen.InsState();
            ComposedColorSet(rGen1, tit1->X2, colors1, rGen2, tit2->X2, colors2, composedSet);
            rResGen.InsColors(tmpstate, composedSet);
            rReverseCompositionMap[newstates] = tmpstate;
            FD_DF("mtcParallel:   todo push: (" << newstates.first << "|" 
                << newstates.second << ") -> " 
                << rReverseCompositionMap[newstates]);
          }
          else {
            tmpstate = rcit->second;
          }
          rResGen.SetTransition(rReverseCompositionMap[currentstates], 
              tit1->Ev, tmpstate);
          FD_DF("mtcParallel:   add transition to new generator: " 
              << rReverseCompositionMap[currentstates] << "-" 
              << tit1->Ev << "-" << tmpstate);
        }
      }
    }
    // iterate over all rGen2 transitions 
    // (without execution of shared events)
    tit2 = rGen2.TransRelBegin(currentstates.second);
    tit2_end = rGen2.TransRelEnd(currentstates.second);
    for (; tit2 != tit2_end; ++tit2) {
      if (! sharedalphabet.Exists(tit2->Ev)) {
        FD_DF("mtcParallel:   exists only in rGen2");
        newstates = std::make_pair(currentstates.first, tit2->X2);
        // add to todo list if composition state is new
        rcit = rReverseCompositionMap.find(newstates);
        if (rcit == rReverseCompositionMap.end()) {
          todo.push(newstates);
          tmpstate = rResGen.InsState();
          ComposedColorSet(rGen1, currentstates.first, colors1, rGen2, tit2->X2, colors2, composedSet);
          rResGen.InsColors(tmpstate, composedSet);
          rReverseCompositionMap[newstates] = tmpstate;
          FD_DF("mtcParallel:   todo push: (" << newstates.first << "|" 
              << newstates.second << ") -> " 
              << rReverseCompositionMap[newstates]);
        }
        else {
          tmpstate = rcit->second;
        }
        rResGen.SetTransition(rReverseCompositionMap[currentstates], 
            tit2->Ev, tmpstate);
        FD_DF("mtcParallel:   add transition to new generator: " 
            << rReverseCompositionMap[currentstates] << "-" 
            << tit2->Ev << "-" << tmpstate);
      }
    }
  }
}

  // compose colors
void ComposedColorSet(const MtcSystem& rGen1, const Idx sidx1, ColorSet& rColors1,
                      const MtcSystem& rGen2, const Idx sidx2, ColorSet& rColors2,
                      ColorSet& composedSet) {

  composedSet.Clear();

  AttributeColoredState attr1, attr2;
  ColorSet::Iterator cit;

#ifdef FAUDES_CHECKED
  try {
    attr1 = rGen1.States().Attribute(sidx1);
  }
  catch (faudes::Exception& exception){
    std::stringstream errstr;
    errstr << "Index " << sidx1 << " not member of set" << std::endl;
    throw Exception("mtcparallel: ComposedColorSet(rGen1, sidx1, rColors1, rGen2, sidx2, rColors2, composedSet)",
      errstr.str(), 200);
  }
  try {
    attr2 = rGen2.States().Attribute(sidx2);
  }
  catch (faudes::Exception& exception){
    std::stringstream errstr;
    errstr << "Index " << sidx2 << " not member of set" << std::endl;
    throw Exception("mtcparallel: ComposedColorSet(rGen1, sidx1, rColors1, rGen2, sidx2, rColors2, composedSet)",
      errstr.str(), 200);
  }
#else
  attr1 = rGen1.States().Attribute(sidx1);
  attr2 = rGen2.States().Attribute(sidx2);
#endif

  if(!attr1.IsDefault()) {
    // iterate over colors in current state of generator 1
    for (cit = attr1.ColorsBegin(); cit != attr1.ColorsEnd(); cit++) {
#ifdef FAUDES_CHECKED
      if (!rColors1.Exists(*cit))
        throw Exception("mtcparallel: ComposedColorSet(rGen1, sidx1, rColors1, rGen2, sidx2, rColors2, rGenRes, sidxRes)",
          "Color index \""+ToStringInteger(*cit)+"\" not found in generator's color set rColors1", 201);
#endif
      if(rColors2.Exists(*cit)) {
        // color also exists in second generator
        if(!attr2.IsDefault() && attr2.Colors().Exists(*cit)) {
          // current color exists in second generator's current state
          // -> insert into composedSet
          composedSet.Insert(*cit);
        }
      }
      else {
        // current color does not exist in generator 2
        // -> insert into composedSet
        composedSet.Insert(*cit);
      }
    }
  }

  if(!attr2.IsDefault()) {
    // iterate over second generator's colors
    for (cit = attr2.ColorsBegin(); cit != attr2.ColorsEnd(); cit++) {
#ifdef FAUDES_CHECKED
      if (!rColors2.Exists(*cit))
        throw Exception("mtcparallel: ComposedColorSet(rGen1, sidx1, rColors1, rGen2, sidx2, rColors2, rGenRes, sidxRes)",
          "Color index \""+ToStringInteger(*cit)+"\" not found in generator's color set rColors2", 201);
#endif
      if (!rColors1.Exists(*cit)) {
        // color does not exist in generator 1
        // -> insert into composedSet
        composedSet.Insert(*cit);
      }
    }
  }
}

} // namespace faudes
