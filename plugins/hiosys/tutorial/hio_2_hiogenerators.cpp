/** @file hio_2_hiogenerators.cpp 

Tutorial, i/o system generator classes.

This tutorial demonstrates basic maintenance of HioPlant, HioController
and HioEnvironment objects.

@ingroup Tutorials 

@include hio_2_hiogenerators.cpp

*/

#include "libfaudes.h"

// make the faudes namespace available to our program
using namespace faudes;

/////////////////
// main program
/////////////////

int main() {

  // for intermediate results
  bool tmpresult;
  
 /**************************************************
 * HIO PLANT
 ***************************************************/
  
  /******************
   * constructor and file io
   ********************/

  // Create an empty HioPlant object
  HioPlant g1;

  // Create a HioPlant by reading a HioPlant file
  HioPlant g2("data/2_hiogenerators/hio_simplemachine_A.gen");
  FAUDES_TEST_DUMP("HioPlant from genfile",g2);
 
  // Create a HioPlant by reading a plain Generator file (no event attributes)
  g1.Read("data/2_hiogenerators/simplemachine_A.gen");

  // Assign/copy a plain Generator to a HioPlant (no event attributes)
  Generator g4("data/2_hiogenerators/simplemachine_A.gen");
  HioPlant g3=g4;
  HioPlant g5(g4);

  // Assign/copy a HioPlant to a HioPlant (no event attributes)
  HioPlant g6=g2;
  FAUDES_TEST_DUMP("HioPlant = operator",g6);
  HioPlant g7(g2);
  FAUDES_TEST_DUMP("HioPlant construct from HioPlant",g7);
  HioPlant g8;
  g8.Assign(g2);
  FAUDES_TEST_DUMP("HioPlant::Assign()",g8);
  
  // Construct HioPlant from plain generator and event sets
  EventSet ypEvents,upEvents,yeEvents,ueEvents;
  ypEvents=g2.YpEvents();
  FAUDES_TEST_DUMP("get Yp events",ypEvents);
  upEvents=g2.UpEvents();
  yeEvents=g2.YeEvents();
  ueEvents=g2.UeEvents();
  
  HioPlant g9(g4,ypEvents,upEvents,yeEvents,ueEvents);
  FAUDES_TEST_DUMP("HioPlant(vGen,EventSets)",g9);

  // Write the HioPlant to files 
  g1.Write("tmp_hiosimplemachine_A1.gen");
  g2.Write("tmp_hiosimplemachine_A2.gen");
  g3.Write("tmp_hiosimplemachine_A3.gen");
  g5.Write("tmp_hiosimplemachine_A5.gen");
  g6.Write("tmp_hiosimplemachine_A6.gen");
  g9.Write("tmp_hiosimplemachine_A9.gen");
  
  g1.Write("tmp_hiosimplemachine_A.gen");


  // Report to console
  std::cout << "######################################\n";
  std::cout << "# hio simple machine\n";
  g2.Write();
  std::cout << "######################################\n";

 /*****************************
  * Test whether HioPlant meets IO-Plant form
  ******************************/

  std::cout << "######################################\n";
  
  // Fails for g1 due to missing event attributes
  std::cout << "# IsHioPlantForm A: expect to fail for missing attributes\n";  
  std::string report;
  tmpresult=IsHioPlantForm(g1,report);
  FAUDES_TEST_DUMP("IsHioPlant fail",tmpresult);
  std::cout<<report;

  // Fulfilled for g2:
  std::cout << "# IsHioPlantForm B: expect to pass\n";  
  tmpresult=IsHioPlantForm(g2,report);
  FAUDES_TEST_DUMP("IsHioPlant pass",tmpresult);
  std::cout<<report;
  
  // Remove some transitions with input-event
  TransSet::Iterator tit;
  for (tit = g2.TransRelBegin(); tit != g2.TransRelEnd(); ++tit) {
     //if(g2.IsUp(tit->Ev)) {
     if(g2.EventName(tit->Ev)=="A_stp" || g2.EventName(tit->Ev)=="A_nack") {
		 g2.ClrTransition(*tit);
		}
    }
  g2.Write("tmp_hiosimplemachine_A_broken.gen");
  g2.GraphWrite("tmp_hiosimplemachine_A_broken.png");
  // Now, the input is no longer free in g2
  std::cout << "# IsHioPlantForm C: expect to fail for missing input transitions\n";
  IsHioPlantForm(g2,report);
  std::cout<<report;
  // Repair
  HioFreeInput(g2,g2);
  FAUDES_TEST_DUMP("HioFreeInput",g2);
  g2.Write("tmp_hiosimplemachine_A_fixed.gen");
  // HioPlantForm is retrieved for g2:
  std::cout << "# IsHioPlantForm D: expect to pass\n";
  IsHioPlantForm(g2,report);
  std::cout<<report;
  
  FAUDES_TEST_DUMP("HioPlant construction",g2);
  g2.Write();
  g2.Write("tmp_hiosimplemachine_A_repaired.gen");

  std::cout << "######################################\n";

 /*******************************************
  * Access to hio-property of states
  *
  * Note: hio state properties need not be up to date
  *  - always run IsHioPlantForm() to set state attributes
  **************************************/
  
  // Get states of g2 sorted according to their active event sets
  StateSet QYpYe,QUp,QUe;
  QYpYe=g2.QYpYeStates();
  QUp=g2.QUpStates();
  FAUDES_TEST_DUMP("get Up states",QUp);
  QUe=g2.QUeStates();
  std::cout<<std::endl;
  
  // Show on console
  std::cout << "######################################\n";
  std::cout << "# QYpYe,QUp and QUe of simple machine:\n";
  QYpYe.Write();
  QUp.Write();
  QUe.Write();
  std::cout << "######################################\n"; 


 /*************************
  * Access to hio-property of events
  **************************/

  // Retrieve EventSets containing all YP-, UP-, YE-, UE-events  from g2
  ypEvents=g2.YpEvents();
  upEvents=g2.UpEvents();
  yeEvents=g2.YeEvents();
  ueEvents=g2.UeEvents();

  // Set YP-, UP-, YE-, UE-events  in g1
  g1.SetYp(ypEvents);
  g1.SetUp(upEvents);
  g1.SetYe(yeEvents);
  g1.SetUe(ueEvents);
  
  // Now, also g1 is in HioPlantForm
  std::cout << "######################################\n";
  std::cout << "# IsHioPlantForm:\n";
  IsHioPlantForm(g1,report);
  std::cout<<report;
  std::cout << "######################################\n";

  // File i/o and access to attributes are analogous for HioConstraint, HioController and HioEnvironment.
  std::cout<<std::endl<<std::endl;
 /**************************************************
 * HIO CONTROLLER
 ***************************************************/

  // Construct simple HioController structure
  HioController c1;
  c1.Name("HioController");
  Idx yp1=c1.InsYpEvent("yp1");
  Idx yp2=c1.InsYpEvent("yp2");
  Idx yc=c1.InsYcEvent("yc");
  Idx uc1=c1.InsUcEvent("uc1");
  Idx uc2=c1.InsUcEvent("uc2");
  Idx up=c1.InsUpEvent("up");
  
  Idx st1=c1.InsInitState();
  c1.SetMarkedState(st1);
  Idx st2=c1.InsMarkedState();
  Idx st3=c1.InsMarkedState();
  Idx st4=c1.InsMarkedState();
  
  c1.SetTransition(st1,yp1,st2);
  c1.SetTransition(st2,yc,st3);
  c1.SetTransition(st3,uc1,st4);
  c1.SetTransition(st4,up,st1);

  // Up to now, no I/O-controller form as inputs yp2 and uc2 are not accepted:
  std::cout<<std::endl<<"######################################\n";
  std::cout<<"######################################\n";
  std::cout<<"####### I/O CONTROLLER: #########\n";
  tmpresult=IsHioControllerForm(c1,report);
  FAUDES_TEST_DUMP("IsHioControllerForm fail",tmpresult);
  
  // Test correct file I/O
  c1.Write();
  c1.Write("tmp_hiocontroller_incomplete.gen");
  c1.Read("tmp_hiocontroller_incomplete.gen");
  c1.Write();
  std::cout<<report;
  c1.GraphWrite("tmp_hiocontroller_incomplete.png");
  
  // Repair HioControllerForm using HioFreeInput
  HioFreeInput(c1,c1);
  tmpresult=IsHioControllerForm(c1,report);
  FAUDES_TEST_DUMP("IsHioControllerForm pass",tmpresult);
  std::cout<<report;
  FAUDES_TEST_DUMP("HioController construction",c1);
  c1.Write("tmp_hiocontroller_repaired.gen");
  c1.GraphWrite("tmp_hiocontroller_repaired.png");
  std::cout<<"######################################\n";
  std::cout<<std::endl<<std::endl;
  
 /**************************************************
 * HIO ENVIRONMENT
 ***************************************************/

  // Construct simple HioEnvironment structure
  HioEnvironment e1;
  e1.Name("HioEnvironment");
  Idx ye1=e1.InsYeEvent("ye1");
  Idx ye2=e1.InsYeEvent("ye2");
  Idx yl=e1.InsYlEvent("yl");
  Idx ul1=e1.InsUlEvent("ul1");
  Idx ul2=e1.InsUlEvent("ul2");
  Idx ue=e1.InsUeEvent("ue");
  
  st1=e1.InsInitState();
  e1.SetMarkedState(st1);
  st2=e1.InsMarkedState();
  st3=e1.InsMarkedState();
  st4=e1.InsMarkedState();
  
  e1.SetTransition(st1,ye1,st2);
  e1.SetTransition(st2,yl,st3);
  e1.SetTransition(st3,ul1,st4);
  e1.SetTransition(st4,ue,st1);

  // Up to now, no I/O-environment form as inputs ye2 and ul2 are not accepted:
  std::cout<<std::endl<<"######################################\n";
  std::cout<<"######################################\n";
  std::cout<<"####### I/O ENVIRONMENT: #########\n";
  tmpresult=IsHioEnvironmentForm(e1,report);
  FAUDES_TEST_DUMP("IsHioEnvironmentForm fail",tmpresult);
  // Test correct file I/O  
  e1.Write();
  e1.Write("tmp_hioenvironment_incomplete.gen");
  e1.Read("tmp_hioenvironment_incomplete.gen");
  e1.Write();
  std::cout<<report;
  e1.GraphWrite("tmp_hioenvironment_incomplete.png");
  
  // Repair HioEnvironmentForm using HioFreeInput
  HioFreeInput(e1,e1);
  tmpresult=IsHioEnvironmentForm(e1,report);
  FAUDES_TEST_DUMP("IsHioEnvironmentForm pass",tmpresult);
  e1.Write();
  std::cout<<report;
  FAUDES_TEST_DUMP("HioEnvironment construction",e1);
  e1.Write("tmp_hioenvironment_repaired.gen");
  e1.GraphWrite("tmp_hioenvironment_repaired.png");
  std::cout<<"######################################\n";
  std::cout<<std::endl<<std::endl;
 /**************************************************
 * HIO CONSTRAINT
 ***************************************************/
  // Construct simple HioConstraint structure
  HioConstraint cnstr1;
  cnstr1.Name("HioConstraint");
  Idx y1=cnstr1.InsYEvent("y1");
  Idx y2=cnstr1.InsYEvent("y2");
  Idx u=cnstr1.InsUEvent("u");
  
  st1=cnstr1.InsInitState();
  cnstr1.SetMarkedState(st1);
  st2=cnstr1.InsMarkedState();
  
  cnstr1.SetTransition(st1,y1,st2);
  cnstr1.SetTransition(st2,u,st1);

  // Up to now, no I/O-constraint form as input u2 is not accepted:
  std::cout<<std::endl<<"######################################\n";
  std::cout<<"######################################\n";
  std::cout<<"####### I/O CONSTRAINT: #########\n";
  std::cout<<"#######\n";
  // Access to event properties:
  EventSet yEvents=cnstr1.YEvents();
  yEvents.Name("####### HioConstraint: Y-Events");
  yEvents.Write();
  tmpresult=IsHioConstraintForm(cnstr1,report);
  FAUDES_TEST_DUMP("IsHioConstraintForm fail",tmpresult);
  // Test correct file I/O
  cnstr1.Write();
  cnstr1.Write("tmp_hioconstraint_incomplete.gen");
  cnstr1.Read("tmp_hioconstraint_incomplete.gen");
  cnstr1.Write();
  std::cout<<report;
  cnstr1.GraphWrite("tmp_hioconstraint_incomplete.png");
  
  // Repair HioEnvironmentForm using HioFreeInput
  HioFreeInput(cnstr1,cnstr1);
  tmpresult=IsHioConstraintForm(cnstr1,report);
  FAUDES_TEST_DUMP("IsHioConstraintForm pass",tmpresult);
  FAUDES_TEST_DUMP("HioController construction",cnstr1);
  cnstr1.Write();
  std::cout<<report;
  cnstr1.Write("tmp_hioconstraint_repaired.gen");
  cnstr1.GraphWrite("tmp_hioconstraint_repaired.png");
  std::cout<<"######################################\n";
  
  return 0;
}



