/** @file 2_containers.cpp 

Tutorial, container classes. This tutorial demonstrates
how to use the faudes::EventSet and faudes::StateSet containers.

The EventSet class consists of an internal sorted set of unique elements
of integer type faudes::Idx. Events are required to have
globally unique names. For event name resolution, the EventSet holds a 
pointer to a (static) SymbolTable object. File IO is via event names
as opposed to event indices. EventSets are seen as selfcontained.

The StateSet class consists of an internal sorted set of unique elements
of integer type faudes::Idx. StateSets  do *NOT* provide state names
and file IO is via indices only. (Note: generators provide states names,
so you may prefer generator methods for file IO)

@ingroup Tutorials 

@include 2_containers.cpp
*/



#include "libfaudes.h"


using namespace faudes;



int main() {

  ////////////////////////////////////////////////////
  // EventSets
  ////////////////////////////////////////////////////

  // Create EventSet objects	
  EventSet alphabet1;
  EventSet alphabet2;
  EventSet alphabet3;

  // Set names
  alphabet1.Name("A1");
  alphabet2.Name("A2");
  alphabet3.Name("A3");

  // New events can be inserted by calling the Insert method
  // with a symbolic name. If the symbolic name is not kown, it is assigned
  // the next free index in the static EventSymbolTable. It any case,
  // Insert returns the index inserted to the set.
  Idx ev1 = alphabet1.Insert("a");
  Idx ev2 = alphabet1.Insert("b");
  Idx ev3 = alphabet1.Insert("c");
  Idx ev4 = alphabet1.Insert("d");
  Idx ev5 = alphabet2.Insert("c"); // ev3 == ev5
  Idx ev6 = alphabet2.Insert("d"); // ev4 == ev6
  Idx ev7 = alphabet2.Insert("e");
  Idx ev8 = alphabet2.Insert("f");
  Idx ev9 = alphabet2.Insert("g");

  // The event index can be used to refer to existing events. This avoids
  // name lookup.
  alphabet3.Insert(ev1); // "a"
  alphabet3.Insert(ev7); // "e"
  alphabet3.Insert(ev8); // "f"
 
  // Report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, alphabets A1,A2 and A3 \n";
  alphabet1.DWrite();
  alphabet2.DWrite();
  alphabet3.DWrite();
  std::cout << "################################\n";


  // Iterator usage
  EventSet::Iterator eit;
  std::cout << "################################\n";
  std::cout << "# tutorial, iterators \n";
  for (eit = alphabet1.Begin(); eit != alphabet1.End(); eit++) {
    std::cout << alphabet1.SymbolicName(*eit) << ": " << *eit << std::endl;
  }
  std::cout << "################################\n";


  // Read an alphabet from generator file
  alphabet3.Read("data/simplemachine.gen", "Alphabet");

  // Read an alphabet from file at object construction
  EventSet alphabet4("data/simplemachine.gen", "Alphabet");

  // Write a alphabet to file
  alphabet2.Write("tmp_alphabet.txt");

  // Report
  std::cout << "################################\n";
  std::cout << "# tutorial, alphabets of simple machine  \n";
  alphabet4.DWrite();
  std::cout << "################################\n";


  // Set inclusion by overloaded <= operator
  if(alphabet1 <= alphabet2) {
    std::cout << "################################\n";
    std::cout << "alphabet1 includes alphabet2" << std::endl;
    std::cout << "################################\n";
  }
  else {
    std::cout << "################################\n";
    std::cout << "alphabet1 does not include alphabet2" << std::endl;
    std::cout << "################################\n";
  }
	
  // Delete an event by name
  alphabet2.Erase("e");

  // Delete an event by index
  alphabet2.Erase(alphabet2.Index("f"));

  // Clear an eventset
  alphabet4.Clear();

  // Test existence of event
  if (alphabet2.Exists("d")) {
    std::cout << "alphabet2: event d exists" << std::endl;
  }


  // Report
  std::cout << "################################\n";
  std::cout << "# tutorial, updated alphabets 1 and 2  \n";
  alphabet1.DWrite();
  alphabet2.DWrite();
  std::cout << "################################\n";


  // Set difference
  EventSet adifference = alphabet1 - alphabet2;
  std::cout << "################################\n";
  std::cout << "set difference: " << adifference.ToString() << std::endl;
  std::cout << "################################\n";


  // Set union
  EventSet aunion = alphabet1 + alphabet2;
  std::cout << "################################\n";
  std::cout << "set union: " << aunion.ToString() << std::endl;
  std::cout << "################################\n";

  // Set intersection
  EventSet aintersection = alphabet1 * alphabet2;
  std::cout << "################################\n";
  std::cout << "set intersection: " << aintersection.ToString() << std::endl;
  std::cout << "################################\n";

  // Test protocol
  FAUDES_TEST_DUMP("set difference",adifference);
  FAUDES_TEST_DUMP("set union",aunion);
  FAUDES_TEST_DUMP("set intersection",aintersection);


  ////////////////////////////////////////////////////
  // StateSets
  ////////////////////////////////////////////////////


  std::cout << "################################\n";
  std::cout << "# tutorial, state sets \n";

  // Create a StateSet
  StateSet stateset1;

  // Introduce states
  Idx state1 = stateset1.Insert(47);
  Idx state2 = stateset1.Insert(11);
  Idx state3 = stateset1.Insert();    // becomes 48

  // Introduce more states
  for(int i=0; i<25; i++) stateset1.Insert(); // becomes 49 ... 73
  Idx state4 = stateset1.Insert(100);

  // Iterator usage 
  StateSet::Iterator sit;
  std::cout << "stateset1: ";
  for (sit = stateset1.Begin(); sit != stateset1.End(); ++sit) {
    std::cout << stateset1.Str(*sit) << " ";
  }
  std::cout << std::endl;

  // Print as string (using file format)
  std::cout << "stateset1: " << stateset1.ToString() << std::endl;
	
  // Write a stateset to file (section defaults to "IndexSet")
  stateset1.Write("tmp_stateset.txt");

  // Read back from file (section defaults to "current begin token")
  StateSet stateset2;
  stateset2.Read("tmp_stateset.txt");

  // Debug output to console
  stateset2.Write();

  // Delete a state by index
  stateset2.Erase(state2);

  // Copy a StateSet
  StateSet stateset3 = stateset1;

  // Clear a StateSet
  stateset1.Clear();

  // Test existence of state
  if (stateset3.Exists(state1)) {
    std::cout << "stateset3: state " << state1 << " exists" << std::endl;
  }

  std::cout << "################################\n\n";


  ////////////////////////////////////////////////////
  // advanced topic: attributed sets
  ////////////////////////////////////////////////////

       
  std::cout << "################################\n";
  std::cout << "# tutorial, attributes \n";

  // Convenience type definition for states with flag attribute (see attributes.h)
  typedef TaStateSet<AttributeFlags> FStateSet;

  FStateSet fstateset1();

  /*
  // Construct from a file (read attributes if specified)
  FStateSet fstateset1("tmp_stateset.txt");


  // Construct from stateset with no attributes
  FStateSet fstateset3(stateset3);


  // Report
  std::cout << "fstateset3: " << fstateset3.ToString() << std::endl;
	
  // Manipulate attribute by state index (this requires the state to exist)
  fstateset3.Attributep(60)->Set(0xff);

  // Insert new state with attribute
  AttributeFlags fattr;
  fattr.Set(0x55);
  Idx fstate = fstateset3.Insert(fattr);

  // Report
  std::cout << "fstateset3: attribute of state 60: " 
            << fstateset3.Attribute(60).ToString() << std::endl;
  std::cout << "fstateset3: attribute of state " << fstate 
	    << ": " << fstateset3.Attribute(fstate).ToString() << std::endl;
  std::cout << "fstateset3: " << fstateset3.ToString() << std::endl;

  // Write to file 
  fstateset3.Write("tmp_fstateset.txt");	

  // Convert to set without attributes (drop attributes)
  stateset3 = fstateset3;

  // Report
  std::cout << "stateset3: " << stateset3.ToString() << std::endl;
       
  // Set comparision ignores attributes
  if(stateset3==fstateset3) 
    std::cout << "stateset3 indeed equals fstateset3 " << std::endl;

  // Explicit equality test shows
  if(!stateset3.EqualAttributes(fstateset3)) 
    std::cout << "stateset3 indeed has different attributes as fstateset3 " << std::endl;

  // Provided that actual types match, attributes are copied even when accesssing 
  // via non attributed StateSet methods
  FStateSet fstateset4;
  StateSet& rfstateset3 = fstateset3;
  StateSet& rfstateset4 = fstateset4;
  rfstateset4 = rfstateset3;

  // Remove a state with no attribute (provoce deep copy)
  rfstateset3.Erase(50);
    
  // Test attribute equality
  if(fstateset4.EqualAttributes(fstateset3)) 
    std::cout << "fstateset4 indeed has equal attributes with fstateset3 " << std::endl;
  if(rfstateset4.EqualAttributes(rfstateset3)) 
    std::cout << "rfstateset4 indeed has equal attributes with rfstateset3 " << std::endl;

  // Report
  std::cout << "fstateset4: " << fstateset4.ToString() << std::endl;
  std::cout << "################################\n\n";

  // Test protocol
  FAUDES_TEST_DUMP("attrbibutes eq0", stateset3 == fstateset3);
  FAUDES_TEST_DUMP("attrbibutes eq1", stateset3.EqualAttributes(fstateset3));
  FAUDES_TEST_DUMP("attrbibutes eq2", fstateset4.EqualAttributes(fstateset3));
  FAUDES_TEST_DUMP("attrbibutes eq3", rfstateset4.EqualAttributes(rfstateset3));


  ////////////////////////////////////////////////////
  // Vectors
  ////////////////////////////////////////////////////

  std::cout << "################################\n";
  std::cout << "# tutorial, vectors \n";

  // Have a vector of event sets
  EventSetVector alphvect;

  // Populate the vector (take copies)
  alphvect.PushBack(alphabet1);
  alphvect.PushBack(alphabet2);
  alphvect.PushBack(alphabet3);
 
  // Access entries
  std::cout << "# event set no 1 (counting from 0):\n";
  alphvect.At(1).Write();
  
  // Manipulate entries
  alphvect.At(1).Insert("delta_1");

  // Report
  std::cout << "# all three event sets:\n";
  alphvect.Write();

  // Set filenames for convenient token io
  alphvect.FilenameAt(0,"tmp_alph0.txt");
  alphvect.FilenameAt(1,"tmp_alph1.txt");
  alphvect.FilenameAt(2,"tmp_alph2.txt");
  alphvect.Write("tmp_alphvect.txt");


  // query, whether the vector can take an element
  System cgen;
  bool vcast = alphvect.ElementTry(cgen);
  if(vcast) 
    std::cout << "# EventSetVector can take Generator elements [fail]\n";
  else
    std::cout << "# EventSetVector cannot take Generator elements [expected]\n";

  // record
  FAUDES_TEST_DUMP("vect element cast",vcast);


  // Done
  std::cout << "################################\n";

  */
  
  ////////////////////////////////////////////////////
  // Developper internal: deferred copy stress test
  ////////////////////////////////////////////////////


  std::cout << "################################\n";
  std::cout << "# tutorial, deferred copy stress test \n";

  // Create state set {1,2,...44}
  StateSet setA;
  for(Idx state=1; state<45; state++) {
    setA.Insert(state);
  }
  setA.Write();

  // Have a deferred copy
  StateSet setB=setA;

  // Test protocol
  FAUDES_TEST_DUMP("deferred copy A",setA);
  FAUDES_TEST_DUMP("deferred copy B",setB);

  // Collect iterators
  std::vector<StateSet::Iterator> edIts;

  // Investigate deferred copy setB
  StateSet::Iterator cit=setB.Begin(); 
  for(;cit!=setB.End(); cit++) {
    if(*cit % 5 !=0) continue;
    edIts.push_back(cit);
  } 
  
  // Test protocol
  FAUDES_TEST_DUMP("deferred copy A - 2",setA);
  FAUDES_TEST_DUMP("deferred copy B - 2",setB);

  // setB should share with setA and have quite some iterators
  setB.DWrite();

  // Trigger detach and lock set B
  setB.Lock();

  // Further investigate true copy of setB
  cit=setB.Begin(); 
  for(;cit!=setB.End(); cit++) {
    if(*cit % 5 ==0) continue;
    if(*cit % 2 !=0) continue;
    edIts.push_back(cit);
  } 
  
  // setB neither shares nor track iterators
  setB.DWrite();

  // Have other deferred copy
  StateSet setC=setB;

  // Write on the deferred copy to trigger actual copy 
  setC.Insert(77);

  // Perform edit on deferred copy
  std::vector<StateSet::Iterator>::iterator iit=edIts.begin();
  for(;iit!=edIts.end(); iit++) {
    Idx oldstate = **iit;
    setC.Erase(oldstate); 
    setC.Insert(100+oldstate); 
  }
     
  // setB should not share and still dont track any iterators
  setB.Write();
  setB.DWrite();
  std::cout << "################################\n";
     
  // Test protocol
  FAUDES_TEST_DUMP("deferred copy A - 3",setA);
  FAUDES_TEST_DUMP("deferred copy B - 3",setB);
  FAUDES_TEST_DUMP("deferred copy C - 3",setC);


  ////////////////////////////////////////////////////
  // Developper internal: memory leak in BaseSet
  ////////////////////////////////////////////////////

  /*
  EventSet evs2;
  Generator gsm2;
  evs2.Insert("alpha");
  Generator gsm("data/simplemachine.gen");
  for(int i=0;i<2000000;i++) {
  for(int j=0;j<20000;j++) {
    gsm.IsCoaccessible();
    Deterministic(gsm,gsm2);
    Project(gsm,evs2,gsm2);
  }
  }
  */

  return 0;
}
