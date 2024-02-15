/** fts2ftx.cpp  Utility to convert a faudes token stream to XML format. 

This utility converts the std faudes token format, as generated e.g. by
Write(), to the more rigourus XML format written by XWrite(). Regarding
generators and alphabets, the former is considered the preferred format 
for human editable files, while the latter provides an interface to 
XML editors/processors and is the preferred format for configuration files.

In particular, fts2ftx can be used to convert pre 2.16b configuration
files to the XML format scheduled for version 3.00.

Copyright (C) 2010  Thomas Moor

*/

#include "corefaudes.h"

using namespace faudes;


// print usage info and exit
void usage_exit(const std::string& message="") {
  if(message!="") {
    std::cout << "fts2ftx: " << message << std::endl;
    std::cout << "" << std::endl;
    exit(-1);
  }
  std::cout << "fts2ftx: version" << VersionString()  << std::endl;
  std::cout << "" << std::endl;
  std::cout << "usage: fts2ftx [-t <ftype>] [-l <label>] [-o <outfile>] <infile>" << std::endl;
  std::cout << "where " << std::endl;
  std::cout << "  <infile>: faudes token stream to convert" << std::endl;
  std::cout << "" << std::endl;
  std::cout << "  -t <ftype>:  faudes type of infile" << std::endl;
  std::cout << "  -l <label>: section label to read (defaults to entire file)" << std::endl;
  std::cout << "  -o <outfile>:  file to write (defaults to infile with .ftx suffix)" << std::endl;
  std::cout << "" << std::endl;
  exit(-1);
}



// development: mount emscripten/js file system
/*
#include <emscripten.h>
void emjs_mount(void) {
  // mount the current folder as a NODEFS instance inside of emscripten
  EM_ASM(
    FS.mkdir('/working_dir');
    FS.mount(NODEFS, { root: '.' }, '/working_dir');
  );
  // tell C++ programm about the current folder 
  chdir("/working_dir");
}
*/


// process file
int main(int argc, char *argv[]) {

  // invoke emscripten/js mount
  //emjs_mount();

  // parameters
  std::string mInFile;
  std::string mOutFile;
  std::string mType;
  std::string mLabel;
  int mVerify = 0;

  // primitive commad line parsing
  for(int i=1; i<argc; i++) {
    std::string option(argv[i]);
    // option: -t
    if((option=="-t") || (option=="--ftype")) {
      i++; if(i>=argc) usage_exit();
      mType=argv[i];
      continue;
    }
    // option: -l
    if((option=="-l") || (option=="--lable")) {
      i++; if(i>=argc) usage_exit();
      mLabel=argv[i];
      continue;
    }
    // option: -o
    if((option=="-o") || (option=="--outfile")) {
      i++; if(i>=argc) usage_exit();
      mOutFile=argv[i];
      continue;
    }
    // option: -v
    if((option=="-v") || (option=="--verify")) {
      mVerify++;
      continue;
    }
    // option: -vv
    if(option=="-vv"){
      mVerify+=2;
      continue;
    }
    // option: -vvv
    if(option=="-vvv"){
      mVerify+=3;
      continue;
    }
    // option: help
    if((option=="-?") || (option=="--help")) {
      usage_exit();
      continue;
    }
    // option: unknown
    if(option.c_str()[0]=='-') {
      usage_exit("unknown option "+ option);
      continue;
    }
    // input 
    if(mInFile!="") 
      usage_exit("more than one filname specified");
    mInFile=option;
  }

  // very verbose: dump registry
  if(mVerify>2) {
    TypeRegistry::G()->Write();
  }

  // fix output file name
  std::string basename = ExtractFilename(mInFile);
  if(basename.find_last_of(".") !=std::string::npos) {
    basename.resize(basename.find_last_of("."));
  }  
  if(mOutFile=="") {
    mOutFile= basename+".ftx";
  }  

  // read input file
  Type* fobject=NewFaudesObject(mType);
  fobject->Read(mInFile,mLabel);

  // verify: report
  if(mVerify>2) {
    std::cout << "fts2ftx: reporting faudes object of type " << mType << " (id " << typeid(*fobject).name() << ")" << std::endl;
    fobject->Write();
  }

  // write output: console
  if(mOutFile=="-") {
    fobject->XWrite();
  }
  
  // write output: file
  if(mOutFile!="-") {
    fobject->XWrite(mOutFile);
  }

  // verify: read back
  if(mOutFile!="-") 
  if(mVerify>0) {
    std::cout << "fts2ftx: reading back ftx output" << std::endl;
    Type* readback = fobject->New();
    readback->Read(mOutFile);
    if(!(*fobject==*readback))
      std::cout << "fts2ftx: warning: objects dont match (!)" << std::endl;
  }

  // done
  return 0;
}
