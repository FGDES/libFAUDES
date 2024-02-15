/** @file pd_tutorial.cpp

 Tutorial, example plugin. This tutorial demonstrates
 how to use pushdown generators.

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

	//print dot-file
	g.DotWrite("tmp_pd_tutorial_specification.dot");

	//*******************************
	//create the plant, a DFA
	//*******************************
	System s;

	s.InsState("s1");
	s.InsState("s2");
	s.InsState("s3");

	s.InsControllableEvent("a");
	s.InsEvent("b");

	s.SetTransition("s1", "a", "s2");
	s.SetTransition("s2", "a", "s2");
	s.SetTransition("s2", "b", "s3");

	s.SetInitState("s1");
	s.SetMarkedState("s3");

	//print dot-file
	s.DotWrite("tmp_pd_tutorial_plant.dot");

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

	//print controller
	rPd.DotWrite("tmp_pd_tutorial_controller.png");

	//*******************************
	//print result
	//*******************************
	std::cout << "***************** RESULT *****************" << std::endl;
	rPd.Write();


	//*******************************
	//print words that are possible in the closed loop (only "ab" expected)
	//*******************************
	LangK test(rPd);
	test.FindLangK(50);
	test.PrintWords();


	//*******************************
	//parse former constructed generators from dot-file
	//*******************************
	PushdownGenerator g2("data/graph/pd_tut_spec.dot");
	System s2 = SystemFromDot("data/graph/pd_tut_plant.dot");

	//call synthesis algorithm
	PushdownGenerator rPd2;
	PushdownConstructController(g2, s2, rPd2);

	//print controller graph
	rPd2.DotWrite("tmp__pd_tutorial_controllerFromDot.png");

	// done
	return 0;
}

