/** @file pd_parser.cpp  parser data structure*/


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_parser.h"

namespace faudes {


/*******************************
* 
* Implementation of Lr1Configuration
*
*/ 

Lr1Configuration::Lr1Configuration (const Nonterminal& lhs, const GrammarSymbolVector& beforeDot, const GrammarSymbolVector& afterDot, const Terminal& lookahead) : mLhs(lhs), mBeforeDot(beforeDot), mAfterDot(afterDot), mLookahead(lookahead){
  if(beforeDot.empty()) {
    std::stringstream errstr;
    errstr << "mBeforeDot must not be empty." <<std::endl;
    throw Exception("Lr1Configuration::Lr1Configuration", errstr.str(), 1001);
  }
  if(afterDot.empty()) {
    std::stringstream errstr;
    errstr << "mAfterDot must not be empty." <<std::endl;
    throw Exception("Lr1Configuration::Lr1Configuration", errstr.str(), 1001);
  }
}

bool Lr1Configuration::operator<(const Lr1Configuration& other) const{
  //compare Lhs
  if(mLhs < other.Lhs()) return true;
  if(other.Lhs() < mLhs) return false;
  
  //compare beforeDot
  if(CompareGsVector(mBeforeDot,other.BeforeDot())) return true;
  if(CompareGsVector(other.BeforeDot(),mBeforeDot)) return false;
  
  //compare afterDot
  if(CompareGsVector(mAfterDot,other.AfterDot())) return true;
  if(CompareGsVector(other.AfterDot(),mAfterDot)) return false;
  
  //compare lookahead
  if(mLookahead < other.Lookahead()) return true;  
  
  return false; 
}

bool Lr1Configuration::operator==(const Lr1Configuration& other) const{
  return !((*this < other) || (other < *this)); 
}

bool Lr1Configuration::operator!=(const Lr1Configuration& other) const{
  return ((*this < other) || (other < *this)); 
}

std::string Lr1Configuration::Str() const{
  
  std::stringstream s;
  GrammarSymbolVector::const_iterator gsvit;
  
  s << mLhs.Str() << " → ";
  for(gsvit = mBeforeDot.begin(); gsvit != mBeforeDot.end(); gsvit++){
    s << (*gsvit)->Str();
  }
  s << " • ";
  for(gsvit = mAfterDot.begin(); gsvit != mAfterDot.end(); gsvit++){
    s << (*gsvit)->Str();
  }
  s << ", " << mLookahead.Str();
  
  
  return s.str();
}

/*******************************
* 
* Implementation of CompareConfigSet
*
*/  
bool CompareConfigSet(const std::set<Lr1Configuration>& lhs, const std::set<Lr1Configuration>& rhs){
  if (lhs.size() < rhs.size()) return true; //also makes sure that it2 will not go out of bounds
  if (rhs.size() < lhs.size()) return false;
  
  std::set<Lr1Configuration>::const_iterator it1, it2;
  it2 = rhs.begin();
  for(it1 = lhs.begin(); it1 != lhs.end(); it1++){
    if(*it1 < *it2) return true;
    if(*it2 < *it1) return false;
    it2++;
  }
  return false;
}

/*******************************
* 
* Implementation of CompareConfigGsPair
*
*/
bool CompareConfigGsPair(const ConfigSetGsPair& lhs, const ConfigSetGsPair& rhs){
  if(*lhs.second < *rhs.second) return true;
  if(*rhs.second < *lhs.second) return false;
  if(CompareConfigSet(lhs.first,rhs.first)) return true;
  return false;
}

/*******************************
* 
* Implementation of ConfigSetToStr
*
*/
std::string ConfigSetToStr(const std::set<Lr1Configuration>& configs){
  std::stringstream s;
  std::set<Lr1Configuration>::const_iterator configit;
  for(configit = configs.begin(); configit != configs.end(); configit++){
    if(configit != configs.begin()){
      s << ", ";
    }
    s << "(" << configit->Str() <<")";
  }
  return s.str();
}

/*******************************
* 
* Implementation of ConfigSetSetToStr
*
*/
std::string ConfigSetSetToStr(const Lr1ConfigurationSetSet configSetSet){
  std::stringstream s;
  Lr1ConfigurationSetSet::const_iterator setit;
  int i = 0;
  for(setit = configSetSet.begin(); setit != configSetSet.end(); setit++){
    s << i << ":\n";
    s << "  " << ConfigSetToStr(*setit) << "\n";
    i++;
  }
  return s.str();
}
/*******************************
* 
* Implementation of TransitionMapToStr
*
*/
std::string TransitionMapToStr(const LrmTransitionMap& transitionMap){
  std::stringstream s;
  LrmTransitionMap::const_iterator transmapit;
  std::set<Lr1Configuration>::const_iterator configit;
  for(transmapit = transitionMap.begin(); transmapit != transitionMap.end(); transmapit++){
    s << "CONFIGURATION SET " << ConfigSetToStr(transmapit->first.first) << "\n";
    s << "  with symbol " << transmapit->first.second->Str() << ":\n";
    s << "  leads to " << ConfigSetToStr(transmapit->second) << "\n";
  }
  return s.str();
}

/*******************************
* 
* Implementation of Lr1ParserActionElement
*
*/  
bool Lr1ParserActionElement::operator<(const Lr1ParserActionElement& other) const{
  
  //compare nextTerminal
  if(mNextTerminal < other.NextTerminal()) return true;
  if(other.NextTerminal() < mNextTerminal) return false;
  
  //compare stateStack
  if (mStateStack.size() < other.StateStack().size()) return true; //also makes sure that it2 will not go out of bounds
  if (other.StateStack().size() < mStateStack.size()) return false;

  std::vector<Idx>::const_iterator it1, it2;
  it2 = other.StateStack().begin();
  for(it1 = mStateStack.begin(); it1 != mStateStack.end(); it1++){
    if(*it1 < *it2) return true;
    if(*it2 < *it1) return false;
    it2++;
  }
  return false;
}

std::string Lr1ParserActionElement::Str() const{
  std::stringstream s;
  std::vector<Idx>::const_iterator it;
  for(it = mStateStack.begin(); it != mStateStack.end(); it++){
    if (it == mStateStack.begin())
      s << "(" << *it;
    else
      s << ", " << *it;
  }
  s << " | " << mNextTerminal.Str() << ")";
  return s.str();
}

/*******************************
* 
* Implementation of Lr1ParserAction
*
*/

bool Lr1ParserAction::operator<(const Lr1ParserAction& other) const{
  if (mLhs < other.Lhs()) return true;
  if (other.Lhs() < mLhs) return false;
  if (mRhs < other.Rhs()) return true;
  return false;
}

std::string Lr1ParserAction::Str() const{
  std::stringstream s;
  s << mLhs.Str() << " -> " << mRhs.Str();
  if(mIsReduce){
    s << "  [" << mProduction.Str() << "]";
  }
  return s.str();
}

/*******************************
* 
* Implementation of Lr1Parser
*
*/  

bool Lr1Parser::SetStartState(Idx start){
  mStartState = start;
  return mNonterminals.insert(start).second;
}

bool Lr1Parser::InsFinalState(Idx final){
 // mFinalState = final;
	mFinalStates.insert(final);
  return mNonterminals.insert(final).second;
}

bool Lr1Parser::InsNonterminal(Idx nt){
  return mNonterminals.insert(nt).second;
}

bool Lr1Parser::InsTerminal(const Terminal& t){
  return mTerminals.insert(t).second;
}

bool Lr1Parser::InsAction(const Lr1ParserAction& action){
  return mActions.insert(action).second;
}

void Lr1Parser::SetAugSymbol(Terminal t){
  mAugSymbol = t;
}


std::string Lr1Parser::StrFinalStates() const{
  std::stringstream s; // TM 2017
  std::set<Idx>::const_iterator it;
  for(it=mFinalStates.begin();it != mFinalStates.end();it++){
    if(it != mFinalStates.begin()) s << ", ";
    s << *it;
  }  
  std::string res = s.str();
  return res;
}

std::string Lr1Parser::StrNonterminals() const{
  std::stringstream s; // TM 2017
  std::set<Idx>::const_iterator it;
  for(it=mNonterminals.begin();it != mNonterminals.end();it++){
    if(it != mNonterminals.begin()) s << ", ";
    s << *it;
  }  
  std::string res = s.str();
  return res;
}

std::string Lr1Parser::StrTerminals() const{
  std::stringstream s; // TM 2017
  std::set<Terminal>::const_iterator it;
  for(it=mTerminals.begin();it != mTerminals.end();it++){
    if(it != mTerminals.begin()) s << ", ";
    s << it->Str();
  }  
  std::string res = s.str();
  return res;
}

std::string Lr1Parser::StrActions(std::string separator) const{
  std::string s = "";
  std::set<Lr1ParserAction>::const_iterator it;
  for(it = mActions.begin(); it != mActions.end(); it++){
    if (it == mActions.begin())
      s += it->Str();
    else
      s += separator + it->Str();
  }
  return s;
}

std::string Lr1Parser::Str() const{
  std::stringstream s;
  s << "************************" << std::endl;
  s << "  nonterminals: " << Nonterminals().size() << std::endl;
  s << "  terminals:    " << Terminals().size() << std::endl;
  s << "  actions:      " << Actions().size() << std::endl;
  s << "  final states: " << FinalStates().size() << std::endl;
  s << "************************" << std::endl << std::endl;

  s << "start state: " << mStartState << std::endl;
 // s << "final state: " << mFinalState << std::endl;
  s << "final state: " << StrFinalStates() << std::endl;
  s << "augment symbol: " << mAugSymbol.Str() << std::endl;
  s << "nonterminals: " << StrNonterminals() << std::endl;
  s << "terminals: " << StrTerminals() << std::endl;
  s << "actions: \n" << StrActions("\n") << std::endl;
  return s.str();
}

} // namespace faudes

