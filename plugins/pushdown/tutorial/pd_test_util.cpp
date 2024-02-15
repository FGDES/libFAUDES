/** @file pd_test_util.cpp  Creation of test classes */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/
#include "pd_test_util.h"

namespace faudes {

  
/* *****************
 * TestGenerator1
 * *****************/
PushdownGenerator TestGenerator1(){
  
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");
  g.InsState("s4");
  g.InsState("s5");
  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  

  g.InsEvent(FAUDES_PD_LAMBDA);
  g.InsControllableEvent("a");         
  g.InsControllableEvent("b");
  g.InsEvent("u");
  g.InsEvent("v");
  
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vSquareDot;
  vSquareDot.push_back(StackSymbol("square"));
  vSquareDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotDot;
  vDotDot.push_back(StackSymbol("dot"));
  vDotDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s1", "a", "s2",vLambda,vDotDot);
  g.SetTransition("s2", "a", "s3",vDotDot,vDot);  
  g.SetTransition("s3", "b", "s2",vDot,vDotDot);
  g.SetTransition("s1", "b", "s5",vLambda,vDotSquare);
  g.SetTransition("s5", "a", "s4",vSquare,vLambda);
  g.SetTransition("s4", "b", "s3",vLambda,vLambda);
  
  g.SetInitState("s1");              
  g.SetMarkedState("s3");
  g.SetMarkedState("s5");
  
  return g;
}

/* *****************
 * TestGenerator2
 * *****************/
PushdownGenerator TestGenerator2(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");
  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");

  g.InsEvent(FAUDES_PD_LAMBDA);
  g.InsControllableEvent("a");         
  g.InsEvent("b");
  g.InsEvent("d");
  
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));

  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDotDot;
  vDotDot.push_back(StackSymbol("dot"));
  vDotDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s1", "a", "s2",vLambda,vDotDot);
  g.SetTransition("s1", "b", "s2",vSquare,vDotSquare);  
  g.SetTransition("s2", FAUDES_PD_LAMBDA, "s3",vDotDot,vSquare);
  g.SetTransition("s3", "b", "s1",vSquare,vSquare);
  
  g.SetInitState("s1");              
  g.SetMarkedState("s2");
  
  return g;
}
/* *****************
 * TestGenerator3
 * *****************/
PushdownGenerator TestGenerator3(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");
  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.InsStackSymbol("square");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");         
  g.InsEvent("b");
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));

  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotDot;
  vDotDot.push_back(StackSymbol("dot"));
  vDotDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s1", "a", "s2",vDot,vDotDot);
  g.SetTransition("s1", "b", "s2",vSquare,vDotSquare);  
  g.SetTransition("s2", "a", "s3",vDot,vDot);
  g.SetTransition("s3", "b", "s1",vDot,vDot);
  
  g.SetInitState("s1");              
  g.SetMarkedState("s2");
  
  return g;
}

/* *****************
 * TestGenerator4
 * *****************/
PushdownGenerator TestGenerator4(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");
  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.InsStackSymbol("square");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");         
  g.InsEvent("b");
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotDot;
  vDotDot.push_back(StackSymbol("dot"));
  vDotDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotDotDot;
  vDotDotDot.push_back(StackSymbol("dot"));
  vDotDotDot.push_back(StackSymbol("dot"));
  vDotDotDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s1", "a", "s2",vDotDotDot,vDotDot);
  g.SetTransition("s1", "b", "s2",vDotSquare,vDotSquare);  
  g.SetTransition("s2", "a", "s3",vDotDot,vDot);
  g.SetTransition("s3", "b", "s1",vDot,vDot);
  
  g.SetInitState("s1");              
  g.SetMarkedState("s2");
  
  return g;
}

/* *****************
 * TestGenerator5
 * *****************/
PushdownGenerator TestGenerator5(){
    
  PushdownGenerator g;
  
  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  Idx square = g.InsStackSymbol("square");
  g.SetStackBottom("square");
  
  Idx s;
  //insert heads and ears
  s = g.InsState("s1");
  MergeStateSplit mss(1);
  g.SetMerge(s,mss);
  g.SetDfaState(s,1);
  
  s = g.InsState("s11");
  mss = MergeStateSplit(1,square);
  g.SetMerge(s,mss);
  g.SetDfaState(s,1);
  
  s = g.InsState("s2");
  mss = MergeStateSplit(2);
  g.SetMerge(s,mss);
  g.SetDfaState(s,2);
  
  s = g.InsState("s21");
  mss = MergeStateSplit(2,square);
  g.SetMerge(s,mss);
  g.SetDfaState(s,2);
  
  s = g.InsState("s3");
  mss = MergeStateSplit(3);
  g.SetMerge(s,mss);
  g.SetDfaState(s,1);
  
  s = g.InsState("s31");
  mss = MergeStateSplit(3,square);
  g.SetMerge(s,mss);
  g.SetDfaState(s,1);
  
  s = g.InsState("s4");
  mss = MergeStateSplit(4);
  g.SetMerge(s,mss);
  g.SetDfaState(s,2);
  
  s = g.InsState("s41");
  mss = MergeStateSplit(4,square);
  g.SetMerge(s,mss);
  g.SetDfaState(s,2);
  
  g.InsControllableEvent("a");         
  g.InsEvent("b");
  g.InsEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  
  //connect heads to ears
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s11",vSquare,vSquare);
  g.SetTransition("s2", FAUDES_PD_LAMBDA, "s21",vSquare,vSquare);
  g.SetTransition("s3", FAUDES_PD_LAMBDA, "s31",vSquare,vSquare);
  g.SetTransition("s4", FAUDES_PD_LAMBDA, "s41",vSquare,vSquare);
  //connect ears to head
  g.SetTransition("s11", "a", "s4",vSquare,vSquare);
  g.SetTransition("s11", "b", "s3",vSquare,vSquare);
  g.SetTransition("s21", "a", "s4",vSquare,vSquare);
  g.SetTransition("s21", "b", "s3",vSquare,vSquare);
  g.SetTransition("s31", "a", "s2",vSquare,vSquare);
  g.SetTransition("s41", "a", "s2",vSquare,vSquare);
  
  g.SetInitState("s1");              
  g.SetMarkedState("s21");
  g.SetMarkedState("s41");
  
  return g;
}

/* *****************
 * TestGenerator6
 * *****************/
PushdownGenerator TestGenerator6(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");
  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.InsStackSymbol("square");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");         
  g.InsEvent("b");
  g.InsEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vSquareDot;
  vSquareDot.push_back(StackSymbol("square"));
  vSquareDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vSquareDotDot;
  vSquareDotDot.push_back(StackSymbol("square"));
  vSquareDotDot.push_back(StackSymbol("dot"));
  vSquareDotDot.push_back(StackSymbol("dot"));
  
  g.SetTransition("s1", "a", "s2",vSquare,vSquare);
  g.SetTransition("s1", "b", "s2",vDot,vSquare);
  g.SetTransition("s2", FAUDES_PD_LAMBDA, "s3",vDot,vDot);
  g.SetTransition("s2", FAUDES_PD_LAMBDA, "s3",vDot,vSquareDotDot);
  g.SetTransition("s3", FAUDES_PD_LAMBDA, "s1",vSquare,vSquareDot);
  g.SetTransition("s3", FAUDES_PD_LAMBDA, "s1",vSquare,vLambda);
  
  g.SetInitState("s1");              
  g.SetMarkedState("s2");
  
  return g;
}

/* *****************
 * TestGenerator7
 * *****************/
PushdownGenerator TestGenerator7(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");
  g.InsState("s4");
  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.InsStackSymbol("square");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");         
  g.InsEvent("b");
  g.InsEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s1", "a", "s2",vLambda,vLambda);
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s3",vSquare,vDotSquare);
  g.SetTransition("s2", FAUDES_PD_LAMBDA, "s4",vSquare,vDotSquare);
  g.SetTransition("s3", FAUDES_PD_LAMBDA, "s2",vDot,vLambda);
  g.SetTransition("s4", FAUDES_PD_LAMBDA, "s3",vDot,vLambda);
  
  g.SetInitState("s1");              
  g.SetMarkedState("s2");
  g.SetMarkedState("s4");
  
  return g;
}

/* *****************
 * TestGenerator8
 * *****************/
PushdownGenerator TestGenerator8(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");         
  g.InsEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDotSquare;
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s2", "a", "s1",vDot,vDot);
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s2",vSquare,vDotSquare);
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s2",vDot,vLambda);
  
  g.SetInitState("s1");              
  g.SetMarkedState("s2");
  
  return g;
}

/* *****************
 * TestGenerator9
 * *****************/
PushdownGenerator TestGenerator9(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s1s");
  g.InsState("s1d");
  g.InsState("s2");
  g.InsState("s2d");
  g.InsState("s3");
  g.InsState("s4");
  g.InsState("s4s");
  g.InsState("s4d");
  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsControllableEvent("b");
  g.InsUncontrollableEvent("u");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vSquareDot;
  vSquareDot.push_back(StackSymbol("square"));
  vSquareDot.push_back(StackSymbol("dot"));
  
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s1d",vDot,vDot);
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s1s",vSquare,vSquare);
  g.SetTransition("s1s", "a", "s1",vSquare,vDotSquare);
  g.SetTransition("s1d", "b", "s2",vDot,vDot);
  g.SetTransition("s2", FAUDES_PD_LAMBDA, "s2d",vDot,vDot);
  g.SetTransition("s2d", "b", "s3",vDot,vLambda);
  g.SetTransition("s2d", "u", "s4",vDot,vDot);
  g.SetTransition("s4", FAUDES_PD_LAMBDA, "s4d",vDot,vDot);
  g.SetTransition("s4", FAUDES_PD_LAMBDA, "s4s",vSquare,vSquare);
  g.SetTransition("s4d", "u", "s4",vDot,vDot);
  g.SetTransition("s4s", "u", "s4",vSquare,vSquare);

  
  g.SetInitState("s1");
  g.SetMarkedState("s1s");
  g.SetMarkedState("s1d");
  g.SetMarkedState("s4s");
  g.SetMarkedState("s4d");
  
  return g;
}


/* *****************
 * TestGenerator10
 * *****************/
PushdownGenerator TestGenerator10(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s1s");
  g.InsState("s1d");
  //g.InsState("3");
  //g.InsState("4");

  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  //g.SetStackBottom("fullsquare");
  
  g.InsControllableEvent("a");

  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vFullsquare;
  vFullsquare.push_back(StackSymbol("fullsquare"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vSquareSquare;
  vSquareSquare.push_back(StackSymbol("square"));
  vSquareSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vSquareFullsquare;
  vSquareFullsquare.push_back(StackSymbol("square"));
  vSquareFullsquare.push_back(StackSymbol("fullsquare"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vSquareDot;
  vSquareDot.push_back(StackSymbol("square"));
  vSquareDot.push_back(StackSymbol("dot"));
  
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s1d",vDot,vDot);
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s1s",vSquare,vSquare);
  g.SetTransition("s1s", "a", "s1",vSquare,vDotSquare);
  //g.SetTransition("s1s", "a", "s1",vSquare,vDotSquare);
  //g.SetTransition("s1d", "b", "s2",vDot,vDot);
  
  //g.SetTransition("s1", FAUDES_PD_LAMBDA, "3",vSquare,vLambda);
  //g.SetTransition("3", FAUDES_PD_LAMBDA, "s1s",vFullsquare,vSquareFullsquare);
  //g.SetTransition("s1s", "a", "s1",vDot,vSquareDot);
  //g.SetTransition("4", FAUDES_PD_LAMBDA, "s1",vFullsquare,vSquareFullsquare);


  g.SetInitState("s1");
  g.SetMarkedState("s1s");
  g.SetMarkedState("s1d");

  
  return g;
}

/* *****************
 * TestGenerator11
 * *****************/
PushdownGenerator TestGenerator11(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s1s");
  g.InsState("s1d");
  g.InsState("s2");
  g.InsState("s2d");
  //g.InsState("s3");
  g.InsState("s4");
  g.InsState("s4s");
  g.InsState("s4d");
  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsControllableEvent("b");
  g.InsUncontrollableEvent("u");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vSquareDot;
  vSquareDot.push_back(StackSymbol("square"));
  vSquareDot.push_back(StackSymbol("dot"));
  
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s1d",vDot,vDot);
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s1s",vSquare,vSquare);
  g.SetTransition("s1s", "a", "s1",vSquare,vDotSquare);
  g.SetTransition("s1d", "b", "s2",vDot,vDot);
  g.SetTransition("s2", FAUDES_PD_LAMBDA, "s2d",vDot,vDot);
  //g.SetTransition("s2d", "b", "s3",vDot,vLambda);
  g.SetTransition("s2d", "u", "s4",vDot,vDot);
  g.SetTransition("s4", FAUDES_PD_LAMBDA, "s4d",vDot,vDot);
//   g.SetTransition("s4", FAUDES_PD_LAMBDA, "s4s",vSquare,vSquare);
//   g.SetTransition("s4d", "u", "s4",vDot,vDot);
//   g.SetTransition("s4s", "u", "s4",vSquare,vSquare);

  
  g.SetInitState("s1");
  g.SetMarkedState("s1s");
  g.SetMarkedState("s1d");
  g.SetMarkedState("s4s");
  g.SetMarkedState("s4d");
  
  return g;
}

/* *****************
 * TestGenerator12
 * *****************/
PushdownGenerator TestGenerator12(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");
  g.InsState("s4");

  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vSquareDot;
  vSquareDot.push_back(StackSymbol("square"));
  vSquareDot.push_back(StackSymbol("dot"));
  
  g.SetTransition("s1", "a", "s1",vSquare,vDotSquare);
  g.SetTransition("s1", "a", "s2",vDot,vDot);
  g.SetTransition("s2", "a", "s3",vSquare,vDotSquare);

  g.SetInitState("s1");
  g.SetMarkedState("s2");
  
  return g;
}

/* *****************
 * TestGenerator13
 * *****************/
PushdownGenerator TestGenerator13(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");
  g.InsState("s4");

  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsControllableEvent("b");
  g.InsControllableEvent("c");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vSquareDot;
  vSquareDot.push_back(StackSymbol("square"));
  vSquareDot.push_back(StackSymbol("dot"));
  
  g.SetTransition("s1", "a", "s1",vSquare,vDotSquare);
  g.SetTransition("s1", "a", "s2",vDot,vDot);
  g.SetTransition("s2", "b", "s2",vDot,vLambda);
  g.SetTransition("s2", "a", "s3",vSquare,vDotSquare);
  g.SetTransition("s2", "c", "s4",vLambda,vLambda);

  g.SetInitState("s1");
  g.SetMarkedState("s2");
  
  return g;
}

/* *****************
 * TestGenerator14
 * *****************/
PushdownGenerator TestGenerator14(){
    
  PushdownGenerator g;
  
  Idx s1 = g.InsState("s1");
  Idx s1s = g.InsState("s1s");
  Idx s1d = g.InsState("s1d");
  Idx s2 = g.InsState("s2");
  Idx s2s = g.InsState("s2s");
  Idx s2d = g.InsState("s2d");

  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  Idx dot = g.InsStackSymbol("dot");
  Idx square = g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s1s",vSquare,vSquare);
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s1d",vDot,vDot);
  g.SetTransition("s1s", "a", "s2",vSquare,vDotSquare);
  g.SetTransition("s2", FAUDES_PD_LAMBDA, "s2s",vSquare,vSquare);
  g.SetTransition("s2", FAUDES_PD_LAMBDA, "s2d",vDot,vDot);
  
  MergeStateSplit mss1(s1);
  g.SetMerge(s1,mss1);
  MergeStateSplit mss1s(s1s,square);
  g.SetMerge(s1s,mss1s);
  MergeStateSplit mss1d(s1d,dot);
  g.SetMerge(s1d,mss1d);
  MergeStateSplit mss2(s2);
  g.SetMerge(s2,mss2);
  MergeStateSplit mss2s(s2s,square);
  g.SetMerge(s2s,mss2s);
  MergeStateSplit mss2d(s2d,dot);
  g.SetMerge(s2d,mss2d);
  

  g.SetInitState("s1");
  g.SetMarkedState("s1d");
  g.SetMarkedState("s2d");
  
  return g;
}

/* *****************
 * TestGenerator15
 * *****************/
PushdownGenerator TestGenerator15(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");


  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsControllableEvent("b");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s1", "a", "s1",vDot,vDot);
  g.SetTransition("s2", "b", "s3",vSquare,vDotSquare);
  
  g.SetInitState("s1");
  g.SetMarkedState("s3");
  
  return g;
}

/* *****************
 * TestGenerator16
 * *****************/
PushdownGenerator TestGenerator16(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");


  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsControllableEvent("b");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s1", "a", "s1",vDot,vDot);
  g.SetTransition("s2", "b", "s3",vSquare,vDotSquare);
  g.SetTransition("s1", "a", "s1",vSquare,vDotSquare);
  g.SetTransition("s1", "b", "s2",vSquare,vSquare);
  
  g.SetInitState("s1");
  g.SetMarkedState("s3");
  
  return g;
}

/* *****************
 * TestGenerator17
 * *****************/
PushdownGenerator TestGenerator17(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");
  g.InsState("s4");
  g.InsState("s5");


  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsControllableEvent("b");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  
  g.SetTransition("s1", "a", "s1",vDot,vDot);
  g.SetTransition("s1", "b", "s2",vSquare,vSquare);
  g.SetTransition("s2", "b", "s3",vDot,vDot);
  g.SetTransition("s2", "a", "s4",vSquare,vSquare);
  
  g.SetInitState("s1");
  g.SetMarkedState("s3");
  g.SetMarkedState("s4");
  
  return g;
}

/* *****************
 * TestGenerator18
 * *****************/
PushdownGenerator TestGenerator18(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");


  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsEvent("b");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s1", "b", "s2",vSquare,vDotSquare);
  g.SetTransition("s2", "b", "s3",vDot,vDot);
  
  g.SetInitState("s1");
  g.SetMarkedState("s3");
  
  return g;
}

/* *****************
 * TestGenerator19
 * *****************/
PushdownGenerator TestGenerator19(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");


  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsEvent("b");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s1", "a", "s2",vSquare,vDotSquare);
  g.SetTransition("s2", "b", "s3",vDot,vDot);
  
  g.SetInitState("s1");
  g.SetMarkedState("s1");
  g.SetMarkedState("s2");
  g.SetMarkedState("s3");
  
  return g;
}

/* *****************
 * TestGenerator20
 * *****************/
PushdownGenerator TestGenerator20(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");
  g.InsState("s4");


  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsControllableEvent("b");
  g.InsEvent("u");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s1", "a", "s1",vSquare,vDotSquare);
  g.SetTransition("s1", "b", "s2",vDot,vDot);
  g.SetTransition("s2", "b", "s3",vDot,vLambda);
  g.SetTransition("s2", "u", "s4",vDot,vDot);
  g.SetTransition("s4", "u", "s4",vDot,vDot);
  
  g.SetInitState("s1");
  g.SetMarkedState("s3");
  g.SetMarkedState("s4");
  
  return g;
}

/* *****************
 * TestGenerator21
 * *****************/
PushdownGenerator TestGenerator21(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");
  g.InsState("s4");


  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.InsStackSymbol("dot");
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsControllableEvent("b");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotDot;
  vDotDot.push_back(StackSymbol("dot"));
  vDotDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s1", "a", "s2",vSquare,vDotSquare);
  g.SetTransition("s1", "a", "s2",vDot,vDotDot);
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s4",vSquare,vSquare);
  g.SetTransition("s2", "a", "s3",vSquare,vDotSquare);
  g.SetTransition("s2", "a", "s3",vDot,vDotDot);
  g.SetTransition("s2", "b", "s3",vLambda,vDot);
  g.SetTransition("s4", FAUDES_PD_LAMBDA, "s3",vSquare,vDotSquare);
  
  g.SetInitState("s1");
  g.SetMarkedState("s3");
  
  return g;
}

/* *****************
 * TestGenerator22
 * *****************/
PushdownGenerator TestGenerator22(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");


  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotDot;
  vDotDot.push_back(StackSymbol("dot"));
  vDotDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  
  g.SetTransition("s1", FAUDES_PD_LAMBDA, "s2",vSquare,vSquare);
  
  g.SetInitState("s1");
  g.SetMarkedState("s2");
  
  return g;
}

/* *****************
 * TestGenerator23
 * *****************/
PushdownGenerator TestGenerator23(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");

  Idx lambda = g.InsStackSymbol(FAUDES_PD_LAMBDA);
  Idx dot = g.InsStackSymbol("dot");
  Idx heart= g.InsStackSymbol("heart");
  Idx diamond = g.InsStackSymbol("diamond");
  Idx square = g.SetStackBottom("square");
  
  g.InsControllableEvent("a");
  g.InsControllableEvent("b");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<Idx> vLambda;
  vLambda.push_back(lambda);
  std::vector<Idx> vSquare;
  vSquare.push_back(square);
  std::vector<Idx> vDot;
  vDot.push_back(dot);
  std::vector<Idx> vDotDot;
  vDotDot.push_back(dot);
  vDotDot.push_back(dot);
  std::vector<Idx> vDiamond;
  vDiamond.push_back(diamond);
  std::vector<Idx> vHeart;
  vHeart.push_back(heart);
  
  g.SetTransition("s1", "b", "s1",vSquare,vSquare);
  g.SetTransition("s1", "a", "s2",vDot,vDotDot);
  g.SetTransition("s1", "b", "s2",vLambda,vLambda);
  g.SetTransition("s2", "a", "s2",vHeart,vHeart);
  g.SetTransition("s3", "a", "s1",vDiamond,vDiamond);
  g.SetTransition("s3", "a", "s3",vDiamond,vLambda);
  
  g.SetInitState("s1");
  g.SetMarkedState("s2");
  
  return g;
}

/* *****************
 * TestGenerator24
 * *****************/
PushdownGenerator TestGenerator24(){
    
  PushdownGenerator g;
  
  g.InsState("s0");
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");
  g.InsState("s4");
  g.InsState("s5");
  g.InsState("s6");
  g.InsState("s7");
  g.InsState("s8");
  
  
  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.SetStackBottom("square");
  g.InsStackSymbol("dot");
  
  g.InsControllableEvent("a");
  g.InsControllableEvent("b");
  g.InsUncontrollableEvent("u");
  g.InsUncontrollableEvent("v");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotDot;
  vDotDot.push_back(StackSymbol("dot"));
  vDotDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  Idx lambda = g.InsStackSymbol(FAUDES_PD_LAMBDA);
  
  g.SetTransition("s0", "a", "s0",vSquare,vDotSquare);
  g.SetTransition("s0", "a", "s0",vDot,vDotDot);
  g.SetTransition("s0", "b", "s1",vDot,vDot);
  g.SetTransition("s0", "b", "s8",vSquare,vSquare);
  g.SetTransition("s1", "b", "s2",vDot,vLambda);
  g.SetTransition("s1", "u", "s7",vDot,vDot);
  g.SetTransition("s1", "v", "s7",vDot,vDot);
  g.SetTransition("s2", "b", "s1",vDot,vLambda);
  g.SetTransition("s2", FAUDES_PD_LAMBDA, "s3",vSquare,vDot);
  g.SetTransition("s3", "b", "s3",vDot,vDotDot);
  g.SetTransition("s3", "u", "s4",vDot,vDotDot);
  g.SetTransition("s3", "v", "s5",vDot,vLambda);
  g.SetTransition("s3", "v", "s5",vSquare,vSquare);
  g.SetTransition("s4", FAUDES_PD_LAMBDA, "s6",vDot,vLambda);
  g.SetTransition("s5", FAUDES_PD_LAMBDA, "s5",vDot,vLambda);
  g.SetTransition("s5", "u", "s7",vSquare,vSquare);
  g.SetTransition("s6", FAUDES_PD_LAMBDA, "s4",vDot,vLambda);
  g.SetTransition("s6", "u", "s7",vSquare,vSquare);
  g.SetTransition("s6", "v", "s7",vSquare,vSquare);
  g.SetTransition("s7", "u", "s7",vDot,vDot);
  g.SetTransition("s7", "v", "s7",vDot,vDot);
  g.SetTransition("s7", "u", "s7",vSquare,vSquare);
  g.SetTransition("s7", "v", "s7",vSquare,vSquare);
  
  
  g.SetInitState("s0");
  g.SetMarkedState("s2");
  g.SetMarkedState("s5");
  g.SetMarkedState("s6");
  g.SetMarkedState("s7");
  g.SetMarkedState("s8");
  
  return g;
}

/* *****************
 * TestGenerator25
 * *****************/
PushdownGenerator TestGenerator25(){
    
  PushdownGenerator g;
  
  g.InsState("s1");
  g.InsState("s2");
  g.InsState("s3");
  g.InsState("s4");
  g.InsState("s5");
  
  g.InsStackSymbol(FAUDES_PD_LAMBDA);
  g.SetStackBottom("square");
  g.InsStackSymbol("dot");
  
  g.InsControllableEvent("a");
  g.InsControllableEvent("b");
  g.InsUncontrollableEvent("u");
  g.InsUncontrollableEvent("v");
  g.InsUncontrollableEvent(FAUDES_PD_LAMBDA);
  
  std::vector<StackSymbol> vLambda;
  vLambda.push_back(StackSymbol(FAUDES_PD_LAMBDA));
  std::vector<StackSymbol> vSquare;
  vSquare.push_back(StackSymbol("square"));
  std::vector<StackSymbol> vDot;
  vDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotDot;
  vDotDot.push_back(StackSymbol("dot"));
  vDotDot.push_back(StackSymbol("dot"));
  std::vector<StackSymbol> vDotSquare;
  vDotSquare.push_back(StackSymbol("dot"));
  vDotSquare.push_back(StackSymbol("square"));
  Idx lambda = g.InsStackSymbol(FAUDES_PD_LAMBDA);
  
  g.SetTransition("s1", "a", "s1",vSquare,vDotSquare);
  g.SetTransition("s1", "a", "s1",vDot,vDotDot);
  g.SetTransition("s1", "b", "s2",vDot,vLambda);
  g.SetTransition("s2", "b", "s2",vDot,vLambda);
  g.SetTransition("s2", "u", "s3",vDot,vDot);
  g.SetTransition("s2", "v", "s4",vDot,vLambda);
  g.SetTransition("s2", "v", "s4",vSquare,vSquare);
  g.SetTransition("s3", FAUDES_PD_LAMBDA, "s4",vDot,vLambda);
  g.SetTransition("s4", FAUDES_PD_LAMBDA, "s4",vDot,vLambda);
  g.SetTransition("s4", "u", "s5",vSquare,vSquare);
  g.SetTransition("s5", "u", "s5",vSquare,vSquare);
  
  g.SetInitState("s1");
  g.SetMarkedState("s5");
  
  return g;
}

/* *****************
 * TestSystem1
 * *****************/
System TestSystem1(){
    
  System s;
  
  s.InsState("s1");
  s.InsState("s2");
  s.InsState("s3");
  
  s.InsControllableEvent("a");         
  s.InsEvent("b");
  s.InsControllableEvent("c");
  
  s.SetTransition("s1", "a", "s2");
  s.SetTransition("s1", "b", "s3");
  s.SetTransition("s1", "c", "s3");
  s.SetTransition("s2", "b", "s1");
  s.SetTransition("s3", "a", "s2");
  
  s.SetInitState("s1");              
  s.SetMarkedState("s3");
  
  return s;
}

/* *****************
 * TestSystem2
 * *****************/
System TestSystem2(){
    
  System s;
  
  s.InsState("s1");
  s.InsState("s2");
  
  s.InsControllableEvent("a");         
  s.InsEvent("b");
  
  s.SetTransition("s1", "a", "s2");
  s.SetTransition("s1", "b", "s1");
  s.SetTransition("s2", "a", "s2");
  s.SetTransition("s2", "b", "s1");
  
  s.SetInitState("s1");              
  s.SetMarkedState("s2");
  
  return s;
}

/* *****************
 * TestSystem3
 * *****************/
System TestSystem3(){
    
  System s;
  
  s.InsState("s1");
  
  s.InsControllableEvent("a");         
  s.InsEvent("b");
  
  s.SetTransition("s1", "a", "s1");
  s.SetTransition("s1", "b", "s1");
  
  s.SetInitState("s1");              
  s.SetMarkedState("s1");
  
  return s;
}

/* *****************
 * TestSystem4
 * *****************/
System TestSystem4(){
    
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
  
  return s;
}

/* *****************
 * TestSystem5
 * *****************/
System TestSystem5(){
    
  System s;
  
  s.InsState("s1");
  s.InsState("s2");
  s.InsState("s3");
  
  s.InsControllableEvent("a");
  s.InsControllableEvent("b");
  s.InsEvent("u");
  //s.InsEvent("v");
  
  s.SetTransition("s1", "a", "s1");
  s.SetTransition("s1", "b", "s2");
  s.SetTransition("s2", "b", "s2");
  s.SetTransition("s2", "u", "s3");
  //s.SetTransition("s2", "v", "s3");
  s.SetTransition("s3", "u", "s3");
  //s.SetTransition("s3", "v", "s3");
  
  s.SetInitState("s1");              
  s.SetMarkedState("s2");
  s.SetMarkedState("s3");
  
  return s;
}

/* *****************
 * TestGrammar1
 * *****************/
Grammar TestGrammar1(){
  
  PushdownGenerator g1 = TestGenerator1();
  
  StackSymbolSet stackSymbols = g1.StackSymbols();
  
  std::vector<Idx> ssvDot;
  ssvDot.push_back(stackSymbols.Index("dot"));
  std::vector<Idx> ssvSquare;
  ssvSquare.push_back(stackSymbols.Index("square"));

  //construct Terminals
  Terminal* t1 = new Terminal(g1.Alphabet().Index("a"));
  GrammarSymbolPtr t1Ptr(t1);
  Terminal* t2 = new Terminal(g1.Alphabet().Index("b"));
  GrammarSymbolPtr t2Ptr(t2);
  
  //construct NonterminalEnd
  
  Nonterminal* nte1 = new Nonterminal(g1.StateIndex("s3"),ssvDot);
  GrammarSymbolPtr nte1Ptr(nte1);
  Nonterminal* nte2 = new Nonterminal(g1.StateIndex("s4"),ssvSquare);
  GrammarSymbolPtr nte2Ptr(nte2);
  
  //construct NonterminalMid
  Nonterminal* ntm3 = new Nonterminal(g1.StateIndex("s1"),ssvDot,g1.StateIndex("s2"));
  GrammarSymbolPtr ntm3Ptr(ntm3);
  Nonterminal* ntm4 = new Nonterminal(g1.StateIndex("s2"),ssvSquare,g1.StateIndex("s3"));
  GrammarSymbolPtr ntm4Ptr(ntm4);
  Nonterminal* ntm5 = new Nonterminal(g1.StateIndex("s5"),ssvSquare,g1.StateIndex("s1"));
  GrammarSymbolPtr ntm5Ptr(ntm5);
  
  //construct GrammarProduction
  GrammarSymbolVector v1;
  v1.push_back(nte2Ptr);
  v1.push_back(t2Ptr);
  GrammarProduction gp1 = GrammarProduction(*nte1,v1);
  GrammarSymbolVector v2;
  v2.push_back(t1Ptr);
  v2.push_back(ntm3Ptr);
  v2.push_back(t2Ptr);
  GrammarProduction gp2 = GrammarProduction(*nte2,v2);
  GrammarSymbolVector v3;
  v3.push_back(ntm4Ptr);
  GrammarProduction gp3 = GrammarProduction(*ntm3,v3);
  GrammarSymbolVector v4;
  v4.push_back(ntm5Ptr);
  GrammarProduction gp4 = GrammarProduction(*ntm3,v4);
  GrammarSymbolVector v5;
  v5.push_back(t2Ptr);
  GrammarProduction gp5 = GrammarProduction(*ntm5,v5);

  //construct Grammar
  Grammar gr = Grammar(*nte1);
  gr.InsTerminal(*t1);
  gr.InsTerminal(*t2);
  
  gr.InsNonterminal(*nte1);
  gr.InsNonterminal(*nte2);
  gr.InsNonterminal(*ntm3);
  gr.InsNonterminal(*ntm4);
  gr.InsNonterminal(*ntm5);
  
  gr.InsGrammarProduction(gp1);
  gr.InsGrammarProduction(gp2);
  gr.InsGrammarProduction(gp3);
  gr.InsGrammarProduction(gp4);
  gr.InsGrammarProduction(gp5);  
  
  return gr;
}

/* *****************
 * TestGrammar2
 * *****************/
Grammar TestGrammar2(){
  
  PushdownGenerator g1 = TestGenerator1();
  
  StackSymbolSet stackSymbols = g1.StackSymbols();

  std::vector<Idx> ssvDot;
  ssvDot.push_back(stackSymbols.Index("dot"));
  std::vector<Idx> ssvSquare;
  ssvSquare.push_back(stackSymbols.Index("square"));
  
  //construct Terminals
  Terminal* t1 = new Terminal(g1.Alphabet().Index("a"));
  GrammarSymbolPtr t1Ptr(t1);
  Terminal* t2 = new Terminal(g1.Alphabet().Index("b"));
  GrammarSymbolPtr t2Ptr(t2);
  
  //construct NonterminalEnd
  Nonterminal* nte1 = new Nonterminal(g1.StateIndex("s3"),ssvDot);
  GrammarSymbolPtr nte1Ptr(nte1);
  Nonterminal* nte2 = new Nonterminal(g1.StateIndex("s4"),ssvSquare);
  GrammarSymbolPtr nte2Ptr(nte2);
  
  //construct NonterminalMid
  Nonterminal* ntm3 = new Nonterminal(g1.StateIndex("s1"),ssvDot,g1.StateIndex("s2"));
  GrammarSymbolPtr ntm3Ptr(ntm3);
  Nonterminal* ntm4 = new Nonterminal(g1.StateIndex("s2"),ssvSquare,g1.StateIndex("s3"));
  GrammarSymbolPtr ntm4Ptr(ntm4);
  Nonterminal* ntm5 = new Nonterminal(g1.StateIndex("s5"),ssvSquare,g1.StateIndex("s1"));
  GrammarSymbolPtr ntm5Ptr(ntm5);
  
  //construct GrammarProduction
  GrammarSymbolVector v2;
  v2.push_back(t1Ptr);
  v2.push_back(ntm3Ptr);
  v2.push_back(t2Ptr);
  GrammarProduction gp2 = GrammarProduction(*nte2,v2);
  GrammarSymbolVector v3;
  v3.push_back(ntm4Ptr);
  GrammarProduction gp3 = GrammarProduction(*ntm3,v3);
  GrammarSymbolVector v4;
  v4.push_back(ntm5Ptr);
  GrammarProduction gp4 = GrammarProduction(*ntm3,v4);
  GrammarSymbolVector v5;
  v5.push_back(t2Ptr);
  GrammarProduction gp5 = GrammarProduction(*ntm5,v5);

  //construct Grammar
  Grammar gr = Grammar(*nte2);
  gr.InsTerminal(*t1);
  gr.InsTerminal(*t2);
  
  gr.InsNonterminal(*nte1);
  gr.InsNonterminal(*nte2);
  gr.InsNonterminal(*ntm3);
  gr.InsNonterminal(*ntm4);
  gr.InsNonterminal(*ntm5);
  
  gr.InsGrammarProduction(gp2);
  gr.InsGrammarProduction(gp3);
  gr.InsGrammarProduction(gp4);
  gr.InsGrammarProduction(gp5);  
  
  return gr;
}

/* *****************
 * TestGrammar3
 * *****************/
Grammar TestGrammar3(){
  
  PushdownGenerator g1 = TestGenerator1();
  
  StackSymbolSet stackSymbols = g1.StackSymbols();

  std::vector<Idx> ssvDot;
  ssvDot.push_back(stackSymbols.Index("dot"));
  std::vector<Idx> ssvSquare;
  ssvSquare.push_back(stackSymbols.Index("square"));
  
  //construct Terminals
  Terminal* ta = new Terminal(g1.Alphabet().Index("a"));
  GrammarSymbolPtr taPtr(ta);
  Terminal* tb = new Terminal(g1.Alphabet().Index("b"));
  GrammarSymbolPtr tbPtr(tb);
  Terminal* tlambda = new Terminal(g1.Alphabet().Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr tlambdaPtr(tlambda);
  
  //construct NonterminalEnd
  Nonterminal* nt1dot = new Nonterminal(1,ssvDot);
  GrammarSymbolPtr nt1dotPtr(nt1dot);
  Nonterminal* nt1square = new Nonterminal(1,ssvSquare);
  GrammarSymbolPtr nt1squarePtr(nt1square);
  Nonterminal* nt2square = new Nonterminal(2,ssvSquare);
  GrammarSymbolPtr nt2squarePtr(nt2square);
  
  //construct NonterminalMid
  Nonterminal* nt1dot1 = new Nonterminal(1,ssvDot,1);
  GrammarSymbolPtr nt1dot1Ptr(nt1dot1);
  Nonterminal* nt2square2 = new Nonterminal(2,ssvSquare,2);
  GrammarSymbolPtr nt2square2Ptr(nt2square2);
  
  //construct GrammarProduction
  GrammarSymbolVector v;
  v.push_back(taPtr);
  v.push_back(nt1dotPtr);
  GrammarProduction gp1 = GrammarProduction(*nt1dot,v);
  v.clear();
  v.push_back(tbPtr);
  v.push_back(nt2squarePtr);
  GrammarProduction gp2 = GrammarProduction(*nt1dot,v);
  v.clear();
  v.push_back(nt1dot1Ptr);
  v.push_back(nt2squarePtr);
  GrammarProduction gp3 = GrammarProduction(*nt2square,v);
  v.clear();
  v.push_back(tlambdaPtr);
  GrammarProduction gp4 = GrammarProduction(*nt2square,v);
  v.clear();
  v.push_back(nt1squarePtr);
  GrammarProduction gp5 = GrammarProduction(*nt1dot1,v);
  v.clear();
  v.push_back(nt1dotPtr);
  GrammarProduction gp6 = GrammarProduction(*nt1dot1,v);
  v.clear();
  v.push_back(taPtr);
  v.push_back(nt1dotPtr);
  GrammarProduction gp7 = GrammarProduction(*nt2square2,v);

  //construct Grammar
  Grammar gr = Grammar();
  gr.InsTerminal(*ta);
  gr.InsTerminal(*tb);
  gr.InsTerminal(*tlambda);
  
  gr.SetStartSymbol(*nt1dot);
  gr.InsNonterminal(*nt1square);
  gr.InsNonterminal(*nt2square);
  gr.InsNonterminal(*nt1dot1);
  gr.InsNonterminal(*nt2square2);
  
  gr.InsGrammarProduction(gp1);
  gr.InsGrammarProduction(gp2);
  gr.InsGrammarProduction(gp3);
  gr.InsGrammarProduction(gp4);
  gr.InsGrammarProduction(gp5);  
  gr.InsGrammarProduction(gp6);
  gr.InsGrammarProduction(gp7);
  
  return gr;
}

/* *****************
 * TestGrammar4
 * *****************/
Grammar TestGrammar4(){
  
  PushdownGenerator g1 = TestGenerator1();
  
  StackSymbolSet stackSymbols = g1.StackSymbols();

  std::vector<Idx> ssvDot;
  ssvDot.push_back(stackSymbols.Index("dot"));
  std::vector<Idx> ssvSquare;
  ssvSquare.push_back(stackSymbols.Index("square"));
  
  //construct Terminals
  Terminal* ta = new Terminal(g1.Alphabet().Index("a"));
  GrammarSymbolPtr taPtr(ta);
  Terminal* tlambda = new Terminal(g1.Alphabet().Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr tlambdaPtr(tlambda);
  
  //construct NonterminalEnd
  Nonterminal* nt1dot = new Nonterminal(1,ssvDot);
  GrammarSymbolPtr nt1dotPtr(nt1dot);
  Nonterminal* nt2square = new Nonterminal(2,ssvSquare);
  GrammarSymbolPtr nt2squarePtr(nt2square);
  
  //construct NonterminalMid
  Nonterminal* nt1dot1 = new Nonterminal(1,ssvDot,1);
  GrammarSymbolPtr nt1dot1Ptr(nt1dot1);
  
  //construct GrammarProduction
  GrammarSymbolVector v;
  v.push_back(taPtr);
  v.push_back(nt1dotPtr);
  GrammarProduction gp1 = GrammarProduction(*nt1dot,v);
  v.clear();
  v.push_back(nt1dot1Ptr);
  v.push_back(taPtr);
  v.push_back(nt2squarePtr);
  GrammarProduction gp2 = GrammarProduction(*nt2square,v);
  v.clear();
  v.push_back(tlambdaPtr);
  GrammarProduction gp3 = GrammarProduction(*nt2square,v);

  //construct Grammar
  Grammar gr = Grammar();
  gr.InsTerminal(*ta);
  gr.InsTerminal(*tlambda);
  
  gr.SetStartSymbol(*nt1dot);
  gr.InsNonterminal(*nt2square);
  gr.InsNonterminal(*nt1dot1);
  
  gr.InsGrammarProduction(gp1);
  gr.InsGrammarProduction(gp2);
  gr.InsGrammarProduction(gp3);
  
  return gr;
}

/* *****************
 * TestGrammar5
 * *****************/
Grammar TestGrammar5(){
  
  PushdownGenerator g1 = TestGenerator1();
  
  StackSymbolSet stackSymbols = g1.StackSymbols();

  std::vector<Idx> ssvDot;
  ssvDot.push_back(stackSymbols.Index("dot"));
  std::vector<Idx> ssvSquare;
  ssvSquare.push_back(stackSymbols.Index("square"));
  
  //construct Terminals
  Terminal* ta = new Terminal(g1.Alphabet().Index("a"));
  GrammarSymbolPtr taPtr(ta);
  Terminal* tb = new Terminal(g1.Alphabet().Index("b"));
  GrammarSymbolPtr tbPtr(tb);
  Terminal* tlambda = new Terminal(g1.Alphabet().Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr tlambdaPtr(tlambda);
  
  //construct NonterminalEnd
  Nonterminal* nt1dot = new Nonterminal(1,ssvDot);
  GrammarSymbolPtr nt1dotPtr(nt1dot);
  Nonterminal* nt1square = new Nonterminal(1,ssvSquare);
  GrammarSymbolPtr nt1squarePtr(nt1square);
  Nonterminal* nt2square = new Nonterminal(2,ssvSquare);
  GrammarSymbolPtr nt2squarePtr(nt2square);
  
  //construct NonterminalMid
  Nonterminal* nt1dot1 = new Nonterminal(1,ssvDot,1);
  GrammarSymbolPtr nt1dot1Ptr(nt1dot1);
  
  //construct GrammarProduction
  GrammarSymbolVector v;
  v.push_back(nt1dot1Ptr);
  v.push_back(taPtr);
  v.push_back(nt1squarePtr);
  GrammarProduction gp1 = GrammarProduction(*nt1dot,v);
  v.clear();
  v.push_back(tlambdaPtr);
  GrammarProduction gp2 = GrammarProduction(*nt1square,v);
  v.clear();
  v.push_back(tbPtr);
  GrammarProduction gp3 = GrammarProduction(*nt1square,v);
  v.clear();
  v.push_back(nt1dotPtr);
  GrammarProduction gp4 = GrammarProduction(*nt1dot1,v);
  v.clear();
  v.push_back(taPtr);
  v.push_back(nt1squarePtr);
  GrammarProduction gp5 = GrammarProduction(*nt1dot1,v);
  v.clear();
  v.push_back(nt1squarePtr);
  v.push_back(nt1dotPtr);
  GrammarProduction gp6 = GrammarProduction(*nt2square,v);

  //construct Grammar
  Grammar gr = Grammar();
  gr.InsTerminal(*ta);
  gr.InsTerminal(*tb);
  gr.InsTerminal(*tlambda);
  
  gr.SetStartSymbol(*nt1dot);
  gr.InsNonterminal(*nt1square);
  gr.InsNonterminal(*nt2square);
  gr.InsNonterminal(*nt1dot1);
  
  gr.InsGrammarProduction(gp1);
  gr.InsGrammarProduction(gp2);
  gr.InsGrammarProduction(gp3);
  gr.InsGrammarProduction(gp4);
  gr.InsGrammarProduction(gp5);
  gr.InsGrammarProduction(gp6);
  
  return gr;
}

/* *****************
 * TestGrammar6
 * *****************/
Grammar TestGrammar6(){
  
  PushdownGenerator g1 = TestGenerator1();
  
  StackSymbolSet stackSymbols = g1.StackSymbols();

  std::vector<Idx> ssvDot;
  ssvDot.push_back(stackSymbols.Index("dot"));
  std::vector<Idx> ssvSquare;
  ssvSquare.push_back(stackSymbols.Index("square"));
  
  //construct Terminals
  Terminal* ta = new Terminal(g1.Alphabet().Index("a"));
  GrammarSymbolPtr taPtr(ta);
  Terminal* tb = new Terminal(g1.Alphabet().Index("b"));
  GrammarSymbolPtr tbPtr(tb);
  Terminal* tlambda = new Terminal(g1.Alphabet().Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr tlambdaPtr(tlambda);
  
  //construct Nonterminals
  Nonterminal* nt1dot = new Nonterminal(1,ssvDot);
  GrammarSymbolPtr nt1dotPtr(nt1dot);
  Nonterminal* nt1dot1 = new Nonterminal(1,ssvDot,1);
  GrammarSymbolPtr nt1dot1Ptr(nt1dot1);
  Nonterminal* nt1square = new Nonterminal(1,ssvSquare);
  GrammarSymbolPtr nt1squarePtr(nt1square);
  Nonterminal* nt1square1 = new Nonterminal(1,ssvSquare,1);
  GrammarSymbolPtr nt1square1Ptr(nt1square1);

  //construct GrammarProduction
  GrammarSymbolVector v;
  v.push_back(nt1dot1Ptr);
  v.push_back(taPtr);
  GrammarProduction gp1 = GrammarProduction(*nt1dot,v);
  v.clear();
  v.push_back(tlambdaPtr);
  GrammarProduction gp2 = GrammarProduction(*nt1dot1,v);
  v.clear();
  v.push_back(tbPtr);
  v.push_back(nt1squarePtr);
  GrammarProduction gp3 = GrammarProduction(*nt1square1,v);
  
  //construct Grammar
   Grammar gr = Grammar();
  gr.InsTerminal(*ta);
  gr.InsTerminal(*tb);
  gr.InsTerminal(*tlambda);
  
  gr.SetStartSymbol(*nt1dot);
  gr.InsNonterminal(*nt1square);
  gr.InsNonterminal(*nt1square1);
  gr.InsNonterminal(*nt1dot1);
 
  gr.InsGrammarProduction(gp1);
  gr.InsGrammarProduction(gp2);
  gr.InsGrammarProduction(gp3);
  
  return gr;
}

/* *****************
 * TestGrammar7
 * *****************/
Grammar TestGrammar7(){
  
  PushdownGenerator g1 = TestGenerator1();
  
  StackSymbolSet stackSymbols = g1.StackSymbols();

  std::vector<Idx> ssvDot;
  ssvDot.push_back(stackSymbols.Index("dot"));
  std::vector<Idx> ssvSquare;
  ssvSquare.push_back(stackSymbols.Index("square"));
  
  g1.InsEvent("$");
  
  //construct Terminals
  Terminal* ta = new Terminal(g1.Alphabet().Index("a"));
  GrammarSymbolPtr taPtr(ta);
  Terminal* tb = new Terminal(g1.Alphabet().Index("b"));
  GrammarSymbolPtr tbPtr(tb);
  Terminal* tdollar = new Terminal(g1.Alphabet().Index("$"));
  GrammarSymbolPtr tdollarPtr(tdollar);
  Terminal* tlambda = new Terminal(g1.Alphabet().Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr tlambdaPtr(tlambda);
  
  //construct Nonterminals
  Nonterminal* nt1dot = new Nonterminal(1,ssvDot);
  GrammarSymbolPtr nt1dotPtr(nt1dot);
  Nonterminal* nt1square = new Nonterminal(1,ssvSquare);
  GrammarSymbolPtr nt1squarePtr(nt1square);

  //construct GrammarProduction
  GrammarSymbolVector v;
  v.push_back(tdollarPtr);
  v.push_back(nt1squarePtr);
  v.push_back(tdollarPtr);
  GrammarProduction gp1 = GrammarProduction(*nt1dot,v);
  v.clear();
  v.push_back(taPtr);
  v.push_back(nt1squarePtr);
  GrammarProduction gp2 = GrammarProduction(*nt1square,v);
  v.clear();
  v.push_back(tbPtr);
  GrammarProduction gp3 = GrammarProduction(*nt1square,v);
  
  //construct Grammar
   Grammar gr = Grammar();
  gr.InsTerminal(*ta);
  gr.InsTerminal(*tb);
  gr.InsTerminal(*tdollar);
  gr.InsTerminal(*tlambda);
  
  gr.SetStartSymbol(*nt1dot);
  gr.InsNonterminal(*nt1square);
 
  gr.InsGrammarProduction(gp1);
  gr.InsGrammarProduction(gp2);
  gr.InsGrammarProduction(gp3);
  
  return gr;
}

/* *****************
 * TestGrammar8
 * *****************/
Grammar TestGrammar8(){
  
  PushdownGenerator g1 = TestGenerator1();
  
  StackSymbolSet stackSymbols = g1.StackSymbols();

  std::vector<Idx> ssvDot;
  ssvDot.push_back(stackSymbols.Index("dot"));
  std::vector<Idx> ssvSquare;
  ssvSquare.push_back(stackSymbols.Index("square"));
  
  g1.InsEvent("$");
  g1.InsEvent("d");
  g1.InsEvent("e");
  
  //construct Terminals
  Terminal* ta = new Terminal(g1.Alphabet().Index("a"));
  GrammarSymbolPtr taPtr(ta);
  Terminal* tb = new Terminal(g1.Alphabet().Index("b"));
  GrammarSymbolPtr tbPtr(tb);
  Terminal* td = new Terminal(g1.Alphabet().Index("d"));
  GrammarSymbolPtr tdPtr(td);
  Terminal* te = new Terminal(g1.Alphabet().Index("e"));
  GrammarSymbolPtr tePtr(te);
  Terminal* tlambda = new Terminal(g1.Alphabet().Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr tlambdaPtr(tlambda);
  Terminal* tdollar = new Terminal(g1.Alphabet().Index("$"));
  GrammarSymbolPtr tdollarPtr(tdollar);
  
  //construct Nonterminals
  Nonterminal* nt1dot = new Nonterminal(1,ssvDot);
  GrammarSymbolPtr nt1dotPtr(nt1dot);
  Nonterminal* nt1square = new Nonterminal(1,ssvSquare);
  GrammarSymbolPtr nt1squarePtr(nt1square);
  Nonterminal* nt2dot = new Nonterminal(2,ssvDot);
  GrammarSymbolPtr nt2dotPtr(nt2dot);
  Nonterminal* nt2square = new Nonterminal(2,ssvSquare);
  GrammarSymbolPtr nt2squarePtr(nt2square);
  Nonterminal* nt1dot1 = new Nonterminal(1,ssvDot,1);
  GrammarSymbolPtr nt1dot1Ptr(nt1dot1);

  //construct GrammarProduction
  GrammarSymbolVector v;
  v.push_back(tdollarPtr);
  v.push_back(nt1squarePtr);
  v.push_back(tdollarPtr);
  GrammarProduction gp1 = GrammarProduction(*nt1dot,v);
  
  v.clear();
  v.push_back(nt1dot1Ptr);
  GrammarProduction gp2 = GrammarProduction(*nt1square,v);
  
  v.clear();
  v.push_back(nt2dotPtr);
  GrammarProduction gp3 = GrammarProduction(*nt1square,v);
  
  v.clear();
  v.push_back(tlambdaPtr);
  GrammarProduction gp4 = GrammarProduction(*nt1dot1,v);
  
  v.clear();
  v.push_back(taPtr);
  GrammarProduction gp5 = GrammarProduction(*nt2dot,v);
  
  v.clear();
  v.push_back(tbPtr);
  v.push_back(nt1squarePtr);
  v.push_back(nt2squarePtr);
  v.push_back(tePtr);
  GrammarProduction gp6 = GrammarProduction(*nt2dot,v);
  
  v.clear();
  v.push_back(tlambdaPtr);
  GrammarProduction gp7 = GrammarProduction(*nt2square,v);
  
  v.clear();
  v.push_back(tdPtr);
  v.push_back(nt1squarePtr);
  v.push_back(nt2squarePtr);
  GrammarProduction gp8 = GrammarProduction(*nt2square,v);
  
  //construct Grammar
   Grammar gr = Grammar();
  gr.InsTerminal(*ta);
  gr.InsTerminal(*tb);
  gr.InsTerminal(*td);
  gr.InsTerminal(*te);
  gr.InsTerminal(*tdollar);
  gr.InsTerminal(*tlambda);
  
  gr.SetStartSymbol(*nt1dot);
  gr.InsNonterminal(*nt1square);
  gr.InsNonterminal(*nt2dot);
  gr.InsNonterminal(*nt2square);
  gr.InsNonterminal(*nt1dot1);
 
  gr.InsGrammarProduction(gp1);
  gr.InsGrammarProduction(gp2);
  gr.InsGrammarProduction(gp3);
  gr.InsGrammarProduction(gp4);
  gr.InsGrammarProduction(gp5);
  gr.InsGrammarProduction(gp6);
  gr.InsGrammarProduction(gp7);
  gr.InsGrammarProduction(gp8);
  
  return gr;
}

/* *****************
 * TestGrammar9
 * *****************/
Grammar TestGrammar9(){
  
  PushdownGenerator g1 = TestGenerator1();
  
  StackSymbolSet stackSymbols = g1.StackSymbols();

  std::vector<Idx> ssvSquare;
  ssvSquare.push_back(stackSymbols.Index("square"));
  
  //construct Terminals
  Terminal* ta = new Terminal(g1.Alphabet().Index("a"));
  GrammarSymbolPtr taPtr(ta);
  Terminal* tb = new Terminal(g1.Alphabet().Index("b"));
  GrammarSymbolPtr tbPtr(tb);
  Terminal* tlambda = new Terminal(g1.Alphabet().Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr tlambdaPtr(tlambda);
  
  //construct Nonterminals
  Nonterminal* nt1square = new Nonterminal(1,ssvSquare);
  GrammarSymbolPtr nt1squarePtr(nt1square);

  //construct GrammarProduction
  GrammarSymbolVector v;
  v.push_back(taPtr);
  v.push_back(nt1squarePtr);
  GrammarProduction gp1 = GrammarProduction(*nt1square,v);
  v.clear();
  v.push_back(tbPtr);
  GrammarProduction gp2 = GrammarProduction(*nt1square,v);
  
  //construct Grammar
  Grammar gr = Grammar();
  gr.InsTerminal(*ta);
  gr.InsTerminal(*tb);
  gr.InsTerminal(*tlambda);
  
  gr.SetStartSymbol(*nt1square);
 
  gr.InsGrammarProduction(gp1);
  gr.InsGrammarProduction(gp2);
  
  return gr;
}

/* *****************
 * TestGrammar10
 * *****************/
Grammar TestGrammar10(){
  
  PushdownGenerator g1 = TestGenerator1();
  g1.InsEvent("dollar");
  
  StackSymbolSet stackSymbols = g1.StackSymbols();

  std::vector<Idx> ssvSquare;
  ssvSquare.push_back(stackSymbols.Index("square"));
  std::vector<Idx> ssvDot;
  ssvDot.push_back(stackSymbols.Index("dot"));
  
  //construct Terminals
  Terminal* ta = new Terminal(g1.Alphabet().Index("a"));
  GrammarSymbolPtr taPtr(ta);
  Terminal* tdollar = new Terminal(g1.Alphabet().Index("dollar"));
  GrammarSymbolPtr tdollarPtr(tdollar);
  Terminal* tlambda = new Terminal(g1.Alphabet().Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr tlambdaPtr(tlambda);
  
  //construct Nonterminals
  Nonterminal* nt1square = new Nonterminal(1,ssvSquare);
  GrammarSymbolPtr nt1squarePtr(nt1square);
  Nonterminal* nt1dot1 = new Nonterminal(1,ssvDot,1);
  GrammarSymbolPtr nt1dot1Ptr(nt1dot1);

  //construct GrammarProduction
  GrammarSymbolVector v;
  v.push_back(nt1dot1Ptr);
  v.push_back(taPtr);
  GrammarProduction gp1 = GrammarProduction(*nt1square,v);
  v.clear();
  v.push_back(tlambdaPtr);
  GrammarProduction gp2 = GrammarProduction(*nt1dot1,v);
  
  //construct Grammar
  Grammar gr = Grammar();
  gr.InsTerminal(*ta);
  gr.InsTerminal(*tlambda);
  
  gr.InsNonterminal(*nt1square);
  gr.InsNonterminal(*nt1dot1);
  
  gr.SetStartSymbol(*nt1square);
 
  gr.InsGrammarProduction(gp1);
  gr.InsGrammarProduction(gp2);
  
  return gr;
}

/* *****************
 * TestGrammar11
 * *****************/
Grammar TestGrammar11(){
  
  PushdownGenerator g1 = TestGenerator1();
  
  StackSymbolSet stackSymbols = g1.StackSymbols();

  std::vector<Idx> ssvSquare;
  ssvSquare.push_back(stackSymbols.Index("square"));
  
  //construct Terminals
  Terminal* ta = new Terminal(g1.Alphabet().Index("a"));
  GrammarSymbolPtr taPtr(ta);
  Terminal* tb = new Terminal(g1.Alphabet().Index("b"));
  GrammarSymbolPtr tbPtr(tb);
  Terminal* tlambda = new Terminal(g1.Alphabet().Index(FAUDES_PD_LAMBDA));
  GrammarSymbolPtr tlambdaPtr(tlambda);
  
  //construct Nonterminals
  Nonterminal* nt1square = new Nonterminal(1,ssvSquare);
  GrammarSymbolPtr nt1squarePtr(nt1square);

  //construct GrammarProduction
  GrammarSymbolVector v;
  v.push_back(taPtr);
  v.push_back(nt1squarePtr);
  v.push_back(tbPtr);
  GrammarProduction gp1 = GrammarProduction(*nt1square,v);
  v.clear();
  v.push_back(tlambdaPtr);
  GrammarProduction gp2 = GrammarProduction(*nt1square,v);
  
  //construct Grammar
  Grammar gr = Grammar();
  gr.InsTerminal(*ta);
  gr.InsTerminal(*tb);
  gr.InsTerminal(*tlambda);
  
  gr.InsNonterminal(*nt1square);

  gr.SetStartSymbol(*nt1square);
 
  gr.InsGrammarProduction(gp1);
  gr.InsGrammarProduction(gp2);
  
  return gr;
}




void TestStart(std::string name){
  std::cout << "Testing " << name << " ..." <<  std::endl;
}

void TestEnd(std::string name){
  std::cout << "Finished " << name << std::endl;  
}

} // namespace faudes

