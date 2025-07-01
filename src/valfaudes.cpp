/** valfaudes.cpp  Utility validate test cases */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2025  Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

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
	 
#include "corefaudes.h"

using namespace faudes;

// mini help
void usage(const std::string& msg="") {
  if (msg != "") {
    std::cerr << "valfaudes: error: " << msg << std::endl;
    std::cerr << std::endl;
    exit(1);
  }
  std::cerr << "valfaudes --- run and validate testcases (" << faudes::VersionString() << ")" << std::endl;
  std::cerr << std::endl;
  std::cerr << "usage:" << std::endl;
  std::cerr << "  valfaudes [-v|q] [-t <tmp>] <main-in>" << std::endl;
  std::cerr << std::endl;
  std::cerr << "with:" << std::endl;
  std::cerr << "  <main-in>  specify '.prot' or a '.flx' input"  << std::endl;
  std::cerr << std::endl;
  std::cerr << "options:" << std::endl;
  std::cerr << "  -t <tmp>  temp dir for extracting/validating flx packages"  << std::endl;
  std::cerr << std::endl;  
  std::cerr << "note: this tool is meant to facilitate the libFAUDES build process and" << std::endl;
  std::cerr << "relies on std libFAUDES folder layout" << std::endl;
  std::cerr << std::endl;
  exit(0);
}

// config
bool mOptV=false;
bool mOptQ=false;
std::string mLibFaudes=".";
std::string mLuaFaudes;
std::string mArgFile;
std::string mProtFile;
std::string mTestCase;
std::string mTestType;
std::string mTestPath;
std::string mFlxFile;
std::string mBinFile;
std::string mLuaFile;
std::string mPyFile;
std::string mTmpProtFile;
std::string mTmpDir;;


// helper: exe suffix
#ifdef FAUDES_POSIX
std::string exesfx(void) { return "";}
#endif  
#ifdef FAUDES_WINDOWS
std::string exesfx(void) { return ".exe";}
#endif  
#ifdef FAUDES_GENERIC
std::string exesfx(void) { return "";}
#endif  


// helper: run faudes executable (0<>success)
int runfexec(const std::string& command, const std::string& arguments="") {
  std::string cmd=command;
#ifdef FAUDES_POSIX
  if(cmd.size()>0)
    if((cmd.at(0)!='/') && (cmd.at(0)!='.'))
     cmd= "." + faudes_pathsep() + cmd;
  if(!arguments.empty())
    cmd += " " + arguments;
  if(!mOptV)
    cmd = cmd + " > /dev/null 2>&1 ";
#endif 
#ifdef FAUDES_WINDOWS
  cmd=faudes_extpath(cmd);
  if(!arguments.empty())
    cmd += " " + arguments;
  if(!mOptV)
    cmd = cmd + " > NUL 2>&1";
#endif 
  if(!mOptQ)
    std::cout << "valfaudes: running: \"" << cmd << "\"" << std::endl;
  return std::system(cmd.c_str());
}

// helper: run system executable (o<>success)
int runsexec(const std::string& command, const std::string& arguments="") {
  std::string cmd=command;
#ifdef FAUDES_POSIX
  if(!arguments.empty())
    cmd += " " + arguments;
  if(!mOptV)
    cmd = cmd + " > /dev/null 2>&1 ";
#endif 
#ifdef FAUDES_WINDOWS
  cmd=faudes_extpath(cmd);
  if(!arguments.empty())
    cmd += " " + arguments;
  if(!mOptV)
    cmd = cmd + " > NUL 2>&1";
#endif 
  if(!mOptQ)
    std::cout << "valfaudes: running: \"" << cmd << "\"" << std::endl;
  return std::system(cmd.c_str());
}

// helper: diff (0<>match)
int rundiff(const std::string& file1, const std::string& file2) {
  std::string cmd;
#ifdef FAUDES_POSIX
  if(!mOptV)
    cmd = cmd + " > /dev/null 2>&1 ";
  cmd= "diff -w --context=3 --show-function-line=mark " + file1 + " " + file2;
#endif 
#ifdef FAUDES_WINDOWS
  cmd= "fc /W " + faudes_extpath(file1) + " " + faudes_extpath(file2);
  if(!mOptV)
    cmd = cmd + " > NUL 2>&1";
#endif 
  if(!mOptQ)
    std::cout << "valfaudes: running: \"" << cmd << "\"" << std::endl;
  return std::system(cmd.c_str());
}


// helper: find luafaudes
bool findlua(void) {
  // is it right here?
  mLuaFaudes="luafaudes"+exesfx();
  if(FileExists(mLuaFaudes)) return true;
  // is it in bin?
  mLuaFaudes=PrependPath("bin",mLuaFaudes);
  if(FileExists(mLuaFaudes)) return true;
  // is it in bin one up?
  mLuaFaudes=PrependPath("..",mLuaFaudes);
  if(FileExists(mLuaFaudes)) return true;
  // is it in bin two up?
  mLuaFaudes=PrependPath("..",mLuaFaudes);
  if(FileExists(mLuaFaudes)) return true;
  // is it in bin three up?
  mLuaFaudes=PrependPath("..",mLuaFaudes);
  if(FileExists(mLuaFaudes)) return true;
  // were lost
  if(!mOptQ) {
    std::cout << "valfaudes: could not find luafaudes ( last try \"" << mLuaFaudes <<"\""
      << "from \"" << faudes_getwd() <<"\""<<std::endl;
  }
  return false;
}

// runner
int main(int argc, char *argv[]) {

  // primitive command line parser 
  for(int i=1; i<argc; i++) {
    std::string option(argv[i]);
    // option: help
    if((option=="-?") || (option=="--help")) {
      usage();
      continue;
    }
    // option: verb
    if((option=="-v") || (option=="--verbose")) {
      mOptV=true;
      mOptQ=false;
      continue;
    }
    // option: verb
    if((option=="-q") || (option=="--quiet")) {
      mOptV=false;
      mOptQ=true;
      continue;
    }
    // option: tmp dir
    if((option=="-t") || (option=="--temp")) {
      if(++i>argc)
        usage("cannot read temp dir (-t)");	
      mTmpDir=argv[i];
      continue;
    }
    // option: unknown
    if(option.c_str()[0]=='-') {
      usage("unknown option "+ option);
      continue;
    }
    // argument #1 input file
    if(mArgFile.empty()) {
      mArgFile=argv[i];
      continue;
    }
    // fail
    usage("no more than one argument must be specified" );
  }

  // fail on no input
  if(mArgFile.empty())
    usage("no input file specified");
  
  //report
  if(!mOptQ)
    std::cout << "varfaudes: input file: \"" << mArgFile <<"\"" << std::endl;

  // derive config: test type
  mTestPath=ExtractDirectory(mArgFile);
  std::string sfx=ExtractSuffix(mArgFile);
  // case a) its a .prot
  if(sfx=="prot") {
    mProtFile=mArgFile;
    mTestCase=ExtractBasename(mArgFile);
    size_t seppos=mTestCase.find_last_of('_');
    if(seppos==std::string::npos) {
      usage("could not figure test type (no seperator '_' in '.prot' file)");
    }
    mTestType=mTestCase.substr(seppos+1);
    if(ToLowerCase(mTestType)=="cpp") {
      mBinFile=mTestCase.substr(0,seppos);
    }
    if(ToLowerCase(mTestType)=="lua") {
      mLuaFile=mTestCase;
      mLuaFile.at(seppos)='.';
    }
    if(ToLowerCase(mTestType)=="py") {
      mPyFile=mTestCase;
      mPyFile.at(seppos)='.';
    }
    mTmpProtFile= "tmp_" + mTestCase + ".prot";
    mTestCase = mTestCase.substr(0,seppos);
  }
  // case b) its an .flx
  if(sfx=="flx") {
    mTestType="flx";
    mFlxFile=ExtractFilename(mArgFile);
    mTestCase=ExtractBasename(mArgFile);
    mTestPath=mTmpDir;
  }
  // failed test type
  if(mTestType.empty()) {
    usage("could not figure test type");
  }

  // derive config: working dir (for .prot files)
  if(sfx=="prot") {
    size_t datapos=mTestPath.rfind("data"); 
    if(datapos==std::string::npos) {
      usage("could not figure working dir (path must end with 'data' [a])");
    }
    if(datapos!=mTestPath.size()-5) {
      usage("could not figure working dir (path must end with 'data' [b])");
    }
    if(datapos==0) {
      usage("could not figure working dir (layout mismatch)");
    }
    mTestPath=mTestPath.substr(0,datapos-1);
    mTmpProtFile=PrependPath(mTestPath,mTmpProtFile);
  }
  
  //report
  if(!mOptQ) {
    std::cout << "varfaudes:" << std::endl;
    std::cout << "  test case: \"" << mTestCase <<"\"" << std::endl;
    std::cout << "  test working dir: \"" << mTestPath <<"\"" << std::endl;
    if(!mBinFile.empty())
      std::cout << "  exeutable: \"" << mBinFile <<"\"" << std::endl;
    if(!mLuaFile.empty())
      std::cout << "  lua script: \"" << mLuaFile <<"\"" << std::endl;
    if(!mPyFile.empty())
      std::cout << "  python script: \"" << mPyFile <<"\"" << std::endl;
    if(!mFlxFile.empty())
      std::cout << "  flx file: \"" << mFlxFile <<"\"" << std::endl;
  }

  // result code (0 for ok)
  int testok=-1;

  // is there nothing to test?
  if(mBinFile.empty() && mLuaFile.empty() && mFlxFile.empty() && mPyFile.empty()) {
    std::cout << "varfaudes: error: nothing we can validate" << std::endl;
    return 1;
  }

  // change working dir
  std::string pwd=faudes_getwd();
  if(pwd.empty()) {
    usage("could not figure current working dir");
  }
  int cdok=faudes_chdir(mTestPath);
  if(cdok!=0) {
    usage("could change to test working dir");
  }

  // cpp tutorials
  if(!mBinFile.empty()) {
    testok=runfexec(mBinFile);
  }

  // lua tutorials
  if(!mLuaFile.empty()) {
    if(!findlua()) {
#ifdef FAUDES_PLUGIN_LUABUINDINGS      
      usage("could not find luafaudes");
#endif
      std::cout << "valfaudes: silently skipping test case" << std::endl;
      testok=0;            
    } else {
      std::string arg=mLuaFile;
      if(mOptV)
	arg= "-d " + arg;
      testok=runfexec(mLuaFaudes,arg);
    }
  }

    // python tutorials
  if(!mPyFile.empty()) {
    testok=runsexec("python",mPyFile);
  }


  // flx extensions
  if(!mFlxFile.empty()) {
    // alt: have absolut paths 
    std::string args;
    args+= "-tbin ../bin";
    args+= " -t";
    args+= " ../"+mArgFile+" .";
    testok=runfexec("../bin/flxinstall",args);
  }
  		 
  // go back to original dir
  int pwdok=faudes_chdir(pwd);
  if(pwdok!=0) {
    usage("could change to back working dir");
  }

  // fail if no tests raun
  if(testok!=0) {
    usage("test failed to run");
  }

  // flx is done here
  if(!mFlxFile.empty()) {
    return testok;
  }

  // fail if no protocol found
  if(!FileExists(mTmpProtFile)) {
    usage("no test results (expected at \""+mTmpProtFile+"\")");
  }

  // do diff
  testok = rundiff(mProtFile,mTmpProtFile);
  if(!mOptQ) {
    if(testok==0) 
      std::cout << "valfaudes: no differences detected: test passed" << std::endl;
    else 
      std::cout << "valfaudes: diff returncode \"" << testok << "\": test failed" << std::endl;
  }
  
  return (testok==0 ? 0 : 1);
}
