/** @file diag_1_eventdiagnosis.cpp
Demonstrate diagnoser structure and methods for std event diagnosability. 
@ingroup Tutorials 
@include diag_1_eventdiagnosis.cpp
*/

#include "diag_include.h"


using namespace std;
using namespace faudes;


int main(void) {

  // ******************** Basic usage of diagnoser objects

  // The diagnoser is derived from the basic generator and, hence,
  // provides std access to states, events and transitions

  // Declare diagnoser 
  Diagnoser d1, d2;

  // Assign name to diagnoser d1
  d1.Name("Diagnoser");
  
  // Insert some states in diagnoser d1
  Idx s1 = d1.InsInitState("s1");
  Idx s2 = d1.InsState("s2");
  Idx s3 = d1.InsState("s3");
    
  // Insert some events in diagnoser d1
  Idx eventA = d1.InsEvent("a");
  Idx eventB = d1.InsEvent("b");
  
  // Set transitions in diagnoser d1
  d1.SetTransition(s1, eventA, s2);
  d1.SetTransition(s2, eventB, s3);
  d1.SetTransition(s3, eventA, s3);

  
  // ******************** Failure type map of diagnoser objects

  // Declare some two failure event sets
  EventSet failures1, failures2;

  failures1.Insert("WPblocked");
  failures1.Insert("WPfelldown");
  failures2.Insert("sfRunsContinuously");
  failures2.Insert("cb1RunsContinuously");
  
  // Introduce failure types to diagnoser
  Idx F1 = d1.InsFailureTypeMapping("F1", failures1);
  Idx F2 = d1.InsFailureTypeMapping("F2", failures2);
  
  // delete all failure types
  // d1.ClearFailureTypeMap();

  
  // ******************** State estimate attributes of diagnoser objects

  // Attatch generator state estimates to the diagnoser states

  // Diagnoser state s1 carries the generator state estimate 1N
  d1.InsStateLabelMapping(s1,1,DiagLabelSet::IndexOfLabelN());
  // Diagnoser state s2 carries the generator state estimate 3F1, 5N
  d1.InsStateLabelMapping(s2,3,F1);
  d1.InsStateLabelMapping(s2,5,DiagLabelSet::IndexOfLabelN());
  // Diagnoser state s3 carries the generator state estimate 7F1F2, 9A
  d1.InsStateLabelMapping(s3,7,F1);
  d1.InsStateLabelMapping(s3,7,F2);
  d1.InsStateLabelMapping(s3,9,DiagLabelSet::IndexOfLabelA());
  
  // delete all state estimates
  // d1.ClearStateAttributes();
    
  // ******************** Diagnoser file io

  // write diagnoser d1 to file
  d1.Write("tmp_diag_diagnoser_1.gen");
  try {
    d1.GraphWrite("tmp_demo_diagnoser_1.svg");
  } catch(faudes::Exception& exception) {
    std::cout << "warning: cannot execute graphviz' dot. " << std::endl;
  } 
  
  // Dead diagnoser d2 from file
  d2.Read("tmp_diag_diagnoser_1.gen");

  // Report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, diagnoser d2\n";
  d2.Write();
  std::cout << "################################\n";

  
  // Report to console
  AttributeDiagnoserState currDStateAttr;
  TaIndexSet<DiagLabelSet> currDStateMap; 
  TaIndexSet<DiagLabelSet>::Iterator currDStateMapIt;
  currDStateAttr = d2.StateAttribute(s3);
  currDStateMap = currDStateAttr.DiagnoserStateMap();
  
  std::cout << "################################\n";
  std::cout << "# tutorial, parsing state estimates for state s3 of d2\n";
  for(currDStateMapIt = currDStateMap.Begin(); currDStateMapIt != currDStateMap.End(); ++ currDStateMapIt){
      cout <<  *currDStateMapIt << " " << currDStateMap.Attribute(*currDStateMapIt).ToString() << endl;
  }  
  std::cout << "################################\n";
  
  // Test protocol
  FAUDES_TEST_DUMP("diagnoser d2", d2);

  // ******************** Failure typemaps

  // AttributeFailureEvents stores a set of failure and indicator events
  AttributeFailureEvents attrFE;
  
  // Add indicator events to attrFE
  attrFE.mIndicatorEvents.Insert("Indicator1");
  attrFE.mIndicatorEvents.Insert("Indicator2");
  // Add failure events to attrFE
  attrFE.mFailureEvents.Insert("Failure1");
  attrFE.mFailureEvents.Insert("Failure2");
  attrFE.mFailureEvents.Insert("Failure3");
  
  // Declare a AttributeFailureTypeMap (to store the failure and indicator partition)
  AttributeFailureTypeMap failureTypes;  
  
  // Insert attrFE in the partition and associate it with the failure type name "FailureType1"
  failureTypes.mFailureTypeMap.Insert("FailureType1",attrFE);

  // Write the failure and indicator partition to console and file
  std::cout << "################################\n";
  std::cout << "# tutorial, failure types\n";
  failureTypes.Write();
  std::cout << "################################\n";
  failureTypes.Write("tmp_diag_failure_typemap_2.txt");

  // ******************** Event-diagnosability with respect to a failure partition 
 
  // Declare needed variables
  System gen;
  Diagnoser diag;
  AttributeFailureTypeMap failurePartition;  
  string reportString;
  
  // Report to console
  std::cout << "################################\n";
  std::cout << "# diagnosability, failure types, system 4 \n";
  std::cout << "# a) read data \n";

  // Read input generator and failure/indicator partition from file
  gen.Read("data/diag_system_4.gen");
  failureTypes.Read("data/diag_failure_typemap_4.txt");
   
  // Write input generator to png file
  try {
    gen.GraphWrite("tmp_demo_system_4.png");
  } catch(faudes::Exception& exception) {
    std::cout << "warning: cannot execute graphviz' dot. " << std::endl;
  } 
  
  // Report to console
  std::cout << "# b) run diagnosability test (expect result FALSE and warning)\n";
  
  // Test generator gen for diagnosability with respect to failure partition failureTypes
  bool isdiagft=IsEventDiagnosable(gen,failureTypes,reportString);
  if(isdiagft){
    cout << "System is diagnosable." << endl;
  } else {
    cout << "System is not diagnosable." << endl;
    cout << reportString << endl;
  }

  // Report to console
  std::cout << "# c) run i-diagnosability test (expect result TRUE)\n";

  // Test protocol
  FAUDES_TEST_DUMP("diag failuretype",isdiagft);

  // Test generator gen for I-diagnosability with respect to failure partition failureTypes
  bool isdiagie=IsIndicatorEventDiagnosable(gen,failureTypes,reportString); 
  if(isdiagie) {
    cout << "System is I-diagnosable." << endl;
  } else {
    cout << "System is not I-diagnosable." << endl;
    cout << reportString << endl;
  }

  // Report to console
  std::cout << "# done \n";
  std::cout << "################################\n";

  // Test protocol
  FAUDES_TEST_DUMP("diag indicator",isdiagie);
  
  
  // ******************** Event-diagnoser synthesis w.r.t. failure types
  
  // Report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, event-diagnoser synthesis, wrt failure types\n";

  // Read the generator from file
  gen.Read("data/diag_system_3.gen");

  // Read the failure partition from file
  failurePartition.Read("data/diag_failure_typemap_3.txt");

  // Write generator to file
  gen.Write("tmp_diag_system_3.gen");
  
  // compute the diagnoser
  EventDiagnoser(gen,failurePartition,diag);
  
  // Write the diagnoser to file
  diag.Write("tmp_diag_diagnoser_3.gen");
  
  // Report result
  std::cout << "Diagnoser statistics\n";
  diag.SWrite();
  std::cout << "################################\n";

  // Test protocol
  FAUDES_TEST_DUMP("synthesis failure types", diag);


  return 0;
}

