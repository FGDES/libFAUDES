/** @file hyb_reachability.cpp  Reachable states of linear hybrid automata */

/* 
   Hybrid systems plug-in for FAU Discrete Event Systems Library 

   Copyright (C) 2017 Thomas Moor

*/



#include "hyb_reachability.h"
#include "hyb_compute.h"

namespace faudes {


/*
*************************************************************
*************************************************************

Implementation: HybridStateSet

*************************************************************
*************************************************************
*/

/** constructors */
HybridStateSet::HybridStateSet(void) {}

/** copy constrcutor */
HybridStateSet::HybridStateSet(const  HybridStateSet& rOther) 
{  
  Assign(rOther);
}

/** destructor */
HybridStateSet::~HybridStateSet(void) {
  /* delete all polyhedra */
  IndexSet::Iterator qit=LocationsBegin();
  IndexSet::Iterator qit_end=LocationsEnd();
  for(;qit!=qit_end;++qit) {
    Iterator pit=StatesBegin(*qit);
    Iterator pit_end=StatesEnd(*qit);
    for(;pit!=pit_end;++pit) delete *pit;
  }
}

/** assignment */  
void HybridStateSet::Assign(const  HybridStateSet& rOther) {
  /* delete all polyhedra */
  IndexSet::Iterator qit=LocationsBegin();
  IndexSet::Iterator qit_end=LocationsEnd();
  for(;qit!=qit_end;++qit) {
    Iterator pit=StatesBegin(*qit);
    Iterator pit_end=StatesEnd(*qit);
    for(;pit!=pit_end;++pit) delete *pit;
  }
  mLocations.Clear();
  mStates.clear();
  /* copy all polyhedra */
  qit=rOther.LocationsBegin();
  qit_end=rOther.LocationsEnd();
  for(;qit!=qit_end;++qit) {
    Insert(*qit);
    Iterator pit=rOther.StatesBegin(*qit);
    Iterator pit_end=rOther.StatesEnd(*qit);
    for(;pit!=pit_end;++pit) {
      Polyhedron* poly = new Polyhedron(**pit);
      Insert(*qit,poly);
    }
  }
}

/** access to locations */
const IndexSet& HybridStateSet::Locations(void){
  return mLocations;
}
IndexSet::Iterator HybridStateSet::LocationsBegin(void) const {
  return mLocations.Begin();
}
IndexSet::Iterator HybridStateSet::LocationsEnd(void) const {
  return mLocations.End();
}

/** access to polyhedra of states */
HybridStateSet::Iterator HybridStateSet::StatesBegin(Idx q) const {
  HybridStateSet* fakeconst = const_cast<HybridStateSet*>(this);
  return fakeconst->mStates[q].begin();
}
HybridStateSet::Iterator HybridStateSet::StatesEnd(Idx q) const {
  HybridStateSet* fakeconst = const_cast<HybridStateSet*>(this);
  return fakeconst->mStates[q].end();
}

/** insert / erase (we take owvership of polyhedra) */
void HybridStateSet::Insert(Idx q) {
  mLocations.Insert(q);
}
void HybridStateSet::Insert(Idx q, Polyhedron* states) {
  mLocations.Insert(q);
  mStates[q].push_back(states);
}
void HybridStateSet::Erase(Idx q) {
  /* delete all polyhedra */
  Iterator pit=StatesBegin(q);
  Iterator pit_end=StatesEnd(q);
  for(;pit!=pit_end;++pit) delete *pit;
  mStates.erase(q);
}
void HybridStateSet::Clear(void) {
  /* delete all polyhedra */
  IndexSet::Iterator qit=LocationsBegin();
  IndexSet::Iterator qit_end=LocationsEnd();
  for(;qit!=qit_end;++qit) 
    Erase(*qit);
}

bool HybridStateSet::IsEmpty(void) const {
  /* test all polyhedra */
  IndexSet::Iterator qit=LocationsBegin();
  IndexSet::Iterator qit_end=LocationsEnd();
  for(;qit!=qit_end;++qit) {
    Iterator pit=StatesBegin(*qit);
    Iterator pit_end=StatesEnd(*qit);
    for(;pit!=pit_end;++pit) 
      if(!PolyIsEmpty(**pit)) return false;
  }
  return true;
}


/** inspect */
void HybridStateSet::DWrite(const LinearHybridAutomaton& lha) {
  FAUDES_WRITE_CONSOLE("%%%%% dump: hybrid state set");
  /* loop all locations */
  IndexSet::Iterator qit=LocationsBegin();
  IndexSet::Iterator qit_end=LocationsEnd();
  for(;qit!=qit_end;++qit) {
    /* report */
    FAUDES_WRITE_CONSOLE("%%% location: " << lha.SStr(*qit));
    /* loop all polyhedra */
    Iterator pit=StatesBegin(*qit);
    Iterator pit_end=StatesEnd(*qit);
    for(;pit!=pit_end;++pit) 
      PolyDWrite(**pit);
  }
  FAUDES_WRITE_CONSOLE("%%%%% dump: done");
}



/*
*************************************************************
*************************************************************

Implementation: Reachability

*************************************************************
*************************************************************
*/

void LhaReach(
  const LinearHybridAutomaton& lha, 
  const HybridStateSet& states, 
  std::map< Idx, HybridStateSet* >& ostates,
  int* pCnt) {

  // prepare result
  std::map< Idx, HybridStateSet* >::iterator eit=ostates.begin();
  for(;eit!=ostates.end();++eit)
    if(eit->second) delete eit->second;
  ostates.clear();
  
  // loop over all locations
  IndexSet::Iterator qit = states.LocationsBegin();
  IndexSet::Iterator qit_end = states.LocationsEnd();
  for(;qit!=qit_end;++qit) {

    // get flow/invariant
    const Polyhedron& rate = lha.Rate(*qit);
    const Polyhedron& inv = lha.Invariant(*qit);
    bool noflow = PolyIsEmpty(rate);
    
    // loop over all polyhedra
    HybridStateSet::Iterator pit = states.StatesBegin(*qit);
    HybridStateSet::Iterator pit_end = states.StatesEnd(*qit);
    for(;pit!=pit_end;++pit) {

      // book keeping
      if(pCnt && ! noflow) ++(*pCnt);

      // get states
      Polyhedron& poly = **pit;

      // apply flow
      Polyhedron reach;
      PolyCopy(poly,reach);
      PolyTimeElapse(rate,reach);
      PolyIntersection(inv,reach);
  
      //FD_WARN("inv - poly - - rate -reach");
      //inv.Write();
      //poly.Write();
      //rate.Write();
      //reach.Write();
    
      // intersect with guard
      TransSet::Iterator tit = lha.TransRelBegin(*qit);
      TransSet::Iterator tit_end = lha.TransRelEnd(*qit);
      for(;tit!=tit_end;++tit) {

        // intersect with guard
        Polyhedron* yreach=new Polyhedron();
        PolyCopy(reach,*yreach);
        PolyIntersection(lha.Guard(*tit),*yreach);

        // if empty, delete and continue
        if(PolyIsEmpty(*yreach)) {
          delete yreach;
          continue;
	}

        // apply reset, intersect with target invariant
        PolyLinearRelation(lha.Reset(*tit),*yreach);
        PolyIntersection(lha.Invariant(tit->X2),*yreach);

        // if empty, delete and continue
        /*
        if(PolyIsEmpty(*yreach)) {
          delete yreach;
          continue;
	}
	*/

        // record result
        PolyFinalise(*yreach);
        if(ostates.find(tit->Ev)==ostates.end())
	  ostates[tit->Ev] = new HybridStateSet();
	ostates[tit->Ev]->Insert(tit->X2,yreach);

      } // loop guards

    } // loop polyherda
  
  } // loop locations
}

} // namespace faudes
