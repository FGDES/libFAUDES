/** @file pd_pushdown_V2.cpp  

Construct a controller from given files
@ingroup Tutorials 

@include pd_pushdown_V2.cpp

*/

/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/


#include "libfaudes.h"
#include "pd_include.h"


// make the faudes namespace available to our program
namespace faudes{


int main(std::string& arg1,std::string& arg2){

    std::cout << "********** Controller Construction ****************" << std::endl;
    std::cout << "********** pushdown_V2 ****************" << std::endl;


	System sys = SystemFromDot(arg1);
	PushdownGenerator pd(arg2);

	PushdownGenerator controller;

	PushdownConstructController(pd,sys,controller);

	controller.DotWrite("pushdown_V2_Controller.dot");

  return 0;
}

} // namespace

int main( int argc, char **argv ){

	if(argc < 3){
	    std::cout << "Missing arguments " << std::endl;
	    std::cout << "Use: pd_pushdown_V2 < dot-file DFA> <dot-file DPDA>" << std::endl;

		return 1;
	}
	static time_t starttime = time(0);

	std::string arg1 = argv[1];
	std::string arg2 = argv[2];
	faudes::main(arg1,arg2);

	int funTime = static_cast<int>(difftime(time(0),starttime));
    std::stringstream sfunTime;

    std::cout << "Finish: " << funTime/60 << "min " << funTime % 60 << "s \n";

  return 0;
}


