/** @file pd_alg_nb_sub_a.cpp Blockfree and Accessible subfunctions */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013/14 Ramon Barakat, Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_alg_sub.h"

namespace faudes {



/* *************************
 * AnnotatePdStates
 * *************************/
PushdownGenerator AnnotatePdStates(const std::string word, const PushdownGenerator& pd){

  //take a copy of the old generator
  PushdownGenerator rPd = PushdownGenerator(pd);

  //annotate states of the new generator
  StateSet::Iterator its;
  for(its = pd.StatesBegin(); its != pd.StatesEnd(); its++){
    MergeStateAnnotation msa = MergeStateAnnotation(*its,word);
    rPd.SetMerge(*its,msa);
  }
  return rPd;
}

/* *************************
 * RenameStackSymbold
 * *************************/
PushdownGenerator RenameStackSymbold(const std::string word, const PushdownGenerator& pd){

  //the annotation string
  std::string annotation = word + "-";

  //take a copy of the old generator
  PushdownGenerator rPd = PushdownGenerator(pd);

  //rename every stack symbol and delete the old one
  StackSymbolSet::Iterator it;
  for(it = pd.StackSymbolsBegin(); it != pd.StackSymbolsEnd(); it++){
    //don't rename lambda
    if(!pd.IsStackSymbolLambda(*it)){
      rPd.InsStackSymbol(annotation + pd.StackSymbolName(*it));
      rPd.DelStackSymbol(pd.StackSymbolName(*it));
    }
  }
  //rename stack bottom
  rPd.SetStackBottom(annotation + pd.StackSymbolName(pd.StackBottom()));

  //iterate over transitions and replace stack symbols in push and pop with
  //the renamed versions
  TransSet::Iterator tit;
  std::vector<Idx> oldPush, oldPop, push, pop;
  std::vector<Idx>::const_iterator pushit, popit;
  PopPushSet popPush;
  PopPushSet::const_iterator ppit;

  //take copy of transitions to iterate over
  //copying is necessary because transitions will get deleted
  //iterating over the original TransSet can lead to undefined behaviour
  TransSet transRel = rPd.TransRel();
  for(tit = transRel.Begin(); tit != transRel.End(); tit++){

    //iterate over pop/push pairs
    popPush = rPd.PopPush(*tit);
    for(ppit = popPush.begin(); ppit != popPush.end(); ppit++){

      //rename pop stack symbols
      oldPop = ppit->first;
      for(popit = oldPop.begin(); popit != oldPop.end(); popit++){
        //dont rename lambda
        if(pd.IsStackSymbolLambda(*popit)){
          pop.push_back(*popit);
        }
        else{
          pop.push_back(rPd.StackSymbolIndex(annotation + rPd.StackSymbolName(*popit)));
        }
      }

      //rename push stack symbols
      oldPush = ppit->second;
      for(pushit = oldPush.begin(); pushit != oldPush.end(); pushit++){
        //dont rename lambda
        if(pd.IsStackSymbolLambda(*pushit)){
          push.push_back(*pushit);
        }
        else{
          push.push_back(rPd.StackSymbolIndex(annotation + rPd.StackSymbolName(*pushit)));
        }
      }

      //create new transitions
      rPd.SetTransition(*tit, pop, push);
      pop.clear();
      push.clear();

      //and delete the old one
      rPd.ClrTransition(*tit, oldPop, oldPush);
    }
  }
  return rPd;
}

/* *************************
 * Nda
 * *************************/
PushdownGenerator Nda(const PushdownGenerator& pd){
  FUNCTION(__FUNCTION__);	//scope log
  FD_DF("Nda(" << &pd << ")");

  PushdownGenerator rPd = pd;

  //states and transition relation will be rebuilt, so delete the existing ones
  rPd.ClearTransRel();
  rPd.ClearStates();

  StateSet::Iterator stateit;
  Idx newStateAct, newStatePas;
  std::string active = "active";
  std::string passive = "passive";
  std::map<Idx,std::pair<Idx,Idx> > stateMap;
  //for insert an active and a passive state for each state of the old generator
  for(stateit = pd.StatesBegin(); stateit != pd.StatesEnd(); stateit++){

    //active state
    newStateAct = rPd.InsState();
    MergeStateAnnotation mssAct(*stateit,active);
    rPd.SetMerge(newStateAct,mssAct);

    //if the old state was a final state, the active state is final as well
    if(pd.ExistsMarkedState(*stateit)){
      rPd.SetMarkedState(newStateAct);
    }

    //if the old state was the starting state, the active state is a starting state
    //as well
    if(pd.ExistsInitState(*stateit)){
      rPd.SetInitState(newStateAct);
    }

    //passive state
    newStatePas = rPd.InsState();
    MergeStateAnnotation mssPas(*stateit,passive);
    rPd.SetMerge(newStatePas,mssPas);

    //save relation between old state and new states for later reference
    stateMap.insert(std::make_pair(*stateit,std::make_pair(newStateAct,newStatePas)));
  }

  TransSet::Iterator transit;
  PopPushSet::const_iterator ppit;
  std::vector<Idx> pop, push;
  Idx startState, endState, oldStartState, oldEndState, oldEvent;
  //insert transitions
  for(transit = pd.TransRelBegin(); transit != pd.TransRelEnd(); transit++){
    for(ppit = pd.PopPushBegin(*transit); ppit != pd.PopPushEnd(*transit); ppit++){

      //for convenience
      pop = ppit->first;
      push = ppit->second;
      oldStartState = transit->X1;
      oldEndState = transit->X2;
      oldEvent = transit->Ev;

      //for any reading transition
      if(!pd.IsEventLambda(oldEvent)){

        //connect active state to active state
        startState = stateMap.find(oldStartState)->second.first;
        endState = stateMap.find(oldEndState)->second.first;
        rPd.SetTransition(startState,oldEvent,endState,pop,push);

        //connect passive state to active state
        startState = stateMap.find(oldStartState)->second.second;
        rPd.SetTransition(startState,oldEvent,endState,pop,push);
      }
      else{

        //for any pushing and popping transition
        if(push.size() == 2 || pd.IsStackSymbolLambda(push.front())){

          //connect passive state to passive state
          startState = stateMap.find(oldStartState)->second.second;
          endState = stateMap.find(oldEndState)->second.second;
          rPd.SetTransition(startState,oldEvent,endState,pop,push);

          //if the old start state was marked, connect active state to passive state
          if(pd.ExistsMarkedState(oldStartState)){
            startState = stateMap.find(oldStartState)->second.first;
            rPd.SetTransition(startState,oldEvent,endState,pop,push);
          }

          //else connect active state to active state
          else{
            startState = stateMap.find(oldStartState)->second.first;
            endState = stateMap.find(oldEndState)->second.first;
            rPd.SetTransition(startState,oldEvent,endState,pop,push);
          }
        }

        //should never get here
        else{
          std::stringstream errstr;
          errstr << "While executing Nda(): Found transition which is neither read nor pop nor push, which is not allowed" << std::endl;
          throw Exception("Nda", errstr.str(), 1001);
        }
      }
    }
  }

  return rPd;
}

/* *************************
 * ReachableStateMap
 * *************************/
void ReachableStateMap(const PushdownGenerator& rPd, std::map<Idx,StateSet>& rMap){

	StateSet reachableStates;
	std::stack<Idx> todo;
	StateSet::Iterator stateit;
	TransSet::Iterator transit;
	std::map<Idx,StateSet>::iterator mapit;

	rMap.clear();

	//for all states
	for(stateit = rPd.StatesBegin(); stateit != rPd.StatesEnd(); ++stateit)
		if(*stateit == rPd.InitState()){
			//after trim, all states should be reachable from init state
			rMap[*stateit] = rPd.States();
		}else{
			//calculate reachable states (transitive closure)
			reachableStates.Clear();
			todo.push(*stateit);

			while(!todo.empty()){
				//get next
				Idx curr = todo.top();
				todo.pop();
				reachableStates.Insert(curr);

				//for all directly reachable states (neighbors)
				for(transit = rPd.TransRelBegin(curr); transit != rPd.TransRelEnd(curr); ++ transit){
					//if already regard, nothing todo
					if(reachableStates.Exists(transit->X2))
						continue;

					mapit = rMap.find(transit->X2);

					//if for the neighbor the reachable states are already calculated (= neighbor in map)
					//all states reachable from neighbor are also reachable from the current one
					if(mapit != rMap.end())
						reachableStates.InsertSet(mapit->second);
					else
						todo.push(transit->X2);
				}
			}

			rMap[*stateit] = reachableStates;
		}
}

/* *************************
 * TrimPushdownGenerator
 * *************************/
bool PushdownTrim(PushdownGenerator& pd, const uint n, bool debug) {
	DEBUG_PRINTF(debug,"**** Trim **** ","");

	if(pd.InitStatesEmpty() || pd.MarkedStatesEmpty()){
		//remember stack bottom
		StackSymbol sb = pd.StackSymbolObj(pd.StackBottom());

		//clear states, transitions and stacksymbols (except stack bottom)
		pd.ClearStates();
		pd.ClearTransRel();
		pd.DelStackSymbols(pd.StackSymbols());

		//reset stackbottom
		pd.SetStackBottom(sb);
		return false;
	}

	if(n == 0)
		//trim generator
		return pd.vGenerator::Trim();

	PushdownGenerator rPd(pd);

	//remove all transitions and just add possible ones
	rPd.ClearTransRel();

    StackSymbolSet usedStackSymbols;
    std::vector<Idx>::const_iterator ssit;
	TransSet::Iterator transIt;
	PopPushSet::iterator ppIt;
	std::vector<Idx> transPop, transPush, newStack, tmpStack, initStack, currStack;

	//stack bottom always used
	usedStackSymbols.Insert(rPd.StackBottom());

	// initialize todo stack
	//pairs (state,stack) represent a state with the current stack (in size of n)
	std::stack<std::pair<Idx,std::vector<Idx> > > todo;

	// initialize done set
	std::set<std::pair<Idx,std::vector<Idx> > > done;
	std::set<std::pair<Idx,std::vector<Idx> > >::iterator doneit;
	StateSet accStates,inaccStates;

	accStates.Insert(rPd.InitState());

	//create initial stack
	initStack.push_back(rPd.StackBottom());

	//create start situation (init state with stack = [stackBottom])
	std::pair<Idx,std::vector<Idx> >  start = std::make_pair(rPd.InitState(), initStack);
	todo.push(start);

	//start loop
	while (!todo.empty()) {
		//get last added
		std::pair<Idx,std::vector<Idx> >  curr = todo.top();
		todo.pop();

		//get state
		Idx currState = curr.first;
		//get stack
		currStack = curr.second;

		done.insert(curr);

		//for each transition from current
		for (transIt = pd.TransRelBegin(currState);
				transIt != pd.TransRelEnd(currState); ++transIt) {
			for (ppIt = pd.PopPushBegin(*transIt); ppIt != pd.PopPushEnd(*transIt); ++ppIt) {
				transPop = ppIt->first;
				transPush = ppIt->second;

				// check if pop-condition is satisfied by the given stack
				// The pop-condition is satisfied if the first k stack symbols correspond
				// with pop ( for k <= n)
				uint k = std::min(currStack.size(),transPop.size());
				bool popPossible = std::equal(transPop.begin(),transPop.begin() +std::min(k,n),currStack.begin());


				if (popPossible || pd.IsStackSymbolLambda(transPop.front())) {
					DEBUG_PRINTF(debug,"  pop is possible","");

					//add transition to resulting generator
					rPd.SetTransition(*transIt,transPop,transPush);
					DEBUG_PRINTF(debug,"Trim_SetTransition",rPd);


				    //extract stack symbols
				    for(ssit = transPop.begin(); ssit != transPop.end(); ssit++){
				      usedStackSymbols.Insert(*ssit);
				    }
				    //extract stack symbols
				    for(ssit = transPush.begin(); ssit != transPush.end(); ssit++){
				      usedStackSymbols.Insert(*ssit);
				    }

					tmpStack.clear();
					newStack.clear();

					//tmpStack = oldStack - pop   or empty
					if(currStack.size() > transPop.size())
						tmpStack.insert(tmpStack.begin(),currStack.begin()+transPop.size(),currStack.end());

					//newStack = push[0 to n]  if push != lambda
					if(!pd.IsStackSymbolLambda(transPush.front()))
						newStack.insert(newStack.begin(),transPush.begin(),std::min(transPush.begin()+n, transPush.end()));

					if(newStack.size() < n){
						uint j = n - newStack.size();
						//newStack = newStack + tmpStack[0 to j]
						newStack.insert(newStack.end(),tmpStack.begin(),std::min(tmpStack.begin()+j,tmpStack.end()));
					}

					//create new reachable situation (reached state with the new stack)
					std::pair<Idx,std::vector<Idx> > newSituation = std::make_pair(transIt->X2, newStack);

					//if the new situation already exists, no need to check
					if (done.find(newSituation) == done.end()) {
						DEBUG_PRINTF(debug,"  find new situation","");
						todo.push(newSituation);
						//add accessible state
						accStates.Insert(transIt->X2);
					}
				}
			}
		}
	}  //while

	//remove inaccessible states
	inaccStates = rPd.States() - accStates;
	rPd.DelStates(inaccStates);

	//delete all stack symbols except for the used ones
	StackSymbolSet::Iterator currentssit;
	for(currentssit = pd.StackSymbolsBegin(); currentssit != pd.StackSymbolsEnd(); currentssit++){
	  //stack symbol is not used and is not lambda
	  if(!usedStackSymbols.Exists(*currentssit) && !pd.IsStackSymbolLambda(*currentssit)){
	    rPd.DelStackSymbol(*currentssit);
	  }
	}

	DEBUG_PRINTF(debug,"TrimEnd",rPd);

	pd=rPd;

	//trim generator
	return pd.vGenerator::Trim();
}

/* *************************
 * SPDA
 * *************************/
PushdownGenerator SPDA(const PushdownGenerator& pd,
		TransformationHistoryVec* pTransformHistory) {
	FUNCTION(__FUNCTION__);	//scope log
	FD_DF("A SPDA(" << &pd << ")");

	//remember changes during this iteration
	std::vector<std::pair<Idx,MergeTransition> > changes;	//XXX:NEW

	//take a copy of the old generator
	PushdownGenerator rPd = pd;

	//annotate states of the new generator
	StateSet::Iterator stateit;
	for (stateit = pd.StatesBegin(); stateit != pd.StatesEnd(); stateit++) {
		MergeStateAnnotation msa = MergeStateAnnotation(*stateit, "old");
		rPd.SetMerge(*stateit, msa);
	}

	if (!rPd.ExistsStackSymbol(FAUDES_PD_LAMBDA))
		rPd.InsStackSymbol(FAUDES_PD_LAMBDA);

	//clear transition relation
	rPd.ClearTransRel();

	std::multimap<Transition, std::pair<std::vector<Idx>, std::vector<Idx> > >
			readSet, multiPushSet, popPushSet, unchangedSet, onlyStateChangeSet;
	std::multimap<Transition, std::pair<std::vector<Idx>, std::vector<Idx> > >::iterator multimapit;
	std::pair<Transition, std::pair<std::vector<Idx>, std::vector<Idx> > > transition;
	std::pair<std::vector<Idx>, std::vector<Idx> > popPushPair;
	std::vector<Idx> pop, push, pushEnd;
	TransSet::Iterator transit;
	PopPushSet::const_iterator ppit;

	//partition transitions of old generator into four sets
	//because states have only been copied (and annotated), the state indices remain
	//the same for both generators
	for (transit = pd.TransRelBegin(); transit != pd.TransRelEnd(); transit++) {
		for (ppit = pd.PopPushBegin(*transit); ppit != pd.PopPushEnd(*transit);
				ppit++) {

			//get pop and push pair
			pop = ppit->first;
			push = ppit->second;

			//create transition and decide further down into which set it will be put
			transition = std::make_pair(*transit, *ppit);

			//first set contains all transitions whose event is not lambda and pop does not
			//equal push
			//(i. e., are of the form (p,a,x,y,q) with x != y)
			if (!pd.IsEventLambda(transit->Ev) && pop != push) {
				readSet.insert(transition);
			}

			//all other sets do not read anything (they have lambda events)
			else if (pd.IsEventLambda(transit->Ev)) {

				//filter out all push transitions (p,lambda,x,yx,q), they will not need to be changed
				if (push.size() == 2 && push.back() == pop.front()) {
					unchangedSet.insert(transition);
					continue;
				}

				//XXX: NEW!
				//onlyStateChangeSet contains all transitions that pop and push same stack symbols
				//(i. e., are of the form (p,lambda,x,x,q) )
				else if (pop == push && pop.size() == 1) {
					onlyStateChangeSet.insert(transition);
					continue;
				}
				//(i. e., are of the form (p,lambda,x,yzx,q) with z not empty)
				else if (push.size() > 1) {
					//multiPushSet contains all transitions that push more then one symbol
					//(i. e., are of the form (p,lambda,x,yzx,q) with z not empty)
					if (push.back() == pop.front()) {
						multiPushSet.insert(transition);
					}

					//popPushSet set contains all transitions that don't effectively pop
					//(i. e., are of the form (p,lambda,x,yz,q) with a not empty, x != z)
					else {
						popPushSet.insert(transition);
					}
				}
				//also put transitions without lambda push into popPushSet
				//(i. e., are of the form (p,lambda,x,z,q) with z not empty and x != z)
				else if (!pd.IsStackSymbolLambda(push.front())) {
					popPushSet.insert(transition);
				}

				//put transitions with lambda push into unchangedSet
				else {
					unchangedSet.insert(transition);
				}
			}
			//put all remaining transitions into fourth set as well
			else {
				unchangedSet.insert(transition);
			}
		}
	}

	//add lambda to events in case it has not already been added
	Idx lambdaIdx = rPd.InsEvent(FAUDES_PD_LAMBDA);
	Idx newState;

	//for every transition (p,a,x,y,q) in the readSet
	for (multimapit = readSet.begin(); multimapit != readSet.end();
			multimapit++) {

		//add new intermediate state and annotate it with (p,a,x,x,p)
		newState = rPd.InsState();
		pop = multimapit->second.first;
		push = multimapit->second.first;
		MergeTransition mt(multimapit->first.X1, multimapit->first.Ev,
				multimapit->first.X1, pop, push);
		rPd.SetMerge(newState, mt);

		//add transition to and from intermediate state
		//(p,a,x,x,newstate)
		rPd.SetTransition(multimapit->first.X1, multimapit->first.Ev, newState,	pop, push);
		//(newstate,lambda,x,y,q)
		rPd.SetTransition(newState, lambdaIdx, multimapit->first.X2, pop, multimapit->second.second);

		//XXX:NEW
		//if history is needed, add new State merged with old Transition to changes
		if (pTransformHistory != NULL){
			MergeTransition oldTrans = MergeTransition(multimapit->first,multimapit->second.first, multimapit->second.second);
			changes.push_back(std::make_pair(newState,oldTrans));
		}
	}

	//for every transition (p,lambda,x,yax,q) with a not empty in the multiPushSet
	for (multimapit = multiPushSet.begin(); multimapit != multiPushSet.end();
			multimapit++) {

		//add new intermediate state and annotate it with (p,lambda,x,ax,p)
		newState = rPd.InsState();
		pop = multimapit->second.first;
		push = std::vector<Idx>(
				multimapit->second.second.end()	- multimapit->second.first.size() - 1,
				multimapit->second.second.end());
		MergeTransition mt(multimapit->first.X1, multimapit->first.Ev,
				multimapit->first.X1, pop, push);
		rPd.SetMerge(newState, mt);

		//add transition to and from intermediate state
		//(p,lambda,x,ax,newstate)
		rPd.SetTransition(multimapit->first.X1, multimapit->first.Ev, newState,	pop, push);
		//(newstate,lambda,a,ya,q)
		pop.clear();
		pop.push_back(multimapit->second.second.at(	multimapit->second.second.size()
								- multimapit->second.first.size() - 1));
		push = std::vector<Idx>(multimapit->second.second.begin(),
							    multimapit->second.second.end()	- multimapit->second.first.size());
		rPd.SetTransition(newState, lambdaIdx, multimapit->first.X2, pop, push);

		//XXX:NEW
		//if history is needed, add new State merged with old Transition to changes
		if (pTransformHistory != NULL){
			MergeTransition oldTrans = MergeTransition(multimapit->first,multimapit->second.first, multimapit->second.second);
			changes.push_back(std::make_pair(newState,oldTrans));
		}
	}

	//for every transition (p,lambda,x,yz,q) with z not empty and x != z in the popPushSet
	for (multimapit = popPushSet.begin(); multimapit != popPushSet.end();
			multimapit++) {

		//add new intermediate state and annotate it with (p,lambda,x,lambda,p)
		newState = rPd.InsState();
		pop = multimapit->second.first;
		push.clear();
		push.push_back(pd.StackSymbolIndex(FAUDES_PD_LAMBDA));
		MergeTransition mt(multimapit->first.X1, multimapit->first.Ev,
				multimapit->first.X1, pop, push);
		rPd.SetMerge(newState, mt);

		//add transition to and from intermediate state
		//(p,lambda,x,lambda,newstate)
		rPd.SetTransition(multimapit->first.X1, lambdaIdx, newState, pop, push);
		//(newstate,lambda,lambda,yz,q)
		pop.clear();
		pop.push_back(pd.StackSymbolIndex(FAUDES_PD_LAMBDA));
		push = multimapit->second.second;
		rPd.SetTransition(newState, lambdaIdx, multimapit->first.X2, pop, push);

		//XXX:NEW
		//if history is needed, add new State merged with old Transition to changes
		if (pTransformHistory != NULL){
			MergeTransition oldTrans = MergeTransition(multimapit->first,multimapit->second.first, multimapit->second.second);
			changes.push_back(std::make_pair(newState,oldTrans));
		}
	}

	//XXX:NEW!
	/* ******************************************************************************************************** */

	if (!onlyStateChangeSet.empty()) {
		//get new stack symbol
		std::string newStackSym = rPd.UniqueStackSymbolName("x");
		rPd.InsStackSymbol(newStackSym);

		//for every transition (p,lambda,x,x,q) in the set
		for (multimapit = onlyStateChangeSet.begin();
				multimapit != onlyStateChangeSet.end(); multimapit++) {

			//add new intermediate state and annotate it with (p,lambda,x,lambda,p)
			newState = rPd.InsState();
			pop = multimapit->second.first;
			push = multimapit->second.second;

			//add new stack symbol at the beginning of the push vector
			push.insert(push.begin(), rPd.StackSymbolIndex(newStackSym));

			MergeTransition mt(multimapit->first.X1, multimapit->first.Ev,
					multimapit->first.X1, pop, push);
			rPd.SetMerge(newState, mt);

			//add transition to and from intermediate state
			//(p,lambda,x,yx,newstate) with y = newStackSym
			rPd.SetTransition(multimapit->first.X1, lambdaIdx, newState, pop, push);

			pop.clear();
			pop.push_back(rPd.StackSymbolIndex(newStackSym));
			push.clear();
			push.push_back(pd.StackSymbolIndex(FAUDES_PD_LAMBDA));

			//(newstate,lambda,newStackSym,lambda,q)
			rPd.SetTransition(newState, lambdaIdx, multimapit->first.X2, pop, push);

			//if history is needed, add new State merged with old Transition to changes
			if (pTransformHistory != NULL){
				MergeTransition oldTrans = MergeTransition(multimapit->first,multimapit->second.first, multimapit->second.second);
				changes.push_back(std::make_pair(newState,oldTrans));
			}

		}
	}

	/* ************************************************************************************************************ */

	//for every transitions in the unchangedSet
	for (multimapit = unchangedSet.begin(); multimapit != unchangedSet.end();
			multimapit++) {

		//read pop and push for convenience
		pop = multimapit->second.first;
		push = multimapit->second.second;

		//XXX: OLD
		//if it is a read only transition in the form of (p,a,x,x,q), changed
		//it to (p,a,lambda,lambda,q)
		/*
		 if(!rPd.IsEventLambda(multimapit->first.Ev) && !pop.front().IsLambda() && pop == push ){
		   pop.clear();
		   pop.push_back(StackSymbol(FAUDES_PD_LAMBDA));
		   push.clear();
		   push.push_back(StackSymbol(FAUDES_PD_LAMBDA));
		 }
		 */
		//insert the transition again
		rPd.SetTransition(multimapit->first.X1, multimapit->first.Ev,
				multimapit->first.X2, pop, push);
	}

        //FD_WARN("SPDAa " << rPd.StackSymbols().ToString());
	
	//XXX:NEW
	//if history is needed, add all changes to history
	if (pTransformHistory != NULL)
		pTransformHistory->push_back(changes);


	//if lambda popping edges were added, they need to be removed first
	if (popPushSet.size() != 0) {
		rPd = RemoveLambdaPop(rPd);
	}
	//if changes were made, repeat recursively
	if (popPushSet.size() + onlyStateChangeSet.size() + readSet.size()
			+ multiPushSet.size() != 0) {
		//PushdownGeneratorTrim(rPd, FAUDES_PD_LOOKAHEAD);
		rPd = SPDA(rPd, pTransformHistory);
	}


        //FD_WARN("SPDAx " << rPd.StackSymbols().ToString());
	return rPd;

}
	

/* *************************
 * RebuildFromSPDA
 * *************************/
PushdownGenerator RebuildFromSPDA(const PushdownGenerator& sdpda,
		const TransformationHistoryVec& history) {
	FUNCTION(__FUNCTION__);	//scop log

	//copy
	PushdownGenerator rPd = PushdownGenerator(sdpda);

	TransformationHistoryVec::const_reverse_iterator historyit;
	std::vector<std::pair<Idx,MergeTransition> > merges;
	std::vector<std::pair<Idx,MergeTransition> >::iterator mergeit;
	std::vector<Idx> pop,push;
	std::vector<Idx>::iterator it;

	std::vector<Idx> t;
	t.push_back(rPd.InsLambdaStackSymbol());

	//go through history backwards
	for (historyit = history.rbegin(); historyit != history.rend();
			++historyit) {
		//get merges
		merges = *historyit;

		//recreate transition from merge (if possible)
		for (mergeit = merges.begin(); mergeit != merges.end(); ++mergeit) {
			//merge state
			Idx state = mergeit->first;
			//merge transition
			MergeTransition mt = mergeit->second;

			//if merge state still exists
			if (rPd.ExistsState(state)) {
				//if states of transition still exists
				if (rPd.ExistsState(mt.X1())
						&& rPd.ExistsState(mt.X2())) {

					pop = mt.Pop();
					push = mt.Push();

					for(it = pop.begin(); it != pop.end(); ++it)
						rPd.InsStackSymbol(rPd.StackSymbolName(*it));
					for(it = push.begin(); it != push.end(); ++it)
						rPd.InsStackSymbol(rPd.StackSymbolName(*it));

					//add transition
					rPd.SetTransition(mt.X1(), mt.Ev(),
							mt.X2(), mt.Pop(), mt.Push());
					//delete merge state
					rPd.DelState(state);
				}
			}
		}
	}

	return rPd;
}

/* *************************
 * RemainingPd
 * *************************/
PushdownGenerator RemainingPd(const Grammar& gr, const PushdownGenerator& pd) {
    FUNCTION(__FUNCTION__);	//scope log
    bool debug = false;		//debug RemainingPd?

    
	//take copy
	PushdownGenerator rPd(pd);

	//remove all transition and add only remaining ones
	rPd.ClearTransRel();

	std::set<GrammarProduction>::const_iterator proit;
	TransSet::Iterator transit;
	PopPushSet::const_iterator ppit;

	//check for all productions
	for (proit = gr.GrammarProductionsBegin();
			proit != gr.GrammarProductionsEnd(); ++proit) {
		Nonterminal lhs = proit->Lhs();
		GrammarSymbolVector rhs = proit->Rhs();
    
		// lhs = (qi,b) ( or (qi,b,qj) if lhs.mEndState != 0)
		//if (lhs.EndState() == 0) {
		TerminalPtr termPtr = std::dynamic_pointer_cast<Terminal>(
				rhs.front());
		if (termPtr) {
			// first elem of rhs is either lambda or an event (both are terminals)
			if (termPtr->IsLambda()) {
				// rhs = lambda
				if (lhs.EndState() == 0) {
					// (q,b) -> lambda
					// nothing to do
					DEBUG_PRINTF(debug, "Find Tfinal P = " + (*proit).Str(),"");
				} else {
					// type (ii)
					// (qi,b,qj) --> lambda
					for (transit = pd.TransRelBegin(lhs.StartState());
							transit != pd.TransRelEnd(lhs.StartState());
							++transit) {
						for (ppit = pd.PopPushBegin(*transit);
								ppit != pd.PopPushEnd(*transit); ++ppit) {
							std::vector<Idx> pop = ppit->first;
							std::vector<Idx> push = ppit->second;

							//pop transition (qi,lambda,b,lambda,qj) ==> (qi,b,qj) --> lambda
							if (transit->X2 == lhs.EndState()			// qj == qj
								&& pop.front() == lhs.OnStack().front()	// b == b
								&& pd.IsStackSymbolLambda(push.front())	// push == lambda
								&& pd.IsEventLambda(transit->Ev)) {		// event == lambda

								//add transition
								rPd.SetTransition(*transit, pop, push);	// add transition
								DEBUG_PRINTF(debug, "Find T2 P = "+ (*proit).Str(), "");
							}
						}
					}
				}
			} else {
				// rhs = a(qj,b[,qt])

				// type (i)
				// (qi,b[,qt]) --> a(qj,b[,qt])
				for (transit = pd.TransRelBegin(lhs.StartState());
						transit != pd.TransRelEnd(lhs.StartState());
						++transit) {
					for (ppit = pd.PopPushBegin(*transit);
							ppit != pd.PopPushEnd(*transit); ++ppit) {
						std::vector<Idx> pop = ppit->first;
						std::vector<Idx> push = ppit->second;

						NonterminalPtr nontermPtr =
								std::dynamic_pointer_cast<Nonterminal>(rhs.back()); // ->(qj,b[,qt])

						// read Transition (qi,a,b,b,qj) ==> (qi,b[,qt]) --> a(qj,b[,qt])
						if (nontermPtr) { // tmoor 2016-08: fix segfault
						  if(transit->X2 == nontermPtr->StartState() // qj == qj
								&& transit->Ev == termPtr->Event()			// a == a
								&& pop.front() == lhs.OnStack().front()		// b == b
								&& push.front() == lhs.OnStack().front()) {

							//add transition
							rPd.SetTransition(*transit, pop, push);	// add transition
							DEBUG_PRINTF(debug, "Find T1 P = "+ (*proit).Str(), "");

						  }} else {
							FD_WARN("RemainingPd: Find not identifiable production" + (*proit).Str());
						}
					}
				}
			}
		} else {
			// first elem of rhs is a nonterminal

			NonterminalPtr nonterm1Ptr =
					std::dynamic_pointer_cast<Nonterminal>(rhs.front());
			if (nonterm1Ptr) {
				if (rhs.size() == 1) {
					// type (iii L)
					// (qi,b) --> (qj,c)
					for (transit = pd.TransRelBegin(lhs.StartState());
							transit != pd.TransRelEnd(lhs.StartState());
							++transit) {
						for (ppit = pd.PopPushBegin(*transit);
								ppit != pd.PopPushEnd(*transit); ++ppit) {
							std::vector<Idx> pop = ppit->first;
							std::vector<Idx> push = ppit->second;

							// push Transition (qi,lambda,b,cb,qj) ==> (qi,b) --> (qj,c)
							if (transit->X2 == nonterm1Ptr->StartState() 	// qj == qj
									&& pop.front() == lhs.OnStack().front()	// b == b
									// check push
									&& push.size() == 2 // push 2 elements c and b
									&& push.front()
											== nonterm1Ptr->OnStack().front()// c == c
									&& push.back() == lhs.OnStack().front() // b == b
									&& pd.IsEventLambda(transit->Ev)) {		// ev == lambda

								//add transition
								rPd.SetTransition(*transit, pop, push);	// add transition
								DEBUG_PRINTF(debug, "Find T3L P = "+ (*proit).Str(), "");

							}
						}
					}
				} else { //rhs.size() > 1
					// type (iii R)
					// (qi,b[,qt]) --> (qj,c,qs)(qs,b[,qt])
					for (transit = pd.TransRelBegin(lhs.StartState());
							transit != pd.TransRelEnd(lhs.StartState());
							++transit) {
						for (ppit = pd.PopPushBegin(*transit);
								ppit != pd.PopPushEnd(*transit); ++ppit) {
							std::vector<Idx> pop = ppit->first;
							std::vector<Idx> push = ppit->second;

							NonterminalPtr nonterm2Ptr =
									std::dynamic_pointer_cast<Nonterminal>(
											rhs.back()); // ->(qs,b[,qt])

							// push Transition (qi,lambda,b,cb,qj) ==> (qi,b[,qt]) --> (qj,c,qs)(qs,b[,qt])
							if (nonterm2Ptr) {
							  if(transit->X2 == nonterm1Ptr->StartState() // qj == qj
									&& pop.front() == lhs.OnStack().front()		// pop b == b
									// check push
									&& push.size() == 2 // push 2 elements c and b
									&& push.front()
											== nonterm1Ptr->OnStack().front() // c == c
									&& push.back()
											== nonterm2Ptr->OnStack().front() // b == b
									&& pd.IsEventLambda(transit->Ev)) {	// ev == lambda

								//add transition
								rPd.SetTransition(*transit, pop, push);	// add transition
								DEBUG_PRINTF(debug, "Find T3R P = "+ (*proit).Str(), "");
							  }
							} else  {
								//ERROR:
								FD_WARN("RemainingPd: Find not identifiable production" + (*proit).Str());
							}
						}
					}
				}
			}
		}
	}	// foreach production


    return rPd;
}

/* *************************
 * RemoveLambdaPop
 * *************************/
PushdownGenerator RemoveLambdaPop(const PushdownGenerator& pd) {
	FUNCTION(__FUNCTION__); //scope log

	if(!pd.IsDeterministic()){
		FD_WARN("RemoveLambdaPop: Generator is not deterministic ! Perhaps not all lambda popping transitions will remove.") // TODO
	}

	//rename stack symbols and states
	PushdownGenerator rPd (pd);


	// XXX:is no longer needed
	/*********************************************************************************
	//remember important attributes of the generator before proceeding
	StackSymbolSet oldStackSymbols = rPd.StackSymbols();
	Idx oldInitState = rPd.InitState();
	std::string oldStackBottom = rPd.StackSymbolName(rPd.StackBottom());


	 //clear old init state and create new init state which will be annotated with "new"
	 //and derived from pd.InitState() in a merge state annotation
	 rPd.ClrInitState(rPd.InitState());
	 Idx newInitState = rPd.InsState();
	 rPd.SetInitState(newInitState);
	 MergeStateAnnotation msa(pd.InitState(),"new");
	 rPd.SetMerge(newInitState, msa);

	 //set new stack bottom
	 rPd.SetStackBottom("new-" + pd.StackSymbolName(pd.StackBottom()));

	 //if the old initial state was marked, the new initial state is marked as well
	 if(pd.ExistsMarkedState(pd.InitState())){
	 rPd.InsMarkedState(rPd.InitState());
	 }

	 //add transition that adds the old stack bottom symbol to the stack
	 //maybe lambda event needs to be inserted first
	 Idx lambdaIdx = rPd.InsEvent(FAUDES_PD_LAMBDA);
	 std::vector<StackSymbol> pop, push;
	 std::string newStackBottom = rPd.StackSymbolName(rPd.StackBottom());
	 //pop new stack bottom
	 pop.push_back(StackSymbol(newStackBottom));
	 //push new stack bottom and old stack bottom
	 push.push_back(StackSymbol(oldStackBottom));
	 push.push_back(StackSymbol(newStackBottom));
	 //add transition
	 rPd.SetTransition(newInitState, lambdaIdx, oldInitState, pop, push);

	 // ********************************************************************/

	//iterate over all transitions
	//take copy of transitions to iterate over
	//copying is necessary because transitions will get deleted
	//iterating over the original TransSet can lead to undefined behavior
	TransSet transRel = rPd.TransRel();
	TransSet::Iterator tit;
	StackSymbolSet::Iterator ssit;
	std::vector<Idx> examinedPop, examinedPush, newPop, newPush;

	for (tit = transRel.Begin(); tit != transRel.End(); tit++) {

		//look for lambda popping transitions (s1,ev,s2,lambda,w)
		//only need to look at the first pop/push pair, because only deterministic
		//generator are considered. the generator would be nondeterministic if there
		//was a transition in addition to a lambda popping one
		examinedPop = rPd.PopPush(*tit).begin()->first;
		examinedPush = rPd.PopPush(*tit).begin()->second;
		if (pd.IsStackSymbolLambda(examinedPop.front())) {

			//remove the transition
			rPd.ClrTransition(*tit, examinedPop, examinedPush);

			//add new transitions for every possible stack symbol u minus the just
			//inserted bottom and lambda so that it looks like (s1,ev,s2,u,uw) for
			for (ssit = rPd.StackSymbolsBegin(); ssit != rPd.StackSymbolsEnd();
					ssit++) {
				if (!rPd.IsStackSymbolLambda(*ssit)) {
					//fill pop with stack symbol
					newPop.clear();
					newPop.push_back(*ssit);
					//fill push with examinedPush plus stack symbol
					newPush.clear();
					newPush.insert(newPush.end(), examinedPush.begin(),
							examinedPush.end());
					newPush.push_back(*ssit);
					//add transition
					rPd.SetTransition(*tit, newPop, newPush);
				}
			}
		}
	}

	return rPd;
}

/* *************************
 * Sp2Lr
 * *************************/
Grammar Sp2Lr(const PushdownGenerator& xrPd,uint n, bool ignorReducible,bool debug) {
	FUNCTION(__FUNCTION__);	//scope log
	FD_DF("Sp2Lr(" << &xrPd << "," << n << ")");

	//Grammar to return
	Grammar rGr;

	StateSet::Iterator stateit1, stateit2;
	EventSet::Iterator eventit;
	TransSet::Iterator transit, transit2, transit3;
	StackSymbolSet::Iterator ssit;
	PopPushSet::const_iterator ppit, ppit2, ppit3;
	std::vector<Idx>::const_iterator popit, pushit;
	std::vector<Idx> ssVector, pop, push;
	std::map<Idx,StateSet> reachableStatesMap;
	GrammarSymbolVector rhs;

	//take a copy of pushdown generator
	PushdownGenerator tmpPd(xrPd);

	if(!ignorReducible){
		//Trim
		PushdownTrim(tmpPd,n);
	}

	//fill map
	ReachableStateMap(tmpPd,reachableStatesMap);

	//terminals equal the generator's events
	for (eventit = tmpPd.AlphabetBegin(); eventit != tmpPd.AlphabetEnd(); eventit++) {
		rGr.InsTerminal(Terminal(*eventit));
	}
	//Start Symbol (q0,$)
	ssVector.clear();
	ssVector.push_back(tmpPd.StackBottom());
	rGr.SetStartSymbol(Nonterminal(tmpPd.InitState(), ssVector));

	//create grammar productions:

	//for each final state qm and each stack symbol A
	//create (qm,A) -> lambda
	for (stateit1 = tmpPd.MarkedStatesBegin(); stateit1 != tmpPd.MarkedStatesEnd();
			stateit1++) {
		for (ssit = tmpPd.StackSymbolsBegin(); ssit != tmpPd.StackSymbolsEnd();
				ssit++) {
			if (!tmpPd.IsStackSymbolLambda(*ssit)) {

				//save used nonterminal
				ssVector.clear();
				ssVector.push_back(*ssit);
				Nonterminal nt(*stateit1, ssVector);
				rGr.InsNonterminal(nt);

				//insert a production ((qm,A) --> lambda)
				Terminal* t = new Terminal(tmpPd.EventIndex(FAUDES_PD_LAMBDA));
				GrammarSymbolPtr tPtr(t);
				GrammarSymbolVector v;
				v.push_back(tPtr);
				GrammarProduction gp(nt, v);
				rGr.InsGrammarProduction(gp);

				FD_DF("Sp2Lr: Inserting" + gp.Str())
				DEBUG_PRINTF(debug,"Sp2Lr: Inserting" + gp.Str(),"");
			}
		}
	}

	//For each pop-transition in form (qi,lambda,pop,lambda,qj)
	for (transit = tmpPd.TransRelBegin(); transit != tmpPd.TransRelEnd(); ++transit) {
		for (ppit = tmpPd.PopPushBegin(*transit); ppit != tmpPd.PopPushEnd(*transit); ppit++) {
			push = ppit->second;

			//pop transition (qi,lambda,pop,lambda,qj)
			if (tmpPd.IsStackSymbolLambda(push.front())) {
				Idx qi = transit->X1;
				Idx qj = transit->X2;
				pop = ppit->first;

		        //ensure that event = lambda
		        if(! tmpPd.IsEventLambda(transit->Ev)){
		          std::stringstream errstr;
		          errstr << "While executing Sp2Lr(): Found transition that push lambda but don't read lambda, "
		        		  "so is neither read nor pop, which is not allowed" << std::endl;
		          throw Exception("Sp2Lr", errstr.str(), 1001);
		        }

				//save lhs nonterminal as reducible
				Nonterminal ntLhs(qi, pop, qj);
				rGr.InsNonterminal(ntLhs);

				//insert production (qi,pop,qj) --> lambda
				Terminal* t = new Terminal(transit->Ev);
				GrammarSymbolPtr tPtr(t);
				rhs.clear();
				rhs.push_back(tPtr);
				rGr.InsGrammarProduction(GrammarProduction(ntLhs, rhs));

				FD_DF("Sp2Lr: Inserting" + GrammarProduction(ntLhs,rhs).Str())
				DEBUG_PRINTF(debug,"Sp2Lr: Inserting" + GrammarProduction(ntLhs,rhs).Str(),"");
			}
		}
	}

	//generate reducible productions until no more productions can be generated
	uint oldSize = 0;

	do{
		//save old number of grammar productions
		oldSize = rGr.GrammarProductions().size();

		//try to generate new grammar productions for each transition
		for (transit = tmpPd.TransRelBegin(); transit != tmpPd.TransRelEnd(); transit++) {
			for (ppit = tmpPd.PopPushBegin(*transit); ppit != tmpPd.PopPushEnd(*transit); ppit++) {

				Idx qi = transit->X1;
				Idx qj = transit->X2;
				Idx a = transit->Ev;
				std::vector<Idx> b = ppit->first;
				push = ppit->second;

				//read transition (qi,a,b,b,qj)
				if (!tmpPd.IsEventLambda(transit->Ev)) {

			        //ensure that pop = push
			        if(b != push){
			          std::stringstream errstr;
			          errstr << "While executing Sp2Lr(): Found transition that pop != push but don't read lambda, so is neither read nor pop nor push, which is not allowed" << std::endl;
			          throw Exception("Sp2Lr", errstr.str(), 1001);
			        }

					//insert production (qi,b) --> a(qj,b)
					Nonterminal ntLhs(qi, b);
					Terminal* t = new Terminal(a);
					GrammarSymbolPtr tPtr(t);
					Nonterminal* ntRhs = new Nonterminal(qj, b);
					GrammarSymbolPtr ntRhsPtr(ntRhs);

					if(ignorReducible)
						rGr.InsNonterminal(*ntRhs);

					//... but only if (qj,b) is reducible
					if (rGr.Nonterminals().find(*ntRhs)
							!= rGr.NonterminalsEnd()) {

						//save lhs nonterminal as reducible
						rGr.InsNonterminal(ntLhs);

						//insert production
						rhs.clear();
						rhs.push_back(tPtr);
						rhs.push_back(ntRhsPtr);
						rGr.InsGrammarProduction(GrammarProduction(ntLhs, rhs));

						FD_DF("Sp2Lr: Inserting" + GrammarProduction(ntLhs,rhs).Str())
						DEBUG_PRINTF(debug,"Sp2Lr: Inserting" + GrammarProduction(ntLhs,rhs).Str(),"");

					}

					//For all states qt that are reachable from qj
					StateSet reachFromTrg = reachableStatesMap.find(qj)->second;
					for (stateit1 = reachFromTrg.Begin(); stateit1 != reachFromTrg.End(); stateit1++) {

						Idx qt = *stateit1;

						//insert production (qi,b,qt) --> a (qj,b,qt)
						Nonterminal ntLhs(qi, b, qt);
						Nonterminal* ntRhs = new Nonterminal(qj, b, qt);

						if(ignorReducible)
							rGr.InsNonterminal(*ntRhs);

						//... but only if (qj,b,qt) is reducible
						if (rGr.Nonterminals().find(*ntRhs)
								!= rGr.NonterminalsEnd()) {
							//save lhs nonterminal as ignorReducible
							rGr.InsNonterminal(ntLhs);

							//insert production
							GrammarSymbolPtr ntRhsPtr(ntRhs);
							rhs.clear();
							rhs.push_back(tPtr);
							rhs.push_back(ntRhsPtr);
							rGr.InsGrammarProduction(GrammarProduction(ntLhs, rhs));

							FD_DF("Sp2Lr: Inserting" + GrammarProduction(ntLhs,rhs).Str())
							DEBUG_PRINTF(debug,"Sp2Lr: Inserting" + GrammarProduction(ntLhs,rhs).Str(),"");

						}
					}
				}  // end if read != lambda
				else {

					//pop transition (qi,lambda,b,lambda,qj)
					if (tmpPd.IsStackSymbolLambda(push.front())) {
						//already done
					}

					//push transition (qi,lambda,b,cb,qj)
					else if (push.size() == 2) {

						std::vector<Idx> c;
						c.push_back(push.front());

						//insert production (qi,b) --> (qj,c)
						Nonterminal ntLhs(qi, b);
						Nonterminal* ntRhs = new Nonterminal(qj, c);
						GrammarSymbolPtr ntRhsPtr(ntRhs);

						if(ignorReducible)
							rGr.InsNonterminal(*ntRhs);

						//... but only if (qj,c) is reducible
						if (rGr.Nonterminals().find(*ntRhs)
								!= rGr.NonterminalsEnd()) {

							//save lhs nonterminal as ignorReducible
							rGr.InsNonterminal(ntLhs);

							//insert production
							rhs.clear();
							rhs.push_back(ntRhsPtr);
							rGr.InsGrammarProduction(GrammarProduction(ntLhs, rhs));

							FD_DF("Sp2Lr: Inserting" + GrammarProduction(ntLhs,rhs).Str())
							DEBUG_PRINTF(debug,"Sp2Lr: Inserting" + GrammarProduction(ntLhs,rhs).Str(),"");
						}

						//For all states qs that are reachable from qj
						StateSet reachFromTrg = reachableStatesMap.find(qj)->second;
						for (stateit1 = reachFromTrg.Begin(); stateit1 != reachFromTrg.End(); stateit1++) {

							Idx qs = *stateit1;

							//insert production (qi,b) --> (qj,c,qs)(qs,b)
							Nonterminal ntLhs(qi, b);
							Nonterminal* ntRhs1 = new Nonterminal(qj, c, qs);
							GrammarSymbolPtr ntRhs1Ptr(ntRhs1);
							Nonterminal* ntRhs2 = new Nonterminal(qs, b);
							GrammarSymbolPtr ntRhs2Ptr(ntRhs2);

							if(ignorReducible){
								rGr.InsNonterminal(*ntRhs1);
								rGr.InsNonterminal(*ntRhs2);
							}

							//... but only if (qj,c,qs) and (qs,b) are reducible
							if (rGr.Nonterminals().find(*ntRhs1) != rGr.NonterminalsEnd()
									&& rGr.Nonterminals().find(*ntRhs2)	!= rGr.NonterminalsEnd()){

								//save lhs nonterminal as ignorReducible
								rGr.InsNonterminal(ntLhs);

								//insert production
								rhs.clear();
								rhs.push_back(ntRhs1Ptr);
								rhs.push_back(ntRhs2Ptr);
								rGr.InsGrammarProduction(GrammarProduction(ntLhs, rhs));

								FD_DF("Sp2Lr: Inserting" + GrammarProduction(ntLhs,rhs).Str())
								DEBUG_PRINTF(debug,"Sp2Lr: Inserting" + GrammarProduction(ntLhs,rhs).Str(),"");
							}

							//for each push transition (qs,_,c,lambda,qp)
							for (transit2 = tmpPd.TransRelBegin(qs); transit2 != tmpPd.TransRelEnd(qs); ++transit2) {
								for (ppit2 = tmpPd.PopPushBegin(*transit2); ppit2 != tmpPd.PopPushEnd(*transit2); ++ppit2) {

									if (tmpPd.IsStackSymbolLambda((ppit2->second).front()) && (ppit2->first).front() == push.front()) {
										//ey
										Idx qp = transit2->X2;

										//For all states qs' that are reachable from qs
										StateSet reachFromqs = reachableStatesMap.find(qs)->second;
										for (stateit2 = reachFromqs.Begin(); stateit2 != reachFromqs.End(); ++stateit2) {   // reach from qp
											//for each transition (qs',_,B,lambda,qt)
											for (transit3 = tmpPd.TransRelBegin(*stateit2); transit3 != tmpPd.TransRelEnd(*stateit2);
													++transit3) {
												//ex
												for (ppit3 = tmpPd.PopPushBegin(*transit3); ppit3 != tmpPd.PopPushEnd(*transit3);
														++ppit3) {
													if (tmpPd.IsStackSymbolLambda((ppit3->second).front())
															&& (ppit3->first).front() == push.back()) { //ex

														Idx qt = transit3->X2;

														//insert production (qi,b,qt) --> (qj,c,qp)(qp,b,qt)
														Nonterminal ntLhs(qi, b, qt);
														Nonterminal* ntRhs1 = new Nonterminal(qj,c,qp);
														GrammarSymbolPtr ntRhs1Ptr(ntRhs1);
														Nonterminal* ntRhs2 = new Nonterminal(qp,b,qt);
														GrammarSymbolPtr ntRhs2Ptr(ntRhs2);

														if(ignorReducible){
															rGr.InsNonterminal(*ntRhs1);
															rGr.InsNonterminal(*ntRhs2);
														}

														//... but only if (qj,c,qs) and (qs,b,qt) are reducible
														if (rGr.Nonterminals().find(*ntRhs1) != rGr.NonterminalsEnd()
															  && rGr.Nonterminals().find(*ntRhs2)	!= rGr.NonterminalsEnd()) {

															//save lhs nonterminal as ignorReducible
															rGr.InsNonterminal(ntLhs);

															//insert production
															rhs.clear();
															rhs.push_back(ntRhs1Ptr);
															rhs.push_back(ntRhs2Ptr);
															rGr.InsGrammarProduction(GrammarProduction(ntLhs,rhs));

															FD_DF("Sp2Lr: Inserting" + GrammarProduction(ntLhs,rhs).Str())
															DEBUG_PRINTF(debug,"Sp2Lr: Inserting" + GrammarProduction(ntLhs,rhs).Str(),"");
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		if(ignorReducible)
			return rGr;
	} while (rGr.GrammarProductions().size() != oldSize);

	//return CFG
	return rGr;
}

/* *************************
 * Filter
 * *************************/
std::set<Nonterminal> Filter(const std::set<Nonterminal>& symbolSet, const GrammarSymbolVector& w){

  //resulting set
  std::set<Nonterminal> rSet;

  GrammarSymbolVector::const_iterator wit;
  std::set<Nonterminal>::const_iterator ntit;
  //iterate over all symbols of the word
  for(wit = w.begin(); wit != w.end(); wit++){
    //test if current symbol is a nonterminal
    ConstNonterminalPtr nt = std::dynamic_pointer_cast<const Nonterminal>(*wit);
    if(nt){

      //look for the symbol in the nonterminal set
      ntit = symbolSet.find(*nt);

      //if the symbol was found, insert it into resulting set
      if(ntit != symbolSet.end()){
        rSet.insert(*ntit);
      }
    }
  }
  return rSet;
}

/* *************************
 * Rnpp1
 * *************************/
std::set<Nonterminal> Rnpp1(const Grammar& gr, const std::set<Nonterminal>& ntSet){

  std::set<Nonterminal> rSet, filterSet;
  //copy resulting set from set of already eliminated nonterminals
  rSet = ntSet;

  std::set<GrammarProduction>::const_iterator pit;
  //iterate over all productions

  for(pit = gr.GrammarProductions().begin(); pit != gr.GrammarProductions().end(); pit++){

    filterSet = Filter(gr.Nonterminals(),pit->Rhs());

    //test if the nonterminals on the right hand side of the production are all in
    //the set of eliminable nonterminals (i. e., they are eliminable)
    //if so, then the nonterminal on the left hand side is eliminable as well
    if(std::includes(ntSet.begin(), ntSet.end(), filterSet.begin(), filterSet.end())){
      //insert left hand side into set of eliminable terminals
      rSet.insert(pit->Lhs());
    }
  }
  return rSet;
}

/* *************************
 * Rnppl
 * *************************/
std::set<Nonterminal> Rnppl(const Grammar& gr, const std::set<Nonterminal>& ntSet){

  //get nonterminals that are eliminable in one step
  std::set<Nonterminal> rSet = Rnpp1(gr,ntSet);
  //look if that changed anything
  if(ntSet != rSet){
    //if any changes were detected, go on
    rSet = Rnppl(gr,rSet);
  }
  return rSet;
}

/* *************************
 * Rnpp
 * *************************/
Grammar Rnpp(const Grammar& gr){

  //the grammar to be returned. keep the start symbol
  Grammar rGr(gr.StartSymbol());

  //compute removable nonterminals
  std::set<Nonterminal> removableNts = Rnppl(gr,std::set<Nonterminal>());

  std::set<GrammarProduction> productions;
  //if the start symbol is a removable nonterminal

  if(removableNts.find(gr.StartSymbol()) != removableNts.end()){

    std::set<GrammarProduction>::const_iterator gpit;
    //iterate over grammar productions A -> w
    for(gpit = gr.GrammarProductionsBegin(); gpit != gr.GrammarProductionsEnd(); gpit++){

      //convert A to GrammarSymbolPtr and put it togther with w into a vector
      Nonterminal* a = new Nonterminal(gpit->Lhs());
      NonterminalPtr aPtr(a);
      GrammarSymbolVector symbols(gpit->Rhs());
      symbols.push_back(aPtr);

      std::set<Nonterminal> filteredSet;
      //filter all nonterminals that are in A and w
      filteredSet = Filter(gr.Nonterminals(), symbols);

      //test if filteredSet is a subset of the removable nonterminals
      if(std::includes(removableNts.begin(), removableNts.end(), filteredSet.begin(), filteredSet.end())){
        productions.insert(*gpit);
      }
    }
  }

  //keep the terminals
  rGr.InsTerminals(gr.Terminals());

  //keep only removable nonterminals
  rGr.InsNonterminals(removableNts);

  //keep only the productions from above
  rGr.InsGrammarProductions(productions);

  return rGr;
}

/* *************************
 * Rup
 * *************************/
Grammar Rup(const Grammar& gr){

   Grammar rGr;

  //copy terminals and start symbol from the old grammar
  rGr.InsTerminals(gr.Terminals());
  rGr.SetStartSymbol(gr.StartSymbol());

  std::set<GrammarProduction> todoProductions, reachableProductions;
  std::set<GrammarProduction>::const_iterator gpit;
  std::set<Nonterminal> usedNonterminals;

  //start with the start symbol, which is always used in a grammar
  usedNonterminals.insert(gr.StartSymbol());

  //get productions with start symbol as righthand side
  for(gpit = gr.GrammarProductionsBegin(); gpit != gr.GrammarProductionsEnd(); gpit++){
    if(gpit->Lhs() == gr.StartSymbol()){
     todoProductions.insert(*gpit);
    }
  }

  //look at all todo productions
  while(!todoProductions.empty()){

    //get the current todo production
    GrammarProduction currentGp(*todoProductions.begin());
    todoProductions.erase(currentGp);

    //since this production was reached, it is a reachable production
    reachableProductions.insert(currentGp);

    //insert all nonterminals of the current production into set of used nonterminals
    std::set<Nonterminal> filtered = Filter(gr.Nonterminals(),currentGp.Rhs());
    usedNonterminals.insert(filtered.begin(), filtered.end());

    //look at all productions
    for(gpit = gr.GrammarProductionsBegin(); gpit != gr.GrammarProductionsEnd(); gpit++){
      //if they have any of the used nonterminals on their lefthand side, insert them
      //into the todo productions
      if(usedNonterminals.find(gpit->Lhs()) != usedNonterminals.end()){
        todoProductions.insert(*gpit);
      }
    }

    //do not look at already reachable productions twice
    //this avoids infinite loops
    for(gpit = reachableProductions.begin(); gpit != reachableProductions.end(); gpit++){
      todoProductions.erase(*gpit);
    }
  }

  //insert all used nonterminals and all reachable productions
  rGr.InsNonterminals(usedNonterminals);
  rGr.InsGrammarProductions(reachableProductions);

  return rGr;
}


/* *************************
 * RemoveMultPop
 * *************************/
PushdownGenerator RemoveMultPop(const PushdownGenerator& pd) {

	//take a annotate copy of the old generator
	PushdownGenerator rPd = AnnotatePdStates("old",pd);

	//clear transition relation
	rPd.ClearTransRel();

	//lambda event
	Idx lambdaEv = rPd.InsEvent(FAUDES_PD_LAMBDA);

	//lambda stack symbol
	Idx lambdaSS = rPd.InsStackSymbol(FAUDES_PD_LAMBDA);

	StateSet::Iterator stateit;
	TransSet::Iterator transit;
	PopPushSet::const_iterator ppIt;

	Idx src,trg,lastState;
	std::vector<Idx> oldPop,oldPush,pop,push;
	std::vector<Idx>::iterator popit;
	//remember for each sub-vector of pop the new state
	std::map<std::vector<Idx>,Idx> popStateMap;
	std::map<std::vector<Idx>,Idx>::iterator mapit;

	//remove all multiple pop-transition
	//e.g. (p,a,x1x2...xN,w,q)  =>
	//(p,lambda,x1,lambda,p1), (p1,lambda,x2,lambda,p2), ... , (pN-1,lambda,xN,xN,pN),(pN,a,xN,w,q)
	//note: by the previous call of there is no lambda pop
	for(stateit = pd.StatesBegin(); stateit != pd.StatesEnd(); ++stateit){
		popStateMap.clear();
		for (transit = pd.TransRelBegin(*stateit); transit != pd.TransRelEnd(*stateit);	++transit) {
			for (ppIt = pd.PopPushBegin(*transit);	ppIt != pd.PopPushEnd(*transit); ++ppIt) {
				//get pop/push
				oldPop = ppIt->first;
				oldPush = ppIt->second;

				//ensure transition doesn't pop lambda
				if(pd.IsStackSymbolLambda(oldPop.front())){
				    std::stringstream errstr;
					errstr << "While executing RemoveMultPop(): Found transition that pop lambda, which is not allowed !" << std::endl;
					throw Exception("RemoveMultPop", errstr.str(), 1001);
				}

				//lastState contains the src for the next iteration
				lastState = transit->X1;
				for (popit = oldPop.begin(); popit != oldPop.end(); ++popit) {
					std::vector<Idx> tmpPop(oldPop.begin(),popit+1);
					src = lastState;

					//check if derivation of sup-pop exists
					mapit = popStateMap.find(tmpPop);
					if(mapit != popStateMap.end()){
						trg = mapit->second;
					}else{
						//if not, create new state
						trg = rPd.InsState();
						popStateMap[tmpPop] = trg;
					}

					//get pop for new transition
					pop.clear();
					pop.push_back(*popit);

					//get push for new transition
					push.clear();
					if(std::distance(popit, oldPop.end() ) == 1)
						//if last element of pop, push it
						push.push_back(*popit);
					else
						//else  push lambda
						push.push_back(lambdaSS);

					//add transition
					rPd.SetTransition(src, lambdaEv, trg, pop, push);

					//set new last state
					lastState = trg;
				}

				//create transition to original trg
				src = lastState;
				pop.clear();
				pop.push_back(oldPop.back());

				//add transition (pN,a,xN,w,q)
				rPd.SetTransition(src, transit->Ev, transit->X2, pop, oldPush);
			}
		}
	}
	return rPd;
}

/* *************************
 * InTransitionSize
 * *************************/
uint InTransitionSize(Idx trg,const PushdownGenerator& rPd){
	uint count = 0;

	// alternative order X2-Ev-X1
	TransSetX2EvX1 transByX2;
	TransSetX2EvX1::Iterator titEnd,transit;

	// copy and re-order transrel
	rPd.TransRel().ReSort(transByX2);

	//Set the iterator to a variable, so it won't be recalculated every iteration.
	titEnd = transByX2.EndByX2(trg);

	//for each incoming transition
	for(transit = transByX2.BeginByX2(trg); transit != titEnd; ++transit){
		if(rPd.PopPush(*transit).empty())
			//if no pop-push pair, just add one
			count++;
		else
			//else each pop-push pair represent a different transition
			count += rPd.PopPush(*transit).size();
	}

	return count;
}

/* *************************
 * PossibleAdjacentMerges
 * *************************/
std::set<std::pair <Transition,Transition> > PossibleAdjacentMerges (Idx q, const PushdownGenerator& pd){

	//result and temporary set
	std::set<std::pair <Transition,Transition> > res,tmp;

	// If no out edges, nothing to do
	if(!pd.ExistsTransition(q))
		return res;

	//Set the iterator to a variable, so it won't be recalculated every iteration.
	TransSet::Iterator transRel1Begin, transRel1End,transRel1it,
					   transRel2Begin, transRel2End, transRel2it;

	//init iterators
	transRel1Begin = pd.TransRelBegin(q);
	transRel1End = pd.TransRelEnd(q);

	//for each transition from state q
	for(transRel1it = transRel1Begin; transRel1it != transRel1End ; ++transRel1it){
		Idx q2 = transRel1it->X2;

		//ignore self-loop
		if(q == q2)
			continue;
		//ignore leading to marked state
		if(pd.ExistsMarkedState(q2))
			continue;
		//ignore leading to init state
		if(q2 == pd.InitState())
			continue;
		//ignore, if no transitions from q2
		if(!pd.ExistsTransition(q2))
			continue;
		//ignore if current transition isn't the only transition that leads to q2
		if(InTransitionSize(q2,pd) != 1)
			continue;

		//Set the iterator to a variable, so it won't be recalculated every iteration.
		transRel2Begin = pd.TransRelBegin(q2);
		transRel2End = pd.TransRelEnd(q2);
		tmp.clear();

		bool canNotMerge = false;
		transRel2it = transRel2Begin;
		while((transRel2it != transRel2End) && !canNotMerge){
			//Transitions can not merge, if...
			canNotMerge =	canNotMerge
					//...its a transition from a marked state to a not marked state
					||  (pd.ExistsMarkedState(transRel2it->X1) && !pd.ExistsMarkedState(transRel2it->X2))
					//...selfe loop
					||	(transRel2it->X1 == transRel2it->X2)
					//...the event are not lambda
					||	(!pd.IsEventLambda(transRel1it->Ev) && !pd.IsEventLambda(transRel2it->Ev));

			//if almost one transition can not merge, clear all
			if(canNotMerge)
				tmp.clear();
			else
				tmp.insert(std::make_pair(*transRel1it,*transRel2it));

			++transRel2it;
		}

		res.insert(tmp.begin(),tmp.end());
	}

	return res;

}

/* *************************
 * CombinedTransitions
 * *************************/
bool CombinedTransitions(Idx q,PushdownGenerator& rPd, /*std::set<Idx>&*/ StateSet& next,/*std::set<Idx>&*/ StateSet& done){
	bool debug = false;

	std::multimap<Transition, std::pair<std::vector<Idx>, std::vector<Idx> > > toRmTrans;
	std::multimap<Transition, std::pair<std::vector<Idx>, std::vector<Idx> > >::iterator toRmTransit;

	std::pair<Transition, std::pair<std::vector<Idx>, std::vector<Idx> > > pdtransition;
	std::pair<std::vector<Idx>, std::vector<Idx> > pp;
	std::vector<Idx> pop1,push1, pop2, push2, newPop, newPush;

	Transition trans1,trans2;
	TransSet::Iterator tit,tit_end;

	std::set<std::pair <Transition,Transition> > pairs ;
	std::set<std::pair <Transition,Transition> >::iterator pairsIt;
	PopPushSet::const_iterator pp1it, pp2it;

	DEBUG_PRINTF(debug,"CombinedTransitions: Start combine edge for "+rPd.StateStr(q),"");

	//get all possible mergeable pairs
	pairs = PossibleAdjacentMerges(q,rPd);
	DEBUG_PRINTF(debug,"mup done "+rPd.StateStr(q),"");

	if(!pairs.empty()){
		bool merged = false;

		//try to combine edges
		for(pairsIt = pairs.begin(); pairsIt != pairs.end(); ++pairsIt){
			//get transitions
			trans1 = pairsIt->first;
			trans2 = pairsIt->second;

			DEBUG_PRINTF(debug,"CombinedTransitions: Try ("+trans1.Str()+"/"+trans2.Str()+")","");

			for(pp1it = rPd.PopPushBegin(trans1); pp1it != rPd.PopPushEnd(trans1); ++pp1it){
				for(pp2it = rPd.PopPushBegin(trans2); pp2it != rPd.PopPushEnd(trans2); ++pp2it){
					// pop/push of first transition
					pop1  = pp1it->first;
					push1 = pp1it->second;

					//pop/push of second transition
					std::vector<Idx> pop2  = pp2it->first;
					std::vector<Idx> push2 = pp2it->second;

					//check if push1 and pop2 match
					if(std::equal(push1.begin(),push1.begin() + std::min(push1.size(),pop2.size()) ,pop2.begin())){
						Idx ev = (rPd.IsEventLambda(trans1.Ev))? trans2.Ev : trans1.Ev;
						newPop.clear();
						newPush.clear();

						if(!rPd.IsStackSymbolLambda(pop1.front()))
							newPop = pop1;

						if(push1.size() <= pop2.size()){
							//newPop = pop1 :: (pop2\push1)
							newPop.insert(newPop.end(),pop2.begin()+ push1.size(), pop2.end());
						}

						if(newPop.size() == 1){
							if(!rPd.IsStackSymbolLambda(push2.front()))
								newPush = push2;

							if(pop2.size() <= push1.size())
								//newPush = push1\pop2
								newPush.insert(newPush.end(),push1.begin() + pop2.size(), push1.end());

							Idx lambda = rPd.InsStackSymbol(FAUDES_PD_LAMBDA);
							if(newPop.empty()) newPop.push_back(lambda);
							if(newPush.empty())newPush.push_back(lambda);

							//remember to remove old transitions
							pp = std::make_pair(pop1,push1);
							pdtransition = std::make_pair(trans1,pp);
							toRmTrans.insert(pdtransition);

							pp = std::make_pair(pop2,push2);
							pdtransition = std::make_pair(trans2,pp);
							toRmTrans.insert(pdtransition);

							// set new Transition
							rPd.SetTransition(trans1.X1,ev,trans2.X2,newPop,newPush);

							Transition nt = *rPd.FindTransition(trans1.X1,ev,trans2.X2);

							DEBUG_PRINTF(debug,
									"CombinedTransitions: Merge ("+ trans1.Str()+ " | "+ trans2.Str() + ")  to "+ nt.Str() +"\n","");

							next.Insert(q);
							merged = true;
						}
						else
							DEBUG_PRINTF(debug,"CombinedTransitions: New Pop != 1","");

					}// pop-push incompatible
					else
						DEBUG_PRINTF(debug,"CombinedTransitions: Pop - Push incompatible","");
				}
			}
		}//for

		// remove merged transitions
		for(toRmTransit = toRmTrans.begin(); toRmTransit != toRmTrans.end(); ++toRmTransit)
			rPd.ClrTransition(toRmTransit->first,toRmTransit->second.first,toRmTransit->second.second);

		if(merged){
			return true;
		}

	}// pairs empty
	else
		DEBUG_PRINTF(debug,"CombinedTransitions: Pairs empty","");

	// if no transition merged together, get successor states
	StateSet succStates;

	tit = rPd.TransRelBegin(q);
	tit_end = rPd.TransRelEnd(q);
	for (; tit != tit_end; ++tit) {
	   succStates.Insert(tit->X2);
	}

	next.InsertSet(succStates);
	done.Insert(q);

	return false;
}

/* *************************
 * MergeAdjacentTransitions
 * *************************/
void MergeAdjacentTransitions(PushdownGenerator& rPd){

	bool merged;
	StateSet todo, done,next;
	StateSet::Iterator it;

	//repeat until all possible transitions are merged together
	do{

		//clean sets
		todo.Clear();
		done.Clear();

		//start with init state
		todo.Insert(rPd.InitState());

		//initially no transition merged
		merged = false;

		//check if at least one merge is possible
		while(!todo.Empty()){
			next.Clear();

			for(it = todo.Begin(); it != todo.End(); ++it){
				merged = merged || CombinedTransitions(*it,rPd,next,done);
			}

			todo.Clear();

			todo = next - done;
		}
	}while(merged);
}






} // namespace faudes

