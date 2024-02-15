/** @file pd_alg_lrm_test.cpp  Unit Tests */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/
#include "pd_alg_lrm_test.h"

namespace faudes {

  /* *****************
 * TestDesc11Terminal
 * *****************/
void TestDesc11Terminal(){
  std::string name = "Desc11 Terminal";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar5();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  
  //create nonterminals (1, [dot])
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  //test for ((1, [dot]) -> lambda . a, lambda)
  //    ((1, [dot]) -> lambda . a, lambda)
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrta);
  Lr1Configuration config(*nt1dot, beforeDot, afterDot, *tlambda);
  
  std::set<Lr1Configuration> descSet = Desc11(gr, 1, config);
  try{

    if(descSet.size() != 1){
      std::stringstream errstr;
      errstr << "descendant set of (" << config.Str() << ") was expected to be of size 1, but was of size " << descSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    if(descSet.find(config) == descSet.end()){
      std::stringstream errstr;
      errstr << "descendant set of  (" << config.Str() << ") was expected to contain the configuration (" << config.Str() << "), but the configuration was not found." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestDesc11Nonterminal
 * *****************/
void TestDesc11Nonterminal(){
  std::string name = "Desc11 Nonterminal";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar5();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  
  //create nonterminals (1, [dot])
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot1 = new Nonterminal(1,v,1);
  GrammarSymbolPtr ptrnt1dot1(nt1dot1);
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);
  
  //test for ((1, [dot]) -> lambda . (1, [dot], 1), b
  //    ((1, [dot]) -> lambda . (1, [dot], 1), b
  //    (1, [dot], 1) -> lambda . (1, [dot]), b
  //    (1, [dot], 1) -> lambda . a(1, [square]), b
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrnt1dot1);
  Lr1Configuration config(*nt1dot, beforeDot, afterDot, *tb);
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrnt1dot);
  Lr1Configuration descConfig1(*nt1dot1, beforeDot, afterDot, *tb);
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrta);
  afterDot.push_back(ptrnt1square);
  Lr1Configuration descConfig2(*nt1dot1, beforeDot, afterDot, *tb);
  
  std::set<Lr1Configuration> descSet = Desc11(gr, 1, config);
  try{

    if(descSet.size() != 3){
      std::stringstream errstr;
      errstr << "descendant set of (" << config.Str() << ") was expected to be of size 3, but was of size " << descSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    if(descSet.find(config) == descSet.end()){
      std::stringstream errstr;
      errstr << "descendant set of  (" << config.Str() << ") was expected to contain the configuration (" << config.Str() << "), but the configuration was not found." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    if(descSet.find(descConfig1) == descSet.end()){
      std::stringstream errstr;
      errstr << "descendant set of  (" << config.Str() << ") was expected to contain the configuration (" << descConfig1.Str() << "), but the configuration was not found." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    if(descSet.find(descConfig2) == descSet.end()){
      std::stringstream errstr;
      errstr << "descendant set of  (" << config.Str() << ") was expected to contain the configuration (" << descConfig2.Str() << "), but the configuration was not found." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestDescSelectedConfigs
 * *****************/
void TestDescSelectedConfigs(){
    std::string name = "Desc SelectedConfigs";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar5();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot1 = new Nonterminal(1,v,1);
  GrammarSymbolPtr ptrnt1dot1(nt1dot1);
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);
  
  //test for (1, [dot]) -> lambda . (1, [dot], 1)a(1, [square]), b
  //and (1, [dot]) -> a . a, a
  //and (1, [dot]) -> lambda . lambda  , b
  //    (1, [dot], 1) -> lambda . (1, [dot]), a
  //    (1, [dot], 1) -> lambda . a(1, [square]), a
  //    ((1, [dot]) -> lambda . (1, [dot], 1)a(1, [dot]), a
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrnt1dot1);
  afterDot.push_back(ptrta);
  afterDot.push_back(ptrnt1square);
  Lr1Configuration config1(*nt1dot, beforeDot, afterDot, *tb);
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrta);
  afterDot.push_back(ptrta);
  Lr1Configuration config2(*nt1dot, beforeDot, afterDot, *ta);
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrtlambda);
  Lr1Configuration config3(*nt1dot, beforeDot, afterDot, *tb);
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrnt1dot);
  Lr1Configuration descConfig1(*nt1dot1, beforeDot, afterDot, *ta);
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrta);
  afterDot.push_back(ptrnt1square);
  Lr1Configuration descConfig2(*nt1dot1, beforeDot, afterDot, *ta);
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrnt1dot1);
  afterDot.push_back(ptrta);
  afterDot.push_back(ptrnt1square);
  Lr1Configuration descConfig3(*nt1dot, beforeDot, afterDot, *ta);
  
  
  std::set<Lr1Configuration> configs;
  configs.insert(config1);
  configs.insert(config2);
  configs.insert(config3);
  ;
  std::set<Lr1Configuration> descSet = Desc(gr, 1, configs);

  try{

    if(descSet.size() != 6){
      std::stringstream errstr;
      errstr << "descendant set was expected to be of size 5, but was of size " << descSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    if(descSet.find(config1) == descSet.end()){
      std::stringstream errstr;
      errstr << "descendant set was expected to contain the configuration (" << config1.Str() << "), but the configuration was not found." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    if(descSet.find(config2) == descSet.end()){
      std::stringstream errstr;
      errstr << "descendant set was expected to contain the configuration (" << config2.Str() << "), but the configuration was not found." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    if(descSet.find(config3) == descSet.end()){
      std::stringstream errstr;
      errstr << "descendant set was expected to contain the configuration (" << config3.Str() << "), but the configuration was not found." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    if(descSet.find(descConfig1) == descSet.end()){
      std::stringstream errstr;
      errstr << "descendant set was expected to contain the configuration (" << descConfig1.Str() << "), but the configuration was not found." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    if(descSet.find(descConfig2) == descSet.end()){
      std::stringstream errstr;
      errstr << "descendant set was expected to contain the configuration (" << descConfig2.Str() << "), but the configuration was not found." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    if(descSet.find(descConfig3) == descSet.end()){
      std::stringstream errstr;
      errstr << "descendant set was expected to contain the configuration (" << descConfig3.Str() << "), but the configuration was not found." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestPassesXNonterminal
 * *****************/
void TestPassesXNonterminal(){
  std::string name = "PassesX Nonterminal";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar5();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);
  
  //test for ((1, [dot]) -> a . (1, [square])b, b)
  //    ((1, [dot]) -> a(1, [square]) . b, b)
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrta);
  afterDot.push_back(ptrnt1square);
  afterDot.push_back(ptrtb);
  Lr1Configuration config(*nt1dot, beforeDot, afterDot, *tb);
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrta);
  beforeDot.push_back(ptrnt1square);
  afterDot.push_back(ptrtb);
  Lr1Configuration expectedConfig(*nt1dot, beforeDot, afterDot, *tb);
  
  std::set<Lr1Configuration> shiftSet = PassesX(config, ptrnt1square);
  try{

    if(shiftSet.size() != 1){
      std::stringstream errstr;
      errstr << "shift set of (" << config.Str() << ") to be shifted over symbol " << ptrnt1square->Str() << " was expected to be of size 1, but was of size " << shiftSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    if(shiftSet.find(expectedConfig) == shiftSet.end()){
      std::stringstream errstr;
      errstr << "Shift of the configuration (" << config.Str() << ") over the symbol " << ptrnt1square->Str() << " was expected to result in the configuration (" << expectedConfig.Str() << "), but resulted in " << shiftSet.begin()->Str() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestPassesXTerminal
 * *****************/
void TestPassesXTerminal(){
  std::string name = "PassesX Terminal";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar5();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  
  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  //test for ((1, [dot]) -> lambda . a, b)
  //    ((1, [dot]) -> a . lambda , b)
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrta);
  Lr1Configuration config(*nt1dot, beforeDot, afterDot, *tb);
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrta);
  afterDot.push_back(ptrtlambda);
  Lr1Configuration expectedConfig(*nt1dot, beforeDot, afterDot, *tb);
  
  std::set<Lr1Configuration> shiftSet = PassesX(config, ptrta);
  try{

    if(shiftSet.size() != 1){
      std::stringstream errstr;
      errstr << "shift set of (" << config.Str() << ") to be shifted over symbol " << ptrta->Str() << " was expected to be of size 1, but was of size " << shiftSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    if(shiftSet.find(expectedConfig) == shiftSet.end()){
      std::stringstream errstr;
      errstr << "Shift of the configuration (" << config.Str() << ") over the symbol " << ptrta->Str() << " was expected to result in the configuration (" << expectedConfig.Str() << "), but resulted in " << shiftSet.begin()->Str() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestPassesXNoShift
 * *****************/
void TestPassesXNoShift(){
  std::string name = "PassesX No Shift";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar5();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
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
  
  //test for ((1, [dot]) -> a . lambda , b)
  //    empty
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrta);
  afterDot.push_back(ptrtlambda);
  Lr1Configuration config1(*nt1dot, beforeDot, afterDot, *tb);
  
  std::set<Lr1Configuration> shiftSet = PassesX(config1, ptrnt1square);
  try{

    if(!shiftSet.empty()){
      std::stringstream errstr;
      errstr << "shift set of (" << config1.Str() << ") to be shifted over symbol " << ptrnt1square->Str() << " was expected to be empty, but was of size " << shiftSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  //test for ((1, [dot]) -> a . (1, [dot]), b)
  //    empty
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrta);
  afterDot.push_back(ptrnt1dot);
  Lr1Configuration config2(*nt1dot, beforeDot, afterDot, *tb);
  
  shiftSet = PassesX(config2, ptrnt1square);
  try{

    if(!shiftSet.empty()){
      std::stringstream errstr;
      errstr << "shift set of (" << config2.Str() << ") to be shifted over symbol " << ptrnt1square->Str() << " was expected to be empty, but was of size " << shiftSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestLrm1FindOne
 * *****************/
void TestLrm1FindOne(){
  std::string name = "Lrm1 Find One";
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
  
  //test for ((1, [dot]) -> lambda . $(1, [square])$, lambda)
  // $: ((1, [dot]) -> $ . (1, [square])$, lambda)
  //    ((1, [square]) -> lambda . a(1, [square]), $)
  //    ((1, [square]) -> lambda . b, $)
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrtdollar);
  afterDot.push_back(ptrnt1square);
  afterDot.push_back(ptrtdollar);
  Lr1Configuration config(*nt1dot, beforeDot, afterDot, *tlambda);
  
  std::set<Lr1Configuration> expectedConfigs;
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtdollar);
  afterDot.push_back(ptrnt1square);
  afterDot.push_back(ptrtdollar);
  expectedConfigs.insert(Lr1Configuration(*nt1dot, beforeDot, afterDot, *tlambda));
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrta);
  afterDot.push_back(ptrnt1square);
  expectedConfigs.insert(Lr1Configuration(*nt1square, beforeDot, afterDot, *tdollar));
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrtb);
  expectedConfigs.insert(Lr1Configuration(*nt1square, beforeDot, afterDot, *tdollar));
  
  std::set<Lr1Configuration> configSet;
  configSet.insert(config);
  
  Lr1ConfigurationSetSet configSetSet;
  configSetSet.insert(configSet);
  
  LrmTransitionMap transitionMap = Lrm1(gr, 1, configSetSet);
  try{
    
    //size of the transition map must be one
    if(transitionMap.size() != 1){
      std::stringstream errstr;
      errstr << "number of successor config sets of config set (" << ConfigSetToStr(configSet) << ") was expected to be 1, but there were " << transitionMap.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //the symbol must be dollar
    if(*transitionMap.begin()->first.second != *ptrtdollar){
      std::stringstream errstr;
      errstr << "event to transition out of config set (" << ConfigSetToStr(configSet) << ") was expected $, but was not found." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //the size of the successor set must be three
    if(transitionMap.begin()->second.size() != 3){
      std::stringstream errstr;
      errstr << "config set \n" << ConfigSetToStr(configSet) << "\n has the successor config set \n" << ConfigSetToStr(transitionMap.begin()->second) << ",\n which was expected to be of size 3, but was of size " << transitionMap.begin()->second.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    std::set<Lr1Configuration>::const_iterator configit;
    //the three expected configs must be in the successor set
    for(configit = expectedConfigs.begin(); configit != expectedConfigs.end(); configit++){
      if(transitionMap.begin()->second.find(*configit) == transitionMap.begin()->second.end()){
        std::stringstream errstr;
        errstr << "config set \n" << ConfigSetToStr(configSet) << "\n has the successor config set \n" << ConfigSetToStr(transitionMap.begin()->second) << ",\n which was expected to contain configuration " << configit->Str() << ", but was not found." << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
  }
  catch (Exception e){ 
  }

  TestEnd(name);
}

/* *****************
 * TestLrm1FindThree
 * *****************/
void TestLrm1FindThree(){
  std::string name = "Lrm1 Find Three";
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
  
  //test for ((1, [dot]) -> $ lambda . (1, [square])$, lambda)
  //         ((1, [square]) -> lambda . a(1, [square]), $)
  //         ((1, [square]) -> lambda . b, $)
  // (1, [square]):
  //    (1, [dot]) -> $(1, [square]) . $, lambda)
  // a:
  //    ((1, [square]) -> a . (1, [square]), $)
  //    ((1, [square]) -> lambda . a(1, [square]), $)
  //    ((1, [square]) -> lambda . b, $)
  // b:
  //    (1, [square]) -> b . lambda, $
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrtdollar);
  afterDot.push_back(ptrnt1square);
  afterDot.push_back(ptrtdollar);
  Lr1Configuration config(*nt1dot, beforeDot, afterDot, *tlambda);
  
  std::set<Lr1Configuration> configSet;
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtdollar);
  afterDot.push_back(ptrnt1square);
  afterDot.push_back(ptrtdollar);
  configSet.insert(Lr1Configuration(*nt1dot, beforeDot, afterDot, *tlambda));
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrta);
  afterDot.push_back(ptrnt1square);
  configSet.insert(Lr1Configuration(*nt1square, beforeDot, afterDot, *tdollar));
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrtb);
  configSet.insert(Lr1Configuration(*nt1square, beforeDot, afterDot, *tdollar));
  
  Lr1ConfigurationSetSet configSetSet;
  configSetSet.insert(configSet);
  
  LrmTransitionMap transitionMap = Lrm1(gr, 1, configSetSet);
  
  //std::cout << TransitionMapToStr(transitionMap) << std::endl;
  try{
    
    //size of the transition map must be three
    if(transitionMap.size() != 3){
      std::stringstream errstr;
      errstr << "number of successor config sets of config set (" << ConfigSetToStr(configSet) << ") was expected to be 3, but there were " << transitionMap.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }

  TestEnd(name);
}

/* *****************
 * TestLrmLoopAnB
 * *****************/
void TestLrmLoopAnB(){
  std::string name = "LrmLoop a^n b";
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
  
  //test with start state ((1, [dot]) -> lambda . $(1, [square])$, lambda)
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrtdollar);
  afterDot.push_back(ptrnt1square);
  afterDot.push_back(ptrtdollar);
  Lr1Configuration config(*nt1dot, beforeDot, afterDot, *tlambda);
  
  std::set<Lr1Configuration> configSet;
  configSet.insert(config);
  
  Lr1ConfigurationSetSet configSetSet;
  configSetSet.insert(configSet);
  
  std::pair<LrmTransitionMap,Lr1ConfigurationSetSet> transitionStates = LrmLoop(gr, 1, LrmTransitionMap(), Lr1ConfigurationSetSet(), configSetSet);
  
  LrmTransitionMap transitionMap = transitionStates.first;
  Lr1ConfigurationSetSet states = transitionStates.second;
  try{
    
    //size of the transition map must be eight
    if(transitionMap.size() != 8){
      std::stringstream errstr;
      errstr << "number of transitions was expected to be 8, but was " << transitionMap.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //the number of states must be seven
    if(states.size() != 7){
      std::stringstream errstr;
      errstr << "number of states was expected to be 7, but there were " << states.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }  
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestDescInitialFindTwo
 * *****************/
void TestDescInitialFindTwo(){
  std::string name = "DescInitial Find Two";
  TestStart(name);
  
  Grammar gr = TestGrammar3();
  
  //create terminals
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);

  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt2square = new Nonterminal(2,v);
  GrammarSymbolPtr ptrnt2square(nt2square);
  
  //create expected configurations
  GrammarSymbolVector beforeDot, afterDot;
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrta);
  afterDot.push_back(ptrnt1dot);
  Lr1Configuration config1(*nt1dot, beforeDot, afterDot, *tlambda);
  
  beforeDot.clear();
  afterDot.clear();
  beforeDot.push_back(ptrtlambda);
  afterDot.push_back(ptrtb);
  afterDot.push_back(ptrnt2square);
  Lr1Configuration config2(*nt1dot, beforeDot, afterDot, *tlambda);
  
  std::set<Lr1Configuration> expectedConfigSet;
  expectedConfigSet.insert(config1);
  expectedConfigSet.insert(config2);
  
  std::set<Lr1Configuration> configSet = DescInitial(gr);
  
  try{
    
    //size of the config set must be two
    if(configSet.size() != expectedConfigSet.size()){
      std::stringstream errstr;
      errstr << "number of configurations was expected to be " << expectedConfigSet.size() << ", but was " << configSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    std::set<Lr1Configuration>::const_iterator configit;
    //the expected configurations must be in the config set
    for(configit = expectedConfigSet.begin(); configit != expectedConfigSet.end(); configit++){
      if(configSet.find(*configit) == configSet.end()){
        std::stringstream errstr;
        errstr << "configurations " << configit->Str() << " was expected to be in the initial configuration set, but was not found" << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }  
    }
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestLrmGr1
 * *****************/
void TestLrmGr1(){
  std::string name = "Lrm Gr1";
  TestStart(name);
  
  Grammar gr = TestGrammar7();
  
  GotoGenerator gotoGen = Lrm(gr, 1);
  
  try{
    
    //number of transitions must be eight
    if(gotoGen.TransRelSize() != 8){
      std::stringstream errstr;
      errstr << "number of transitions was expected to be 8, but was " << gotoGen.TransRelSize() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //the number of states must be seven
    if(gotoGen.Size()!= 7){
      std::stringstream errstr;
      errstr << "number of states was expected to be 7, but there were " << gotoGen.Size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }  
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestLrmGr2
 * *****************/
void TestLrmGr2(){
  //TODO dont know what to expect as a result
//   std::string name = "Lrm Gr2";
//   TestStart(name);
//   
//   Grammar gr = TestGrammar8();
//   
//   GotoMachine gotoMachine = Lrm(gr, 1);
//   
//   LrmTransitionMap transitions = gotoMachine.Transitions();
//   Lr1ConfigurationSetSet states = gotoMachine.States();
//   
//   std::cout << "transitions: " << transitions.size() << ", states: " << states.size() << std::endl;
//   
//   std::cout << ConfigSetSetToStr(states) << std::endl;
//   std::cout << TransitionMapToStr(transitions) << std::endl;
//   
//   try{
//     
//     //size of the transition map must be 31
//     if(transitions.size() != 31){
//       std::stringstream errstr;
//       errstr << "number of transitions was expected to be 31, but was " << transitions.size() << "." << std::endl;
//       throw Exception(name, errstr.str(), 1003);
//     }
//     
//     //the number of states must be 25
//     if(states.size() != 25){
//       std::stringstream errstr;
//       errstr << "number of states was expected to be 25, but there were " << states.size() << "." << std::endl;
//       throw Exception(name, errstr.str(), 1003);
//     }  
//   }
//   catch (Exception e){ 
//   }
//   TestEnd(name);
}

/* *****************
 * TestAugSuccess
 * *****************/
void TestAugSuccess(){
  std::string name = "Aug Success";
  TestStart(name);
  
  Grammar gr = TestGrammar6();
  
  //create terminals
  PushdownGenerator gen;
  gen.InsEvent("$");
  Terminal* tdollar = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("$"));
  GrammarSymbolPtr ptrtdollar(tdollar);

  //create nonterminals
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt0dot = new Nonterminal(0,v);
  GrammarSymbolPtr ptrnt0dot(nt0dot);
  
  //create grammar production
  GrammarSymbolVector rhs;
  rhs.push_back(ptrtdollar);
  rhs.push_back(ptrnt1dot);
  rhs.push_back(ptrtdollar);
  GrammarProduction gp(*nt0dot,rhs);
  
  Grammar rGr = Aug(gr, *nt0dot, *tdollar);
  
  try{
    
    //the must be one grammar production more in the new grammar
    if(gr.GrammarProductions().size() + 1 != rGr.GrammarProductions().size()){
      std::stringstream errstr;
      errstr << "number of grammar productions was expected to be " << gr.GrammarProductions().size() + 1 << ", but was " << rGr.GrammarProductions().size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    std::set<GrammarProduction>::const_iterator gpit;
    //there must be one production with the start symbol as left-hand side
    int i = 0;
    for(gpit = rGr.GrammarProductionsBegin(); gpit != rGr.GrammarProductionsEnd(); gpit++){
      if(gpit->Lhs() == *nt0dot){
        i++;
      }
    }
    if(i != 1){
      std::stringstream errstr;
      errstr << "number of grammar productions with " << nt0dot->Str() << " as right-hand side was expected to be 1, but was " << i << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //the new start production must be in the grammar's productions
    if(gr.GrammarProductions().find(gp) == rGr.GrammarProductionsEnd()){
      std::stringstream errstr;
      errstr << "grammar production " << gp.Str() << " was expected to be in the grammar, but was not found." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  TestEnd(name);
}

/* *****************
 * TestDesc11
 * *****************/
void TestDesc11(){
  TestDesc11Terminal();
  TestDesc11Nonterminal();
}

/* *****************
 * TestDesc
 * *****************/
void TestDesc(){
  TestDescSelectedConfigs();
}

/* *****************
 * TestPassesX
 * *****************/
void TestPassesX(){
  TestPassesXNonterminal();
  TestPassesXTerminal();
  TestPassesXNoShift(); 
}

/* *****************
 * TestLrm1
 * *****************/
void TestLrm1(){
  TestLrm1FindOne();
  TestLrm1FindThree();
}

/* *****************
 * TestLrmLoop
 * *****************/
void TestLrmLoop(){
 TestLrmLoopAnB(); 
}

/* *****************
 * TestDescInitial
 * *****************/
void TestDescInitial(){
  TestDescInitialFindTwo();
}

/* *****************
 * TestDescInitial
 * *****************/
void TestLrm(){
  TestLrmGr1(); 
  //TestLrmGr2();
}

/* *****************
 * TestAug
 * *****************/
void TestAug(){
  TestAugSuccess();
}
  
} // namespace faudes

