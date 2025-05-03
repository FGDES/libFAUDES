/** @file 3_functions.cpp 

Tutorial, operations on genarators.

This tutorial  demonstrates the use of 
general purpose operations on generators.
It also produces example output for the HTML
documentation.

@ingroup Tutorials 

@include 3_functions.cpp
*/


#include "libfaudes.h"


// make the faudes namespace available to our program

using namespace faudes;



/////////////////
// main program
/////////////////

int main() {



  ////////////////////////////
  // make deterministic
  ////////////////////////////

     
  // read nondeterministic generator and  perform operation 
  Generator deter_nondet("data/deterministic_nondet.gen");
  Generator deter_det;
  Deterministic(deter_nondet, deter_det);
  deter_det.Write("tmp_deterministic_det.gen");
  deter_nondet.Write("tmp_deterministic_nondet.gen");

  // report result to console
  std::cout << "################################\n";
  std::cout << "# deterministic generator \n";
  deter_det.DWrite();
  std::cout << "################################\n";


  // Test protocol
  FAUDES_TEST_DUMP("deterministic",deter_det);


  ////////////////////////////
  // make minimal
  ////////////////////////////


  // read nonminimal generator and  perform operation 
  Generator minimal_nonmin("data/minimal_nonmin.gen");
  Generator minimal_min;
  StateMin(minimal_nonmin, minimal_min);
  minimal_nonmin.Write("tmp_minimal_nonmin.gen");
  minimal_min.Write("tmp_minimal_min.gen");

  // report result to console
  std::cout << "################################\n";
  std::cout << "# minimal generator \n";
  minimal_min.DWrite();
  std::cout << "################################\n";


  // Test protocol
  FAUDES_TEST_DUMP("minimal",minimal_min);

  ////////////////////////////
  // project
  ////////////////////////////


  // read generator to project 
  Generator project_g("data/project_g.gen");
  Generator project_prog;
  EventSet alph_proj;
  alph_proj.Insert("a");
  alph_proj.Insert("c");
  alph_proj.Insert("e");
  alph_proj.Insert("g");
  Project(project_g,  alph_proj, project_prog);
  project_g.Write("tmp_project_g.gen");

  // tmoor 201308: minimize stateset (for html docu, purely cosmetic)
  Generator project_prog_min;
  project_prog_min.StateNamesEnabled(false);
  StateMin(project_prog,project_prog_min); 
  project_prog_min.Write("tmp_project_prog.gen");

  // report result to console
  std::cout << "################################\n";
  std::cout << "# projected generator \n";
  project_prog_min.DWrite();
  std::cout << "################################\n";

  // Test protocol
  FAUDES_TEST_DUMP("project",project_prog_min);


  ////////////////////////////
  // synchronous composition
  ////////////////////////////


  // read generators
  Generator parallel_g1("data/parallel_g1.gen");
  Generator parallel_g2("data/parallel_g2.gen");

  // perform composition
  Generator parallel_g1g2;
  Parallel(parallel_g1, parallel_g2, parallel_g1g2);
  
  // write result and operands for html docu
  parallel_g1.Write("tmp_parallel_g1.gen");
  parallel_g2.Write("tmp_parallel_g2.gen");
  parallel_g1g2.Write("tmp_parallel_g1g2.gen");
 
  // read generators
  Generator parallel_m1("data/wparallel_g1.gen");
  Generator parallel_m2("data/wparallel_g2.gen");

  // perform composition
  Generator parallel_m1m2;
  Parallel(parallel_m1, parallel_m2, parallel_m1m2);
  
  // write result and operands for html docu
  parallel_m1.Write("tmp_parallel_m1.gen");
  parallel_m2.Write("tmp_parallel_m2.gen");
  parallel_m1m2.Write("tmp_parallel_m1m2.gen");
 
  // read generators
  Generator wparallel_g1("data/wparallel_g1.gen");
  Generator wparallel_g2("data/wparallel_g2.gen");

  // perform composition
  Generator wparallel_g1g2;
  OmegaParallel(wparallel_g1, wparallel_g2, wparallel_g1g2);
  
  // write result and operands for html docu
  wparallel_g1.Write("tmp_wparallel_g1.gen");
  wparallel_g2.Write("tmp_wparallel_g2.gen");
  wparallel_g1g2.Write("tmp_wparallel_g1g2.gen");

  // Test protocol
  FAUDES_TEST_DUMP("parallel",parallel_g1g2);
  FAUDES_TEST_DUMP("wparallel",wparallel_g1g2);


  ////////////////////////////
  // boolean language operations
  ////////////////////////////


  // read generators
  Generator boolean_g1("data/boolean_g1.gen");
  Generator boolean_g2("data/boolean_g2.gen");
  Generator lequal_g1("data/lequal_g1.gen");
  Generator lequal_g2("data/lequal_g2.gen");

  // perform union
  Generator boolean_union;
  boolean_union.StateNamesEnabled(false);
  LanguageUnion(boolean_g1, boolean_g2, boolean_union);
  
  // perform intersection
  Generator boolean_intersection;
  boolean_intersection.StateNamesEnabled(false);
  LanguageIntersection(boolean_g1, boolean_g2, boolean_intersection);
  
  // perform complement twice
  Generator boolean_complement_g1=boolean_g1; 
  boolean_complement_g1.StateNamesEnabled(false);
  LanguageComplement(boolean_complement_g1);
  Generator boolean_complement2_g1=boolean_complement_g1;
  LanguageComplement(boolean_complement2_g1);
  
  // write results and operands for html docu
  boolean_g1.Write("tmp_boolean_g1.gen");
  boolean_g2.Write("tmp_boolean_g2.gen");
  boolean_union.Write("tmp_union_g1g2.gen");
  boolean_intersection.Write("tmp_intersection_g1g2.gen");
  boolean_complement_g1.Write("tmp_complement_g1.gen");
  boolean_complement2_g1.Write("tmp_complement2_g1.gen");

  // inspect on console
  std::cout << "################################\n";
  std::cout << "# boolean language operations\n";
  boolean_union.DWrite();
  boolean_intersection.DWrite();
  boolean_complement_g1.DWrite();
  std::cout << "################################\n";

  // compare languages
  std::cout << "################################\n";
  std::cout << "# boolean language operations\n";
  if(LanguageInclusion(boolean_g1,boolean_union))  
    std::cout << "Lm(g1) <= Lm(g1) v Lm(g2): OK\n";
  else
    std::cout << "Lm(g1) <= Lm(g1) v Lm(g2): ERR\n";
  if(LanguageDisjoint(boolean_complement_g1,boolean_g1))  
    std::cout << "Lm(g1) ^ ~Lm(g1) = empty: OK\n";
  else
    std::cout << "(Lm(g1) v Lm(g2)) ^ ~(Lm(g1) v Lm(g2)) != empty: ERR\n";
  if(LanguageEquality(boolean_g1,boolean_complement2_g1))  
    std::cout << "Lm(g1) = ~~Lm(g1) : OK\n";
  else
    std::cout << "Lm(g1) != ~~Lm(g1) : ERR\n";
  std::cout << "################################\n";

  // compare languages (track error report, thanks)
  std::cout << "################################\n";
  std::cout << "# boolean language operations for empty Lm(g1)\n";
  if(LanguageInclusion(lequal_g1,lequal_g2))  
    std::cout << "Lm(g1) <= Lm(g2): OK\n";
  else
    std::cout << "Lm(g1) <= Lm(g2): ERR\n";
  if(LanguageInclusion(lequal_g2,lequal_g1))  
    std::cout << "Lm(g2) <= Lm(g1): ERR\n";
  else
    std::cout << "~(Lm(g2) <= Lm(g1)): OK\n";
  bool boolean_lequal = LanguageEquality(lequal_g1,lequal_g2);
  if(boolean_lequal)  
    std::cout << "Lm(g1) == Lm(g2): ERR\n";
  else
    std::cout << "Lm(g1) != Lm(g2): OK\n";

  // Record test case
  FAUDES_TEST_DUMP("boolean union",boolean_union);
  FAUDES_TEST_DUMP("boolean inter",boolean_intersection);
  FAUDES_TEST_DUMP("boolean compl",boolean_complement_g1);
  FAUDES_TEST_DUMP("boolean equal",boolean_lequal);


  ////////////////////////////
  // language difference
  ////////////////////////////
  

  // read generator and write for html docu
  Generator difference_g1("data/difference_g1.gen");
  difference_g1.Write("tmp_difference_g1.gen");
  
  Generator difference_g2("data/difference_g2.gen");
  difference_g2.Write("tmp_difference_g2.gen");
    
  // compute language difference Lm(g1)-Lm(g2)
  
  Generator difference_g1minusg2;
  LanguageDifference(difference_g1,difference_g2,difference_g1minusg2);
  difference_g1minusg2.Write("tmp_difference_g1minusg2.gen");

  // inspect on console 
  std::cout << "################################\n";
  std::cout << "# language difference Lm(g1)-Lm(g2) \n";
  difference_g1minusg2.DWrite();
  std::cout << "################################\n";

  // Record test case
  FAUDES_TEST_DUMP("difference",difference_g1minusg2);


  ////////////////////////////
  // convert to automaton
  ////////////////////////////
  

  // read generator and write for html docu
  Generator automaton_g("data/automaton_g.gen");
  automaton_g.Write("tmp_automaton_g.gen");
  
  // convert to automaton
  Automaton(automaton_g);
  automaton_g.Write("tmp_automaton_gRes.gen");

  // inspect on console 
  std::cout << "################################\n";
  std::cout << "# automaton from generator \n";
  automaton_g.DWrite();
  std::cout << "################################\n";
  
  // Record test case  
  FAUDES_TEST_DUMP("automaton ", automaton_g);
  

  ////////////////////////////
  // language concatenation
  ////////////////////////////


  // read generators and write for html docu
  Generator concat_g1("data/concat_g1.gen");
  Generator concat_g2("data/concat_g2.gen");
  Generator concat_g3("data/concat_g3.gen");
  Generator concat_g4("data/concat_g4.gen");
  Generator concat_g5("data/concat_g5.gen");
  Generator concat_g6("data/concat_g6.gen");
  concat_g1.Write("tmp_concat_g1.gen");
  concat_g2.Write("tmp_concat_g2.gen");
  concat_g3.Write("tmp_concat_g3.gen");
  concat_g4.Write("tmp_concat_g4.gen");
  concat_g5.Write("tmp_concat_g5.gen");
  concat_g6.Write("tmp_concat_g6.gen");


  // perform language concatenations and inspect on console
  std::cout << "################################\n";
  std::cout << "# language concatenation\n";
  Generator concat;
  concat.StateNamesEnabled(false);
  LanguageConcatenate(concat_g1,concat_g3,concat);
  concat.Write("tmp_concat_g1g3.gen");
  concat.DWrite();
  LanguageConcatenate(concat_g1,concat_g4,concat);
  concat.Write("tmp_concat_g1g4.gen");
  concat.DWrite();
  LanguageConcatenate(concat_g2,concat_g3,concat);
  concat.Write("tmp_concat_g2g3.gen");
  concat.DWrite();
  LanguageConcatenate(concat_g2,concat_g4,concat);
  concat.Write("tmp_concat_g2g4.gen");
  concat.DWrite();
  LanguageConcatenate(concat_g5,concat_g6,concat);
  concat.Write("tmp_concat_g5g6.gen");
  concat.DWrite();
  std::cout << "################################\n";

  // Record test case  
  FAUDES_TEST_DUMP("concat ", concat);
  

  ////////////////////////////
  // full language, alphabet language, empty string language and empty language
  ////////////////////////////
  
  // create alphabet={a,b}
  EventSet languages_alphabet;
  languages_alphabet.Insert("a");
  languages_alphabet.Insert("b");
  
  // generator for result
  Generator languages_g;
  
  // construct full language from alphabet
  FullLanguage(languages_alphabet,languages_g);
  languages_g.Write("tmp_languagesFull_result.gen");

  // inspect on console
  std::cout << "################################\n";
  std::cout << "# full language \n";
  languages_g.DWrite();
  
  // construct alphabet language from alphabet
  AlphabetLanguage(languages_alphabet,languages_g);
  languages_g.Write("tmp_languagesAlphabet_result.gen");
  std::cout << "# alphabet language \n";
  languages_g.DWrite();
  
  // construct empty string language from alphabet
  EmptyStringLanguage(languages_alphabet,languages_g);
  languages_g.Write("tmp_languagesEmptyString_result.gen");
  std::cout << "# empty string language \n";
  languages_g.DWrite();
  
  // construct empty language from alphabet
  EmptyLanguage(languages_alphabet,languages_g);
  languages_g.Write("tmp_languagesEmpty_result.gen");
  std::cout << "# empty language \n";
  languages_g.DWrite();
  std::cout << "################################\n";



  ////////////////////////////
  // Kleene closure
  ////////////////////////////
  

  // read generator and write for html docu
  Generator kleene_g1("data/kleene_g1.gen");
  kleene_g1.Write("tmp_kleene_g1.gen");
  
  // compute Kleene closure
  kleene_g1.StateNamesEnabled(false);
  KleeneClosure(kleene_g1);
  kleene_g1.Write("tmp_kleene_g1Res.gen");

  // inspect on console 
  std::cout << "################################\n";
  std::cout << "# Kleene closure \n";
  kleene_g1.DWrite();
  std::cout << "################################\n";
  
  // Record test case  
  FAUDES_TEST_DUMP("kleene closure ", kleene_g1);
  

  // read generator and write for html docu
  Generator kleene_g2("data/kleene_g2.gen");
  kleene_g2.Write("tmp_kleene_g2.gen");
  
  // compute Kleene closure
  kleene_g2.StateNamesEnabled(false);
  KleeneClosureNonDet(kleene_g2);
  kleene_g2.Write("tmp_kleene_g2Res.gen");

  // inspect on console 
  std::cout << "################################\n";
  std::cout << "# Kleene closure \n";
  kleene_g2.DWrite();
  std::cout << "################################\n";
  
  // Record test case  
  FAUDES_TEST_DUMP("kleene closure ", kleene_g2);
  
  ////////////////////////////
  // prefix closure
  ////////////////////////////
  

  // read generator and write for html docu
  Generator prefixclosure_g("data/prefixclosure_g.gen");
  prefixclosure_g.Write("tmp_prefixclosure_g.gen");

  // test prefix closure
  bool isprefixclosed_g = IsPrefixClosed(prefixclosure_g);
  
  // compute prefix closure
  PrefixClosure(prefixclosure_g);
  prefixclosure_g.Write("tmp_prefixclosure_gRes.gen");

  // test prefix closure
  bool isprefixclosed_gRes = IsPrefixClosed(prefixclosure_g);

  // inspect on console 
  std::cout << "################################\n";
  std::cout << "# prefix closure \n";
  if(isprefixclosed_g) 
     std::cout << "# argument was prefix closed (test case error!)\n";
  else
     std::cout << "# argument was not prefix closed (expected)\n";
  if(isprefixclosed_gRes) 
     std::cout << "# result is prefix closed (expected)\n";
  else
     std::cout << "# result is not prefix closed (test case error!)\n";
  prefixclosure_g.DWrite();
  std::cout << "################################\n";
  
  // Record test case  
  FAUDES_TEST_DUMP("prefix closure", prefixclosure_g);


  ////////////////////////////
  // omega closure
  ////////////////////////////
  

  // read generator and write for html docu
  Generator omegaclosure_g("data/omegaclosure_g.gen");
  omegaclosure_g.Write("tmp_omegaclosure_g.gen");
  
  // test omega closure
  bool isomegaclosed_g = IsOmegaClosed(omegaclosure_g);
  
  // compute omega closure
  OmegaClosure(omegaclosure_g);
  omegaclosure_g.Write("tmp_omegaclosure_gRes.gen");

  // test omega closure
  bool isomegaclosed_gRes = IsOmegaClosed(omegaclosure_g);
  
  // inspect on console 
  std::cout << "################################\n";
  std::cout << "# omega closure \n";
  if(isomegaclosed_g) 
     std::cout << "# argument was omega closed (test case error!)\n";
  else
     std::cout << "# argument was not omega closed (expected)\n";
  if(isomegaclosed_gRes) 
     std::cout << "# result is omega closed (expected)\n";
  else
     std::cout << "# result is not omega closed (test case error!)\n";
  omegaclosure_g.DWrite();
  std::cout << "################################\n";
  
  // Record test case  
  FAUDES_TEST_DUMP("omega closure", omegaclosure_g);


  ////////////////////////////
  // selfloop
  ////////////////////////////
  

  // read generator and write for html docu
  Generator selfloop_g("data/selfloop_g.gen");
  selfloop_g.Write("tmp_selfloop_g.gen");
  
  // create alphabet={e,f}
  EventSet selfloop_alphabet;
  selfloop_alphabet.Insert("e");
  selfloop_alphabet.Insert("f");
  
  // generator for result
  Generator selfloop_gRes=selfloop_g;
  
  // add selfloops in each state
  SelfLoop(selfloop_gRes,selfloop_alphabet);
  selfloop_gRes.Write("tmp_selfloop_gRes.gen");

  // inspect on console 
  std::cout << "################################\n";
  std::cout << "# selfloop, each state \n";
  selfloop_gRes.DWrite();
  
  // add selfloops in marked states
  selfloop_gRes=selfloop_g;
  SelfLoopMarkedStates(selfloop_gRes,selfloop_alphabet);
  selfloop_gRes.Write("tmp_selfloopMarked_gRes.gen");
  std::cout << "# selfloop, marked states \n";
  selfloop_gRes.DWrite();
  
  // add selfloops in inital state(s)
  selfloop_gRes=selfloop_g;
  SelfLoop(selfloop_gRes,selfloop_alphabet,selfloop_gRes.InitStates());
  selfloop_gRes.Write("tmp_selfloopInit_gRes.gen");
  std::cout << "# selfloop, init state(s) \n";
  selfloop_gRes.DWrite();
  std::cout << "################################\n";


  ////////////////////////////
  // nonconflicting
  ////////////////////////////

  // under construction --

  std::cout << "################################\n";
  std::cout << "# non-blocking test \n";


  std::cout << "# conflict equiv. abstract test 1 \n";
  Generator nonblocking_g;
  EventSet silent;
  silent.Insert("tau");
  nonblocking_g.Read("data/noblo_gae2.gen");
  ConflictEquivalentAbstraction(nonblocking_g,silent);
  nonblocking_g.Write();

  FAUDES_TEST_DUMP("nonblocking 0", nonblocking_g);

  // test cases from Michael Meyer's thesis
  // Nonblocking: 	(1,2,3,6,7); (1,2,3,4,5,6,7); (1,2,3,4,5,6); (2,4,6,8); (2,3,4,5); (5,6,7,8,9)
  // Blocking:		(3,4,5,6,7); (1,3,5,7)

  // read generators from file 
  GeneratorVector nonblocking_gv;
  nonblocking_gv.PushBack("data/noblo_g2.gen");
  nonblocking_gv.PushBack("data/noblo_g3.gen");
  nonblocking_gv.PushBack("data/noblo_g4.gen");
  nonblocking_gv.PushBack("data/noblo_g5.gen");
  // test for conflicts
  std::cout << "# non-blocking exec test 1\n";
  bool nonblocking_ok1;
  nonblocking_ok1=IsNonblocking(nonblocking_gv);
  if(nonblocking_ok1) 
     std::cout << "# non-blocking test: passed (expected)\n";
  else
     std::cout << "# non-blocking test: failed (test case error!)\n";

  FAUDES_TEST_DUMP("nonblocking 1", nonblocking_ok1);

  // read generators from file 
  nonblocking_gv.Clear();
  nonblocking_gv.PushBack("data/noblo_g5.gen");
  nonblocking_gv.PushBack("data/noblo_g6.gen");
  nonblocking_gv.PushBack("data/noblo_g7.gen");
  nonblocking_gv.PushBack("data/noblo_g8.gen");
  nonblocking_gv.PushBack("data/noblo_g9.gen");
  // test for conflicts
  std::cout << "# non-blocking exec test 2\n";
  bool nonblocking_ok2;
  nonblocking_ok2=IsNonblocking(nonblocking_gv);
  if(nonblocking_ok2) 
     std::cout << "# non-blocking test: passed (expected)\n";
  else
     std::cout << "# non-blocking test: failed (test case error!)\n";

  FAUDES_TEST_DUMP("nonblocking 2", nonblocking_ok2);

  // read generators from file
  nonblocking_gv.Clear();
  nonblocking_gv.PushBack("data/noblo_g3.gen");
  nonblocking_gv.PushBack("data/noblo_g4.gen");
  nonblocking_gv.PushBack("data/noblo_g5.gen");
  nonblocking_gv.PushBack("data/noblo_g6.gen");
  nonblocking_gv.PushBack("data/noblo_g7.gen");
  // test for conflicts
  std::cout << "# non-blocking exec test 3\n";
  bool nonblocking_ok3;
  nonblocking_ok3=IsNonblocking(nonblocking_gv);
  if(nonblocking_ok3) 
     std::cout << "# non-blocking test: passed (test case error!)\n";
  else
     std::cout << "# non-blocking test: failed (expected)\n";

  FAUDES_TEST_DUMP("nonblocking 3", nonblocking_ok3);

  // read generators from file
  nonblocking_gv.Clear();
  nonblocking_gv.PushBack("data/noblo_g1.gen");
  nonblocking_gv.PushBack("data/noblo_g3.gen");
  nonblocking_gv.PushBack("data/noblo_g5.gen");
  nonblocking_gv.PushBack("data/noblo_g7.gen");
  // test for conflicts
  std::cout << "# non-blocking exec test 4\n";
  bool nonblocking_ok4;
  nonblocking_ok4=IsNonblocking(nonblocking_gv);
  if(nonblocking_ok4) 
    std::cout << "# non-blocking test: passed (test case error!)\n";
  else
    std::cout << "# non-blocking test: failed (expected)\n";

  FAUDES_TEST_DUMP("nonblocking 4", nonblocking_ok4);

  // read generators from file 
  nonblocking_gv.Clear();
  nonblocking_gv.PushBack("data/noblo_g1.gen");
  nonblocking_gv.PushBack("data/noblo_g2.gen");
  nonblocking_gv.PushBack("data/noblo_g3.gen");
  nonblocking_gv.PushBack("data/noblo_g6.gen");
  nonblocking_gv.PushBack("data/noblo_g7.gen");
  // test for conflicts
  std::cout << "# non-blocking exec test 5\n";
  bool nonblocking_ok5;
  nonblocking_ok5=IsNonblocking(nonblocking_gv);
  if(nonblocking_ok5) 
     std::cout << "# non-blocking test: passed (expected)\n";
  else
     std::cout << "# non-blocking test: failed (test case error!)\n";
  
  // read generators from file
  nonblocking_gv.Clear();
  nonblocking_gv.PushBack("data/noblo_g1.gen");
  nonblocking_gv.PushBack("data/noblo_g2.gen");
  nonblocking_gv.PushBack("data/noblo_g3.gen");
  nonblocking_gv.PushBack("data/noblo_g4.gen");
  nonblocking_gv.PushBack("data/noblo_g5.gen");
  nonblocking_gv.PushBack("data/noblo_g6.gen");
  nonblocking_gv.PushBack("data/noblo_g7.gen");
  // test for conflicts
  std::cout << "# non-blocking exec test 6\n";
  bool nonblocking_ok6;
  nonblocking_ok6=IsNonblocking(nonblocking_gv);
  if(nonblocking_ok6) 
     std::cout << "# non-blocking test: passed (expected)\n";
  else
     std::cout << "# non-blocking test: failed (test case error!)\n";

  // read generators from file 
  nonblocking_gv.Clear();
  nonblocking_gv.PushBack("data/noblo_g1.gen");
  nonblocking_gv.PushBack("data/noblo_g2.gen");
  nonblocking_gv.PushBack("data/noblo_g3.gen");
  nonblocking_gv.PushBack("data/noblo_g4.gen");
  nonblocking_gv.PushBack("data/noblo_g5.gen");
  nonblocking_gv.PushBack("data/noblo_g6.gen");
  // test for conflicts
  std::cout << "# non-blocking exec test 7\n";
  bool nonblocking_ok7;
  nonblocking_ok7=IsNonblocking(nonblocking_gv);
  if(nonblocking_ok7) 
     std::cout << "# non-blocking test: passed (expected)\n";
  else
     std::cout << "# non-blocking test: failed (test case error!)\n";

  // read generators from file 
  nonblocking_gv.Clear();
  nonblocking_gv.PushBack("data/noblo_g2.gen");
  nonblocking_gv.PushBack("data/noblo_g4.gen");
  nonblocking_gv.PushBack("data/noblo_g6.gen");
  nonblocking_gv.PushBack("data/noblo_g8.gen");
  // test for conflicts
  std::cout << "# non-blocking exec test 8\n";
  bool nonblocking_ok8;
  nonblocking_ok8=IsNonblocking(nonblocking_gv);
  if(nonblocking_ok8) 
     std::cout << "# non-blocking test: passed (expected)\n";
  else
     std::cout << "# non-blocking test: failed (test case error!)\n";
  

  // test cases from Yiheng Tang (blocking)
  nonblocking_gv.Clear();
  nonblocking_gv.PushBack("data/noblo_yt1.gen");
  nonblocking_gv.PushBack("data/noblo_yt2.gen");

  // test for conflicts A
  std::cout << "# non-blocking exec test 9a\n";
  Generator parallel12;
  Parallel(nonblocking_gv.At(0),nonblocking_gv.At(1),parallel12);
  bool nonblocking_ok9;
  nonblocking_ok9=parallel12.BlockingStates().Size()==0;
  if(nonblocking_ok9) 
    std::cout << "# non-blocking test: passed (test case error!)\n";
  else
    std::cout << "# non-blocking test: failed (expected)\n";

  FAUDES_TEST_DUMP("nonblocking 9a", nonblocking_ok9);
  
  // test for conflicts B
  std::cout << "# non-blocking exec test 9b\n";
  nonblocking_ok9=IsNonblocking(nonblocking_gv);
  if(nonblocking_ok9) 
     std::cout << "# non-blocking test: passed (test case error!)\n";
  else
     std::cout << "# non-blocking test: failed (expected)\n";

  FAUDES_TEST_DUMP("nonblocking 9b", nonblocking_ok9);


  FAUDES_TEST_DIFF()

  // say good bye    
  std::cout << "done.\n";
  return 0;
}



