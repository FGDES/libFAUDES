/** @file hio_3_hiofunctions.cpp

Tutorial, functions provided by hiosys plugin.

@ingroup Tutorials 

@include hio_3_hiofunctions.cpp

*/

#include "libfaudes.h"

// make the faudes namespace available to our program
using namespace faudes;
using namespace std;

/** simple machine example to demonstrate I/O controller synthesis */
void simpleMachine() {
 
	/* 
	% Simple machine example for hiosys-tutorial.
	% Machine sends YP-event "A_wait" to operator.
	% Does nothing after UP-event "A_stp", or executes
	% the process "A_do", which requires a shared
	% resource from the environment, modelled by
	% "A_req". The environment may provide or
	% deny access to the resource ("A_pack" or
	% "A_nack", respectively).
	*/
	 
	// read plant A model
	HioPlant plantA;
	plantA.Read("data/3_hiofunctions/hio_simplemachine_A.gen");
	plantA.GraphWrite("tmp_hio_simplemachine_A.png");
	// must be in I/O plant form
	if(!IsHioPlantForm(plantA)){
	  	std::stringstream errstr;
	  	errstr << "plant A not in HioPlantForm.\n";
		throw Exception("simpleMachine", errstr.str(), 0);
	}
	
	/*
	% Environment constraint:
	% Resources are always provided as requested.
	*/
	// read plant A environment constraint
	HioConstraint SeA;
	SeA.Read("data/3_hiofunctions/hio_simpleEnvConstr_A.gen");
	//should be in I/O constraint form:
	if(!IsHioConstraintForm(SeA)) {
		std::cout<<"Warning: environment constraint A not in I/O constraint form.";
	}
	SeA.GraphWrite("tmp_hio_simpleEnvConstr_A.png");
	
	/*
	% Operator constraint:
	% simple machine is complete and YP-life wrt. to the
	% environment constraint and a minimal operator
	% constraint, which - for convenience - can be
	% modelled by an epsilon language HioConstraint.
	*/
	// construct epsilon language operator constraint
	HioConstraint SpA;
	Idx init=SpA.InsInitState();
	SpA.SetMarkedState(init);
	
	// first, we synthesise a controller for the single plant A:	
/*============================================================================
================ CONTROLLER SYNTHESIS FOR MONOLITHIC PLANT =============
==============================================================================*/

	/*
	% Specification:
	% Send YC-event "A_READY" as feedback to operator.
	% Accept UC-events "A_STANDBY" or "A_OPERATE" as 
	% operator inputs.
	% Command "A_STANDBY": do nothing.
	% Command "A_OPERATE": Process two resources.
	*/
	HioPlant specA;
	specA.Read("data/3_hiofunctions/hio_simplespec_A.gen");
	specA.GraphWrite("tmp_hio_simplespec_A.png");
    
	/*
	% external operator constraint:
	% also specification A is complete and YP-life wrt. to the
	% environment constraint and a minimal operator
	% constraint.
	*/
	// construct epsilon language external operator constraint
	HioConstraint ScA;
	init=ScA.InsInitState();
	ScA.SetMarkedState(init);
	
	// run controller synthesis algorithm
	HioController controllerA;
	HioSynthMonolithic(plantA,specA,ScA,SpA,SeA,controllerA);
  FAUDES_TEST_DUMP("HioSynthMonolithic",controllerA);
	controllerA.Write("tmp_hio_simpleController_A.gen");
	controllerA.GraphWrite("tmp_hio_simpleController_A.png");
	
	// now, we synthesise a controller for two plants A and B:	
/*============================================================================
===================== HIERARCHICAL CONTROLLER SYNTHESIS =================
==============================================================================*/
	
	/*
	% Machine B Sends YP-event "B_wait" to operator.
	% Does nothing after UP-event "B_stp", or executes
	% the process "B_do", which produces a shared
	% resource provided to the environment, modelled
	% by "B_prov". The environment may or may not
	% accept the resource ("B_pack" or "B_nack",
	% respectively).
	 */
	 
	// read plant B model
	HioPlant plantB;
	plantB.Read("data/3_hiofunctions/hio_simplemachine_B.gen");
	plantB.GraphWrite("tmp_hio_simplemachine_B.png");
	// must be in I/O plant form
	if(!IsHioPlantForm(plantB)){
	  	std::stringstream errstr;
	  	errstr << "plant B not in HioPlantForm.\n";
		throw Exception("simpleMachine", errstr.str(), 0);
	}
	
	/*
	% Environment constraint:
	% Resources are always provided as requested.
	*/
	// read plant A environment constraint
	HioConstraint SeB;
	SeB.Read("data/3_hiofunctions/hio_simpleEnvConstr_B.gen");
	if(!IsHioConstraintForm(SeB)) {
		std::cout<<"Warning: environment constraint B not in I/O constraint form.";
	}
	SeB.GraphWrite("tmp_hio_simpleEnvConstr_B.png");
	
	/*
	% Operator constraint:
	% simple machine B is complete and YP-life wrt. to the
	% environment constraint and a minimal operator
	% constraint, which - for convenience - can be
	% modelled by an epsilon language HioConstraint.
	*/
	// construct epsilon language operator constraint
	HioConstraint SpB;
	init=SpB.InsInitState();
	SpB.SetMarkedState(init);
	
	/*===============================================
	I/O SHUFFLE OF PLANT A AND PLANT B
	=================================================*/

	HioPlant ioShuffleAB;
	HioShuffle(plantA,plantB,ioShuffleAB);
  FAUDES_TEST_DUMP("HioShuffle",ioShuffleAB);
	ioShuffleAB.Write("tmp_hio_simpleHioShuffle_AB.gen");
	ioShuffleAB.GraphWrite("tmp_hio_simpleHioShuffle_AB.png");
	
	// compose constraints of plant A and B_do
	HioConstraint intConstrAB;
	Parallel(SpA,SpB,intConstrAB);
	Parallel(intConstrAB,SeA,intConstrAB);
	Parallel(intConstrAB,SeB,intConstrAB);
	intConstrAB.StateNamesEnabled(false);
	
	/*===============================================
	ENVIRONMENT MODEL FOR PLANT AB
	=================================================*/
	/*
	% Simple machine example for hiosys-tutorial.
	% Environment:
	% A request of machine A (A_req) for a resource is
	% denied (A_nack) until machine B provides it
	% (B_prov, B_pack). Then, machine A has to request
	% the resource until machine B can provide the next
	% one. A resource provided by machine B and
	% requested by machine A is readily processed
	% and can be provided to the external
	% environment (AB_prov), which may or may not
	% accept the processed resource (AB_pack,
	% AB_nack).
	*/
	HioEnvironment envAB("data/3_hiofunctions/hio_simpleenvironment_AB.gen");
	envAB.GraphWrite("tmp_hio_simpleenvironment_AB.png");
	
	/*
	% Environment constraint:
	% Processed resources are always accepted as provided.
	*/
	// read plant A environment constraint
	HioConstraint SlAB;
	SlAB.Read("data/3_hiofunctions/hio_simpleEnvConstr_AB.gen");
	if(!IsHioConstraintForm(SlAB)) {
		std::cout<<"Warning: environment constraint AB not in I/O constraint form.";
	}
	SlAB.GraphWrite("tmp_hio_simpleEnvConstr_AB.png");
	
	/*====================================================
	SPECIFICATION AND EXTERNAL OPERATOR CONSTRAINT
	======================================================*/	
	/*
	% Simple machine example for hiosys-tutorial.
	% Specification:
	% Send YC-event "AB_READY" as feedback to operator.
	% Accept UC-events "AB_STANDBY" or "AB_OPERATE" as 
	% operator inputs.
	% Command "AB_STANDBY": do nothing.
	% Command "AB_OPERATE": Provide processed resource (AB_prov).
	 */
	HioPlant specAB;
	specAB.Read("data/3_hiofunctions/hio_simplespec_AB.gen");
	specAB.GraphWrite("tmp_hio_simplespec_AB.png");
    
	/*
	% external operator constraint:
	% also specification AB is complete and YP-life wrt. to the
	% environment constraint and a minimal operator
	% constraint.
	*/
	// construct epsilon language external operator constraint
	HioConstraint ScAB;
	init=ScAB.InsInitState();
	ScAB.SetMarkedState(init);
	
	// run controller synthesis algorithm
	HioController controllerAB;
	HioSynthHierarchical(ioShuffleAB,envAB,specAB,intConstrAB,ScAB,SlAB,controllerAB);
  FAUDES_TEST_DUMP("HioSynthHierarchical",controllerAB);
	controllerAB.Write("tmp_hio_simpleController_AB.gen");
	controllerAB.GraphWrite("tmp_hio_simpleController_AB.png");
	
    return;
}


/** Run the tutorial */
int main() {
  // call simple machine example
  try {
  simpleMachine();
  }
  catch (Exception& e) {
  std::cout << "function: " << e.Where() << std::endl;
  std::cout << "exception description: " << e.What() << std::endl;
  std::cout << "exception id: " << e.Id() << std::endl;
}
  return 0;
}
