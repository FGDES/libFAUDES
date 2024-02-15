/** @file pd_controller_test.cpp

 Tests the result of the synthesis algorithm


 @ingroup Tutorials

 @include pd_tutorial.cpp

 */

#include "libfaudes.h"

// make the faudes namespace available to our program
using namespace faudes;

/** Run the tutorial */
int main() {

	//*******************************
	//create the specification, a pushdown generator
	//*******************************
	PushdownGenerator g;

	//insert states
	g.InsState("s1");
	g.InsState("s2");
	g.InsState("s3");

	//insert stack symbols
	Idx lambda = g.InsStackSymbol(FAUDES_PD_LAMBDA); //always use FAUDES_PD_LAMBDA for empty stack symbol!
	Idx dot = g.InsStackSymbol("dot");
	Idx square = g.SetStackBottom("square");

	//insert events
	g.InsControllableEvent("a");
	g.InsEvent("b");
	g.InsUncontrollableEvent(FAUDES_PD_LAMBDA); //always use FAUDES_PD_LAMBDA for empty event!

	//build stack symbol vectors to be inserted as pop and push attributes of transitions
	std::vector<Idx> vLambda;
	vLambda.push_back(lambda);
	std::vector<Idx> vSquare;
	vSquare.push_back(square);
	std::vector<Idx> vDot;
	vDot.push_back(dot);
	std::vector<Idx> vDotSquare;
	vDotSquare.push_back(dot);
	vDotSquare.push_back(square);

	//insert transitions
	g.SetTransition("s1", "a", "s2", vSquare, vDotSquare);
	g.SetTransition("s2", "b", "s3", vDot, vDot);

	//set state attributes
	g.SetInitState("s1");
	g.SetMarkedState("s1");
	g.SetMarkedState("s2");
	g.SetMarkedState("s3");

	// Record test case
	FAUDES_TEST_DUMP("specification", g)

	//*******************************
	//create the plant, a DFA
	//*******************************
	System s;

	s.InsState("s1");
	s.InsState("s2");
	s.InsState("s3");

	s.InsControllableEvent("a");
	s.InsEvent("b");

	s.SetTransition("s1", "a", "s1");
	s.SetTransition("s1", "a", "s2");
	s.SetTransition("s2", "b", "s3");

	s.SetInitState("s1");
	s.SetMarkedState("s3");

	// Record test case
	FAUDES_TEST_DUMP("plant", s)

	//*******************************
	//print plant and specification
	//*******************************
	g.Write();
	s.Write();

	//*******************************
	//call synthesis algorithm
	//*******************************
	PushdownGenerator rPd;
	PushdownConstructController(g, s, rPd);

	//*******************************
	//print result
	//*******************************
	std::cout << "***************** RESULT *****************" << std::endl;
	rPd.Write();

	// Record test case
	FAUDES_TEST_DUMP("controller", rPd)

	//*******************************
	//print words that are possible in the closed loop (only "ab" expected)
	//*******************************
	LangK test(rPd);
	test.FindLangK(50);
	test.PrintWords();

	// Validate result
	FAUDES_TEST_DIFF()

	// done
	return 0;
}

