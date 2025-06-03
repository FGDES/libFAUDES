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
  std::cerr << "gen2hao --- convert generators to HOA format (" << faudes::VersionString() << ")" << std::endl;
  if (msg != "") {
    std::cerr << "error: " << msg << std::endl;
    std::cerr << std::endl;
    exit(1);
  }
  std::cerr << std::endl;
  std::cerr << "usage:" << std::endl;
  std::cerr << "  gen2hoa [-s <sym-out>] [<gen-in> [<hoa-out>]] " << std::endl;
  std::cerr << std::endl;
  std::cerr << "with:" << std::endl;
  std::cerr << "  <gen-in>   main input file (defaults to std in)" << std::endl;
  std::cerr << "  <hoa-out>  main output file (defaults to std out)" << std::endl;
  std::cerr << "  <sym-out>  symbol table file (defaults to no such)" << std::endl;
  std::cerr << std::endl;
  exit(0);
}


// streams to operate on
TokenReader*  tgenin=nullptr;
std::ostream* shoaout=nullptr;
std::ostream* ssymout=nullptr;

// entry point
int main(int argc, char *argv[]) {

  // config
  std::string hoaout;
  std::string symout;
  std::string genin;

  // primitive command line parser 
  for(int i=1; i<argc; i++) {
    std::string option(argv[i]);
    // option: s
    if(option=="-s") {
      i++;
      if(i>=argc) usage("output file fro symbol table not specified");
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
    // argument #1 input file
    if(genin.empty()) {
      genin=argv[i];
      continue;
    }
    // argument #2 output file
    if(hoaout.empty()) {
      hoaout=argv[i];
      continue;
    }
    // fail
    usage("no more than two arguments must be specified" );
  }

  // set input stream
  if(!genin.empty()) {
    tgenin=new TokenReader(genin);
  } else {
    tgenin=new TokenReader(std::cin);
  }

  // sense generator type
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

  // output in HOA format
  if(hoaout.empty()) 
    ExportHoa(std::cout,gen);
  else
    ExportHoa(hoaout,gen);
  

  // tidy up
  delete tgenin;

  // done
  return 0;
}

