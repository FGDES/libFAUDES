/** hao2gen.cpp  Utility to convert HAO files to .gen files  */

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*

This program uses the library cpphaofparser to parse HAO files and forwards
relevant content as libFAUDES token stream. The source code is derived from
the example command-line tool, that comes with cpphaofparser v0.99.2.

The cpphaofparser is authored/copyrighted by

Joachim Klein <klein@tcs.inf.tu-dresden.de>
David Mueller <david.mueller@tcs.inf.tu-dresden.de>

Original sources are redistributed with libFAUDES. We obtained our copy from

http://automata.tools/hoa/cpphoafparser

*/

#include <queue>
#include <iostream>
#include <fstream>

#include "cpphoafparser/consumer/hoa_intermediate_trace.hh"
#include "cpphoafparser/consumer/hoa_intermediate_resolve_aliases.hh"
#include "cpphoafparser/parser/hoa_parser.hh"
#include "faudesxmlout.hh"

#include "libfaudes.h"

using namespace cpphoafparser;


// mini help
void usage(std::string msg="") {
  std::cerr << "hoa2gen " << faudes::VersionString() << ": convert libFAUDES generators to HOA format" << std::endl;
  if (msg != "") {
    std::cerr << "error: " << msg << std::endl;
    std::cerr << std::endl;
    exit(1);
  }
  std::cerr << std::endl;
  std::cerr << "hoa2gen uses the cpphoafparser library, see  http://automata.tools/hoa" << std::endl;
  std::cerr << std::endl;
  std::cerr << "usage: hoa2gen [flags*] [-o <genout>] [-sym <symin>] [<hoafile>]" << std::endl;
  std::cerr << std::endl;
  std::cerr << "  -o <genout>   specify main output file (defaults to std out)" << std::endl;
  std::cerr << "  -s <symin>    specify symbol table file (defaults to no such)" << std::endl;
  std::cerr << "  <hoafile>     specify main input file (defaults to std in)" << std::endl;
  std::cerr << std::endl;
  std::cerr << "valid flags:" << std::endl;
  std::cerr << "  --resolve-aliases  resolve aliases" << std::endl;
  std::cerr << "  --trace            trace the function calls to HOAConsumer" << std::endl;
  exit(0);
}


// runner
int main(int argc, char* argv[]) {

  // options/args
  std::string hoain;
  std::string symin;
  std::string genout;
  bool resolve_aliases = false;
  bool trace = false;
  // primitive command line parser 
  for(int i=1; i<argc; i++) {
    std::string option(argv[i]);
    // option: output file
    if(option=="-o") {
      i++;
      if(i>=argc) usage("output file not specified");
      genout=argv[i];
      continue;
    }
    // option: symbol table
    if(option=="-s") {
      i++;
      if(i>=argc) usage("symbol file not specified");
      symin=argv[i];
      continue;
    }
    // option: help
    if((option=="-?") || (option=="--help")) {
      usage();
      continue;
    }
    // option: resolve aliases
    if(option=="--resolve-aliases") {
      resolve_aliases = true;
      continue;
    }
    // option: trace
    if(option=="--trace") {
      trace = true;
      continue;
    }
    // option: unknown
    if(option.c_str()[0]=='-') {
      usage("unknown option "+ option);
      continue;
    }
    // main input file
    if(i==argc-1) {
      hoain=argv[i];
      continue;
    }
    // fail
    usage("exactly one input file must be specified last" );
  }

  // set main input stream
  std::istream* shoain= &std::cin;
  std::filebuf fhoain;
  if(!hoain.empty()) {
    fhoain.open(hoain,std::ios::in);
    shoain= new std::istream(&fhoain);
  }

  // set output stream
  faudes::TokenWriter* tgenout=nullptr;  
  if(!genout.empty()) 
    tgenout=new faudes::TokenWriter(genout);
  else 
    tgenout=new faudes::TokenWriter(faudes::TokenWriter::Stdout);
  

  // configure parser
  HOAConsumer::ptr consumer(new HOAConsumerFaudes(*tgenout)); 
  if (resolve_aliases) 
    consumer.reset(new HOAIntermediateResolveAliases(consumer));
  if (trace) 
    consumer.reset(new HOAIntermediateTrace(consumer));

  // run parser
  try {
    HOAParser::parse(*shoain, consumer, true);
    return 0;
  } catch (HOAParserException& e) {
    std::cerr << e.what() << std::endl;
  } catch (HOAConsumerException& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 1;
}


