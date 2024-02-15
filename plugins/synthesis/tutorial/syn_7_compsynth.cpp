/**
 * @file syn_7_compsynth.cpp
 * 
 * Test case for compositional synthesis algorithm:
 * Abfuellanlage 

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
  faudes::Generator gen;

  // input1: plants
  faudes::GeneratorVector PlantGenVec;
  gen.Read("data/syn_lfeed.gen");
  PlantGenVec.PushBack(gen);
  gen.Read("data/syn_lplace2.gen");
  PlantGenVec.PushBack(gen);
  gen.Read("data/syn_lplace3.gen");
  PlantGenVec.PushBack(gen);
  gen.Read("data/syn_lplace4.gen");
  PlantGenVec.PushBack(gen);
  gen.Read("data/syn_lplace5.gen");
  PlantGenVec.PushBack(gen);
  gen.Read("data/syn_lplace6.gen");
  PlantGenVec.PushBack(gen);
  gen.Read("data/syn_lplace7.gen");
  PlantGenVec.PushBack(gen);
  gen.Read("data/syn_lexit7.gen");
  PlantGenVec.PushBack(gen);

  // input2: controllable events
  faudes::EventSet ConAlph;
  ConAlph.Insert("V");
  ConAlph.Insert("R");

  // input3: specifications
  faudes::GeneratorVector SpecGenVec;
  gen.Read("data/syn_efeeda.gen");
  SpecGenVec.PushBack(gen);
  gen.Read("data/syn_eexit7.gen");
  SpecGenVec.PushBack(gen);

  // output1: map
  std::map<faudes::Idx,faudes::Idx> MapEventsToPlant;
  // output2: distinguishers
  faudes::GeneratorVector DisGenVec; 
  // output3: supervisors
  faudes::GeneratorVector SupGenVec; 

  ////////////////////////////////////////////////////////////////
  // run algorithm
  ////////////////////////////////////////////////////////////////

  CompositionalSynthesis(PlantGenVec, ConAlph, SpecGenVec, 
	   MapEventsToPlant, DisGenVec, SupGenVec);

  ////////////////////////////////////////////////////////////////
  // inspect result 
  ////////////////////////////////////////////////////////////////
  using namespace faudes;

  std::cout << "################################\n";
  std::cout << "Hello, this is a test case for compositional synthesis \n";
  std::cout << "################################\n";

  // SHOW the map
  faudes::SymbolTable* pEvSymTab = SupGenVec.At(0).EventSymbolTablep();
  std::map<faudes::Idx,faudes::Idx>::iterator mit = MapEventsToPlant.begin();
  std::cout << "################\n";
  std::cout << "this is map\n";
  std::cout << "################\n";
  for(; mit != MapEventsToPlant.end(); ++mit) {
    std::cout << "map    "<<pEvSymTab->Symbol(mit->first) << "    to ";
    std::cout << pEvSymTab->Symbol(mit->second) << std::endl;
  }

  // SHOW the distinguisher
  faudes::GeneratorVector::Position i=0;
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
  faudes::Idx Size = 0;
  faudes::Idx TransRelSize = 0;
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
  
  // !!ATTENTION: There must exist at least one distinguisher-generator
  // in this case no distinguisher

  /*
  // single distinguisher
  faudes::Generator SigDisGen;
  faudes::aParallel(DisGenVec, SigDisGen);

  // single supervisor
  faudes::Generator SigSupGen;
  faudes::aParallel(SupGenVec, SigSupGen);

  // monolithic representation of supervisor with Sigma2 (alias)
  faudes::Generator My_Closed;
  faudes::Parallel(SigSupGen, SigDisGen, My_Closed);
  
  // restore original events (Sigma2 -> Sigma1)
  // construct inverse transition relation
  faudes::TransSetEvX1X2 invtr;
  My_Closed.TransRel(invtr);
  
  // if need 
  mit = MapEventsToPlant.begin();
  for(; mit != MapEventsToPlant.end(); ++mit) {
    if(mit->first == mit->second) continue;

    faudes::Idx kid = mit->first;
    faudes::Idx parent = mit->second;

    // insert parent
    My_Closed.InsEvent(parent);
    // replace transition 
    faudes::TransSetEvX1X2::Iterator tit = invtr.BeginByEv(kid);
    for(; tit != invtr.EndByEv(kid); ++tit)
      My_Closed.SetTransition(tit->X1, parent, tit->X2);
    // remove kid
    My_Closed.DelEvent(kid);
  }
  */

  faudes::Generator My_Closed;
  faudes::aParallel(SupGenVec,My_Closed);

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

  faudes::Generator Plant;
  faudes::aParallel(PlantGenVec, Plant);

  bool my_iscon = false;
  my_iscon = faudes::IsControllable(Plant, ConAlph, My_Closed);

  bool my_isNB = false;
  my_isNB = faudes::IsNonblocking(My_Closed);

  // LOG
  FAUDES_TEST_DUMP("Controllablity Test",my_iscon);
  FAUDES_TEST_DUMP("Nonblocking Test",my_isNB);

  // SHOW
  std::cout <<"the supervisor is " << (my_iscon?"":" not ") << "controllable\n";
  std::cout <<"the supervisor is " << (my_isNB?"":" not ") << "nonblocking\n";

  FAUDES_TEST_DIFF();

  return 0;
}

