/** waut2dot.cpp  Utility to convert gen files to dot files  */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
   Copyright (C) 2025  Thomas Moor
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
void usage(const std::string& msg="") {
  std::cerr << "waut2dot --- convert omega automata to DOT format (" << faudes::VersionString() << ")" << std::endl;
  if (msg != "") {
    std::cerr << "error: " << msg << std::endl;
    std::cerr << std::endl;
    exit(1);
  }
  std::cerr << std::endl;
  std::cerr << "usage:" << std::endl;
  std::cerr << "  waut2dot [-?] <aut-in> [<dot-out>]" << std::endl;
  std::cerr << std::endl;
  std::cerr << "with:" << std::endl;
  std::cerr << "  <aut-in>   main input file" << std::endl;
  std::cerr << "  <dot-out>  main output file" << std::endl;
  std::cerr << std::endl;
  std::cerr << "note: <dot-out> defaults <aut-in> with suffix substituted to \"dot\"" << std::endl;
  std::cerr << "note: the current implementation is restricted to Rabin automata" << std::endl;
  std::cerr << std::endl;
  exit(0);
}

// runner
int main(int argc, char *argv[]) {

  // config
  std::string autin;
  std::string dotout;

  // primitive command line parser 
  for(int i=1; i<argc; i++) {
    std::string option(argv[i]);
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
    if(autin.empty()) {
      autin=argv[i];
      continue;
    }
    // argument #2 output file
    if(dotout.empty()) {
      dotout=argv[i];
      continue;
    }
    // fail
    usage("no more than two arguments must be specified" );
  }

  // fail on no input
  if(autin.empty())
    usage("no input file specified");

  // fix output file name
  if(dotout.empty()) {
    std::string basename = autin;
    if(basename.rfind(".gen") < basename.size()) 
      basename.resize(basename.rfind(".gen"));
    std::string dotout = basename+".dot";
  }
  
  // read gen file
  RabinAutomaton aut(autin);

  // write dot file
  aut.DotWrite(dotout);

  return 0;
}
