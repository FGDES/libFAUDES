/** @file omg_1_buechi.cpp 

Tutorial on basic operations on Buechi automata

@ingroup Tutorials 

@include omg_1_buechi.cpp

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
  // language inclusion
  ////////////////////////////

  std::cout << "################################\n";
  std::cout << "# tutorial, language inclusion\n";

  // recycle example ffrom parallel
  Generator bparallel_g1full=bparallel_g1;
  InvProject(bparallel_g1full,bparallel_g1g2.Alphabet());

  // do test and report
  bool langinc_a=BuechiLanguageInclusion(bparallel_g1g2,bparallel_g1full);
  bool langinc_b=BuechiLanguageInclusion(bparallel_g1full,bparallel_g1g2);
  bool langinc_c=BuechiLanguageEquality(bparallel_g1full,bparallel_g1g2);
  if(langinc_a) 
    std::cout << "# g1g2 subseteq g1full (expecte)\n";
  else
    std::cout << "# g1g2 not subseteq g1full (test case failed!)\n";
  if(langinc_b) 
    std::cout << "# g1full subseteq g1g2 (test case failed!))\n";
  else
    std::cout << "# g1full not subseteq g1g2 (expected)\n";
  if(langinc_c) 
    std::cout << "# g1full == g1g2 (test case failed!))\n";
  else
    std::cout << "# g1full not == g1g2 (expected)\n";

  // record
  FAUDES_TEST_DUMP("language inc a", langinc_a);
  FAUDES_TEST_DUMP("language inc b", langinc_b);
  FAUDES_TEST_DUMP("language inc c", langinc_c);
 
  
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
