/** @file pd_lang_k.cpp  Test class to derive strings from pushdown automata */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_lang_k.h"

namespace faudes {

std::set<std::string> LangK::FindLangK(uint k, bool showStack){
  
  Idx i = pd.InitState();
  std::string word = "";
  std::vector<Idx> stack;
  stack.push_back(pd.StackBottom());
  this->k = k;
  words.clear();
  //search for words
  Traverse(i,word,stack,0,showStack);
  return words;
}

void LangK::PrintWords(){
  std::cout << "found the following words while traversing at most " << k << " transitions:" << std::endl;
  std::set<std::string>::iterator it;
  for(it = words.begin(); it != words.end(); it++){
    std::cout << *it << std::endl;
  }    
}

void LangK::Traverse(Idx i, std::string word, std::vector<Idx> stack, uint depth, bool showStack){
  
  
  //print current state for better debugging visualization
  if(showStack){
    std::cout<< "Traverse("<<i<<","<< word <<",";
    std::vector<Idx>::reverse_iterator stackit;
    for(stackit = stack.rbegin(); stackit != stack.rend(); stackit++){
      std::cout << " " << pd.StackSymbolName(*stackit);
    }
    std::cout<<"), depth " << depth <<std::endl;
  }
  
  
  //add word if current state is final state
  if(pd.ExistsMarkedState(i)){
    if(word.compare("") == 0){
      words.insert(FAUDES_PD_LAMBDA);
    }
    else{
      words.insert(word);
    }
  }
  
  //test if final word length is reached
  if(depth == k){
    //add current word with indication that it is not finished and only if its not already been inserted
    if(!pd.ExistsMarkedState(i)){
      words.insert(word + " ...?");
    }
    //stop recursion
    return;
  }
  
  
  TransSet::Iterator it;
  std::vector<Idx> pop, currentStack;
  std::vector<Idx>::const_iterator itpush, itpop;
  PopPushSet popPush;
  PopPushSet::const_iterator ppit;
  std::vector<Idx>::const_reverse_iterator itstack;
  uint j;
  //examine all transitions with i as start state
  for(it = pd.TransRelBegin(i); it != pd.TransRelEnd(i); it++){
    
    //examine all pop/push pairs of the current transition
    popPush = pd.PopPush(*it);
    for(ppit = popPush.begin(); ppit != popPush.end(); ppit++){
      
      //take copy of current stack to work on
      currentStack = stack;

      //test if pop is lambda pop
      if(pd.IsStackSymbolLambda(ppit->first.front())){
        //pop is not necessary since it's a lambda pop
        //push onto stack, but don't push lambda!
        if(pd.IsStackSymbolLambda(!ppit->second.front())){
          currentStack.insert(currentStack.end(), ppit->second.rbegin(), ppit->second.rend());
        }
        //traverse next state
        std::string newWord = word;
        if(!pd.IsEventLambda(it->Ev)){
          newWord = word + pd.EventName(it->Ev) + " ";
        }
        Traverse(it->X2,newWord,currentStack, depth+1,showStack);
        continue;
      }
      
      //pop is no lambda pop
      //test if pop matches stack top
      itpop = ppit->first.begin();
      for(itstack = currentStack.rbegin(); itstack != currentStack.rend(); itstack++){
        
        //stack top and pop are not equal, stop comparison
        if(*itstack != *itpop){
          break;
        }
        
        //If all pop elements were equal and the end of pop is reached, then pop has been fully compared and is therefore equal to the stack top
        itpop++;
        if(itpop == ppit->first.end()){
          
          //pop from stack 
          for(j = 0; j < ppit->first.size(); j++){
            currentStack.pop_back();
          }
          //push onto stack,  but don't push lambda!
          if(!pd.IsStackSymbolLambda(ppit->second.front())){
            currentStack.insert(currentStack.end(), ppit->second.rbegin(), ppit->second.rend());
          }
          //traverse next state
          std::string newWord = word;
          if(!pd.IsEventLambda(it->Ev)){
            newWord = word + pd.EventName(it->Ev) + " ";
          }
          Traverse(it->X2,newWord,currentStack, depth+1,showStack);
          break;
        };
        
      }
      }
    
  }
}
  

} // namespace faudes

