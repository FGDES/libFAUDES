/** gen2dot.cpp  Utility to convert gen files to dot files  */

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
	 
#include "corefaudes.h"

using namespace faudes;

// mini help
void usage(const std::string& msg="") {
  std::cerr << "gen2dot --- convert generators to DOT format (" << faudes::VersionString() << ")" << std::endl;
  if (msg != "") {
    std::cerr << "error: " << msg << std::endl;
    std::cerr << std::endl;
    exit(1);
  }
  std::cerr << std::endl;
  std::cerr << "usage:" << std::endl;
  std::cerr << "  gen2dot [-?] <gen-in> [<dot-out>]" << std::endl;
  std::cerr << std::endl;
  std::cerr << "with:" << std::endl;
  std::cerr << "  <gen-in>   main input file" << std::endl;
  std::cerr << "  <dot-out>  main output file" << std::endl;
  std::cerr << std::endl;
  std::cerr << "note: <dot-out> defaults <gen-in> with suffix substituted to \"dot\"" << std::endl;
  std::cerr << std::endl;
  exit(0);
}

// runner
int main(int argc, char *argv[]) {

  // config
  std::string genin;
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
    if(genin.empty()) {
      genin=argv[i];
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
  if(genin.empty())
    usage("no input file specified");

  // fix output file name
  if(dotout.empty()) {
    std::string basename = genin;
    if(basename.rfind(".gen") < basename.size()) 
      basename.resize(basename.rfind(".gen"));
    std::string dotout = basename+".dot";
  }
  
  // read gen file
  Generator g(genin);

  // write dot file
  g.DotWrite(dotout);

  return 0;
}
