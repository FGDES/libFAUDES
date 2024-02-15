/** @file hio_5_conveyor_belts.cpp

Tutorial, conveyor belt chain example for  hiosys plugin 

@ingroup Tutorials 

@include hio_5_conveyor_belts.cpp

*/

#include "libfaudes.h"

// make the faudes namespace available to our program
using namespace faudes;

int conveyor_belts(){

/*
-----------------------------------------------------------------
Conveyor Belts, only left to right, designed with hiosys-approach
-----------------------------------------------------------------
Controller design, physical interface and simulation model for the conveyor-belt chain "cb11-cb4-cb12-cb5-cb13-cb6-cb14-cb10" of the LRT Fischertechnik model.
E.g. for the first 4 conveyor belts, we obtain the following hierarchy of automata:

							ABCD_controller
								  |
				-------------------------------------
				|									|
			AB_controller					CD_controller
				|									|
	  -------------------- 	    	  ------------------
	  |		          		    |		    		  |					   |
A_controller		B_controller	  C_controller		D_controller    
	  |		          		    |		    		  |					   |
     -------------------------------------------------------------------------
			phys_interface(eg.'phys_A_l_implies_cb11-x-y.vio')          
	-------------------------------------------------------------------------
	  |		          		    |		    		  |					   |	       \
  plant_cb11	 	       plant_cb4		     plant_cb12		  plant_cb5  	plant_TICK

The lowest level ("plant_") implements a timed physical model of the conveyor-belt chain.
The model 'plant_TICK' implements the passing of time for the controllers 'A_'
to 'D_'(20 time units per wait-loop). For HIL-Simulation, the "plant_" layer is replaced 
by IO devices.

The physical interface translates physical sensor edges to abstract inputs for
the controllers 'A_' to 'D_', and translates abstract controller outputs to
physical actuator edges.

The folder data/5_conveyor_belts/hil_demo/ contains a script 'run.sh' and
appendage to run the controllers in HIL simulation with the lrtlab Fischertechnik model.
See data/5_conveyor_belts/hil_demo/README.txt for details.  To update hil_demo 
with the latest controllers computed by this tutorial, run 
hio_5_conveyor_belts_update_hil_demo.sh

Note: execution of this tutorial can take about two hours!
*/

  std::string errstr;

//////////////////////////////////////////
// read plantA:
//////////////////////////////////////////

  std::cout << std::endl << "******** reading files: plant A, spec A, constraints LC_A, LP_A and LE_A."  << std::endl;
  //read plantA and corresp. alphabets:
  HioPlant plantA("data/5_conveyor_belts/A_plant.gen");
  plantA.Write("tmp_hio_cb_A_plant.gen");
  if(!IsHioPlantForm(plantA,errstr)) std::cout<<errstr;
  std::cout<<std::endl<<"plant done.";
  //read contraints:
  // L_P of plant A: has been left minimal, designed as empty generator for simmplicity
  //                       this is allowed, because, for SINGLE plants, the controller is computed
  //                       such that its projection to SigmaP is a constraint for liveness of the plant.
  Generator LP_A("data/5_conveyor_belts/A_LP.gen");
  // L_E of plant A (no A_nack): designed as empty generator with alphabet
  // {A_nack} for simmplicity
  Generator LE_A("data/5_conveyor_belts/A_LE.gen");
  
  //read specification and corr. alphabets:
  HioPlant specA("data/5_conveyor_belts/A_spec.gen");
  if(!IsHioPlantForm(specA,errstr)) std::cout<<errstr;
  specA.Write("tmp_hio_cb_A_spec.gen");
  
  // read constraint L_C:
  HioConstraint LC_A("data/5_conveyor_belts/A_LC.gen");
  if(!IsHioConstraintForm(LC_A,errstr)) std::cout<<errstr;
  LC_A.Write("tmp_hio_cb_A_LC.gen");
  
  // // do controller synthesis:
  HioController controllerA;
  std::cout << std::endl << "******** local controller synthesis for plant A, B, C,...,H."  << std::endl;
  HioSynthMonolithic(plantA,specA,LC_A,LP_A,LE_A,controllerA);
  LP_A.Clear();
  FAUDES_TEST_DUMP("HioSynthMonolithic: Fischertechnik Conveyor Belts",controllerA);
  // controllerA.Name("Controller A");
  controllerA.Write("tmp_hio_cb_A_controller.gen");
  // // controller without errStates
  // Parallel(controllerA,plantA,controllerA);
  // Project(controllerA,plantA.PEvents()+specA.PEvents(),controllerA);
  // PrefixClosure(controllerA);
  // StateMin(controllerA,controllerA);
  // controllerA.StateNamesEnabled(false);
  // controllerA.Name("controller A");
  // controllerA.Write("tmp_hio_cb_A_controller.gen");
  
  plantA.Clear();
	
	// // remark: controller synthesis for plants B, C,... is analogous.
	// //         Hence, the controllers are obtained by copy of the controllerA-file and appr. string replacement
  HioController controllerX;
  controllerA.Version("A_","B_",controllerX);
  controllerX.Write("tmp_hio_cb_B_controller.gen");
  controllerA.Version("A_","C_",controllerX);
  controllerX.Write("tmp_hio_cb_C_controller.gen");
  controllerA.Version("A_","D_",controllerX);
  controllerX.Write("tmp_hio_cb_D_controller.gen");
  controllerA.Version("A_","E_",controllerX);
  controllerX.Write("tmp_hio_cb_E_controller.gen");
  controllerA.Version("A_","F_",controllerX);
  controllerX.Write("tmp_hio_cb_F_controller.gen");
  controllerA.Version("A_","G_",controllerX);
  controllerX.Write("tmp_hio_cb_G_controller.gen");
  controllerA.Version("A_","H_",controllerX);
  controllerX.Write("tmp_hio_cb_H_controller.gen");

  controllerA.Clear();
  controllerX.Clear();
  
// //////////////////////////////////////////
// // plantAB: I/O shuffle and composition with environment
// //////////////////////////////////////////

  // plants are abstracted by specs, where all specs specB,specC... for a single conveyor belt are analogous to specA
  HioPlant specB;
  specA.Version("A_","B_",specB);
  specB.Write("tmp_hio_cb_B_spec.gen");

  std::cout <<std::endl<< "******** IOshuffle of spec A and spec B."  << std::endl;
  HioPlant shuffAB;
  HioShuffle(specA, specB, shuffAB);
  FAUDES_TEST_DUMP("HioShuffle: Fischertechnik Conveyor Belts",shuffAB);
  specA.Clear();
  specB.Clear();
  
  std::cout<<std::endl<<"**** Statistics of shuffAB: "<<std::endl;
	shuffAB.SWrite();
	std::cout<<std::endl<<"**** Doing statemin...";
  StateMin(shuffAB,shuffAB);
  std::cout<<std::endl<<"**** Statistics after statemin: "<<std::endl;
	shuffAB.SWrite();
	shuffAB.StateNamesEnabled(false);
  shuffAB.Name("IO shuffle AB");
  shuffAB.Write("tmp_hio_cb_AB_shuff.gen_rename"); // _rename to avoid dot processing
  
  std::cout <<std::endl<< "******** environment: read files AB_Env1 and AB_Env2, composition to environment model."  << std::endl;
  //read environmentAB, which is derived from two components:
  HioEnvironment envAB1("data/5_conveyor_belts/AB_Env1.gen");
  HioEnvironment envAB2("data/5_conveyor_belts/AB_Env2.gen");
  envAB1.Write("tmp_hio_cb_AB_env1.gen");
  envAB2.Write("tmp_hio_cb_AB_env2.gen");
  HioEnvironment envAB;
  // compose preserving event attributes
  aParallel(envAB1,envAB2,envAB);
  envAB1.Clear();
  envAB2.Clear();
  StateMin(envAB,envAB);
  envAB.StateNamesEnabled(false);
  envAB.Name("envAB");
  if(!IsHioEnvironmentForm(envAB,errstr)) std::cout<<errstr;
  envAB.Write("tmp_hio_cb_AB_env.gen");
  
  std::cout <<std::endl<< "******** Composition shuffAB||envAB."  << std::endl;  
  
  ///////////////////////////////////////////////////
  // controller synthesis for plantAB:
  ///////////////////////////////////////////////////

  std::cout <<std::endl<< "******** Specification and constraints for compound AB."  << std::endl;  

  //read specAB
  HioPlant specAB("data/5_conveyor_belts/AB_spec.gen");
  if(!IsHioPlantForm(specAB,errstr)) std::cout<<errstr;
  specAB.Write("tmp_hio_cb_AB_spec.gen");

  // read LC_AB
  HioConstraint LC_AB("data/5_conveyor_belts/AB_LC.gen");
  if(!IsHioConstraintForm(LC_AB,errstr)) std::cout<<errstr;
  LC_AB.Write("tmp_hio_cb_AB_LC.gen");
  // read LL_AB
  Generator LL_AB("data/5_conveyor_belts/AB_LL.gen");
  
  // local constraints LC_B and LE_B from LC_A and LE_A
  Generator LC_B, LE_B;
  LC_A.Version("A_","B_",LC_B);
  LE_A.Version("A_","B_",LE_B);
  Generator intConstr_AB;
  Parallel(LC_A,LC_B,intConstr_AB);
  Parallel(intConstr_AB,LE_A,intConstr_AB);
  Parallel(intConstr_AB,LE_B,intConstr_AB);
  intConstr_AB.Name("intConstr_AB");  
  intConstr_AB.StateNamesEnabled(false);  
  LE_A.Clear();
  LE_B.Clear();
  LC_A.Clear();
  LC_B.Clear();

  std::cout <<std::endl<< "******** Controller synthesis for compound AB."  << std::endl;  
  //calculate IO controller for plantAB
  HioController controllerAB;
  HioSynthHierarchical(shuffAB, envAB, specAB, intConstr_AB, LC_AB, LL_AB, controllerAB);
  controllerAB.Name("controllerAB");
  controllerAB.SWrite();
  controllerAB.Write("tmp_hio_cb_AB_controller.gen_rename");
  
  HioController controllerX2;
  
  controllerAB.Version("AB_","CD_",controllerX);
  controllerX.Version("A_","C_",controllerX2);
  controllerX2.Version("B_","D_",controllerX);
  controllerX.Write("tmp_hio_cb_CD_controller.gen_rename");  
  
  controllerAB.Version("AB_","EF_",controllerX);
  controllerX.Version("A_","E_",controllerX2);
  controllerX2.Version("B_","F_",controllerX);
  controllerX.Write("tmp_hio_cb_EF_controller.gen_rename");  
  
  controllerAB.Version("AB_","GH_",controllerX);
  controllerX.Version("A_","G_",controllerX2);
  controllerX2.Version("B_","H_",controllerX);
  controllerX.Write("tmp_hio_cb_GH_controller.gen_rename");  
  


  shuffAB.Clear();
  envAB.Clear();
  intConstr_AB.Clear();
  
//######################################################################
  //*****
  //***** plantABCD: I/O shuffle and composition with environment.*****
  //*****
//######################################################################

  std::cout <<std::endl<< "******** IOshuffle of spec AB and spec CD."  << std::endl;  

  // specCD from specAB
  HioPlant specCD;
  specAB.Version("AB_","CD_",specCD);
  specCD.Write("tmp_hio_cb_CD_spec.gen");
  
  // HioShuffle
  HioPlant shuffABCD;
  HioShuffle(specAB, specCD, shuffABCD);
  //specAB.Clear(); // needed later to create specABCD
  specCD.Clear();
  StateMin(shuffABCD,shuffABCD);
  shuffABCD.Name("shuff ABCD");
	shuffABCD.StateNamesEnabled(false);
	shuffABCD.SWrite();
  shuffABCD.Write("tmp_hio_cb_ABCD_shuff.gen_rename");

  std::cout <<std::endl<< "******** environmentAB: read files, composition to environment model."  << std::endl;  
  //read environmentABCD:
  HioEnvironment envABCD1("data/5_conveyor_belts/ABCD_Env1.gen");
  envABCD1.Write("tmp_hio_cb_ABCD_env1.gen");
  HioEnvironment envABCD2("data/5_conveyor_belts/ABCD_Env_ABCDfreetr.gen");
  envABCD2.Write("tmp_hio_cb_ABCD_env_freetr.gen");
  HioEnvironment envABCD;
  aParallel(envABCD1,envABCD2,envABCD);
  envABCD2.Read("data/5_conveyor_belts/ABCD_Env_ABCDoccfl.gen");
  envABCD2.Write("tmp_hio_cb_ABCD_env_occfl.gen");
  aParallel(envABCD,envABCD2,envABCD);
  envABCD1.Clear();
  envABCD2.Clear();
  StateMin(envABCD,envABCD);
  envABCD.StateNamesEnabled(false);
  envABCD.Name("envABCD");
  if(!IsHioEnvironmentForm(envABCD,errstr)) std::cout<<errstr;
  envABCD.SWrite();
  envABCD.Write("tmp_hio_cb_ABCD_env.gen");

  ///////////////////////////////////////////////////
  // controller synthesis for plantABCD:
  ///////////////////////////////////////////////////

  // specABCD: same as spec AB or spec CD, spec EF, spec GH
  HioPlant specABCD;
  specAB.Version("AB_","ABCD_",specABCD);
  specAB.Clear(); 
  specABCD.Write("tmp_hio_cb_ABCD_spec.gen");
  
  // internal contraints:
  Generator LC_CD,LL_CD;
  LC_AB.Version("AB_","CD_",LC_CD);
  LL_AB.Version("AB_","CD_",LL_CD);

  Generator intConstr_ABCD;
  Parallel(LC_AB,LC_CD,intConstr_ABCD);
  Parallel(intConstr_ABCD,LL_AB,intConstr_ABCD);
  Parallel(intConstr_ABCD,LL_CD,intConstr_ABCD);
  intConstr_ABCD.Name("intConstr_ABCD");  
  intConstr_ABCD.StateNamesEnabled(false);
  
  // external constraints:
  Generator LC_ABCD,LL_ABCD;
  LC_AB.Version("AB_","ABCD_",LC_ABCD);
  LL_AB.Version("AB_","ABCD_",LL_ABCD);

  //calculate IO controller for plantABCD

  std::cout <<std::endl<< "******** Controller synthesis for compound ABCD."  << std::endl;  
  //calculate IO controller for plantABCD
  HioController controllerABCD;
  // caution: takes up to 2 hours!
  std::cout <<std::endl<< "**** Synthesis procedure is running - this can take about two hours! ..."  << std::endl;  
  //HioSynthHierarchical(shuffABCD, envABCD, specABCD, intConstr_ABCD, LC_ABCD, LL_ABCD, controllerABCD);
  controllerABCD.Name("controllerABCD");
  controllerABCD.SWrite();
  controllerABCD.Write("tmp_hio_cb_ABCD_controller.gen_rename");
  controllerABCD.Read("tmp_hio_cb_ABCD_controller.gen_rename");

  shuffABCD.Clear();
  envABCD.Clear();
  intConstr_ABCD.Clear();
  
  controllerABCD.Version("ABCD_","EFGH_",controllerX);
  controllerX.Version("AB_","EF_",controllerX2);
  controllerX2.Version("CD_","GH_",controllerX);
  controllerX.Write("tmp_hio_cb_EFGH_controller.gen_rename");  
  
  controllerABCD.Version("ABCD_","ABCDEFGH_",controllerX);
  controllerX.Version("CD_","EFGH_",controllerX2);
  controllerX2.Version("AB_","ABCD_",controllerX);
  controllerX.Write("tmp_hio_cb_ABCDEFGH_controller.gen_rename");  

	std::cout << std::endl << "done.";

  return 0;
}

/** Run the tutorial */
int main() {
  // call simple machine example
  try {
  conveyor_belts();
  }
  catch (Exception& e) {
  std::cout << "function: " << e.Where() << std::endl;
  std::cout << "exception description: " << e.What() << std::endl;
  std::cout << "exception id: " << e.Id() << std::endl;
}
  return 0;
}
