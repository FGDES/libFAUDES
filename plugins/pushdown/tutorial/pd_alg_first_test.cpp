/** @file pd_alg_first_test.cpp  Unit Tests */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/
#include "pd_alg_first_test.h"

namespace faudes {
 
  
/* *****************
 * TestPostClPostfixes
 * *****************/  
void TestPostClPostfixes(){
  std::string name = "PostCl Postfixes";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  GrammarSymbolVector word;
  GrammarSymbolWordSet wordSet;

  //create terminals (a) and (lambda)
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  //create nonterminals (1, [dot]) and (2, [square], 1)
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt2square1 = new Nonterminal(2,v,1);
  //create word ((2, [square], 1)a(1, [dot]))
  GrammarSymbolPtr ptrnt2square1(nt2square1);
  word.push_back(ptrnt2square1);
  GrammarSymbolPtr ptrta(ta);
  word.push_back(ptrta);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  word.push_back(ptrnt1dot);
  
  wordSet = PostCl(word);
  
  try{
    
    //the set must contain::
    GrammarSymbolWordSet expectedWords;
    GrammarSymbolWordSet::const_iterator wordit;

    //(2, [square], 1)a(1, [dot])
    GrammarSymbolVector v2square1a1dot;
    v2square1a1dot.push_back(ptrnt2square1);
    v2square1a1dot.push_back(ptrta);
    v2square1a1dot.push_back(ptrnt1dot);    
    expectedWords.insert(v2square1a1dot);
    //a(1, [dot])
    GrammarSymbolVector va1dot;
    va1dot.push_back(ptrta);
    va1dot.push_back(ptrnt1dot);
    expectedWords.insert(va1dot);
    //(1, [dot])
    GrammarSymbolVector v1dot;
    v1dot.push_back(ptrnt1dot);
    expectedWords.insert(v1dot);
    //lambda
    GrammarSymbolVector v1lambda;
    GrammarSymbolPtr ptrtlambda(tlambda);
    v1lambda.push_back(ptrtlambda);
    expectedWords.insert(v1lambda);
    
    //the set must contain 4 items
    if(wordSet.size() != expectedWords.size()){
      std::stringstream errstr;
      errstr << "Word set size was " << wordSet.size() << ", but " << expectedWords.size() << " was expected." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //check for specific items
    for(wordit = expectedWords.begin(); wordit != expectedWords.end(); wordit++){
      if(!ContainsWord(wordSet, *wordit)){
        std::stringstream errstr;
        errstr << "Word set did not contain the expected word ";
        
        GrammarSymbolVector::const_iterator gsit;
        for(gsit = wordit->begin(); gsit != wordit->end(); gsit++){
          errstr << (*gsit)->Str();
        }
        errstr << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
  
}

/* *****************
 * TestFdsWords
 * *****************/
void TestFdsWords(){
  std::string name = "Fds Words";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar4();
  
  GrammarSymbolWordSet wordSet = Fds(gr);
  
  try{
    
    //create terminals (a) and (lambda)
    Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
    GrammarSymbolPtr ptrta(ta);
    Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
    GrammarSymbolPtr ptrtlambda(tlambda);
    //create nonterminals (1, [dot]), (1, [dot], 1) and (2,square)
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
    Nonterminal* nt2square = new Nonterminal(2,v);
    GrammarSymbolPtr ptrnt2square(nt2square);
    
    //the set must contain::
    GrammarSymbolWordSet expectedWords;
    GrammarSymbolWordSet::const_iterator wordit;
    
    //(1, [dot])
    GrammarSymbolVector v1dot;
    v1dot.push_back(ptrnt1dot);
    expectedWords.insert(v1dot);
    //a(1, [dot])
    GrammarSymbolVector va1dot;
    va1dot.push_back(ptrta);
    va1dot.push_back(ptrnt1dot);
    expectedWords.insert(va1dot);
    //lambda
    GrammarSymbolVector v1lambda;
    v1lambda.push_back(ptrtlambda);
    expectedWords.insert(v1lambda);
    //(2, [square])
    GrammarSymbolVector v2square;
    v2square.push_back(ptrnt2square);
    expectedWords.insert(v2square);
    //a(2, [square])
    GrammarSymbolVector va2square;
    va2square.push_back(ptrta);
    va2square.push_back(ptrnt2square);
    expectedWords.insert(va2square);
    //(1, [dot] ,1)a(2, [square])
    GrammarSymbolVector v1dot1a2square;
    v1dot1a2square.push_back(ptrnt1dot1);
    v1dot1a2square.push_back(ptrta);
    v1dot1a2square.push_back(ptrnt2square);
    expectedWords.insert(v1dot1a2square);
    
    
    //the set must contain 6 items
    if(wordSet.size() != expectedWords.size()){
      std::stringstream errstr;
      errstr << "Word set size was " << wordSet.size() << ", but " << expectedWords.size() << " was expected." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //check for specific items
    for(wordit = expectedWords.begin(); wordit != expectedWords.end(); wordit++){
      if(!ContainsWord(wordSet, *wordit)){
        std::stringstream errstr;
        errstr << "Word set did not contain the expected word ";
        
        GrammarSymbolVector::const_iterator gsit;
        for(gsit = wordit->begin(); gsit != wordit->end(); gsit++){
          errstr << (*gsit)->Str();
        }
        errstr << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
    
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestFirstLMap
 * *****************/
void TestFirstLMap(){
  std::string name = "FirstL Map";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar5();
  
  GrammarSymbolWordMap f = FirstL(gr,GrammarSymbolWordMap());
  
  /* 
   * the expected map is as follows:
   * 
   * lambda -> lambda
   * b -> b
   * (1, dot) -> a
   * (1, square) -> b, lambda
   * (1, dot, 1) -> a
   * a(1, square) -> a
   * (1, dot, 1)a(1, square) -> a
   * (2, square) -> a, b
   * (1, square)(1, dot) -> a, b
   */
  try{
    
    //create terminals (a), (b) and (lambda)
    Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
    GrammarSymbolPtr ptrta(ta);
    Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
    GrammarSymbolPtr ptrtb(tb);
    Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
    GrammarSymbolPtr ptrtlambda(tlambda);
    //create nonterminals (1, [dot]), (1, [dot], 1), (1, [square]) and (2,[square])
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
    v.clear();
    v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
    Nonterminal* nt2square = new Nonterminal(2,v);
    GrammarSymbolPtr ptrnt2square(nt2square);
    
    //test size of result set and result set contents for each word
    //lambda
    GrammarSymbolVector gsv;
    gsv.push_back(ptrtlambda);
    if(f.find(gsv)->second.size() != 1){
      std::stringstream errstr;
      errstr << "Word set of word lambda was expected to be of size 1, but was of size " << f.find(gsv)->second.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(f.find(gsv)->second.find(*tlambda) == f.find(gsv)->second.end()){
      std::stringstream errstr;
      errstr << "Word set of word lambda did not contain the expected word lambda." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //b
    gsv.clear();
    gsv.push_back(ptrtb);
    if(f.find(gsv)->second.size() != 1){
      std::stringstream errstr;
      errstr << "Word set of word b was expected to be of size 1, but was of size " << f.find(gsv)->second.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(f.find(gsv)->second.find(*tb) == f.find(gsv)->second.end()){
      std::stringstream errstr;
      errstr << "Word set of word b did not contain the expected word b." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //(1, dot)
    gsv.clear();
    gsv.push_back(ptrnt1dot);
    if(f.find(gsv)->second.size() != 1){
      std::stringstream errstr;
      errstr << "Word set of word (1, dot) was expected to be of size 1, but was of size " << f.find(gsv)->second.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(f.find(gsv)->second.find(*ta) == f.find(gsv)->second.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, dot) did not contain the expected word a." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //(1, square)
    gsv.clear();
    gsv.push_back(ptrnt1square);
    if(f.find(gsv)->second.size() != 2){
      std::stringstream errstr;
      errstr << "Word set of word (1, square) was expected to be of size 2, but was of size " << f.find(gsv)->second.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(f.find(gsv)->second.find(*tb) == f.find(gsv)->second.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, square) did not contain the expected word b." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(f.find(gsv)->second.find(*tlambda) == f.find(gsv)->second.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, square) did not contain the expected word lambda." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //(1, dot, 1)
    gsv.clear();
    gsv.push_back(ptrnt1dot1);
    if(f.find(gsv)->second.size() != 1){
      std::stringstream errstr;
      errstr << "Word set of word (1, dot, 1) was expected to be of size 1, but was of size " << f.find(gsv)->second.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(f.find(gsv)->second.find(*ta) == f.find(gsv)->second.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, dot, 1) did not contain the expected word a." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //a(1, square)
    gsv.clear();
    gsv.push_back(ptrta);
    gsv.push_back(ptrnt1square);
    if(f.find(gsv)->second.size() != 1){
      std::stringstream errstr;
      errstr << "Word set of word a(1, square) was expected to be of size 1, but was of size " << f.find(gsv)->second.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(f.find(gsv)->second.find(*ta) == f.find(gsv)->second.end()){
      std::stringstream errstr;
      errstr << "Word set of word a(1, square) did not contain the expected word a." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //(1, dot, 1)a(1, square)
    gsv.clear();
    gsv.push_back(ptrnt1dot1);
    gsv.push_back(ptrta);
    gsv.push_back(ptrnt1square);
    if(f.find(gsv)->second.size() != 1){
      std::stringstream errstr;
      errstr << "Word set of word (1, dot, 1)a(1, square) was expected to be of size 1, but was of size " << f.find(gsv)->second.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(f.find(gsv)->second.find(*ta) == f.find(gsv)->second.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, dot, 1)a(1, square) did not contain the expected word a." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //(2, square)
    gsv.clear();
    gsv.push_back(ptrnt2square);
    if(f.find(gsv)->second.size() != 2){
      std::stringstream errstr;
      errstr << "Word set of word (2, square) was expected to be of size 2, but was of size " << f.find(gsv)->second.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(f.find(gsv)->second.find(*ta) == f.find(gsv)->second.end()){
      std::stringstream errstr;
      errstr << "Word set of word (2, square) did not contain the expected word a." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(f.find(gsv)->second.find(*tb) == f.find(gsv)->second.end()){
      std::stringstream errstr;
      errstr << "Word set of word (2, square) did not contain the expected word b." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //(1, square)(1, dot)
    gsv.clear();
    gsv.push_back(ptrnt1square);
    gsv.push_back(ptrnt1dot);
    if(f.find(gsv)->second.size() != 2){
      std::stringstream errstr;
      errstr << "Word set of word (1, square)(1, dot) was expected to be of size 2, but was of size " << f.find(gsv)->second.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(f.find(gsv)->second.find(*ta) == f.find(gsv)->second.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, square)(1, dot) did not contain the expected word a." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(f.find(gsv)->second.find(*tb) == f.find(gsv)->second.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, square)(1, dot) did not contain the expected word b." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestFirstASelectedWords
 * *****************/
void TestFirstASelectedWords(){
  std::string name = "FirstA Selected Words";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar5();
  
  //create terminals (a), (b) and (lambda)
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  //create nonterminals (1, [dot]) (1, [square])
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);    
  
  //test for lambda -> lambda
  GrammarSymbolVector word;
  word.push_back(ptrtlambda);  
  std::set<Terminal> firstSet = FirstA(gr, word);
  try{

    if(firstSet.size() != 1){
      std::stringstream errstr;
      errstr << "Word set of word lambda was expected to be of size 1, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*tlambda) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word lambda did not contain the expected word lambda." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  //test for (1, square) -> b, lambda
  word.clear();
  word.push_back(ptrnt1square);  
   firstSet = FirstA(gr, word);
  try{

    if(firstSet.size() != 2){
      std::stringstream errstr;
      errstr << "Word set of word (1, square) was expected to be of size 2, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*tb) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, square) did not contain the expected word b." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*tlambda) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, square) did not contain the expected word lambda." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  //test for (1, square)(1, dot) -> a, b
  word.clear();
  word.push_back(ptrnt1square);
  word.push_back(ptrnt1dot);  
   firstSet = FirstA(gr, word);
  try{

    if(firstSet.size() != 2){
      std::stringstream errstr;
      errstr << "Word set of word (1, square)(1, dot) was expected to be of size 2, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*ta) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, square)(1, dot) did not contain the expected word a." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*tb) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, square)(1, dot) did not contain the expected word b." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestFirstSelectedWords
 * *****************/
void TestFirstSelectedWords(){
  std::string name = "First Selected Words";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar5();
  
  //create terminals (a), (b) and (lambda)
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  //create nonterminals (1, [dot]), (1, [square]) and (2, [square])
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);    
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt2square = new Nonterminal(2,v);
  GrammarSymbolPtr ptrnt2square(nt2square);    
  
  //test for lambda -> lambda
  GrammarSymbolVector word;
  word.push_back(ptrtlambda);  
  std::set<Terminal> firstSet = First(gr, word);
  try{

    if(firstSet.size() != 1){
      std::stringstream errstr;
      errstr << "Word set of word lambda was expected to be of size 1, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*tlambda) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word lambda did not contain the expected word lambda." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  //test for (2, square)(1, dot) -> a, b
  word.clear();
  word.push_back(ptrnt2square);
  word.push_back(ptrnt1dot);
  firstSet = First(gr, word);
  try{

    if(firstSet.size() != 2){
      std::stringstream errstr;
      errstr << "Word set of word (2, square)(1, dot) was expected to be of size 2, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*tb) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word (2, square)(1, dot) did not contain the expected word b." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*ta) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word (2, square)(1, dot) did not contain the expected word a." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  //test for (1, square)(1, dot) -> a, b
  word.clear();
  word.push_back(ptrnt1square);
  word.push_back(ptrnt1dot);  
   firstSet = First(gr, word);
  try{

    if(firstSet.size() != 2){
      std::stringstream errstr;
      errstr << "Word set of word (1, square)(1, dot) was expected to be of size 2, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*ta) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, square)(1, dot) did not contain the expected word a." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*tb) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, square)(1, dot) did not contain the expected word b." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestFirstRedConsistentWord
 * *****************/
void TestFirstRedConsistentWord(){
  std::string name = "FirstRed Consistent Word";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar5();
  
  //create terminals (b), lambda
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  //create nonterminal (1, [square])
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1square(nt1square);    
  
  
  //test for (1, square) -> b, lambda
  GrammarSymbolVector word;
  word.push_back(ptrnt1square);
  std::set<Terminal> firstSet = FirstRed(gr, word);
  try{

    if(firstSet.size() != 2){
      std::stringstream errstr;
      errstr << "Word set of word (1, square) was expected to be of size 2, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*tb) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, square) did not contain the expected word b." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*tlambda) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, square) did not contain the expected word lambda." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestFirstRedInconsistentWord
 * *****************/
void TestFirstRedInconsistentWord(){
  std::string name = "FirstRed Inconsistent Word";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar5();
  
  //create terminal (a)
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);

  //create nonterminal (1, [dot], 2)
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot2 = new Nonterminal(1,v,2);
  GrammarSymbolPtr ptrnt1dot2(nt1dot2);   

  
  //test for (1, dot, 2) -> empty
  GrammarSymbolVector word;
  word.push_back(ptrnt1dot2);  
  std::set<Terminal> firstSet = FirstRed(gr, word);
  try{

    if(!firstSet.empty()){
      std::stringstream errstr;
      errstr << "Word set of word (1, dot, 2) was expected to be empty, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
  
}

/* *****************
 * TestFilter1MixedGrammarSymbols
 * *****************/
void TestFilter1MixedGrammarSymbols(){
  
  std::string name = "Filter1 Mixed Grammar Symbols";
  TestStart(name);
  
  Grammar gr1 = TestGrammar1();
  
  GrammarSymbolVector word;
  std::set<Terminal>::const_iterator tit;
  std::vector<TerminalPtr> tpv;
  std::vector<TerminalPtr>::iterator tpvit;
  
  //build word to filter
  //contains two nonterminal and all terminals
  Terminal* t;
  Nonterminal* nt = new Nonterminal(*gr1.Nonterminals().begin());
  GrammarSymbolPtr ntPtr(nt);
  word.push_back(ntPtr);
  word.push_back(ntPtr);

  for(tit = gr1.Terminals().begin(); tit != gr1.Terminals().end(); tit++){
    //create new terminal
    t = new Terminal(*tit);
    TerminalPtr tPtr(t);
    //push terminal into word
    word.push_back(tPtr);
  }
  
  NonterminalPtr rPtr = Filter1(gr1.Nonterminals(),word);
  
  try{
    //resulting nonterminal needs to be the same the nonterminal in the word 
    if(*rPtr != *ntPtr){
      std::stringstream errstr;
      errstr << "result nonterminal is " << rPtr->Str() << ", but " << ntPtr->Str() << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestFilter1Nothing
 * *****************/
void TestFilter1Nothing(){
  std::string name = "Filter1 Nothing";
  TestStart(name);
  
  std::set<Nonterminal> nt;
  GrammarSymbolVector gs;
  
  NonterminalPtr rPtr = Filter1(nt,gs);
  
  try{
    //resulting pointer has to be NULL
    if(rPtr != NULL){
      std::stringstream errstr;
      errstr << "size of result set is no NULL" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}


/* *****************
 * TestFirstAllTerminalsOnly
 * *****************/
void TestFirstAllTerminalsOnly(){
  std::string name = "FirstAll Terminals Only";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar6();
  
  //create terminals (a), (b)
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  Terminal* tb = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("b"));
  GrammarSymbolPtr ptrtb(tb);
  
  //test for ab -> a
  GrammarSymbolVector word;
  word.push_back(ptrta);  
  word.push_back(ptrtb);  
  std::set<Terminal> firstSet = FirstAll(gr, word);
  try{

    if(firstSet.size() != 1){
      std::stringstream errstr;
      errstr << "Word set of word ab was expected to be of size 1, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*ta) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word ab did not contain the expected word a." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestFirstAllNonterminalsIrreducibleWord
 * *****************/
void TestFirstAllNonterminalsIrreducibleWord(){
  std::string name = "FirstAll Nonterminals Irreducible Word";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar6();
  
  //create nonterminal (1, [square], 1)
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square1 = new Nonterminal(1,v,1);
  GrammarSymbolPtr ptrnt1square1(nt1square1);
  
  //test for (1, square, 1) -> empty
  GrammarSymbolVector word;
  word.push_back(ptrnt1square1);  
  std::set<Terminal> firstSet = FirstAll(gr, word);
  try{

    if(!firstSet.empty()){
      std::stringstream errstr;
      errstr << "Word set of word (1, square, 1) was expected to be empty, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
  
}

/* *****************
 * TestFirstAllNonterminalsReducibleWord
 * *****************/
void TestFirstAllNonterminalsReducibleWord(){
  std::string name = "FirstAll Nonterminals Reducible Word";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar6();
  
  //create terminal (a)
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  
  //create nonterminals (1, [dot], 1) and (1, [dot])
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  v.clear();
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot1 = new Nonterminal(1,v,1);
  GrammarSymbolPtr ptrnt1dot1(nt1dot1);
  
  //test for (1, [dot])(1, [dot], 1) -> a
  GrammarSymbolVector word;
  word.push_back(ptrnt1dot);  
  word.push_back(ptrnt1dot1);  
  std::set<Terminal> firstSet = FirstAll(gr, word);
  try{

    if(firstSet.size() != 1){
      std::stringstream errstr;
      errstr << "Word set of word (1, [dot])(1, [dot], 1) was expected to be of size 1, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*ta) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of word (1, [dot])(1, [dot], 1) did not contain the expected word a." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestFirstLeq1ZeroEmpty
 * *****************/
void TestFirstLeq1ZeroEmpty(){
  std::string name = "FirstLeq1 Zero Empty";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar6();
  
  //create nonterminals (1, [square], 1)
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("square"));
  Nonterminal* nt1square1 = new Nonterminal(1,v,1);
  GrammarSymbolPtr ptrnt1square1(nt1square1);

  //test for 0, (1, [square], 1) -> empty
  GrammarSymbolVector word;
  word.push_back(ptrnt1square1);  
  std::set<Terminal> firstSet = FirstLeq1(gr, 0, word);
  try{

    if(!firstSet.empty()){
      std::stringstream errstr;
      errstr << "Word set of k = 0 and word (1, [square], 1) was expected to be empty, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestFirstLeq1ZeroNonempty
 * *****************/
void TestFirstLeq1ZeroNonempty(){
  std::string name = "FirstLeq1 Zero Nonempty";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar6();
  
  //create terminal lambda
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptrtlambda(tlambda);
  
  //create nonterminals (1, [dot])
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  //test for 0, (1, [dot]) -> lambda
  GrammarSymbolVector word;
  word.push_back(ptrnt1dot);  
  std::set<Terminal> firstSet = FirstLeq1(gr, 0, word);
  try{

    if(firstSet.size() != 1){
      std::stringstream errstr;
      errstr << "Word set of k = 0 and word (1, [dot]) was expected to be of size 1, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*tlambda) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of k = 0 and word (1, [dot]) did not contain the expected word lambda." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestFirstLeq1OneNonempty
 * *****************/
void TestFirstLeq1OneNonempty(){
  std::string name = "FirstLeq1 One Nonempty";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar6();
  
  //create terminal a
  Terminal* ta = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index("a"));
  GrammarSymbolPtr ptrta(ta);
  
  //create nonterminals (1, [dot])
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);
  
  //test for 1, (1, [dot]) -> a
  GrammarSymbolVector word;
  word.push_back(ptrnt1dot);  
  std::set<Terminal> firstSet = FirstLeq1(gr, 1, word);
  try{

    if(firstSet.size() != 1){
      std::stringstream errstr;
      errstr << "Word set of k = 1 and word (1, [dot]) was expected to be of size 1, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(firstSet.find(*ta) == firstSet.end()){
      std::stringstream errstr;
      errstr << "Word set of k = 1 and word (1, [dot]) did not contain the expected word lambda." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestFirstLeq1TwoNonempty
 * *****************/
void TestFirstLeq1TwoNonempty(){
  std::string name = "FirstLeq1 Two Nonempty";
  TestStart(name);
  
  //just to populate events and stack symbols
  TestGenerator1();
  
  Grammar gr = TestGrammar6();
  
  //create nonterminals (1, [dot])
  std::vector<Idx> v;
  v.push_back(PushdownGenerator::GlobalStackSymbolTablep()->Index("dot"));
  Nonterminal* nt1dot = new Nonterminal(1,v);
  GrammarSymbolPtr ptrnt1dot(nt1dot);

  //test for 2, (1, [dot]) -> empty
  GrammarSymbolVector word;
  word.push_back(ptrnt1dot);  
  std::set<Terminal> firstSet = FirstLeq1(gr, 2, word);
  try{

   if(!firstSet.empty()){
      std::stringstream errstr;
      errstr << "Word set of k = 2 and word (1, [dot]) was expected to be empty, but was of size " << firstSet.size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestPostCl
 * *****************/
void TestPostCl(){
  TestPostClPostfixes();  
}

/* *****************
 * TestFds
 * *****************/
void TestFds(){
  TestFdsWords();  
}

/* *****************
 * TestFirstA
 * *****************/
void TestFirstA(){
  TestFirstASelectedWords();
}

/* *****************
 * TestFirstL
 * *****************/
void TestFirstL(){
  TestFirstLMap();
}

/* *****************
 * TestFirst
 * *****************/
void TestFirst(){
  TestFirstSelectedWords();
}


/* *****************
 * TestFirstRed
 * *****************/
void TestFirstRed(){
  TestFirstRedConsistentWord();
  TestFirstRedInconsistentWord();
}

/* *****************
 * TestFilter1
 * *****************/
void TestFilter1(){
  TestFilter1MixedGrammarSymbols();
  TestFilter1Nothing();
}

/* *****************
 * TestFirstAll
 * *****************/
void TestFirstAll(){
  TestFirstAllTerminalsOnly();
  TestFirstAllNonterminalsReducibleWord();
  TestFirstAllNonterminalsIrreducibleWord();
}

/* *****************
 * TestFirstLeq1
 * *****************/
void TestFirstLeq1(){
  TestFirstLeq1ZeroEmpty();
  TestFirstLeq1ZeroNonempty();
  TestFirstLeq1OneNonempty();
  TestFirstLeq1TwoNonempty();
}

} // namespace faudes

