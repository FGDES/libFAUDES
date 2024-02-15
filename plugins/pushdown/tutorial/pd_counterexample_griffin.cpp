/** @file pd_counterexample_griffin.cpp  

Synthesizes the minimal restrictive supervisor for a combination of plant and specification presented in the appendix of the technical report by Schneider and Schmuck (referenced at the webpage of this plugin). This is a counterexample to the results by Griffin (2008) as his algorithm returns a real subset of the supremal controllable sublanguage. Using our tool (i.e. this tutorial), the correct supervisor is synthesized.

To run the tutorial and both save the output to a file and display it in the terminal, use 
"./pd_counterexample_griffin ls -l 2>&1 | tee pd_counterexample_griffin.txt" 

@ingroup Tutorials 

@include pd_counterexample_griffin.cpp

*/

/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013 Sven Schneider,Anne-Kathrin Schmuck,Stefan Jacobi

*/

#include "libfaudes.h"
#include "pd_include.h"

// make the faudes namespace available to our program
using namespace faudes;

int main(){
  
  //***********************************************
  //create the specification, a pushdown generator
  //************************************************
  PushdownGenerator g;
   
  //insert states
  g.InsState("q0");
  g.InsState("q1");
  g.InsState("q2");

  //insert stack symbols
  Idx lambda = g.InsStackSymbol(FAUDES_PD_LAMBDA); //always use FAUDES_PD_LAMBDA for lambda!
  Idx dot = g.InsStackSymbol("dot");
  Idx square = g.SetStackBottom("square");
  
  //insert events
  g.InsControllableEvent("a");
  g.InsControllableEvent("b");
  g.InsUncontrollableEvent("u");
  //g.InsUncontrollableEvent(FAUDES_PD_LAMBDA); //here, we do not have lambda-transitions
  
  //build stack symbol vectors to be inserted as pop and push attributes of transitions
  std::vector<Idx> vLambda; //vLambda is the abbreviation for the empty string
  vLambda.push_back(lambda);

  std::vector<Idx> vSquare; //vSquare is the abbreviation for the string "Box" 
  vSquare.push_back(square);

  std::vector<Idx> vDot; //vDot is the abbreviation for the string "Dot" 
  vDot.push_back(dot);

  std::vector<Idx> vDotSquare; 	// vDotSquare is the abbreviation for the string "Dot Box" 
  vDotSquare.push_back(dot);   	// here the dot is pushed AFTER (popped BEFORE) the Box
  vDotSquare.push_back(square); 

  std::vector<Idx> vDotDot;	// vDotDot is the abbreviation for the string "Dot Dot"
  vDotDot.push_back(dot);  // here two dots are pushed (popped)
  vDotDot.push_back(dot);
  
  //insert transitions
  g.SetTransition("q0", "a", "q0",vSquare,vDotSquare);
  g.SetTransition("q0", "a", "q0",vDot,vDotDot);
  g.SetTransition("q0", "b", "q1",vDot,vLambda);
  g.SetTransition("q1", "u", "q2",vDot,vDot);
  
  //set state attributes
  g.SetInitState("q0"); // one initial state
  g.SetMarkedState("q0");//here all states are marked, as required by the algorithm of Griffin (2008)
  g.SetMarkedState("q1");
  g.SetMarkedState("q2");
  
  //*******************************
  //create the plant, a DFA
  //*******************************
  System s;
  
  s.InsState("p0");
  s.InsState("p1");
  s.InsState("p2");
  
  s.InsControllableEvent("a");
  s.InsControllableEvent("b");
  s.InsUncontrollableEvent("u");
  
  s.SetTransition("p0", "a", "p0");
  s.SetTransition("p0", "b", "p1");
  s.SetTransition("p1", "u", "p2");
  
  s.SetInitState("p0"); // one initial state
  s.SetMarkedState("p0");//here all states are marked, as required by the algorithm of Griffin (2008)
  s.SetMarkedState("p1");
  s.SetMarkedState("p2");

  //*******************************
  //print plant and specification
  //*******************************
  g.Write();
  s.Write();
  
  //*******************************
  //call synthesis algorithm
  //*******************************
  PushdownGenerator rPd;
  PushdownConstructController(g,s,rPd);
  
  //*******************************
  //print result
  //*******************************
  std::cout << "***************** RESULT *****************" << std::endl;
  rPd.Write();

  //*******************************
  //print words that are possible in the closed loop 
  //*******************************
  LangK test(rPd);
  test.FindLangK(50); //at most 50 are checked when generating words in the language
  test.PrintWords();
  
  return 0;
}

