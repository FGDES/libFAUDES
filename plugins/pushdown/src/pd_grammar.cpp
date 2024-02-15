/** @file pd_merge.cpp  grammar data structure */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_grammar.h"

namespace faudes {

/*******************************
* 
* Implementation of GrammarSymbol
*
*/ 
bool GrammarSymbol::operator==(const GrammarSymbol& other) const{
  if (!(*this < other) && !(other < *this)) return true;
  return false;
}

bool GrammarSymbol::operator!=(const GrammarSymbol& other) const{
  if (!(*this < other) && !(other < *this)) return false;
  return true;
}

/*******************************
* 
* Implementation of Terminal
*
*/  

std::string Terminal::Str() const{
  return PushdownGenerator::GlobalEventSymbolTablep()->Symbol(mEvent);
}

bool Terminal::IsLambda() const{
  if (PushdownGenerator::GlobalEventSymbolTablep()->Symbol(mEvent).compare(FAUDES_PD_LAMBDA) == 0)
    return true;
  return false;
}

bool Terminal::operator<(const GrammarSymbol& other) const{
  if(typeid(*this) != typeid(other)) return false; //other is a nonterminal
  const Terminal& t = dynamic_cast<const Terminal&>(other); //other is a terminal
  return mEvent < t.Event(); 
}

Terminal* Terminal::Clone() const{
  
  Terminal* t = NULL;
  try{
    t = new Terminal(*this);
  }
  catch (std::bad_alloc& ba){
    std::cerr << "bad_alloc caught in Terminal::Clone new Clone(*this): " << ba.what() << std::endl;
  }
  return t;
}
  
/*******************************
* 
* Implementation of Nonterminal
*
*/ 
std::string Nonterminal::Str() const{
  std::stringstream s;
  std::vector<Idx>::const_iterator it;
  s << "(" <<  mStartState << ", [";
  for(it = mOnStack.begin(); it != mOnStack.end(); it++){
    if(it == mOnStack.begin()){
      s << PushdownGenerator::GlobalStackSymbolTablep()->Symbol(*it);
    }
    else{
      s << ", " << PushdownGenerator::GlobalStackSymbolTablep()->Symbol(*it);
    }
  }
  s << "]";
  if(mEndState != 0){
    s << ", " << mEndState; 
  }
  s << ")";
  return s.str();
}

bool Nonterminal::operator<(const GrammarSymbol& other) const{
  if(typeid(*this) != typeid(other)) return true; //other is a terminal
  const Nonterminal& nt = dynamic_cast<const Nonterminal&>(other); //other is a nonterminal
  if (mStartState < nt.StartState()) return true;
  if (mStartState > nt.StartState()) return false;
  
  if (mOnStack.size() < nt.OnStack().size()) return true; //also makes sure that it2 will not go out of bounds
  if (nt.OnStack().size() < mOnStack.size()) return false;
  
  std::vector<Idx>::const_iterator it1, it2;
  it2 = nt.OnStack().begin();
  for(it1 = mOnStack.begin(); it1 != mOnStack.end(); it1++){
    if(*it1 < *it2) return true;
    if(*it2 < *it1) return false;
    it2++;
  }
  
  if (mEndState < nt.EndState()) return true;
  return false;    
}

Nonterminal* Nonterminal::Clone() const{
  Nonterminal* nt = NULL;
  try{
    nt = new Nonterminal(*this);
  }
  catch (std::bad_alloc& ba){
    std::cerr << "bad_alloc caught in Nonterminal::Clone new Clone(*this): " << ba.what() << std::endl;
  }
  return nt;
}

/*******************************
* 
* Implementation of CompareGsVector
*
*/

bool CompareGsVector(const GrammarSymbolVector& lhs, const GrammarSymbolVector& rhs){
  if (lhs.size() < rhs.size()) return true; //also makes sure that it2 will not go out of bounds
  if (rhs.size() < lhs.size()) return false;
  
  GrammarSymbolVector::const_iterator it1, it2;
  it2 = rhs.begin();
  for(it1 = lhs.begin(); it1 != lhs.end(); it1++){
    if(**it1 < **it2) return true;
    if(**it2 < **it1) return false;
    it2++;
  }
  return false;
}

/*******************************
* 
* Implementation of EqualsGsVector
*
*/
bool EqualsGsVector(const GrammarSymbolVector& lhs, const GrammarSymbolVector& rhs){
  return(!CompareGsVector(lhs,rhs) && !CompareGsVector(rhs,lhs));
}

/*******************************
* 
* Implementation of CompareGs
*
*/

bool CompareGs(const GrammarSymbolPtr& lhs, const GrammarSymbolPtr& rhs){
  return *lhs < *rhs;
}

/* *************************
 * ContainsWord
 * *************************/
bool ContainsWord(const GrammarSymbolWordSet& set, const GrammarSymbolVector& word){
  
  GrammarSymbolWordSet::const_iterator setit;
  GrammarSymbolVector::const_iterator gsit1, gsit2;
  bool found = false;
  
  //iterate over the word set and look for the word
  for(setit = set.begin(); setit != set.end(); setit++){
    
    //sizes are the same so this word is a candidate
    if(setit->size() == word.size()){
      
      found = true;
      
      //compare the words' symbols
      gsit2 = word.begin();
      for(gsit1 = setit->begin(); gsit1 != setit->end(); gsit1++){
        
        //symbols were not the same, this is not the word we are looking for
        if(**gsit1 != **gsit2){
          found = false;
          break;
        }
        gsit2++;
      }
      //the word was found and no further search is necessary
      if(found){
        break;
      }
    }
  }
  return found;
}
/*******************************
* 
* Implementation of GrammarProduction
*
*/

std::string GrammarProduction::Str() const{
  std::stringstream s;
  s << mLhs.Str() << " --> ";
  GrammarSymbolVector::const_iterator it;
  for(it = mRhs.begin(); it < mRhs.end(); it++){
    s << (*it)->Str();
  }
  return s.str();;
}

bool GrammarProduction::operator<(const GrammarProduction& other) const{
  if (mLhs < other.Lhs()) return true;
  if (other.Lhs() < mLhs) return false;
  if(CompareGsVector(mRhs,other.Rhs())) return true;
//   if (mRhs.size() < other.Rhs().size()) return true; //also makes sure that it2 will not go out of bounds
//   if (other.Rhs().size() < mRhs.size()) return false;
//   
//   std::vector<GrammarSymbol*>::const_iterator it1, it2;
//   it2 = other.Rhs().begin();
//   for(it1 = mRhs.begin(); it1 != mRhs.end(); it1++){
//     if(**it1 < **it2) return true;
//     if(**it2 < **it1) return false;
//     it2++;
//   }
  return false;    
}


/*******************************
* 
* Implementation of Grammar
*
*/ 

bool Grammar::SetStartSymbol(const Nonterminal& s){
  mStartSymbol = s;
  return mNonterminals.insert(s).second;
}

bool Grammar::InsTerminal(const Terminal& t){
  return mTerminals.insert(t).second;
}

void Grammar::InsTerminals(const std::set<Terminal>& t){
  mTerminals.insert(t.begin(),t.end());
}

bool Grammar::InsNonterminal(const Nonterminal& nt){
  return mNonterminals.insert(nt).second;
}

void Grammar::InsNonterminals(const std::set<Nonterminal>& nt){
  mNonterminals.insert(nt.begin(),nt.end());
}

bool Grammar::InsGrammarProduction(const GrammarProduction& gp){
  
  //check lefthand side for validity
  if(mNonterminals.find(gp.Lhs()) == mNonterminals.end()) {
    std::stringstream errstr;
    errstr << "Grammar symbol mismatch: Lefthand-side nonterminal " << gp.Lhs().Str() << " does not exist in grammar." <<std::endl;
    throw Exception("Grammar::InsGrammarProduction", errstr.str(), 1001);
  }
  
  //check righthand side for validity
  GrammarSymbolVector::const_iterator gsit;
  for(gsit = gp.Rhs().begin(); gsit != gp.Rhs().end(); gsit ++){
    
    //check nonterminals
    ConstNonterminalPtr nt = std::dynamic_pointer_cast<const Nonterminal>(*gsit);
    if(nt){
      if(mNonterminals.find(*nt) == mNonterminals.end()) {
        std::stringstream errstr;
        errstr << "Grammar symbol mismatch: Righthand-side nonterminal " << nt->Str() << " does not exist in grammar." <<std::endl;
        throw Exception("Grammar::InsGrammarProduction", errstr.str(), 1001);
      }
    }
    //check terminals
    ConstTerminalPtr t = std::dynamic_pointer_cast<const Terminal>(*gsit);
    if(t){
      if(mTerminals.find(*t) == mTerminals.end()) {
        std::stringstream errstr;
        errstr << "Grammar symbol mismatch: Righthand-side terminal " << t->Str() << " does not exist in grammar." <<std::endl;
        throw Exception("Grammar::InsGrammarProduction", errstr.str(), 1001);
      }
    }
  }
  
  return mGrammarProductions.insert(gp).second;
}

void Grammar::InsGrammarProductions(const std::set<GrammarProduction>& gp){
  std::set<GrammarProduction>::const_iterator gpit;
  for(gpit = gp.begin(); gpit != gp.end(); gpit++){
    InsGrammarProduction(*gpit);
  }
}

std::set<Terminal>::const_iterator Grammar::TerminalsBegin() const{
  return mTerminals.begin();
}

std::set<Terminal>::const_iterator Grammar::TerminalsEnd() const{
  return mTerminals.end();
}

std::set<Nonterminal>::const_iterator Grammar::NonterminalsBegin() const{
  return mNonterminals.begin();
}

std::set<Nonterminal>::const_iterator Grammar::NonterminalsEnd() const{
  return mNonterminals.end();
}

std::set<GrammarProduction>::const_iterator Grammar::GrammarProductionsBegin() const{
  return mGrammarProductions.begin();
}

std::set<GrammarProduction>::const_iterator Grammar::GrammarProductionsEnd() const{
  return mGrammarProductions.end();
}

std::string Grammar::StrTerminals() const{
  std::string s;
  std::set<Terminal>::const_iterator it;
  it = mTerminals.end();
  for(it = mTerminals.begin(); it != mTerminals.end(); it++){
    if(it == mTerminals.begin()) s+= it->Str();
    else s+= ", " + it->Str();
  }
  return s;
}

std::string Grammar::StrNonterminals() const{
  std::string s;
  std::set<Nonterminal>::const_iterator it;
  it = mNonterminals.end();
  for(it = mNonterminals.begin(); it != mNonterminals.end(); it++){
    if(it == mNonterminals.begin()) s+= it->Str();
    else s+= ", " + it->Str();
  }
  return s;
}

std::string Grammar::StrStartSymbol() const{
  return mStartSymbol.Str();
}

std::string Grammar::StrGrammarProductions() const{
  std::string s;
  std::set<GrammarProduction>::const_iterator it;
  it = mGrammarProductions.end();
  for(it = mGrammarProductions.begin(); it != mGrammarProductions.end(); it++){
    s+= "\n     " + it->Str();
  }
  return s;
}

std::string Grammar::Str() const{
  std::string s;
  
  s += "start symbol: " + StrStartSymbol() + "\n";
  s += "nonterminals: " + StrNonterminals() + "\n";
  s += "terminals: " + StrTerminals() + "\n";
  s += "grammar productions: " + StrGrammarProductions() + "\n";
  
  return s;
  
}



} // namespace faudes

