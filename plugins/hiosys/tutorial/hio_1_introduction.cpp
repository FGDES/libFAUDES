/** @file hio_1_introduction.cpp 

Tutorial, simple i/o-based controller synthesis.

This tutorial uses a very simple example to exhibit
monolithic input-/output based controller synthesis.
The purpose is to generate graphs for the
introduction of hiosys in the libFAUDES user
reference, see "hiosys_index.html".
For other purposes such as learning how to use hiosys,
we refer to the tutorials hio_2_hiogenerators,
hio_3_hiofunctions and hio_4_transport_unit.

@ingroup Tutorials 

@include hio_1_introduction.cpp

*/

#include "libfaudes.h"

// make the faudes namespace available to our program
using namespace faudes;

/////////////////
// main program
/////////////////

int main() {

  ////////////////////////////////////////////
  // PLANT MODEL WITH INPUTS AND OUTPUTS
  ////////////////////////////////////////////
  
	// plant dynamics of machines 1 and 2
  // note: only the plant-I/O port (Up,Yp)  of HioPlant is used in this
  //         simple non-hierarchical example. The remaining port (Ue,Ye)
  //         is introduced as dummmy.
  HioPlant tempgen, machine1, machine2;
  tempgen.Read("data/1_introduction/hio_simplemachine_intro.gen");
  tempgen.Version("1",machine1);
  tempgen.Version("2",machine2);
  machine1.Write("tmp_hio_machine1_intro.gen");
  machine1.GraphWrite("tmp_hio_machine1_intro.png");
  machine2.Write("tmp_hio_machine2_intro.gen");
  machine2.GraphWrite("tmp_hio_machine2_intro.png");
  
  // plant inputs up and outputs yp
  EventSet up1, up2, yp1, yp2;
  up1=machine1.UpEvents();
  up2=machine2.UpEvents();
  yp1=machine1.YpEvents();
  yp2=machine2.YpEvents();

  ////////////////////
  // IOSHUFFLE
  ////////////////////  
  
  // note: only the plant-I/O port (Up,Yp)  of HioPlant is used in this
  //         simple non-hierarchical example. The remaining port (Ue,Ye)
  //         is introduced as dummmy.
  HioPlant plant;
  HioShuffle(machine1,machine2,plant);
  // ignore marking
  PrefixClosure(plant);
  // cosmetics
  StateMin(plant,plant);
  plant.StateNamesEnabled(false);
  //remove dummy events for file output
  tempgen=plant;
  Project(plant,yp1+yp2+up1+up2,tempgen);
  tempgen.SetYp(yp1+yp2);
  tempgen.SetUp(up1+up2);
  tempgen.Name("HioShuffle(Simple Machine_1,Simple Machine_2)");
  tempgen.Write("tmp_hio_ioshuffle_intro.gen");
  tempgen.GraphWrite("tmp_hio_ioshuffle_intro.png");
  // constraint to avoid err-states of ioshuffle
  HioConstraint ioconstr("data/1_introduction/hio_ioconstraint_intro.gen");

  ////////////////////
  // SPECIFICATION
  // (poses a constraint, in the non-hierarchical case)
  ////////////////////  
  
  HioConstraint spec("data/1_introduction/hio_specification_intro.gen");
  spec.Write("tmp_hio_specification_intro.gen");
  spec.GraphWrite("tmp_hio_specification_intro.png");
  // compose constraint and specification to compound requirement to be met by controller
  Parallel(spec,ioconstr,spec);  
  // in the hierarchical case, the actual specification is formulated over additional alphabets.
  // here, this spec and according alphabets and constraint are introduced as dummies
  HioPlant dummy_spec;
  Idx ds1=dummy_spec.InsInitState();
  dummy_spec.SetMarkedState(ds1);
  EventSet dummy_yc, dummy_uc;
  dummy_yc.Insert("yc");
  dummy_uc.Insert("uc");
  HioConstraint dummy_constraint;
  ds1=dummy_constraint.InsInitState();
  dummy_constraint.SetMarkedState(ds1);
  
  ////////////////////
  // SYNTHESIS
  ////////////////////    
  
  // note: only the controller-I/O port (Up,Yp)  of HioController is used in this
  //         simple non-hierarchical example. The remaining port (Uc,Yc)
  //         is introduced as dummmy.
  HioController controller;
  HioSynthUnchecked(plant,dummy_spec,dummy_constraint,spec,dummy_yc,dummy_uc,yp1+yp2,up1+up2,plant.YeEvents(),plant.UeEvents(),controller);
  
  // remove dummies yc,uc,ye,ue
  Project(controller,yp1+up1+yp2+up2,controller);
  // restore event attributes
  controller.SetYp(yp1+yp2);
  controller.SetUp(up1+up2);
  controller.Write("tmp_hio_controller_intro.gen");
  
    ////////////////////
    // CLOSED LOOP
    ////////////////////      
  Generator closedloop;
  Parallel(plant,controller,closedloop);
  // remove dummy events
  Project(closedloop,yp1+up1+yp2+up2,closedloop);
  closedloop.Write("tmp_hio_closedloop_intro.gen");
  
    ////////////////////
    // COMPARE WITH SCT
    ////////////////////   
  
  // remove nonphysical io events
  Generator supervisor;
  EventSet alphabeta;
  alphabeta.Insert("alpha_1");
  alphabeta.Insert("alpha_2");
  alphabeta.Insert("beta_1");
  alphabeta.Insert("beta_2");
  Project(closedloop,alphabeta,supervisor);
  supervisor.Write("tmp_hio_physical_intro.gen");
  //   ->  should lead to same supervisor as in Synthesis PlugIn example !

  return 0;
}



