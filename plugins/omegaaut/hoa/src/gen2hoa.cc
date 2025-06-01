/** gen2hoa.cpp  Utility to convert gen files to HOA files  */

/* FAU Discrete Event Systems Library (libfaudes)

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
	 
#include "libfaudes.h"

using namespace faudes;

// mini help
void usage(std::string msg="") {
  std::cerr << "gen2hoa " << VersionString() << ": convert libFAUDES generators to HOA format" << std::endl;
  if (msg != "") {
    std::cerr << "error: " << msg << std::endl;
    std::cerr << std::endl;
    exit(1);
  }
  std::cerr << "usage: gen2hoa [-o <hoaout>] [-s <symout>] [<generator>]" << std::endl;
  std::cerr << std::endl;
  std::cerr << "  -o <hoaout>   specify main output file (defaults to std out)" << std::endl;
  std::cerr << "  -o <symout>  specify symbol table file (defaults to no such)" << std::endl;
  std::cerr << "  <generator>   specify main input file (defaults to std in)" << std::endl;
  exit(0);
}

// helper: convert bits to boolean expression
std::string bits2expr(uint32_t bits, int apc) {
  std::string res;
  int i;
  for(i=0;i<apc;i++) {
    if(bits & (1L << i)) res+=faudes::ToStringInteger(i);
    else res+= "!"+faudes::ToStringInteger(i);
    if(i+1<apc) res+=" & ";
  }
  return res;
}

// streams to operate on
TokenReader*  tgenin=nullptr;
std::ostream* shoaout=nullptr;
std::ostream* ssymout=nullptr;

// runner
void runner(void) {
  // inspectors
  EventSet::Iterator eit;
  EventSet::Iterator eit_end;
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  // figure faudes type
  bool israbin=false;
  bool isbuechi=true;
  Token btag;
  tgenin->SeekBegin("Generator");
  tgenin->Peek(btag);
  if(btag.ExistsAttributeString("ftype")) {
    israbin=false;
    isbuechi=false;  
    std::string ftype= btag.AttributeStringValue("ftype");
    if(ftype == "RabinAutomaton") israbin=true; 
    if(ftype == "Generator") isbuechi=true;
  }
  if(!isbuechi){
    throw Exception("gen2hoa", "only buechi automata for the time being -- sorry", 80);
  }  
  if(israbin){
    throw Exception("gen2hoa", "only buechi automata for the time being -- sorry", 80);
  }  
  // have and read generator
  Generator gen;
  gen.Read(*tgenin);
  gen.MinStateIndex();
  // refuse trivial
  if(gen.States().Size() <1) {
    throw Exception("gen2hoa", "refusing to convert generator with no states", 80);
  }
  if(gen.Alphabet().Size() <1) {
    throw Exception("gen2hoa", "refusing to convert generator with no events", 80);
  }
  // write Buchi automaton in HOA format: intro
  (*shoaout) << "HOA: v1" << std::endl;
  (*shoaout) << "name: \"" << gen.Name() << "\""<< std::endl;
  // figure number of atomic propositions
  uint32_t asz=gen.Alphabet().Size();
  int apc=1;
  for(;asz>2;asz=(asz>>1))
    apc++;
  //FD_WARN("Atomic propositions: apc " << apc << " asz " << asz);
  (*shoaout) << "AP: " << apc;
  for(int i=0; i<apc; ++i)
    (*shoaout) << " \"ap"<<i<<"\"";
  (*shoaout)  << std::endl;
  // set up event mapping
  std::map<Idx,uint32_t> ev2bits;
  std::map<Idx,std::string> ev2expr;
  uint32_t cnt=0;
  eit=gen.AlphabetBegin();
  eit_end=gen.AlphabetEnd();
  for(;eit!=eit_end;++eit) {
    ev2bits[*eit]=cnt;
    ev2expr[*eit]=bits2expr(cnt,apc);
    cnt++;
  }
  // write event aliases
  eit=gen.AlphabetBegin();
  eit_end=gen.AlphabetEnd();
  for(;eit!=eit_end;++eit) {
    (*shoaout) << "Alias: @" << gen.EventName(*eit) << " " << ev2expr[*eit] << std::endl;
  }
  // write initial states
  (*shoaout) << "Start:";
  sit=gen.InitStatesBegin();
  sit_end=gen.InitStatesEnd();
  for(;sit!=sit_end;++sit) 
    (*shoaout) << " " << (*sit)-1;
  (*shoaout) << std::endl;
  // write number of states
  (*shoaout) << "States: " << gen.States().Size() << std::endl;
  // configure for  Buchi acceptance
  (*shoaout) << "Acceptance: 1 Inf(0)" << std::endl;
  (*shoaout) << "acc-name: Buchi" << std::endl;
  // do the graph structure
  (*shoaout) << "--BODY--" << std::endl;
  // iterate over all states
  sit=gen.StatesBegin();
  sit_end=gen.StatesEnd();
  for(;sit!=sit_end;++sit) {
    // state section
    (*shoaout) << "State: " << (*sit)-1;
    if(gen.ExistsMarkedState(*sit)) {
      (*shoaout) << " {0}";
    }
    (*shoaout) << std::endl;
    // iterate over transitions from this state
    tit=gen.TransRelBegin(*sit);
    tit_end=gen.TransRelEnd(*sit);
    for(;tit!=tit_end;++tit)
      (*shoaout) << "[@" << gen.EventName(tit->Ev) << "] " << tit->X2-1 << std::endl;
  }
  // end of graph
  (*shoaout) << "--END--" << std::endl;
}


// entry point
int main(int argc, char *argv[]) {

  // config
  std::string hoaout;
  std::string symout;
  std::string genin;

  // primitive command line parser 
  for(int i=1; i<argc; i++) {
    std::string option(argv[i]);
    // option: o
    if(option=="-o") {
      i++;
      if(i>=argc) usage("output file not specified");
      hoaout=argv[i];
      continue;
    }
    // option: o
    if(option=="-os") {
      i++;
      if(i>=argc) usage("output file not specified");
      symout=argv[i];
      continue;
    }
    // option: help
    if((option=="-?") || (option=="--help")) {
      usage();
      continue;
    }
    // option: unknown
    if(option.c_str()[0]=='-') {
      usage("unknown option "+ option);
      continue;
    }
    // argument input file
    if(i==argc-1) {
      genin=argv[i];
      continue;
    }
    // fail
    usage("input file must be specified last" );
  }

  // set input stream
  if(!genin.empty()) {
    tgenin=new TokenReader(genin);
  } else {
    tgenin=new TokenReader(std::cin);
  }

  // set main output stream
  shoaout= &std::cout;
  std::filebuf fhoaout;
  if(!hoaout.empty()) {
    fhoaout.open(hoaout,std::ios::out);
    shoaout= new std::ostream(&fhoaout);
  }
  
  // run
  runner();

  // tidy up
  delete tgenin;
  if(!hoaout.empty()) {
    delete shoaout;
    fhoaout.close();
  }  
  return 0;
}
