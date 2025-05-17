/** @file omg_1_buechi.cpp 

Tutorial on basic operations on Buechi automata

@ingroup Tutorials 

@include omp_1_buechi.cpp

*/

#include "libfaudes.h"


using namespace faudes;



int main() {

  ///////////////////////////////////
  // Buechi Trim
  ///////////////////////////////////

  // read example generator for reachability analysis
  Generator greach("data/omg_nottrim.gen");

  std::cout << "################################\n";
  std::cout << "# tutorial, buechi trim  \n";

  bool isotrim = IsBuechiTrim(greach);
  if(isotrim) 
    std::cout << "w-trimness: ok [error]\n";
  else
    std::cout << "w-trimness: failed [expected]\n";
  std::cout << "################################\n";

  // trim generator
  Generator gbtrim;
  BuechiTrim(greach,gbtrim);

  // write operand and results for html docu
  greach.Write("tmp_omg_notbtrim.gen");
  gbtrim.Write("tmp_omg_gobtrim.gen");

  // record test case
  FAUDES_TEST_DUMP("buechi trim",isotrim);

  
  ////////////////////////////
  // synchronous composition
  ////////////////////////////

  // read generators
  Generator bparallel_g1("data/omg_bparallel_g1.gen");
  Generator bparallel_g2("data/omg_bparallel_g2.gen");

  // perform composition w.r.t buechi acceptance
  Generator bparallel_g1g2;
  BuechiParallel(bparallel_g1, bparallel_g2, bparallel_g1g2);
  
  // write result and operands for html docu
  bparallel_g1.Write("tmp_omg_bparallel_g1.gen");
  bparallel_g2.Write("tmp_omg_bparallel_g2.gen");
  bparallel_g1g2.Write("tmp_omg_bparallel_g1g2.gen");

  // Test protocol
  FAUDES_TEST_DUMP("bparallel",bparallel_g1g2);
  FAUDES_TEST_DUMP("buechi parallel",bparallel_g1g2);


  
  ////////////////////////////
  // closure w.r.t. Buechi acceptance
  ////////////////////////////
  

  // Read generator and write for html docu
  Generator closure_g("data/omg_bclosure_g.gen");
  closure_g.Write("tmp_omg_bclosure_g.gen");
  
  // test buechi closure
  bool isclosed_g = IsBuechiClosed(closure_g);
  
  // compute buechi closure
  BuechiClosure(closure_g);
  closure_g.Write("tmp_omg_bclosure_gRes.gen");

  // test buechi closure
  bool isclosed_gRes = IsBuechiClosed(closure_g);
  
  // inspect on console 
  std::cout << "################################\n";
  std::cout << "# buechi closure \n";
  if(isclosed_g) 
     std::cout << "# argument was buechi closed (test case error!)\n";
  else
     std::cout << "# argument was not buechi closed (expected)\n";
  if(isclosed_gRes) 
     std::cout << "# result is buechi closed (expected)\n";
  else
     std::cout << "# result is not buechi closed (test case error!)\n";
  closure_g.DWrite();
  std::cout << "################################\n";
  
  // Record test case  
  FAUDES_TEST_DUMP("buechi closure", closure_g);




}
