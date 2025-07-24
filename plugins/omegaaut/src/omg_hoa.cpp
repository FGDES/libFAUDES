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
  FD_DF("hoa_bits2explr: bits " << bits << " expr " << res);
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
    accstr1 = "Acceptance: 1 Inf(0)";
    accstr2 = "acc-name: Buchi";
  }
  if(rabin) {
    // we use J.Klen style from ltl2dstar: (Fin(0))&Inf(1)) | etc
    const RabinAcceptance& acc=pRAut->RabinAcceptance();
    accstr1 = "Acceptance: " + ToStringInteger(acc.Size()) + " ";
    accstr2 = "acc-name: Rabin " + ToStringInteger(acc.Size());	  
    size_t i;
    for(i=0;i<acc.Size();++i) {
      if(i>0) accstr1 += "|";
      accstr1 += "(Fin(" + ToStringInteger(2*i) + ")&Inf("+ ToStringInteger(2*i+1) + "))";
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
    errstr << "Exception weriting to file \""<< rFilename << "\"";
    throw Exception("ExportHoa", errstr.str(), 2);
  }
}


// helper bit vector to faudes event name
std::string omg_hoa_bits2event(uint32_t bits, int apc) {
  std::string res="ev";;
  for(int i=apc-1;i>=0;--i) {
    if(bits & (1L << i)) res+="1";
    else res+= "0";
  }
  return res;
}

// cpphoaparser comsumer to read to generator
using namespace cpphoafparser;
class HOAConsumerFaudes : public cpphoafparser::HOAConsumer {
public:
  /** Constructor, holding a reference to the generator to read to */
  HOAConsumerFaudes(Generator& gen, const SymbolTable& syms) :
    rGen(gen), rSymTab(syms), mImplEdgeHlp(0) {}
  /** Constructs a HOAParserExeption to indicate an unsupported but presumably relevent feature */
  HOAParserException error(const std::string& msg) {
    std::stringstream ss;
    ss << "HOAConsumerFaudes: unsupported feature: " << msg;
    return HOAParserException(ss.str());
  }
  // Unsure about this feature (tmoor)
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
    throw error("aliases");
  }
  // consume "AP: ... "
  virtual void setAPs(const std::vector<std::string>& aps) override {
    // record the names
    int i=0;
    for (const std::string& ap : aps) 
      mApSymbols[i++]=ap;
    // set up alphabet
    mApCount=aps.size();
    unsigned int evcnt= 1L << mApCount;
    for(uint32_t i=0;i<evcnt;++i) {
      std::string evname=omg_hoa_bits2event(i,mApCount);
      if(rSymTab.Exists(i+1)) // todo: consider to mute unknown
        evname=rSymTab.Symbol(i+1);
      rGen.InsEvent(evname);
      mEdgeBitsToEvIdx[i]=rGen.EventIndex(evname);
    }
  }
  // consume "ACC: ..."
  virtual void setAcceptanceCondition(unsigned int numberOfSets, acceptance_expr::ptr accExpr) override {
    if(mRabin) {
      pRAut->RabinAcceptance().Size((numberOfSets+1)/2);
      // todo: check that this is in J. Klein style
      return;
    }
    if(mBuechi) {
      // todo: check for one set
      return;
    }
    throw error("ACC befor acc-name");
  }
  // consume "acc-name: ..."
  virtual void provideAcceptanceName(const std::string& name, const std::vector<IntOrString>& extraInfo) override {
    if(name=="Buchi") {
      mBuechi=true;
      return;
    }
    if(name=="Rabin") {
      if(pRAut==nullptr)
        throw error("acc-name is Rabin, buit only a plain Generator was passed");
      mRabin=true;
      return;
    }
    // can only deal with Buchi and Rabin
    throw error("acc-name must be Buchi or Rabin");
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
    mImplEdgeHlp = ImplicitEdgeHelper(mApCount);
  }
  // comsume "State: ..."
  virtual void addState(unsigned int id,
                        std::shared_ptr<std::string> info,
                        label_expr::ptr labelExpr,
                        std::shared_ptr<int_list> accSignature) override {
    if (labelExpr) {
      throw error("state label expression");
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
    if (accSignature) 
      throw error("transition marking");
    uint32_t edgebits = mImplEdgeHlp.nextImplicitEdge();
    Idx evindex=mEdgeBitsToEvIdx[edgebits]; // todo: guard this, e.g. mute unknown
    for (unsigned int succ : conjSuccessors) 
      rGen.SetTransition(stateId+1,evindex,succ+1);
  }
  // consume expilcit tarnsitio
  virtual void addEdgeWithLabel(unsigned int stateId,
    label_expr::ptr labelExpr,
    const int_list& conjSuccessors,
    std::shared_ptr<int_list> accSignature) override
  {
    throw error("explicit edge label");
    /*
    if (labelExpr) {
      std::cerr << "[" << *labelExpr << "] ";
    }
    */
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
    std::cerr << "HAO parser: abort" << std::endl;
    std::cerr.flush();
  }
  // some sort of parser warning
  virtual void notifyWarning(const std::string& warning) override {
    std::cerr << "HAO parser: Warning: " << warning << std::endl;
    std::cerr.flush();
  }

private:

  /** Payload: automaton to parse to */
  Generator& rGen;
  RabinAutomaton* pRAut=nullptr;
  const SymbolTable& rSymTab;

  /** Payload: intermediate parsing results */
  bool mRabin=false;
  bool mBuechi=false;
  int mApCount=0;
  std::map<int,std::string> mApSymbols;
  ImplicitEdgeHelper mImplEdgeHlp;
  std::map<uint32_t,Idx> mEdgeBitsToEvIdx;
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
    std::cerr << e.what() << std::endl; // todo: forward als faudes exception
  } catch (HOAConsumerException& e) {
    std::cerr << "Exception: " << e.what() << std::endl; // todo: forward als faudes exception
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

