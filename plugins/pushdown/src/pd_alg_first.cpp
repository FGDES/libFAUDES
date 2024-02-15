/** @file pd_alg_first.cpp  functions related to the first function*/


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_alg_first.h"

namespace faudes {

  
/* *************************
 * PostCl
 * *************************/
GrammarSymbolWordSet PostCl(const GrammarSymbolVector& word){

  GrammarSymbolWordSet rSet;
  GrammarSymbolVector postfix;
  GrammarSymbolVector::const_reverse_iterator vit;
  //iterate backwards over the word, thus building all postfixes
  for(vit = word.rbegin(); vit != word.rend(); vit++){
    
    postfix.insert(postfix.begin(),*vit);
    rSet.insert(postfix);
  }
  
  //lambda always belongs in the set
  GrammarSymbolVector lambda;
  Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr ptr(tlambda);
  lambda.push_back(ptr);
  rSet.insert(lambda);
  
  return rSet;
}

/* *************************
 * Fds
 * *************************/
GrammarSymbolWordSet Fds(const Grammar& gr){

  GrammarSymbolWordSet rSet, postfixSet;
  GrammarSymbolVector gsVector;
  std::set<GrammarProduction>::const_iterator gpit;
  
  //iterate over all grammar productions
  for(gpit = gr.GrammarProductionsBegin(); gpit != gr.GrammarProductionsEnd(); gpit++){
    
    gsVector.clear();
    
    //add lefthand side
    Nonterminal* nt = new Nonterminal(gpit->Lhs());
    NonterminalPtr ntPtr(nt);
    gsVector.push_back(ntPtr);
    rSet.insert(gsVector);
    
    //add postfixes of righthand side
    postfixSet = PostCl(gpit->Rhs());
    rSet.insert(postfixSet.begin(), postfixSet.end());
    
  }
  return rSet;
}

/* *************************
 * First1
 * *************************/
GrammarSymbolWordMap First1(const Grammar& gr, const GrammarSymbolWordMap& f, bool* madeChanges){

  //take a copy rf of the old first function f for extending and returning
  GrammarSymbolWordMap rf = f;
  
  //helper variable to determine if changes were made from f to rf
  bool changed = false;
  *madeChanges = false;
  
  //get all postfixes and lefthand sides of the grammar
  GrammarSymbolWordSet words = Fds(gr);
  
  GrammarSymbolWordSet::const_iterator wordit;
  std::set<Terminal>::const_iterator insertWordit;
  
  std::set<GrammarProduction>::const_iterator gpit;
  //for every word w in Fds(gr)
  for(wordit = words.begin(); wordit != words.end(); wordit++){
    
    //get the first symbol of the word w = a.w'
    GrammarSymbolPtr firstSymbol = wordit->front();
    
    //if the first symbol is a terminal
    TerminalPtr terminalA = std::dynamic_pointer_cast<Terminal>(firstSymbol);
    if(terminalA){
      
      std::set<Terminal> rfw;
      //add the terminal to the first function rf
      //if w does not already exist in rf, create it
      if(rf.find(*wordit) == rf.end()){
        rf.insert(std::make_pair(*wordit, std::set<Terminal>()));
      }
      
      //add rf(w) = a to the first function rf
      changed = rf.find(*wordit)->second.insert(*terminalA).second;
      if(changed) *madeChanges = true;
    }
    
    //if the first symbol is a nonterminal
    NonterminalPtr nonterminalA = std::dynamic_pointer_cast<Nonterminal>(firstSymbol);
    if(nonterminalA){
      
      std::set<Terminal> directDependencies;
      
      //get the first symbols f(x) for every production (a-->x)
      for(gpit = gr.GrammarProductionsBegin(); gpit != gr.GrammarProductionsEnd(); gpit++){
        
        //check if the production has the nonterminal a on its lefthand side and if the
        //righthand side x exists in f
        if(gpit->Lhs() == *nonterminalA){
          if(f.find(gpit->Rhs()) != f.end()){
            
            //get f(x) and add it to directDependencies
            std::set<Terminal> fx = f.find(gpit->Rhs())->second;
            directDependencies.insert(fx.begin(),fx.end());
          }
        }
      }
      
      //construct the symbol a ...
      GrammarSymbolVector a;
      a.push_back(nonterminalA);
      //... and get the first symbols of a (f(a))
      std::set<Terminal> fa;
      if(f.find(a) != f.end()){
        fa = f.find(a)->second;
      }
      
      //get w' (the second part of the current word w = a.w'),
      GrammarSymbolVector wBack(wordit->begin() + 1, wordit->end());//can never go out of bounds because of outermost for loop
      
      //if w' is not empty, remove lambda from f(a) and the direct dependencies,
      //because eliminability of a does not imply eliminability of w'
      GrammarSymbolVector lambdaWord;
      Terminal tlambda(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
      
      //remove lambda and remember for later if lambda was in f(a)
      bool faContainsLambda = false;
      if(!wBack.empty()){  
        if(fa.erase(tlambda) == 1){
          faContainsLambda = true;
        }
        directDependencies.erase(tlambda);
      }
      
      //add new items to rf(w)
      std::set<Terminal> rfw;
      //if w does not already exist in rf, create it
      if(rf.find(*wordit) == rf.end()){
        rf.insert(std::make_pair(*wordit, std::set<Terminal>()));
      }

      //add f(a)
      for(insertWordit = fa.begin(); insertWordit != fa.end(); insertWordit++){
        changed = rf.find(*wordit)->second.insert(*insertWordit).second;
        if(changed) *madeChanges = true;
      }
      
      //add direct dependencies
      for(insertWordit = directDependencies.begin(); insertWordit != directDependencies.end(); insertWordit++){
        changed = rf.find(*wordit)->second.insert(*insertWordit).second;
        if(changed) *madeChanges = true;
      }
              
      //if a is eliminable, add f(w') to f(w)
      if(faContainsLambda){
        std::set<Terminal> fwBack;
        //get f(w')
        if(f.find(wBack) != f.end()){
          fwBack = f.find(wBack)->second;
        }
        
        //add f(w')
        for(insertWordit = fwBack.begin(); insertWordit != fwBack.end(); insertWordit++){
          changed = rf.find(*wordit)->second.insert(*insertWordit).second;
          if(changed) *madeChanges = true;
        }
      }
    }
  }
  return rf;  
}

/* *************************
 * FirstL
 * *************************/
GrammarSymbolWordMap FirstL(const Grammar& gr, const GrammarSymbolWordMap& f){

  //calculate first function
  bool changed;
  GrammarSymbolWordMap fNew = First1(gr,f,&changed);
  
  //repeat until no changes are made
  if(changed){
    fNew = FirstL(gr,fNew);
  }
  
  return fNew;
}

/* *************************
 * FirstA
 * *************************/
std::set<Terminal> FirstA(const Grammar& gr, const GrammarSymbolVector& word){

  //calculate first function
  GrammarSymbolWordMap f = FirstL(gr,f);
  
  GrammarSymbolWordMap::const_iterator fit = f.find(word);
  if(fit == f.end()){
    GrammarSymbolVector::const_iterator vit;
    std::stringstream errstr;
    errstr << "word " << std::endl;
    for(vit = word.begin(); vit != word.end(); vit++){
      errstr << (*vit)->Str();
    }
    errstr << " was not found in the first function";
    throw Exception("FirstA", errstr.str(), 1001);
  }
  
  return fit->second;
}

/* *************************
 * First
 * *************************/
std::set<Terminal> First(const Grammar& gr, const GrammarSymbolVector& word){

  //"empty" words must still contain lambda!
  if(word.empty()){
    std::stringstream errstr;
    errstr << "word parameter was empty. try calling it with lambda instead of an empty vector.";
    throw Exception("First", errstr.str(), 1001);
  }
  
  //set of nonterminals to return
  std::set<Terminal> rSet;
  
  //if the first symbol of the word is a terminal (or lambda)
  TerminalPtr t = std::dynamic_pointer_cast<Terminal>(word.front());
  if(t) {
    
    //insert the terminal (can be lambda)
    rSet.insert(*t);
  }
  
  //if the first symbol of the word is a nonterminal
  NonterminalPtr nt = std::dynamic_pointer_cast<Nonterminal>(word.front());
  if(nt){
    
    //get first set of the nonterminal
    GrammarSymbolVector v;
    v.push_back(nt);
    rSet = FirstA(gr,v);
    
    //if the word contains more symbols and the set contains, it needs to erased
    //be removed from the set
    if(word.size() > 1){
      
      Terminal tlambda(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
      //the set contains lambda (the nonterminal is eliminable). the remaining word
      //needs to be examined
      if(rSet.erase(tlambda) == 1){
        
        //recursive call of first
        std::set<Terminal> recSet = First(gr, GrammarSymbolVector(word.begin() + 1, word.end()));
        rSet.insert(recSet.begin(), recSet.end());
      }
    }
  }
  return rSet;
}

/* *************************
 * FirstRed
 * *************************/
std::set<Terminal> FirstRed(const Grammar& gr, const GrammarSymbolVector& word){

  //set to return
  std::set<Terminal> rSet;
  
  //if every symbol of the word is a symbol of the grammar, the word is consistent
  bool consistent = true;
  GrammarSymbolVector::const_iterator wordit;
  std::set<Terminal>::const_iterator tit;
  std::set<Nonterminal>::const_iterator ntit;
  
  //check every symbol of the word
  for(wordit = word.begin(); wordit != word.end(); wordit++){
    
    //if it's a terminal
    TerminalPtr t = std::dynamic_pointer_cast<Terminal>(*wordit);
    if(t) {
      //if it's not in the grammar's terminals
      if(gr.Terminals().find(*t) == gr.TerminalsEnd()){
        consistent = false;
        break;
      }
    }
    
    //if it's a nonterminal
    NonterminalPtr nt = std::dynamic_pointer_cast<Nonterminal>(*wordit);
    if(nt) {
      //if it's not in the grammar's nonterminals
      if(gr.Nonterminals().find(*nt) == gr.NonterminalsEnd()){
        consistent = false;
        break;
      }
    }
  }
  
  //call first only if it's consistent
  if(consistent){
    rSet = First(gr, word);
  }
  //else return empty set
  return rSet;
}

/* *************************
 * Filter1
 * *************************/
NonterminalPtr Filter1(const std::set<Nonterminal>& symbolSet, const GrammarSymbolVector& w){

  GrammarSymbolVector::const_iterator wit;
  std::set<Nonterminal>::const_iterator ntit;
  //iterate over all symbols of the word
  for(wit = w.begin(); wit != w.end(); wit++){
    //test if current symbol is a nonterminal
    ConstNonterminalPtr nt = std::dynamic_pointer_cast<const Nonterminal>(*wit);
    if(nt){
          
      //look for the symbol in the nonterminal set
      ntit = symbolSet.find(*nt);
      
      //if the symbol was found, return it
      if(ntit != symbolSet.end()){
        Nonterminal* rNt = new Nonterminal(*nt);
        return NonterminalPtr(rNt);
      }
    }
  }
  //else return NULL
  return NonterminalPtr();
}

/* *************************
 * FirstAll
 * *************************/
std::set<Terminal> FirstAll(const Grammar& gr, const GrammarSymbolVector& word){

  //set to return
  std::set<Terminal> rSet;
  
  //get nonterminals contained in the word
  std::set<Nonterminal> filterSet = Filter(gr.Nonterminals(), word);
  
  //if no nonterminals were found, call First
  if(filterSet.empty()){
    rSet = First(gr, word);
  }
  //if nonterminals were found
  else{
    
    //create temporary grammar with the first nonterminal as a start symbol
    Grammar tempGr = gr;
    NonterminalPtr firstNt = Filter1(gr.Nonterminals(), word);
    tempGr.SetStartSymbol(*firstNt);
    //remove nonproducte productions from this grammar
    Grammar optGr = Rnpp(tempGr);
    //if the grammar is not empty, call FirstRed
    if(!optGr.GrammarProductions().empty()){
      rSet = FirstRed(optGr, word);
    }
  }
  
  return rSet;
}

/* *************************
 * FirstLeq1
 * *************************/
std::set<Terminal> FirstLeq1(const Grammar& gr, uint k, const GrammarSymbolVector& word){

  //set to return
  std::set<Terminal> rSet;
  
  //if k > 1, return empty set
  if(k > 1){
    return rSet;
  }
  
  //else determine first set
  rSet = FirstAll(gr, word);
  
  //if k == 0 and the first set is not empty, return lambda as first set
  if(k == 0 && !rSet.empty()){
    Terminal tlambda(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
    
    rSet.clear();
    rSet.insert(tlambda);
  }
    
  return rSet;
}

/* *****************
 * WriteMap
 * *****************/
void WriteMap(GrammarSymbolWordMap f, bool changed){

  GrammarSymbolWordMap::const_iterator fit;
  std::set<Terminal>::const_iterator setit;
  GrammarSymbolVector::const_iterator wordit;
  std::stringstream mapstr;
  mapstr << "************************************************" << std::endl;
  mapstr << "changes made:";
  if(changed) mapstr << "yes" << std::endl;
  else mapstr << "no" << std::endl;
  mapstr << std::endl;
  for(fit = f.begin(); fit != f.end(); fit++){
    
    for(wordit = fit->first.begin(); wordit != fit->first.end(); wordit++){
      mapstr << (*wordit)->Str();
    }
    
    mapstr << ": ";
    mapstr << std::endl << "    ";
    for(setit = fit->second.begin(); setit != fit->second.end(); setit++){
      if(setit != fit->second.begin()){
        mapstr << ", ";
      }
        mapstr << (*wordit)->Str();
    }
    mapstr << std::endl;
  }
  
  std::cout << mapstr.str() << std::endl;
}
  
} // namespace faudes

