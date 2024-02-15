/**
 * @file syn_6_compsynth.cpp
 * 
 * Test case and demo for compositional synthesis algorithm:
 *
 * @ingroup Tutorials 
*/

#include "libfaudes.h"

using namespace faudes;

/////////////////
// main program
/////////////////

int main() {

  ////////////////////////////////////////////////////////////////
  // prepare data
  ////////////////////////////////////////////////////////////////

  // helper
  Generator gen;

  // input1: plants
  GeneratorVector PlantGenVec;
  gen.Read("data/syn_manufacturing_w1.gen");
  PlantGenVec.PushBack(gen);
  gen.Read("data/syn_manufacturing_m1.gen");
  PlantGenVec.PushBack(gen);
  gen.Read("data/syn_manufacturing_w2.gen");
  PlantGenVec.PushBack(gen);
  gen.Read("data/syn_manufacturing_m2.gen");
  PlantGenVec.PushBack(gen);

  // input2: controllable events
  EventSet ConAlph;
  ConAlph.Insert("sus1");
  ConAlph.Insert("res1");
  ConAlph.Insert("s1");
  ConAlph.Insert("s2");
  ConAlph.Insert("s3");
  ConAlph.Insert("sus2");
  ConAlph.Insert("res2");

  // input3: specifications
  GeneratorVector SpecGenVec;
  gen.Read("data/syn_manufacturing_b1.gen");
  SpecGenVec.PushBack(gen);
  gen.Read("data/syn_manufacturing_b2.gen");
  SpecGenVec.PushBack(gen);

  // output1: map
  std::map<Idx,Idx> MapEventsToPlant;
  // output2: distinguishers
  GeneratorVector DisGenVec; 
  // output3: supervisors
  GeneratorVector SupGenVec; 

  ////////////////////////////////////////////////////////////////
  // run algorithm
  ////////////////////////////////////////////////////////////////

  CompositionalSynthesis(PlantGenVec, ConAlph, SpecGenVec, 
	   MapEventsToPlant, DisGenVec, SupGenVec);

  ////////////////////////////////////////////////////////////////
  // inspect result 
  ////////////////////////////////////////////////////////////////

  std::cout << "################################\n";
  std::cout << "Hello, this is a test case for compositional synthesis \n";
  std::cout << "################################\n";

  // SHOW the map
  SymbolTable* pEvSymTab = SupGenVec.At(0).EventSymbolTablep();
  std::map<Idx,Idx>::iterator mit = MapEventsToPlant.begin();
  std::cout << "################\n";
  std::cout << "this is map\n";
  std::cout << "################\n";
  for(; mit != MapEventsToPlant.end(); ++mit) {
    std::cout << "map    "<<pEvSymTab->Symbol(mit->first) << "    to ";
    std::cout << pEvSymTab->Symbol(mit->second) << std::endl;
  }

  // SHOW the distinguisher
  GeneratorVector::Position i=0;
  for(; i < DisGenVec.Size(); ++i) {
    DisGenVec.At(i).Write();
    // LOG
    FAUDES_TEST_DUMP("DisGenVec_i",DisGenVec.At(i));
  }    

  // SHOW the supervisor
  for(i=0; i < SupGenVec.Size(); ++i) {
    SupGenVec.At(i).Write();
    // LOG
    FAUDES_TEST_DUMP("SupGenVec_i",SupGenVec.At(i));
  }

  // SHOW the statistic
  Idx Size = 0;
  Idx TransRelSize = 0;
  for(i=0; i < SupGenVec.Size(); ++i) {
    Size += SupGenVec.At(i).Size();
    TransRelSize += SupGenVec.At(i).TransRelSize();
  }

  std::cout<<" the number of supervisors is "<<SupGenVec.Size()<<std::endl;
  std::cout<<" the total size is "<<Size<<std::endl;
  std::cout<<" the total number of TransRel is "<<TransRelSize<<std::endl;
  std::cout << "################################\n";
  
  ////////////////////////////////////////////////////////////////
  // monolithic representation
  ////////////////////////////////////////////////////////////////

  // There must exist at least one distinguisher-generator for the 
  // below routine to be functional (!)

  // single distinguisher
  Generator SigDisGen;
  aParallel(DisGenVec, SigDisGen);
  // single supervisor
  Generator SigSupGen;
  aParallel(SupGenVec, SigSupGen);

  // monolithic representation of supervisor with Sigma2 (alias)
  Generator My_Closed;
  Parallel(SigSupGen, SigDisGen, My_Closed);
  
  // restore original events (Sigma2 -> Sigma1)
  // construct inverse transition relation
  TransSetEvX1X2 invtr;
  My_Closed.TransRel(invtr);
  
  // if need 
  mit = MapEventsToPlant.begin();
  for(; mit != MapEventsToPlant.end(); ++mit) {
    if(mit->first == mit->second) continue;

    Idx kid = mit->first;
    Idx parent = mit->second;

    // insert parent
    My_Closed.InsEvent(parent);
    // replace transition 
    TransSetEvX1X2::Iterator tit = invtr.BeginByEv(kid);
    for(; tit != invtr.EndByEv(kid); ++tit)
      My_Closed.SetTransition(tit->X1, parent, tit->X2);
    // remove kid
    My_Closed.DelEvent(kid);
  }

  // LOG
  FAUDES_TEST_DUMP("Monolithic Representation",My_Closed);

  // SHOW the statistic
  std::cout << "the size of closed behavior is :\n";
  std::cout << "the states : " << My_Closed.Size() << std::endl;
  std::cout << "the TransRel : " << My_Closed.TransRelSize() << std::endl;
  std::cout << "################################\n";

  ////////////////////////////////////////////////////////////////
  // Controllablity and Nonblocking
  ////////////////////////////////////////////////////////////////

  Generator Plant;
  aParallel(PlantGenVec, Plant);

  bool my_iscon = true;
  my_iscon = IsControllable(Plant, ConAlph, My_Closed);

  bool my_isNB = true;
  my_isNB = IsNonblocking(My_Closed);

  // LOG
  FAUDES_TEST_DUMP("Controllablity Test",my_iscon);
  FAUDES_TEST_DUMP("Nonblocking Test",my_isNB);

  // SHOW
  std::cout <<"the supervisor is " << (my_iscon?"":" not ") << "controllable\n";
  std::cout <<"the supervisor is " << (my_isNB?"":" not ") << "nonblocking\n";

  FAUDES_TEST_DIFF();
  return 0;
}

