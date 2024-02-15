/** @file pd_alg_lrp.cpp  functions related to parsers*/


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013/14  Stefan Jacobi, Ramon Barakat, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_alg_lrp.h"

namespace faudes {

  
/* *************************
 * MachineGoto
 * *************************/
std::set<Idx> GeneratorGoto(const GotoGenerator& gotoGen, Idx state, const GrammarSymbolPtr& symbol){

  //state set to return
  std::set<Idx> successorStates;
  
  TransSet::Iterator transit;
  //iterate over all states with state as a start state
  for(transit = gotoGen.TransRelBegin(state); transit != gotoGen.TransRelEnd(state); transit++){
    
    //if the symbol is the same, insert end state of the transition as successor state
    if(*gotoGen.Symbol(*transit) == *symbol){
      successorStates.insert(transit->X2);
    }
  }
  return successorStates;
}
  
  
/* *************************
 * GeneratorGotoSeq
 * *************************/
std::vector<Idx> GeneratorGotoSeq(const GotoGenerator& gotoGen, Idx startState, const GrammarSymbolVector& word){

  //state sequence to return
  std::vector<Idx> rStateSequence;
  
  //for later use
  Terminal* lambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr lambdaPtr(lambda);
  

  //if the word is not empty (or does not contain lambda), it is of the form w = a.w'
  if(!word.empty()){
    if(*word.front() != *lambda){
      
      //get front symbol a and remaining word w'
      GrammarSymbolPtr frontSymbol = word.front();
      GrammarSymbolVector remainingWord(word.begin() + 1, word.end());
      
      //get next state in the sequence
      std::set<Idx> nextStateSet = GeneratorGoto(gotoGen, startState, frontSymbol);
      Idx nextState;
      if(!nextStateSet.empty()){
        nextState = *nextStateSet.begin();
        
        //append the next state in the sequence
        rStateSequence.push_back(nextState);

      /* } */ // use of uninitialised nextState must be a bug ...
      
      //get the rest of the sequence
      std::vector<Idx> remainingSequence = GeneratorGotoSeq(gotoGen, nextState, remainingWord);
      
      //append the rest of the sequence
      rStateSequence.insert(rStateSequence.end(), remainingSequence.begin(), remainingSequence.end());

      } // ... however, not quite sure with the fix. (tmoor 2016-08)
    } 
  }
  return rStateSequence;
}


  
/* *************************
 * LrpShiftRules
 * *************************/
std::set<Lr1ParserAction> LrpShiftRules(const Grammar& gr, const Grammar& augGr, const GotoGenerator& gotoGen, uint k){

  //parser actions to return
  std::set<Lr1ParserAction> rActions;
  
  //lambda terminal for later use
  Terminal* lambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr lambdaPtr(lambda);
  
  StateSet::Iterator stateit;
  //iterate over all states q of the gotoGenerator
  for(stateit = gotoGen.StatesBegin(); stateit != gotoGen.StatesEnd(); stateit++){
    
    std::set<Terminal>::const_iterator tit;
    //iterate over all terminals a of the grammar
    for(tit = gr.TerminalsBegin(); tit != gr.TerminalsEnd(); tit++){
      
      //do not care for lambda
      if(tit->IsLambda()){
        continue;
      }
      
      //convert terminal into GrammarSymbolPtr
      Terminal* a = new Terminal(*tit);
      GrammarSymbolPtr aPtr(a);
      
      //get the successor state q' for this terminal
      std::set<Idx> succStateSet = GeneratorGoto(gotoGen, *stateit, aPtr);
      
      //continue, if there is no successor state (there can't be too many since gotoGen 
      //is supposed to be deterministic)
      if(succStateSet.size() != 1){
        continue;
      }
      
      //extract the successor state
      Idx succState = *succStateSet.begin();
      
      std::set<Lr1Configuration>::const_iterator configit;
      //iterate over all configurations contained in the successor state
      for(configit = gotoGen.ConfigSet(*stateit).begin(); configit != gotoGen.ConfigSet(*stateit).end(); configit++){
        
        //only consider this configuration (A -> w1 . a w2, z) if the
        //production (A -> w1 a w2) is in the grammar.
        
        GrammarSymbolVector beforeDot, afterDot;
        //construct right-hand side of the production, but need to make sure to delete
        //unnecessary lambdas
        beforeDot = configit->BeforeDot();
        if(**beforeDot.begin() == *lambda){
          beforeDot.clear();
        }
        afterDot = configit->AfterDot();
        if(**afterDot.begin() == *lambda){
          afterDot.clear();
        }
        
        //create w1 a w2 from afterDot and beforeDot
        GrammarSymbolVector rhs(beforeDot);
        rhs.insert(rhs.end(), afterDot.begin(), afterDot.end());
        if(rhs.empty()){
          rhs.push_back(lambdaPtr);
        }
        
        //construct production
        GrammarProduction gp(configit->Lhs(), rhs);
        
        //test if the configuration is not in the grammar and if so, continue
        if(gr.GrammarProductions().find(gp) == gr.GrammarProductionsEnd()){
          continue;
        }
        
        //construct the word w2 z
        //w2 can't be lambda
        GrammarSymbolVector w2z(configit->AfterDot().begin() + 1, configit->AfterDot().end());
        
        //append z if it's not lambda
        if(!configit->Lookahead().IsLambda()){
          Terminal* lookahead = new Terminal(configit->Lookahead());
          GrammarSymbolPtr lookaheadPtr(lookahead);
          w2z.push_back(lookaheadPtr);
        }
        
        //get first terminals after w2z (will be either nothing or lambda)
        std::set<Terminal> firstTerminals = FirstLeq1(augGr, k - 1, w2z);
        
        std::set<Terminal>::const_iterator firsttit;
        //iterate over first terminals and add a parser action for each terminal y
        for(firsttit = firstTerminals.begin(); firsttit != firstTerminals.end(); firsttit++){
          
          //just to make sure that the terminal must be lambda
          if(!firsttit->IsLambda()){
            std::stringstream errstr;
            errstr << "While executing LrpRules(): Constructing shift actions and the terminal " << firsttit->Str() << ", which was expected to be lambda" << std::endl;
            throw Exception("LrpRules", errstr.str(), 1001);
          }
          
          //construct left-hand side of the rule (q | a y) (which is (q | a),
          //because y is lambda)
          std::vector<Idx> stateStack;
          stateStack.push_back(*stateit);
          Lr1ParserActionElement actionLhs(stateStack, *a);
          
          //construct right-hand side of the rule (q q' | y)
          stateStack.push_back(succState);
          Lr1ParserActionElement actionRhs(stateStack, *lambda);
          
          //construct shift action (q | a) -> (q q' | lambda) and insert into 
          //set of actions
          Lr1ParserAction action(actionLhs, actionRhs);
          rActions.insert(action);
        }
      }
    }
  }
  return rActions;
}

/* *************************
 * LrpReduceRules
 * *************************/
std::set<Lr1ParserAction> LrpReduceRules(const Grammar& gr, const Grammar& augGr, const GotoGenerator& gotoGen, uint k){

  //parser actions to return
  std::set<Lr1ParserAction> rActions;
  
  //lambda terminal for later use
  Terminal* lambda = new Terminal(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));
  
  StateSet::Iterator stateit;
  //iterate over all states q of the gotoGenerator
  for(stateit = gotoGen.StatesBegin(); stateit != gotoGen.StatesEnd(); stateit++){
    
    std::set<Lr1Configuration>::const_iterator configit;
    //iterate over all configurations  contained in the state q
    for(configit = gotoGen.ConfigSet(*stateit).begin(); configit != gotoGen.ConfigSet(*stateit).end(); configit++){
      
      //only consider this configuration if it is of the form (A -> lambda . w, z),
      //i. e. if beforeDot is lambda      
      if(**configit->BeforeDot().begin() != *lambda){
        continue;
      }
      
      //only consider this configuration if the production (A -> w) is in the grammar
      GrammarProduction gp(configit->Lhs(), configit->AfterDot());
      if(gr.GrammarProductions().find(gp) == gr.GrammarProductionsEnd()){
        continue;
      }
      
      //convert A into GrammarSymbolPtr and get successor state qa of q with the symbol A
      Nonterminal* a = new Nonterminal(configit->Lhs());
      GrammarSymbolPtr aPtr(a);
      std::set<Idx> succStateSet = GeneratorGoto(gotoGen, *stateit, aPtr);
      
      //if there is no successor state, continue
      if(succStateSet.size() != 1){
        continue;
      }
      Idx succState = *succStateSet.begin();
      
      //get successor state sequence qs of q with the word w
      std::vector<Idx> succStateSequence = GeneratorGotoSeq(gotoGen, *stateit, configit->AfterDot());
      
      //join state q and state sequence qs
      std::vector<Idx> qQs;
      qQs.push_back(*stateit);
      qQs.insert(qQs.end(), succStateSequence.begin(), succStateSequence.end());
      
      //get last state of qQs state sequence
      Idx lastState = qQs.back();
      
      std::set<Lr1Configuration>::const_iterator lsconfigit;
      //iterate over configurations in the last state
      for(lsconfigit = gotoGen.ConfigSet(lastState).begin(); lsconfigit != gotoGen.ConfigSet(lastState).end(); lsconfigit++){
        
        //only consider this configuration if it is of the form (A -> w . lambda, y),
        if(lsconfigit->Lhs() != configit->Lhs()){
          continue;
        }
        if(!EqualsGsVector(lsconfigit->BeforeDot(), configit->AfterDot())){
          continue;
        }
        if(**lsconfigit->AfterDot().begin() != *lambda){
          continue;
        }
        
        //get the lookahead terminal
        Terminal lookahead(lsconfigit->Lookahead());
        
        //add parser action for each configuration
        //construct left-hand side (q qs | y)
        Lr1ParserActionElement lhs(qQs, lookahead);
        
        //construct right-hand side (q qa | y)
        std::vector<Idx> stateStack;
        stateStack.push_back(*stateit);
        stateStack.push_back(succState);
        Lr1ParserActionElement rhs(stateStack, lookahead);
        
        //construct reduce action (q qs | y) -> (q qa | y) and insert it into set
        //of actions
        Lr1ParserAction action(lhs, rhs, gp);
        rActions.insert(action);
      }
    }
  }
  return rActions;
}

/* *************************
 * LrpRules
 * *************************/
std::set<Lr1ParserAction> LrpRules(const Grammar& gr, const Grammar& augGr, const GotoGenerator& gotoGen, uint k){

  //parser actions to return
  std::set<Lr1ParserAction> rActions;
  
  //get shift actions
  rActions = LrpShiftRules(gr, augGr, gotoGen, k);
  
  //get reduce actions
  std::set<Lr1ParserAction> reduceActions = LrpReduceRules(gr, augGr, gotoGen, k);
  rActions.insert(reduceActions.begin(), reduceActions.end());

  return rActions;
}

/* *************************
 * Lrp
 * *************************/
Lr1Parser Lrp(const Grammar& gr, const Grammar& augGr, const GotoGenerator& gotoGen, uint k, const Terminal& augSymbol){

  //parser to return
  Lr1Parser rParser;
  
  //convert augSymbol $ into GrammarSymbolPtr
  Terminal* augSymbolTemp = new Terminal(augSymbol);
  GrammarSymbolPtr augSymbolPtr(augSymbolTemp);
  
  //convert the grammar's start symbol q0 into GrammarSymbolPtr
  Nonterminal* q0 = new Nonterminal(gr.StartSymbol());
  GrammarSymbolPtr q0Ptr(q0);
  
  //get successor state of the initial goto generator state and $
  std::set<Idx> succStateSetAug = GeneratorGoto(gotoGen, gotoGen.InitState(), augSymbolPtr);
  
  //get successor state sequence of the initial goto generator state and $ q0
  GrammarSymbolVector word;
  word.push_back(augSymbolPtr);
  word.push_back(q0Ptr);
  std::vector<Idx> succStateSeqAugQ0 = GeneratorGotoSeq(gotoGen, gotoGen.InitState(), word);
  
  //get successor state sequence of the initial goto generator state and $ q0 $
  word.push_back(augSymbolPtr);
  std::vector<Idx> succStateSeqAugQ0Aug = GeneratorGotoSeq(gotoGen, gotoGen.InitState(), word);
  
  //if the successor states for $, $ q0 and $ q0 $ exist (i. e. they are not empty)
  if(!succStateSetAug.empty() && !succStateSeqAugQ0.empty() && !succStateSeqAugQ0Aug.empty()){
    
    //get the successor state for $
    Idx succStateAug = *succStateSetAug.begin();
    
    //get the last state of the $ q0 sequence
    Idx lastStateAugQ0 = succStateSeqAugQ0.back();
    
    //get the last state of the $ q0 $ sequence
    Idx lastStateAugQ0Aug = succStateSeqAugQ0Aug.back();
    
    StateSet::Iterator stateit;
    //insert all states of the gotoGen as nonterminals of the parser, but exclude
    //the start state and the end state
    for(stateit = gotoGen.StatesBegin(); stateit != gotoGen.StatesEnd(); stateit++){
      if(*stateit != gotoGen.InitState() && *stateit != lastStateAugQ0Aug){
        rParser.InsNonterminal(*stateit);
      }
    }
    
    std::set<Terminal>::const_iterator tit;
    //insert all the augmented grammar's terminals as the parser's terminals
    for(tit = augGr.TerminalsBegin(); tit != augGr.TerminalsEnd(); tit++){
      rParser.InsTerminal(*tit);
    }
    
    //set the parser's start state
    rParser.SetStartState(succStateAug);
    
    //set the parser's final state
    rParser.InsFinalState(lastStateAugQ0);
    
    //get the parsing actions
    std::set<Lr1ParserAction> actions = LrpRules(gr, augGr, gotoGen, k);
    
    std::set<Lr1ParserAction>::const_iterator actit;
    //insert the parsing actions into the parser
    for(actit = actions.begin(); actit != actions.end(); actit++){
      rParser.InsAction(*actit);
    }
    
    //save $ for later reference in the parser
    rParser.SetAugSymbol(augSymbol);
  }
  
  return rParser;
}


//************************
// detach augment symbol
//************************
void DetachAugSymbol(Lr1Parser& parser) {
	std::set<Lr1ParserAction> tmp;
	std::set<Lr1ParserAction>::iterator it;

	//set each nonterminal before augment symbol as final state
	//and remove those actions
	parser.ClrFinalStates();
	for (it = parser.Actions().begin(); it != parser.Actions().end(); ++it) {
		if (it->Rhs().NextTerminal() != parser.AugSymbol()) {
			tmp.insert(*it);
		} else {
			parser.InsFinalState(it->Lhs().StateStack().back());
		}
	}

	parser.mActions = tmp;
}


/*
 * Get state stack by creating or get new nonterms.
 * Each state from given state stack(except last one) will be converted to a pair of state an lambda.
 * The last state will be converted to a pair of state and given terminal.
 * ( e.g.  given state stack [s1,s2,s3] and terminal t will get
 *  [z1,z2,z3] with z1 = (s1,lambda), z2 = (s2,lambda) and z3 =(s3,t)  )
 *
 * @param rVec
 * 		reference to statestack
 * @param term
 * 		terminal symbol
 * @param indexMap
 * 		map to remember new indices of new nonterminal representing (nonterminal, terminal) pairs.
 * 		New nonterminals will add to this map.
 *
 * @result
 * 		statestack with indices of the new nonterminals
 */
std::vector<Idx> CreateNonterms(const std::vector<Idx>& rVec,Terminal term, std::map<std::pair<Idx,Terminal>,Idx >& indexMap){

	// result vector
	std::vector<Idx> res;

	//lambda terminal
	Terminal lambda(PushdownGenerator::GlobalEventSymbolTablep()->Index(FAUDES_PD_LAMBDA));

	std::map<std::pair<Idx,Terminal>,Idx >::iterator mapit;
	std::vector<Idx>::const_iterator it;


	//find or create(q,lambda)
	for(it = rVec.begin(); it != rVec.end(); ++it ){
		std::pair<Idx,Terminal> p = std::make_pair(*it,lambda);


		if(std::distance(it,rVec.end()) == 1)
			p = std::make_pair(*it,term);

		mapit = indexMap.find(p);

		// if p already exists, get index of p
		if(mapit != indexMap.end()){
			res.push_back(mapit->second);
		}else{
			//if p doesn't exists, get new index
			Idx i = indexMap.size()+1;
			indexMap.insert(std::make_pair(p,i));
			res.push_back(i);
		}
	}

	return res;

}

// Transform following actions of given parser
// (q,a,p,a) 		=> ((q,lambda),a,(p,a),lambda) 		, ((q,a),lambda,(p,a),lambda)
// (q,a,p,lambda) 	=> ((q,lambda),a,(p,lambda),lambda)	, ((q,a),lambda,(p,lambda),lambda)
Lr1Parser TransformParserAction(const Lr1Parser& parser) {
	Lr1Parser rParser;

	// map to remember new indices of new nonterminal representing (nonterminal, terminal) pairs
	std::map<std::pair<Idx,Terminal>,Idx > indexMap;
	std::map<std::pair<Idx,Terminal>,Idx >::iterator mapit;
	std::set<Idx> nonterms;
	std::set<Lr1ParserAction>::iterator actionsit;

	//lambda terminal
	Terminal lambda(PushdownGenerator::GlobalEventSymbolTablep()->Index(
	FAUDES_PD_LAMBDA));

	//set start state
	std::pair<Idx,Terminal> start = std::make_pair(parser.StartState(),lambda);
	indexMap[start] = 1;
	rParser.SetStartState(1);

	//for each action
	for (actionsit = parser.Actions().begin(); actionsit != parser.Actions().end();
			++actionsit) {
		Lr1ParserAction action = *actionsit;

		//(q,a,p,a)
		if (action.Lhs().NextTerminal() == action.Rhs().NextTerminal()) {
			//get Terminal a
			Terminal terminal = action.Lhs().NextTerminal();

			//lhs = (q,lambda),a
			Lr1ParserActionElement lhs1(
					CreateNonterms(action.Lhs().StateStack(), lambda, indexMap),terminal);

			//rhs = (p,a),lambda
			Lr1ParserActionElement rhs1(
					CreateNonterms(action.Rhs().StateStack(), terminal,indexMap), lambda);

			//(q,a,p,a) => ((q,lambda),a,(p,a),lambda)
			rParser.InsAction(Lr1ParserAction(lhs1, rhs1));


			//lhs = (q,a),lambda
			Lr1ParserActionElement lhs2(
					CreateNonterms(action.Lhs().StateStack(), terminal, indexMap), lambda);

			//rhs = (p,a),lambda
			Lr1ParserActionElement rhs2 = rhs1;

			//(q,a,p,a) => ((q,a),lambda,(p,a),lambda)
			rParser.InsAction(Lr1ParserAction(lhs2, rhs2));

		}
		//(q,a,p,lambda)
		else if (action.Rhs().NextTerminal().IsLambda()) {
			Terminal terminal = action.Lhs().NextTerminal();

			//lhs = (q,lambda),a
			Lr1ParserActionElement lhs1(
					CreateNonterms(action.Lhs().StateStack(), lambda, indexMap), terminal);
			//rhs = (p,lambda),lambda
			Lr1ParserActionElement rhs1(
					CreateNonterms(action.Rhs().StateStack(), lambda, indexMap), lambda);

			//(q,a,p,lambda) => ((q,lambda),a,(p,lambda),lambda)
			rParser.InsAction(Lr1ParserAction(lhs1, rhs1));

			//lhs = (q,a),lambda
			Lr1ParserActionElement lhs2(
					CreateNonterms(action.Lhs().StateStack(), terminal, indexMap), lambda);
			//rhs = (p,lambda),lambda
			Lr1ParserActionElement rhs2 = rhs1;

			//(q,a,p,lambda) => ((q,a),lambda,(p,lambda),lambda)
			rParser.InsAction(Lr1ParserAction(lhs2, rhs2));
		}
	}

	//set final states (p,lambda)
	std::set<Idx>::iterator fit;
	for (fit = parser.FinalStates().begin(); fit != parser.FinalStates().end(); fit++) {
		mapit = indexMap.find(std::make_pair(*fit,lambda));
		if(mapit != indexMap.end())
			rParser.InsFinalState(mapit->second);
	}

	//get all nonterminals = values of indexMap
	for(mapit = indexMap.begin(); mapit != indexMap.end(); ++mapit){
		nonterms.insert(mapit->second);
	}

	//set nonterminals, terminals and augment symbol
	rParser.mNonterminals = nonterms;
	rParser.mTerminals = parser.mTerminals;
	rParser.mAugSymbol = parser.mAugSymbol;

	return rParser;
}


/* *************************
 * LrParser2EPDA
 * *************************/
PushdownGenerator LrParser2EPDA(const Lr1Parser& parser) {
	FUNCTION(__FUNCTION__);	//scop log
	bool debug = false;		//debug?

	std::string statepre = "q";
	std::string stacksympre = "x";

	//generator to return
	PushdownGenerator rPd;

	std::set<Idx>::iterator it;
	std::set<Terminal>::iterator termit;
	std::set<Lr1ParserAction>::iterator actionit;

	// the parser's nonterminals are the generator's ...
	for (it = parser.Nonterminals().begin(); it != parser.Nonterminals().end();
			++it) {
		// ... states
		rPd.InsState(*it);
		rPd.StateName(*it,statepre + ToStringInteger(*it));

		// ... and stack symbols
		rPd.InsStackSymbol(ToStringInteger(*it));
	}
	DEBUG_PRINTF(debug,"LrParser2EPDA: Set states and stack symbols","");
	FD_DF("LrParser2EPDA: Set states and stack symbols");

	//set init state
	rPd.SetInitState(parser.StartState());

	//set final states
	for (it = parser.FinalStates().begin(); it != parser.FinalStates().end(); ++it) {
		rPd.SetMarkedState(*it);
	}
	DEBUG_PRINTF(debug,"LrParser2EPDA: Set final states","");
	FD_DF("LrParser2EPDA: Set final states");

	//set events
	for (termit = parser.Terminals().begin(); termit != parser.Terminals().end(); ++termit){
		if ((*termit) != parser.AugSymbol())
			rPd.InsEvent(termit->Str());
	}
	DEBUG_PRINTF(debug,"LrParser2EPDA: Set events","");
	FD_DF("LrParser2EPDA: Set events");


	//set stack bottom
	std::string bottom = rPd.UniqueStackSymbolName("sb");
	rPd.InsStackSymbol(bottom);
	rPd.SetStackBottom(bottom);
	DEBUG_PRINTF(debug,"LrParser2EPDA: Set stack bottom: "+bottom,"");
	FD_DF("LrParser2EPDA: Set stack bottom: "+bottom);

	//lambda is always a stack symbol
	Idx lambda = rPd.InsStackSymbol(FAUDES_PD_LAMBDA);

	//create transitions for each action
	for (actionit = parser.Actions().begin(); actionit != parser.Actions().end(); ++actionit) {
		Lr1ParserAction ac = *actionit;
		Idx src = ac.Lhs().StateStack().back();
		Idx trg = ac.Rhs().StateStack().back();
		Idx ev = rPd.EventIndex(ac.Lhs().NextTerminal().Str());

		std::vector<Idx> pop = ac.Lhs().StateStack();
		pop.pop_back();

		std::vector<Idx> push = ac.Rhs().StateStack();
		push.pop_back();

		std::vector<std::string> popTrans(pop.size());
		std::vector<std::string> pushTrans(push.size());

		//need transformation because stack symbol idx
		//don't have to be the same as former added ones
		//but stack symbol names should be equal
		std::transform(pop.rbegin(), pop.rend(), popTrans.begin(), ToStringInteger);
		std::transform(push.rbegin(), push.rend(), pushTrans.begin(), ToStringInteger);

		if (popTrans.empty())
			popTrans.push_back(rPd.StackSymbolName(lambda));
		if (pushTrans.empty())
			pushTrans.push_back(rPd.StackSymbolName(lambda));

		rPd.SetTransition(src, ev, trg, popTrans, pushTrans);
	}

	DEBUG_PRINTF(debug,"LrParser2EPDA: Set transitions","");
	FD_DF("LrParser2EPDA: Set transitions");

	return rPd;
}

} // namespace faudes

