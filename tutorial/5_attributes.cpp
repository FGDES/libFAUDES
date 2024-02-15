/** @file 5_attributes.cpp

Tutorial, attributes. 

The StateSet, the EventSet and the TransitionSet of the Generator
accept a template parameter to specify attributes for the respective entity.
There is also a parameter to specify attributes of the generator itself. This tutorial
illustrates how to use the attribute parameter to distinguish controllabel events. 
Note that there is a more convenient class System for this particular
purpose (see 4_cgenerator.cpp)

@ingroup Tutorials 

@include 5_attributes.cpp

*/



#include "libfaudes.h"


// we make the faudes namespace available to our program

using namespace faudes;


/////////////////
// main program
/////////////////

int main() {

  // convenience typedef for a generator with event flags
  typedef TaGenerator<AttributeVoid,AttributeVoid,AttributeFlags,AttributeVoid> fGenerator; 
  typedef TaEventSet<AttributeFlags> fEventSet;

  // instantiate generator object
  fGenerator fg1;

  // read from std generator file, all attributes take the default value
  fg1.Read("data/simplemachine.gen");

  // set a flag: 1. read the attribute (e.g. by event index)
  AttributeFlags aflag = fg1.EventAttribute(1); 

  // set a flag: 2. use attribute methods to manipulate
  aflag.Set(0x0000000f);

  // set a flag: 3. copy the new attribute to the generators alphabet
  // note: if the attribute turns out to be the default attribute, no
  // memory will be allocated
  fg1.EventAttribute(1,aflag);

  // set a flag: alternatively, use generator method 
  // note that even if the attrute became the default attribute, memory is allocated
  fg1.EventAttributep(1)->Set(0x00000f000);

  // get a flag: use generator method
  AttributeFlags bflag= fg1.EventAttribute(1); 

  // Retrieve a const reference to the Generator's alphabet, includes attributes 
  const fEventSet&  eset_ref_alph = fg1.Alphabet();

  // Retrieve a const reference of the Generator's alphabet without attributes
  const EventSet& set_ref_alph = fg1.Alphabet();

  // Retrieve a copy of the Generator's alphabet without attributes
  EventSet eset_copy_alph = fg1.Alphabet();

  // report flag to console      
  std::cout << "################################\n";
  std::cout << "# tutorial, show flags \n";
  std::cout << bflag.ToString() << "\n";
  std::cout << "################################\n";


  // write to generator file, incl attributes
  fg1.Write("tmp_fsimplemachine.gen");

  // read back
  fg1.Read("tmp_fsimplemachine.gen");

  // report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, show generator with  flags \n";
  fg1.DWrite();
  std::cout << "################################\n";


  // when reading attributed files to std generator, attributes are ignored
  Generator g1;
  g1.Read("tmp_fsimplemachine.gen");

  // report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, show generator without flags \n";
  g1.DWrite();
  std::cout << "################################\n";

}

