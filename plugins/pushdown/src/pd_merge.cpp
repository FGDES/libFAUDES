/** @file pd_merge.cpp  Attributes for pushdown automata states */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_merge.h"

namespace faudes {

/*******************************
* 
* Implementation of MergeStates
*
*/ 
  
MergeStates::MergeStates(const std::vector<Idx>& states) : mStates(states){
//MergeStates::MergeStates(std::vector<Idx> states, PushdownGenerator* gen){
  if(states.size() == 0){
    std::stringstream errstr;
    errstr << "MergeStates contains no states" << std::endl;
    throw Exception("MergeStates::MergeStates()", errstr.str(), 1001);
  }
  //mpGen = gen;
}


void MergeStates::Write(TokenWriter& rTw, const std::string& rLabel){
  std::string label=rLabel;
  if(label=="") label="MergeStates";
  FD_DC("MergeStates(" << this << ")::DoWrite(tr): to section " << label);
  
  std::vector<Idx>::iterator it;
  std::stringstream s;  // TM 2017
  s << "MergeStates: ";
  for(it=mStates.begin();it != mStates.end();it++){
    if(it != mStates.begin()) s << ", ";
    s << *it;
  }  
  rTw.WriteXmlComment(s.str());
  
//   Token token;
//   int oldcolumns = rTw.Columns();
//   //rTw.Columns(3);
  
//   rTw.WriteBegin(label);
//   
//   //write contained state
//   std::vector<Idx>::iterator it;
//   std::string s;
//   for(it=mStates.begin();it < mStates.end();it++){
//     if(it == mStates.begin())
//       s = static_cast<std::ostringstream*>( &(std::ostringstream() << *it) )->str();
//     else
//       s += ", " + static_cast<std::ostringstream*>( &(std::ostringstream() << *it) )->str();
//   }
//   token.SetOption(s);
//   rTw << token;
//   
//   //recursively write contained states of contained states
//   MergeAbstract* m;
//   for(it=mStates.begin();it < mStates.end();it++){
//     //mpGen->pStates;//doesn't compile
//     //m = mpGen->pStates->Attributep(*it)->mpMerge;
//     if(m != NULL){    
//       m->Write(rTw,"");
//     }
//   }
//   rTw.WriteEnd(label);
//   rTw.Columns(oldcolumns);
  
}

// void MergeStates::Set(std::vector<Idx> states){
//   if(states.size() == 0){
//     std::stringstream errstr;
//     errstr << "MergeStates contains no states" << std::endl;
//     throw Exception("MergeStates::Set()", errstr.str(), 1001);
//   }
//   mStates = states;
// }

/*******************************
* 
* Implementation of MergeStateAnnotation
*
*/ 

MergeStateAnnotation::MergeStateAnnotation(const Idx state, const std::string& annotation) : mState(state), mAnnotation(annotation){
  if(state < 1){
    std::stringstream errstr;
    errstr << "MergeStateAnnotation contains no state" << std::endl;
    throw Exception("MergeStateAnnotation::MergeStateAnnotation()", errstr.str(), 1001);
  }
}

void MergeStateAnnotation::Write(TokenWriter& rTw, const std::string& rLabel){
  std::string label=rLabel;
  if(label=="") label="MergeStateAnnotation";
  FD_DC("MergeStateAnnotation(" << this << ")::DoWrite(tr): to section " << label);
  std::stringstream s; // TM 2017
  s << "MergeStateAnnotation: " << mAnnotation << ", " << mState;
  rTw.WriteXmlComment(s.str());
//   Token t1, t2;
//   int oldcolumns = rTw.Columns();
//   //rTw.Columns(3);
//   
//   rTw.WriteBegin(label);
//   
//   //write contained state
//   std::string s = mAnnotation + ", ";
//   s = static_cast<std::ostringstream*>( &(std::ostringstream() << mState) )->str();
// 
//   token.SetOption(s);
//   rTw << token;
//   
//   rTw.WriteEnd(label);
//   rTw.Columns(oldcolumns);
//   
}

/*******************************
* 
* Implementation of MergeStateEvent
*
*/ 

MergeStateEvent::MergeStateEvent(const Idx state, const Idx event) : mState(state), mEvent(event){
  if(state < 1){
    std::stringstream errstr;
    errstr << "MergeStateEvent contains no state (state argument was " << state << ")"<< std::endl;
    throw Exception("MergeStateEvent::MergeStateEvent()", errstr.str(), 1001);
  }
}

void MergeStateEvent::Write(TokenWriter& rTw, const std::string& rLabel){
  std::string label=rLabel;
  if(label=="") label="MergeStateEvent";
  FD_DC("MergeStateEvent(" << this << ")::DoWrite(tr): to section " << label);
  
  std::stringstream s; // TM 2017
  s << "MergeStateEvent: state " << mState << ", event " << vGenerator::GlobalEventSymbolTablep()->Symbol(mEvent);
  rTw.WriteXmlComment(s.str());
//   Token t1, t2;
//   int oldcolumns = rTw.Columns();
//   //rTw.Columns(3);
//   
//   rTw.WriteBegin(label);
//   
//   //write contained state
//   std::string s = mAnnotation + ", ";
//   s = static_cast<std::ostringstream*>( &(std::ostringstream() << mState) )->str();
// 
//   token.SetOption(s);
//   rTw << token;
//   
//   rTw.WriteEnd(label);
//   rTw.Columns(oldcolumns);
//   
}

/*******************************
* 
* Implementation of MergeStateSplit
*
*/ 

MergeStateSplit::MergeStateSplit(const Idx state) : mState(state), mSymbol(0), mIsHead(true){
  if(state < 1){
    std::stringstream errstr;
    errstr << "MergeStateSplit contains no state" << std::endl;
    throw Exception("MergeStateSplit::MergeStateSplit()", errstr.str(), 1001);
  }
}

MergeStateSplit::MergeStateSplit(const Idx state, const Idx symbol) : mState(state), mSymbol(symbol), mIsHead(false){
  if(state < 1){
    std::stringstream errstr;
    errstr << "MergeStateSplit contains no state" << std::endl;
    throw Exception("MergeStateSplit::MergeStateSplit()", errstr.str(), 1001);
  }
}

void MergeStateSplit::Write(TokenWriter& rTw, const std::string& rLabel){
  std::string label=rLabel;
  if(label=="") label="MergeStateSplit";
  FD_DC("MergeStateSplit(" << this << ")::DoWrite(tr): to section " << label);
  
  std::stringstream annotation; // TM 2017
  if(mIsHead){
    annotation << "MergeStateSplit: head, " << mState;
  }
  else{
    annotation << "MergeStateSplit: ear, " << mState << ", " <<  StackSymbolSet::StaticSymbolTablep()->Symbol(mSymbol);
  }
  
  rTw.WriteXmlComment(annotation.str());
//   Token t1, t2;
//   int oldcolumns = rTw.Columns();
//   //rTw.Columns(3);
//   
//   rTw.WriteBegin(label);
//   
//   //write contained state
//   std::string s = mAnnotation + ", ";
//   s = static_cast<std::ostringstream*>( &(std::ostringstream() << mState) )->str();
// 
//   token.SetOption(s);
//   rTw << token;
//   
//   rTw.WriteEnd(label);
//   rTw.Columns(oldcolumns);
//   
}

/*******************************
* 
* Implementation of MergeTransition
*
*/ 

MergeTransition::MergeTransition(Idx x1, Idx ev, Idx x2,
		const std::vector<Idx>& pop, const std::vector<Idx>& push) : mX1(x1), mEv(ev), mX2(x2), mPop(pop), mPush(push)
{}

MergeTransition::MergeTransition(const Transition trans,
		const std::vector<Idx>& pop, const std::vector<Idx>& push) : mX1(trans.X1), mEv(trans.Ev), mX2(trans.X2), mPop(pop), mPush(push)
{}

void MergeTransition::Write(TokenWriter& rTw, const std::string& rLabel){
  std::string label=rLabel;
  if(label=="") label="MergeTransition";
  FD_DC("MergeTransition(" << this << ")::DoWrite(tr): to section " << label);
  
  std::stringstream annotation;
  std::vector<Idx>::const_iterator ssit;
  annotation << "MergeTransition: (" << mX1 << ", " << mEv << ", " << mX2 << ",";
  for(ssit = mPop.begin(); ssit != mPop.end(); ssit++){
    annotation << " " << StackSymbolSet::StaticSymbolTablep()->Symbol(*ssit);
  }
  annotation << ",";
  for(ssit = mPush.begin(); ssit != mPush.end(); ssit++){
    annotation << " " << StackSymbolSet::StaticSymbolTablep()->Symbol(*ssit);
  }
  annotation << std::endl;
  
  rTw.WriteXmlComment(annotation.str());
//   Token t1, t2;
//   int oldcolumns = rTw.Columns();
//   //rTw.Columns(3);
//   
//   rTw.WriteBegin(label);
//   
//   //write contained state
//   std::string s = mAnnotation + ", ";
//   s = static_cast<std::ostringstream*>( &(std::ostringstream() << mState) )->str();
// 
//   token.SetOption(s);
//   rTw << token;
//   
//   rTw.WriteEnd(label);
//   rTw.Columns(oldcolumns);
//   
}


} // namespace faudes

