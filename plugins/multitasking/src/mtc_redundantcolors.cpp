/** @file mtc_redundantcolors.cpp

Methods for removing redundant colors for the supervisor synthesis from MtcSystems

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


#include "mtc_redundantcolors.h"

namespace faudes {

// SearchScc(state,rCount,rGen, rNewStates,rSTACK,rStackStates,rDFN,rLOWLINK,rSccSet,rRoots)
void SearchScc(
  const Idx state, 
  int& rCount,      // why is this a ref?
  const Generator& rGen,
  StateSet&  rNewStates,
  std::stack<Idx>& rSTACK, 
  StateSet& rStackStates,
  std::map<const Idx, int>& rDFN,
  std::map<const Idx, int>& rLOWLINK,
  std::set<StateSet>& rSccSet,
  StateSet& rRoots)
{
  FD_DF("SearchScc: -- search from state "<< state << "--");

  // mark state "old";
  rNewStates.Erase(state);
  // DFNUMBER[state] <- count;
  rDFN[state]=rCount;
  rCount++;
  // LOWLINK[v] <- DFNUMBER[v];
  rLOWLINK[state]=rDFN[state];
  // push state on STACK;
  rSTACK.push(state);
  rStackStates.Insert(state);
  //<<create set "L[state]" of successor states of state
  StateSet SuccStates = StateSet();
  TransSet::Iterator it = rGen.TransRelBegin(state);
  TransSet::Iterator it_end = rGen.TransRelEnd(state);
  for (; it != it_end; ++it) {
    SuccStates.Insert(it->X2);
  }
  // for each vertex *sit on L[state] do
  StateSet::Iterator sit = SuccStates.Begin();
  StateSet::Iterator sit_end = SuccStates.End();
  for (; sit != sit_end; ++sit)
    {
      // if *sit is marked "new" then
      if(rNewStates.Exists(*sit))
        {// begin
      // SearchC(*sit);
      SearchScc(*sit, rCount, rGen, rNewStates, rSTACK, rStackStates, rDFN, rLOWLINK, rSccSet, rRoots);
      // LOWLINK[state] <- MIN(LOWLINK[state],LOWLINK[*sit]);
      if(rLOWLINK[*sit]<rLOWLINK[state])
        {
          rLOWLINK[state]=rLOWLINK[*sit];
        }
        }//end
      else
        {
      // if DFNUMBER[*sit]<DFNUMBER[state] and [*sit] is on STACK then
      if((rDFN[*sit]<rDFN[state])&&(rStackStates.Exists(*sit)))
        {
          // LOWLINK[state]<-MIN(DFNUMBER[*sit],LOWLINK[state]);
          if(rDFN[*sit]<rLOWLINK[state])
        {
          rLOWLINK[state]=rDFN[*sit];
        }
        }                
        }
    }//end for
  // if LOWLINK[state]=DFNUMBER[state] (i.e. state is root of a SCC) then
  if(rLOWLINK[state]==rDFN[state])
    {
        
      //create SCC
      StateSet Scc;
      Idx top;
      // begin
      // repeat
      while(true)
        {// begin
            // pop x from top of STACK and print x;
            top=rSTACK.top();
            Scc.Insert(top);
            rStackStates.Erase(top);
            rSTACK.pop();
            // until x=state;
            if(top==state){
                // print "end of SCC", insert SCC into SCCset;
                rSccSet.insert(Scc);
                rRoots.Insert(state);
                break;
            }
        } //end while
    } // end if
}

//  ComputeSCC(rGen,rSCCSet,rRoots)
bool ComputeSCC(const Generator& rGen, std::set<StateSet>& rSCCSet, StateSet& rRoots){
    // helpers:
    StateSet newStates = rGen.States();
    int count = 1;
    std::stack<Idx> stack;
    StateSet stackStates;
    std::map<const Idx,int> DFN;
    std::map<const Idx, int> LOWLINK;
    // Search for SCCs until the list of new states is empty
    while(!newStates.Empty() ){
        SearchScc(*newStates.Begin(), count, rGen, newStates, stack, stackStates, DFN, LOWLINK, rSCCSet, rRoots);
    }
    if(rSCCSet.empty() )
        return false;
    else
        return true;
}

// ColoredSCC(rGen,rColors,rColoredSCCs)
void ColoredSCC(MtcSystem& rGen, ColorSet& rColors, std::set<StateSet>& rColoredSCCs){
    //helpers:
    StateSet roots;
    std::set<StateSet>::iterator stIt, stEndIt, tmpIt;
    ColorSet currentColors;
    StateSet::Iterator sIt, sEndIt;
    // first find all SCCs
    ComputeSCC(rGen, rColoredSCCs, roots);
    // Erase the SCCS that do not have all colors in rColors
    stIt = rColoredSCCs.begin();
    stEndIt = rColoredSCCs.end();
    // investigate each SCC
    for( ; stIt != stEndIt; ){
        currentColors.Clear();
        sIt = stIt->Begin();
        sEndIt = stIt->End();
        // collect the  colors from all states in the current SCC
        for( ; sIt != sEndIt; sIt++){
            currentColors = currentColors + rGen.Colors(*sIt);
        }
        // Remove the SCC if not all colors appear
        if(currentColors != rColors){
            tmpIt = stIt;
            stIt++;
            rColoredSCCs.erase(tmpIt);
            continue;
        }
        stIt++;
    }
}

// CheckRedundantColor(rGen,redundantColor)
bool CheckRedundantColor(MtcSystem rGen, Idx redundantColor){
    //helpers:
    ColorSet remainingColors = rGen.Colors();
    remainingColors.Erase(redundantColor);
    std::set<StateSet> coloredSCCs;
    std::set<StateSet>::const_iterator csIt, csEndIt;
    // Remove all states with redundantColor from a copy of rGen
    MtcSystem copyGen = rGen;
    StateSet copyStates = copyGen.States();
    TransSetX2EvX1 copyTransSet;
    copyGen.TransRel(copyTransSet);
    StateSet::Iterator stIt, stEndIt;
    stIt = copyStates.Begin();
    stEndIt = copyStates.End();
    for( ; stIt!= stEndIt; stIt++){
      // delete all states with the redundant color
      StateSet criticalStates;
      if( copyGen.ExistsColor(*stIt,redundantColor) ){
          StateSet::Iterator crIt;
          copyGen.DelState(*stIt);
          criticalStates.Clear();
          TraverseUncontrollableBackwards(copyGen.ControllableEvents(),copyTransSet,criticalStates, *stIt);	
          for(crIt = criticalStates.Begin(); crIt != criticalStates.End(); crIt++){
              copyGen.DelState(*crIt);
          }
      }
    }
    // Fixed point iteration that alternately computes SCCs and removes states with outgoing uncontrollable 
    // transitions from SCCs
    while(true){
        coloredSCCs.clear();
        // Determine the colored strongly connected components in the remaining generator copyGen
        ColoredSCC(copyGen, remainingColors, coloredSCCs);
        // if there are no colored SCCs, the color is redundant
        if(coloredSCCs.empty() ){
            return true;
        }
        // otherwise, the SCCs can be used to construct a strongly nonblocking and controllable subbehavior of rGen that is 
        // blocking w.r.t. the potentially redundant color (just remove all transitions that leave the SCCs in G)
        else{
            // go over all SCCs and remove states with outgoing uncontrollable transisions
            bool done = true;
            csIt = coloredSCCs.begin();
            csEndIt = coloredSCCs.end();
            // go over all states in the SCC
            for( ; csIt != csEndIt; csIt++){
                stIt = csIt->Begin();
                stEndIt = csIt->End();
                for( ; stIt != stEndIt; stIt++){
                    // check all transitions
                    TransSet::Iterator tIt, tEndIt;
                    tIt = copyGen.TransRelBegin(*stIt);
                    tEndIt = copyGen.TransRelEnd(*stIt);
                    for( ; tIt != tEndIt; tIt++){
                        if( (!copyGen.Controllable(tIt->Ev) && !csIt->Exists(tIt->X2) ) ){
                            done = false;
                            copyGen.DelState(*stIt);
                            break;                            
                        }
                    }
                }
            }
            if(done == true)
                return false;
        }
    }
}


// OptimalColorSet(rGen,rOptimalColors,rHighAlph)
void OptimalColorSet(const MtcSystem& rGen, ColorSet& rOptimalColors, EventSet& rHighAlph){
    std::vector<Idx> colorVector;
    ColorSet genColors = rGen.Colors();
    ColorSet::Iterator cIt, cEndIt;
    cIt = genColors.Begin();
    cEndIt = genColors.End();
    for( ; cIt != cEndIt; cIt++){
        colorVector.push_back(*cIt);
    }
    Idx vectorSize = colorVector.size();
    Idx optimalNumberStates, currentNumberStates, optimalNumberColors;
    // the abstracted generator has at most as many states as the original generator with the overall alphabet 
    // as high-level alphabet and all colors
    optimalNumberStates = rGen.Size(); 
    EventSet optimalAlph, currentHighAlph;
    optimalAlph = rGen.Alphabet();
    rOptimalColors = rGen.Colors();
    // check all colors for redundancy
    for(Idx i = 1; i <= vectorSize; i++){
        bool redundant = CheckRedundantColor(rGen, colorVector[i-1] );
        // if the current color can be removed, check the next colors (note that this is only possible if 
        // there are additional colors 
        if(redundant == true){
            MtcSystem reducedGen = rGen;
            reducedGen.DelColor(colorVector[i-1] );
            currentHighAlph = rHighAlph;
            currentNumberStates = calcNaturalObserver(reducedGen, currentHighAlph);
            // Set the optimal values if a smaller generator than before is achieved
            if(currentNumberStates < optimalNumberStates){
                optimalNumberStates = currentNumberStates;
                optimalAlph = currentHighAlph;
                rOptimalColors = reducedGen.Colors();
                optimalNumberColors = rOptimalColors.Size();
            }
            // if the abstraction has the same size but less colors are needed
            else if(currentNumberStates == optimalNumberStates && reducedGen.Colors().Size() < rOptimalColors.Size() ){
                optimalAlph = currentHighAlph;
                rOptimalColors = reducedGen.Colors();
                optimalNumberColors = rOptimalColors.Size();                
            }
            rec_OptimalColorSet(reducedGen, colorVector, i + 1, rOptimalColors, optimalNumberStates, optimalNumberColors, rHighAlph, optimalAlph); 
        }
    }
    rHighAlph = optimalAlph;
}

// rec_OptimalColorSet(rGen,rColorVector,colorNumber,rOptimalColors,rOptimalNumberStates,rOptimalNumberColors,rHighAlph,rOptimalHighAlph)
  void rec_OptimalColorSet(const MtcSystem& rGen, const std::vector<Idx>& rColorVector, Idx colorNumber, ColorSet& rOptimalColors, 
        Idx& rOptimalNumberStates, Idx& rOptimalNumberColors, const EventSet& rHighAlph, EventSet& rOptimalHighAlph){
    Idx vectorSize = rColorVector.size();
    EventSet currentHighAlph;
    Idx currentNumberStates;
    for(Idx i = colorNumber; i <= vectorSize; i++){
        bool redundant = CheckRedundantColor(rGen, rColorVector[i-1] );
        // if there are additional colors and the current color can be removed, check the next colors
        if(redundant == true){
            MtcSystem reducedGen = rGen;
            reducedGen.DelColor(rColorVector[i-1] );
            currentHighAlph = rHighAlph;
            currentNumberStates = calcNaturalObserver(reducedGen, currentHighAlph);
            // Set the optimal values if a smaller generator than before is achieved
            if(currentNumberStates < rOptimalNumberStates){
                rOptimalNumberStates = currentNumberStates;
                rOptimalHighAlph = currentHighAlph;
                rOptimalColors = reducedGen.Colors();
                rOptimalNumberColors = rOptimalColors.Size();
            }
            // if the abstraction has the same size but less colors are needed
            else if(currentNumberStates == rOptimalNumberStates && reducedGen.Colors().Size() < rOptimalColors.Size() ){
                rOptimalHighAlph = currentHighAlph;
                rOptimalColors = reducedGen.Colors();
                rOptimalNumberColors = rOptimalColors.Size();                
            }
            // if there are potential colors to be removed, call the recursive function
            if(i < vectorSize){
                rec_OptimalColorSet(reducedGen, rColorVector, colorNumber + 1, rOptimalColors, rOptimalNumberStates, rOptimalNumberColors, rHighAlph, rOptimalHighAlph); 
            }
        }       
    }
}

} // namespace faudes
