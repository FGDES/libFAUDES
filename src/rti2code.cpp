/** rti2code.cpp  Utility to generate registry initialisation code from rti files */

/* FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2009 Ruediger Berndt
Copyright (C) 2010, 2023, 2025, 2026  Thomas Moor

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */
	 

#include <string>
#include <iostream>
#include <fstream>
#include "corefaudes.h"


using namespace faudes;

// ******************************************************************
// error exit
// ******************************************************************

void usage(const std::string& rMessage="") {

  // UI hints
  if(rMessage!="") {
    std::cerr << rMessage << std::endl;
    std::cout << "" << std::endl;
  }
  std::cerr << "rti2code: " << VersionString()  << std::endl;
  std::cerr << std::endl;
  std::cerr << "utility to generates c code from an rti-file to " << std::endl;
  std::cerr << "1) extract c declarations for bindings interface code," << std::endl;
  std::cerr << "2) register faudes-types and -functions with the run-time interface," << std::endl;
  std::cerr << "3) generate convenience wrappers for scripting languages" << std::endl;
  std::cerr << std::endl;
  std::cerr << "usage:" << std::endl;
  std::cerr << " rti2code [-swig|-loader|-wrapper] <rti input file> <output basename>" << std::endl;
  std::cerr << " rti2code -merge <rti input files> <output rti-file>" << std::endl;
  std::cerr << std::endl;
  std::cerr << "[note: the -loader flag will produce .h/.cpp files to instantiate prototypes" << std::endl;
  std::cerr << "[note: the -wrapper flag will produce .h/.cpp files to declare/define wrappers" << std::endl;
  std::cerr << "[note: the -swig flag will produce a .i file for swig include" << std::endl;
  std::cerr << "[note: the -flat flag circumvents an issue with SWIG pre 4.1.0]" << std::endl;
  exit(1);
}


// ******************************************************************
// main programm
// ******************************************************************

int main(int argc, char *argv[]) {

  // config
  bool loader=false;
  bool swig=false;
  bool flat=false;
  bool wrapper=false;
  bool merge=false;

  // primitive command line parser
  int pos=1;
  for(; pos<argc; pos++) {
    std::string option(argv[pos]);
    // option: help
    if((option=="-?") || (option=="--help")) {
      usage();
      continue;
    }
    // option: loader
    if(option=="-loader") {
      loader=true;
      continue;
    }
    // option: swig
    if(option=="-swig") {
      swig=true;
      continue;
    }
    // option: wrapper
    if(option=="-wrapper") {
      wrapper=true;
      continue;
    }
    // option: flat
    if(option=="-flat") {
      flat=true;
      continue;
    }
    // option: merge
    if(option=="-merge") {
      merge=true;
      continue;
    }
    // option: unknown
    if(option.c_str()[0]=='-') {
      usage("unknown option "+ option);
      continue;
    }
    // break for args
    break;
  }

  // mode test
  if(merge && (loader || swig || wrapper)) {
    usage("mismatching options: either merge or code generation");
  }    
  if(!merge && !(loader || swig || wrapper)) {
    usage("mismatching options: either merge or code generation");
  }    
  if(flat && !swig) {
    usage("mismatching options: flat is only applicable to swig");
  }    
  if(merge && (argc-pos <2)) {
    usage("mismatching agruments: to few files to merge");
  }
  if(!merge && (argc-pos !=2)) {
    usage("mismatching agruments: need one source and one destination for code generation");
  }

  // Merge mode -- get this done first
  if(merge) {
    // Load from files
    for(; pos< argc-1; pos++) {
      TypeRegistry::G()->MergeDocumentation(std::string(argv[pos]));
      FunctionRegistry::G()->MergeDocumentation(std::string(argv[pos]));
    }
    // Dump
    if(std::string(argv[argc-1]) != "-") {
      SaveRegistry(std::string(argv[argc-1]));
    } else {
      SaveRegistry();
    }
    return 0;
  }

  // code-gen modes loader/swig
  LoadRegistry(argv[pos++]);

  // Code output streams
  std::ofstream rtiheader;
  std::ofstream rticode;
  std::ofstream swigheader;
  std::ofstream wrpheader;
  std::ofstream wrpcode;
  if(loader) {
    rtiheader.open((std::string(argv[pos])+".h").c_str(), std::ios::out);
    rticode.open((std::string(argv[pos])+".cpp").c_str(), std::ios::out);
  }
  if(swig){
    swigheader.open((std::string(argv[pos])+".i").c_str(), std::ios::out);
  }
  if(wrapper) {
    wrpheader.open((std::string(argv[pos])+".h").c_str(), std::ios::out);
    wrpcode.open((std::string(argv[pos])+".cpp").c_str(), std::ios::out);
  }

  // Introduce myself
  if(loader) {
    rtiheader << "/* rti2code: autogenerated libFAUDES rti registration: "; 
    rtiheader << VersionString() << " "  << PluginsString() << " */" << std::endl << std::endl; 
    rticode   << "/* rti2code: autogenerated libFAUDES rti registration: "; 
    rticode   << VersionString() << " "  << PluginsString() << " */" << std::endl << std::endl;
  }
  if(swig) {
    swigheader << "/* rti2code: autogenerated libFAUDES swig bindings declarations: "; 
    swigheader << VersionString() << " "  << PluginsString() << " */" << std::endl << std::endl;
  }
  if(wrapper) {
    wrpheader << "/* rti2code: autogenerated convenioence wrapper: "; 
    wrpheader << VersionString() << " "  << PluginsString() << " */" << std::endl << std::endl; 
    wrpcode   << "/* rti2code: autogenerated convenioence wrapper: "; 
    wrpcode   << VersionString() << " "  << PluginsString() << " */" << std::endl << std::endl;
  }

  // Generate C++ static factory objects (intro)
  if(loader) {
    rticode << "namespace faudes {" << std::endl;
    rticode << "/* Register faudes types */" << std::endl;
  }

  // Generate C++ static factory objects (iterate types)
  if(loader) {
    TypeRegistry::Iterator  tit;
    int tcnt;
    for(tit=TypeRegistry::G()->Begin(), tcnt=1; tit!=TypeRegistry::G()->End();tit++,tcnt++) {
      // Get c/f type 
      std::string ctype=tit->second->CType();
      std::string ftype=tit->second->Name();
      // Bail out if no C type specified
      if(ctype=="") continue;
      // Remove name space faudes
      size_t pos=ctype.find("faudes::");
      if(pos!=std::string::npos) 
        ctype=ctype.substr(std::string("faudes::").length());
      // Bail out if programmatically registered (default as of libFAUDES 2.34a
      if(tit->second->AutoRegistered()) continue;
      // report
      std::cout << "rti2code: generating auto-registration code for \"" << ftype << "\"" << std::endl;
      // Produce c code
      std::string rtiname = std::string("gRti") + ToStringInteger(tcnt) + "Register" + ftype;
      rticode << "AutoRegisterType<" << ctype << "> " << rtiname << "(\"" << ftype <<"\");";
      rticode << std::endl;
      // Extra data set: element tag
      if(tit->second->ElementTag()!="") {
        rtiname = std::string("gRti") + ToStringInteger(tcnt) + "ElementTag" + ftype;
        rticode << "AutoRegisterElementTag<" << ctype << "> " << rtiname << "(\"" << ftype << 
          "\", \"" << tit->second->ElementTag() << "\");";
        rticode << std::endl;
      }
      // Extra data set: element tag
      if(tit->second->ElementType()!="") {
        rtiname = std::string("gRti") + ToStringInteger(tcnt) + "ElementType" + ftype;
        rticode << "AutoRegisterElementType<" << ctype << "> " << rtiname << "(\"" << ftype << 
          "\", \"" << tit->second->ElementType() << "\");";
        rticode << std::endl;
      }
    }
  }

  // Generate C++ static factory objects (end)
  if(loader) {
    rticode << "} // namespace" << std::endl;
  }

  // C++ function declarations: load types 
  if(loader) {
    rtiheader << "namespace faudes {" << std::endl;
    rtiheader << "void LoadRegisteredTypes(void);" << std::endl;
    rtiheader << "} // namespace" << std::endl;
  }

  // C++ function definition: load types
  if(loader) {
    rticode << "namespace faudes {" << std::endl;
    rticode << "/* Register faudes types */" << std::endl;
    rticode << "void LoadRegisteredTypes(void) {" << std::endl;
  }

  // Traverse type registry to figure faudes types
  if(loader) {
    TypeRegistry::Iterator  tit;
    for(tit=TypeRegistry::G()->Begin(); tit!=TypeRegistry::G()->End();tit++) {
      // Get C type 
      std::string ctype=tit->second->CType();
      // Bail out if no c type specified
      if(ctype=="") continue;
      // Remove name space faudes
      size_t pos=ctype.find("faudes::");
      if(pos!=std::string::npos) 
        ctype=ctype.substr(std::string("faudes::").length());
      // Bail out if programmatically registered (default as of libFAUDES 2.34a
      if(tit->second->AutoRegistered()) continue;
     // Report
      std::cout << "rti2code: generating registration code for \"" << tit->second->Name() << "\"" << std::endl;
      // Produce c code
      rticode << "  TypeRegistry::G()->Insert<" << ctype << ">(\"" << tit->second->Name() <<"\");";
      rticode << std::endl;
    }
  }

  // C++ function definition: load types end
  if(loader) {
    rticode << "}" << std::endl;
    rticode << "} // namespace" << std::endl;
  }


  // C++ function declaration: load functions
  if(loader) {
    rtiheader << "namespace faudes {" << std::endl;
    rtiheader << "void LoadRegisteredFunctions(void);" << std::endl;
    rtiheader << "} // namespace" << std::endl;
  }

  // C++ function definition: load functions
  if(loader) {
    rticode << "namespace faudes {" << std::endl;
    rticode << "/* Register faudes functions */" << std::endl;
    rticode << "void LoadRegisteredFunctions(void) {" << std::endl;
  }

  // C++ class definition: Function derivates 
  if(loader) {
    rtiheader << "namespace faudes {" << std::endl;
  }

  // C++ wrapper: intro
  if(wrapper) {
    wrpcode << "namespace faudes {" << std::endl;
    wrpheader << "namespace faudes {" << std::endl;
  }
  
   // Traverse function registry: define rti function objects
  int fcnt=0;
  FunctionRegistry::Iterator  fit;
  for(fit=FunctionRegistry::G()->Begin(); fit!=FunctionRegistry::G()->End();fit++, fcnt++) {
    // Current function definition
    const FunctionDefinition* fdef = fit->second;
    // Get C type and faudes function name
    std::string ctype=fdef->CType();
    std::string fname = fdef->Name();  
    // Bail out if no c type specified
    if(ctype=="") continue;
    // Remove name space faudes
    size_t pos=ctype.find("faudes::");
    if(pos!=std::string::npos) 
      ctype=ctype.substr(std::string("faudes::").length());
    // Bail out if no signature
    if(fdef->VariantsSize()==0) {
      std::cout << "rti2cocde: function registration: " << fname << ": no signatures" << std::endl;
      continue;
    }

    
    // Interpret signatures: set up types array 
    std::vector< std::vector<std::string> > cparams;
    std::vector< std::vector<Parameter::ParamAttr> > cattrib;
    std::vector< std::vector<bool> > cretval;
    // Loop all signatures
    for(int i=0; i<fdef->VariantsSize(); i++) {
      const Signature& sigi=fdef->Variant(i);
      std::vector<std::string> cparamsi;
      std::vector<Parameter::ParamAttr> cattribi;
      std::vector<bool> cretvali;
      int retcount=0;
      for(int j=0; j<sigi.Size(); j++) {
        // Retrieve faudes type and attrib
        std::string ftype=sigi.At(j).Type();
	Parameter::ParamAttr fattr=sigi.At(j).Attribute();
	bool fcret=sigi.At(j).CReturn();   
        // Count ret values
        if(fcret) retcount++;
        // Bail out on unknown faudestype
        if(!TypeRegistry::G()->Exists(ftype)) break;
        // Get corresponding ctype
        std::string ctype=TypeRegistry::G()->Definition(ftype).CType();
        // Bail out on unknown ctype
        if(ctype=="") break;
        // bail out on non-out ret value
        if(fcret && !(fattr==Parameter::Out)) break;
        // Bail out on undef attribute
        if(fattr==Parameter::UnDef) break;
        // Bail out on more than one ret values
        if(retcount>1) break;
        // Remove name space faudes
        size_t pos=ctype.find("faudes::");
        if(pos!=std::string::npos) 
          ctype=ctype.substr(std::string("faudes::").length());
        // Param ok
        cparamsi.push_back(ctype);
        cattribi.push_back(fattr);
        cretvali.push_back(fcret);
      } 
      // Test for signature error
      if((int) cparamsi.size()!=sigi.Size()) {
        std::cout << "rti2code: function registration: " << fname << ": cannot interpret signature " 
		  << sigi.Name() << std::endl;
        break;
      }
      // Record
      cparams.push_back(cparamsi);
      cattrib.push_back(cattribi);	
      cretval.push_back(cretvali);	
    }
    // Report
    std::cout << "rti2code: generating rti wrapper for \"" << fdef->Name() << "\"" << 
      " #" << cparams.size() << " variants" << std::endl;
    std::string rtiname = std::string("Rti") + ToStringInteger(fcnt) + ctype;

    // Produce C++ code: register faudes function
    if(loader) {
      rticode << "  FunctionRegistry::G()->Insert<" << rtiname << ">(\"" << fname <<"\");" << std::endl;
    }
    // Produce C++ code: class declaration intro
    if(loader) {
      rtiheader << "/* Function class for C++ function " << ctype << "*/" << std::endl;
      rtiheader << "class " << rtiname << " : public Function { " << std::endl;
      rtiheader << "public:" << std::endl;
      rtiheader << "  using Function::operator=;" << std::endl;
      rtiheader << rtiname << "(const FunctionDefinition* fdef) : Function(fdef) {};" << std::endl;
      rtiheader << "virtual Function* New(void) const { return new " << rtiname << "(pFuncDef); };" << std::endl;
      rtiheader << "protected:" << std::endl;
    }
    // Produce C++ code: function class: have typed param
    if(loader) {
      for(unsigned int i=0; i<cparams.size(); i++) 
        for(unsigned int j=0; j<cparams.at(i).size(); j++) 
          rtiheader << cparams.at(i).at(j) << "* " << "mP_" << i << "_" << j << ";" << std::endl;
    }
    // Produce C++ code: function class: do type check
    if(loader) {
      rtiheader << "virtual bool DoTypeCheck(int n) {" << std::endl;
      rtiheader << "  bool res=false;" << std::endl;
      rtiheader << "  switch(mVariantIndex) { "<< std::endl;
      for(unsigned int i=0; i<cparams.size(); i++) {
        rtiheader << "  case " << i << ": { // variant " << fdef->Variant(i).Name() << std::endl;
        rtiheader << "    switch(n) { "<< std::endl;
        for(unsigned int j=0; j<cparams.at(i).size(); j++) {
          rtiheader << "    case " << j << ": ";
          rtiheader << "    res=DoTypeCast<" << cparams.at(i).at(j) << ">(" << j << ", mP_" << i <<"_" << j << "); ";
          rtiheader << "break; "<< std::endl;
        }
        rtiheader << "    default: break; " << std::endl;
        rtiheader << "    } "<< std::endl;
        rtiheader << "    break; "<< std::endl;
        rtiheader << "  } "<< std::endl;
      }
      rtiheader << "  default: break; " << std::endl;
      rtiheader << "  } "<< std::endl;
      rtiheader << "  return res;" << std::endl;
      rtiheader << "};" << std::endl;
    }
    // Produce C++ code: function class: do execute
    if(loader) {
      rtiheader << "virtual void DoExecute(void) {" << std::endl;
    }
    // Produce C++ code: do execute: switch variant
    if(loader) {
      rtiheader << "  switch(mVariantIndex) { "<< std::endl;
      for(unsigned int i=0; i<cparams.size(); i++) {
        rtiheader << "  case " << i << ": { // variant " << fdef->Variant(i).Name() << std::endl;
        rtiheader << "    ";
        // Figure return value (if any)
        for(unsigned int j=0; j<cparams.at(i).size(); j++) {
          if(!cretval.at(i).at(j)) continue;
          // Special case: integer
          if(cparams.at(i).at(j) == "Integer") {
            rtiheader << "*(mP_" << i << "_" << j << "->CReference()) = ";
          } else 
          // Special case: boolean
	    if(cparams.at(i).at(j) == "Boolean") {
            rtiheader << "*(mP_" << i << "_" << j << "->CReference()) = ";
          } else
          // Special case: integer
          if(cparams.at(i).at(j) == "String") {
            rtiheader << "*(mP_" << i << "_" << j << "->CReference()) = ";
          } else
          // Std case
	    rtiheader << "*mP_" << i << "_" << j  << " = ";
	}
	// Function name
        rtiheader << ctype <<"(";
        // Parameters
        int parpos=0;
        for(unsigned int j=0; j<cparams.at(i).size(); j++) {
          if(cretval.at(i).at(j)) continue;
          if((parpos++)!=0) rtiheader << " ,";
          // Special case: integer
          if(cparams.at(i).at(j) == "Integer") {
            rtiheader << "*(mP_" << i << "_" << j << "->CReference())";
          } else 
          // Special case: boolean
            if(cparams.at(i).at(j) == "Boolean") {
            rtiheader << "*(mP_" << i << "_" << j << "->CReference())";
          } else
          // Special case: integer
          if(cparams.at(i).at(j) == "String") {
            rtiheader << "*(mP_" << i << "_" << j << "->CReference())";
          } else
          // Std case
            rtiheader << "*mP_" << i << "_" << j;
	}
        rtiheader << "); break; };" << std::endl;
      }
      // Produce C++ code: switch variant; done
      rtiheader << "  default: break; " << std::endl;
      rtiheader << "  }; "<< std::endl;
      // Produce C++ code: do execute: done
      rtiheader << "}; "<< std::endl;
      // Produce c code: function class: done
      rtiheader << "};"  << std::endl;
    }
    
    // Produce SWIG interface: function declarations
    if(swig) {
      swigheader << "/* faudes-function \"" << fname << "\" */" << std::endl;
    }
    // Figure my plugin to insert a conditional
    if(swig) {
      if(!flat) {
        std::string plugin=fdef->PlugIn();
        swigheader << "#if " << "( SwigModule == \"Swig" << plugin << "\")";
        //swigheader << " || ( SwigModule == \"SwigLibFaudes\")";  // requires SWIG 4.1
        swigheader << std::endl;
      }
    }
    // Use C-type function name
    if(swig) {
      if(ctype!=fname)
        swigheader << "%rename(" << fname << ") " << ctype << ";" << std::endl;
    }

    // Prepare interfeces per signature
    std::vector< std::string > lfsigs;
    std::vector< std::string > lfdecs;
    std::vector< std::string > lfdefs;
    std::vector< std::string > lrtypes;
    std::vector< std::string > lhelp;

    // Process per signature (nominal wrappers)
    for(unsigned int i=0; i<cparams.size(); i++) {
      // Create function declaration: return value
      std::string lrtype="void";
      // Special case: C++ function has a C++ return type
      for(unsigned int j=0; j<cparams.at(i).size(); j++) {
        if(!cretval.at(i).at(j)) continue;
        // Special case: integer
        if(cparams.at(i).at(j) == "Integer") {
          lrtype="long int";
        } else 
        // Special case: boolean
        if(cparams.at(i).at(j) == "Boolean") {
          lrtype="bool";
        } else
        // Special case: string
        if(cparams.at(i).at(j) == "String") {
          lrtype="std::string";
        } else
        // Std case ctype as refernce
        lrtype = cparams.at(i).at(j);
        // No more than one return value
        break;
      }
      lrtypes.push_back(lrtype);
      // Create ctype function declaration
      std::string lfdec = ctype + "(";
      std::string lfsig = ctype + "-";
      int parpos=0;
      for(unsigned int j=0; j<cparams.at(i).size(); j++) {
        // Skip C-return
        if(cretval.at(i).at(j)) continue;
        if(parpos!=0) {
	  lfdec += ", ";
	  lfsig += "-";
	}
	// Effective signature
	lfsig += cparams.at(i).at(j);
        // Have const for +In+
  	if(cattrib.at(i).at(j)==Parameter::In)
          lfdec +=  "const ";
        // Special case: integer
        if(cparams.at(i).at(j) == "Integer") {
          lfdec += "long int&";
        } else 
        // Special case: boolean
        if(cparams.at(i).at(j) == "Boolean") {
          lfdec += "bool&";
        } else
        // Special case: string
        if(cparams.at(i).at(j) == "String") {
          lfdec += "std::string&";
        } else
        // Std case ctype as refernce
          lfdec += cparams.at(i).at(j) + "&";
	parpos++;
        // Mark elementary outputs
        if(cparams.at(i).at(j) == "Boolean" || cparams.at(i).at(j) == "String" 
           || cparams.at(i).at(j) == "Integer")
        if(cattrib.at(i).at(j)==Parameter::Out)
          lfdec += " OUTPUT";
      }
      // End of function declaration
      lfdec += ")"; 
      lfdecs.push_back(lfdec);
      lfsigs.push_back(lfsig);
      // Add help entry: build nice signature
      std::string luasig = " " + fname + "(";
      bool leftcomma = false;
      bool rightcomma = false;
      for(unsigned int j=0; j<cparams.at(i).size(); j++) {
        // Special case: elementary output
        if(cparams.at(i).at(j) == "Boolean" || cparams.at(i).at(j) == "String" 
           || cparams.at(i).at(j) == "Integer")
        if(cattrib.at(i).at(j)==Parameter::Out) {
          if(leftcomma) luasig = "," + luasig;
          // if(leftcomma) luasig = ", " + luasig; // need tab in help system?
          luasig=cparams.at(i).at(j) + luasig;
          leftcomma=true;
	  continue;
	}
        // Std case
        if(rightcomma) luasig += ", ";
        const Signature& sigi=fdef->Variant(i);
        luasig += sigi.At(j).Str();
        rightcomma=true;
      }  
      luasig+=")";
      // Add help entry: add with topic
      if(fdef->TextDoc()!=""){
        std::string topic= fdef->PlugIn();
        std::string key1=fdef->KeywordAt(1);
        if(topic=="CoreFaudes") {
          topic=fdef->KeywordAt(1);
          key1=fdef->KeywordAt(2);
        }
        if(topic.length()>0) topic.at(0)=toupper(topic.at(0));
        if(key1.length()>0) key1.at(0)=toupper(key1.at(0));
        lhelp.push_back("SwigHelpEntry(\"" + topic + "\", \"" + key1 + "\", \"" +
          luasig + "\")");
      } else {
        lhelp.push_back("");
      }
      // Std no function body
      lfdefs.push_back("");
    }
    
    // Test whether we generate convenience wrappers: dont so if we have in-situ parameters
    bool xwrp=true;
    for(unsigned int i=0; i<cparams.size(); i++) {
      for(unsigned int j=0; j<cparams.at(i).size(); j++) {
        if(cattrib.at(i).at(j)==Parameter::InOut)
	  xwrp=false;
      }
    }

    // Test whether we generate convenience wrappers: for signatures, with exactly one out parameter
    for(unsigned int i=0; i<cparams.size() && xwrp; i++) {
      int fret=-1;
      for(unsigned int j=0; j<cparams.at(i).size(); j++) {
        // No extra wrapper if we have a C return value
        if(cretval.at(i).at(j)) {
          fret=-1;
          break;
        }
	// Insist in exactly one faudes output parameter
	if(cattrib.at(i).at(j)==Parameter::Out) {
	  if(fret>=0) {
  	    fret=-1;
	    break;
	  };
	  fret=j;
	}
	// No extra wrapper if we have an elementary type as the only output
        if(cparams.at(i).at(j) == "Boolean" || cparams.at(i).at(j) == "String" 
           || cparams.at(i).at(j) == "Integer") {
	  if(cattrib.at(i).at(j)==Parameter::Out) {
  	    fret=-1;
	    break;
	  }
	}
      }
      // Do generate extra wrapper with faudes return value
      if(fret>=0) {             
        // record return type
	std::string lrtype = cparams.at(i).at(fret);
        lrtypes.push_back(lrtype);
        // Create ctype function declaration: function body
        std::string lfdec = ctype + "(";
        std::string lfsig = ctype + "-";
        // Create ctype function declaration: parameters
        int parpos=0;
        for(unsigned int j=0; j<cparams.at(i).size(); j++) {
	  // Skip C-return
          if(j==fret) continue;
          if(parpos!=0) {
  	    lfdec += ", ";
	    lfsig += "-";
	  }
	  // Effective signature
	  lfsig += cparams.at(i).at(j);
          // Have const for +In+
          if(cattrib.at(i).at(j)==Parameter::In)
            lfdec +=  "const ";
          // Special case: integer
          if(cparams.at(i).at(j) == "Integer") {
            lfdec += "long int&";
          } else 
          // Special case: boolean
          if(cparams.at(i).at(j) == "Boolean") {
            lfdec += "bool&";
          } else
          // Special case: string
          if(cparams.at(i).at(j) == "String") {
            lfdec += "std::string&";
          } else {
          // Std case ctype as refernce
            lfdec += cparams.at(i).at(j) + "&";
          }
          // Name the parameter
  	  lfdec += " p" + std::to_string(parpos++);
        }
        lfdec += ")";
	// Record function definition
        lfdecs.push_back(lfdec);
        lfsigs.push_back(lfsig);
	// Create ctype body
	std::string lfdef;
        lfdef += "{ ";
	lfdef += lrtype;
	lfdef += " res; ";
	lfdef += ctype;
	lfdef += "(";
	parpos=0;
        for(unsigned int j=0; j<cparams.at(i).size(); j++) {
          if(j!=0)
	    lfdef += ", ";
          if(j==fret) {
	    lfdef += "res";
	    continue;
	  }
  	  lfdef += "p" + std::to_string(parpos++);
        }
	lfdef += "); return res; }";
	// Record function definition
        lfdefs.push_back(lfdef);
	// no help entry
        lhelp.push_back("");
      }
      
    }// Done: itarate signatures
    
    // Filter pseudo doublets (matching signature)
    for(unsigned int i=1; i<lfsigs.size();i++) {
      unsigned int j;
      for(j=0; j<i; j++) 
        if(lfsigs.at(i)==lfsigs.at(j)) break;
      if(j==i) continue; 
      std::cout << "rti2code: ambiguous signatures: ";
      std::cout << lrtypes.at(i) << "+" << lfsigs.at(i) << " vs. ";
      std::cout << lrtypes.at(j) << "+" << lfsigs.at(j) << std::endl;
      // Invalidate entry (prefer non-void return type)
      if((lrtypes.at(j)=="void") && (lrtypes.at(i)!="void")) 
        {lfdecs[j]=""; continue;}  
      if((lrtypes.at(i)=="void") && (lrtypes.at(j)!="void")) 
        {lfdecs[i]=""; continue;}  
      // Invalidate entry (by order: first wins)
      lfdecs[i]="";
    }
   
    // Generate SWIG declarations: write
    if(swig) {
      int lcount=0;
      for(unsigned int i=0; i<lfdecs.size(); i++) {
        if(lfdecs.at(i)=="") continue; 
        swigheader << lrtypes.at(i) << " " << lfdecs.at(i) << ";" << std::endl;
        if(lhelp.at(i)!="")
          swigheader << lhelp.at(i) << ";" << std::endl;
        lcount++;
      }
      if(!flat) {
        swigheader << "#endif " << std::endl;
      }  
      swigheader << std::endl;
      std::cout << "rti2code: generating swig interface for function \"" << fdef->Name() << "\"" << 
      " #" << lcount << " variants" << std::endl;
    }

    // Generate C code: convenience wrappers
    if(wrapper) {
      int lcount=0;
      for(unsigned int i=0; i<lfdecs.size(); i++) {
        if(lfdecs.at(i)=="") continue;
        if(lfdefs.at(i)=="") continue;
        wrpheader << "extern FAUDES_API " <<  lrtypes.at(i) << " " << lfdecs.at(i) << ";" << std::endl;
	wrpcode << lrtypes.at(i) << " " << lfdecs.at(i) << " " << lfdefs.at(i) << ";" << std::endl;
        lcount++;
      }
      std::cout << "rti2code: generating convenience wrappers for function \"" << fdef->Name() << "\"" << 
      " #" << lcount << " variants" << std::endl;
    }

  } // loop functions
  
  // C++ class definition: function class: all such done
  if(loader) {
    rtiheader << "} // namespace" << std::endl;
  }

  // C++ class definition: register function prototypes: done
  if(loader) {
    rticode << "}" << std::endl;
    rticode << "} // namespace" << std::endl;
  }

  // C++ wrappers: done
  if(wrapper) {
    wrpheader << "} // namespace" << std::endl;
    wrpcode << "} // namespace" << std::endl;
  }

  return(0);
}
