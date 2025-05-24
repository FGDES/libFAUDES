/** @file syn_tsupcon.cpp Supremal TDES-controllable sublanguage */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Thomas Moor

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
   

#include "syn_tsupcon.h"


namespace faudes {




/*
***************************************************************************************
***************************************************************************************
 Implementation
***************************************************************************************
***************************************************************************************
*/


// SupTconUnchecked(rPlantGen, rCAlph, rFAlph, rPAlph, rCPAlph, rSupCandGen)
void SupTconUnchecked(
  const Generator& rPlantGen,  // aka G
  const EventSet& rCAlph,  
  const EventSet& rFAlph,  
  const EventSet& rPAlph,  
  const EventSet& rCPAlph,  
  Generator& rSupCandGen       // aka H
) 
{
  FD_DF("SupTconUnchecked(" << &rSupCandGen << "," << &rPlantGen << ")");

  // bail out if plant or candidate contain no initial states
  if(rPlantGen.InitStatesEmpty() || rSupCandGen.InitStatesEmpty()) 
    return;
   
  // todo stack (pairs of states)
  std::stack<Idx> todog, todoh;
  // set of already processed states of H (this is why we need H-states) 
  StateSet processed;

  // critical states
  StateSet critical;

  // push combined initial state on todo stack
  todog.push(*rPlantGen.InitStatesBegin());
  todoh.push(*rSupCandGen.InitStatesBegin());
  FD_DF("SupCon: todo push: (" << rPlantGen.SStr(*rPlantGen.InitStatesBegin()) << "|"
     << rSupCandGen.SStr(*rSupCandGen.InitStatesBegin()) << ")");

  // process todo stack
  while(!todog.empty()) {
    // allow for user interrupt
    // LoopCallback();
    // allow for user interrupt, incl progress report
    FD_WPC(1,2,"TdesControllability(): iterating states"); 
    // get top element from todo stack
    Idx currentg = todog.top();
    Idx currenth = todoh.top();
    todog.pop();
    todoh.pop();
    FD_DF("SupCon: todo pop: (" << rPlantGen.SStr(currentg) << "|" 
       << rSupCandGen.SStr(currenth) << ")");

    // break on doublets (tmoor 201104)
    if(processed.Exists(currenth)) continue;
    processed.Insert(currenth);

#ifdef FAUDES_DEBUG_FUNCTION
    TransSet::Iterator _titg, _tith;
    // print all transitions of current states
    FD_DF("SupCon: transitions from current states:");
    for (_titg = rPlantGen.TransRelBegin(currentg); _titg != rPlantGen.TransRelEnd(currentg); ++_titg) 
       FD_DF("SupCon: g: " << rPlantGen.SStr(_titg->X1) << "-" 
		 << rPlantGen.EStr(_titg->Ev) << "-" << rPlantGen.SStr(_titg->X2));
    for (_tith = rSupCandGen.TransRelBegin(currenth); _tith != rSupCandGen.TransRelEnd(currenth); ++_tith) 
       FD_DF("SupCon: h: " << rSupCandGen.SStr(_tith->X1) << "-" 
		 << rSupCandGen.EStr(_tith->Ev) << "-" << rSupCandGen.SStr(_tith->X2));
#endif 

    // loop vars
    TransSet::Iterator tith, tith_end, titg, titg_end;

    // test current state ...
    bool pass=false;

    // 1. figure disabled in the plant g by the loop g x h
    EventSet disabled;
    tith = rSupCandGen.TransRelBegin(currenth);
    tith_end = rSupCandGen.TransRelEnd(currenth);
    titg = rPlantGen.TransRelBegin(currentg);
    titg_end = rPlantGen.TransRelEnd(currentg);
    while( (tith != tith_end) && (titg != titg_end)) {
      if(tith->Ev > titg->Ev) {
        disabled.Insert(titg->Ev);
	++titg;
      } else if(tith->Ev == titg->Ev) {
	  ++titg;
	  ++tith;
      } else {
	++tith; // cannot happen
      }
    }
    
    // 2. pass, if all disabled are controllable
    if(disabled <= rCAlph) {
      pass=true;
    }

    // 3. is a forcible event enabled for the loop g x h ?
    bool fenabled=false;
    if(!pass) {
      tith = rSupCandGen.TransRelBegin(currenth);
      tith_end = rSupCandGen.TransRelEnd(currenth);
      titg = rPlantGen.TransRelBegin(currentg);
      titg_end = rPlantGen.TransRelEnd(currentg);
      while( (tith != tith_end) && (titg != titg_end)) {
        if(tith->Ev > titg->Ev) {
	  ++titg;
        } else if(tith->Ev == titg->Ev) {
	  if(rFAlph.Exists(titg->Ev)) {fenabled=true; break;} 
	  ++titg;
	  ++tith;
        } else {
	  ++tith; // cannot happen
	}
      }
    }

    // 4. pass if all uncontr. disabled events are preempted by a forcible event
    if((!pass) && fenabled) {
      if(disabled <= rCPAlph) pass=true;
    }

    // 5. record non-passed as critical
    if(!pass) {
       critical.Insert(currenth);
       FD_DF("SupCon: todo pop: (" << rPlantGen.SStr(currentg) << "|" 
       << rSupCandGen.SStr(currenth) << "): FAIL");
       continue;
    }

    // 6. if passed, stack successor state 
    if(pass) {
      // process all h transitions while there could be matching g transitions
      titg = rPlantGen.TransRelBegin(currentg);
      titg_end = rPlantGen.TransRelEnd(currentg);
      tith = rSupCandGen.TransRelBegin(currenth);
      tith_end = rSupCandGen.TransRelEnd(currenth);
      while ((tith != tith_end) && (titg != titg_end)) {
        // increment tith and titg case A: process common events
        if(titg->Ev == tith->Ev) {
	  FD_DF("SupCon: processing common event " << rPlantGen.EStr(titg->Ev));
	  // add to todo list if state not processed (tmoor 201104: record next H state on stack)
	  if(!processed.Exists(tith->X2)) {
	    todog.push(titg->X2);
	    todoh.push(tith->X2);
  	    FD_DF("SupCon: todo push: (" << rPlantGen.SStr(titg->X2) << "|"
	      << rSupCandGen.SStr(tith->X2) << ")");
	  }
          FD_DF("SupCon: incrementing g and h transrel");
          ++titg;
          ++tith;
        }
        // increment tith and titg case B: process g event that is not enabled in h
        else if (titg->Ev < tith->Ev) {
	  FD_DF("SupCon: incrementing g transrel");
	  ++titg;
        }
        // increment tith and titg case C: process h event that is not enabled in g
        else {
	  FD_DF("SupCon: incrementing h transrel"); // cannot happen
         ++tith;
        }
      } // end while tith and titg
    } // end if passed

  } //end while todog


  // remove all states that have been identified as critical or that have
  // been not processed
  critical = rSupCandGen.States() - ( processed - critical );
  rSupCandGen.DelStates(critical);
}






// SupTconUnchecked(rPlantGen, rCAlph, rFAlph, rPAlph, rSpecGen, rCompositionMap, rResGen)
void SupTconUnchecked(
  const Generator& rPlantGen,
  const EventSet& rCAlph,  
  const EventSet& rFAlph,  
  const EventSet& rPAlph,  
  const Generator& rSpecGen,
  std::map< std::pair<Idx,Idx>, Idx>& rCompositionMap, 
  Generator& rResGen) 
{
  FD_DF("SupTcon(" << &rPlantGen << "," << &rSpecGen << ")");

  // PREPARE RESULT:	
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  pResGen->Clear();
  pResGen->Name(CollapsString("SupTcon(("+rPlantGen.Name()+"),("+rSpecGen.Name()+"))"));
  pResGen->InjectAlphabet(rPlantGen.Alphabet());

  // controllable events
  FD_DF("SupTcon: controllable events: "   << rCAlph.ToString());
  FD_DF("SupTcon: forcible events: "       << rFAlph.ToString());
  FD_DF("SupTcon: preemptyble events: "    << rPAlph.ToString());

  // weackly controllable
  EventSet cpalph = rCAlph + rPAlph;

  // ALGORITHM:
  FD_DF("SupTcon(): SupConProduct on #" <<  rPlantGen.Size() << "/ #" << rSpecGen.Size());
  SupConProduct(rPlantGen, cpalph, rSpecGen, rCompositionMap, *pResGen);


  // make resulting generator trim until it's fully tdes controllable 
  while(true) {
    if(pResGen->Empty()) break;
    Idx state_num = pResGen->Size();
    FD_DF("SupTcon(): SupConClosed on #" <<  rPlantGen.Size() << "/ #" << pResGen->Size());
    SupConClosedUnchecked(rPlantGen, cpalph, *pResGen);
    FD_DF("SupTcon(): SupTcon on #" <<  rPlantGen.Size() << "/ #" << pResGen->Size());
    SupTconUnchecked(rPlantGen, rCAlph, rFAlph, rPAlph, cpalph, *pResGen);
    FD_DF("SupTcon(): Trim on #" << pResGen->Size());
    pResGen->Trim();
    if(pResGen->Size() == state_num) break;
  }

  // convenience state names
  if(rPlantGen.StateNamesEnabled() && rSpecGen.StateNamesEnabled() && rResGen.StateNamesEnabled()) 
    SetComposedStateNames(rPlantGen, rSpecGen, rCompositionMap, *pResGen);
  else
    pResGen->StateNamesEnabled(false);

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

}



// SupTcon(rPlantGen, rCAlph, rFAlph, rSpecGen, rResGen)
void SupTcon(
  const Generator& rPlantGen, 
  const EventSet& rCAlph, 
  const EventSet& rFAlph, 
  const EventSet& rPAlph, 
  const Generator& rSpecGen, 
  Generator& rResGen) 
{

  // CONSISTENCY CHECK:
  ControlProblemConsistencyCheck(rPlantGen, rCAlph, rSpecGen);

  // HELPERS:
  std::map< std::pair<Idx,Idx>, Idx> rcmap;

  // ALGORITHM:
  SupTconUnchecked(rPlantGen, rCAlph, rFAlph, rPAlph, rSpecGen, rcmap, rResGen);
}



// supcon for Systems
// uses and maintains controllablity from plant 
void SupTcon(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) {

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }

  // default preemptable but not controllable
  EventSet palph;
  palph.Insert("tick");
  EventSet calph(rPlantGen.ControllableEvents());
  calph=calph - palph;

  // execute
  SupTcon(rPlantGen, calph, rPlantGen.ForcibleEvents(), palph, rSpecGen,*pResGen);

  // copy all attributes of input alphabet
  pResGen->EventAttributes(rPlantGen.Alphabet());

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

}

} // name space 
