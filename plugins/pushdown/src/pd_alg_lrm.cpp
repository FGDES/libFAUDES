/** @file pd_alg_lrm.cpp  functions related to LR machines*/


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_alg_lrm.h"

namespace faudes {

/* *************************
 * Desc11
 * *************************/
std::set<Lr1Configuration> Desc11(const Grammar& gr, uint k, const Lr1Configuration& config){

  //config set to return
  std::set<Lr1Configuration> rSet;
  
  //always contains the original config
  rSet.insert(config);  
  
  //check the configuration (A -> B beta . x, y)
  //if the first symbol B of the afterDot vector is a nonterminal
  if(config.AfterDot().empty()){
    return rSet;
  }
  NonterminalPtr nt = std::dynamic_pointer_cast<Nonterminal>(config.AfterDot().front());
  if(nt){
    
    //look for productions (B -> w) and insert a new configuration
    //(B -> lambda . w , firstleq1(gr, k, beta y)) for each production and lookahead
    std::set<GrammarProduction>::const_iterator gpit;
    for(gpit = gr.GrammarProductionsBegin(); gpit != gr.GrammarProductionsEnd(); gpit++){
      if(gpit->Lhs() == *nt){
        
        
        //beforeDot for the new configuration is lambda
        GrammarSymbolVector beforeDot;
        Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
        GrammarSymbolPtr ptrtlambda(tlambda);
        beforeDot.push_back(ptrtlambda);
        
        
        //lookahead for the new configuration is calculated from the word beta y
        GrammarSymbolVector word(config.AfterDot().begin() + 1, config.AfterDot().end());
        Terminal* lookahead = new Terminal(config.Lookahead());
        GrammarSymbolPtr ptrlookahead(lookahead);
        word.push_back(ptrlookahead);
        
        std::set<Terminal> firstSet = FirstLeq1(gr, k , word);
        std::set<Terminal>::const_iterator terminalit;
        //insert new production for each possible lookahead
        for(terminalit = firstSet.begin(); terminalit != firstSet.end(); terminalit++){
          Lr1Configuration config(*nt, beforeDot, gpit->Rhs(), *terminalit);
          rSet.insert(config);
        }
      }
    }
  }
  return rSet;
}

/* *************************
 * Desc1
 * *************************/
std::set<Lr1Configuration> Desc1(const Grammar& gr, uint k, const std::set<Lr1Configuration>& configs){

  //config set to return
  std::set<Lr1Configuration> rSet;
  
  std::set<Lr1Configuration>::const_iterator configit;
  //iterate over all configurations
  for(configit = configs.begin(); configit != configs.end(); configit++){
    
    //get descendants of current configuration and add them to the return set
    std::set<Lr1Configuration> descSet = Desc11(gr, k, *configit);
    rSet.insert(descSet.begin(), descSet.end());    
  }
  
  return rSet;
}

/* *************************
 * Desc
 * *************************/
std::set<Lr1Configuration> Desc(const Grammar& gr, uint k, const std::set<Lr1Configuration>& configs){

  //config set to return
  std::set<Lr1Configuration> rSet;
  
  //get immediate descendants
  rSet = Desc1(gr, k, configs);
  
  //keep calling recursively if there were changes made
  //((configs.size == rSet.size &&  configs != rSet) || configs.size != rSet.size)
  if((rSet.size() == configs.size() && !std::equal(configs.begin(), configs.end(), rSet.begin())) || rSet.size() != configs.size()){
    
    //recursive call
    rSet = Desc(gr, k, rSet);
  }
  
  return rSet;
}

/* *************************
 * PassesX
 * *************************/
std::set<Lr1Configuration> PassesX(const Lr1Configuration& config, const GrammarSymbolPtr& symbol){

  //config set to return
  std::set<Lr1Configuration> rSet;
  
  //can only shift if there is at least one symbol remaining after the dot
  if(!config.AfterDot().empty()){
    
    //check if it is the right symbol
    if(*symbol == *config.AfterDot().front()){
      
      Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
      GrammarSymbolPtr ptrtlambda(tlambda);
      
      //create new configuration 
      //(A -> a . X b, y) becomes (A -> a X . b, y)
      
      //construct beforeDot
      GrammarSymbolVector beforeDot = config.BeforeDot();
      //if beforeDot contains only lambda, clear it before adding X
      if(*beforeDot.front() == *ptrtlambda){
        beforeDot.clear();
      }
      beforeDot.push_back(config.AfterDot().front());
      
      //construct afterDot 
      GrammarSymbolVector afterDot(config.AfterDot().begin() + 1, config.AfterDot().end());
      
      //if afterDot is empty, insert lambda
      if(afterDot.empty()){
        
        afterDot.push_back(ptrtlambda);
      }
      
      
      Lr1Configuration shiftedConfig(config.Lhs(), beforeDot, afterDot, config.Lookahead());
      
      //insert new configuration into set
      rSet.insert(shiftedConfig);
    }    
  }
  return rSet;  
}

/* *************************
 * Basis
 * *************************/
std::set<Lr1Configuration> Basis(const std::set<Lr1Configuration> configs, const GrammarSymbolPtr& symbol){

  //config set to return
  std::set<Lr1Configuration> rSet;
  
  std::set<Lr1Configuration> shiftSet;
  std::set<Lr1Configuration>::const_iterator configit;
  //iterate over all configurations
  for(configit = configs.begin(); configit != configs.end(); configit++){
    
    //try to shift the dot
    shiftSet = PassesX(*configit,symbol);
    
    //if successful, insert shifted configuration into result set
    if(!shiftSet.empty()){
      rSet.insert(*shiftSet.begin());
    }
  }
  return rSet;
}

/* *************************
 * GoTo
 * *************************/
std::set<Lr1Configuration> GoTo(const Grammar& gr, uint k, const std::set<Lr1Configuration> configs, const GrammarSymbolPtr& symbol){

  //obtain shifted configurations
  std::set<Lr1Configuration> shiftSet = Basis(configs, symbol);
  
  //return the shifted configuration's descendants
  return Desc(gr, k, shiftSet);
}

/* *************************
 * Lrm1
 * *************************/
LrmTransitionMap Lrm1(const Grammar& gr, uint k, const Lr1ConfigurationSetSet& configSetSet){

  //config set map to return
  LrmTransitionMap rTransitions;
  
  Lr1ConfigurationSetSet::const_iterator configssit;
  Terminal lambda(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  //iterate over all configuration sets
  for(configssit = configSetSet.begin(); configssit != configSetSet.end(); configssit++){
    
    std::set<Nonterminal>::const_iterator ntit;
    std::set<Terminal>::const_iterator tit;
    //determine the possible next configuration set for every terminal
    for(tit = gr.TerminalsBegin(); tit != gr.TerminalsEnd(); tit++){
      
      //dont consider lambda!
      if(*tit == lambda){
        continue;
      }
      
      Terminal* t = new Terminal(*tit);
      GrammarSymbolPtr tptr(t);
      //get next configuration set
      std::set<Lr1Configuration> nextConfigSet = GoTo(gr, k, *configssit, tptr);
      
      //if the config set is not empty
      if(!nextConfigSet.empty()){
        
        //insert it into transition map
        ConfigSetGsPair configSetGsPair = std::make_pair(*configssit, tptr);
        rTransitions.insert(std::make_pair(configSetGsPair, nextConfigSet));
      }
    }
    
    //determine the possible next configuration set for every nonterminal
    for(ntit = gr.NonterminalsBegin(); ntit != gr.NonterminalsEnd(); ntit++){
      
      Nonterminal* nt = new Nonterminal(*ntit);
      GrammarSymbolPtr ntptr(nt);
      //get next configuration set
      std::set<Lr1Configuration> nextConfigSet = GoTo(gr, k, *configssit, ntptr);
      
      //if the config set is not empty
      if(!nextConfigSet.empty()){
        
        //insert it into transition map
        ConfigSetGsPair configSetGsPair = std::make_pair(*configssit, ntptr);
        rTransitions.insert(std::make_pair(configSetGsPair, nextConfigSet));
      }
    }
  }
  return rTransitions;
}

/* *************************
 * LrmLoop
 * *************************/
std::pair<LrmTransitionMap,Lr1ConfigurationSetSet> LrmLoop(const Grammar& gr, uint k, const LrmTransitionMap& transitions, const Lr1ConfigurationSetSet& states, Lr1ConfigurationSetSet examineStates){

  //save states that are already found
  Lr1ConfigurationSetSet foundStates(states);
  foundStates.insert(examineStates.begin(), examineStates.end());
  
  //pair to return if no new transition is found
  std::pair<LrmTransitionMap,Lr1ConfigurationSetSet> rPair;
  rPair = std::make_pair(transitions, foundStates);
  
  //calculate new transitions
  LrmTransitionMap newTransitions = Lrm1(gr, k, examineStates);
  
  //if new transitions were found
  if(!newTransitions.empty()){
    
    //determine states that need to be looked at in the next recursive iteration
    //(i. e., the successor states of the transitions)
    Lr1ConfigurationSetSet newExamineStates;
    LrmTransitionMap::const_iterator transit;
    for(transit = newTransitions.begin(); transit != newTransitions.end(); transit++){
      newExamineStates.insert(transit->second);
    }
    
    Lr1ConfigurationSetSet::const_iterator stateit;
    //remove states that were already visited
    for(stateit = foundStates.begin(); stateit != foundStates.end(); stateit++){
      newExamineStates.erase(*stateit);
    }
    
    //recursively look for new transitions and states
    newTransitions.insert(transitions.begin(), transitions.end());
    rPair = LrmLoop(gr, k , newTransitions, foundStates, newExamineStates); 
  }
  return rPair;
}

/* *************************
 * DescInitial
 * *************************/
std::set<Lr1Configuration> DescInitial(const Grammar& gr){

  //item set to return
  std::set<Lr1Configuration> rSet;
  
  std::set<GrammarProduction>::const_iterator gpit;
  //look for initial grammar productions (S,w)
  for(gpit = gr.GrammarProductionsBegin(); gpit != gr.GrammarProductionsEnd(); gpit++){
    if(gpit->Lhs() == gr.StartSymbol()){
      
      //insert configuration (S -> lambda . w, lambda)
      Terminal* tlambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
      GrammarSymbolPtr ptrtlambda(tlambda);
      
      GrammarSymbolVector beforeDot;
      beforeDot.push_back(ptrtlambda);
      
      rSet.insert(Lr1Configuration(gpit->Lhs(), beforeDot, gpit->Rhs(), *tlambda));
    }
  }
  return rSet;
}

/* *************************
 * ValidEmpty
 * *************************/
std::set<Lr1Configuration> ValidEmpty(const Grammar& gr, uint k){

  //item set to return
  std::set<Lr1Configuration> rSet;
  
  //get initial configurations
  rSet = DescInitial(gr);
  
  //get descendants of the initial configurations
  rSet = Desc(gr, k, rSet);
  
  return rSet;
}

/* *************************
 * Lrm
 * *************************/
GotoGenerator Lrm(const Grammar& gr, uint k){

  //generator to return
  GotoGenerator gotoGen;
  
  //get the initial state
  std::set<Lr1Configuration> initState = ValidEmpty(gr, k);
  
  //get the transitions and states
  Lr1ConfigurationSetSet initStateSet;
  initStateSet.insert(initState);
  std::pair<LrmTransitionMap,Lr1ConfigurationSetSet> transitionsStates= LrmLoop(gr, k, LrmTransitionMap(), Lr1ConfigurationSetSet(), initStateSet);
  
  Lr1ConfigurationSetSet::const_iterator stateit;
  //insert states of the generator
  for(stateit = transitionsStates.second.begin(); stateit != transitionsStates.second.end(); stateit++){
    Idx idx = gotoGen.InsState();
    gotoGen.ConfigSet(idx, *stateit);
    
    //if the current config equals the init config, set init state
    if(!(CompareConfigSet(*stateit, initState) || CompareConfigSet(initState, *stateit))){
      gotoGen.SetInitState(idx);
    }
  }
  
  //insert transitions into the generator
  LrmTransitionMap::const_iterator transit;
  for(transit = transitionsStates.first.begin(); transit != transitionsStates.first.end(); transit++){ 
    Idx startState = gotoGen.StateIndex(transit->first.first);
    Idx endState = gotoGen.StateIndex(transit->second);
    GrammarSymbolPtr symbol = transit->first.second;
    
    gotoGen.SetTransition(startState, symbol, endState);
  }
  
  //save transitions and states the a gotoMachine
//   gotoMachine.InitState(initState);
//   gotoMachine.Transitions(transitionsStates.first);
//   gotoMachine.States(transitionsStates.second);

  gotoGen.SWrite();
  
  return gotoGen;
}

/* *************************
 * Aug
 * *************************/
Grammar Aug(const Grammar& gr, const Nonterminal& startSymbol, const Terminal& augSymbol){

  //grammar to return
  Grammar rGr = gr;
  
  //try to set startSymbol S as the new start symbol and throw an exception
  //if it is already contained in the grammar's nonterminals
  if(!rGr.SetStartSymbol(startSymbol)){
    std::stringstream errstr;
    errstr << "While executing Aug(): Tried to insert new startSymbol " << startSymbol.Str() << ", but it already existed in the grammar" << std::endl;
    throw Exception("Aug", errstr.str(), 1001);
  }
  
  //try to insert augSymbol $ into the grammar's terminals and throw an exception if
  //it is already contained in the grammar's terminals
  if(!rGr.InsTerminal(augSymbol)){
    std::stringstream errstr;
    errstr << "While executing Aug(): Tried to insert new augSymbol " << augSymbol.Str() << ", but it already existed in the grammar" << std::endl;
    throw Exception("Aug", errstr.str(), 1001);
  }
  
  //get old startSymbol S'
  Nonterminal* oldStartSymbol = new Nonterminal(gr.StartSymbol());
  GrammarSymbolPtr oldStartSymbolPtr(oldStartSymbol);
  
  //get Pointer to augSymbol
  Terminal* augSymbolTemp = new Terminal(augSymbol);
  GrammarSymbolPtr augSymbolPtr(augSymbolTemp);
  
  //construct new production's right-hand side $ S' $
  GrammarSymbolVector rhs;
  rhs.push_back(augSymbolPtr);
  rhs.push_back(oldStartSymbolPtr);
  rhs.push_back(augSymbolPtr);
  
  //construct new Production S -> $ S' $ and insert into grammar
  GrammarProduction gp(startSymbol,rhs);
  rGr.InsGrammarProduction(gp);
    
  return rGr;
}

  
} // namespace faudes

