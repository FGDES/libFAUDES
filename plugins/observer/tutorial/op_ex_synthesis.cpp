/** @file op_ex_synthesis.cpp

Tutorial, synthesis of natural projections with certain properties. 

Thie tutorial describes the computation of a natural projection that is 
    - an Lm-observer
    - an Lm-observer and locally control consistent (LCC)
    - an Lm-observer and output control consistent (OCC)
The resulting abstractions also illustrate that the respective conditions imply each other:
Lm-observer and OCC => Lm-observer and LCC => Lm-observer

@ingroup Tutorials

@include op_ex_synthesis.cpp
*/

#include <stdio.h>
#include <iostream>
#include <libfaudes.h>


// make libFAUDES namespace available
using namespace faudes;

int main(int argc, char* argv[]) {
  /////////////////////////////
  //Synthesis of an Lm-observer
  /////////////////////////////
	
  // construct original Generator from file
  System genOrig = System("data/ex_synthesis/ex_natural_all.gen");
  genOrig.GraphWrite("data/ex_synthesis/ex_natural_all.png");
  // construct initial abstraction alphabet from file
  EventSet highAlph = EventSet("data/ex_synthesis/ex_natural_all.alph", "Alphabet");
  // compute an abstraction that is an Lm-observer while keeping the events in the original abstraction alphabet
  EventSet newHighAlph;
  System genClosed(genOrig);
  Generator genDyn;	
  // compute closed observer
  calcClosedObserver(genClosed,highAlph);
  System genClosedProj;
  aProject(genClosed,highAlph,genClosedProj);
  genClosedProj.Write("data/ex_synthesis/ex_natural_closed_proj.gen");
  genClosedProj.GraphWrite("data/ex_synthesis/ex_natural_closed_proj.png");    
  // compute natural observer
  highAlph = EventSet("data/ex_synthesis/ex_natural_all.alph", "Alphabet");
  calcNaturalObserver(genClosed,highAlph);
  highAlph.Write();
  aProject(genClosed,highAlph,genClosedProj);
  genClosedProj.Write("data/ex_synthesis/ex_natural_obs_proj.gen");
  genClosedProj.GraphWrite("data/ex_synthesis/ex_natural_obs_proj.png");
  //genClosedProj.GraphWrite("data/ex_synthesis/ex_natural_obs_proj.svg");
	
  // MSA-observer
  highAlph = EventSet("data/ex_synthesis/ex_natural_all.alph", "Alphabet");
  System genMSA(genOrig);
  Generator genDynMSA;	
  // compute natural observer
  calcMSAObserver(genClosed,highAlph);
  highAlph.Write();
  aProject(genClosed,highAlph,genClosedProj);
  genClosedProj.Write("data/ex_synthesis/ex_natural_msa_proj.gen");
  genClosedProj.GraphWrite("data/ex_synthesis/ex_natural_msa_proj.png");
	
  // Natural observer with LCC
  highAlph = EventSet("data/ex_synthesis/ex_natural_all.alph", "Alphabet");
  System genLCC(genOrig);
  Generator genDynLCC;
  EventSet controllableEvents;
  //controllableEvents.Insert("gamma");
  controllableEvents.Insert("a");
  controllableEvents.Insert("f");
  controllableEvents.Insert("g");
  controllableEvents.Insert("h");
  // compute natural observer with lcc
  calcNaturalObserverLCC(genLCC,controllableEvents, highAlph);
  highAlph.Write();
  aProject(genLCC,highAlph,genClosedProj);
  genClosedProj.Write("data/ex_synthesis/ex_natural_obslcc_proj.gen");
  genClosedProj.GraphWrite("data/ex_synthesis/ex_natural_obslcc_proj.png");
 
  // MSA observer with LCC
  highAlph = EventSet("data/ex_synthesis/ex_natural_all.alph", "Alphabet");
  genDynLCC.Clear();
  // compute natural observer with lcc
  calcMSAObserverLCC(genLCC,controllableEvents, highAlph);
  highAlph.Write();
  aProject(genLCC,highAlph,genClosedProj);
  genClosedProj.Write("data/ex_synthesis/ex_natural_msalcc_proj.gen");
  genClosedProj.GraphWrite("data/ex_synthesis/ex_natural_msalcc_proj.png");     


  // ========================================================
  // Observer comptations with event relabeling
  // ========================================================
  genOrig = System("data/ex_synthesis/ex_relabel_all.gen");
  genOrig.GraphWrite("data/ex_synthesis/ex_relabel_all.png");
  System genObs(genOrig);
  std::map< Idx, std::set< Idx > > mapRelabeledEvents;
  highAlph = EventSet("data/ex_synthesis/ex_natural_all.alph", "Alphabet");
  calcAbstAlphClosed(genObs, highAlph, newHighAlph, mapRelabeledEvents);
  // Write the resulting generator and the new high-level alphabet to a file
  genObs.Write("data/ex_synthesis/ex_relabel_closed_result.gen");
  genObs.GraphWrite("data/ex_synthesis/ex_relabel_closed_result.png");
  newHighAlph.Write("data/ex_synthesis/ex_relabel_closed_result.alph");	
  // evaluate the natural projection for the resulting generator and high-level alphabet
  System genHigh;
  aProject(genObs, newHighAlph, genHigh);
  // Write the high-level generator to a file
  genHigh.Write("data/ex_synthesis/ex_relabel_closed_high.gen");
  genHigh.GraphWrite("data/ex_synthesis/ex_relabel_closed_high.png");
  std::cout << "##########################################################################\n";
  std::cout << "# Lm-observer computed; the result can be found in the folder./results/ex_synthesis\n";
  std::cout << "##########################################################################\n";

  //////////////////////////////////////////////////////////////////////////////////
  // Synthesis of a natural projection that fulfills local control consistency (LCC)
  //////////////////////////////////////////////////////////////////////////////////
	
  // read the oritinal generator and high-level alphabetd from file input
  genObs = genOrig;
  highAlph.Read("data/ex_synthesis/ex_natural_all.alph", "Alphabet");
  // compute an abstraction that is an Lm-observer and LCC whilc keeping the events in the original abstraction alphabet
  newHighAlph.Clear();
  mapRelabeledEvents.clear();
  calcAbstAlphObs(genObs, highAlph, newHighAlph, mapRelabeledEvents);
  genObs.Write("data/ex_synthesis/ex_relabel_obs_result.gen");
  genObs.GraphWrite("data/ex_synthesis/ex_relabel_obs_result.png");
  newHighAlph.Write("data/ex_synthesis/ex_relabel_obs_result.alph");
  // evaluate the natural projection for the resulting generator and high-level alphabet
  aProject(genObs, newHighAlph, genHigh);
  // Write the high-level generator to a file
  genHigh.Write("data/ex_synthesis/ex_relabel_obs_high.gen");
  genHigh.GraphWrite("data/ex_synthesis/ex_relabel_obs_high.png");
  std::cout << "################################################################################\n";
  std::cout << "# Lm-observer computed; the result can be found in the folder./results/ex_synthesis\n";
  std::cout << "################################################################################\n";

  //////////////////////////////////////////////////////////////////////////////////
  // Synthesis of a natural projection that fulfills local control consistency (LCC)
  //////////////////////////////////////////////////////////////////////////////////
	
  // read the oritinal generator and high-level alphabetd from file input
  genObs = genOrig;
  highAlph.Read("data/ex_synthesis/ex_natural_all.alph", "Alphabet");
  // compute an abstraction that is an Lm-observer and LCC whilc keeping the events in the original abstraction alphabet
  newHighAlph.Clear();
  mapRelabeledEvents.clear();
  calcAbstAlphMSA(genObs, highAlph, newHighAlph, mapRelabeledEvents);
  genObs.Write("data/ex_synthesis/ex_relabel_msa_result.gen");
  genObs.GraphWrite("data/ex_synthesis/ex_relabel_msa_result.png");
  newHighAlph.Write("data/ex_synthesis/ex_relabel_msa_result.alph");
  // evaluate the natural projection for the resulting generator and high-level alphabet
  aProject(genObs, newHighAlph, genHigh);
  // Write the high-level generator to a file
  genHigh.Write("data/ex_synthesis/ex_relabel_msa_high.gen");
  genHigh.GraphWrite("data/ex_synthesis/ex_relabel_msa_high.png");
  std::cout << "################################################################################\n";
  std::cout << "# MSA-observer omputed; the result can be found in the folder./results/ex_synthesis\n";
  std::cout << "################################################################################\n";


  //////////////////////////////////////////////////////////////////////////////////
  // Synthesis of a natural projection that fulfills local control consistency (LCC) with relabeling
  //////////////////////////////////////////////////////////////////////////////////
	
  // read the oritinal generator and high-level alphabetd from file input
  genObs = genOrig;
  highAlph.Read("data/ex_synthesis/ex_natural_all.alph", "Alphabet");
  // compute an abstraction that is an Lm-observer and LCC whilc keeping the events in the original abstraction alphabet
  newHighAlph.Clear();
  mapRelabeledEvents.clear();
  calcAbstAlphObsLCC(genObs, highAlph, newHighAlph, mapRelabeledEvents);
  genObs.Write("data/ex_synthesis/ex_relabel_obslcc_result.gen");
  genObs.GraphWrite("data/ex_synthesis/ex_relabel_obslcc_result.png");
  newHighAlph.Write("data/ex_synthesis/ex_relabel_obslcc_result.alph");
  // evaluate the natural projection for the resulting generator and high-level alphabet
  aProject(genObs, newHighAlph, genHigh);
  // Write the high-level generator to a file
  genHigh.Write("data/ex_synthesis/ex_relabel_obslcc_high.gen");
  genHigh.GraphWrite("data/ex_synthesis/ex_relabel_obslcc_high.png");
  std::cout << "################################################################################\n";
  std::cout << "# Lm-observer with LCC computed; the result can be found in the folder./results/ex_synthesis\n";
  std::cout << "################################################################################\n";

  //////////////////////////////////////////////////////////////////////////////////
  // Synthesis of a natural projection that fulfills local control consistency (LCC)
  //////////////////////////////////////////////////////////////////////////////////
	
  // read the oritinal generator and high-level alphabetd from file input
  genObs = genOrig;
  highAlph.Read("data/ex_synthesis/ex_natural_all.alph", "Alphabet");
  // compute an abstraction that is an Lm-observer and LCC whilc keeping the events in the original abstraction alphabet
  newHighAlph.Clear();
  mapRelabeledEvents.clear();
  calcAbstAlphMSALCC(genObs, highAlph, newHighAlph, mapRelabeledEvents);
  genObs.Write("data/ex_synthesis/ex_relabel_msalcc_result.gen");
  genObs.GraphWrite("data/ex_synthesis/ex_relabel_msalcc_result.png");
  newHighAlph.Write("data/ex_synthesis/ex_relabel_msalcc_result.alph");
  // evaluate the natural projection for the resulting generator and high-level alphabet
  aProject(genObs, newHighAlph, genHigh);
  // Write the high-level generator to a file
  genHigh.Write("data/ex_synthesis/ex_relabel_msalcc_high.gen");
  genHigh.GraphWrite("data/ex_synthesis/ex_relabel_msalcc_high.png");
  std::cout << "################################################################################\n";
  std::cout << "# MSA-observer with LCC computed; the result can be found in the folder./results/ex_synthesis\n";
  std::cout << "################################################################################\n";

  return 0;
}
