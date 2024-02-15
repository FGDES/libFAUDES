/** @file pd_alg_cc_test.cpp  Unit Tests */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/
#include "pd_alg_main_test.h"

namespace faudes {

/* *****************
 * TestTransientStates
 * *****************/
void TestTransientStates(){
  std::string name = "Transient States";
  TestStart(name);
  
  PushdownGenerator g1 = TestGenerator6();
  
  StateSet states = Transient(g1);
  
  try{
    //expected states are s2 and s3
    if(!states.Exists(g1.StateIndex("s2")) || !states.Exists(g1.StateIndex("s3"))){
      std::stringstream errstr;
      errstr << "States " << g1.StateIndex("s2") << " and " << g1.StateIndex("s3") << " were expected but were not fount in the state set" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestTransientStatesEmpty
 * *****************/
void TestTransientStatesEmpty(){
  std::string name = "Transient States Empty";
  TestStart(name);
  
  PushdownGenerator g1 = TestGenerator4();
  
  StateSet states = Transient(g1);
  
  try{
    //no states are expected
    if(!states.Empty()){
      std::stringstream errstr;
      errstr << "State set was expected to be empty, but had size " << states.Size() << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestRnceRemoveEars
 * *****************/
void TestRnceRemoveEars(){
  std::string name = "Rnce Remove Ears";
  TestStart(name);
  
  System s = TestSystem2();
  PushdownGenerator g1 = TestGenerator5();
  
  PushdownGenerator g2 = Rnce(g1,s);
  
  try{
    //states s31 and s41 must be removed from the generator
    if(g2.ExistsState("s31") || g2.ExistsState("s41")){
      std::stringstream errstr;
      errstr << "States s31 and s41 were expected to be deleted, but at least one of them was found in the state set" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //only 6 states must be left in the generator
    if(g2.States().Size() != 6){
      std::stringstream errstr;
      errstr << "6 states were expected in the generator, but " << g2.States().Size() << " were found" <<  std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch (Exception e){ 
  }
  
  TestEnd(name);
}

/* *****************
 * TestIntersectEventsAll
 * *****************/
void TestIntersectEventsAll(){
  
  std::string name = "Intersect Events All";
  TestStart(name);
  
  System s;
  PushdownGenerator g1;
  
  //build two event sets to test every configuration of (un)controllable 
  //and (un)observable at least once
  s.InsEvent("a");                                      g1.InsEvent("a");
  s.InsControllableEvent("b"); s.ClrObservable("b");    g1.InsControllableEvent("b"); g1.ClrObservable("b");
  s.InsControllableEvent("c");                          g1.InsControllableEvent("c");
  s.InsUnobservableEvent("d");                          g1.InsUnobservableEvent("d");
  s.InsEvent("e");                                      g1.InsUnobservableEvent("e");
  s.InsEvent("f");                                      g1.InsControllableEvent("f"); g1.ClrObservable("f");
  s.InsEvent("g");                                      g1.InsControllableEvent("g");
  s.InsControllableEvent("h"); s.ClrObservable("h");    g1.InsUnobservableEvent("h");
  s.InsControllableEvent("i"); s.ClrObservable("i");    g1.InsControllableEvent("i");
  s.InsControllableEvent("j");                          g1.InsUnobservableEvent("j");
  
  PushdownGenerator g2;
  IntersectEvents(s, g1, g2);
  
  try{
    //resulting size of events has to be 4 (containing a, b, c and d)
    if(g2.Alphabet().Size() != 4){
      std::stringstream errstr;
      errstr << "size of event set is " << g2.Alphabet().Size() << " (";
      EventSet::Iterator evit;
      for(evit = g2.AlphabetBegin(); evit != g2.AlphabetEnd(); evit++){
        if(evit == g2.AlphabetBegin()){
          errstr << g2.EventName(*evit);
        }
        else{
          errstr << ", " << g2.EventName(*evit);
        }
      }
      errstr << "), but 4 (a, b, c, d) was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //a must be observable and must not be controllable
    if(!(!g2.Controllable("a") && g2.Observable("a"))){
      std::stringstream errstr;
      errstr << "event a must not be controllable and must be observable, but controllable: " << g2.Controllable("a") << " and observable: " << g2.Observable("a")<< std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //b must not be observable and must be controllable
    if(!(g2.Controllable("b") && !g2.Observable("b"))){
      std::stringstream errstr;
      errstr << "event b must be controllable and must not  be observable, but controllable: " << g2.Controllable("b") << " and observable: " << g2.Observable("b")<< std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //c must be observable and must be controllable
    if(!(g2.Controllable("c") && g2.Observable("c"))){
      std::stringstream errstr;
      errstr << "event c must be controllable and must be observable, but controllable: " << g2.Controllable("c") << " and observable: " << g2.Observable("c")<< std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    //d must not be observable and must not be controllable
    if(!(!g2.Controllable("d") && !g2.Observable("d"))){
      std::stringstream errstr;
      errstr << "event d must not be controllable and must not be observable, but controllable: " << g2.Controllable("d") << " and observable: " << g2.Observable("d")<< std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestTimesStates
 * *****************/
void TestTimesStates(){
  
  std::string name = "Times States";
  TestStart(name);
  
  System s = TestSystem1();
  PushdownGenerator g1 = TestGenerator2();
  
  //create expected merge states
  std::set< std::vector<Idx> > expectedMergeStateSet;
  std::vector<Idx> expectedMergeStates;
  StateSet::Iterator sit, g1it, g2it;
  for(sit = s.StatesBegin(); sit != s.StatesEnd(); sit++){
    for(g1it = g1.StatesBegin(); g1it != g1.StatesEnd(); g1it++){
      expectedMergeStates.clear();
      expectedMergeStates.push_back(*sit);
      expectedMergeStates.push_back(*g1it);
      expectedMergeStateSet.insert(expectedMergeStates);
    }
  }
  
  PushdownGenerator g2 = Times(s,g1);
  
  try{
    //the number of states in g2 must be g2States = sStates*g1States 
    if(g2.States().Size() != s.States().Size() * g1.States().Size()){
      std::stringstream errstr;
      errstr << "number of states incorrect, was " << g2.States().Size() << ", but " << s.States().Size() * g1.States().Size() << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //all elements of the cartesian product of state indices g1States x sStates
    //must appear uniquely in the merge states attribute
    std::set<std::vector<Idx> >::const_iterator emsit;
    for(g2it = g2.StatesBegin(); g2it != g2.StatesEnd(); g2it++){
      
      //check if the merge attribute is of type MergeStates
      const MergeStates* ms = dynamic_cast<const MergeStates*> (g2.StateAttribute(*g2it).Merge());
      if(ms == NULL){
        std::stringstream errstr;
        errstr << "MergeStateAnnotation in state " << *g2it << " not set." << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
      
      emsit = expectedMergeStateSet.find(ms->States());
      //check if the contained state indices match the expected indices
      if(emsit == expectedMergeStateSet.end()){
        std::stringstream errstr;
        errstr << "mergeStates attribute of state " << *g2it << " was (";
        std::vector<Idx>::const_iterator vit;
        for(vit = ms->States().begin(); vit != ms->States().end(); vit++){
          if(vit == ms->States().begin()){
            errstr << *vit;
          }
          else{
            errstr << ", " << *vit;
          }
        }
       errstr << "), but was not expected. it may have already occured or not been expected at all" << std::endl;
       throw Exception(name, errstr.str(), 1003);
      }
      //if indices match, delete the expected element because it is only expected once
      expectedMergeStateSet.erase(ms->States());
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestTimesStackSymbols
 * *****************/
void TestTimesStackSymbols(){
  std::string name = "Times Stack Symbols";
  TestStart(name);
  
  System s = TestSystem1();
  PushdownGenerator g1 = TestGenerator2();
  
  PushdownGenerator g2 = Times(s,g1);
  
  try{
    StackSymbolSet::Iterator g1ssit, g2ssit;
    //ever stack symbol in g1 must be in g2
    for(g1ssit = g1.StackSymbolsBegin(); g1ssit != g1.StackSymbolsEnd(); g1ssit++){
      g2ssit = g2.StackSymbols().Find(g1.StackSymbolName(*g1ssit));
      if(g2ssit == g2.StackSymbols().End()){
        std::stringstream errstr;
        errstr << "stack symbol " << g1.StackSymbolName(*g1ssit) << " was expected but not found" << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
      
    //the number of stack symbols in g2 must be the number of stack symbols in g1
    if(g2.StackSymbols().Size() != g1.StackSymbols().Size()){
      std::stringstream errstr;
      errstr << "number of stack symbols incorrect, was " << g2.StackSymbols().Size() << ", but " << g1.StackSymbols().Size() << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //the stack bottoms must match
    if(g2.StackSymbolObj(g2.StackBottom()) != g1.StackSymbolObj(g1.StackBottom())){
      std::stringstream errstr;
      errstr << "stack bottom symbol incorrect, was " << g2.StackSymbolName(g2.StackBottom()) << ", but " << g1.StackSymbolName(g1.StackBottom()) << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
   catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestTimesTransitions
 * *****************/
void TestTimesTransitions(){
  std::string name = "Times Transitions";
  TestStart(name);
  
  System s = TestSystem1();
  PushdownGenerator g1 = TestGenerator2();
  
  TransSet::Iterator g1Transit;
  //determine number of expected lambda-read transitions in resulting generator
  uint expectedLambdaReads = 0;
  for(g1Transit = g1.TransRelBegin(); g1Transit != g1.TransRelEnd(); g1Transit++){
    if(g1.EventName(g1Transit->Ev).compare(FAUDES_PD_LAMBDA) == 0){
      expectedLambdaReads += s.States().Size() * g1.PopPush(*g1Transit).size();
    }
  }
  
  TransSet::Iterator sTransit;
  //determine number of expected transtitions (without lambda read)
  uint expectedTransitions = 0;
  for(g1Transit = g1.TransRelBegin(); g1Transit != g1.TransRelEnd(); g1Transit++){
    for(sTransit = s.TransRelBegin(); sTransit != s.TransRelEnd(); sTransit++){
      if(g1.EventName(g1Transit->Ev).compare(s.EventName(sTransit->Ev)) == 0){
        expectedTransitions += g1.PopPush(*g1Transit).size();
      }
    }
  }
  
  PushdownGenerator g2 = Times(s,g1);
  
  try{
    TransSet::Iterator g2Transit;
    //for every lambda-read transition in g1 there must be a lambda read transition
    //for every state in s
    uint lambdaReads = 0;
    for(g2Transit = g2.TransRelBegin(); g2Transit != g2.TransRelEnd(); g2Transit++){
      if(g2.EventName(g2Transit->Ev).compare(FAUDES_PD_LAMBDA) == 0){
        lambdaReads += g2.PopPush(*g2Transit).size();
      }
    }
    if(lambdaReads != expectedLambdaReads){
      std::stringstream errstr;
      errstr << "number of lambda read transition was " << lambdaReads << ", but " << expectedLambdaReads << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //the total number of transitions should be
    //expectedLambdaReads + expectedTransitions
    if(lambdaReads + expectedTransitions != g2.TransRel().Size()){
      std::stringstream errstr;
      errstr << "total number of transitions was " << g2.TransRel().Size() << ", but " << expectedLambdaReads + expectedTransitions << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestSplitStackSymbols
 * *****************/
void TestSplitStackSymbols(){
  std::string name = "Split Stack Symbols";
  TestStart(name);
  
  PushdownGenerator g1 = TestGenerator3();
  
  PushdownGenerator g2 = Split(g1);
  
  try{
    //the stack bottom symbol must not be changed
    if(g2.StackSymbolObj(g2.StackBottom()) != g1.StackSymbolObj(g1.StackBottom())){
      std::stringstream errstr;
      errstr << "stack bottom symbol was " << g2.StackSymbolName(g2.StackBottom()) << ", but " << g1.StackSymbolName(g1.StackBottom()) << " was expected" << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    StackSymbolSet::Iterator ssit;
    //all stack symbols of the old generator must found in the new generator
    for(ssit = g1.StackSymbolsBegin(); ssit != g1.StackSymbolsEnd(); ssit++){
      if(g2.StackSymbols().Exists(g1.StackSymbolName(*ssit)) == 0){
        std::stringstream errstr;
        errstr << "stack symbol " << g1.StackSymbolName(*ssit) << " was expected in the stack symbol set but not found" << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
    }
  }
  catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestSplitStates
 * *****************/
void TestSplitStates(){
  std::string name = "Split States";
  TestStart(name);
  
  PushdownGenerator g1 = TestGenerator3();
  
  //set of split state attributes that are expected
  std::set<std::pair<Idx,Idx> > splitStateSet;
  StateSet::Iterator stateit;    
  StackSymbolSet::Iterator ssit;
  for(stateit = g1.StatesBegin(); stateit != g1.StatesEnd(); stateit++){
    
    //insert expected head
    splitStateSet.insert(std::make_pair(*stateit, 0));
    for(ssit = g1.StackSymbolsBegin(); ssit != g1.StackSymbolsEnd(); ssit++){
      
      if(!g1.IsStackSymbolLambda(*ssit)){
        splitStateSet.insert(std::make_pair(*stateit, *ssit));
      }
    }
  }
  
  PushdownGenerator g2 = Split(g1);
  
  try{
    
    for(stateit = g2.StatesBegin(); stateit != g2.StatesEnd(); stateit++){
      
      const MergeStateSplit* mss = dynamic_cast<const MergeStateSplit*>(g2.StateAttribute(*stateit).Merge());
      
      //test if MergeStateSplit was set
      if(mss == NULL){
        std::stringstream errstr;
        errstr << "MergeStateSplit attribute not set for state " << *stateit << std::endl;
        throw Exception(name, errstr.str(), 1003);
      }
      
      //test if state was expected
      if(splitStateSet.erase(std::make_pair(mss->State(), mss->Symbol())) == 0){
        std::stringstream errstr;
        if(!mss->IsHead()){
          errstr << "Ear state with merge attribute (state: " << mss->State() << ", stack symbol: " << g2.StackSymbolName(mss->Symbol()) << ") found, but was not expected." << std::endl;
        }
        
        else{
          errstr << "Head state with merge attribute (state: " << mss->State() << ") found, but was not expected." << std::endl;
        }
        throw Exception(name, errstr.str(), 1003);
      }
    }
    
    //test if any expected states are left
    if(splitStateSet.size() != 0){
      std::stringstream errstr;
      errstr << splitStateSet.size() << " states are missing from the generator: \n";
      
      std::set<std::pair<Idx,Idx> >::iterator it;
      for(it = splitStateSet.begin(); it != splitStateSet.end(); it++){
        if(it->second != 0){
          errstr << "ear state with merge attribute (state: " << it->first << ", stack symbol: " << g2.StackSymbolName(it->second) << ")\n";
        }
        else{
          errstr << "head state with merge attribute (state: " << it->first << ")\n";
        }
      }
      errstr << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestSplitTransitionsHeadToEar
 * *****************/
void TestSplitTransitionsHeadToEar(){
  std::string name = "Split Transitions Head To Ear";
  TestStart(name);
  
  PushdownGenerator g1 = TestGenerator3();
  
  PushdownGenerator g2 = Split(g1);
  
  try{
    StateSet::Iterator stateit;
    TransSet::Iterator transit;
    PopPushSet::const_iterator ppit;
    //look at all states
    for(stateit = g2.StatesBegin(); stateit != g2.StatesEnd(); stateit++){
      
      const MergeStateSplit* mss1 = dynamic_cast<const MergeStateSplit*>(g2.StateAttribute(*stateit).Merge());
      
      //if the current state is a head
      if(mss1->IsHead()){
        
        //look at all transitions starting here
        for(transit = g2.TransRelBegin(*stateit); transit != g2.TransRelEnd(*stateit); transit++){
          for(ppit = g2.PopPushBegin(*transit); ppit != g2.PopPushEnd(*transit); ppit++){
            
            const MergeStateSplit* mss2 = dynamic_cast<const MergeStateSplit*>(g2.StateAttribute(transit->X2).Merge());
            
            //the end state of the transition must be an ear (i. e. not a head)
            if(mss2->IsHead()){
              std::stringstream errstr;
              errstr << "Transition from head to head found (from " << *stateit << " to " << transit->X2 << ") found, but from head to ear was expected." << std::endl;
              throw Exception(name, errstr.str(), 1003);
            }
            
            //pop and push size must be one
            if(ppit->first.size() != 1 || ppit->second.size() != 1){
              std::stringstream errstr;
              errstr << "Pop size was " << ppit->first.size() << " and push size was " << ppit->second.size() << " at transition from head state " << transit->X1 << " to ear state " << transit->X2 << ", but size 1 was expected for both." << std::endl;
              throw Exception(name, errstr.str(), 1003);
            }
            
            //the ear's associated stack symbol must be popped and pushed
            if(mss2->Symbol() != ppit->first.front() || mss2->Symbol() != ppit->second.front()){
              std::stringstream errstr;
              errstr << "Transition from head state " << transit->X1 << " to ear state " << transit->X2 << "has pop stack symbol idx" << ppit->first.front() << " and push stack symbol idx" << ppit->second.front() << ", but expected was the stack symbol idx" << mss2->Symbol() << ", because it is associated with the ear." <<  std::endl;
              throw Exception(name, errstr.str(), 1003);
            }
          }
        }
      }
    }
  }
  catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestSplitTransitionsEarToHead
 * *****************/
void TestSplitTransitionsEarToHead(){
  std::string name = "Split Transitions Ear To Head";
  TestStart(name);
  
  PushdownGenerator g1 = TestGenerator3();
  
  PushdownGenerator g2 = Split(g1);
  
  try{
    StateSet::Iterator stateit;
    TransSet::Iterator transit;
    PopPushSet::const_iterator ppit;
    //look at all states
    for(stateit = g2.StatesBegin(); stateit != g2.StatesEnd(); stateit++){
      
      const MergeStateSplit* mss1 = dynamic_cast<const MergeStateSplit*>(g2.StateAttribute(*stateit).Merge());
      
      //if the current state is an ear
      if(!mss1->IsHead()){
        
        //look at all transitions starting here
        for(transit = g2.TransRelBegin(*stateit); transit != g2.TransRelEnd(*stateit); transit++){
          for(ppit = g2.PopPushBegin(*transit); ppit != g2.PopPushEnd(*transit); ppit++){
            
            const MergeStateSplit* mss2 = dynamic_cast<const MergeStateSplit*>(g2.StateAttribute(transit->X2).Merge());
            
            //the end state of the transition must be a head
            if(!mss2->IsHead()){
              std::stringstream errstr;
              errstr << "Transition from ear to ear found (from " << *stateit << " to " << transit->X2 << ") found, but from ear to head was expected." << std::endl;
              throw Exception(name, errstr.str(), 1003);
            }
            
            //pop size must be one
            if(ppit->first.size() != 1){
              std::stringstream errstr;
              errstr << "Pop size was " << ppit->first.size() << " but size 1 was expected." << std::endl;
              throw Exception(name, errstr.str(), 1003);
            }
            
            //the ear's associated stack symbol must be popped
            if(mss1->Symbol() != ppit->first.front()){
              std::stringstream errstr;
              errstr << "Transition from ear state " << transit->X1 << " to head state " << transit->X2 << "has pop stack symbol idx " << ppit->first.front() << ", but " << mss1->Symbol() << " was expected, because it is associated with the ear." <<  std::endl;
              throw Exception(name, errstr.str(), 1003);
            }
          }
        }
      }
    }
  }
  catch(Exception e){
  }
  
  TestEnd(name);
}

/* *****************
 * TestAcAccessible
 * *****************/
void TestAcAccessible(){
  std::string name = "Ac Accessible";
  TestStart(name);
  
  PushdownGenerator pd = TestGenerator17();
  
  PushdownGenerator rPd;
  PushdownAccessible(pd,rPd);
    
  try{

    //states s1, s2 and s4 must remain
    if(!rPd.ExistsState("s1")){
      std::stringstream errstr;
      errstr << "State s1 was expected to exist, but did not." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(!rPd.ExistsState("s2")){
      std::stringstream errstr;
      errstr << "State s2 was expected to exist, but did not." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    if(!rPd.ExistsState("s4")){
      std::stringstream errstr;
      errstr << "State s4 was expected to exist, but did not." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //there must be 3 states remaining
    if(rPd.Size() != 3){
      std::stringstream errstr;
      errstr << "Number of remaining states was expected to be 3, but was "<< rPd.Size() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
    
    //there must be 2 transitions remaining
    if(rPd.TransRelSize() != 2){
      std::stringstream errstr;
      errstr << "Number of remaining transitions was expected to be 2, but was "<< rPd.TransRelSize() << "." << std::endl;
      throw Exception(name, errstr.str(), 1003);
    }
  }
  catch(Exception e){
  }
  
  TestEnd(name);
}
  
/* *****************
 * TestTransient
 * *****************/
void TestTransient(){
  
  TestTransientStates();
  TestTransientStatesEmpty();
}

/* *****************
 * TestRnce
 * *****************/
void TestRnce(){
  
  TestRnceRemoveEars();
}

/* *****************
 * TestTimes
 * *****************/
void TestTimes(){
  
  TestIntersectEvents();
  TestTimesStates();
  TestTimesStackSymbols();
  TestTimesTransitions();
}

/* *****************
 * TestIntersectEvents
 * *****************/
void TestIntersectEvents(){
  TestIntersectEventsAll();
}

/* *****************
 * TestSplit
 * *****************/
void TestSplit(){
  TestSplitStackSymbols();
  TestSplitStates();
  TestSplitTransitionsHeadToEar();
  TestSplitTransitionsEarToHead();
}

/* *****************
 * TestAc
 * *****************/
void TestAc(){
  TestAcAccessible();
}
} // namespace faudes

