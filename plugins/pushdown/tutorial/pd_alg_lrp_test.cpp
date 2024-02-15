/** @file pd_alg_lrp_test.cpp  Unit Tests */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/
#include "pd_alg_lrp_test.h"

namespace faudes {

  
/* *****************
 * TestGeneratorGotoOneSuccessor
 * *****************/
void TestGeneratorGotoOneSuccessor(){
  std::string name = "GeneratorGoto One Successor";
  TestStart(name);
  
  Grammar gr = TestGrammar7();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tdollar = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("$"));
  GrammarSymbolPtr ptrtdollar(tdollar);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);
  
  //test with state ((1, [dot]) -> $(1, [square]) . $, lambda) and symbol $
  //    ((1, [dot]) -> $(1, [square])$ . lambda, lambda)
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrtdollar);
  beforeDot.push_back(ptrnt1square);
  afterDot.push_back(ptrtdollar);
  Lr1Configuration config(*nt1dot, beforeDot, afterDot, *tlambda);
  
  std::set<Lr1Configuration> startState;
  startState.insert(config);

  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtdollar);
  beforeDot.push_back(ptrnt1square);
  beforeDot.push_back(ptrtdollar);
  afterDot.push_back(ptrtlambda);
  Lr1Configuration expectedConfig(*nt1dot, beforeDot, afterDot, *tlambda);
  
  GotoGenerator gotoGen = Lrm(gr,1);
  std::set<Idx> successorStates = GeneratorGoto(gotoGen, gotoGen.StateIndex(startState), ptrtdollar);
  try{
    
    //size of successorStates must be one
    if(successorStates.size() != 1){
      std::stringstream errstr;
      errstr << "number of successor states was expected to be 1, but was " << successorStates.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //number of contained configurations must be one
    if(gotoGen.ConfigSet(*successorStates.begin()).size() != 1){
      std::stringstream errstr;
      errstr << "number of configurations in successor state " << *successorStates.begin() << " was expected to be 1, but was " << gotoGen.ConfigSet(*successorStates.begin()).size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //the contained configuration must be expectedConfig
    if(*gotoGen.ConfigSet(*successorStates.begin()).begin() != expectedConfig){
      std::stringstream errstr;
      errstr << "configuration contained in successor state " << *successorStates.begin() << " was expected to be (" << expectedConfig.Str() << "), but was (" << gotoGen.ConfigSet(*successorStates.begin()).begin()->Str() << ")." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }  
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestGeneratorGotoNoSuccessor
 * *****************/
void TestGeneratorGotoNoSuccessor(){
  std::string name = "GeneratorGoto No Successor";
  TestStart(name);
  
  Grammar gr = TestGrammar7();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tdollar = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("$"));
  GrammarSymbolPtr ptrtdollar(tdollar);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);
  
  //test with state ((1, [dot]) -> $(1, [square]) . $, $) and symbol $
  //    empty
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrtdollar);
  beforeDot.push_back(ptrnt1square);
  afterDot.push_back(ptrtdollar);
  Lr1Configuration config(*nt1dot, beforeDot, afterDot, *tdollar);
  
  std::set<Lr1Configuration> startState;
  startState.insert(config);
  
  GotoGenerator gotoGen = Lrm(gr,1);
  
  std::set<Idx> successorStates = GeneratorGoto(gotoGen, gotoGen.StateIndex(startState), ptrtdollar);
  
  try{
    
    //size of successorStates must be zero
    if(successorStates.size() != 0){
      std::stringstream errstr;
      errstr << "number of successor states was expected to be 0, because state with configuration (" << config.Str() << ") does not exist , but was " << successorStates.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  //test with state ((1, [dot]) -> $(1, [square]) . $, lambda) and symbol a
  //    empty
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtdollar);
  beforeDot.push_back(ptrnt1square);
  afterDot.push_back(ptrtdollar);
  Lr1Configuration config2(*nt1dot, beforeDot, afterDot, *tlambda);
  
  startState.clear();
  startState.insert(config2);
  
  successorStates = GeneratorGoto(gotoGen, gotoGen.StateIndex(startState), ptrta);
  
  try{
    
    //size of successorStates must be zero
    if(successorStates.size() != 0){
      std::stringstream errstr;
      errstr << "number of successor states was expected to be 0, because state with configuration (" << config2.Str() << ") and symbol a does not exist, but was " << successorStates.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestGeneratorGotoSeqFull
 * *****************/
void TestGeneratorGotoSeqFull(){
  std::string name = "GeneratorGotoSeq Full";
  TestStart(name);
  
  Grammar gr = TestGrammar7();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tdollar = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("$"));
  GrammarSymbolPtr ptrtdollar(tdollar);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);
  
  //test with state ((1, [dot]) -> lambda . $(1, [square])$, lambda) and
  //word $a(1, [square])
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrtdollar);
  afterDot.push_back(ptrnt1square);
  afterDot.push_back(ptrtdollar);
  Lr1Configuration config(*nt1dot, beforeDot, afterDot, *tlambda);
  
  std::set<Lr1Configuration> startState;
  startState.insert(config);
  
  GrammarSymbolVector word;
  
  word.push_back(ptrtdollar);
  word.push_back(ptrta);
  word.push_back(ptrnt1square);
  
  //create expected last state of the sequence
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrta);
  beforeDot.push_back(ptrnt1square);
  afterDot.push_back(ptrtlambda);
  Lr1Configuration expectedEndConfig(*nt1square, beforeDot, afterDot, *tdollar);
  
  GotoGenerator gotoGen = Lrm(gr,1);
  std::vector<Idx> stateSequence = GeneratorGotoSeq(gotoGen, gotoGen.StateIndex(startState), word);

  try{
    
    //size of stateSequence must be 3
    if(stateSequence.size() != 3){
      std::stringstream errstr;
      errstr << "number of successor states was expected to be 3, but was " << stateSequence.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //number of contained configurations in the last state must be 1
    if(gotoGen.ConfigSet(stateSequence.back()).size() != 1){
      std::stringstream errstr;
      errstr << "number of configurations in the last state of the sequence was expected to be 1, but was " << gotoGen.ConfigSet(stateSequence.back()).size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //the last configuration in the sequence must be expectedEndConfig
    if(*gotoGen.ConfigSet(stateSequence.back()).begin() != expectedEndConfig){
      std::stringstream errstr;
      errstr << "state at the end of the sequence was expected to contain the configuration (" << expectedEndConfig.Str() << "), but was (" << gotoGen.ConfigSet(stateSequence.back()).begin()->Str() << ")." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }  
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestGeneratorGotoSeqPartial
 * *****************/
void TestGeneratorGotoSeqPartial(){
  std::string name = "GeneratorGotoSeq Partial";
  TestStart(name);
  
  Grammar gr = TestGrammar7();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tdollar = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("$"));
  GrammarSymbolPtr ptrtdollar(tdollar);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);
  
  //test with state ((1, [dot]) -> lambda . $(1, [square])$, lambda) and
  //word $b$
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrtdollar);
  afterDot.push_back(ptrnt1square);
  afterDot.push_back(ptrtdollar);
  Lr1Configuration config(*nt1dot, beforeDot, afterDot, *tlambda);
  
  std::set<Lr1Configuration> startState;
  startState.insert(config);
  
  GrammarSymbolVector word;
  
  word.push_back(ptrtdollar);
  word.push_back(ptrtb);
  word.push_back(ptrtdollar);
  
  //create expected last state of the sequence
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtb);
  afterDot.push_back(ptrtlambda);
  Lr1Configuration expectedEndConfig(*nt1square, beforeDot, afterDot, *tdollar);
  
  GotoGenerator gotoGen = Lrm(gr,1);
  std::vector<Idx> stateSequence = GeneratorGotoSeq(gotoGen, gotoGen.StateIndex(startState), word);

  try{
    
    //size of stateSequence must be 2
    if(stateSequence.size() != 2){
      std::stringstream errstr;
      errstr << "number of successor states was expected to be 2, but was " << stateSequence.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //number of contained configurations in the last state must be 1
    if(gotoGen.ConfigSet(stateSequence.back()).size() != 1){
      std::stringstream errstr;
      errstr << "number of configurations in the last state of the sequence was expected to be 1, but was " << gotoGen.ConfigSet(stateSequence.back()).size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //the last configuration in the sequence must be expectedEndConfig
    if(*gotoGen.ConfigSet(stateSequence.back()).begin() != expectedEndConfig){
      std::stringstream errstr;
      errstr << "state at the end of the sequence was expected to contain the configuration (" << expectedEndConfig.Str() << "), but was (" << gotoGen.ConfigSet(stateSequence.back()).begin()->Str() << ")." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }  
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestLrpShiftRulesNumberAndTerminals
 * *****************/
void TestLrpShiftRulesNumberAndTerminals(){
  std::string name = "LrpShiftRules Number And Terminals";
  TestStart(name);
  
  Grammar gr = TestGrammar9();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tdollar = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("$"));
  GrammarSymbolPtr ptrtdollar(tdollar);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);
  
  Grammar augGr = Aug(gr, *nt1dot ,*tdollar);
  
  GotoGenerator gotoGen = Lrm(augGr, 1);
  std::set<Lr1ParserAction> actions = LrpShiftRules(gr, augGr, gotoGen, 1);

  try{
    
    //number of found actions must be 4
    if(actions.size() != 4){
      std::stringstream errstr;
      errstr << "number of shift actions was expected to be 4, but was " << actions.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    } 
    //there must be 2 actions that reduce with terminal a
    int i = 0;
    std::set<Lr1ParserAction>::const_iterator pait;
    for(pait = actions.begin(); pait != actions.end(); pait++){
      if(pait->Lhs().NextTerminal() == *ta){
        i++;
      }
    }
    if(i != 2){
      std::stringstream errstr;
      errstr << "number of shift actions for symbol a of the form (x | a) -> (xy | lambda) was expected to be 2, but was " << i << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //there must be 2 actions that reduce with terminal b
    i = 0;
    for(pait = actions.begin(); pait != actions.end(); pait++){
      if(pait->Lhs().NextTerminal() == *tb){
        i++;
      }
    }
    if(i != 2){
      std::stringstream errstr;
      errstr << "number of shift actions for symbol b of the form (x | b) -> (xy | lambda) was expected to be 2, but was " << i << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestLrpReduceRulesNumberAndTerminals
 * *****************/
void TestLrpReduceRulesNumberAndTerminals(){
  std::string name = "LrpReduceRules Number And Terminals";
  TestStart(name);
  
  Grammar gr = TestGrammar9();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tdollar = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("$"));
  GrammarSymbolPtr ptrtdollar(tdollar);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);
  
  Grammar augGr = Aug(gr, *nt1dot ,*tdollar);
  
  GotoGenerator gotoGen = Lrm(augGr, 1);
  std::set<Lr1ParserAction> actions = LrpReduceRules(gr, augGr, gotoGen, 1);

  try{
    
    //number of found actions must be 4
    if(actions.size() != 4){
      std::stringstream errstr;
      errstr << "number of shift actions was expected to be 4, but was " << actions.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    } 
    //there must be 4 actions that shift with terminal $
    int i = 0;
    std::set<Lr1ParserAction>::const_iterator pait;
    for(pait = actions.begin(); pait != actions.end(); pait++){
      if(pait->Lhs().NextTerminal() == *tdollar && pait->Rhs().NextTerminal() == *tdollar){
        i++;
      }
    }
    if(i != 4){
      std::stringstream errstr;
      errstr << "number of shift actions for symbol a of the form (x | $) -> (xy | $) was expected to be 4, but was " << i << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestLrpActions
 * *****************/
void TestLrpActions(){
  std::string name = "Lrp Actions";
  TestStart(name);
  
  Grammar gr = TestGrammar9();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tdollar = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("$"));
  GrammarSymbolPtr ptrtdollar(tdollar);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);
  
  Grammar augGr = Aug(gr, *nt1dot ,*tdollar);
  
  GotoGenerator gotoGen = Lrm(augGr, 1);
  Lr1Parser parser = Lrp(gr, augGr, gotoGen, 1, *tdollar);
  
  try{
    
    //number of found actions must be 8
    std::set<Lr1ParserAction> actions = parser.Actions();
    if(actions.size() != 8){
      std::stringstream errstr;
      errstr << "number of actions was expected to be 8, but was " << actions.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    } 
    //there must be 4 actions with terminal $ and $on the left-hand and right-hand side,
    //2 with a and lambda, and 2 with b and lambda
    int dollarCount = 0;
    int aCount = 0;
    int bCount = 0;
    std::set<Lr1ParserAction>::const_iterator pait;
    for(pait = actions.begin(); pait != actions.end(); pait++){
      if(pait->Lhs().NextTerminal() == *tdollar && pait->Rhs().NextTerminal() == *tdollar){
        dollarCount++;
      }
      if(pait->Lhs().NextTerminal() == *ta && pait->Rhs().NextTerminal() == *tlambda){
        aCount++;
      }
      if(pait->Lhs().NextTerminal() == *tb && pait->Rhs().NextTerminal() == *tlambda){
        bCount++;
      }
    }
    if(dollarCount != 4){
      std::stringstream errstr;
      errstr << "number of actions for symbol $ of the form (x | $) -> (xy | $) was expected to be 4, but was " << dollarCount << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(aCount != 2){
      std::stringstream errstr;
      errstr << "number of actions for symbol a of the form (x | a) -> (xy | lambda) was expected to be 2, but was " << aCount << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(bCount != 2){
      std::stringstream errstr;
      errstr << "number of actions for symbol b of the form (x | b) -> (xy | $) was expected to be 2, but was " << bCount << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestLrpNonterminalsTerminals
 * *****************/
void TestLrpNonterminalsTerminals(){
  std::string name = "Lrp Nonterminals Terminals";
  TestStart(name);
  
  Grammar gr = TestGrammar9();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tdollar = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("$"));
  GrammarSymbolPtr ptrtdollar(tdollar);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);
  
  Grammar augGr = Aug(gr, *nt1dot ,*tdollar);
  
  GotoGenerator gotoGen = Lrm(augGr, 1);
  Lr1Parser parser = Lrp(gr, augGr, gotoGen, 1, *tdollar);
  
  try{
    
    //number of Nonterminals must be 5
    std::set<Idx> nonterminals = parser.Nonterminals();
    if(nonterminals.size() != 5){
      std::stringstream errstr;
      errstr << "number of nonterminals was expected to be 5, but was " << nonterminals.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //number of Terminals must be 4
    std::set<Terminal> terminals = parser.Terminals();
    if(terminals.size() != 4){
      std::stringstream errstr;
      errstr << "number of terminals was expected to be 4, but was " << terminals.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //the augment symbol must be $
    if(parser.AugSymbol() != *tdollar){
      std::stringstream errstr;
      errstr << "the parser's augment symbol was expected to be " << tdollar->Str() << ", but was " << parser.AugSymbol().Str() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestLrParser2EPDAStates
 * *****************/
void TestLrParser2EPDAStates(){
  std::string name = "LrParser2EPDA States";
  TestStart(name);
  
  Grammar gr = TestGrammar9();
  
  //create terminals
  Terminal* tdollar = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("$"));
  GrammarSymbolPtr ptrtdollar(tdollar);

  //get event indices
  Idx aIdx = PushdownGenerator::GlobalEventSymbolTablep()->Index("a");
  Idx bIdx = PushdownGenerator::GlobalEventSymbolTablep()->Index("b");
  Idx lambdaIdx = PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA);
  Idx dollarIdx = PushdownGenerator::GlobalEventSymbolTablep()->Index("$");
  
  //create state event pairs that are expected to be used in the generator
  std::vector<MergeStateEvent> sePairs;
  std::vector<MergeStateEvent>::iterator sePairsit;
  sePairs.push_back(MergeStateEvent(6,lambdaIdx));
  sePairs.push_back(MergeStateEvent(6,aIdx));
  sePairs.push_back(MergeStateEvent(6,bIdx));
  sePairs.push_back(MergeStateEvent(7,lambdaIdx));
  sePairs.push_back(MergeStateEvent(7,aIdx));
  sePairs.push_back(MergeStateEvent(7,bIdx));
  sePairs.push_back(MergeStateEvent(4,lambdaIdx));
  sePairs.push_back(MergeStateEvent(5,lambdaIdx));
  sePairs.push_back(MergeStateEvent(2,dollarIdx));
  sePairs.push_back(MergeStateEvent(4,dollarIdx));
  sePairs.push_back(MergeStateEvent(5,dollarIdx));
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  //create pushdown generator
  Grammar augGr = Aug(gr, *nt1dot ,*tdollar);
  
  GotoGenerator gotoGen = Lrm(augGr, 1);
  Lr1Parser parser = Lrp(gr, augGr, gotoGen, 1, *tdollar);

  //detach augment symbol
  DetachAugSymbol(parser);

  // Transform following actions
  parser = TransformParserAction(parser);

  PushdownGenerator pd = LrParser2EPDA(parser);

  try{
    
    //number of states must be 7
    if(pd.Size() != 7){
      std::stringstream errstr;
      errstr << "number of states was expected to be 7, but was " << pd.Size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //get the states that are actually used
    StateSet states;
    TransSet::Iterator transit;

    //test whether the used states are the expected ones
    StateSet::Iterator stateit;
    for(stateit = states.Begin(); stateit != states.End(); stateit++){
      
      //get MergeStateEvent
      const MergeStateEvent* mse = dynamic_cast<const MergeStateEvent*>(pd.StateAttribute(*stateit).Merge());
      if(mse == NULL){
        throw Exception(name, "MergeStateEvent not set.", 1003);
      }
      
      //test for existence in expected merge state events
      bool found = false;
      for(sePairsit = sePairs.begin(); sePairsit != sePairs.end(); sePairsit++){
        if(sePairsit->State() == mse->State() && sePairsit->Event() == mse->Event()){
          found = true;
          break;
        }
      }
      
      if(!found){
        std::stringstream errstr;
        errstr << "state with MergeStateEvent (" << mse->State() << ", " << PushdownGenerator::GlobalEventSymbolTablep()->Symbol(mse->Event()) << ") was not expected to be in the set of used states." << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
    
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestLrParser2EPDATransitions
 * *****************/
void TestLrParser2EPDATransitions(){
  std::string name = "LrParser2EPDA Transitions";
  TestStart(name);
  
  Grammar gr = TestGrammar9();
  
  //create terminals
  Terminal* tdollar = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("$"));
  GrammarSymbolPtr ptrtdollar(tdollar);

  //get event indices
  Idx aIdx = PushdownGenerator::GlobalEventSymbolTablep()->Index("a");
  Idx bIdx = PushdownGenerator::GlobalEventSymbolTablep()->Index("b");
  Idx lambdaIdx = PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA);
  Idx dollarIdx = PushdownGenerator::GlobalEventSymbolTablep()->Index("$");
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  //create pushdown generator
  Grammar augGr = Aug(gr, *nt1dot ,*tdollar);
  
  GotoGenerator gotoGen = Lrm(augGr, 1);
  Lr1Parser parser = Lrp(gr, augGr, gotoGen, 1, *tdollar);

  //detach augment symbol
  DetachAugSymbol(parser);

  // Transform following actions
  parser = TransformParserAction(parser);
  PushdownGenerator pd = LrParser2EPDA(parser);

  try{
    
    
    
    int transCount = 0;
    int aCount = 0;
    int bCount = 0;
    int dollarCount = 0;
    int lambdaCount = 0;
    int push6Count = 0;
    int push7Count = 0;
    int popLambdaCount = 0;
    int popSize1 = 0;
    int popSize2 = 0;
    TransSet::Iterator transit;
    for(transit = pd.TransRelBegin(); transit != pd.TransRelEnd(); transit++){
      
      //count events
      if(transit->Ev == aIdx)
        aCount++;
      else if(transit->Ev == bIdx)
        bCount++;
      else if(transit->Ev == lambdaIdx)
        lambdaCount++;
      else if(transit->Ev == dollarIdx)
        dollarCount++;
      
      
      PopPushSet popPush = pd.PopPush(*transit);
      PopPushSet::const_iterator ppit;
      for(ppit = popPush.begin(); ppit != popPush.end(); ppit++){
        //count transitions
        transCount++;
        
        //count pops
        if(ppit->first.size() == 2)
          popSize2++;
        else if(ppit->first.size() == 1){
          popSize1++;
          if(ppit->first.front() == pd.StackSymbolIndex(FAUDES_PD_LAMBDA))
            popLambdaCount++;
        }
        
        //count pushes
        if(ppit->second.size() == 1){
          if(ppit->second.front() == pd.StackSymbolIndex("6"))
            push6Count++;
          else if(ppit->second.front() == pd.StackSymbolIndex("7"))
            push7Count++;
        }
        
      }
    }
    
    //number of transitions must be 8
    if(transCount != 8){
      std::stringstream errstr;
      errstr << "number of transitions was expected to be 8, but was " << transCount << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //number of a events must be 2
    if(aCount != 2){
      std::stringstream errstr;
      errstr << "number of transitions with the event a was expected to be 2, but was " << aCount << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //number of b events must be 2
    if(bCount != 2){
      std::stringstream errstr;
      errstr << "number of transitions with the event b was expected to be 2, but was " << bCount << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //number of $ events must be 0
    if(dollarCount != 0){
      std::stringstream errstr;
      errstr << "number of transitions with the event $ was expected to be 0, but was " << dollarCount << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //number of lambda events must be 8
    if(lambdaCount != 4){
      std::stringstream errstr;
      errstr << "number of transitions with the event lambda was expected to be 4, but was " << lambdaCount << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //number of 6 pushes must be 0
    if(push6Count != 0){
      std::stringstream errstr;
      errstr << "number of transitions that push 6 was expected to be 0, but was " << push6Count << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //number of 7 pushes must be 0
    if(push7Count != 0){
      std::stringstream errstr;
      errstr << "number of transitions that push 7 was expected to be 0, but was " << push7Count << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //number of transitions that pop 2 stack symbols must be 0
    if(popSize2 != 0){
      std::stringstream errstr;
      errstr << "number of transitions that pop 2 stack symbols was expected to be 0, but was " << popSize2 << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //number of transitions that pop 1 stack symbol must be 8
    if(popSize1 != 8){
      std::stringstream errstr;
      errstr << "number of transitions that pop 1 stack symbol was expected to be 8, but was " << popSize1 << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //number of transitions that pop lambda must be 8
    if(popLambdaCount != 8){
      std::stringstream errstr;
      errstr << "number of transitions that pop lambda was expected to be 8, but was " << popLambdaCount << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestDimNoAugment
 * *****************/
void TestDimNoAugment(){
  std::string name = "Dim No Augment";
  TestStart(name);
  
  Grammar gr = TestGrammar9();
  
  //create terminals
  Terminal* tdollar = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("$"));
  GrammarSymbolPtr ptrtdollar(tdollar);

  //get event indices
  Idx dollarIdx = PushdownGenerator::GlobalEventSymbolTablep()->Index("$");
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  //create pushdown generator and diminish it
  Grammar augGr = Aug(gr, *nt1dot ,*tdollar);
  
  GotoGenerator gotoGen = Lrm(augGr, 1);
  Lr1Parser parser = Lrp(gr, augGr, gotoGen, 1, *tdollar);


  //detach augment symbol
  DetachAugSymbol(parser);

  // Transform following actions
  parser = TransformParserAction(parser);

  PushdownGenerator pdtemp = LrParser2EPDA(parser);
  PushdownGenerator pd = Dim(pdtemp,*tdollar);
  
  try{
    
    int transCount = 0;
    Transition trans;
    bool dollarFound = false;
    TransSet::Iterator transit;
    for(transit = pd.TransRelBegin(); transit != pd.TransRelEnd(); transit++){  
      PopPushSet popPush = pd.PopPush(*transit);
      PopPushSet::const_iterator ppit;
      
      //count transitions
      for(ppit = popPush.begin(); ppit != popPush.end(); ppit++){
        transCount++;
      }
      
      //look for transitions with $
      if(transit->Ev == dollarIdx){
        dollarFound = true;
        trans = *transit;
      }
    }
    
    //number of transitions must be 8
    if(transCount != 8){
      std::stringstream errstr;
      errstr << "number of transitions was expected to be 8, but was " << transCount << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //no $ event must be found
    if(dollarFound){
      std::stringstream errstr;
      errstr << "transition "<< trans.Str() << " with event $ was found, but not expected." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }

  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestDimNewFinalStates
 * *****************/
void TestDimNewFinalStates(){
  std::string name = "Dim Final States";
  TestStart(name);
  
  Grammar gr = TestGrammar9();
  
  //create terminals
  Terminal* tdollar = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("$"));
  GrammarSymbolPtr ptrtdollar(tdollar);

  //get event indices
  Idx dollarIdx = PushdownGenerator::GlobalEventSymbolTablep()->Index("$");
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  //create pushdown generator
  Grammar augGr = Aug(gr, *nt1dot ,*tdollar);
  
  GotoGenerator gotoGen = Lrm(augGr, 1);
  Lr1Parser parser = Lrp(gr, augGr, gotoGen, 1, *tdollar);

  //detach augment symbol
  DetachAugSymbol(parser);

  // Transform following actions
  parser = TransformParserAction(parser);

  PushdownGenerator pdtemp = LrParser2EPDA(parser);
  
  //get indices of states that must be made final after diminishing
  TransSet::Iterator transit;
  StateSet expectedFinalStates;
  for(transit = pdtemp.TransRelBegin(); transit != pdtemp.TransRelEnd(); transit++){
    if(transit->Ev == dollarIdx){
      expectedFinalStates.Insert(transit->X1);
    }
  }
  
  //diminish pushdown generator
  PushdownGenerator pd = Dim(pdtemp,*tdollar);
  
  try{
    
    StateSet::Iterator stateit;
    //test for final states
    for(stateit = expectedFinalStates.Begin(); stateit != expectedFinalStates.End(); stateit++){
      if(!pd.ExistsMarkedState(*stateit)){
        std::stringstream errstr;
        errstr << "state " << *stateit << " was expected to be a final state, but was not." << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestGeneratorGoto
 * *****************/
void TestGeneratorGoto(){
  TestGeneratorGotoNoSuccessor();
  TestGeneratorGotoOneSuccessor();
}

/* *****************
 * TestGeneratorGotoSeq
 * *****************/
void TestGeneratorGotoSeq(){
  TestGeneratorGotoSeqFull();
  TestGeneratorGotoSeqPartial();
}

/* *****************
 * TestLrpShiftRules
 * *****************/
void TestLrpShiftRules(){
  TestLrpShiftRulesNumberAndTerminals();
}

/* *****************
 * TestLrpReduceRules
 * *****************/
void TestLrpReduceRules(){
  TestLrpReduceRulesNumberAndTerminals();
}

/* *****************
 * TestLrp
 * *****************/
void TestLrp(){
  TestLrpActions();
  TestLrpNonterminalsTerminals();
}

/* *****************
 * TestLrParser2EPDA
 * *****************/
void TestLrParser2EPDA(){
  
  TestLrParser2EPDATransitions();
  TestLrParser2EPDAStates();
}

/* *****************
 * TestDim
 * *****************/
void TestDim(){
  TestDimNoAugment(); 
  TestDimNewFinalStates();
}
  
} // namespace faudes

