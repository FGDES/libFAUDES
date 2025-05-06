/** @file cfl_utils.cpp   C-level utility functions */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
   Copyright (C) 2008-2010 Thomas Moor
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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA 

*/


#include "cfl_utils.h"


// Debug includes
#include "cfl_attributes.h"
#include "cfl_registry.h"
#include "cfl_types.h"
#include "cfl_elementary.h"


// c++ file io
#include <fstream>


namespace faudes {

// ToStringInteger(number)
std::string ToStringInteger(Int number) { 
  if(number>= std::numeric_limits<Int>::max()) return "inf";
  if(number<= std::numeric_limits<Int>::min()+1) return "-inf";
  std::string res;
  std::stringstream sstr;
  sstr << number;
  sstr >> res;
  return res;
}

// ToStringInteger16(number)
std::string ToStringInteger16(Int number) { 
   std::string res;
   std::stringstream sstr;
   sstr << "0x" << std::setbase(16) << number;
   sstr >> res;
   return res;
}

// ToStringFloat(number)
// todo: check range, prevent sci notation
std::string ToStringFloat(Float number) { 
   if(number>= std::numeric_limits<Float>::max()) return "inf";
   if(number<= -1*std::numeric_limits<Float>::max()) return "-inf";
   std::stringstream sstr;
   if(number == static_cast<Float>( static_cast<Int>(number) )) {
     sstr << static_cast<Int>(number);
   } else {
     sstr << std::fixed;
     sstr << number;
   }
   std::string res;
   sstr >> res;
   return res;
}

// ExpandString(rString, len) 
std::string ExpandString(const std::string& rString, unsigned int len) {
  std::string res;
  res	= rString;
  std::string::size_type xtra = (std::string::size_type) len - rString.length();
  if ((xtra > 0) && (xtra < 10000)) {
	  res.append(xtra, ' ');
	}
  return res;
} 

// CollapseString(rString, len) 
std::string  CollapsString(const std::string& rString, unsigned int len) {
  if(len <=1) return rString;
  if(rString.length() <= len) return rString;
  int chead = len/2;
  int ctail = len-chead;
  return rString.substr(0,chead) + "..." + rString.substr(rString.length()-ctail,ctail);
} 

// ToIdx(rString)
Idx ToIdx(const std::string& rString) {
  char * end;
  unsigned long ul = strtoul (rString.c_str(), &end, 0);
  unsigned long idxmax = std::numeric_limits<Idx>::max();
  if(ul > idxmax) {
    throw Exception("atoidx", "Idx overflow", 600);
  }
  return (Idx) ul;
}

// String Substitution
std::string StringSubstitute(const std::string& rString,const std::string& rFrom,const std::string& rTo) {
  // prep result
  std::string res;
  std::size_t pos=0; 
  // loop over occurences of "from"
  while(pos<rString.length()) {
    std::size_t next=rString.find(rFrom,pos);
    if(next==std::string::npos) break;
    res.append(rString.substr(pos, next-pos));
    res.append(rTo);
    pos=next+rFrom.length();
  }
  // get end
  if(pos<rString.length()) 
    res.append(rString.substr(pos));
  // done
  return res;
}

// VersionString()
std::string VersionString() {
	return std::string(FAUDES_VERSION);
}

// FluginsString()
std::string PluginsString() {
  return std::string(FAUDES_PLUGINS);
}

// ContributorsString()
std::string ContributorsString() {
  return 
    "Ramon Barakat, Ruediger Berndt, Christian Breindl, Christine Baier, Tobias Barthel, Christoph Doerr, Marc Duevel, Norman Franchi, Stefan Goetz, Rainer Hartmann, Jochen Hellenschmidt, Stefan Jacobi, Matthias Leinfelder, Tomas Masopust, Michael Meyer, Andreas Mohr, Thomas Moor, Mihai Musunoi, Bernd Opitz, Katja Pelaic, Irmgard Petzoldt, Sebastian Perk, Thomas Rempel, Daniel Ritter, Berno Schlein, Ece Schmidt, Klaus Schmidt, Anne-Kathrin Schmuck, Sven Schneider, Matthias Singer, Yiheng Tang, Ulas Turan, Christian Wamser, Zhengying Wang, Thomas Wittmann, Shi Xiaoxun, Yang Yi, Jorgos Zaddach, Hao Zhou, Christian Zwick, et al";
}

// ContributorsString()
std::string BuildString() {
  std::string res;
#ifdef FAUDES_BUILDENV
  res = res + std::string(FAUDES_BUILDENV);
#else  
  res = res + std::string("generic");
#endif
#ifdef FAUDES_BUILDTIME  
  res = res + std::string(" ") + std::string(FAUDES_BUILDTIME);
#else
  res = res + std::string(" ") + std::string(FAUDES_CONFIG_TIMESTAMP);
#endif  
  return res;
}


// ProcessDot(infile,outfile,format)
void ProcessDot(const std::string& rDotFile, 
  const std::string& rOutFile, const std::string& rOutFormat, const std::string& rDotExec)
{
  std::string format=rOutFormat;
  // guess format from filename
  if(format=="") {
    if(rOutFile.rfind('.')+1 < rOutFile.size()) {
       format=rOutFile.substr(rOutFile.rfind('.')+1);
    }
  }  
  // test format
  if (format == "canon"); 
  else if (format == "dot"); 
  else if (format == "xdot"); 
  else if (format == "cmap");
  else if (format == "dia");
  else if (format == "fig"); 
  else if (format == "gd"); 
  else if (format == "gd2"); 
  else if (format == "gif"); 
  else if (format == "hpgl"); 
  else if (format == "imap"); 
  else if (format == "cmapx");
  else if (format == "ismap"); 
  else if (format == "jpg"); 
  else if (format == "jpeg"); 
  else if (format == "mif"); 
  else if (format == "mp"); 
  else if (format == "pcl"); 
  else if (format == "pic"); 
  else if (format == "plain"); 
  else if (format == "plain-ext"); 
  else if (format == "png"); 
  else if (format == "ps"); 
  else if (format == "ps2"); 
  else if (format == "svg"); 
  else if (format == "svgz"); 
  else if (format == "vrml"); 
  else if (format == "vtx"); 
  else if (format == "wbmp"); 
  else if (format == "eps"); 
  else if (format == "pdf"); 
  else {
    std::stringstream errstr;
    errstr << "Dot output format \"" << format << "\" unknown";
    throw Exception("faudes::ProcessDot", errstr.str(), 3);
  }
  std::string dotcommand = rDotExec + " -T"+format+" \""+rDotFile+"\" -o \""+rOutFile+"\"";
  if(system(dotcommand.c_str()) != 0) {
    throw Exception("faudes::ProcessDot", 
        "Error in running " + dotcommand, 3);
  }
}


// CreateTempFile(void)
// todo: sys dependant, report, investigate threads
std::string CreateTempFile(void) {
  char filename[]= "faudes_temp_XXXXXX";
  std::string res;
#ifdef FAUDES_POSIX
  // use mkstemp on recent Posix systems
  int filedes = -1;
  filedes= mkstemp(filename);
  if(filedes==-1) {
    FD_DF("faudes::CreateTempFile(): error");
    return res;
  }
  close(filedes);
  res=std::string(filename);
#endif
#ifdef FAUDES_WINDOWS
  // mimique mkstemp on Windows/MinGW
  /*
  int filedes = -1;
  #define _S_IREAD 256
  #define _S_IWRITE 128
  mktemp(filename); 
  filedes=open(filename,O_RDWR|O_BINARY|O_CREAT|O_EXCL|_O_SHORT_LIVED, _S_IREAD|_S_IWRITE); 
  if(filedes==-1) {
    FD_DF("faudes::CreateTempFile(): error");
    return "";
  }
  close(filedes);
  res=std::string(filename);
  */
  // win32 API
  char* tmpname = _mktemp(filename); 
  FILE* file;
  if(tmpname==NULL) {
    FD_DF("faudes::CreateTempFile(): error");
    return res;
  }
  fopen_s(&file,tmpname,"w");
  if(file==NULL) {
    FD_DF("faudes::CreateTempFile(): error");
    return "";
  }
  fclose(file);
  res=std::string(tmpname);
#endif
  FD_DF("faudes::CreateTempFile(): " << res);
  return(res);
}



// ExtractPath
std::string ExtractDirectory(const std::string& rFullPath) {
  std::string res="";
  std::size_t seppos = rFullPath.find_last_of(faudes_pathseps());
  if(seppos==std::string::npos) return res;
  res=rFullPath.substr(0,seppos+1);
  return res;
}

// ExtractFilename
std::string ExtractFilename(const std::string& rFullPath) {
  std::string res=rFullPath;
  std::size_t seppos = rFullPath.find_last_of(faudes_pathseps());
  if(seppos==std::string::npos) return res;
  res=rFullPath.substr(seppos+1);
  return res;
}

// ExtractBasename
std::string ExtractBasename(const std::string& rFullPath) {
  std::string res=rFullPath;
  std::size_t seppos = res.find_last_of(faudes_pathseps());
  if(seppos!=std::string::npos) {
    res=res.substr(seppos+1);
  }
  std::size_t dotpos = res.find_last_of(".");
  if(dotpos!=std::string::npos) {
    res=res.substr(0,dotpos);
  }
  return res;
}

// ExtractSuffix
std::string ExtractSuffix(const std::string& rFullPath) {
  std::string res=rFullPath;
  std::size_t seppos = res.find_last_of(faudes_pathseps());
  if(seppos!=std::string::npos) {
    res=res.substr(seppos+1);
  }
  std::size_t dotpos = res.find_last_of(".");
  if(dotpos!=std::string::npos) 
  if(dotpos +1 < res.size()) {
    return res.substr(dotpos+1,res.size()-dotpos-1);
  }
  return std::string();
}

// PrependPath
std::string PrependPath(const std::string& rLeft, const std::string& rRight) {
  // bail out in trivial args
  if(rLeft=="")
    return std::string(rRight);
  if(rRight=="")
    return std::string(rLeft);
  // system default
  char sepchar=faudes_pathsep().at(0);
  // user overwrite by left argument
  std::size_t seppos;
  seppos=rLeft.find_last_of(faudes_pathseps());
  if(seppos!=std::string::npos) 
    sepchar=rLeft.at(seppos);
  // go doit  
  std::string res=rLeft;
  if(res.at(res.length()-1)!=sepchar)
    res.append(1,sepchar);
  if(rRight.at(0)!=sepchar){
    res.append(rRight);
    return res;
  }
  if(rRight.length()<=1) {
    return res;
  }
  res.append(rRight,1,std::string::npos);  
  return res;
}

// Test directory
bool DirectoryExists(const std::string& rDirectory) {
#ifdef FAUDES_POSIX
  DIR *thedir;
  thedir=opendir(rDirectory.c_str()); 
  if(thedir) closedir(thedir);
  return thedir!= 0;
#endif
#ifdef FAUDES_WINDOWS
  DWORD fattr = GetFileAttributesA(rDirectory.c_str());
  return 
    (fattr!=INVALID_FILE_ATTRIBUTES) && (fattr & FILE_ATTRIBUTE_DIRECTORY); 
#endif
  return false;
}

// scan directory
std::set< std::string > ReadDirectory(const std::string& rDirectory) {
  std::set< std::string >  res;
#ifdef FAUDES_POSIX
  DIR *thedir;
  struct dirent *theent;
  thedir=opendir(rDirectory.c_str()); 
  if(!thedir) return res;
  while((theent=readdir(thedir))) {
    std::string fname(theent->d_name);     
    if(fname==".") continue;
    if(fname=="..") continue;
    res.insert(fname);
  }
  closedir(thedir);
#endif
#ifdef FAUDES_WINDOWS
  HANDLE hf;
  WIN32_FIND_DATA data;
  hf = FindFirstFile((rDirectory+"\\*.*").c_str(), &data);
  if (hf != INVALID_HANDLE_VALUE) {
    do {
      std::string fname(data.cFileName);     
      if(fname==".") continue;
      if(fname=="..") continue;
      res.insert(fname);
    } while (FindNextFile(hf, &data));
    FindClose(hf);
  }
#endif
  return res;
}





// Test file
bool FileExists(const std::string& rFilename) {
  std::fstream fp;
  fp.open(rFilename.c_str(), std::ios::in | std::ios::binary);
  return fp.good();
}

// Delete file
bool FileDelete(const std::string& rFilename) {
  return remove(rFilename.c_str()) == 0;
}

// Copy file
bool FileCopy(const std::string& rFromFile, const std::string& rToFile) {
  std::ifstream froms(rFromFile.c_str(), std::ios::binary);
  std::ofstream tos(rToFile.c_str(), std::ios::binary);
  tos << froms.rdbuf();
  tos.flush();
  return !(froms.fail() || tos.fail());
}

// ConsoleOut class
// Note: console-out is not re-entrant; for multithreaded applications
// you must derive a class that has built-in mutexes; 
ConsoleOut::ConsoleOut(void) : pStream(NULL), mMute(false) , mVerb(0) {
  pInstance=this;
}
ConsoleOut::~ConsoleOut(void) {
  if(pStream) { pStream->flush(); delete pStream; }
  if(this==smpInstance) smpInstance=NULL;
}
ConsoleOut* ConsoleOut::G(void) {
  if(!smpInstance) smpInstance= new ConsoleOut();
  return smpInstance->pInstance;
}
void ConsoleOut::Redirect(ConsoleOut* out) {
  std::string fname = smpInstance->pInstance->Filename();
  smpInstance->pInstance->ToFile("");
  smpInstance->pInstance=out; 
  if(!smpInstance->pInstance) smpInstance->pInstance=smpInstance;
  smpInstance->pInstance->ToFile(fname);
}
void ConsoleOut::ToFile(const std::string& filename) {
  if(pStream) { pStream->flush(); delete pStream; }
  pStream=NULL;
  mFilename=filename;
  if(mFilename=="") return;
  pStream = new std::ofstream();
  pStream->open(mFilename.c_str(),std::ios::app);
}
  void ConsoleOut::Write(const std::string& message,long int cntnow, long int cntdone, int verb) {
  if(mMute) return;
  if(mVerb<verb) return;
  DoWrite(message,cntnow,cntdone,verb);
}
  void ConsoleOut::DoWrite(const std::string& message,long int cntnow, long int cntdone, int verb) {
    (void) cntnow; (void) cntdone; (void) verb;
  std::ostream* sout=pStream;
  if(!sout) sout=&std::cout; // tmoor: used to be std::cerr, using std::cout to facilitate emscripten/js
  *sout << message;
  sout->flush();
}

// global instance
ConsoleOut* ConsoleOut::smpInstance=NULL;
 


// debugging: objectcount
std::map<std::string,long int>* ObjectCount::mspMax=NULL;
std::map<std::string,long int>* ObjectCount::mspCount=NULL;
bool ObjectCount::msDone=false;
ObjectCount::ObjectCount(void) {
  mspCount= new std::map<std::string,long int>();
  mspMax= new std::map<std::string,long int>();
  msDone=true; 
}
void  ObjectCount::Init(void) {
 if(!msDone) ObjectCount();
}
void  ObjectCount::Inc(const std::string& rTypeName) {
  if(!msDone) ObjectCount();
  long int cnt = ((*mspCount)[rTypeName]+=1);
  if((*mspMax)[rTypeName]<cnt) (*mspMax)[rTypeName]=cnt;
}
void  ObjectCount::Dec(const std::string& rTypeName) {
  if(!msDone) ObjectCount();
  (*mspCount)[rTypeName]-=1;
}


// debugging: report on exit function
void ExitFunction(void){
#ifdef FAUDES_DEBUG_CODE
  FAUDES_WRITE_CONSOLE("faudes::ExitFunction():");
  // be sure its up and running
  ObjectCount::Init();
  // get rid of all registry prototypes
  //TypeRegistry::G()->ClearAll();  
  //FunctionRegistry::G()->Clear();  
  // prepare report
  std::map<std::string,long int>::iterator cit;
  cit=ObjectCount::mspCount->begin();
  for(;cit!=ObjectCount::mspCount->end();cit++) {
    FAUDES_WRITE_CONSOLE( cit->first << ": #" << ToStringInteger(cit->second) <<
     " (max #" << (*ObjectCount::mspMax)[cit->first] << ")");
  }
#endif
}
 

#ifdef FAUDES_DEBUG_CODE
// report on exit install
class ExitFunctionInstall {
private:
  static bool mDone;
  static ExitFunctionInstall mInstance;
  ExitFunctionInstall(void) {
    if(mDone) return;
    FAUDES_WRITE_CONSOLE("ExitFunctionInstall()");
    std::atexit(ExitFunction);
    mDone=true;
  } 
};
// exit function: global data
bool ExitFunctionInstall::mDone=false;
ExitFunctionInstall ExitFunctionInstall::mInstance;
#endif

// test protocol: token writer and file
TokenWriter* gTestProtocolTw=NULL;
std::string  gTestProtocolFr;

// test protocol: setup
std::string TestProtocol(const std::string& rSource) {
  if(gTestProtocolTw) return gTestProtocolFr;
  // set up filename
  std::string filename=rSource;
  // fix empty source
  if(filename=="") filename="faudes_dump";
  // remove directory
  filename=ExtractFilename(filename);
  // remove extension
  std::string::size_type pos=0;
  for(;pos<filename.length();pos++) 
    if(filename.at(pos)=='.') filename.at(pos)='_';
  // append extension
  filename.append(".prot");
  // record nominal case
  gTestProtocolFr=filename;
  // prepend prefix
  filename.insert(0,"tmp_");
  // initialise token writer
  gTestProtocolTw= new TokenWriter(filename);
  // report filename
  return gTestProtocolFr;
}
  
// test protocol: dump
void TestProtocol(const std::string& rMessage, const Type& rData, bool full) {
  if(!gTestProtocolTw) return;
  gTestProtocolTw->WriteComment("%%% test mark: " + rMessage);
  if(full) rData.Write(*gTestProtocolTw);
  else rData.SWrite(*gTestProtocolTw);
  gTestProtocolTw->WriteComment("");
  gTestProtocolTw->WriteComment("");
  gTestProtocolTw->WriteComment("");
  *gTestProtocolTw << "\n";
}
void TestProtocol(const std::string& rMessage, bool data) {
  Boolean fbool(data);
  TestProtocol(rMessage,fbool,true);
} 
void TestProtocol(const std::string& rMessage, long int data) {
  Integer fint(data);
  TestProtocol(rMessage,fint,true);
} 
void TestProtocol(const std::string& rMessage, const std::string& rData) {
 String fstr(rData);
 TestProtocol(rMessage,fstr,true);
} 

// test protocol: compare
bool TestProtocol(void) {
  // bail out on no protocol
  if(!gTestProtocolTw) return true;
  // close protocol file
  std::string prot=gTestProtocolTw->FileName();
  delete gTestProtocolTw;
  gTestProtocolTw=NULL;
  // open protocol 
  std::fstream fp;
  fp.open(prot.c_str(), std::ios::in | std::ios::binary);
  if(!fp.good()) { 
    FAUDES_WRITE_CONSOLE("TestProtocol(): could not open protocol \"" << prot << "\"");
    return false;
  }
  // open reference
  std::string ref=gTestProtocolFr;
  std::fstream fr;
  fr.open(ref.c_str(), std::ios::in | std::ios::binary); 
  if(!fr.good()) { 
    ref="data"+faudes_pathsep()+ref;
    fr.clear(); // mingw's runtime will not clear on open
    fr.open(ref.c_str(), std::ios::in | std::ios::binary);
  }
  if(!fr.good()) { 
    FAUDES_WRITE_CONSOLE("TestProtocol(): could not open/find reference \"" << gTestProtocolFr << "\"");
    return true;
  }
  // perform comparision
  int dline=-1;
  int cline=1;    
  try{
    while(true) {
      // read next char
      char cp = fp.get();
      char cr= fr.get();
      // eof
      if(fp.eof() && fr.eof()) { break; }
      if(!fp.good() || !fr.good()) { dline=cline; break;}
      // cheap normalize cr/lf: ignore \r (assume no double \r
      if( cp=='\r' && cr =='\r') continue;
      if( cp=='\r' && fp.eof()){ dline=cline; break;}
      if( cp=='\r') cp = fp.get();      
      if( cr=='\r' && fr.eof()){ dline=cline; break;}
      if( cr=='\r') cr = fr.get();   
      // count lines
      if( cr=='\n') cline++;
      // sense mitmatch
      if( cp!= cr ){dline=cline; break;}
    }
  } catch(std::ios::failure&) {
    throw Exception("TestProtocol()", "io error at line " + ToStringInteger(cline), 1);
  }
  // done
  if(dline>=0) {
    FAUDES_WRITE_CONSOLE("TestProtocol(): using reference " << ref << "");
    FAUDES_WRITE_CONSOLE("TestProtocol(): found difference at line " << dline << "");
  }
  return dline== -1;
}



// declare loop callback
static bool (*gBreakFnct)(void)=0;

// set loop callback
void LoopCallback( bool pBreak(void)) {
  gBreakFnct=pBreak;
}

// do loop callback
// note: this function is meant to be "quiet" during normal
// operation in order not to mess up console logging
void LoopCallback(void){
  if(!gBreakFnct) return;
  if(! (*gBreakFnct)() ) return;
  throw Exception("LoopCallback", "break on application request", 110);
}

} // namespace faudes
