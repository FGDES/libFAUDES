/** @file pd_attributes.cpp  Attributes for pushdown automata */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_attributes.h"
namespace faudes {
  
/*******************************
 * 
 * Implementation of AttributePushdownState
 *
 */

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributePushdownState,AttributeFlags)


AttributePushdownState::~AttributePushdownState(){
  if(mpMerge != NULL){
    delete mpMerge;
  }
}

void AttributePushdownState::SetMerge(const MergeAbstract* pMerge){
  
  if(mpMerge != NULL){
    delete mpMerge;
    mpMerge = NULL;
  }
  
  //test subtype of pMerge and allocate memory according to subtype
  //MergeStates
  const MergeStates* ms = dynamic_cast<const MergeStates*>(pMerge);
  if(ms != NULL){
    try{
      mpMerge = new MergeStates(ms->States());
    }
    catch (std::bad_alloc& ba){
      std::cerr << "bad_alloc caught in AttributePushdownState::SetMerge at new MergeStates(ms->States()): " << ba.what() << std::endl;
    }
    return;
  }
  
  //MergeStateAnnotation
  const MergeStateAnnotation* msa = dynamic_cast<const MergeStateAnnotation*>(pMerge);
  if(msa != NULL){
    try{
      mpMerge = new MergeStateAnnotation(msa->State(), msa->Annotation());
    }
    catch (std::bad_alloc& ba){
      std::cerr << "bad_alloc caught in AttributePushdownState::SetMerge at new MergeStateAnnotation(msa->State(), msa->Annotation()): " << ba.what() << std::endl;
    }
    return;
  }
  
  //MergeStateSplit
  const MergeStateSplit* mss = dynamic_cast<const MergeStateSplit*>(pMerge);
  if(mss != NULL){
    try{
      if(mss->IsHead()){
        mpMerge = new MergeStateSplit(mss->State());
      }
      else{
        mpMerge = new MergeStateSplit(mss->State(), mss->Symbol());
      }
    }
    catch (std::bad_alloc& ba){
      std::cerr << "bad_alloc caught in AttributePushdownState::SetMerge at new MergeStateSplit(mss->Annotation(), mss->State(), mss->Symbol()): " << ba.what() << std::endl;
    }
    return;
  }
  
  //MergeTransition
  const MergeTransition* mt = dynamic_cast<const MergeTransition*>(pMerge);
  if(mt != NULL){
    try{
      mpMerge = new MergeTransition(mt->X1(), mt->Ev(), mt->X2(), mt->Pop(), mt->Push());
    }
    catch (std::bad_alloc& ba){
      std::cerr << "bad_alloc caught in AttributePushdownState::SetMerge at new MergeTransition(mt->X1(), mt->Ev(), mt->X2(), mt->Pop(), mt->Push()): " << ba.what() << std::endl;
    }
    return;
  }
  
  //MergeStateEvent
  const MergeStateEvent* mse = dynamic_cast<const MergeStateEvent*>(pMerge);
  if(mse != NULL){
    try{
      mpMerge = new MergeStateEvent(mse->State(), mse->Event());
    }
    catch (std::bad_alloc& ba){
      std::cerr << "bad_alloc caught in AttributePushdownState::SetMerge at new MergeTransition(mt->X1(), mt->Ev(), mt->X2(), mt->Pop(), mt->Push()): " << ba.what() << std::endl;
    }
    return;
  }
  
  //invalid reference - do nothing
  if(pMerge == NULL){
    return;
  }
  //should never get here
  std::stringstream errstr;
  errstr << "tried to set MergeAbstract with non-existent subtype " << typeid(*pMerge).name() << std::endl;
  throw Exception("AttributePushdownState::SetMerge()", errstr.str(), 1002);
  return;
}

// Assign my members
void AttributePushdownState::DoAssign(const AttributePushdownState& rSrcAttr) { 
  // call base (incl. virtual clear)
  AttributeFlags::DoAssign(rSrcAttr);
  // my additional members
  this->SetMerge(rSrcAttr.mpMerge);
  this->DfaState(rSrcAttr.DfaState());
  //*(mpMerge)=*(rSrcAttr.mpMerge);
}

// Equality
bool AttributePushdownState::DoEqual(const AttributePushdownState& rOther) const {
  // base
  if(!AttributeFlags::DoEqual(rOther)) return false;
  // my members
  if(mpMerge!=rOther.mpMerge) return false;
  // pass
  return true;
}


//DoWrite(rTw,rLabel,pContext);
void AttributePushdownState::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  (void) pContext;
  if(IsDefault()) return;
  AttributeFlags::DoWrite(rTw,"",pContext);
  std::string label=rLabel;
  if(label=="") label="State";
  FD_DC("AttributePushdownState(" << this << ")::DoWrite(tr): to section " << label);
  if(mpMerge != NULL)
    mpMerge->Write(rTw,"");//TODO only compile this for debugging
}


//DoRead(rTr,rLabel,pContext)
void AttributePushdownState::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  // call base first
  AttributeFlags::DoRead(rTr,"",pContext);
  // figure my section
  std::string label=rLabel;
  if(label=="") label="State";
  FD_DC("AttributePushdownState(" << this << ")::DoRead(tr): from section " << label);
  // clear my data
  mpMerge = NULL;
  // test my section
  Token token;
  rTr.Peek(token);
  if(!token.IsBegin())  return;
  if(token.StringValue()!=label) return;
  // read my section (can throw exceptions now)
  //mInvariant.Read(rTr,label); //TODO read of mpMerge may be not needed since its only intended for debugging
}
  
/*******************************
 * 
 * Implementation of AttributePushdownTransition
 *
 */

// faudes type std
FAUDES_TYPE_IMPLEMENTATION(Void,AttributePushdownTransition,AttributeVoid)


bool AttributePushdownTransition::ClrPopPush(const std::vector<Idx>& rPop, const std::vector<Idx>& rPush){
  
  std::pair<std::vector<Idx>,std::vector<Idx> > popPushPair;
  popPushPair.first = rPop;
  popPushPair.second = rPush;
  
  int i = mPopPush.erase(popPushPair);
  if(i != 0){
    return true;
  }
  return false;
}

// Assign my members
void AttributePushdownTransition::DoAssign(const AttributePushdownTransition& rSrcAttr) { 
  // call base (incl. virtual clear)
  AttributeVoid::DoAssign(rSrcAttr);
  // my additional members
  mPopPush=rSrcAttr.mPopPush;
}

// Equality
bool AttributePushdownTransition::DoEqual(const AttributePushdownTransition& rOther) const {
  // my members
  if(mPopPush!=rOther.mPopPush) return false;
  // pass
  return true;
}


//DoWrite(rTw,rLabel,pContext);
void AttributePushdownTransition::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  if(IsDefault()) return;
  std::string label=rLabel;
  FD_DC("AttributePushdownTransition(" << this << ")::DoWrite(tr): to section " << label);
  if(label=="") label="PopPushes";
  
  Token token;
  int oldcolumns = rTw.Columns();
  rTw.Columns(8);
  
  // begin section commands
  token.SetBegin(label);
  rTw << token;
  
  //iterate over all PopPushPairs
  PopPushSet::const_iterator setit;
  std::vector<Idx>::const_iterator popit, pushit;
  std::vector<Idx> pop, push;
  for(setit = mPopPush.begin(); setit != mPopPush.end(); setit++){
    
    token.SetBegin("PopPush");
    rTw << token;

    //write pop
    pop = setit->first;
    for(popit=pop.begin();popit != pop.end();popit++){
      
      token.SetBegin("Pop");
      token.InsAttributeString("name", StackSymbolSet::StaticSymbolTablep()->Symbol(*popit));
      rTw << token;
      
      token.SetEnd("Pop");
      rTw << token;
    }
    
    //write push
    push = setit->second;
    for(pushit=push.begin();pushit != push.end();pushit++){
      
      token.SetBegin("Push");
      token.InsAttributeString("name", StackSymbolSet::StaticSymbolTablep()->Symbol(*pushit));
      rTw << token;
      
      token.SetEnd("Push");
      rTw << token;
    }
    
    token.SetEnd("PopPush");
    rTw << token;
  }
  
    // commands
  token.SetEnd(label);
  rTw << token;
 
  rTw.Columns(oldcolumns);
}


//DoRead(rTr,rLabel,pContext)
void AttributePushdownTransition::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  // find my section
  std::string label=rLabel;
  if(label=="") label="PopPushes";
  FD_DC("AttributePushdownTransition(" << this << ")::DoRead(tr): from section " << label);
  // initialise my data
  mPopPush.clear();
  // test for  my data
  Token token;
  rTr.Peek(token);
  if(!token.IsBegin(label)) return;
  
  std::string symbol;
  std::vector<Idx> pop, push;
  // read my section
  rTr.ReadBegin(label);
  while(!rTr.Eos(label)) {
    // read token
    rTr.Peek(token);
    // skip other than command
    if(!token.IsBegin("PopPush")) {
       rTr.Get(token);
       continue;
    }
    
    // ok, its a command section
    rTr.ReadBegin("PopPush");
    
    pop.clear();
    push.clear();
    
    StackSymbolSet ssSet;
    while(rTr.Peek(token)){
      
      //read pop
      if(token.IsBegin("Pop")){
        
        rTr.ReadBegin("Pop");
        symbol = token.AttributeStringValue("name");
        ssSet.Insert(symbol); //for generating stack symbol indices
        pop.push_back(StackSymbolSet::StaticSymbolTablep()->Index(symbol));
        rTr.ReadEnd("Pop");
      }
      
      //read push
      else if(token.IsBegin("Push")){
        
        rTr.ReadBegin("Push");
        symbol = token.AttributeStringValue("name");
        ssSet.Insert(symbol); ssSet.Insert(symbol); //for generating stack symbol indices
        push.push_back(StackSymbolSet::StaticSymbolTablep()->Index(symbol));
        rTr.ReadEnd("Push");
      }
      
      //stop if end of section
      else if(token.IsEnd("PopPush")){
        
        //insert
        mPopPush.insert(std::make_pair(pop,push));
        break;
      }
    }
    
//     // end section command 
    rTr.ReadEnd("PopPush");
    // 3:
//     std::stringstream errstr;
//     errstr << "invalid transition timing" << rTr.FileLine();
//     throw Exception("AttributeTimedTrans::Read", errstr.str(), 52);
  }
  rTr.ReadEnd(label);
}

  
  /*******************************
  * 
  * Implementation of AttributePushdownGlobal
  *
  */

  // faudes type std
  FAUDES_TYPE_IMPLEMENTATION(Void,AttributePushdownGlobal,AttributeVoid)

  // Assign my members
  void AttributePushdownGlobal::DoAssign(const AttributePushdownGlobal& rSrcAttr) {
    FD_DG("AttributePushdownGlobal::DoAssign("<<this<<"): src " << &rSrcAttr << " type " << typeid(rSrcAttr).name());
    // call base (incl. virtual clear)
    AttributeVoid::DoAssign(rSrcAttr);
    // my additional members
     mStackSymbols=rSrcAttr.mStackSymbols;
     mpStackSymbolTable=rSrcAttr.mpStackSymbolTable;
     mStackBottom = rSrcAttr.mStackBottom;
  }

  // Equality
  bool AttributePushdownGlobal::DoEqual(const AttributePushdownGlobal& rOther) const {
    // my members
     if(mStackSymbols!=rOther.mStackSymbols) return false;
    // pass
    return true;
  }

  //DoWrite(rTw,rLabel,pContext);
  void AttributePushdownGlobal::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
    (void) pContext;
    if(IsDefault()) return;
    std::string label=rLabel;
    if(label=="") label="StackSymbols";
    FD_DC("AttributePushdownGlobal(" << this << ")::DoWrite(tr): to section " << label);
    mStackSymbols.XWrite(rTw,label);
    
    Token token;
    token.SetBegin("StackBottom");
    token.InsAttributeString("name", StackSymbolSet::StaticSymbolTablep()->Symbol(mStackBottom));
    rTw << token;
    
    token.SetEnd("StackBottom");
    rTw << token;
  }

  //DoRead(rTr,rLabel,pContext)
  void AttributePushdownGlobal::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
    std::string label=rLabel;
    if(label=="") label="StackSymbols";
    FD_DC("AttributePushdownGlobal(" << this << ")::DoRead(tr): from section " << label);
    (void) pContext;
    mStackSymbols.Clear();
    Token token;
    rTr.Peek(token);
    if(!token.IsBegin())  return;
    if(token.StringValue() == label){ 
      mStackSymbols.Read(rTr,label);
    }
    
    
    rTr.Peek(token);
    if(token.StringValue() == "StackBottom"){ 
      rTr.ReadBegin("StackBottom");
      std::string symbol = token.AttributeStringValue("name");
      StackSymbolSet ssSet;
      ssSet.Insert(symbol);
      mStackBottom = StackSymbolSet::StaticSymbolTablep()->Index(symbol);
      rTr.ReadEnd("StackBottom");
    }
  }



} // namespace faudes

