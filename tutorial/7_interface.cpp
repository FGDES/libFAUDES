/** @file 7_interface.cpp

Tutorial, runtime interface. This tutorial demonstrates access to 
libFAUDES data types and functions via the type- and function
registries. The runtime interface addresses the development
of applications that transparently propagate libFAUDES extensions
to the user. 


@ingroup Tutorials 

@include 7_interface.cpp

*/

#include <iostream>
#include "libfaudes.h"

using namespace faudes;


int main(){

  // ******************** basic ussage of the faudes type interface

  // load (do so once on application startup)
  LoadRegistry("../include/libfaudes.rti");

  // report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, faudes types \n";

  // instatiate an object by faudes type
  Type* fobject = NewFaudesObject("System");  

  // query type of an object
  std::cout << " faudes type of object: " << FaudesTypeName(*fobject) << "\n";

  // type test: can we cast this to a plain generator?
  Generator gen;
  bool isvg= ( gen.Cast(fobject)!=NULL );
  if(isvg) 
    std::cout << " faudes object casts to Generator: ok [expected]\n";
  else    
    std::cout << " faudes object casts to Generator: failed\n";

  // type test: can we cast this to an eventset?
  EventSet eset;
  bool ises = ( eset.Cast(fobject) != NULL ) ;
  if(ises) 
    std::cout << " faudes object cast to EventSet: ok\n";
  else    
    std::cout << " faudes object cast to EventSet: failed [expected]\n";

  // Record test case
  FAUDES_TEST_DUMP("faudes type",FaudesTypeName(*fobject));
  FAUDES_TEST_DUMP("cast to generator",isvg);
  FAUDES_TEST_DUMP("cast to eventset",ises);

  // report
  std::cout << "################################\n";

  // ******************** basic ussage of the function registry

  // instatiate some generators via the registry 
  Type* data0 = NewFaudesObject("Generator");  
  Type* data1 = NewFaudesObject("Generator");  
  Type* data2 = NewFaudesObject("Generator");  

  // load input data from files
  data0->Read("./data/simplemachine.gen");
  data1->Read("./data/buffer.gen");

  // instantiate a function via registry 
  Function* funct = NewFaudesFunction("Parallel");  
  
  // set parameter values (exception on accessing positional parameter out of range)
  funct->ParamValue(0,data0);
  funct->ParamValue(1,data1);
  funct->ParamValue(2,data2);

  // execute function (exception on type mismatch)
  funct->Execute();

  // report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, rti parallel \n";
  data2->Write();
  std::cout << "################################\n";

  // test case
  Generator* vgen=dynamic_cast<Generator*>(data2);
  if(!vgen) {
    std::cout << "ERR: res does not cast to Generator\n";
    exit(1);
  }
  if(vgen->Size()!=6) {
    std::cout << "ERR: res is expected to have 6 states\n";
    exit(1);
  }

  // record test case
  FAUDES_TEST_DUMP("rti parallel",*data2);

  // clear registry for below demos
  ClearRegistry();

  // delete my objects
  delete data0;
  delete data1;
  delete data2;
 
  // ******************** elementary type, String

  // create new String instance
  String fstring;

  // assign from corresponding c type std::string
  fstring = "hello faudes types";

  // assign to corresponding c type std::string
  std::string sstring = fstring;

  // file io
  fstring.Write("tmp_string.txt");
  fstring.Read("tmp_string.txt");

  // report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, rti string \n";
  fstring.Write();
  std::cout << "################################\n";


  // ******************** elementary type, Integer

  // create new Integer instance
  Integer fint;

  // assign from corresponding c type std::int
  fint = 42;

  // assign to corresponding c type std::int
  int sint = fint;

  // arithmetic
  fint = 2*fint+16;

  // file io
  fint.Write("tmp_int.txt");
  fint.Read("tmp_int.txt");

  // report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, rti integer \n";
  fint.Write();
  std::cout << "################################\n";

  // test case
  if(fint!=100) {
    std::cout << "ERR: expected 100\n";
    exit(1);
  }


  // ******************** elementary type, Boolean

  // create new Integer instance
  Boolean fbool;

  // assign from corresponding c type std::int
  fbool = false;

  // assign to corresponding c type std::int
  bool sbool = !fbool;

  // file io
  fbool.Write("tmp_bool.txt");
  fbool.Read("tmp_bool.txt");

  // report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, rti bool \n";
  fbool.Write();
  std::cout << "################################\n";


  // ******************** advanced: type definitions and registry
  
  // defining a type with specified faudes type name (and no documentation)
  TypeDefinition* tdalphabet = TypeDefinition::Constructor<Alphabet>("EventSet");

  // defining a type with type name "System" and documentation from specified file
  TypeDefinition* tdgenerator = TypeDefinition::FromFile<System>("data/generator.rti");
 

  // register previously defined types
  TypeRegistry::G()->Insert(tdalphabet);
  TypeRegistry::G()->Insert(tdgenerator);

  // load any additional documentation from file (event set is missing from above)
  TypeRegistry::G()->MergeDocumentation("../include/libfaudes.rti");

  // dump registry to console
  std::cout << "################################\n";
  std::cout << "# tutorial, type registry \n";
  TypeRegistry::G()->Write();
  std::cout << "################################\n";

  // use the registry to construct an object by its type name
  Type* generator = NewFaudesObject("System");  

  // test generator object
  if(dynamic_cast<System*>(generator)) 
    std::cout << "Faudes object casts to System: OK [expected]\n";
  else
    std::cout << "Faudes object does not casts to System: ERR [test case error]\n";

  // recover faudes type name from object
  std::cout << "Its a \"" << FaudesTypeName(*generator) << "\"\n";

  // delete my objects
  delete generator;

  // done
  std::cout << "################################\n";

  return(0);
}
