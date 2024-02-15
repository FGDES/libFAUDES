/** @file pd_example_WODES14_nonblock

Makes an example DPDA nonblocking. This example is taken from the paper by Schneider and Nestman submitted to WODES'14, discussing the nonblock-algorithm implemented here.

// To save output to file and display it in shell, use 
// "./pd_example_WODES14_nonblock ls -l 2>&1 | tee pd_example_WODES14_nonblock.txt" 

@ingroup Tutorials 

@include pd_example_WODES14_nonblock.cpp

*/

/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Sven Schneider,Anne-Kathrin Schmuck,Stefan Jacobi

*/

#include "libfaudes.h"
#include "pd_include.h"

// make the faudes namespace available to our program
using namespace faudes;

int main(){
  
  //*****************************************************
  //create the input DPDA by using a pushdown generator
  //****************************************************
  PushdownGenerator g;
   
  //insert states
  g.InsState("q0");
  g.InsState("q1");
  g.InsState("q2");
  g.InsState("q3");
  g.InsState("q4");

  //insert stack symbols
  Idx lambda = g.InsStackSymbol(FAUDES_PD_LAMBDA); //always use FAUDES_PD_LAMBDA for lambda!
  Idx dot = g.InsStackSymbol("dot");
  Idx square = g.SetStackBottom("square");
  
  //insert events
  g.InsControllableEvent("a");
  g.InsControllableEvent("b");
  g.InsControllableEvent("d");
  g.InsEvent(FAUDES_PD_LAMBDA); //always use FAUDES_PD_LAMBDA for lambda-transitions!
  
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
  vDotDot.push_back(dot);  // here two dots are pushed (or popped)
  vDotDot.push_back(dot);
  
  //insert transitions
  g.SetTransition("q0", "a", "q0",vSquare,vDotSquare);
  g.SetTransition("q0", "a", "q0",vDot,vDotDot);
  g.SetTransition("q0", "b", "q1",vDot,vDot);
  g.SetTransition("q0", "b", "q1",vSquare,vSquare);
  g.SetTransition("q1", "b", "q2",vDot,vLambda);
  g.SetTransition("q2", "d", "q1",vDot,vLambda);
  g.SetTransition("q2", FAUDES_PD_LAMBDA , "q2",vSquare,vSquare);
  g.SetTransition("q1", FAUDES_PD_LAMBDA , "q3",vSquare,vDotSquare);
  g.SetTransition("q3", FAUDES_PD_LAMBDA , "q1",vSquare,vDotSquare);
  g.SetTransition("q3", "a", "q4",vSquare,vSquare);

  //set state attributes
  g.SetInitState("q0"); // one initial state
  g.SetMarkedState("q3");
  g.SetMarkedState("q4");

  //*******************************
  //print the DPDA
  //*******************************
  g.Write();
  
  //*******************************
  //call blockfree algorithm 
  //*******************************
  PushdownGenerator rPd;
  PushdownBlockfree(g,rPd);
  
  //*******************************
  //print resulting DPDA
  //*******************************
  std::cout << "***************** RESULT *****************" << std::endl;
  rPd.Write();

  //*******************************
  //print words that are possible in the closed loop 
  //*******************************
  LangK test(rPd);
  test.FindLangK(50);
  test.PrintWords();
  
  return 0;
}

