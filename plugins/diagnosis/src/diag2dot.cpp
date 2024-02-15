/** diag2dot.cpp  Utility to convert gen files to dot files  */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
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
#include <iostream>

using namespace faudes;

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "usage: diag2dot <generator>" << std::endl;
		exit(1);
	}

        // read gen file
 	Diagnoser g(argv[1]);

        // fix output file name
        std::string basename = argv[1];
        if (basename.rfind(".gen") < basename.size()) {
          basename.resize(basename.rfind(".gen"));
        }  
        std::string dotfilename = basename+".dot";

        // write dot file
	g.DotWrite(dotfilename);

	return 0;
}
