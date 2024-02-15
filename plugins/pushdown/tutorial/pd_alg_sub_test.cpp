/** @file pd_alg_nb_sub_a_test.cpp  Unit Tests */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/
#include "pd_alg_sub_test.h"

namespace faudes {


/* *****************
 * TestAnnotatePdStatesRenaming
 * *****************/
void TestRenameStatesRenaming(){
  std::string name = "AnnotatePdStates Renaming";
  std::cout << "Testing " << name << " ..." <<  std::endl;

  std::string renameString = "old";

  PushdownGenerator g1 = TestGenerator1();
  PushdownGenerator g2 = AnnotatePdStates(renameString,g1);

  try{
    StateSet::Iterator its;
    for(its = g2.StatesBegin(); its != g2.StatesEnd(); its++){

      const MergeStateAnnotation* msa = dynamic_cast<const MergeStateAnnotation*>(g2.StateAttribute(*its).Merge());

      if(msa == NULL)
        throw Exception(name, "MergeStateAnnotation not set.", 1003);
      if(msa->Annotation().compare(renameString) != 0){
        std::stringstream errstr;
        errstr << "Annotation incorrect, was " << msa->Annotation() << ", but " << renameString << " was expected." << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
  }
  catch (Exception e){
  }
  std::cout << "Finished " << name << std::endl;
}

/* *****************
 * TestAnnotatePdStatesNumberOfStates
 * *****************/
void TestRenameStatesNumberOfStates(){
  std::string name = "AnnotatePdStates Number of States";
  std::cout << "Testing " << name << " ..." <<  std::endl;

  std::string renameString = "old";

  PushdownGenerator g1 = TestGenerator1();
  PushdownGenerator g2 = AnnotatePdStates(renameString,g1);

  try{
    if(g1.Size() != g2.Size()){
        std::stringstream errstr;
        errstr << "Incorrect, number of states, was " << g1.Size() << ", but " << g2.Size() << " was expected." << std::endl;
        throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){
  }
  std::cout << "Finished " << name << std::endl;
}

/* *****************
 * TestRenameStackSymboldRenamingStackSymbols
 * *****************/
void TestRenameStackSymbolsRenamingStackSymbols(){
  std::string name = "RenameStackSymbold Renaming Stack Symbols";
  std::cout << "Testing " << name << " ..." <<std::endl;

  std::string renameString = "old";

  PushdownGenerator g1 = TestGenerator1();
  PushdownGenerator g2 = RenameStackSymbold(renameString,g1);

  try{
    StackSymbolSet::Iterator it;
    for(it = g1.StackSymbolsBegin(); it != g1.StackSymbolsEnd(); it++){
      //lambda does not get renamed so test for not getting renamed instead
      if(g2.IsStackSymbolLambda(*it)){
        if(!g2.StackSymbols().Exists(g2.StackSymbolName(*it))){
          std::stringstream errstr;
          errstr << "Annotation incorrect, " << g2.StackSymbolName(*it) << " was expected, but not found in the stack symbol list:\n" ;
          StackSymbolSet::Iterator iterr;
          for(iterr = g2.StackSymbolsBegin(); iterr != g2.StackSymbolsEnd(); iterr++){
            errstr << " " << g2.StackSymbolName(*iterr);
          }
          errstr << std::endl;
          throw Exception(name, errstr.str(), 1003);
        }
      }
      //all other symbols must get renamed
      else{
        if(!g2.StackSymbols().Exists(renameString + "-" + g2.StackSymbolName(*it))){
          std::stringstream errstr;
          errstr << "Annotation incorrect, " << renameString << "-" << g2.StackSymbolName(*it) << " was expected, but not found in the stack symbol list:\n" ;
          StackSymbolSet::Iterator iterr;
          for(iterr = g2.StackSymbolsBegin(); iterr != g2.StackSymbolsEnd(); iterr++){
            errstr << " " << g2.StackSymbolName(*iterr);
          }
          errstr << std::endl;
          throw Exception(name, errstr.str(), 1003);
      }
      }

    }
  }
  catch(Exception e){
  }
  std::cout << "Finished " << name << std::endl;
}

/* *****************
 * TestRenameStackSymboldNumberOfStackSymbols
 * *****************/
void TestRenameStackSymbolsNumberOfStackSymbols(){
  std::string name = "RenameStackSymbold Number of Stack Symbols";
  std::cout << "Testing " << name << " ..." <<  std::endl;

  std::string renameString = "old";

  PushdownGenerator g1 = TestGenerator1();
  PushdownGenerator g2 = RenameStackSymbold(renameString,g1);

  try{
     if(g1.StackSymbols().Size() != g2.StackSymbols().Size()){
        std::stringstream errstr;
        errstr << "Incorrect number of stack symbols, was " << g1.StackSymbols().Size() << ", but " << g2.StackSymbols().Size() << " was expected." << std::endl;
        throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){
  }
  std::cout << "Finished " << name << std::endl;
}

/* *****************
 * TestRenameStackSymboldRenamingTransitions
 * *****************/
void TestRenameStackSymbolsRenamingTransitions(){

  FAUDES_TEST_DUMP("test 77",true)

  std::string name = "RenameStackSymbold Renaming Transitions";
  std::cout << "Testing " << name << " ..." <<  std::endl;

  std::string renameString = "old";

  PushdownGenerator g1 = TestGenerator1();
  PushdownGenerator g2 = RenameStackSymbold(renameString,g1);

  TransSet::Iterator tit;
  std::vector<Idx> oldPush, oldPop, push, pop;
  std::vector<Idx>::const_iterator pushit, popit;
  PopPushSet popPush;
  PopPushSet::const_iterator ppit;

  try{
    for(tit = g2.TransRelBegin(); tit != g2.TransRelEnd(); tit++){
      //examine all pop/push pairs
      popPush = g2.PopPush(*tit);
      for(ppit = popPush.begin(); ppit != popPush.end(); ppit++){

        //test pop stack symbols
        oldPop = ppit->first;
        for(popit = oldPop.begin(); popit != oldPop.end(); popit++){

          //everything but lambda must be renamed
          if(!g2.IsStackSymbolLambda(*popit)){
            std::string annotation = g2.StackSymbolName(*popit).substr(0, renameString.size() + 1);
            if(annotation.compare(renameString + "-") != 0){
              std::stringstream errstr;
              errstr << "Annotation incorrect, " << renameString << "- was expected, but symbol was " << g2.StackSymbolName(*popit) << std::endl;
              throw Exception(name, errstr.str(), 1003);
            }
          }
        }

        //test push stack symbols
        oldPush = ppit->second;
        for(pushit = oldPush.begin(); pushit != oldPush.end(); pushit++){

          //everything but lambda must be renamed
          if(!g2.IsStackSymbolLambda(*pushit)){
            std::string annotation = g2.StackSymbolName(*pushit).substr(0, renameString.size() + 1);
            if(annotation.compare(renameString + "-") != 0){
              std::stringstream errstr;
              errstr << "Annotation incorrect, " << renameString << "- was expected, but symbol was " << g2.StackSymbolName(*pushit) << std::endl;
              throw Exception(name, errstr.str(), 1003);
            }
          }
        }
      }

    }
  }
  catch(Exception e){
      FAUDES_TEST_DUMP("test 77 ERROR ",false)
  }
  std::cout << "Finished " << name << std::endl;
}

/* *****************
 * TestRemoveLambdaPopNoLambdaPop
 * *****************/
void TestRemoveLambdaPopNoLambdaPop(){
  std::string name = "RemoveLambdaPop No Lambda Pop";
  std::cout << "Testing " << name << " ..." <<  std::endl;

  PushdownGenerator g1 = TestGenerator1();
  PushdownGenerator g2 = RemoveLambdaPop(g1);

  try{

    //iterate over all transitions
    TransSet::Iterator tit;
    PopPushSet::const_iterator ppsit;
    for(tit = g2.TransRel().Begin(); tit != g2.TransRel().End(); tit++){

      //get all PopPush pairs, extract pop (first), extract foremost
      //stack symbol (front), and test for lambda
      for(ppsit = g2.PopPush(*tit).begin(); ppsit != g2.PopPush(*tit).end(); ppsit++){
        if(g2.IsStackSymbolLambda(ppsit->first.front())){
          std::stringstream errstr;
          errstr << "Lambda popping edges not removed, Lambda pop found in transition" << g2.TransRel().Str(*tit) << std::endl;
          throw Exception(name, errstr.str(), 1003);
        }
      }
    }
  }
   catch(Exception e){
  }
  std::cout << "Finished " << name << std::endl;
}

/* *****************
 * TestRemoveLambdaPopAllExpectingTransition
 * *****************/
void TestRemoveLambdaPopAllExpectingTransition(){
  std::string name = "RemoveLambdaPop All Expecting Transition";
  std::cout << "Testing " << name << " ..." <<  std::endl;

  PushdownGenerator g1 = TestGenerator1();
  PushdownGenerator g2 = RemoveLambdaPop(g1);

  //test g1 for lambda transition
  TransSet::Iterator tit;
  PopPushSet::const_iterator ppsit;
  bool hasLambda;
  for(tit = g1.TransRel().Begin(); tit != g1.TransRel().End(); tit++){

    //get all PopPush pairs, extract pop (first), extract foremost
    //stack symbol (front), and test for lambda
    for(ppsit = g1.PopPush(*tit).begin(); ppsit != g1.PopPush(*tit).end(); ppsit++){
      if(g2.IsStackSymbolLambda(ppsit->first.front())){
        hasLambda = true;
        //lambda was found, no need to search further
        break;
      }
    }
  }

  if(!hasLambda){
      std::cout << "warning: cannot perform test, because there is no lambda pop in the original generator" << std::endl;
      return;
  }

  //test g2 for transition (s1,ev,s2,u,uw), where u is every stack symbol
  //except for lambda and bottom
  try{

    //iterate over all transitions
    std::vector<Idx> pop, push;
    std::vector<Idx>::const_iterator popit, pushit;
    StackSymbolSet::Iterator ssit;
    bool transFound, symbolFound;
    for(tit = g2.TransRel().Begin(); tit != g2.TransRel().End(); tit++){
      transFound = true;

      //iterate over all relevant stack symbols and see if there is one PopPush
      //pair with pop u and push wu
      for(ssit = g2.StackSymbols().Begin(); ssit != g2.StackSymbols().End(); ssit++){

        //lambda and stack bottom are not relevant
        if (*ssit == g2.StackBottom() || g2.IsStackSymbolLambda(*ssit)) continue;

        //test PopPush pairs and find at least one (u,uw) PopPush pair per symbol
        symbolFound = false;
        for(ppsit = g2.PopPush(*tit).begin(); ppsit != g2.PopPush(*tit).end(); ppsit++){

          pop = ppsit->first;
          push = ppsit->second;
          //if front of pop or push are  identical, the pair is found
          if(pop.front() == *ssit && *push.rbegin() == *ssit){
            symbolFound = true;
            break;
          }
        }
        //if any one symbol has not been found, this transition is not relevant
        if(!symbolFound){
          transFound = false;
          break;
        }
      }
      //no need to look for another such transition
      if(transFound) break;
    }

    if(!transFound){
      std::stringstream errstr;
      errstr << "Original generator has lambda popping edge, but result generator has no edge that accepts all stack symbols" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
  }
  std::cout << "Finished " << name << std::endl;
}

/* ****************
 * TestRemoveMultPopNumberOfTransitions
 * *****************/
void TestRemoveMulPopNumberOfStatesTransitions(){

  std::string name = "RemoveMultPop Number of States and Transitions";
  TestStart(name);

  PushdownGenerator g1 = TestGenerator4();

  //expected resulst for this particular test generator
  Idx expectedNumberTransitions = 11;
  Idx expectedNumberStates = 10;

  PushdownGenerator g2 = RemoveMultPop(g1);

  try{

    //test for number of states
    if(g2.Size() != expectedNumberStates){
      std::stringstream errstr;
      errstr << "Number of states  was " << g2.Size() << ", but " << expectedNumberStates << " was expected." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }

    //test for number of transitions
    if(g2.TransRelSize() != expectedNumberTransitions){
      std::stringstream errstr;
      errstr << "Number of transitions incorrect" << g2.TransRelSize() << ", but " << expectedNumberTransitions << " was expected." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){
  }

  TestEnd(name);
}

/* ****************
 * TestSPDAReadPopPushOnly
 * *****************/
void TestSPDANumberStatesTransitions(){
  //TODO dont know what to expect
//   std::string name = "SPDA Number States Transitions";
//   TestStart(name);
//
//   PushdownGenerator g1 = TestGenerator6();
//
//   PushdownGenerator g2 = SPDA(g1);
//   try{
//     //number of states must be 7
//     if(g2.Size() != 7){
//       std::stringstream errstr;
//       errstr << "Number of states was expected to be 7, but was " << g2.Size() << "." << std::endl;
//       throw Exception(name, errstr.str(), 1003);
//     }
//
//     TransSet::Iterator transit;
//     PopPushSet::const_iterator ppit;
//     //number of transitions must be 10
//     int transCount = 0;
//     for(transit = g2.TransRelBegin(); transit != g2.TransRelEnd(); transit++){
//       transCount += g2.PopPush(*transit).size();
//     }
//
//     if(transCount != 10){
//       std::stringstream errstr;
//       errstr << "Number of transitions was expected to be 10, but was " << transCount << "." << std::endl;
//       throw Exception(name, errstr.str(), 1003);
//     }
//   }
//   catch (Exception e){
//   }
//   TestEnd(name);
}

/* ****************
 * TestSPDAReadPopPushOnly
 * *****************/
void TestSPDAReadPopPushOnly(){
  std::string name = "SPDA Read Pop Push Only";
  TestStart(name);

  PushdownGenerator g1 = TestGenerator6();
  PushdownGenerator g2 = SPDA(g1);

  try{
    TransSet::Iterator transit;
    PopPushSet::const_iterator ppit;
    std::vector<Idx> pop, push;
    std::vector<Idx>::const_iterator ssit;
    for(transit = g2.TransRelBegin(); transit != g2.TransRelEnd(); transit++){
      for(ppit = g2.PopPushBegin(*transit); ppit != g2.PopPushEnd(*transit); ppit++){

        //read pop and push for convenience
        pop = ppit->first;
        push = ppit->second;

        //check for read only transition
        if(!g2.IsEventLambda(transit->Ev) &&
          pop == push){
          continue;
        }
        //check for pop only transition
        else if(g2.IsEventLambda(transit->Ev) &&
          !g2.IsStackSymbolLambda(pop.front()) &&
          pop.size() == 1 &&
          g2.IsStackSymbolLambda(push.front())){
          continue;
        }
        //check for push only transition
        else if(g2.IsEventLambda(transit->Ev) &&
          push.size() == 2 &&
          pop.size() == 1 &&
          pop.front() == push.back()){
          continue;
        }
        //error
        else{
          std::stringstream errstr;
          errstr << "Transition (" << transit->X1 << ", " << g2.EventName(transit->Ev) << ", " << transit->X2 << ") with pop [";
          for(ssit = pop.begin(); ssit != pop.end(); ssit++){
            errstr << " " << g2.StackSymbolName(*ssit);
          }
          errstr << "] and push [";
          for(ssit = push.begin(); ssit != push.end(); ssit++){
            errstr << " " << g2.StackSymbolName(*ssit);
          }
          errstr << "] was neither read nor pop nor push." << std::endl;
          throw Exception(name, errstr.str(), 1003);
        }
      }
    }
  }
  catch (Exception e){
  }
  TestEnd(name);
}

/* ****************
 * TestNdaActivePassive
 * *****************/
void TestNdaActivePassive(){
  std::string name = "Nda Active Passive";
  TestStart(name);

  PushdownGenerator g1 = TestGenerator7();
  PushdownGenerator g2 = Nda(g1);

  try{
    //the number of transitions must have doubled
    if(2*g1.States().Size() != g2.States().Size()){
      std::stringstream errstr;
      errstr << "Number of states incorrect, was" << g2.States().Size() << ", but " << 2*g1.States().Size() << " was expected." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }

    StateSet::Iterator stateit;
    int active = 0;
    int passive = 0;
    //there must be an equal amount of active and passive states
    for(stateit = g2.StatesBegin(); stateit != g2.StatesEnd(); stateit++){

      const MergeStateAnnotation* msa = dynamic_cast<const MergeStateAnnotation*>(g2.StateAttribute(*stateit).Merge());

      if(msa == NULL)
        throw Exception(name, "MergeStateAnnotation not set.", 1003);

      if(msa->Annotation().compare("active") == 0){
        active++;
      }
      else if(msa->Annotation().compare("passive") == 0){
        passive++;
      }
      else{
        std::stringstream errstr;
        errstr << "Annotation incorrect, was " << msa->Annotation() << ", but either active or passive was expected." << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
    if(active != passive){
      std::stringstream errstr;
      errstr << "There were " <<  active << " active states and " << passive << " passive states, but equal numbers were expected." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){
  }

  TestEnd(name);
}

/* ****************
 * TestNdaTransitions
 * *****************/
void TestNdaTransitions(){
  std::string name = "Nda Transitions";
  TestStart(name);

  PushdownGenerator g1 = TestGenerator7();
  PushdownGenerator g2 = Nda(g1);

  //put together expected transitions for later comparison
  std::set<std::pair<Idx,Idx> > expectedTransitions;
  expectedTransitions.insert(std::make_pair(1,3));
  expectedTransitions.insert(std::make_pair(1,5));
  expectedTransitions.insert(std::make_pair(2,3));
  expectedTransitions.insert(std::make_pair(2,6));
  expectedTransitions.insert(std::make_pair(3,8));
  expectedTransitions.insert(std::make_pair(4,8));
  expectedTransitions.insert(std::make_pair(5,3));
  expectedTransitions.insert(std::make_pair(6,4));
  expectedTransitions.insert(std::make_pair(7,6));
  expectedTransitions.insert(std::make_pair(8,6));

  try{
    TransSet::Iterator transit;
    for(transit = g2.TransRelBegin(); transit != g2.TransRelEnd(); transit++){

      if(expectedTransitions.erase(std::make_pair(transit->X1, transit->X2)) == 0){
        std::stringstream errstr;
        errstr << "Transition from state " << transit->X1 << " to state " << transit->X2 << " found, but was not expected." << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }

    if(expectedTransitions.size() != 0){
      std::stringstream errstr;
      errstr << "Not all expected transitions were found." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){
  }

  TestEnd(name);
}

/* *****************
 * TestAnnotatePdStates
 * *****************/
void TestAnnotatePdStates(){
  TestRenameStatesRenaming();
  //TestAnnotatePdStatesNumberOfStates();
}

/* *****************
 * TestRenameStackSymbold
 * *****************/
void TestRenameStackSymbols(){
  TestRenameStackSymbolsRenamingStackSymbols();
  TestRenameStackSymbolsNumberOfStackSymbols();
  TestRenameStackSymbolsRenamingTransitions();
}

/* *****************
 * TestRemoveLambdaPop
 * *****************/
void TestRemoveLambdaPop(){
  TestRemoveLambdaPopNoLambdaPop();
  TestRemoveLambdaPopAllExpectingTransition();
}

/* ****************
 * TestSPDA
 * *****************/
void TestSPDA(){
  TestSPDANumberStatesTransitions();
  TestSPDAReadPopPushOnly();
}

/* *****************
 * TestRemoveMultPop
 * *****************/
void TestRemoveMultPop(){
  TestRemoveMulPopNumberOfStatesTransitions();
}

/* *****************
 * TestNda
 * *****************/
void TestNda(){

  TestNdaActivePassive();
  TestNdaTransitions();
}

/* *****************
 * TestFilterMixedGrammarSymbols
 * *****************/
void TestFilterMixedGrammarSymbols(){
  
  std::string name = "Filter Mixed Grammar Symbols";
  TestStart(name);
  
  Grammar gr1 = TestGrammar1();
  
  GrammarSymbolVector word;
  std::set<Terminal>::const_iterator tit;
  std::vector<TerminalPtr> tpv;
  std::vector<TerminalPtr>::iterator tpvit;
  std::set<Nonterminal> rSet;
  
  //build word to filter
  //contains one nonterminal and all terminals
  Terminal* t;
  Nonterminal* nt = new Nonterminal(*gr1.Nonterminals().begin());
  GrammarSymbolPtr ntPtr(nt);
  word.push_back(ntPtr);

  for(tit = gr1.Terminals().begin(); tit != gr1.Terminals().end(); tit++){
    //create new terminal
    t = new Terminal(*tit);
    TerminalPtr tPtr(t);
    //push terminal into word
    word.push_back(tPtr);
  }
  
  rSet = Filter(gr1.Nonterminals(),word);
  
  try{
    //resulting set size has to be one 
    if(rSet.size() != 1){
      std::stringstream errstr;
      errstr << "size of result set is " << rSet.size() << ", but 1 was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //resulting set has to contain *gr1.Nonterminals().begin()
    
    if(*rSet.begin() != *gr1.Nonterminals().begin()){
      std::stringstream errstr;
      errstr << "result set did not contain the expected nonterminal " << gr1.Nonterminals().begin()->Str() << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
      std::cout << (*rSet.begin()).Str() << std::endl;
      std::cout << (*gr1.Nonterminals().begin()).Str() << std::endl;
  }
  
  TestEnd(name);
}

/* *****************
 * TestFilterNothing
 * *****************/
void TestFilterNothing(){
  std::string name = "Filter Nothing";
  TestStart(name);
  
  std::set<Nonterminal> nt;
  GrammarSymbolVector gs;
  
  std::set<Nonterminal> rSet = Filter(nt,gs);
  
  try{
    //resulting set size has to be zero 
    if(rSet.size() != 0){
      std::stringstream errstr;
      errstr << "size of result set is " << rSet.size() << ", but 0 was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestRnpp1FindSymbolsEmptySet
 * *****************/
void TestRnpp1FindSymbolsEmptySet(){
  std::string name = "Rnpp1 Find Symbols Empty Set";
  TestStart(name);
  
  Grammar gr = TestGrammar1();
  
  //eliminable nonterminal which should be found by Rnpp1
  std::vector<Idx> v;
  v.push_back(3);
  Nonterminal nt(5,v,1);
  //the grammar should contain this nonterminal!
  if(gr.Nonterminals().find(nt) == gr.Nonterminals().end()){
    std::cout << "Warning, test parameters seem to be wrong. The test grammar did not contain Nonterminal (5,[3],1)." << std::endl;
    return;
  }
  
  //empty set of already eliminated nonterminals
  std::set<Nonterminal> ntSet;
  
  std::set<Nonterminal> rSet = Rnpp1(gr,ntSet);
  
  try{
    //resulting set size has to be one 
    if(rSet.size() != 1){
      std::stringstream errstr;
      errstr << "size of result set is " << rSet.size() << ", but 1 was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //resulting set size has to contain nonterminal (3,2)
    if(*rSet.begin() != nt){
      std::stringstream errstr;
      errstr << "result set did not contain the expected nonterminal " << nt.Str() << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestRnpp1FindSymbolsNonemptySet
 * *****************/
void TestRnpp1FindSymbolsNonemptySet(){
  std::string name = "Rnpp1 Find Symbols Empty Set";
  TestStart(name);
  
  Grammar gr = TestGrammar1();
  
  //eliminable nonterminal which should be found
  std::vector<Idx> v;
  v.push_back(2);
  Nonterminal nt(1,v,2);
  //the grammar should contain this nonterminal!
  if(gr.Nonterminals().find(nt) == gr.Nonterminals().end()){
    std::cout << "Warning, test parameters seem to be wrong. The test grammar did not contain Nonterminal (1,[2],2)." << std::endl;
    std::cout << gr.Str() << std::endl;
    return;
  }
  
  //empty set of already eliminated nonterminals
  std::set<Nonterminal> ntSet;
  
  std::set<Nonterminal> rSet = Rnpp1(gr,Rnpp1(gr,ntSet));
  
  try{
    //resulting set size has to be one 
    if(rSet.size() != 2){
      std::stringstream errstr;
      errstr << "size of result set is " << rSet.size() << ", but 2 was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //resulting set size has to contain nonterminal (1,2,2)
    if(*rSet.begin() != nt){
      std::stringstream errstr;
      errstr << "result set did not contain the expected nonterminal " << nt.Str() << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestRnpplFindSymbolsEmptySet
 * *****************/
void TestRnpplFindSymbolsEmptySet(){
  std::string name = "Rnppl Find Symbols Empty Set";
  TestStart(name);
  
  Grammar gr = TestGrammar1();
  
  //all nonterminals should be eliminable except (2,3,3)
  std::set<Nonterminal> findThis = gr.Nonterminals();
  std::vector<Idx> v;
  v.push_back(3);
  findThis.erase(Nonterminal(2,v,3));
  
  //empty set of already eliminated nonterminals
  std::set<Nonterminal> ntSet;
  
  std::set<Nonterminal> rSet = Rnppl(gr,ntSet);
  
  try{
    //resulting set size has to be the size of findThis 
    if(rSet.size() != findThis.size()){
      std::stringstream errstr;
      errstr << "size of result set is " << rSet.size() << ", but " << findThis.size() << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //resulting set size has to contain all nonterminals in findThis
    std::set<Nonterminal>::const_iterator ntit, findit;
    for(ntit = findThis.begin(); ntit != findThis.end(); ntit++){
      //look for every nonterminal
      findit = rSet.find(*ntit);
      //throw error if it was not found
      if(findit == findThis.end()){
        std::stringstream errstr;
        errstr << "result set did not contain the expected nonterminal " << ntit->Str() << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestRnpplFindSymbolsNonemptySet
 * *****************/
void TestRnpplFindSymbolsNonemptySet(){
  std::string name = "Rnppl Find Symbols Nonempty Set";
  TestStart(name);
  
  Grammar gr = TestGrammar1();
  
  //all nonterminals should be eliminable except (2,3,3)
  std::set<Nonterminal> findThis = gr.Nonterminals();
  std::vector<Idx> v;
  v.push_back(3);
  findThis.erase(Nonterminal(2,v,3));
  
  //set of already eliminated nonterminals, add any eliminable nonterminals
  std::set<Nonterminal> ntSet;
  v.clear(); v.push_back(2);
  ntSet.insert(Nonterminal(1,v,2));
  v.clear(); v.push_back(3);
  ntSet.insert(Nonterminal(4,v));
  
  std::set<Nonterminal> rSet = Rnppl(gr,ntSet);
  
  try{
    //resulting set size has to be the size of findThis
    if(rSet.size() != findThis.size()){
      std::stringstream errstr;
      errstr << "size of result set is " << rSet.size() << ", but " << findThis.size() << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //resulting set size has to contain all nonterminals in findThis
    std::set<Nonterminal>::const_iterator ntit, findit;
    for(ntit = findThis.begin(); ntit != findThis.end(); ntit++){
      //look for every nonterminal
      findit = rSet.find(*ntit);
      //throw error if it was not found
      if(findit == findThis.end()){
        std::stringstream errstr;
        errstr << "result set did not contain the expected nonterminal " << ntit->Str() << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestRnpplFindSymbolsCompleteSet
 * *****************/
void TestRnpplFindSymbolsCompleteSet(){
  std::string name = "Rnppl Find Symbols Complete Set";
  TestStart(name);
  
  Grammar gr = TestGrammar1();
  
  //all nonterminals should be eliminable except (2,3,3)
  std::set<Nonterminal> findThis = gr.Nonterminals();
  std::vector<Idx> v;
  v.push_back(3);
  findThis.erase(Nonterminal(2,v,3));
  
  //set of already eliminated nonterminals, add any eliminable nonterminals
  std::set<Nonterminal> ntSet = findThis;
  
  std::set<Nonterminal> rSet = Rnppl(gr,ntSet);
  
  try{
    //resulting set size has to be the size of findThis
    if(rSet.size() != findThis.size()){
      std::stringstream errstr;
      errstr << "size of result set is " << rSet.size() << ", but " << findThis.size() << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //resulting set size has to contain all nonterminals in findThis
    std::set<Nonterminal>::const_iterator ntit, findit;
    for(ntit = findThis.begin(); ntit != findThis.end(); ntit++){
      //look for every nonterminal
      findit = rSet.find(*ntit);
      //throw error if it was not found
      if(findit == findThis.end()){
        std::stringstream errstr;
        errstr << "result set did not contain the expected nonterminal " << ntit->Str() << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestRnppGrammar1
 * *****************/
void TestRnppGrammar1(){
  std::string name = "Rnpp Grammar 1";
  TestStart(name);
  
  Grammar gr = TestGrammar1();
  
  //all nonterminals should be eliminable except (2,3,3)
  std::set<Nonterminal> finalNtSet = gr.Nonterminals();
  std::vector<Idx> v;
  v.push_back(3);
  finalNtSet.erase(Nonterminal(2,v,3));
  //only one grammar production contains (2,3,3) and must be removed
  std::set<GrammarProduction> finalGpSet = gr.GrammarProductions();
  GrammarSymbolVector vg;
  Nonterminal* nt = new Nonterminal(2,v,3);
  GrammarSymbolPtr ntPtr(nt);
  vg.push_back(ntPtr);
  v.clear();  v.push_back(2);
  finalGpSet.erase(GrammarProduction(Nonterminal(1,v,2), vg));
  
  Grammar rGr = Rnpp(gr);
  
  try{
    //resulting size of nonterminals has to be the size of finalNtSet
    if(rGr.Nonterminals().size() != finalNtSet.size()){
      std::stringstream errstr;
      errstr << "size of nonterminal set is " << rGr.Nonterminals().size() << ", but " << finalNtSet.size() << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //resulting size of grammar productions has to be the size of finalGpSet
    if(rGr.GrammarProductions().size() != finalGpSet.size()){
      std::stringstream errstr;
      errstr << "size of grammar production set is " << rGr.GrammarProductions().size() << ", but " << finalGpSet.size() << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //resulting set size has to contain all nonterminals in finalNtSet
    std::set<Nonterminal>::const_iterator ntit, findntit;
    for(ntit = finalNtSet.begin(); ntit != finalNtSet.end(); ntit++){
      //look for every nonterminal
      findntit = rGr.Nonterminals().find(*ntit);
      //throw error if it was not found
      if(findntit == finalNtSet.end()){
        std::stringstream errstr;
        errstr << "nonterminal set did not contain the expected nonterminal " << ntit->Str() << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
    
    //resulting set size has to contain all grammar productions in finalGpSet
    std::set<GrammarProduction>::const_iterator gpit, findgpit;
    for(gpit = finalGpSet.begin(); gpit != finalGpSet.end(); gpit++){
      //look for every nonterminal
      findgpit = rGr.GrammarProductions().find(*gpit);
      //throw error if it was not found
      if(findgpit == finalGpSet.end()){
        std::stringstream errstr;
        errstr << "grammar productions set did not contain the expected grammar production " << gpit->Str() << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestRnppGrammar2
 * *****************/
void TestRnppGrammar2(){
  std::string name = "Rnpp Grammar 2";
  TestStart(name);
  
  Grammar gr = TestGrammar2();
  
  //all nonterminals should be eliminable except (2,3,3) and (3,2)
  std::vector<Idx> v2;
  v2.push_back(2);
  std::vector<Idx> v3;
  v3.push_back(3);
  std::set<Nonterminal> finalNtSet = gr.Nonterminals();
  finalNtSet.erase(Nonterminal(2,v3,3));
  finalNtSet.erase(Nonterminal(3,v2));
  //only one grammar production contains (2,3,3) and must be removed, no grammar
  //production contains (3,2)
  std::set<GrammarProduction> finalGpSet = gr.GrammarProductions();
  GrammarSymbolVector vg;
  Nonterminal* nt = new Nonterminal(2,v3,3);
  GrammarSymbolPtr ntPtr(nt);
  vg.push_back(ntPtr);
  finalGpSet.erase(GrammarProduction(Nonterminal(1,v2,2), vg));
  
  Grammar rGr = Rnpp(gr);
    
  try{
    //resulting size of nonterminals has to be the size of finalNtSet
    if(rGr.Nonterminals().size() != finalNtSet.size()){
      std::stringstream errstr;
      errstr << "size of nonterminal set is " << rGr.Nonterminals().size() << ", but " << finalNtSet.size() << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //resulting size of grammar productions has to be the size of finalGpSet
    if(rGr.GrammarProductions().size() != finalGpSet.size()){
      std::stringstream errstr;
      errstr << "size of grammar production set is " << rGr.GrammarProductions().size() << ", but " << finalGpSet.size() << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //resulting set size has to contain all nonterminals in finalNtSet
    std::set<Nonterminal>::const_iterator ntit, findntit;
    for(ntit = finalNtSet.begin(); ntit != finalNtSet.end(); ntit++){
      //look for every nonterminal
      findntit = rGr.Nonterminals().find(*ntit);
      //throw error if it was not found
      if(findntit == finalNtSet.end()){
        std::stringstream errstr;
        errstr << "nonterminal set did not contain the expected nonterminal " << ntit->Str() << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
    
    //resulting set size has to contain all grammar productions in finalGpSet
    std::set<GrammarProduction>::const_iterator gpit, findgpit;
    for(gpit = finalGpSet.begin(); gpit != finalGpSet.end(); gpit++){
      //look for every nonterminal
      findgpit = rGr.GrammarProductions().find(*gpit);
      //throw error if it was not found
      if(findgpit == finalGpSet.end()){
        std::stringstream errstr;
        errstr << "grammar productions set did not contain the expected grammar production " << gpit->Str() << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestRnppEmptyGrammar
 * *****************/
void TestRnppEmptyGrammar(){
  std::string name = "Rnpp Empty Grammar";
  TestStart(name);
  
  Grammar gr;
  
  Grammar rGr = Rnpp(gr);
  
  try{
    //resulting size of nonterminals has to be zero
    if(rGr.Nonterminals().size() != 0){
      std::stringstream errstr;
      errstr << "size of nonterminal set is " << rGr.Nonterminals().size() << ", but 0 was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //resulting size of grammar productions has to be zero
    if(rGr.GrammarProductions().size() != 0){
      std::stringstream errstr;
      errstr << "size of grammar production set is " << rGr.GrammarProductions().size() << ", but 0 was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestSp2LrTerminals
 * *****************/
void TestSp2LrTerminals(){
  std::string name = "Sp2Lr Terminals";
  TestStart(name);
  
  PushdownGenerator g = TestGenerator8();
  
  Grammar gr = Sp2Lr(g);
  
  try{
    //size of terminals must be the same as size of events
    if(g.Alphabet().Size() != gr.Terminals().size()){
      std::stringstream errstr;
      errstr << "size of terminal set is " << gr.Terminals().size() << ", but "<< g.Alphabet().Size() << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    EventSet::Iterator eventit;
    //terminals must be all events
    for(eventit = g.AlphabetBegin(); eventit != g.AlphabetEnd(); eventit++){
      if(gr.Terminals().find(*eventit) == gr.Terminals().end()){
        std::stringstream errstr;
        errstr << "terminal " << g.EventName(*eventit) << " was not found in terminal set" << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestSp2LrNonterminals
 * *****************/
void TestSp2LrNonterminals(){
  std::string name = "Sp2Lr Nonterminals";
  TestStart(name);
  
  PushdownGenerator g = TestGenerator8();
  
  Grammar gr = Sp2Lr(g,0,true);
  
  try{
	//number of expected nonterminals
    uint expectedNumberNonterminals = 8;

    //if the size of nonterminals matches the expected size, none can be missing
    if(expectedNumberNonterminals != gr.Nonterminals().size()){
      std::stringstream errstr;
      errstr << "size of nonterminal set is " << gr.Nonterminals().size() << ", but "<< expectedNumberNonterminals << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //test for the correct starting nonterminal
    if(gr.StartSymbol().StartState() != 1 || gr.StartSymbol().EndState() != 0 || gr.StartSymbol().OnStack().front() != g.StackSymbolIndex("square")){
      std::stringstream errstr;
      errstr << "start symbol was " << gr.StartSymbol().Str() << ", but (2, [square]) was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestSp2Lr2Productions
 * *****************/
void TestSp2LrProductions(){
  std::string name = "Sp2Lr2 Productions ";
  TestStart(name);
  
  PushdownGenerator g = TestGenerator8();
  
  Grammar gr = Sp2Lr(g);
  
  std::vector<Idx> dot, lambda, square;
  dot.push_back(g.StackSymbolIndex("dot"));
  square.push_back(g.StackSymbolIndex("square"));
  lambda.push_back(g.StackSymbolIndex("lambda"));
  
  Nonterminal* nt1dot = new Nonterminal(1,dot);
  GrammarSymbolPtr nt1dotPtr(nt1dot);
  Nonterminal* nt1square = new Nonterminal(1,square);
  GrammarSymbolPtr nt1squarePtr(nt1square);
  Nonterminal* nt2dot = new Nonterminal(2,dot);
  GrammarSymbolPtr nt2dotPtr(nt2dot);
  Nonterminal* nt2square = new Nonterminal(2,square);
  GrammarSymbolPtr nt2squarePtr(nt2square);
  Nonterminal* nt1dot1 = new Nonterminal(1,dot,1);
  GrammarSymbolPtr nt1dot1Ptr(nt1dot1);
  Nonterminal* nt1dot2 = new Nonterminal(1,dot,2);
  GrammarSymbolPtr nt1dot2Ptr(nt1dot2);
  Nonterminal* nt1square1 = new Nonterminal(1,square,1);
  GrammarSymbolPtr nt1square1Ptr(nt1square1);
  Nonterminal* nt1square2 = new Nonterminal(1,square,2);
  GrammarSymbolPtr nt1square2Ptr(nt1square2);
  Nonterminal* nt2dot1 = new Nonterminal(2,dot,1);
  GrammarSymbolPtr nt2dot1Ptr(nt2dot1);
  Nonterminal* nt2dot2 = new Nonterminal(2,dot,2);
  GrammarSymbolPtr nt2dot2Ptr(nt2dot2);
  Nonterminal* nt2square1 = new Nonterminal(2,square,1);
  GrammarSymbolPtr nt2square1Ptr(nt2square1);
  Nonterminal* nt2square2 = new Nonterminal(2,square,2);
  GrammarSymbolPtr nt2square2Ptr(nt2square2);
  Terminal* ta = new Terminal(g.EventIndex("a"));
  GrammarSymbolPtr taPtr(ta);
  Terminal* tlambda = new Terminal(g.EventIndex(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr tlambdaPtr(tlambda);
  
  
  GrammarSymbolVector v;
  //expected read set
  std::set<GrammarProduction> read;
  v.clear();
  v.push_back(taPtr);
  v.push_back(nt1dot2Ptr);
  read.insert(GrammarProduction(*nt2dot2,v));
  
  //expected pop set
  std::set<GrammarProduction> pop;
  v.clear();
  v.push_back(tlambdaPtr);
  pop.insert(GrammarProduction(*nt1dot2,v));
  
  //expected push set
  std::set<GrammarProduction> push;
  v.clear();
  v.push_back(nt2dot2Ptr);
  v.push_back(nt2squarePtr);
  push.insert(GrammarProduction(*nt1square,v));
  v.clear();
  v.push_back(nt2dotPtr);
  push.insert(GrammarProduction(*nt1square,v));
  
  //expected final state set
  std::set<GrammarProduction> final;
  v.clear();
  v.push_back(tlambdaPtr);
  final.insert(GrammarProduction(*nt2dot,v));
  v.clear();
  v.push_back(tlambdaPtr);
  final.insert(GrammarProduction(*nt2square,v));

  try{
    
    std::set<GrammarProduction> gp = gr.GrammarProductions();
    std::set<GrammarProduction>::const_iterator gpit;
    //test for read
    for(gpit = read.begin(); gpit != read.end(); gpit++){
      if(gp.erase(*gpit) == 0){
        std::stringstream errstr;
        errstr << "grammar production " << gpit->Str() << " , which is generated by a read transition, was expected but not found in the grammar" << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
    
    //test for pop
    for(gpit = pop.begin(); gpit != pop.end(); gpit++){
      if(gp.erase(*gpit) == 0){
        std::stringstream errstr;
        errstr << "grammar production " << gpit->Str() << " , which is generated by a pop transition, was expected but not found in the grammar" << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
    
    //test for push
    for(gpit = push.begin(); gpit != push.end(); gpit++){
      if(gp.erase(*gpit) == 0){
        std::stringstream errstr;
        errstr << "grammar production " << gpit->Str() << " , which is generated by a push transition, was expected but not found in the grammar" << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
    
    //test for final states
    for(gpit = final.begin(); gpit != final.end(); gpit++){
      if(gp.erase(*gpit) == 0){
        std::stringstream errstr;
        errstr << "grammar production " << gpit->Str() << " , which is generated by a final transition, was expected but not found in the grammar" << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
    
    //all productions must have been looked at by now
    if(gp.size() != 0){
      std::stringstream errstr;
      errstr << "the grammar contained" << gp.size() << " more productions than expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestRupProductions
 * *****************/
void TestRupProductions(){
  std::string name = "Test Rup Productions";
  TestStart(name);
  
  Grammar gr = TestGrammar3();
  Grammar rGr = Rup(gr);
  
  try{
    
    //only one production must have been removed
    if(gr.GrammarProductions().size() - 1 != rGr.GrammarProductions().size()){
      std::stringstream errstr;
      errstr << "size of grammar productions was " << rGr.GrammarProductions().size() << ", but " << gr.GrammarProductions().size() - 1 << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //only the grammar production (2, [dot], 2) -> a(1, [dot]) must have been removed
    std::vector<Idx> v;
    v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
    Nonterminal* nt1 = new Nonterminal(2,v,2);
    GrammarSymbolPtr nt1Ptr(nt1);
    Nonterminal* nt2 = new Nonterminal(1,v);
    GrammarSymbolPtr nt2Ptr(nt2);
    Terminal* t = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
    GrammarSymbolPtr tPtr(t);
    GrammarSymbolVector gs;
    gs.push_back(tPtr);
    gs.push_back(nt2Ptr);
    GrammarProduction gp(*nt1,gs);
    
    if(gr.GrammarProductions().find(gp) != gr.GrammarProductions().end()){
      std::stringstream errstr;
      errstr << "grammar production (2, [dot], 2) -> a(1, [dot]) was present, but was expected to be deleted" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestRupNonterminals
 * *****************/
void TestRupNonterminals(){
  std::string name = "Test Rup Nonterminals";
  TestStart(name);
  
  Grammar gr = TestGrammar3();
  Grammar rGr = Rup(gr);
  
  try{
    
    //only one nonterminal must have been removed
    if(gr.Nonterminals().size() - 1 != rGr.Nonterminals().size()){
      std::stringstream errstr;
      errstr << "size of nonterminals was " << rGr.Nonterminals().size() << ", but " << gr.Nonterminals().size() - 1 << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //only the nonterminal (2, [dot], 2) must have been removed
    std::vector<Idx> v;
    v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
    Nonterminal nt(2,v,2);
    if(gr.Nonterminals().find(nt) != gr.Nonterminals().end()){
      std::stringstream errstr;
      errstr << "nonterminal (2, [dot], 2) was present, but was expected to be deleted" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
  }

  
  TestEnd(name);
}

/* *****************
 * TestFilter
 * *****************/
void TestFilter(){
  TestFilterMixedGrammarSymbols();
  TestFilterNothing();
}

/* *****************
 * TestRnpp1
 * *****************/
void TestRnpp1(){
  TestRnpp1FindSymbolsEmptySet();
  TestRnpp1FindSymbolsNonemptySet();
}

/* *****************
 * TestRnpp1
 * *****************/
void TestRnppl(){
  TestRnpplFindSymbolsEmptySet();
  TestRnpplFindSymbolsNonemptySet();
  TestRnpplFindSymbolsCompleteSet();
}

/* *****************
 * TestRnpp
 * *****************/
void TestRnpp(){
  TestRnppGrammar1();
  TestRnppGrammar2();
  TestRnppEmptyGrammar();
}

/* *****************
 * Sp2Lr
 * *****************/
void TestSp2Lr(){
  
  TestSp2LrTerminals();
  TestSp2LrNonterminals();
  TestSp2LrProductions();
}


/* *****************
 * Rup
 * *****************/
void TestRup(){
  
  TestRupProductions();
  TestRupNonterminals();
}
} // namespace faudes

