/** @file omg_hoa.cpp Serialisation in HOA format */

/* FAU Discrete Event Systems Library (libFAUDES)

Copyright (C) 2025 Thomas Moor
Exclusive copyright is granted to Klaus Schmidt

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

  

#include "omg_hoa.h"

#if __clang__
#define OMG_HOA_GCCMUTE
#endif
#ifdef OMG_HOA_GCCMUTE

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-private-field"
#pragma GCC diagnostic ignored "-Winconsistent-missing-override"
#endif

#include "cpphoafparser/consumer/hoa_intermediate_trace.hh"
#include "cpphoafparser/consumer/hoa_intermediate_resolve_aliases.hh"
#include "cpphoafparser/parser/hoa_parser.hh"
#include "cpphoafparser/parser/hoa_parser_helper.hh"
#include "cpphoafparser/parser/hoa_parser_exception.hh"
#include "cpphoafparser/util/implicit_edge_helper.hh"
#include "cpphoafparser/consumer/hoa_consumer.hh"
#include "cpphoafparser/consumer/hoa_consumer_exception.hh"

#ifdef OMG_HOA_GCCMUTE
#pragma GCC diagnostic pop
#endif

// local debug
//#undef FD_DF
//#define FD_DF(m) FD_WARN(m)

namespace faudes {

// helper function: convert bit vector to HOA boolean expression  
std::string omg_hoa_bits2expr(uint32_t bits, int apc) {
  std::string res;
  for(int i=0;i<apc;i++) {
    if(bits & (1L << i)) res+=faudes::ToStringInteger(i);
    else res+= "!"+faudes::ToStringInteger(i);
    if(i+1<apc) res+=" & ";
  }
  FD_DF("hoa_bits2expr: bits " << bits << " expr " << res);
  return res;
}

// write in HOA format
void omg_export_hoa(std::ostream& rOutStream, const Generator& rAut, SymbolTable* pSymTab){
  // inspectors
  EventSet::Iterator eit;
  EventSet::Iterator eit_end;
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  // figure type
  bool buechi=true;
  bool rabin=false;
  const RabinAutomaton* pRAut = dynamic_cast<const RabinAutomaton*>(&rAut);
  if(pRAut!=nullptr) {
    rabin=true;
    buechi=false;
  }
  // set up HOA style acceptance
  std::vector<StateSet> accvec;
  std::string accstr1;
  std::string accstr2;
  if(buechi) {
    accvec.push_back(rAut.MarkedStates());
    accstr1 = "acc-name: Buchi";
    accstr2 = "Acceptance: 1 Inf(0)";
  }
  if(rabin) {
    // we use J.Klen style from ltl2dstar: (Fin(0))&Inf(1)) | [...]
    const RabinAcceptance& acc=pRAut->RabinAcceptance();
    accstr1 = "acc-name: Rabin " + ToStringInteger(acc.Size());	  
    accstr2 = "Acceptance: " + ToStringInteger(2*acc.Size()) + " ";
    size_t i;
    for(i=0;i<acc.Size();++i) {
      if(i>0) accstr2 += "|";
      accstr2 += "(Fin(" + ToStringInteger(2*i) + ")&Inf("+ ToStringInteger(2*i+1) + "))";
      StateSet fin=rAut.States();
      fin.EraseSet(acc.At(i).ISet());
      accvec.push_back(fin);
      accvec.push_back(acc.At(i).RSet());
    }
  }
  // figure number of atomic propositions to log_2(#events)
  uint32_t asz=rAut.Alphabet().Size();
  int apc=1;
  for(;asz>2;asz=(asz>>1))
    apc++;
  // set up event mapping: faudes-idx -> [ consecutive integer starting from 0 , HOA expression ]
  std::map<Idx,uint32_t> ev2bits;
  std::map<Idx,std::string> ev2expr;
  uint32_t cnt=0;
  eit=rAut.AlphabetBegin();
  eit_end=rAut.AlphabetEnd();
  for(;eit!=eit_end;++eit) {
    ev2bits[*eit]=cnt;
    ev2expr[*eit]=omg_hoa_bits2expr(cnt,apc);
    cnt++;
  }
  // set up symbol table: [integer+1] -> [faudes name]
  if(pSymTab) {
    pSymTab->Clear();
    eit=rAut.AlphabetBegin();
    eit_end=rAut.AlphabetEnd();
    for(;eit!=eit_end;++eit) 
      pSymTab->InsEntry(ev2bits[*eit]+1,rAut.EventName(*eit));
  }
  // write HOA format: intro
  rOutStream << "HOA: v1" << std::endl;
  rOutStream << "name: \"" << rAut.Name() << "\""<< std::endl;
  // write HOA format: atomic propositions
  rOutStream << "AP: " << apc;
  for(int i=0; i<apc; ++i)
    rOutStream << " \"ap"<<i<<"\"";
  rOutStream  << std::endl;
  // write HOA format: event aliases 
  eit=rAut.AlphabetBegin();
  eit_end=rAut.AlphabetEnd();
  for(;eit!=eit_end;++eit) 
    rOutStream << "Alias: @" << rAut.EventName(*eit) << " " << ev2expr[*eit] << std::endl;
  // write HOA format: initial states
  rOutStream << "Start:";
  sit=rAut.InitStatesBegin();
  sit_end=rAut.InitStatesEnd();
  for(;sit!=sit_end;++sit) 
    rOutStream << " " << (*sit)-1;
  rOutStream << std::endl;
  // write HOA format: number of states
  rOutStream << "States: " << rAut.States().Size() << std::endl;
  // write HOA format: acceptance condition
  rOutStream << accstr1 << std::endl;
  rOutStream << accstr2 << std::endl;
  // write HOA format: graph structure
  rOutStream << "--BODY--" << std::endl;
  // iterate over all states
  sit=rAut.StatesBegin();
  sit_end=rAut.StatesEnd();
  for(;sit!=sit_end;++sit) {
    // state section
    rOutStream << "State: " << (*sit)-1;
    bool none=true;
    for(int i=0; i<accvec.size(); ++i) {
      if(!accvec[i].Exists(*sit)) continue;
      if(none)
	rOutStream << " {" + ToStringInteger(i);
      else 
	rOutStream << " " + ToStringInteger(i);
      none=false;
    }
    if(!none) rOutStream << "}";
    rOutStream << std::endl;
    // iterate over transitions from this state
    tit=rAut.TransRelBegin(*sit);
    tit_end=rAut.TransRelEnd(*sit);
    for(;tit!=tit_end;++tit)
      rOutStream << "[@" << rAut.EventName(tit->Ev) << "] " << tit->X2-1 << std::endl;
  }
  // end of graph
  rOutStream << "--END--" << std::endl;
}

// API wrapper
void ExportHoa(std::ostream& rOutStream, const Generator& rAut, SymbolTable* pSymTab){
  // refuse trivial
  if(rAut.States().Size() <1) {
    throw Exception("ExportHoa", "refusing to export generator with no states", 80);
  }
  if(rAut.Alphabet().Size() <1) {
    throw Exception("ExportHoa", "refusing to export generator with no events", 80);
  }
  // do export and catch ios errors
  try {
    omg_export_hoa(rOutStream,rAut,pSymTab);
  } catch (std::ios::failure&) {
    throw Exception("ExportHoa", "Exception writing to anonymous stream", 2);
  }
}

// API wrapper  
void ExportHoa(const std::string& rFilename, const Generator& rAut, SymbolTable* pSymTab){
  // refuse trivial
  if(rAut.States().Size() <1) {
    throw Exception("ExportHoa", "refusing to export generator with no states", 80);
  }
  if(rAut.Alphabet().Size() <1) {
    throw Exception("ExportHoa", "refusing to export generator with no events", 80);
  }
  // open file
  std::ofstream fout;
  fout.exceptions(std::ios::badbit|std::ios::failbit);
  try{
    fout.open(rFilename.c_str(), std::ios::out|std::ios::trunc); 
  }
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening file \""<< rFilename << "\"";
    throw Exception("ExportHoa", errstr.str(), 2);
  }
  // do export and catch ios exceptions
  try {
    omg_export_hoa(fout,rAut,pSymTab);
  } catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception writing to file \""<< rFilename << "\"";
    throw Exception("ExportHoa", errstr.str(), 2);
  }
}



// cpphoaparser comsumer to read to generator
using namespace cpphoafparser;
class HOAConsumerFaudes : public cpphoafparser::HOAConsumer {
public:
  /** Constructor, holding a reference to the generator to read to */
  HOAConsumerFaudes(Generator& gen, const SymbolTable& syms) :
    rGen(gen), rSymTab(syms), mImplEdgeHlp(0) {}
  // indicate to parser that aliases should not be resolved
  virtual bool parserResolvesAliases() override {
    return false;
  }
  // consume start of header
  virtual void notifyHeaderStart(const std::string& version) override {
    // should we the check version?
    rGen.Clear();
    pRAut = dynamic_cast<RabinAutomaton*>(&rGen);
  }
  // consume "States:"
  virtual void setNumberOfStates(unsigned int numberOfStates) override {
    for(unsigned int i=0;i<numberOfStates;++i) rGen.InsState(i+1);
  }
  // consume "Start:"
  virtual void addStartStates(const int_list& stateConjunction) override {
    for(unsigned int state : stateConjunction) 
       rGen.InsInitState(state+1);
  }
  // consume "Alias: @..."
  virtual void addAlias(const std::string& name, label_expr::ptr labelExpr) override {
    mAliases[name]=labelExpr;
  }
  // consume "AP: ... "
  virtual void setAPs(const std::vector<std::string>& aps) override {
    mApCount=aps.size();
    int i=0;
    for (const std::string& ap : aps) 
      mApSymbols[i++]=ap;
  }
  // consume "ACC: ..."
  virtual void setAcceptanceCondition(unsigned int numberOfSets, acceptance_expr::ptr accExpr) override {
    mAccSetCount=numberOfSets;
  }
  // consume "acc-name: ..."
  virtual void provideAcceptanceName(const std::string& name, const std::vector<IntOrString>& extraInfo) override {
    if(name=="Buchi") {
      mBuechi=true;
      return;
    }
    if(name=="Rabin") {
      if(pRAut==nullptr)
        throw HOAConsumerException("acc-name is Rabin, but only a plain Generator was passed");
      mRabin=true;
      return;
    }
  }
  // consume "Name: ..."
  virtual void setName(const std::string& name) override {
    rGen.Name(name);
  }
  // consume "tool: ..."
  virtual void setTool(const std::string& name, std::shared_ptr<std::string> version) override {
  }
  // consume "properties: ..."
  virtual void addProperties(const std::vector<std::string>& properties) override {
  }
  // consume misc
  virtual void addMiscHeader(const std::string& name, const std::vector<IntOrString>& content) override {
  }
  // start graph data
  virtual void notifyBodyStart() override {
    // test acceptance condition
    if(!(mRabin || mBuechi))
      throw HOAConsumerException("\"acc-name\" must specify Buchi or Rabin");
    // sanity test Rabin acceptance
    if(mRabin) {
      if(mAccSetCount % 2 != 0)
        throw HOAConsumerException("Rabin acceptance requires an even number of sets in \"Acceptance:\"");
      pRAut->RabinAcceptance().Size((mAccSetCount+1)/2);
    }
    // sanity test  Buechi acceptance
    if(mBuechi) {
      if(mAccSetCount!=1)
        throw HOAConsumerException("Buchi acceptance requires exactly on set in \"Acceptance:\"");
    }
    // initialise implicit edge lables
    mImplEdgeHlp = ImplicitEdgeHelper(mApCount);
    // set up alphabet: with explicit symbol table
    if(rSymTab.Size()!=0) {
      unsigned int evcnt= 1L << mApCount;
      for(uint32_t i=0;i<evcnt;++i) {
        if(!rSymTab.Exists(i+1)) continue;
	std::string evname=rSymTab.Symbol(i+1);
        mEdgeBitsToEvIdx[i]=rGen.InsEvent(evname);
      }
    }
    // set up alphabet: without explicit symbol table    
    if(rSymTab.Size()==0) {
      std::map<std::string,label_expr::ptr>::iterator ait;
      for(ait=mAliases.begin();ait!=mAliases.end();++ait) {
	int_list bitslist;
        expr2bits(ait->second,bitslist);
        if(bitslist.size()!=1) continue;
        mEdgeBitsToEvIdx[*bitslist.begin()]=rGen.InsEvent(ait->first);
      }
    }
  }
  // comsume "State: ..."
  virtual void addState(unsigned int id,
                        std::shared_ptr<std::string> info,
                        label_expr::ptr labelExpr,
                        std::shared_ptr<int_list> accSignature) override {
    if (labelExpr) {
      throw HOAConsumerException("state label expression not supported");
    }
    // have the state (redundant)
    rGen.InsState(id+1);
    // record to acceptance condition
    if (accSignature && mBuechi) {
      if(accSignature->size()>0)
	rGen.InsMarkedState(id+1);
    }
    // record to acceptance condition
    if (accSignature && mRabin) {
      for (unsigned int acc : *accSignature) {
	RabinPair& rpair = pRAut->RabinAcceptance().At(acc/2);
	bool rnoti = acc%2;
	if(rnoti) rpair.RSet().Insert(id+1);
	else rpair.ISet().Insert(id+1);
      }
    }
    // reset event index for implicit edges to come
    mImplEdgeHlp.startOfState(id);
  }
  // cosume implicit edge, i.e., conscutive labels
  virtual void addEdgeImplicit(
    unsigned int stateId,
    const int_list& conjSuccessors,
    std::shared_ptr<int_list> accSignature) override
  {
    if(accSignature) 
      throw HOAConsumerException("transition marking not supported");
    uint32_t edgebits = mImplEdgeHlp.nextImplicitEdge();
    std::map<uint32_t,Idx>::iterator eit;
    eit=mEdgeBitsToEvIdx.find(edgebits);
    Idx evidx;
    if(eit!=mEdgeBitsToEvIdx.end()) {
      evidx=eit->second;
    } else {
      std::string evname=bits2event(edgebits);
      evidx=rGen.InsEvent(evname);
      mEdgeBitsToEvIdx[edgebits]=evidx;
    }
    for (unsigned int succ : conjSuccessors) 
      rGen.SetTransition(stateId+1,evidx,succ+1);
  }
  // consume expilcit tarnsition
  virtual void addEdgeWithLabel(unsigned int stateId,
    label_expr::ptr labelExpr,
    const int_list& conjSuccessors,
    std::shared_ptr<int_list> accSignature) override
  {
    if(accSignature) 
      throw HOAConsumerException("transition marking not supported");
    int_list bitslist;
    expr2bits(labelExpr, bitslist);
    for (unsigned int edgebits : bitslist) {
      std::map<uint32_t,Idx>::iterator eit;
      eit=mEdgeBitsToEvIdx.find(edgebits);
      Idx evidx;
      if(eit!=mEdgeBitsToEvIdx.end()) {
        evidx=eit->second;
      } else {
        std::string evname=bits2event(edgebits);
        evidx=rGen.InsEvent(evname);
        mEdgeBitsToEvIdx[edgebits]=evidx;
      }
      for (unsigned int succ : conjSuccessors) 
        rGen.SetTransition(stateId+1,evidx,succ+1);
    }
  }
  // end of graph data
  virtual void notifyEndOfState(unsigned int stateId) override {
    mImplEdgeHlp.endOfState();
  }
  // end of body
  virtual void notifyEnd() override {
    // invert ISets
    if(mRabin) {
      RabinAcceptance::Iterator rit=pRAut->RabinAcceptance().Begin();
      for(;rit!=pRAut->RabinAcceptance().End();++rit) 
        rit->ISet()=pRAut->States() - rit->ISet();
    }
  }
  // some sort of parser error
  virtual void notifyAbort() override {
    rGen.Clear();
  }
  // some sort of parser warning
  virtual void notifyWarning(const std::string& warning) override {
    rGen.Clear();
  }

private:

  /** Payload: automaton to parse to */
  Generator& rGen;
  RabinAutomaton* pRAut=nullptr;
  const SymbolTable& rSymTab;

  /** Payload: intermediate parsing results */
  bool mRabin=false;
  bool mBuechi=false;
  unsigned int mAccSetCount=0;
  int mApCount=0;
  std::map<int,std::string> mApSymbols;
  std::map<std::string,label_expr::ptr> mAliases;
  ImplicitEdgeHelper mImplEdgeHlp;
  std::map<uint32_t,Idx> mEdgeBitsToEvIdx;
  
  /** bit vector to dummy faudes event name */
  std::string bits2event(uint32_t bits) {
    std::string res="ev";
    for(int i=mApCount-1;i>=0;--i) {
      if(bits & (1L << i)) res+="1";
      else res+= "0";
    }
    return res;
  }

  /** evaluate a label expression */
  bool evalexpr(label_expr::ptr expr, uint32_t bits) {
    switch (expr->getType()) {
    case label_expr::EXP_AND: 
      return evalexpr(expr->getLeft(),bits) && evalexpr(expr->getRight(),bits);
    case label_expr::EXP_OR: 
      return evalexpr(expr->getLeft(),bits) || evalexpr(expr->getRight(),bits);
    case label_expr::EXP_NOT: 
      return !evalexpr(expr->getLeft(),bits);
    case label_expr::EXP_TRUE: 
      return true;
    case label_expr::EXP_FALSE: 
      return false;
    case label_expr::EXP_ATOM:       
      if(!expr->getAtom().isAlias()) 
        return bits & (1UL << expr->getAtom().getAPIndex());
      std::map<std::string,label_expr::ptr>::iterator ait;
      ait=mAliases.find(expr->getAtom().getAliasName());
      if(ait==mAliases.end())
	throw HOAConsumerException("unreckonizes alias");
      return evalexpr(ait->second,bits);      
    }
    throw HOAConsumerException("could not evaluate label expression");
  }


  /** label expression to list of bit vectors */
  /** (we'ld need SAT solver or at least a hash/cache to do this efficiently) */
  void  expr2bits(label_expr::ptr labelExpr, HOAConsumer::int_list& bitslist) {
    uint32_t bits_sup = 1UL << mApCount;
    uint32_t bits=0;
    for(; bits<bits_sup;++bits)
      if(evalexpr(labelExpr,bits))
	 bitslist.push_back(bits);
  }    

};


// read from HOA formated tream
void ImportHoa(std::istream& rInStream, Generator& rGen, const SymbolTable* pSymTab, bool resolve, bool trace){
  // symboltable incl fallback
  static SymbolTable syms;
  const SymbolTable* psyms=&syms;
  if(pSymTab)
    psyms=pSymTab;  
  // configure parser
  HOAConsumer::ptr consumer(new HOAConsumerFaudes(rGen,*psyms));
  if(resolve)
    consumer.reset(new HOAIntermediateResolveAliases(consumer));
  if(trace)
   consumer.reset(new HOAIntermediateTrace(consumer));
  // run parser
  try {
    HOAParser::parse(rInStream, consumer, true);
  } catch (HOAParserException& e) {
    std::stringstream errstr;
    errstr << "ImportHOA parser: \""<< e.what() << "\"";
    throw Exception("ImportHoa", errstr.str(), 3);
  } catch (HOAConsumerException& e) {
    std::stringstream errstr;
    errstr << "ImportHOA consumer: \""<< e.what() << "\"";
    throw Exception("ImportHoa", errstr.str(), 3);
  }  
};

// API wrapper
void ImportHoa(const std::string& rFilename, Generator& rGen, const SymbolTable* pSymTab, bool resolve, bool trace) {
  // open file
  std::ifstream fin;
  fin.exceptions(std::ios::badbit); // dont throw on failbit because the hoa patrser will provoke that
  try{
    fin.open(rFilename.c_str(), std::ios::in | std::ios::binary); 
  }
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening file \""<< rFilename << "\"";
    throw Exception("ImportHoa", errstr.str(), 2);
  }
  // pass on
  ImportHoa(fin,rGen,pSymTab,resolve,trace);
}

} // namespace faudes

