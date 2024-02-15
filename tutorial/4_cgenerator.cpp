/** @file 4_cgenerator.cpp

Tutorial, generator with controllability attributes


The faudes::System class is inherited from the faudes::Generator class and only
differs by definition of the controllabilty flags for events. Because 
of the inheritance, the System class's methods are a superset
of the Generator.  This tutorial demonstrates  System specific methods.


@ingroup Tutorials 

@include 4_cgenerator.cpp

*/

#include "libfaudes.h"


// we make the faudes namespace available to our program
using namespace faudes;


/////////////////
// main program
/////////////////

int main() {


  ////////////////////////////////////////////
  // Constructors & Assignment Operator
  ////////////////////////////////////////////

  // Create an empty System object

  System cgen1;

  // Create a System by reading a Generator file 
  // Events default to observable and uncontrollable

  System cgen2("data/simplemachine.gen");

  // Create a System by reading a System file 

  System cgen3("data/csimplemachine.gen");

  // Copy constructor & assignment operator
  // (same behaviour as in Generator class)

  System cgen4(cgen3); 
  cgen4 = cgen3;           

  // report
  cgen4.Write("tmp_csimplemachine.gen");
  cgen4.Write();

  ////////////////////////
  // Controllable events
  ////////////////////////

  // Insert an event and set it controllable 

  cgen1.InsControllableEvent(1); // by index
  cgen1.InsControllableEvent("newcevent1"); // by symbolic name

  // Insert an event and set it uncontrollable 

  cgen1.InsUncontrollableEvent(2); // by index
  cgen1.InsUncontrollableEvent("newcevent2"); // by symbolic name

  // Set an existing event controllable

  cgen1.SetControllable(2); // by index
  cgen1.SetControllable((const std::string&) "newcevent2"); // by symbolic name

  // Set an existing event uncontrollable

  cgen1.ClrControllable(1); // by index
  cgen1.ClrControllable((const std::string&) "newcevent1"); // by symbolic name

  // Test wether an event is controllable

  bool bool_eventcontrollable = cgen1.Controllable(1); // by index
  bool_eventcontrollable = cgen1.Controllable("newcevent1"); // by symbolic name

  // Retrieve an EventSet containing all the controllabe events of the
  // System

  EventSet eset_cevents = cgen2.ControllableEvents();

  // Retrieve an EventSet containing all the uncontrollabe events of the
  // System

  EventSet eset_ucevents = cgen2.UncontrollableEvents();


  return 0;
}



