/** @file pd_debug.cpp Debug functions */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013/14  Ramon Barakat, Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_debug.h"

namespace faudes {

//count number of files
int fileCounter = 0;

void debug_printf(const std::string& msg, const PushdownGenerator& pd){
	if (PD_DEBUG_LEVEL > 0)
		std::cout << msg << std::endl;
	if (PD_DEBUG_LEVEL > 1)
		ToFile(pd, msg);
}

void debug_printf(const std::string& msg, const std::string& tex){
	if (PD_DEBUG_LEVEL > 0)
		std::cout << msg << std::endl;
	if (PD_DEBUG_LEVEL > 1 && tex != "")
		ToFile(tex, msg);
}


//***************************************************************
//   print files options
// **************************************************************

void ToFile(const PushdownGenerator& p, std::string name, bool printDOT, bool printPNG) {
		std::stringstream fileName;
		fileName << "pd." << std::setfill('0') << std::setw(5) << fileCounter << name;

		try {
			if ((PD_DEBUG_MAX_TRANS >= p.TransRelSize())) {
				if(printDOT||  printPNG){
					std::cout << "print file '" << name << "'...";

					if(printPNG)
						p.GraphWrite(fileName.str() + ".png");
					if(printDOT)
						p.DotWrite(fileName.str() + ".dot");

					std::cout << "done." << std::endl;
				}
			} else
				std::cout << "ToFile:Graph get more than " << PD_DEBUG_MAX_TRANS
						<< " Transitions !" << std::endl;

		} catch (faudes::Exception& exception) {
			std::cout << "ToFile: Graph error for '" << name << "'" << std::endl;
		}

		fileCounter++;
}

void ToFile(const std::string& tex, std::string name) {
		std::cout << "print file '" << name << "'...";
		std::stringstream fileName;
		fileName << "pd." << std::setfill('0') << std::setw(5) << fileCounter << name << ".tex";
		std::ofstream out(fileName.str().c_str());
		out << tex;

		std::cout << "done." << std::endl;

		fileCounter++;
}


}

