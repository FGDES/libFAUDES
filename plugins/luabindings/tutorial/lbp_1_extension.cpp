/** @file lbp_1_extension.cpp 

Registering a lua script with the libFAUDES run-time interface 

This tutorial loads a luascript from file an registers it
with the libFAUDES run-time interface. Thus, the script can be
invoked by an application in the same way as built in functions.

@ingroup Tutorials

@include lbp_1_extension.cpp

*/



// libfaudes include
#include "libfaudes.h"

// we make the faudes namespace available to our program
using namespace faudes;


/////////////////
// main program
/////////////////


int main (int argc, char **argv) {

  // ******************** load/inspect lua function definition

  // load std registry for data types
  LoadRegistry("../../../include/libfaudes.rti");

  // initialize a lua function definition from file
  LuaFunctionDefinition lfdef1;
  lfdef1.Read("data/largegen.rti");

  // report to console
  std::cout << "################################\n";
  std::cout << "# lua extension from rti file\n";
  lfdef1.Write();
  std::cout << "################################\n";

  // run syntax check on script
  std::string err=lfdef1.SyntaxCheck();
  if(err=="")
    std::cout << "script seems ok\n";
  else {
    std::cout << err << "\n";
    return 1;
  }

  // test faudes type interface (assignment/equality)
  LuaFunctionDefinition lfdef2;
  lfdef2 = lfdef1;
  if(lfdef2!=lfdef1) {
    std::cout << "# ERR: the two definitions should be equal\n";
    exit(1);
  }

  // report to console
  std::cout << "################################\n";
  std::cout << "# copy of lua extension\n";
  lfdef2.Write();
  std::cout << "################################\n";


  // ******************** execute lua function via rti 

  // instantiate a function object 
  faudes::Function* fnct = lfdef2.NewFunction();

  // prepare arguments
  Integer cntq=10;
  Integer cnts=5;
  Generator gres;

  // set argumants
  fnct->Variant(0);
  fnct->ParamValue(0,&cntq);
  fnct->ParamValue(1,&cnts);
  fnct->ParamValue(2,&gres);

  // execute lua script
  fnct->Execute();

  // report statistics on result
  std::cout << "################################\n";
  std::cout << "# statistics \n";
  gres.SWrite();
  std::cout << "################################\n";

  // report test case
  FAUDES_TEST_DUMP("large gen",gres);


  // ******************** execute lua function via rti 

  // load/execute another function definition from file
  NameSet sres;
  LuaFunctionDefinition lfdef3;
  lfdef3.Read("data/xtractalph.rti");
  lfdef3.Write();
  faudes::Function* fnct3 = lfdef3.NewFunction();
  fnct3->Variant(0);
  fnct3->ParamValue(0,&gres);
  fnct3->ParamValue(1,&sres);
  fnct3->Execute();

  // on result
  std::cout << "################################\n";
  std::cout << "# alphabet \n";
  sres.Write();
  std::cout << "################################\n";

  // report test case
  FAUDES_TEST_DUMP("alphabet",sres);



  // ******************** advanced: C++ rtti / vtables / multiple inheritance

  // experiment with compiler's memory layout (base matches void, intermediate cast is fine)
  Generator* vgen_vgen = new Generator();
  Type*       vgen_ftype = ((Type*)(vgen_vgen));
  void*       vgen_void = ((void*)(vgen_vgen));
  void*       vgen_void_ftype = ((void*)(vgen_ftype));
  std::cout << "C++ casting of a Generator: " <<
    " vgen at " << &vgen_vgen << ", ftype at " << vgen_ftype << ", void at " << vgen_void << 
    ", void via ftype at " << vgen_void_ftype <<  "\n";

  // experiment with compiler's memory layout (base does not match void for multiple inheritance)
  Alphabet* cevs_cevs = new Alphabet();
  Type*       cevs_ftype = ((Type*)(cevs_cevs));
  void*       cevs_void = ((void*)(cevs_cevs));
  void*       cevs_void_ftype = ( (void*)(cevs_ftype));
  void*       cevs_dvoid_ftype = ( dynamic_cast<void*>(cevs_ftype));
  std::cout << "C++ casting a Alphabet: " <<
    " cevs at " << &cevs_cevs << ", ftype at " << cevs_ftype << ", void at " << cevs_void << 
    ", void via ftype at " << cevs_void_ftype << 
    ", dynamic-void via ftype at " << cevs_dvoid_ftype <<  "\n";

  // conclusion: depending on the compiler, casting to void* via different paths may 
  // very well lead to a different result; when using dynamic_cast however, the compiler
  // has a chance and (at least) gcc manages; thus, dynamic cast is not only for down casts,
  // but also for up casts.

  // record test case
  FAUDES_TEST_DUMP("dynamic up-cast (a)", vgen_void_ftype== vgen_void);
  FAUDES_TEST_DUMP("dynamic up-cast (a)", cevs_dvoid_ftype== cevs_void);

  // delete my objects
  delete vgen_vgen;
  delete cevs_cevs;


}


