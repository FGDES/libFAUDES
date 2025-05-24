/** @file syn_supcmpl.cpp Supremal complete sublanguage for infinite time behaviours */

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

#include "syn_supcmpl.h"
#include "syn_supcon.h"
#include "syn_supnorm.h"
#include "syn_functions.h"

#include "omg_include.h"

namespace faudes {



/*
***************************************************************************************
***************************************************************************************
 Implementation SupConCmplClosed
***************************************************************************************
***************************************************************************************
*/


// supcon complete
void SupConCmplClosed(
  const Generator& rPlantGen, 
  const EventSet&  rCAlph,
  const Generator& rSpecGen, 
  Generator& rResGen)
{ 
  FD_DF("SupConCmplClosed(" << rPlantGen.Name() << "," << rSpecGen.Name()<< ")");
  
  // exceptions on invalid parameters, same as std synthesis
  ControlProblemConsistencyCheck(rPlantGen,rCAlph,rSpecGen);  

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  pResGen->Clear();
  pResGen->Name("SupConCmplClosed("+rPlantGen.Name()+", "+rSpecGen.Name()+")");
  pResGen->InjectAlphabet(rPlantGen.Alphabet());
  
  //check for trivial result
  if(rSpecGen.InitStatesEmpty()){
    FD_DF("SupConCmplClosed: empty language specification - empty result.");
  }  
    

  // have a reverse composition map
  std::map< std::pair<Idx,Idx>, Idx> revmap;

  // parallel composition (result is reachable)
  SupConProduct(rPlantGen, rCAlph, rSpecGen, revmap, *pResGen);

  // make resulting generator complete and controllabel
  while(true) {
    Idx state_num = pResGen->Size();
    if(pResGen->InitStates().Empty()) break;
    pResGen->Complete();
    if(pResGen->InitStates().Empty()) break;
    SupConClosedUnchecked(rPlantGen, rCAlph, *pResGen);
    if(pResGen->Size() == state_num) break;
  }

  // convenience state names
  if(rPlantGen.StateNamesEnabled() && rSpecGen.StateNamesEnabled() && rResGen.StateNamesEnabled()) 
    SetComposedStateNames(rPlantGen, rSpecGen, revmap, *pResGen);
  else
    pResGen->StateNamesEnabled(false);

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

}


// user wrapper
void SupConCmplClosed(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) 
{

  // execute 
  SupConCmplClosed(rPlantGen, rPlantGen.ControllableEvents(),rSpecGen,rResGen);

  // copy all attributes of input alphabet
  rResGen.EventAttributes(rPlantGen.Alphabet());

}




/*
***************************************************************************************
***************************************************************************************
 Implementation SupConCmplNB
***************************************************************************************
***************************************************************************************
*/


// supcon complete
void SupConCmplNB(
  const Generator& rPlantGen, 
  const EventSet&  rCAlph,
  const Generator& rSpecGen, 
  Generator& rResGen)
{ 
  FD_DF("SupConCmplNB(" << rPlantGen.Name() << "," << rSpecGen.Name()<< ")");
  
  // exceptions on invalid parameters, same as std synthesis
  ControlProblemConsistencyCheck(rPlantGen,rCAlph,rSpecGen);  

  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  pResGen->Clear();
  pResGen->Name("SupConCmplNB("+rPlantGen.Name()+", "+rSpecGen.Name()+")");
  pResGen->InjectAlphabet(rPlantGen.Alphabet());
  
  //check for trivial result
  if(rSpecGen.InitStatesEmpty()){
    FD_DF("SupConCmplNB: empty language specification - empty result.");
  }  
    

  // have a reverse composition map
  std::map< std::pair<Idx,Idx>, Idx> revmap;

  // parallel composition (result is reachable)
  SupConProduct(rPlantGen, rCAlph, rSpecGen, revmap, *pResGen);

  // make resulting generator complete and controllabel and coaccessible
  while(true) {
    Idx state_num = pResGen->Size();
    if(pResGen->InitStates().Empty()) break;
    pResGen->Coaccessible();
    if(pResGen->InitStates().Empty()) break;
    pResGen->Complete();
    if(pResGen->InitStates().Empty()) break;
    SupConClosedUnchecked(rPlantGen, rCAlph, *pResGen);
    if(pResGen->Size() == state_num) break;
  }

  // convenience state names
  if(rPlantGen.StateNamesEnabled() && rSpecGen.StateNamesEnabled() && rResGen.StateNamesEnabled()) 
    SetComposedStateNames(rPlantGen, rSpecGen, revmap, *pResGen);
  else
    pResGen->StateNamesEnabled(false);

  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }

}


// user wrapper
void SupConCmplNB(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) 
{

  // execute 
  SupConCmplNB(rPlantGen, rPlantGen.ControllableEvents(),rSpecGen,rResGen);

  // copy all attributes of input alphabet
  rResGen.EventAttributes(rPlantGen.Alphabet());

}



/*
***************************************************************************************
***************************************************************************************
 Implementation SupConNormCmplNB
***************************************************************************************
***************************************************************************************
*/


//  SupConNormCmplNB(rL,rCAlph,rOAlph,rK,rResult)
void SupConNormCmplNB(
  const Generator& rL,
  const EventSet& rCAlph,
  const EventSet& rOAlph,
  const Generator& rK, 
  Generator& rResult)
{
  FD_DF("SupConNormCmplNB(" << rL.Name() << "," << rK.Name() << ")");
  // initialize: K0
  Generator K0;
  K0.StateNamesEnabled(false);
  Product(rL,rK,K0);
  K0.Coaccessible();
  // initialize: closure(rL)
  Generator L=rL;
  L.StateNamesEnabled(false);
  L.Trim();
  MarkAllStates(L);
  // loop
  Generator Ki=K0;
  Ki.StateNamesEnabled(false);
  while(1) {
    FD_DF("SupConNormCmplNB(" << rL.Name() << "," << rK.Name() << "): #" << Ki.Size() << " m#" << Ki.MarkedStatesSize());
    // keep copy of recent
    rResult=Ki;
    // cheep closure (for coreachable generator)
    Ki.InjectMarkedStates(Ki.States());
    // synthesise closed
    SupConNormClosed(L,rCAlph,rOAlph,Ki,Ki);
    Complete(Ki);
    // restrict
    Product(K0,Ki,Ki);
    Ki.Coaccessible();
    // test (sequence is decreasing anyway)
    if(LanguageInclusion(rResult,Ki)) break;
  }  
  FD_DF("SupConNormCmplNB(" << rL.Name() << "," << rK.Name() << "): done");
}  


/** rti wrapper */
void SupConNormCmplNB(
  const System& rPlantGen, 
  const Generator& rSpecGen, 
  Generator& rResGen) 
{
  FD_DF("SupConNormCmplNB(" << rPlantGen.Name() << "," << rSpecGen.Name() << "): rti wrapper");
  // prepare result
  Generator* pResGen = &rResGen;
  if(&rResGen== &rPlantGen || &rResGen== &rSpecGen) {
    pResGen= rResGen.New();
  }
  // execute
  SupConNormCmplNB(rPlantGen,rPlantGen.ControllableEvents(),rPlantGen.ObservableEvents(),rSpecGen,*pResGen);
  // copy all attributes of input alphabet
  pResGen->EventAttributes(rPlantGen.Alphabet());
  // copy result
  if(pResGen != &rResGen) {
    pResGen->Move(rResGen);
    delete pResGen;
  }
}




} // name space 
