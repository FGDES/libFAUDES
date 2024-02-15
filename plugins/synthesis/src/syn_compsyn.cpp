/** @file syn_compsyn.cpp Compositional synthesis */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2015  Hao Zhou 
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


/*
Notes:

This file is a copy edited version of the "syn_supnbcon.h", 
"syn_supnbcon.cpp", "libzhou.h" and "libzhou.cpp", originally 
written by Hao Zhou in course of his Master Thesis. The merge is
meant to minimise the public interface and to decouple side
effects of further development of libFAUDES. 

Relevant literature:

"On Compostional Approaches for Discrete Event Systems Verification and Synthesis"
Sahar Mohajerani, PhD, Sweden, 2015


*/

#include "syn_compsyn.h"
#include "syn_synthequiv.h"
#include "corefaudes.h"

namespace faudes {

/*
****************************************************************
PART A: Class "ComSyn" 

This class provides a buffer for input and output data to
iteratively apply indivual algorithms for compositional synthesis

****************************************************************
*/

class ComSyn {

 public:

  /**
   * Constructer:
   * construct the synthesis-buffer for running compositional synthesis algorithmus 
   * and initialize the map and supervisors.
   *
   * Input: 
   *   @param rPlantGenVec
   *     plant generator-vector
   *   @param rConAlph
   *     controllable events
   *   @param rSpecGenVec
   *     specification generator-vector
   *
   * Output:
   *   @param rMapEventsToPlant
   *     map the events in supervisor to plant
   *   @param rDisGenVec
   *     distinguisher generator-vector
   *   @param rSupGenVec
   *     supervisor generator-vector
   */
  ComSyn(const GeneratorVector& rPlantGenVec,
	 const EventSet& rConAlph,
	 const GeneratorVector& rSpecGenVec,
	 std::map<Idx,Idx>& rMapEventsToPlant,
	 GeneratorVector& rDisGenVec,
	 GeneratorVector& rSupGenVec);

  /**
   * Destructer
   */
  ~ComSyn(void);

  /**
   * Preprocess:
   * firstly abstract every generator in synthesis-buffer before running synthesis-algorithmus
   */
  void Preprocess(void);

  /**
   * Compositional Synthesis Algorithmus after Preprocess
   */
  void Synthesis(void);


 private:

  /**
   * global controllable events
   */
  EventSet GConAlph;

  /**
   * synthesis-buffer
   */
  GeneratorVector GenVec;

  /**
   * keep reference to event-map 
   */
  std::map<Idx,Idx>* pMapEventsToPlant;

  /**
   * keep reference to distinguisher-generator
   */
  GeneratorVector* pDisGenVec;

  /**
   * keep reference to supervisor-generator
   */
  GeneratorVector* pSupGenVec;

  /**
   * keep reference to EventSymbolTable
   */
  SymbolTable* pEvSymTab;

  /**
   * the termination-event
   */
  Idx w;

  /**
   * the union of eventset from generators but without a speicified generator
   * 
   * @param rGenVec
   *   the generators which to be extracted
   * @param WithoutMe
   *   the specified generator
   * @param rRestAlph
   *   result
   */
  void SetPartUnion(const GeneratorVector& rGenVec, 
		    GeneratorVector::Position WithoutMe,
		    EventSet& rRestAlph);
  /**
   * adjust bisimulation relation to compositional synthesis
   * 
   * @param rGen
   *   the adjusted generator 
   */
  void comsyn_ComputeBisimulation(Generator& rGen);

};


/*
****************************************************************
PART B: Subroutines for compositional synthesis algorithm

****************************************************************
*/


/**
 * translate specification into plant
 * 
 * @param rSpecGenVec
 *   specification generator-vector
 * @param rGUncAlph
 *   global uncontrollable events
 * @param rResGenVec
 *   synthesis-buffer
 */
void TransSpec(const GeneratorVector& rSpecGenVec, 
	       const EventSet& rGUncAlph,
	       GeneratorVector& rResGenVec);

/**
 * weak synthesis obeservation equivalence [not implemented]
 *
 * @param rGenOrig
 *   original generator
 * @param rConAlph
 *   controllable events
 * @param rLocAlph
 *   local events
 * @param rMapStateToPartition
 *   map state to equivalent class
 * @param rResGen
 *   the quotient automaton
 */
void ComputeWSOE(const Generator& rGenOrig,
	      const EventSet& rConAlph,
	      const EventSet& rLocAlph, 
	      std::map<Idx,Idx>& rMapStateToPartition,
	      Generator& rResGen);

/**
 * remove the events from the entire buffer which have only selfloop transitions 
 *
 * @param rGenVec
 *   synthesis-buffer
 */
void GlobalSelfloop(GeneratorVector& rGenVec);


/**
 * remove the events from a generator which have only selfloop transitions 
 *
 * @param rGen
 *   generator which is to be abstracted
 * @param rLocAlph
 *   local events
 */
void LocalSelfloop(Generator& rGen, EventSet& rLocAlph);


/**
 * make a distinguisher and a map for solving nondeterminism
 * and rewrite abstracted automaton 
 * 
 * @param AbstGen
 *   the abstracted generator 
 * @param rMapStateToPartition
 *   map state to equivalent class
 * @param OrigGen
 *   the non-abstracted generator (after halbway-synthesis bevor abstraction)
 * @param rMapOldToNew
 *   map the replaced events to new events
 */
void MakeDistinguisher(Generator& AbstGen,
		       std::map<Idx,Idx>& rMapStateToPartition, 
		       Generator& OrigGen,
		       std::map<Idx,std::vector<Idx> >& rMapOldToNew);


/**
 * update the generators in synthesis-buffer and in supervisor with new events
 *
 * @param rMapOldToNew
 *   map the replaced events to new events
 * @param rConAlph
 *   controllable events
 * @param rGenVec
 *   synthesis-buffer
 * @param rMapEventsToPlant
 *   map the events in supervisor to plant
 * @param rDisGenVec
 *   distinguisher generator-vector
 * @param rSupGenVec
 *   supervisor generator-vector
 */ 
void Splitter(const std::map<Idx, std::vector<Idx> >& rMapOldToNew,
	      EventSet& rConAlph,
	      GeneratorVector& rGenVec,
	      std::map<Idx,Idx>& rMapEventsToPlant,
	      GeneratorVector& rDisGenVec,
	      GeneratorVector& rSupGenVec);


/**
 * heuristic:
 * vorious approaches to select subsystem from synthesis-buffer then compose them
 * and remove them from buffer.
 * 1. determine subsystem
 * 2. parallel composition for subsystem
 * 3. delete subsystem from "GenVec"
 *
 * @param rGenVec
 *   synthesis-buffer
 * @param rResGen
 *   the composed new generator
 */
void SelectSubsystem_V1(GeneratorVector& rGenVec,
			Generator& rResGen);

void SelectSubsystem_V2(GeneratorVector& rGenVec,
			Generator& rResGen);

/**
 * halbway-synthesis
 *
 * @param rOrigGen
 *   the resulting composed new generator from synthesis-buffer
 * @param rConAlph
 *   controllable events
 * @param rLocAlph
 *   local events
 * @param rHSupGen
 *   the resulting generator after halbway-synthesis
 */
void ComputeHSupConNB(const Generator& rOrigGen,
		   const EventSet& rConAlph,
		   const EventSet& rLocAlph,
		   Generator& rHSupGen);



/*
****************************************************************
PART C: implementation of all of the above

****************************************************************
*/


// Constructor 
ComSyn::ComSyn(const GeneratorVector& rPlantGenVec,
	       const EventSet& rConAlph,
	       const GeneratorVector& rSpecGenVec,
	       std::map<Idx,Idx>& rMapEventsToPlant,
	       GeneratorVector& rDisGenVec,
	       GeneratorVector& rSupGenVec)
{
  // write reference to output
  pMapEventsToPlant = &rMapEventsToPlant;
  pDisGenVec = &rDisGenVec;
  pSupGenVec = &rSupGenVec;

  // construct global controllable events
  GConAlph.Assign(rConAlph);

  // construct global events
  EventSet GAlph;
  SetUnion(rPlantGenVec, GAlph);
  EventSet GSpecAlph;
  SetUnion(rSpecGenVec, GSpecAlph);
  SetUnion(GAlph, GSpecAlph, GAlph);

  // initialize map
  EventSet::Iterator eit=GAlph.Begin();
  for(; eit!=GAlph.End(); ++eit) rMapEventsToPlant[*eit]=*eit;

  // construct global uncontrollable events
  EventSet GUncAlph;
  SetDifference(GAlph, GConAlph, GUncAlph);

  // take a copy of plant-generators for synthese-buffer and supervisors 
  GeneratorVector::Position pos = 0;
  for(; pos < rPlantGenVec.Size(); ++pos) {
    GenVec.PushBack(rPlantGenVec.At(pos));
    pSupGenVec->PushBack(rPlantGenVec.At(pos));
  }

  // take a copy of specification-generators for supervisors
  pos=0;
  for(; pos < rSpecGenVec.Size(); ++pos)
    pSupGenVec->PushBack(rSpecGenVec.At(pos));

  // translation of specifications into plants
  
  TransSpec(rSpecGenVec, GUncAlph, GenVec); 

  // add a new event for termination in event-symboltable 
  pEvSymTab = GenVec.At(0).EventSymbolTablep();
  std::string wname = pEvSymTab->UniqueSymbol("WEvent");
  w = pEvSymTab->InsEntry(wname);

  // add w to controllable events
  GConAlph.Insert(w);

  // add w and w-selfloop to every automat in GenVec
  pos=0;
  for(; pos < GenVec.Size(); ++pos) {
    GenVec.At(pos).InsEvent(w);
    StateSet::Iterator sit = GenVec.At(pos).MarkedStatesBegin();
    for(; sit != GenVec.At(pos).MarkedStatesEnd(); ++sit)
      GenVec.At(pos).SetTransition(*sit, w, *sit);
  }

}


// void ComSyn::Preprocess(void);
void ComSyn::Preprocess()
{
  // helpers
  EventSet RestAlph;     // global alphabet without the specified generator
  EventSet LocAlph;      // local alphabet of the specified generator
  EventSet ConAlph;      // local controllable events in the specified generator

  Generator AbstGen;     // generator after abstraction

  // a state map from "HSupGen" to "AbstGen"
  std::map<Idx,Idx> MapStateToPartition;

  // a map in which a event[original] (the first idx) -
  // is replaced by two events[alias] (the second idx)
  std::map<Idx,std::vector<Idx> >  MapOldToNew;

  // loop for GenVec 
  GeneratorVector::Position i = 0;
  for(; i < GenVec.Size(); ++i) {

    // build related eventset
    // ****from class private ****
    SetPartUnion(GenVec, i, RestAlph); 
    // construct local alphabet of OrigGen
    SetDifference(GenVec.At(i).Alphabet(), RestAlph, LocAlph);
    // construct controllable alphabet of OrigGen
    SetIntersection(GenVec.At(i).Alphabet(), GConAlph, ConAlph);

    //GenVec.At(i).Write();
    // abstraction rule -> SOE 
    
    ComputeSynthObsEquiv(GenVec.At(i), ConAlph, LocAlph, MapStateToPartition, AbstGen);
    //AbstGen.Write();

    // abstraction rule -> remove local events if it hat only selfloop-transitions
    LocalSelfloop(AbstGen, LocAlph);
    
    // abstraction rule -> give up the AbstGen if it hat only one state
    if(AbstGen.Size() == 1) {
      GenVec.Erase(i--);
      continue;
    }
    
    // replace the OrigGen by AbstGen if AbstGen is deterministic 
    if(IsDeterministic(AbstGen)) 
      GenVec.Replace(i, AbstGen);
    
    else {

      // rewrite OrigGen, AbstGen and construct the related MapOldToNewMap
      MakeDistinguisher(AbstGen, MapStateToPartition, GenVec.At(i), MapOldToNew); 

      // remove w 
      GenVec.At(i).DelEvent(w);
      // set a new name for distinguisher
      std::stringstream str_dis;
      str_dis<<" Distinguisher from "<<GenVec.At(i).Name();
      GenVec.At(i).Name(str_dis.str());
      // save distinguisher 
      pDisGenVec->PushBack(GenVec.At(i)); 

      // replace the OrigGen by AbstGen in GenVec
      GenVec.Replace(i, AbstGen);
      
      
      Splitter(MapOldToNew, GConAlph, GenVec,
	       *pMapEventsToPlant, *pDisGenVec, *pSupGenVec);
    }
  }

  // abstraction rule -> GlobalSelfloop
  
  GlobalSelfloop(GenVec);
}


// void ComSyn::Synthesis(void);
void ComSyn::Synthesis()
{
  // helpers
  EventSet RestAlph;     // global alphabet without the current generator
  EventSet LocAlph;      // local alphabet of the specified generator
  EventSet ConAlph;      // controllable events in the specified generator

  Generator OrigGen;     // generator after synchronisation
  Generator HSupGen;     // generator after halbway-synthsis
  Generator AbstGen;     // generator after abstraction

  // a state map from "HSupGen" to "AbstGen"
  std::map<Idx,Idx> MapStateToPartition;

  // a map in which a event[original] (the first idx) -
  // is replaced by two events[alias] (the second idx)
  std::map<Idx,std::vector<Idx> >  MapOldToNew;

  // loop for GenVec
  while(GenVec.Size() > 1) {

    // Step 1: Select subsystem and compose them, then remove subsystem from buffer    
    
    SelectSubsystem_V1(GenVec, OrigGen);
    // set default state name of OrigGen
    OrigGen.SetDefaultStateNames();
    // test
    ////////////////////////////////////////////////////////////////

    // Step 2: construct related events
    SetUnion(GenVec, RestAlph);
    SetDifference(OrigGen.Alphabet(), RestAlph, LocAlph);
    SetIntersection(OrigGen.Alphabet(), GConAlph, ConAlph);
    // test
    ////////////////////////////////////////////////////////////////
    
    // Step 3: abstraction rule -> halbway synthesis
    
    ComputeHSupConNB(OrigGen, ConAlph, LocAlph, HSupGen);
    // test
    //HSupGen.Write();
    ////////////////////////////////////////////////////////////////

    // Step 4: compare HSupGen with OrigGen
    if(HSupGen.TransRelSize() < OrigGen.TransRelSize()) {
      // add to supervisors
      pSupGenVec->PushBack(HSupGen);
      // remove w
      pSupGenVec->At(pSupGenVec->Size()-1).DelEvent(w);
    }
    // test
    ////////////////////////////////////////////////////////////////

    // Step 5: abstraction rule -> bisimulation
    // an additional interface for function ComputeBisimulation
    
    comsyn_ComputeBisimulation(HSupGen);
    // test
    ////////////////////////////////////////////////////////////////
       
    // Step 6: abstraction rule -> bisimulation
    
    ComputeSynthObsEquiv(HSupGen, ConAlph, LocAlph, MapStateToPartition, AbstGen);
    // test
    ////////////////////////////////////////////////////////////////

    // Step 7: abstraction rule -> LocalSelfloop
    // remove local events if it hat only selfloop-transitions
    
    LocalSelfloop(AbstGen, LocAlph);
    // test
    ////////////////////////////////////////////////////////////////

    // Step 8 : test determinism
    // take a copy of AbstGen to buffer if AbstGen is deterministic 
    if(IsDeterministic(AbstGen)) 
      GenVec.PushBack(AbstGen);
    else {
      // rewrite HSupGen, AbstGen and construct the related MapOldToNewMap
      
      MakeDistinguisher(AbstGen, MapStateToPartition, HSupGen, MapOldToNew);

      // remove w
      HSupGen.DelEvent(w);
      // set a new name for distinguisher
      std::stringstream str_dis;
      str_dis<<" Distinguisher from "<<HSupGen.Name();
      HSupGen.Name(str_dis.str());
      // rSupGenVec take a copy of HSupGen  (HSupGen is now a distinguisher)
      pDisGenVec->PushBack(HSupGen);

      // let GenVec take a copy of AbstGen 
      GenVec.PushBack(AbstGen);

      // update the event everywhere
      
      Splitter(MapOldToNew, GConAlph, GenVec,
	       *pMapEventsToPlant, *pDisGenVec, *pSupGenVec);
    }
  }
}


// Destructer
ComSyn::~ComSyn(void)
{
  pEvSymTab->ClrEntry(w);  
}


// SetPartUnion
void ComSyn::SetPartUnion(const faudes::GeneratorVector& rGenVec, 
			  GeneratorVector::Position WithoutMe,
			  EventSet& rRestAlph) {
  // clear date
  rRestAlph.Clear();

  // loop
  faudes::GeneratorVector::Position i = 0;
  for(; i < rGenVec.Size(); ++i)
    if(i != WithoutMe) rRestAlph.InsertSet(rGenVec.At(i).Alphabet());
}


// comsyn_ComputeBisimulation
void ComSyn::comsyn_ComputeBisimulation(faudes::Generator& rGen)
{
  // prepare data for interface ComputeBisimulation of libfaudes
  faudes::Generator OrigGen;
  OrigGen.Assign(rGen);
  std::map<faudes::Idx,faudes::Idx> MapStateToPartition;

  // run interface 
  ComputeBisimulation(OrigGen, MapStateToPartition, rGen);
}


////////////////////////////////////////////////////////////////
// TransSpec
////////////////////////////////////////////////////////////////

// subfunction for TransSpec
void SingleTransSpec(const Generator& rSpecGen,
		     const EventSet& rUncAlph,
		     Generator& rResGen) {

  // copy the particular specification generator to rResGen
  rResGen.Assign(rSpecGen);

  // construct local uncontrollable alphabet
  EventSet UncAlph;
  SetIntersection(rUncAlph, rResGen.Alphabet(), UncAlph);

  // test the need 
  if(UncAlph.Empty()) return;

  // helpers
  TransSet toadd;
  EventSet todo;
  Generator gen;
  Idx temp = 1;       // not used, only take place 

  // loop for every state in the rResGen
  StateSet::Iterator sit = rResGen.StatesBegin();
  for(; sit != rResGen.StatesEnd(); ++sit){
    // build todo that will to be used for setting new Transitions
    todo.Clear();
    SetDifference(UncAlph, rResGen.ActiveEventSet(*sit), todo);
    // loop for setting transitions if todo is not empty   
    EventSet::Iterator eit=todo.Begin();    
    for(; eit != todo.End(); ++eit) toadd.Insert(*sit, *eit, temp);
  }
 
  // no need to insert new state if toadd is empty
  if(toadd.Empty()) return;
  // add a new blocking state to rResGen
  Idx bstate = rResGen.InsState();
 
  // rewrite rResGen by inserting new transitions 
  TransSet::Iterator tit = toadd.Begin();
   for(; tit != toadd.End(); ++tit)
    rResGen.SetTransition(tit->X1, tit->Ev, bstate);
}



// TransSpec(rSpecGenVec, GUncAlph, GenVec)
void TransSpec(const GeneratorVector& rSpecGenVec, 
	       const EventSet& rGUncAlph,
	       GeneratorVector& rResGenVec) {

  // helpers
  Generator NewGen;

  // loop
  GeneratorVector::Position i = 0;
  for(; i < rSpecGenVec.Size(); ++i) {
   
    // translate a generator into plant
    SingleTransSpec(rSpecGenVec.At(i), rGUncAlph, NewGen);
    rResGenVec.Append(NewGen);
  }

}



////////////////////////////////////////////////////////////////
// GlobalSelfloop
////////////////////////////////////////////////////////////////

void GlobalSelfloop(GeneratorVector& rGenVec){

  EventSet toinspect;
  SetUnion(rGenVec, toinspect);
  
  // a loop for every event in "toinspect" to check 
  for(EventSet::Iterator eit = toinspect.Begin(); eit != toinspect.End(); ++eit){

    // collect Positions of generator in rGenVec that have the current event *eit
    std::vector<GeneratorVector::Position> PsubGens;
    for(GeneratorVector::Position i=0; i<rGenVec.Size(); ++i){
      if(rGenVec.At(i).ExistsEvent(*eit))
	PsubGens.push_back(i);
    }


    // <<gibt's ein solche MemberFunction von generator class, zwar lässt sich TransSet::Iterator nur durch ev auslesen??>>
    // test whether all the transtions with the current event *eit that are in collected generators is only selfloop.
    // when yes, "remove" remains true; when no, set "remove" false and break the loop
    bool remove = true;
    for(std::vector<GeneratorVector::Position>::iterator vit = PsubGens.begin(); vit != PsubGens.end(); ++vit){
      for(TransSet::Iterator tit = rGenVec.At(*vit).TransRelBegin(); tit != rGenVec.At(*vit).TransRelEnd(); ++tit){
	if((*eit == tit->Ev) && (tit->X1 != tit->X2)){
	  remove = false;
	  break;
	}// end if
      }// end inner forn
      if(remove == false)
	break;
    }// end outer for


    //when "remove" is true, remove all the transitions with current event that are only selfloop and minimize the Alphabet through remove the unused events
    if(remove){
      for(std::vector<GeneratorVector::Position>::iterator vit = PsubGens.begin(); vit != PsubGens.end(); ++vit){
	for(TransSet::Iterator tit = rGenVec.At(*vit).TransRelBegin(); tit != rGenVec.At(*vit).TransRelEnd(); ){
	  if((*eit == tit->Ev) && (tit->X1 == tit->X2))
	    rGenVec.At(*vit).ClrTransition(tit++); // tmoor 201602
          else
            ++tit;
	}//end inner for
	rGenVec.At(*vit).MinimizeAlphabet();
      }//end outer for
    }//end if	  
  }//end first for 

}

////////////////////////////////////////////////////////////////
// LocalSelfloop
////////////////////////////////////////////////////////////////

void LocalSelfloop(Generator& rGen, EventSet& rLocAlph) {

  // construct inverse transition set
  TransSetEvX1X2 InvTransSet;
  rGen.TransRel(InvTransSet);

  // loop for every local event
  EventSet::Iterator eit = rLocAlph.Begin();
  for(; eit != rLocAlph.End(); ++eit) {
    bool IsSelfloop = true;
    // find related transitions in the inverse transition set
    TransSetEvX1X2::Iterator tit = InvTransSet.BeginByEv(*eit);
    for(; tit != InvTransSet.EndByEv(*eit); ++tit)
      // test whether it is selfloop
      if(tit->X1 != tit->X2) {IsSelfloop = false; break;}

    // remove transitions and event from alphabet if selfloop
    if(IsSelfloop) rGen.DelEvent(*eit);
  }
}


////////////////////////////////////////////////////////////////
// MakeDistinguisher 
////////////////////////////////////////////////////////////////


//MakeDistinguisher(AbstGen, MapStateToPartition, OrigGen, MapOldToNew);
void MakeDistinguisher(Generator& AbstGen,
		       std::map<Idx,Idx>& rMapStateToPartition, 
		       Generator& OrigGen,
		       std::map<Idx,std::vector<Idx> >& rMapOldToNew) {

  // clear rMapOldToNew
  rMapOldToNew.clear();

  // helpers
  std::vector<std::vector<TransSet::Iterator> > NonDet;
  std::vector<Idx> OrigStates;

  // find the transitions in AbstGen that are nondeterministic
  TransSet::Iterator tit1 = AbstGen.TransRelBegin();
  TransSet::Iterator tit2;
  for(tit2 = tit1++; tit1 != AbstGen.TransRelEnd(); tit2 = tit1++) {
    if((tit2->X1 == tit1->X1) && (tit2->Ev == tit1->Ev)) {
      std::vector<TransSet::Iterator> nondet;
      nondet.push_back(tit2);
      nondet.push_back(tit1);
      NonDet.push_back(nondet);
    }
  }

  std::string oldev;
  std::string new_1_name;
  std::string new_2_name;

  // loop for all nondet:
  std::vector<std::vector<TransSet::Iterator> >::iterator vvit=NonDet.begin();
  for(; vvit != NonDet.end(); ++vvit) {
    // 1. set new events
    oldev = AbstGen.EventName((*vvit)[0]->Ev);
    //new_1_name = AbstGen.EventSymbolTablep->UniqueSymbol(oldev);
    //new_2_name = AbstGen.EventSymbolTablep->UniqueSymbol(oldev);
    new_1_name = oldev + "_1_";
    new_2_name = oldev + "_2_";
    Idx new1 = AbstGen.InsEvent(new_1_name);
    Idx new2 = AbstGen.InsEvent(new_2_name);
    OrigGen.InsEvent(new1);
    OrigGen.InsEvent(new2);

    // 2. record in map
    if(rMapOldToNew.find((*vvit)[0]->Ev) == rMapOldToNew.end()) {
      rMapOldToNew[(*vvit)[0]->Ev].push_back(new1);
      rMapOldToNew[(*vvit)[0]->Ev].push_back(new2);
    }

    // 3. replace old by two new in AbstGen
    AbstGen.SetTransition((*vvit)[0]->X1, new1, (*vvit)[0]->X2);
    AbstGen.SetTransition((*vvit)[1]->X1, new2, (*vvit)[1]->X2);

    // 4. find the related transitions in OrigGen
    OrigStates.clear();
    std::map<Idx,Idx>::iterator mitSt = rMapStateToPartition.begin();
    for(; mitSt != rMapStateToPartition.end(); ++mitSt) 
      if(mitSt->second == (*vvit)[0]->X1) OrigStates.push_back(mitSt->first);

    // 5. set new and remove old transitions in OrigGen
    std::vector<Idx>::iterator vit = OrigStates.begin();
    for(; vit != OrigStates.end(); ++vit) {
      // find related transitions
      TransSet::Iterator tit = OrigGen.TransRelBegin(*vit, (*vvit)[0]->Ev);
      TransSet::Iterator tit_end = OrigGen.TransRelEnd(*vit, (*vvit)[0]->Ev);
      if(tit!=tit_end) { // this means at least transition does exist      

        if(rMapStateToPartition[tit->X2] == (*vvit)[0]->X2) {
	  OrigGen.SetTransition(tit->X1, new1, tit->X2);
	  OrigGen.ClrTransition(tit);
        }
        if(rMapStateToPartition[tit->X2] == (*vvit)[1]->X2) {	 
	  OrigGen.SetTransition(tit->X1, new2, tit->X2);
	  OrigGen.ClrTransition(tit);
        }
      } 

    }
    // 6. clr old transitions in AbstGen
    AbstGen.ClrTransitions((*vvit)[0]->X1, (*vvit)[0]->Ev);
    // delete the related events
    // AbstGen.DelEventFromAlphabet(titDet->Ev);
    // OrigGen.DelEventFromAlphabet(titDet->Ev);
    // ...not right now, 
    // later in function Splitter which test where exist yet old event
  }

}

////////////////////////////////////////////////////////////////
// Splitter
////////////////////////////////////////////////////////////////

/** 
 * subfunction for Splitter:
 * splitt the events in a generator
 * 
 * @param rGen
 *   a generator 
 * @param parent
 *   primal event
 * @param kids
 *   new events
 */
void SplittGen(Generator& rGen,
	       Idx parent,
	       const std::vector<Idx>& kids) {

  
  // insert kids and debug
  std::vector<Idx>::const_iterator vit = kids.begin();
  for(; vit != kids.end(); ++vit) 
    rGen.InsEvent(*vit);
  
  // find the transitions which via parent  
  TransSetEvX1X2 InvRel;
  rGen.TransRel(InvRel);
  TransSetEvX1X2::Iterator tit = InvRel.BeginByEv(parent);
  TransSetEvX1X2::Iterator tit_end = InvRel.EndByEv(parent);
  //	int ts=0;
  for(; tit != tit_end; ++tit) {
    // record
    //++ts;
    
    // clr the transitions with the parent
    rGen.ClrTransition(tit->X1, tit->Ev, tit->X2);
    
    // set new transitions
    std::vector<Idx>::const_iterator vit = kids.begin();
    for(; vit != kids.end(); ++vit) {
      Boolean isset = false;
      isset = rGen.SetTransition(tit->X1, *vit, tit->X2);
      if(!isset) {
	std::cout << "in Function Splitter(): ";
	std::cout << "the SetTransition in rGenVec is failed\n";
      }
    }
  }

  // delete the no longer used parent from alphabet 
  rGen.DelEventFromAlphabet(parent);
    
}

// Splitter(MapOldToNew, GConAlph, GenVec,
//	    rMapEventsToPlant, rDisGenVec, rSupGenVec);
void Splitter(const std::map<Idx, std::vector<Idx> >& rMapOldToNew,
	      EventSet& rConAlph,
	      GeneratorVector& rGenVec,
	      std::map<Idx,Idx>& rMapEventsToPlant,
	      GeneratorVector& rDisGenVec,
	      GeneratorVector& rSupGenVec) {


  // loop for every element in rMapOldToNew
  std::map<Idx, std::vector<Idx> >::const_iterator nemit = rMapOldToNew.begin();
  for(; nemit != rMapOldToNew.end(); ++nemit) {

    // helper
    Idx parent = nemit->first;
    const std::vector<Idx>& kids = nemit->second;

    ////////////////////////////////
    // splitt event in rConAlph
    ////////////////////////////////

    if(rConAlph.Exists(parent)) {
      // erase parent
      rConAlph.Erase(parent);
      // insert kids
      std::vector<Idx>::const_iterator vit = kids.begin();
      for(; vit != kids.end(); ++vit) rConAlph.Insert(*vit);
    }

    ////////////////////////////////
    // splitt event in rMapEventsToPartition
    ////////////////////////////////

    // erase parent
    rMapEventsToPlant.erase(rMapEventsToPlant.find(parent));
    // insert kids
    std::vector<Idx>::const_iterator vit = kids.begin();
    for(; vit != kids.end(); ++vit)
      rMapEventsToPlant.insert(std::make_pair(*vit, parent));

    ////////////////////////////////
    // splitt event in rGenVec
    ////////////////////////////////

    GeneratorVector::Position i=0;
    for(; i < rGenVec.Size(); ++i) 
      if(rGenVec.At(i).ExistsEvent(parent)) SplittGen(rGenVec.At(i), parent, kids);

    ////////////////////////////////
    // split event in rSupGenVec
    ////////////////////////////////

    i=0;
    for(; i < rSupGenVec.Size(); ++i)
      if(rSupGenVec.At(i).ExistsEvent(parent)) SplittGen(rSupGenVec.At(i), parent, kids);

    ////////////////////////////////
    // split event in rDisGenVec
    ////////////////////////////////

    i=0;
    for(; i < rDisGenVec.Size(); ++i)
      if(rDisGenVec.At(i).ExistsEvent(parent)) SplittGen(rDisGenVec.At(i), parent, kids);

    
    ////////////////////////////////
    // The End
    ////////////////////////////////

  } // end the loop for rMapOldToNew
}



////////////////////////////////////////////////////////////////
// SelectSubsystem_V1
////////////////////////////////////////////////////////////////


// make class as predicate for passing more parameters in STL algorithm
class SmallSize {

private:
  const GeneratorVector& rGenVec;

public:
  SmallSize(const GeneratorVector& rGenVec):
    rGenVec(rGenVec) {}

  bool operator()(const std::vector<GeneratorVector::Position> rPosVec1,
		  const std::vector<GeneratorVector::Position> rPosVec2) {

    Generator Gen1;
    GeneratorVector GenVec1;
    std::vector<GeneratorVector::Position>::const_iterator vit1;
    vit1 = rPosVec1.begin();
    for(; vit1 != rPosVec1.end(); ++vit1) GenVec1.PushBack(rGenVec.At(*vit1));
    aParallel(GenVec1, Gen1);

    Generator Gen2;
    GeneratorVector GenVec2;
    std::vector<GeneratorVector::Position>::const_iterator vit2;
    vit2 = rPosVec2.begin();
    for(; vit2 != rPosVec2.end(); ++vit2) GenVec2.PushBack(rGenVec.At(*vit2));
    aParallel(GenVec2, Gen2);

    return Gen1.Size() < Gen2.Size();
  }
};

////////////////
// select_V1
// MustL + MinS
////////////////

// BiggerMax(Candidate, rGenVec)
bool BiggerMax(std::vector<GeneratorVector::Position>& rCandidate,
	       GeneratorVector& rGenVec) {

  //the size of rCandidate is always >= 2 

  // set Max 
  Idx Max=5000;

  Generator gen;
  gen=rGenVec.At(rCandidate[0]);
  std::vector<GeneratorVector::Position>::iterator vit=rCandidate.begin();
  for(std::advance(vit,1);vit!=rCandidate.end();++vit) {
    Parallel(rGenVec.At(*vit),gen,gen);
    if(gen.Size()>Max) return true;
  }
  return false;
  
}

void SelectSubsystem_V1(GeneratorVector& rGenVec,
			Generator& rResGen) {
  
  // if only two generators in the vector
  if(rGenVec.Size() == 2) {
    Parallel(rGenVec.At(0), rGenVec.At(1), rResGen);
    rGenVec.Clear();
    return;
  }
  
  // construct a global alphabet from "rGenVec"
  EventSet GAlph;
  SetUnion(rGenVec, GAlph);

  // prepare data
  std::vector<GeneratorVector::Position> Candidate;
  std::vector<std::vector<GeneratorVector::Position> > Candidates;
  Candidates.reserve(GAlph.Size());

  // Step 1: construct candidate and candidates
  EventSet::Iterator eit = GAlph.Begin();
  for(; eit != GAlph.End(); ++eit) {
    Candidate.clear();
    GeneratorVector::Position i = 0;
    for(; i < rGenVec.Size(); ++i)
      if(rGenVec.At(i).ExistsEvent(*eit)) Candidate.push_back(i);
    if(Candidate.size() == 1) continue;
    // give up the candidate which is bigger than MAX
    if(BiggerMax(Candidate, rGenVec)) continue;
    Candidates.push_back(Candidate);
  }

  // step 2 is expensive, because of composing for large automaton
  // before the step2 firstly test the size of every candidate !! 
  // give up which is bigger than a given size 
  // 

  // Step 2: find the target
  std::vector<std::vector<GeneratorVector::Position> >::iterator PosTarget;
  PosTarget = std::min_element(Candidates.begin(), Candidates.end(),
			    SmallSize(rGenVec));

  // *parallel composition 

  rResGen.Assign(rGenVec.At(PosTarget->at(0)));
  std::vector<GeneratorVector::Position>::iterator vit=PosTarget->begin();
  for(std::advance(vit,1); vit!=PosTarget->end();++vit)
    Parallel(rResGen, rGenVec.At(*vit), rResGen);

  // *delete subsystem from "GenVec"
  GeneratorVector::Position x = 0;
  vit = PosTarget->begin();
  for(; vit != PosTarget->end(); ++vit) {
    rGenVec.Erase(*vit-x);
    ++x;
  }
}


////////////////////////////////////////////////////////////////
// SelectSubsystem_V2
////////////////////////////////////////////////////////////////

////////////////
// Select_V2
// MaxC + MinS
////////////////

void SelectSubsystem_V2(GeneratorVector& rGenVec,
			Generator& rResGen) {

  // clear output
  rResGen.Clear();

  // if only two generators in the vector
  if(rGenVec.Size() == 2) {
    Parallel(rGenVec.At(0), rGenVec.At(1), rResGen);
    rGenVec.Clear();
    return;
  }
  // *select subsystem
  // helpers 
  std::vector<GeneratorVector::Position> candidate;
  std::vector<std::vector<GeneratorVector::Position> > candidates;

  Idx maxsize;
  GeneratorVector::Position pos;
  EventSet alph;

  // step 1: loop for constructing candidates
  GeneratorVector::Position i = 0;
  for(; i < rGenVec.Size()-1; ++i) {
    // component 1
    const EventSet& alph1 = rGenVec.At(i).Alphabet();

    // determine component 2
    SetIntersection(alph1, rGenVec.At(i+1).Alphabet(), alph);
    pos = i+1;
    maxsize = alph.Size();
    // loop from i+2 
    GeneratorVector::Position j = i+2;
    for(; j < rGenVec.Size(); ++j) {
      // setintersection
      SetIntersection(alph1, rGenVec.At(j).Alphabet(), alph);
      // "MaxC": maximal common event
      if(alph.Size() > maxsize) {pos = j; maxsize = alph.Size();}
      else if(alph.Size() == maxsize) {
	Generator g_pos;
	Generator g_j;
	Parallel(rGenVec.At(i), rGenVec.At(pos), g_pos);
	Parallel(rGenVec.At(i), rGenVec.At(j), g_j);
	// choose "MinS" if the number of common events is same
	// or choose others: "MaxL"
	if(g_j.Size() < g_pos.Size()) {pos = j; maxsize = alph.Size();}
      }
    }

    // save as a candidate in candidates
    candidate.clear();
    candidate.push_back(i);
    candidate.push_back(pos);
    candidates.push_back(candidate);
  }

  // step 2: pick out the best candidate
  std::vector<std::vector<GeneratorVector::Position> >::iterator PosTarget;
  PosTarget = std::min_element(candidates.begin(), candidates.end(),
			    SmallSize(rGenVec));

  // *parallel composition 
  Parallel(rGenVec.At(PosTarget->at(0)), rGenVec.At(PosTarget->at(1)), rResGen);
  
  // *delete subsystem from "GenVec"
  rGenVec.Erase(PosTarget->at(0)); 
  rGenVec.Erase(PosTarget->at(1)-1);
}




////////////////////////////////////////////////////////////////
// Halbway-Synthesis
////////////////////////////////////////////////////////////////


// for function "ComputeHSupConNB()"

//this is a internal function that aiding to collecting states 
//which lead to blockingstates via local uncontrollable events
//and collecting transition relation which is further to redirecting

// internal function for HSupConNB()
void H_tocollect(StateSet& rBlockingstates, 
		 const TransSetX2EvX1& rRtrel,
		 const EventSet& rLouc, 
		 const EventSet& rShuc, 
		 TransSetX2EvX1& rToredirect) {

  //initialize todo stack
  std::stack<Idx> todo;
  StateSet::Iterator sit;
  for(sit = rBlockingstates.Begin(); sit != rBlockingstates.End(); ++sit){
    todo.push(*sit);
  }

  rBlockingstates.Clear();

  //loop variables
  Idx x2;
  TransSetX2EvX1::Iterator tit;
  TransSetX2EvX1::Iterator tit_end;
  //loop
  while(!todo.empty()){
    //pop
    x2 = todo.top();
    todo.pop();
    //save result
    if(rBlockingstates.Exists(x2)) continue;
    rBlockingstates.Insert(x2);
    //iterate/push
    tit = rRtrel.BeginByX2(x2);
    tit_end = rRtrel.EndByX2(x2);
    for(; tit_end != tit; ++tit){
      if(rLouc.Exists(tit->Ev))
	todo.push(tit->X1);
      else if(rShuc.Exists(tit->Ev))
	rToredirect.Insert(*tit);
    }
  }

}

// ComputeHSupConNB(OrigGen, ConAlph, LocAlph, rHSupGen)
void ComputeHSupConNB(const Generator& rOrigGen,
		   const EventSet& rConAlph,
		   const EventSet& rLocAlph,
		   Generator& rHSupGen) {

  // initialize rHSupGen
  rHSupGen.Assign(rOrigGen);

  // initialize blockingstates 
  StateSet blockingstates;
  blockingstates = rHSupGen.BlockingStates();

  // verify the need to perform
  Boolean isneed;
  isneed = !blockingstates.Empty(); 

  // construct uncontrollable alphabet
  EventSet rUCAlph;
  SetDifference(rHSupGen.Alphabet(), rConAlph, rUCAlph);

  // loop variables
  EventSet shalph;
  EventSet louc;
  EventSet shuc;
  TransSetX2EvX1 toredirect;
  TransSetX2EvX1 itrel;

  //initialize loop variables if need
  if(isneed){
    //set inverse transition relation from generator
    rHSupGen.TransRel(itrel);
    //set shared alphabet
    SetDifference(rHSupGen.Alphabet(), rLocAlph, shalph);
    //verify local uncontrollable events 
    SetIntersection(rUCAlph, rLocAlph, louc);
    //verify shared uncontrollable events
    SetIntersection(rUCAlph, shalph, shuc);
  }

  //loop until no more blockingstates 
  while(!blockingstates.Empty()){
    //collect more blockingstates and transition relation which need to be redirected
    H_tocollect(blockingstates, itrel, louc, shuc, toredirect);    //to implement
    //delete blockingstates
    rHSupGen.DelStates(blockingstates);
    //update blockingstates
    blockingstates = rHSupGen.BlockingStates();
    //update itrel
    rHSupGen.TransRel(itrel);
  }

  if(!toredirect.Empty()) {
    //insert a blockingstate into gererator to redirect
    Idx bstate = rHSupGen.InsState();
    // set the transitions which is via shared uncontrollable events
    TransSetX2EvX1::Iterator tit = toredirect.Begin();
    TransSetX2EvX1::Iterator tit_end = toredirect.End();
    for(; tit != tit_end; ++tit) {
      if(!rHSupGen.ExistsState(tit->X1)) continue;
      rHSupGen.SetTransition(tit->X1, tit->Ev, bstate);
    }
  }

}



/*
****************************************************************
****************************************************************
PART C: Minimal API

****************************************************************
****************************************************************
*/

void ControlProblemConsistencyCheck(const GeneratorVector& rPlantGenVec, 
		   const EventSet& rConAlph,
		   const GeneratorVector& rSpecGenVec)
{
  // Check 1: all generators in "rPlantGenVec" and "rSpecGenVec" must be deterministic
  GeneratorVector::Position i=0;
  for(; i < rPlantGenVec.Size(); ++i) 
    if(!rPlantGenVec.At(i).IsDeterministic()) {
      std::stringstream errstr;
      errstr<<"Plant:"<<rPlantGenVec.At(i).Name()<<" is nondeterministic\n";
      throw Exception("Zhou::ParameterCheck", errstr.str(), 900);
    }

  i=0;
  for(; i < rSpecGenVec.Size(); ++i) 
    if(!rSpecGenVec.At(i).IsDeterministic()) {
      std::stringstream errstr;
      errstr<<"Spec:"<<rSpecGenVec.At(i).Name()<<" is nondeterministic\n";
      throw Exception("Zhou::ParameterCheck", errstr.str(), 901);
    }

  // Check 2: all generators must have marked states
  i=0;
  for(; i < rPlantGenVec.Size(); ++i) {
    const StateSet& stateset = rPlantGenVec.At(i).MarkedStates();
    if(stateset.Empty()) {
      std::stringstream errstr;
      errstr<<"Plant:"<<rPlantGenVec.At(i).Name()<<" hat no marked states\n";
      throw Exception("Zhou::ParameterCheck", errstr.str(), 910);
    }
  }
  
  i=0;
  for(; i < rSpecGenVec.Size(); ++i) {
    const StateSet& stateset = rSpecGenVec.At(i).MarkedStates();
    if(stateset.Empty()) {
      std::stringstream errstr;
      errstr<<"Spec:"<<rSpecGenVec.At(i).Name()<<" hat no marked states\n";
      throw Exception("Zhou::ParameterCheck", errstr.str(), 911);
    }
  }

  // Check 3: the correctness of controllable events
  // i.e. all controllable events muss be included in global alphabet
  EventSet GAlph;
  SetUnion(rPlantGenVec, GAlph);
  EventSet GSpecAlph;
  SetUnion(rSpecGenVec, GSpecAlph);
  SetUnion(GAlph, GSpecAlph, GAlph);

  EventSet::Iterator eit = rConAlph.Begin();
  for(; eit != rConAlph.End(); ++eit) 
    if(!GAlph.Exists(*eit)) {
      std::stringstream errstr;
      errstr<<"event:"<<rConAlph.SymbolicName(*eit)<<" is not included in generator\n";
      throw Exception("Zhou::ParameterCheck", errstr.str(), 920);
    }
}


void CompositionalSynthesisUnchecked(const GeneratorVector& rPlantGenVec, 
		       const EventSet& rConAlph,
		       const GeneratorVector& rSpecGenVec, 
		       std::map<Idx,Idx>& rMapEventsToPlant,
		       GeneratorVector& rDisGenVec,
		       GeneratorVector& rSupGenVec)
{
  // Construct an instance of Class "ComSyn"
  ComSyn comsyn = ComSyn(rPlantGenVec, rConAlph, rSpecGenVec,
			 rMapEventsToPlant, rDisGenVec, rSupGenVec);
  // run Preprocess
  comsyn.Preprocess();
  // run Synthesis
  comsyn.Synthesis();
}



void CompositionalSynthesis(const GeneratorVector& rPlantGenVec, 
	      const EventSet& rConAlph,
	      const GeneratorVector& rSpecGenVec, 
	      std::map<Idx,Idx>& rMapEventsToPlant,
	      GeneratorVector& rDisGenVec,
	      GeneratorVector& rSupGenVec)
{
  // PARAMETER CHECK
  ControlProblemConsistencyCheck(rPlantGenVec, rConAlph, rSpecGenVec);

  // ALGORITHM
  CompositionalSynthesisUnchecked(rPlantGenVec, rConAlph, rSpecGenVec, rMapEventsToPlant, rDisGenVec, rSupGenVec);
}


} // namespace
