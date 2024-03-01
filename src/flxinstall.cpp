/** flxinstall.spp Utility to create/install/remove faudes-lua-extensions */

/* FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2011 Thomas Moor

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
#include <set>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include "corefaudes.h"


using namespace faudes;

// ******************************************************************
// error exit
// ******************************************************************

void usage_exit(const std::string& rMessage="") {
  // ui hints
  if(rMessage!="") {
    std::cerr << "" << std::endl;
    std::cerr << "flxinstall: " << rMessage << ": ERROR." << std::endl;
    std::cerr << "" << std::endl;
    exit(1);
  }
  std::cerr << "flxinstall: " << VersionString()  << std::endl;
  std::cerr << "" << std::endl;
  std::cerr << "utility to create/install/remove faudes-lua-extension "<< std::endl;
  std::cerr << std::endl << "usage: " << std::endl;
  std::cerr << "flxinstall  <advanced options> <mode>  <input-file(s)> <output-file/path>" << std::endl;
  std::cerr << "with <mode> as follows:" << std::endl;
  std::cerr << " -c  create extension (*.flx file) from input files" << std::endl;
  std::cerr << " -i  install extension (*.flx file) to libFAUDES installation" << std::endl;
  std::cerr << " -r  remove extensions from libFAUDES installation" << std::endl;
  std::cerr << " -x  extract extension (*.flx file) to output directory" << std::endl;
  std::cerr << " -t  extract and test extension (*.flx file) to output directory" << std::endl;
  std::cerr << std::endl;
  std::cerr << "advanced options for non-standard target layout" << std::endl;
  std::cerr << " -tbin <path> location of binaries, default $target/bin" << std::endl;
  exit(1);
}


// ******************************************************************
// Configuration
// ******************************************************************


std::string mXmlSeparator = "<!-- ================================================================================ -->";


// source/contents/detination
std::set < std::string > mSourceFiles;
std::string mSourceFile;
std::string mTarget;
std::string mExtensionName;
std::set < std::string > mReferencePages;
std::set < std::string > mGeneratorFiles;
std::set < std::string > mImageFiles;
std::set < std::string > mLuaFunctions;


// destination layout (effective, defaults to libfaudes std)
std::string mFaudesBase;
std::string mFaudesBin;
std::string mFaudesBinLuafaudes;
std::string mFaudesBinLuaflx;
std::string mFaudesBinRef2html;
bool mFaudesStandalone = false;
std::string mFaudesBinLua2ref;
std::string mFaudesDoc;
std::string mFaudesDocCss;
std::string mFaudesDocToc;
std::string mFaudesDocNav;
std::string mFaudesDocRti;
std::string mFaudesDocReference;
std::string mFaudesDocLuafaudes;
std::string mFaudesDocImages;
std::string mFaudesDocRefsrc;
std::string mFaudesDocTemp;



// ******************************************************************
// helper: make/remove dir
// ******************************************************************

// mkdir 
void MakeDirectory(const std::string& rPath, const std::string& rDir="") {
  std::string dir = rPath;
  if(rDir!="") dir += faudes_pathsep() + rDir;
  if(DirectoryExists(dir)) return;
  std::cerr << "flxinstall: creating dir \"" << dir << "\"" << std::endl;
  int sysret=std::system(std::string("mkdir " + dir).c_str());
  if(sysret!=0) {
    std::cerr << "flxinstall: error while creating directory \"" << dir << "\"" << std::endl;
    usage_exit();
  }
}


// remove unix hidden files from list
std::set < std::string >  EraseHiddenFiles(const std::set < std::string > & src) {
  std::set < std::string > res;
  for(std::set < std::string >::iterator fit=src.begin(); fit!=src.end(); fit++) {
    if(*fit=="") continue;
    if((*fit).at(0)=='.') continue;
    res.insert(*fit);
  }
  return res;
}



// ******************************************************************
// helper: lua to ref process
// ******************************************************************

void Lua2ref(const std::string& rLuaFile, const std::string& rRefFile="") {
  // bail out 
  if(mFaudesBinLua2ref=="") {
    std::cerr << "flxinstall: ignoring lua script \"" << rLuaFile << "\"" << std::endl;
    return;
  }
  // set up target
  std::string dst=rRefFile;
  if(dst=="") {
    dst=PrependDirectory(ExtractDirectory(rLuaFile),ExtractBasename(rLuaFile)+ ".fref");
  }
  // set up command
  std::string cmd= mFaudesBinLua2ref + " " + rLuaFile + " > " + dst;
  // run 
  std::cerr << "flxinstall: converting lua script \"" << rLuaFile << "\"" << std::endl;
  std::cerr << "flxinstall: using command \"" << cmd << "\"" << std::endl;
  int sysret=std::system(cmd.c_str());
  if(sysret!=0) {
    std::cerr << "flxinstall: error while converting lua script \"" << rLuaFile << "\"" << std::endl;
    usage_exit();
  }
    
}

// ******************************************************************
// helper: gen to ref process
// ******************************************************************

void Gen2ref(const std::string& rGenFile, const std::string& rRefFile="") {
  std::cerr << "flxinstall: converting generator file \"" << rGenFile << "\"" << std::endl;
  std::string bas = ExtractBasename(rGenFile);
  // set up source and target
  std::string dst=rRefFile;
  if(dst=="") {
    dst=PrependDirectory(ExtractDirectory(rGenFile),bas+".fref");
  }
  TokenWriter tw(dst);
  TokenReader tr(rGenFile);
  // convert
  try {
    // reference page
    Token btag;
    btag.SetBegin("ReferencePage");
    btag.InsAttributeString("chapter","images");
    btag.InsAttributeString("section","none");
    btag.InsAttributeString("page","none");
    btag.InsAttributeString("title","Example Data");
    tw << btag;
    // headline
    tw.WriteCharacterData("<h1> Example Date: "+bas+".gen </h1>\n");
    // svg image
    tw.WriteCharacterData("<h3> Dot-processed graph as SVG-image </h3>\n");
    tw.WriteCharacterData("<object type=\"image/svg+xml\" name=\"graph\" data=\""+bas+".svg\">\n");
    tw.WriteCharacterData("<a class=\"faudes_image\" href=\""+bas+".svg\">\n");
    tw.WriteCharacterData("<img src=\"$genfile.png\" title=\"Click on image to download SVG formated file.\" />\n");
    tw.WriteCharacterData("</a></object>");
    // headline
    tw.WriteCharacterData("<h3>Token IO</h3>\n");
    // copy generator
    tw.WriteBegin("pre");
    std::string genstr;
    tr.ReadSection(genstr);
    tw.WriteText(genstr); // this will escape critical entities
    tw.WriteEnd("pre");
    // close my elements
    tw.WriteCharacterData("<p>&nbsp;</p><p>&nbsp;</p>\n");
    tw.WriteEnd("ReferencePage");
  } catch( faudes::Exception&){
    std::cerr << "flxinstall: error while converting generator \"" << rGenFile << "\"" << std::endl;
    usage_exit();
  }
}


// ******************************************************************
// helper: copy files
// ******************************************************************

// extract and copy token section
void InsertSection(TokenReader& rTr, TokenWriter& rTw, const std::string& mLabel) {
  // get begin
  Token btag;
  rTr.ReadBegin(mLabel,btag);
  // copy section
  rTw.Write(btag);
  std::string scttext;
  rTr.ReadSection(scttext);
  rTw.WriteCharacterData(scttext);
  rTw.WriteEnd(mLabel);    
  // read end tag
  rTr.ReadEnd(mLabel);
}

// extract and copy page data
void InsertReferencePage(TokenReader& rTr, TokenWriter& rTw, const std::string mSection="") {
  // get begin
  Token btag;
  rTr.ReadBegin("ReferencePage",btag);
  // fix attributes
  if(!btag.ExistsAttributeString("chapter")) 
    btag.InsAttributeString("chapter","Reference");
  if(!btag.ExistsAttributeString("section")) 
  if(mSection!="")
    btag.InsAttributeString("section",mSection);
  // copy section
  rTw.Write(btag);
  std::string scttext;
  rTr.ReadSection(scttext);
  rTw.WriteCharacterData(scttext);
  rTw.WriteEnd("ReferencePage");    
  // read end tag
  rTr.ReadEnd("ReferencePage");
}


// extract and copy luafunction
void InsertLuaFunction(TokenReader& rTr, TokenWriter& rTw) {
  // get begin
  Token btag;
  rTr.ReadBegin("LuaFunctionDefinition",btag);
  // copy section
  rTw.Write(btag);
  std::string scttext;
  rTr.ReadSection(scttext);
  rTw.WriteCharacterData(scttext);
  rTw.WriteEnd("LuaFunctionDefinition");    
  rTw.Endl();
  // read end tag
  rTr.ReadEnd("LuaFunctionDefinition");
}



// extract and copy tutorial (from xml to plain)
void InsertPlainLuaTutorial(TokenReader& rTr, TokenWriter& rTw) {
  std::string codestr;
  Token btag;
  rTr.ReadVerbatim("LuaTutorial", codestr);
  *rTw.Streamp() << codestr;
}


// extract and copy tutorial (from plain to xml)
void InsertLuaTutorial(TokenReader& rTr, TokenWriter& rTw) {
  // read script file to buffer
  char* buffer=0;
  long int size=0;
  try{
    rTr.Rewind();
    rTr.Streamp()->seekg(0, std::ios::end);
    std::streampos last = rTr.Streamp()->tellg();
    rTr.Streamp()->seekg(0, std::ios::beg);
    std::streampos first = rTr.Streamp()->tellg();
    size=(long int) last-first;
    buffer = new char[last-first]; 
    rTr.Streamp()->read(buffer, last-first);
    rTr.Rewind();
  } catch (std::ios::failure&) {
    std::cerr << "flxinstall: io error when reading  \"" << rTr.FileName() << "\": ERROR." << std::endl;
    exit(1);
  }
  // convert (better solution?)
  std::string bufferstr;
  bufferstr.assign(buffer,size);
  // relative destination
  std::string name=rTr.FileName();
  name=ExtractFilename(name);
  // write cdata encoded entry
  Token btag;
  btag.SetBegin("LuaTutorial");
  btag.InsAttributeString("name",name);
  rTw.WriteVerbatim(btag,bufferstr);
  // discard buffer
  delete buffer;
}



// copy source as binary image file
void InsertImageFile(TokenReader& rTr,TokenWriter& rTw) {
  // read image file to buffer
  char* buffer=0;
  long int size=0;
  try{
    rTr.Rewind();
    rTr.Streamp()->seekg(0, std::ios::end);
    std::streampos last = rTr.Streamp()->tellg();
    rTr.Streamp()->seekg(0, std::ios::beg);
    std::streampos first = rTr.Streamp()->tellg();
    size=(long int) last-first;
    buffer = new char[last-first]; 
    rTr.Streamp()->read(buffer, last-first);
    rTr.Rewind();
  } catch (std::ios::failure&) {
    std::cerr << "flxinstall: io error when reading  \"" << rTr.FileName() << "\": ERROR." << std::endl;
    exit(1);
  }
  // relative destination
  std::string name=rTr.FileName();
  name=ExtractFilename(name);
  // write Base64 encoded entry
  Token btag;
  btag.SetBegin("ImageFile");
  btag.InsAttributeString("name",name);
  rTw.Write(btag);
  rTw.WriteBinary(buffer,size);
  rTw << "\n";
  rTw.WriteEnd("ImageFile");
  // discard buffer
  delete buffer;
}



// copy source as binary data file
void InsertDataFile(TokenReader& rTr,TokenWriter& rTw) {
  // read data file to buffer
  char* buffer=0;
  long int size=0;
  try{
    rTr.Rewind();
    rTr.Streamp()->seekg(0, std::ios::end);
    std::streampos last = rTr.Streamp()->tellg();
    rTr.Streamp()->seekg(0, std::ios::beg);
    std::streampos first = rTr.Streamp()->tellg();
    size=(long int) last-first;
    buffer = new char[last-first]; 
    rTr.Streamp()->read(buffer, last-first);
    rTr.Rewind();
  } catch (std::ios::failure&) {
    std::cerr << "flxinstall: io error when reading  \"" << rTr.FileName() << "\": ERROR." << std::endl;
    exit(1);
  }
  // relative destination
  std::string name=rTr.FileName();
  name=ExtractFilename(name);
  // write Base64 encoded entry
  Token btag;
  btag.SetBegin("DataFile");
  btag.InsAttributeString("name",name);
  rTw.Write(btag);
  rTw.WriteBinary(buffer,size);
  rTw << "\n";
  rTw.WriteEnd("DataFile");
  // discard buffer
  delete buffer;
}




// ******************************************************************
// create
// ******************************************************************


// control
void CreateExtensionFile(void) {

  // ************ pass 1: inspect sources

  // traverse images
  for(std::set < std::string >::iterator fit=mSourceFiles.begin(); fit!=mSourceFiles.end(); fit++) {
    // test extension
    std::string ext=ExtractExtension(*fit);
    std::string bas=ExtractBasename(*fit);
    std::string pbas=ExtractDirectory(*fit)+bas; 
    // case: full generator image
    if( mSourceFiles.find(pbas + ".gen") != mSourceFiles.end())
    if( mSourceFiles.find(pbas + ".svg") != mSourceFiles.end())
    if( mSourceFiles.find(pbas + ".png") != mSourceFiles.end()) { 
      if( mGeneratorFiles.find(pbas) == mGeneratorFiles.end()) {
        std::cerr << "flxinstall: scanning full generator image \"" << pbas << ".*\"" << std::endl;
        mGeneratorFiles.insert(pbas);
      }
      continue;
    } 
    // skip non images
    if(! ((ext=="png") || (ext=="svg") || (ext=="jpeg") || (ext=="jpg") )) continue;
    // case: std image/binary
    std::cerr << "flxinstall: scanning image file \"" << *fit << "\"" << std::endl;
    mImageFiles.insert(*fit);
  }
  // traverse luafunctions
  for(std::set < std::string >::iterator fit=mSourceFiles.begin(); fit!=mSourceFiles.end(); fit++) {
    // test extension
    std::string ext=ExtractExtension(*fit);
    std::string bas=ExtractBasename(*fit);
    // cases: luafunction
    if(ext=="rti" && mImageFiles.find(bas)==mImageFiles.end()) {
      std::cerr << "flxinstall: scanning luafunction \"" << *fit << "\"" << std::endl;
      TokenReader tr(*fit);
      Token btag;
      tr.ReadBegin("LuaFunctionDefinition",btag);
      if(!btag.ExistsAttributeString("name")) {
        std::cerr << "flxinstall: name not specified " << tr.FileLine() << ": ERROR." << std::endl;
        exit(1);
      }
      // extract name and space
      std::string name;
      std::string space;
      name=btag.AttributeStringValue("name");
      size_t pos=name.find("::"); 
      if(pos!=std::string::npos) {
        space=name.substr(0,pos);
        name=name.substr(pos+2);
      }
      // insist in matching space
      if(space!="" && mExtensionName!="" && space!=mExtensionName) {
        std::cerr << "flxinstall: namespace must match extension name" << tr.FileLine() << ": ERROR." << std::endl;
        exit(1);
      }
      mExtensionName=space;
      // done
      tr.ReadEnd("LuaFunctionDefinition");
    }
  }
  // traverse ref pages
  for(std::set < std::string >::iterator fit=mSourceFiles.begin(); fit!=mSourceFiles.end(); fit++) {
    // test extension
    std::string ext=ExtractExtension(*fit);
    std::string bas=ExtractBasename(*fit);
    // cases: reference page
    if(ext=="fref") {
      std::cerr << "flxinstall: scanning reference page \"" << *fit << "\"" << std::endl;
      TokenReader tr(*fit);
      Token btag;
      tr.ReadBegin("ReferencePage",btag);
      if(!btag.ExistsAttributeString("page")) {
        std::cerr << "flxinstall: page not specified " << tr.FileLine() << ": ERROR." << std::endl;
        exit(1);
      }
      if(!btag.ExistsAttributeString("title")) {
        std::cerr << "flxinstall: title not specified " << tr.FileLine() << std::endl;
        exit(1);
      }
      if(btag.ExistsAttributeString("chapter")) 
      if(btag.AttributeStringValue("chapter")!="Reference") {
        std::cerr << "flxinstall: chapter must be \"Reference\" " << tr.FileLine() << ": ERROR." << std::endl;
        exit(1);
      }
      if(btag.ExistsAttributeString("section")) 
      if(mExtensionName.size()==0) {
        mExtensionName=btag.AttributeStringValue("section");
      }
      if(btag.ExistsAttributeString("section")) 
      if(mExtensionName!=btag.AttributeStringValue("section")) {
        std::cerr << "flxinstall: section name \"" << mExtensionName << "\" expected " 
             << tr.FileLine() << ": ERROR." << std::endl;
        exit(1);
      }
      std::string page=btag.AttributeStringValue("page");
      std::transform(page.begin(), page.end(), page.begin(), tolower);
      // swallow page number
      std::string ppage=page;
      std::size_t upos = ppage.find_first_of("_");
      std::size_t dpos = 0;
      for(; dpos < ppage.size();dpos++) 
        if(!isdigit(ppage.at(dpos))) break;
      if(upos!=std::string::npos)
        if(upos==dpos)
          if(upos+1<ppage.size()) 
            ppage=ppage.substr(upos+1,ppage.size()-upos-1);
      // record
      if(mReferencePages.find(ppage)!=mReferencePages.end()) {
        std::cerr << "flxinstall: double page label \"" << ppage << "\" "
            << tr.FileLine() << ": ERROR." << std::endl;
        exit(1);
      }
      mReferencePages.insert(ppage);
      tr.ReadEnd("ReferencePage");
    }
  }
  if(mReferencePages.find("index")==mReferencePages.end()) {
    std::cerr << "flxinstall: missing index page, will be generated on installation." << std::endl;
  }


  // ************ pass 2: read/copy sources

  // set up target
  TokenWriter* ptw=0;
  if(mTarget=="") {
    mTarget=mExtensionName + ".flx";
    std::transform(mTarget.begin(), mTarget.end(), mTarget.begin(), tolower);
  }
  if(mTarget!="-")
    ptw=new TokenWriter(mTarget,"LuaExtension");
  else    
    ptw=new TokenWriter(TokenWriter::Stdout);
  // indicate tool version
  *ptw->Streamp() << "<!-- flxinstall " << VersionString() << " -->" << std::endl;
  // start tag
  Token btag;
  btag.SetBegin("LuaExtension");
  btag.InsAttributeString("name",mExtensionName);
  ptw->Write(btag);
  // traverse files
  for(std::set < std::string >::iterator fit=mSourceFiles.begin(); fit!=mSourceFiles.end(); fit++) {
    // test extension
    std::string ext=ExtractExtension(*fit);
    std::string bas=ExtractBasename(*fit);
    std::string pbas=ExtractDirectory(*fit)+bas; 
    // cases: reference page
    if(ext=="fref") {
      std::cerr << "flxinstall: appending reference page from \"" << *fit << "\"" << std::endl;
      *ptw << "\n" << "\n";
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << "<!-- reference page from source \"" << *fit << "\" -->" << std::endl;
      TokenReader tr(*fit);
      InsertReferencePage(tr,*ptw,mExtensionName);
      continue;
    }  
    // cases: code
    if(ext=="rti") {
      std::cerr << "flxinstall: appending lua function from \"" << *fit << "\"" << std::endl;
      *ptw << "\n" << "\n";
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << "<!-- lua function from source \"" << *fit << "\" -->" << std::endl;
      TokenReader tr(*fit);
      InsertLuaFunction(tr,*ptw);
      continue;
    }  
    // cases: binary image
    if(mImageFiles.find(*fit)!=mImageFiles.end()) {
      std::cerr << "flxinstall: appending image/binary file from \"" << *fit << "\"" << std::endl;
      *ptw << "\n" << "\n";
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << "<!-- binary file from source \"" << *fit << "\" -->" << std::endl;
      TokenReader tr(*fit);
      InsertImageFile(tr,*ptw);
      continue;
    } 
    // cases: full generator image
    if(mGeneratorFiles.find(pbas)!=mGeneratorFiles.end()) {
      std::cerr << "flxinstall: appending full generator from \"" << pbas << ".*\"" << std::endl;
      *ptw << "\n" << "\n";
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << "<!-- full generator image from source \"" << pbas << ".*\" -->" << std::endl;
      TokenReader trg(pbas+".gen");
      InsertImageFile(trg,*ptw);
      *ptw << "\n";
      TokenReader trs(pbas+".svg");
      InsertImageFile(trs,*ptw);
      *ptw << "\n";
      TokenReader trp(pbas+".png");
      InsertImageFile(trp,*ptw);
      mGeneratorFiles.erase(pbas);
      continue;
    } 
    // cases: tutorial
    if(ext=="lua") {
      std::cerr << "flxinstall: appending tutorial from \"" << *fit << "\"" << std::endl;
      *ptw << "\n" << "\n";
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << "<!-- tutorial from source \"" << *fit << "\" -->" << std::endl;
      TokenReader tr(*fit);
      InsertLuaTutorial(tr,*ptw);
      continue;
    }  
    // cases: data directory
    if(ext=="" && bas=="data") {
      std::set< std::string > datafiles = ReadDirectory(*fit);
      datafiles=EraseHiddenFiles(datafiles);
      if(datafiles.size()==0) continue;
      std::cerr << "flxinstall: appending data files \"" << *fit << "\"" << std::endl;
      *ptw << "\n" << "\n";
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << mXmlSeparator << std::endl;
      *ptw->Streamp() << "<!-- data from source \"" << *fit << "\" -->" << std::endl;
      std::set< std::string >::iterator dit;
      for(dit=datafiles.begin();dit!=datafiles.end();dit++) {
	std::string srcfile=PrependDirectory(*fit,*dit);
        TokenReader tr(srcfile);
        InsertDataFile(tr,*ptw);
      }
    }  
  }
  // cleanup/close
  *ptw << "\n" << "\n";
  ptw->WriteEnd("LuaExtension"); 
  delete ptw;
}


// ******************************************************************
// test libfaudes installation
// ******************************************************************

// inspect libfaudes distribution to locate luafaudes (use mFaudesBin)
void TestLuafaudes(void) {
  // read bin dir
  if(!DirectoryExists(mFaudesBin)){
    std::cerr << "flxinstall: cannot open libfaudes binary path \"" << mFaudesBin << "\": ERROR." << std::endl;
    exit(1);
  }
  std::set< std::string > binfiles = ReadDirectory(mFaudesBin);
  // find luafaudes
  mFaudesBinLuafaudes="";
  if(binfiles.find("luafaudes.exe")!= binfiles.end()) {
    mFaudesBinLuafaudes=PrependDirectory(mFaudesBin,"luafaudes.exe");
  }
  if(binfiles.find("luafaudes")!= binfiles.end()) {
    mFaudesBinLuafaudes=PrependDirectory(mFaudesBin,"luafaudes");
  }
  if(!FileExists(mFaudesBinLuafaudes)) {
    std::cerr << "flxinstall: warning: cannot open luafaudes in \"" << mFaudesBin << "\"" << std::endl;
    mFaudesBinLuafaudes="";
  }
}

// inspect libfaudes distribution
void TestFaudesTarget(void) {
  mFaudesBase=mTarget;
  std::set< std::string > faudesfiles = ReadDirectory(mFaudesBase);
  // bad dir
  if(faudesfiles.empty()) {
    std::cerr << "flxinstall: cannot open target directory \"" << mFaudesBase << "\": ERROR." << std::endl;
    exit(1);
  }
  // bin (allow overwrite)
  if(mFaudesBin=="") {
    mFaudesBin=mFaudesBase;
    if(faudesfiles.find("bin")!= faudesfiles.end())
      mFaudesBin=PrependDirectory(mFaudesBase,"bin");
  }
  if(!DirectoryExists(mFaudesBin)){
    std::cerr << "flxinstall: cannot open libfaudes binary path in \"" << mFaudesBin << "\": ERROR." << std::endl;
    exit(1);
  }
  std::set< std::string > binfiles = ReadDirectory(mFaudesBin);
  // insist in ref2html
  mFaudesBinRef2html="";
  if(binfiles.find("ref2html.exe")!= binfiles.end()) {
    mFaudesBinRef2html=PrependDirectory(mFaudesBin,"ref2html.exe");
  }
  if(binfiles.find("ref2html")!= binfiles.end()) {
    mFaudesBinRef2html=PrependDirectory(mFaudesBin,"ref2html");
  }
  if(mFaudesBinRef2html=="") {
    std::cerr << "flxinstall: cannot open ref2html tool in \"" << mFaudesBin << "\": ERROR." << std::endl;
    exit(1);
  }
  // ref2html options
  if(mFaudesStandalone) mFaudesBinRef2html += " -app";
  // find luafaudes
  TestLuafaudes();
  // default flx
  mFaudesBinLuaflx=PrependDirectory(mFaudesBin,"luafaudes.flx");
  // doc (allow overwrite)
  if(mFaudesDoc=="") {
    mFaudesDoc=mFaudesBase;
    if(faudesfiles.find("doc")!= faudesfiles.end()) {
      mFaudesDoc=PrependDirectory(mFaudesDoc,"doc");
    } else if(faudesfiles.find("Doc")!= faudesfiles.end()) {
      mFaudesDoc=PrependDirectory(mFaudesDoc,"Doc");
    } else if(faudesfiles.find("Documentation")!= faudesfiles.end()) {
      mFaudesDoc=PrependDirectory(mFaudesDoc,"Documentation");
    } 
    if(!DirectoryExists(mFaudesDoc)){
      std::cerr << "flxinstall: cannot open libfaudes documentation path in \"" << mFaudesBase << "\": ERROR." << std::endl;
      exit(1);
    }
  }
  if(!DirectoryExists(mFaudesDoc)){
    std::cerr << "flxinstall: cannot open libfaudes documentation path at \"" << mFaudesDoc << "\": ERROR." << std::endl;
    exit(1);
  }
  std::set< std::string > docfiles = ReadDirectory(mFaudesDoc);
  // css (allow overwrite)
  if(mFaudesDocCss=="") mFaudesDocCss="faudes.css";
  mFaudesDocCss=ExtractFilename(mFaudesDocCss); // ignore directory
  // reference
  mFaudesDocReference=mFaudesDoc;
  if(docfiles.find("reference")!= docfiles.end()) {
    mFaudesDocReference=PrependDirectory(mFaudesDoc,"reference");
  } else if(docfiles.find("Reference")!= faudesfiles.end()) {
    mFaudesDocReference=PrependDirectory(mFaudesDoc,"Reference");
  }
  if(!DirectoryExists(mFaudesDocReference)){
    std::cerr << "flxinstall: cannot open libfaudes reference path in \"" << mFaudesDoc << "\": ERROR." << std::endl;
    exit(1);
  }
  std::set< std::string > regfiles = ReadDirectory(mFaudesDocReference);
  // luafaudes doc (default: non-existent)
  mFaudesDocLuafaudes="";
  if(docfiles.find("luafaudes")!= docfiles.end()) {
    mFaudesDocLuafaudes=PrependDirectory(mFaudesDoc,"luafaudes");
  } else if(docfiles.find("Luafaudes")!= docfiles.end()) {
    mFaudesDocLuafaudes=PrependDirectory(mFaudesDoc,"Luafaudes");
  } 
  // lua2ref (try luafaudes on lua2ref.lua)
  if(mFaudesBinLuafaudes!="") {
    std::string script =  mFaudesBase + faudes_pathsep() + "tools" + faudes_pathsep() 
       + "lua2ref" + faudes_pathsep() + "lua2ref.lua";
    mFaudesBinLua2ref=mFaudesBinLuafaudes + " " + script;
    if(!FileExists(script)) mFaudesBinLua2ref = "";
    if(!FileExists(mFaudesBinLuafaudes)) {
      std::cerr << "flxinstall: cannot find converter \"lua2ref.lua\"" << std::endl;
      mFaudesBinLua2ref = "";
    }
  }
  // lua2ref (try perl on lua2ref.pl)
  if(mFaudesBinLua2ref=="") {
    mFaudesBinLua2ref = mFaudesBase + faudes_pathsep() + "tools" + faudes_pathsep() 
       + "lua2ref" + faudes_pathsep() + "lua2ref.pl";
    if(!FileExists(mFaudesBinLua2ref)) {
      std::cerr << "flxinstall: cannot find converter \"lua2ref.pl\"" << std::endl;
      mFaudesBinLua2ref = "";
    }
  }
  // report
  if(mFaudesBinLua2ref=="") {
    std::cerr << "flxinstall: cannot process lua tutorial sources: ERROR." << std::endl;
  }
  if(mFaudesBinLua2ref!="") {
    std::cerr << "flxinstall: using \"" << mFaudesBinLua2ref <<"\" to convert lua tutorials," << std::endl;
  }
  // images
  mFaudesDocImages="";
  if(docfiles.find("images")!= docfiles.end()) {
    mFaudesDocImages=PrependDirectory(mFaudesDoc,"images");
  } else {
    std::cerr << "flxinstall: cannot open images in \"" << mFaudesDoc << "\": ERROR." << std::endl;
    exit(1);
  }
  // refsrc
  mFaudesDocRefsrc="";
  if(docfiles.find("refsrc")!= docfiles.end()) {
    mFaudesDocRefsrc=PrependDirectory(mFaudesDoc,"refsrc");
  } else {
    std::cerr << "flxinstall: cannot open refsrc in \"" << mFaudesDoc << "\": ERROR." << std::endl;
    exit(1);
  }
  std::set< std::string > refsrcfiles = ReadDirectory(mFaudesDocRefsrc);
  // rti (allow overwrite)
  if(mFaudesDocRti=="") mFaudesDocRti = PrependDirectory(mFaudesDocRefsrc,"libfaudes.rti");
  if(!FileExists(mFaudesDocRti)){
    std::cerr << "flxinstall: cannot open libfaudes.rti at \"" << mFaudesDocRti << "\": ERROR." << std::endl;
    exit(1);
  }
  // cnav (allow overwrite)
  if(mFaudesDocNav=="") mFaudesDocNav="faudes_navigation.include_fref";
  mFaudesDocNav=ExtractFilename(mFaudesDocNav); // ignore directory
  mFaudesDocNav=PrependDirectory(mFaudesDocRefsrc,mFaudesDocNav); // enforce directory
  if(!FileExists(mFaudesDocNav)){
    std::cerr << "flxinstall: cannot open navigation file \"" << mFaudesDocNav << "\": ERROR." << std::endl;
    exit(1);
  }
  // temp
  mFaudesDocTemp=PrependDirectory(mFaudesDocRefsrc,"tmp_flx");
  if(docfiles.find("tmp_flx")== docfiles.end()) {
    std::cerr << "flxinstall: creating temp dir \"" << mFaudesDocTemp << "\"" << std::endl;
    MakeDirectory(mFaudesDocTemp);
  } 
  mFaudesDocToc=PrependDirectory(mFaudesDocTemp,"toc.ftoc");
}

 
// ******************************************************************
// extract reference pages
// ******************************************************************


void  XtractReferencePages(TokenReader& rTr, const std::string& rDstDir) {
  // read outer tag
  Token btag;
  rTr.Rewind();
  rTr.ReadBegin("LuaExtension",btag);
  if(!btag.ExistsAttributeString("name")) {
    std::cerr << "flxinstall: lua-extension must have a name attribute " << rTr.FileLine() << ": ERROR." << std::endl;
    exit(1);
  }
  mExtensionName=btag.AttributeStringValue("name");  
  // scan for reference page sections
  while(!rTr.Eos("LuaExtension")) {
    rTr.Peek(btag);
    // skip tokens
    if(!btag.IsBegin()) {
      rTr.Get(btag);
      continue;
    }
    // skip sections
    if(btag.StringValue()!="ReferencePage") {
      rTr.ReadBegin(btag.StringValue());
      rTr.ReadEnd(btag.StringValue());
      continue;
    }
    // extract title & friends
    std::string title="libFAUDES Reference";
    if(btag.ExistsAttributeString("title")) 
      title=btag.AttributeStringValue("title");
    std::string chapter="Reference";
    if(btag.ExistsAttributeString("chapter")) 
      chapter=btag.AttributeStringValue("chapter");
    std::string section="";
    if(btag.ExistsAttributeString("section")) 
      section=btag.AttributeStringValue("section");
    std::string page="";
    if(btag.ExistsAttributeString("page")) 
      page=btag.AttributeStringValue("page");
    // insist in page and section
    if(page=="" || section=="") {
      std::cerr << "flxinstall: skipping undefined page at " << rTr.FileLine() << std::endl;
      rTr.ReadBegin("ReferencePage",btag);
      rTr.ReadEnd("ReferencePage");
      continue;
    } 
    // normalize & report 
    std::transform(section.begin(), section.end(), section.begin(), tolower);
    std::transform(page.begin(), page.end(), page.begin(), tolower);
    // swallow page number
    std::string ppage=page;
    std::size_t upos = ppage.find_first_of("_");
    std::size_t dpos = 0;
    for(; dpos < ppage.size();dpos++) 
      if(!isdigit(ppage.at(dpos))) break;
    if(upos!=std::string::npos)
      if(upos==dpos)
        if(upos+1<ppage.size()) 
          ppage=ppage.substr(upos+1,ppage.size()-upos-1);
    // basename
    std::string basename= section + "_" + ppage;
    mReferencePages.insert(basename);
    // dst file
    std::string dstfile=rDstDir + faudes_pathsep() + basename + ".fref";
    std::cerr << "flxinstall: extracting reference page to \"" << dstfile << "\"" << std::endl;
    TokenWriter dst(dstfile);
    InsertReferencePage(rTr,dst);
  }  
}

// ******************************************************************
// extract image files
// ******************************************************************


void  XtractImageFiles(TokenReader& rTr,const std::string& rDstDir) {
  // read outer tag
  Token btag;
  rTr.Rewind();
  rTr.ReadBegin("LuaExtension",btag);
  if(!btag.ExistsAttributeString("name")) {
    std::cerr << "flxinstall: lua-extension must have a name attribute " << rTr.FileLine() << ": ERROR." << std::endl;
    exit(1);
  }
  mExtensionName=btag.AttributeStringValue("name");  
  // scan for image files
  while(!rTr.Eos("LuaExtension")) {
    rTr.Peek(btag);
    // skip tokens
    if(!btag.IsBegin()) {
      rTr.Get(btag);
      continue;
    }
    // skip sections
    if(btag.StringValue()!="ImageFile") {
      rTr.ReadBegin(btag.StringValue());
      rTr.ReadEnd(btag.StringValue());
      continue;
    }
    // read begin tag
    rTr.ReadBegin("ImageFile",btag);
    std::string name=btag.AttributeStringValue("name");
    if(name==""){
      std::cerr << "flxinstall: image file must specify name " << rTr.FileLine() << std::endl;
      rTr.ReadEnd("ImageFile");
      continue;
    } 
    // skip non-image formats
    std::string ext = ExtractExtension(name);
    if(ext!="png" && ext!="svg" && ext!="jpg" && ext!="jpeg") {
      rTr.ReadEnd("ImageFile");
      continue;
    }
    // read data
    Token data;
    rTr.Get(data);
    if(!data.IsBinary()){
      std::cerr << "flxinstall: skipping invalid image data " << rTr.FileLine() << std::endl;
      rTr.ReadEnd("ImageFile");
      continue;
    } 
    // dst file
    std::transform(name.begin(), name.end(), name.begin(), tolower);
    std::string dstfile=rDstDir + faudes_pathsep() + name;
    std::cerr << "flxinstall: extracting image to \"" << dstfile << "\"" << std::endl;
    // record
    mImageFiles.insert(dstfile);
    // setup stream
    std::fstream fsout;
    fsout.exceptions(std::ios::badbit|std::ios::failbit);
    try{
      fsout.open(dstfile.c_str(), std::ios::out | std::ios::binary); 
      fsout.write(data.StringValue().c_str(),data.StringValue().size());
      fsout.close();
    } 
    catch (std::ios::failure&) {
      std::cerr << "flxinstall: file io error when writing  \"" << dstfile << "\"" << std::endl;
    }
    // read end tag
    rTr.ReadEnd("ImageFile");
  }  
}

void  XtractImageGenFiles(TokenReader& rTr,const std::string& rDstDir) {
  // read outer tag
  Token btag;
  rTr.Rewind();
  rTr.ReadBegin("LuaExtension",btag);
  if(!btag.ExistsAttributeString("name")) {
    std::cerr << "flxinstall: lua-extension must have a name attribute " << rTr.FileLine() << ": ERROR." << std::endl;
    exit(1);
  }
  mExtensionName=btag.AttributeStringValue("name");  
  // scan for image files
  while(!rTr.Eos("LuaExtension")) {
    rTr.Peek(btag);
    // skip tokens
    if(!btag.IsBegin()) {
      rTr.Get(btag);
      continue;
    }
    // skip sections
    if(btag.StringValue()!="ImageFile") {
      rTr.ReadBegin(btag.StringValue());
      rTr.ReadEnd(btag.StringValue());
      continue;
    }
    // read begin tag
    rTr.ReadBegin("ImageFile",btag);
    std::string name=btag.AttributeStringValue("name");
    if(name==""){
      std::cerr << "flxinstall: image file must specify name " << rTr.FileLine() << std::endl;
      rTr.ReadEnd("ImageFile");
      continue;
    } 
    // skip non-gen formats
    std::string ext = ExtractExtension(name);
    if(ext!="gen"){
      rTr.ReadEnd("ImageFile");
      continue;
    }
    // read data
    Token data;
    rTr.Get(data);
    if(!data.IsBinary()){
      std::cerr << "flxinstall: skipping invalid image data " << rTr.FileLine() << std::endl;
      rTr.ReadEnd("ImageFile");
      continue;
    } 
    // dst file
    std::transform(name.begin(), name.end(), name.begin(), tolower);
    std::string dstfile=rDstDir + faudes_pathsep() + name;
    std::cerr << "flxinstall: extracting image to \"" << dstfile << "\"" << std::endl;
    // record
    mImageFiles.insert(dstfile);
    // setup stream
    std::fstream fsout;
    fsout.exceptions(std::ios::badbit|std::ios::failbit);
    try{
      fsout.open(dstfile.c_str(), std::ios::out | std::ios::binary); 
      fsout.write(data.StringValue().c_str(),data.StringValue().size());
      fsout.close();
    } 
    catch (std::ios::failure&) {
      std::cerr << "flxinstall: file io error when writing  \"" << dstfile << "\"" << std::endl;
    }
    // read end tag
    rTr.ReadEnd("ImageFile");
  }  
}


// ******************************************************************
// extract lua code
// ******************************************************************



void  XtractLuaFunctions(TokenReader& rTr, TokenWriter& rTw) {
  // read outer tag
  Token btag;
  rTr.Rewind();
  rTr.ReadBegin("LuaExtension",btag);
  if(!btag.ExistsAttributeString("name")) {
    std::cerr << "flxinstall: lua-extension must have a name attribute " << rTr.FileLine() << ": ERROR." << std::endl;
    exit(1);
  }
  mExtensionName=btag.AttributeStringValue("name");  
  // scan for lua function definitions
  while(!rTr.Eos("LuaExtension")) {
    rTr.Peek(btag);
    // skip tokens
    if(!btag.IsBegin()) {
      rTr.Get(btag);
      continue;
    }
    // skip sections
    if(btag.StringValue()!="LuaFunctionDefinition") {
      rTr.ReadBegin(btag.StringValue());
      rTr.ReadEnd(btag.StringValue());
      continue;
    }
    // extract title & friends
    std::string name;
    std::string space;
    if(btag.ExistsAttributeString("name")){
      name=btag.AttributeStringValue("name");
      size_t pos=name.find("::"); 
      if(pos!=std::string::npos) {
        space=name.substr(0,pos);
        name=name.substr(pos+2);
      }
    }
    // insist in name 
    if(name=="") {
      std::cerr << "flxinstall: skipping undefined lua function at " << rTr.FileLine() << std::endl;
      rTr.ReadBegin("LuaFunctionDefinition",btag);
      rTr.ReadEnd("LuafunctionDefinition");
      continue;
    } 
    // insist in space to match 
    /*
    if(space!=mExtensionName) {
      std::cerr << "flxinstall: skipping undefined lua function at " << rTr.FileLine() << std::endl;
      rTr.ReadBegin("LuaFunctionDefinition",btag);
      rTr.ReadEnd("LuafunctionDefinition");
      continue;
    } 
    */
    // record
    mLuaFunctions.insert(name);
    // copy
    std::cerr << "flxinstall: extracting lua function \"" << name << "\"" << std::endl;
    *rTw.Streamp() << mXmlSeparator << std::endl;
    *rTw.Streamp() << mXmlSeparator << std::endl;
    *rTw.Streamp() << "<!-- lua function from lua-extension " << rTr.FileLine() << " -->" << std::endl;
    InsertLuaFunction(rTr,rTw);
  }  
}

// ******************************************************************
// extract lua tutorials
// ******************************************************************

void  XtractLuaTutorials(TokenReader& rTr, const std::string& rDstDir) {
  // read outer tag
  Token btag;
  rTr.Rewind();
  rTr.ReadBegin("LuaExtension",btag);
  if(!btag.ExistsAttributeString("name")) {
    std::cerr << "flxinstall: lua-extension must have a name attribute " << rTr.FileLine() << ": ERROR." << std::endl;
    exit(1);
  }
  mExtensionName=btag.AttributeStringValue("name");  
  // scan for reference page sections
  while(!rTr.Eos("LuaExtension")) {
    rTr.Peek(btag);
    // skip tokens
    if(!btag.IsBegin()) {
      rTr.Get(btag);
      continue;
    }
    // skip sections
    if(btag.StringValue()!="LuaTutorial") {
      rTr.ReadBegin(btag.StringValue());
      rTr.ReadEnd(btag.StringValue());
      continue;
    }
    // test for name
    std::string name=btag.AttributeStringValue("name");
    if(name==""){
      std::cerr << "flxinstall: lua tutorial must specify name " << rTr.FileLine() << std::endl;
      rTr.ReadEnd("LuaTutorial");
      continue;
    } 
    // set up destination and copy
    std::transform(name.begin(), name.end(), name.begin(), tolower);
    std::string dstfile=rDstDir + faudes_pathsep() + name;
    std::cerr << "flxinstall: extracting lua tutorial to \"" << dstfile << "\"" << std::endl;
    TokenWriter tw(dstfile);
    InsertPlainLuaTutorial(rTr,tw);
  }
}

// ******************************************************************
// generate default reference page
// ******************************************************************

void DefaultIndexPage(const std::string& rDstDir) {
  // name of index files
  std::string index=mExtensionName + "_index";
  std::transform(index.begin(), index.end(), index.begin(), tolower);
  // test existence
  if(mReferencePages.find(index)!=mReferencePages.end()) {
    std::cerr << "flxinstall: index page provided" << std::endl;
    return;
  } 
  // error case
  if(mReferencePages.size()>0) {
    std::cerr << "flxinstall: reference page missing: \"" << index << ".fref\": ERROR" << std::endl;
    exit(1);
  } 
  // generates defaultindex page
  std::cerr << "flxinstall: generate index page" << std::endl;
  TokenWriter tw(rDstDir + faudes_pathsep() + index + ".fref","ReferencePage");
  *tw.Streamp() << "<!-- flxinstall " << VersionString() << ": auto generated index -->" << std::endl;
  *tw.Streamp() << "<ReferencePage chapter=\"Reference\" section=\"" << mExtensionName << 
    "\" page=\"0_Index\" title=\""<< mExtensionName << " Index\" >" << std::endl;
  // headline
  *tw.Streamp() << "<h1> " << mExtensionName << ": Functions </h1>" << std::endl;
  // list functions
  std::set< std::string >::iterator fit;
  for(fit=mLuaFunctions.begin(); fit!= mLuaFunctions.end(); fit++) {
    *tw.Streamp() << "<ffnct_reference name=\"" << *fit << "\" />" << std::endl;
  }
  // done
  *tw.Streamp() << "</ReferencePage>" << std::endl;
} 




// ******************************************************************
// install extension
// ******************************************************************

// control
void InstallExtensionFiles(void) {

  // clear context
  mImageFiles.clear();

  // clean tmp (this is for dstinstall to see only relevant files in the temp directory)
  std::set< std::string > tmpfiles = ReadDirectory(mFaudesDocTemp);
  for(std::set < std::string >::iterator fit=tmpfiles.begin(); fit!=tmpfiles.end(); fit++) {
    std::string dfile=PrependDirectory(mFaudesDocTemp,*fit);
    if(!RemoveFile(dfile)) {
      std::cerr << "flxinstall: failed to remove \"" << *fit << "\"" << std::endl;
    };
  }

  // prepare luafaudes.flx
  TokenWriter* twflx = new TokenWriter(PrependDirectory(mFaudesDocTemp,"luafaudes.flx"));

  // convenience: reinterpret directories 
  std::set< std::string > srcfiles;
  for(std::set < std::string >::iterator fit=mSourceFiles.begin(); fit!=mSourceFiles.end(); fit++) {
    std::string sfile = *fit;
    if(ExtractExtension(sfile)=="flx") {
      srcfiles.insert(sfile);
      continue;
    }
    std::set< std::string > sdir = ReadDirectory(sfile);
    sdir=EraseHiddenFiles(sdir);
    for(std::set < std::string >::iterator dit=sdir.begin();dit!=sdir.end();dit++) {
      sfile=PrependDirectory(*fit,*dit);
      srcfiles.insert(sfile);
    }
  }

  // traverse flx-files and extract
  for(std::set < std::string >::iterator fit=srcfiles.begin(); fit!=srcfiles.end(); fit++) {
    // test extension
    std::string ext=ExtractExtension(*fit);
    std::string bas=ExtractBasename(*fit);
    // cases: flx
    if(ext=="flx") {
      std::cerr << "flxinstall: extracting lua-extension from \"" << *fit << "\"" << std::endl;
      // clear extension context
      mReferencePages.clear();
      mLuaFunctions.clear();
      mExtensionName="";
      // extract component files
      TokenReader rTr(*fit);
      XtractReferencePages(rTr,mFaudesDocTemp);
      XtractLuaTutorials(rTr,mFaudesDocTemp);
      XtractImageFiles(rTr,mFaudesDocImages);  
      XtractImageGenFiles(rTr,mFaudesDocTemp);  
      XtractLuaFunctions(rTr,*twflx);
      // autogenerate index if necessary
      DefaultIndexPage(mFaudesDocTemp);      
      continue;
    } 
  }

  // copy composed flx file to accompanie luafaudes binary
  delete twflx;
  FileCopy(PrependDirectory(mFaudesDocTemp,"luafaudes.flx"),mFaudesBinLuaflx);

  // report
  std::cerr << "flxinstall: generating list of source files" << std::endl;

  // record all files relevant to toc
  std::set< std::string > tocsource;
  // record all basenames to detect doublet fref
  std::set< std::string > frefbase;


  // collect all fref files for processing to doc/
  std::set< std::string > docsource;
  std::set< std::string > docrefsrc = ReadDirectory(mFaudesDocRefsrc);
  for(std::set < std::string >::iterator fit=docrefsrc.begin(); fit!=docrefsrc.end(); fit++) {
    std::string ext=ExtractExtension(*fit);
    std::string bas=ExtractBasename(*fit);
    std::string ffile=PrependDirectory(mFaudesDocRefsrc,*fit);
    if(ext!="fref") continue;
    if(frefbase.find(bas)!=frefbase.end()){
      std::cerr << "flxinstall: reference file doublet \"" << *fit << "\" from std dist: ERROR." << std::endl;
      exit(1);
    }
    docsource.insert(ffile);
    tocsource.insert(ffile);
    frefbase.insert(bas);
  }

  // collect all fref files for processing to doc/reference
  std::set< std::string > docrefsource;
  std::set< std::string > docrefsrcref = ReadDirectory(PrependDirectory(mFaudesDocRefsrc,"reference"));
  for(std::set < std::string >::iterator fit=docrefsrcref.begin(); fit!=docrefsrcref.end(); fit++) {
    std::string ext=ExtractExtension(*fit);
    std::string bas=ExtractBasename(*fit);
    std::string ffile=PrependDirectory(PrependDirectory(mFaudesDocRefsrc,"reference"),*fit);
    if(ext!="fref") continue;
    if(frefbase.find(bas)!=frefbase.end()){
      std::cerr << "flxinstall: reference file doublet \"" << *fit << "\" from std dist: ERROR." << std::endl;
      exit(1);
    }
    docrefsource.insert(ffile);
    tocsource.insert(ffile);
    frefbase.insert(bas);
  }
  std::set< std::string > doctmpdir = ReadDirectory(mFaudesDocTemp);
  for(std::set < std::string >::iterator fit=doctmpdir.begin(); fit!=doctmpdir.end(); fit++) {
    std::string ext=ExtractExtension(*fit);
    std::string bas=ExtractBasename(*fit);
    std::string ffile=PrependDirectory(mFaudesDocTemp,*fit);
    if(ext!="fref") continue;
    if(frefbase.find(bas)!=frefbase.end()){
      std::cerr << "flxinstall: reference file doublet \"" << *fit << "\" from ext: ERROR." << std::endl;
      exit(1);
    }
    docrefsource.insert(ffile);
    tocsource.insert(ffile);
    frefbase.insert(bas);
  }


  // collect/generate all luatutorial files for processing to doc/luafaudes
  std::set< std::string > docluasource;
  std::set< std::string > docrefsrclua = ReadDirectory(PrependDirectory(mFaudesDocRefsrc,"luafaudes"));
  for(std::set < std::string >::iterator fit=docrefsrclua.begin(); fit!=docrefsrclua.end(); fit++) {
    std::string ext=ExtractExtension(*fit);
    std::string bas=ExtractBasename(*fit);
    std::string ffile=PrependDirectory(PrependDirectory(mFaudesDocRefsrc,"luafaudes"),*fit);
    if(ext!="fref") continue;
    if(frefbase.find(bas)!=frefbase.end()){
      std::cerr << "flxinstall: reference file doublet \"" << *fit << "\" from lua doc: ERROR." << std::endl;
      exit(1);
    }
    docluasource.insert(ffile);
    tocsource.insert(ffile);  // list of tutorials is required in toc
    frefbase.insert(bas);
  }
  /*std::set< std::string > */tmpfiles = ReadDirectory(mFaudesDocTemp);
  for(std::set < std::string >::iterator fit=tmpfiles.begin(); fit!=tmpfiles.end(); fit++) {
    std::string ext=ExtractExtension(*fit);
    std::string bas=ExtractBasename(*fit);
    std::string lfile=PrependDirectory(mFaudesDocTemp,*fit);
    std::string ffile=PrependDirectory(mFaudesDocTemp,bas+".fref");
    // skip non-lua
    if(ext!="lua") continue;
    // test for corresponding fref file
    std::string fref = ExtractBasename(*fit) + ".fref";
    if(tmpfiles.find(fref)!=tmpfiles.end()) continue;
    // process
    Lua2ref(lfile,ffile);
    // record (conditional for e.g. dstinstall, where no lua tutorials are generated)
    if(FileExists(ffile)) {
      docluasource.insert(ffile);
      tocsource.insert(ffile);  // list of tutorials is required in toc
      frefbase.insert(bas);
    }
  }

  // convert/generate full generator images to fref (dest: doc/images)
  std::set< std::string > docimgsource;
  std::set< std::string > docrefsrcimg = ReadDirectory(PrependDirectory(mFaudesDocRefsrc,"images"));
  for(std::set < std::string >::iterator fit=docrefsrcimg.begin(); fit!=docrefsrcimg.end(); fit++) {
    std::string ext=ExtractExtension(*fit);
    std::string bas=ExtractBasename(*fit);
    std::string ffile=PrependDirectory(PrependDirectory(mFaudesDocRefsrc,"images"),*fit);
    if(ext!="fref") continue;
    docimgsource.insert(ffile);
  }
  std::set< std::string > imgfiles = ReadDirectory(mFaudesDocTemp);
  for(std::set < std::string >::iterator fit=imgfiles.begin(); fit!=imgfiles.end(); fit++) {
    std::string ext=ExtractExtension(*fit);
    std::string bas=ExtractBasename(*fit);
    std::string gfile=PrependDirectory(mFaudesDocTemp,*fit);
    std::string ffile=PrependDirectory(mFaudesDocTemp,bas+".fref");
    // skip non-gen
    if(ext!="gen") continue;
    // test whther we wrote that file
    if(mImageFiles.find(gfile)==mImageFiles.end()) continue;
    // process to fref
    Gen2ref(gfile,ffile);
    // record (conditional)
    if(FileExists(ffile)) 
      docimgsource.insert(ffile);
  }


  // compose toc
  std::string toccmd;
  for(std::set < std::string >::iterator fit=tocsource.begin(); fit!=tocsource.end(); fit++) {
    toccmd+= " " + *fit;
  }
  toccmd=mFaudesBinRef2html + " -rti " + mFaudesDocRti + " -flx " + mFaudesBinLuaflx + " -toc " + toccmd
    + " " + mFaudesDocToc;
  std::cerr << "flxinstall: creating toc" << std::endl;
  if(std::system(toccmd.c_str())!=0) {
    std::cerr << "flxinstall: processing" << std::endl << toccmd << std::endl;
    std::cerr << "flxinstall: error setting up toc: ERROR." << std::endl;
    exit(1);
  }
  std::cerr << "flxinstall: creating toc: done" << std::endl;


  // process all pages to doc
  std::string doccmd;
  for(std::set < std::string >::iterator fit=docsource.begin(); fit!=docsource.end(); fit++) {
    doccmd += " " + *fit;
  }
  doccmd= mFaudesBinRef2html 
       + " -rti " + mFaudesDocRti + " -flx " + mFaudesBinLuaflx + " -cnav " + mFaudesDocNav
       + " -css " + mFaudesDocCss + " -inc " + mFaudesDocToc 
       + doccmd +  " " + mFaudesDoc;
  std::cerr << "flxinstall: processing doc base" << std::endl;
  if(std::system(doccmd.c_str())!=0) {
    std::cerr << "flxinstall: processing" << std::endl << doccmd << std::endl;
    std::cerr << "flxinstall: error while processing doc base: ERROR." << std::endl;
    exit(1);
  }
  std::cerr << "flxinstall: processing doc base: done" << std::endl;

  // process all pages to doc/reference
  std::string refcmd;
  for(std::set < std::string >::iterator fit=docrefsource.begin(); fit!=docrefsource.end(); fit++) {
    refcmd += " " + *fit;
  }
  refcmd= mFaudesBinRef2html 
       + " -rti " + mFaudesDocRti + " -flx " + mFaudesBinLuaflx + " -cnav " + mFaudesDocNav
       + " -css " + mFaudesDocCss + " -inc " + mFaudesDocToc + " -rel ../  " 
       + refcmd +  " " + mFaudesDocReference;
  std::cerr << "flxinstall: processing user reference" << std::endl;
  if(std::system(refcmd.c_str())!=0) {
    std::cerr << "flxinstall: processing" << std::endl << refcmd << std::endl;
    std::cerr << "flxinstall: error while processing user reference: ERROR." << std::endl;
    exit(1);
  }
  std::cerr << "flxinstall: processing user reference: done" << std::endl;

  // process all pages to doc/luafaudes
  if(mFaudesDocLuafaudes!="" && docluasource.size()>0) {
    std::string luacmd;
    for(std::set < std::string >::iterator fit=docluasource.begin(); fit!=docluasource.end(); fit++) {
      luacmd += " " + *fit;
    }
    luacmd= mFaudesBinRef2html 
         + " -rti " + mFaudesDocRti + " -flx " + mFaudesBinLuaflx + " -cnav " + mFaudesDocNav
         + " -css " + mFaudesDocCss + " -inc " + mFaudesDocToc + " -rel ../  " 
         + luacmd +  " " + mFaudesDocLuafaudes; 
    std::cerr << "flxinstall: processing lua tutorial" << std::endl;
    if(std::system(luacmd.c_str())!=0) {
      std::cerr << "flxinstall: processing" << std::endl << luacmd << std::endl;
      std::cerr << "flxinstall: error while processing lua tutorial: ERROR." << std::endl;
      exit(1);
    }
    std::cerr << "flxinstall: processing lua tutorial: done" << std::endl;
  }

  // process all pages to doc/images/
  if(mFaudesDocImages!="" && docimgsource.size()>0) {
    std::cerr << "flxinstall: processing image files" << std::endl;
    // do at most 20 at the time (limit length of commandline)
    std::set < std::string >::iterator fit=docimgsource.begin();
    while(fit!=docimgsource.end()) {
      std::string imgcmd;
      for(; fit!=docimgsource.end(); fit++) {
        imgcmd += " " + *fit;
        if(imgcmd.length()>500) break;
      }
      imgcmd= mFaudesBinRef2html 
           + " -rti " + mFaudesDocRti + " -flx " + mFaudesBinLuaflx + " -cnav " + mFaudesDocNav
           + " -css " + mFaudesDocCss + " -inc " + mFaudesDocToc + " -rel ../  " 
           + imgcmd +  " " + mFaudesDocImages;
      if(std::system(imgcmd.c_str())!=0) {
        std::cerr << "flxinstall: processing" << std::endl << imgcmd << std::endl;
        std::cerr << "flxinstall: error while processing image files: ERROR." << std::endl;
        exit(1);
      }
    }
    std::cerr << "flxinstall: processing image files: done" << std::endl;
  }


  // copy index file: main index
  if(FileExists(PrependDirectory(mFaudesDocRefsrc,"faudes_about.fref"))) {
    std::string dst=PrependDirectory(mFaudesDoc,"index.html");
    std::string proccmd= mFaudesBinRef2html 
        + " -rti " + mFaudesDocRti + " -flx " + mFaudesBinLuaflx + " -cnav " + mFaudesDocNav
        + " -css " + mFaudesDocCss + " -inc " + mFaudesDocToc + " -rel ./  " 
        + PrependDirectory(mFaudesDocRefsrc,"faudes_about.fref") +  " " + dst;
    std::cerr << "flxinstall: fix html index " << std::endl;
    if(std::system(proccmd.c_str())!=0) {
      std::cerr << "flxinstall: error when processing index.html: ERROR." <<std::endl;
      exit(1);
    }
  }

  // copy index file: luafaudes
  if(mFaudesDocLuafaudes!="" && DirectoryExists(PrependDirectory(mFaudesDocRefsrc,"luafaudes"))){
    std::string dst=PrependDirectory(mFaudesDocLuafaudes,"index.html");
    std::string proccmd= mFaudesBinRef2html 
        + " -rti " + mFaudesDocRti + " -flx " + mFaudesBinLuaflx + " -cnav " + mFaudesDocNav
        + " -css " + mFaudesDocCss + " -inc " + mFaudesDocToc + " -rel ../  " 
        + PrependDirectory(mFaudesDocRefsrc,"luafaudes/faudes_luafaudes.fref") +  " " + dst;
    std::cerr << "flxinstall: fix html index " << std::endl;
    if(std::system(proccmd.c_str())!=0) {
      std::cerr << "flxinstall: error when processing index.html: ERROR." <<std::endl;
      exit(1);
    }
  }

  // copy index file: reference
  if(mFaudesDocReference!="" && DirectoryExists(PrependDirectory(mFaudesDocRefsrc,"reference"))){
    std::string dst=PrependDirectory(mFaudesDocReference,"index.html");
    std::string proccmd= mFaudesBinRef2html 
        + " -rti " + mFaudesDocRti + " -flx " + mFaudesBinLuaflx + " -cnav " + mFaudesDocNav
        + " -css " + mFaudesDocCss + " -inc " + mFaudesDocToc + " -rel ../  " 
        + PrependDirectory(mFaudesDocRefsrc,"reference/reference_index.fref") +  " " + dst;
    std::cerr << "flxinstall: fix html index " << std::endl;
    if(std::system(proccmd.c_str())!=0) {
      std::cerr << "flxinstall: error when processing index.html: ERROR." <<std::endl;
      exit(1);
    }
  }

  // clean tmp 
  // (dont do so: keep frefs for dstinstall to extract index for qhc)
  /*
  for(std::set < std::string >::iterator fit=tmpfiles.begin(); fit!=tmpfiles.end(); fit++) {
    std::string dfile=PrependDirectory(mFaudesDocTemp,*fit);
    RemoveFile(*fit);
  }
  */

  std::cerr << "flxinstall: done" << std::endl;
}


// ******************************************************************
// extract extension
// ******************************************************************

// control
void ExtractExtensionFile(void) {

  // test extension
  std::string ext=ExtractExtension(mSourceFile);
  std::string bas=ExtractBasename(mSourceFile);
  if(ext!="flx") 
    usage_exit("extract must specify a *.flx source");
  // prepare to read
  TokenReader tr(mSourceFile);
  Token btag;
  tr.ReadBegin("LuaExtension",btag);
  if(!btag.ExistsAttributeString("name")) {
    std::cerr << "flxinstall: lua-extension must have a name attribute " << tr.FileLine() << ": ERROR." << std::endl;
    exit(1);
  }
  mExtensionName=btag.AttributeStringValue("name");  
  // scan for relevant sections
  while(!tr.Eos("LuaExtension")) {
    tr.Peek(btag);
    // skip tokens
    if(!btag.IsBegin()) {
      tr.Get(btag);
      continue;
    }
    // switch sections: fref
    if(btag.StringValue()=="ReferencePage") {
      // figure destination
      if(!btag.ExistsAttributeString("page")) {
        std::cerr << "flxinstall: skipping referencepage without page attribute" << std::endl;
        tr.ReadBegin(btag.StringValue());
        tr.ReadEnd(btag.StringValue());
        continue;
      }
      std::string page=mExtensionName + "_" + btag.AttributeStringValue("page") +".fref";
      std::transform(page.begin(), page.end(), page.begin(), tolower);
      std::string dstname= mTarget + faudes_pathsep() + page;
      std::cerr << "flxinstall: extracting reference page to \"" << dstname << "\"" << std::endl;
      // do copy incl outer tags
      TokenWriter tw(dstname,"ReferencePage");
      InsertReferencePage(tr,tw,mExtensionName);
      continue;
    }
    // switch sections: lua function
    if(btag.StringValue()=="LuaFunctionDefinition") {
      // figure destination
      if(!btag.ExistsAttributeString("name")) { 
        std::cerr << "flxinstall: skipping lua function without name attribute" << std::endl;
        tr.ReadBegin(btag.StringValue());
        tr.ReadEnd(btag.StringValue());
        continue;
      }
      std::string name=btag.AttributeStringValue("name");
      size_t pos=name.find("::"); // test this construct for "xyz::"
      if(pos!=std::string::npos) name=name.substr(pos+2);
      name = name +".rti";
      std::transform(name.begin(), name.end(), name.begin(), tolower);
      std::string dstname= mTarget + faudes_pathsep() + name;
      std::cerr << "flxinstall: extracting lua function to \"" << dstname << "\"" << std::endl;
      // do copy incl outer tags
      TokenWriter tw(dstname,"LuaFunctionDefinition");
      InsertLuaFunction(tr,tw);
      continue;
    }
    // switch sections: image file
    if(btag.StringValue()=="ImageFile") {
      // figure destination
      if(!btag.ExistsAttributeString("name")) { 
        std::cerr << "flxinstall: skipping image file without name attribute" << std::endl;
        tr.ReadBegin(btag.StringValue());
        tr.ReadEnd(btag.StringValue());
        continue;
      }
      std::string name= btag.AttributeStringValue("name");
      std::transform(name.begin(), name.end(), name.begin(), tolower);
      std::string dstname= mTarget + faudes_pathsep() + name;
      std::cerr << "flxinstall: extracting image file to \"" << dstname << "\"" << std::endl;
      TokenWriter tw(dstname);
      // read data
      tr.ReadBegin("ImageFile");
      Token data;
      tr.Get(data);
      if(!data.IsBinary()){
      }
      // copy to C++ stream
      std::fstream fsout;
      fsout.exceptions(std::ios::badbit|std::ios::failbit);
      try{
        fsout.open(dstname.c_str(), std::ios::out | std::ios::binary); 
        fsout.write(data.StringValue().c_str(),data.StringValue().size());
        fsout.close();
      } 
      catch (std::ios::failure&) {
        std::cerr << "flxinstall: file io error when writing  \"" << dstname << "\"" << std::endl;
      }
      // done
      tr.ReadEnd("ImageFile");
      continue;
    }
    // switch sections: data file
    if(btag.StringValue()=="DataFile") {
      // figure destination
      if(!btag.ExistsAttributeString("name")) { 
        std::cerr << "flxinstall: skipping data file without name attribute" << std::endl;
        tr.ReadBegin(btag.StringValue());
        tr.ReadEnd(btag.StringValue());
        continue;
      }
      std::string name= "data" + faudes_pathsep() + btag.AttributeStringValue("name");
      std::transform(name.begin(), name.end(), name.begin(), tolower);
      std::string dstname= mTarget + faudes_pathsep() + name;
      std::cerr << "flxinstall: extracting data file to \"" << dstname << "\"" << std::endl;
      // insist in data directiory
      MakeDirectory(mTarget,"data");
      TokenWriter tw(dstname);
      // read data
      tr.ReadBegin("DataFile");
      Token data;
      tr.Peek(data);
      // case 1: binary
      if(data.IsBinary()){
        tr.Get(data);
        // copy to C++ stream
        std::fstream fsout;
        fsout.exceptions(std::ios::badbit|std::ios::failbit);
        try{
          fsout.open(dstname.c_str(), std::ios::out | std::ios::binary); 
          fsout.write(data.StringValue().c_str(),data.StringValue().size());
          fsout.close();
        } catch (std::ios::failure&) {
          std::cerr << "flxinstall: file io error when writing  \"" << dstname << "\"" << std::endl;
        }
      }
      // case 2: token stream
      else if(data.IsBegin()){
        // copy to token writer excl outer tags
        InsertSection(tr,tw,data.StringValue());
      }
      // case 3: error
      else {
        std::cerr << "flxinstall: skipping invalid data " << tr.FileLine() << std::endl;
      } 
      // read end tag
      tr.ReadEnd("DataFile");
      continue;
    }
    // switch sections: lua tutorial
    if(btag.StringValue()=="LuaTutorial") {
      // figure destination
      if(!btag.ExistsAttributeString("name")) { 
        std::cerr << "flxinstall: skipping tutorial without name attribute" << std::endl;
        tr.ReadBegin(btag.StringValue());
        tr.ReadEnd(btag.StringValue());
        continue;
      }
      std::string name=btag.AttributeStringValue("name");
      std::transform(name.begin(), name.end(), name.begin(), tolower);
      std::string dstname= mTarget + faudes_pathsep() + name;
      std::cerr << "flxinstall: extracting tutorial to \"" << dstname << "\"" << std::endl;
      // do copy
      TokenWriter tw(dstname);
      InsertPlainLuaTutorial(tr,tw);
      continue;
    }
    // skip unknown
    tr.ReadBegin(btag.StringValue());
    tr.ReadEnd(btag.StringValue());
  }
  // done
  tr.ReadEnd("LuaExtension");
}

// ******************************************************************
// run test cases
// ******************************************************************

// uses mSource for std flx

void RunTestCases() {
  // insist in luafaudes
  if(!FileExists(mFaudesBinLuafaudes)) {
    std::cerr << "flxinstall: cannot execute luafaudes" << std::endl;
    exit(1);
  }
  // read target directory to extract lua scripts
  std::set< std::string > allfiles = ReadDirectory(mTarget);
  std::set< std::string > luascripts;
  for(std::set < std::string >::iterator fit=allfiles.begin(); fit!=allfiles.end(); fit++) 
    if(ExtractExtension(*fit)=="lua") luascripts.insert(*fit);
  // loop scripts
  for(std::set < std::string >::iterator fit=luascripts.begin(); fit!=luascripts.end(); fit++) {
    // build command
    std::string cmd = mFaudesBinLuafaudes + " -x " +mSourceFile + " " + *fit;
    // run
    std::cerr << "flxinstall: execute: " << cmd << std::endl;
    int sysret=std::system(cmd.c_str());
    if(sysret!=0) {
      std::cerr << "flxinstall: error while running lua script \"" << *fit << "\"" << std::endl;
      exit(1);
    }
  }


}

// ******************************************************************
// command line ui
// ******************************************************************


int main(int argc, char *argv[]) {

  // local config 
  bool doc=false;
  bool doi=false;
  bool dor=false;
  bool dox=false;
  bool dot=false;

  // min args
  if(argc < 2) usage_exit();

  // primitive commad line parsing
  int i;
  for(i=1; i<argc; i++) {
    std::string option(argv[i]);
    // overwrite doc
    if(option=="-tdoc") { 
      i++; if(i>=argc) usage_exit();
      mFaudesDoc=argv[i];
      continue;
    }
    // overwrite bin
    if(option=="-tbin") { 
      i++; if(i>=argc) usage_exit();
      mFaudesBin=argv[i];
      continue;
    }
    // overwrite rti
    if(option=="-trti") { 
      i++; if(i>=argc) usage_exit();
      mFaudesDocRti=argv[i];
      continue;
    }
    // overwrite cnav
    if(option=="-tcnav") { 
      i++; if(i>=argc) usage_exit();
      mFaudesDocNav=argv[i];
      continue;
    }
    // overwrite css
    if(option=="-tcss") { 
      i++; if(i>=argc) usage_exit();
      mFaudesDocCss=argv[i];
      continue;
    }
    // target standalone
    if(option=="-tapp") { 
      mFaudesStandalone = true;
      continue;
    }
    // mode: compile
    if(option=="-c") { 
      i++; doc=true;
      break;
    }
    // mode: install
    if(option=="-i") { 
      i++; doi=true;
      break;
    }
    // mode: remove
    if(option=="-r") { 
      i++; dor=true;
      break;
    }
    // mode: extract
    if(option=="-x") { 
      i++; dox=true;
      break;
    }
    // mode: test
    if(option=="-t") { 
      i++; dot=true;
      break;
    }
    // option: help
    if((option=="-?") || (option=="--help")) {
      usage_exit();
      continue;
    }
    // option: unknown
    if(option.size()>1)
    if(option.at(0)=='-') {
      usage_exit("unknown option " + option);
      continue;
    }
    // must choose mode
    usage_exit("must set either -c, -i, -r, -x or -t mode" );
  }

  // create
  if(doc) {
    // figure source files 
    for(;i<argc-1;i++) {
      mSourceFiles.insert(std::string(argv[i]));
    }
    // convenience: if its a directory, use all files inside
    if(mSourceFiles.size()==1) {
      std::set< std::string > srcfiles = ReadDirectory(std::string(argv[i-1]));
      srcfiles=EraseHiddenFiles(srcfiles);
      if(srcfiles.size()>0) {
        mSourceFiles.clear();
        for(std::set < std::string >::iterator fit=srcfiles.begin(); fit!=srcfiles.end(); fit++) 
          mSourceFiles.insert(PrependDirectory(std::string(argv[i-1]),*fit));
      }
    }
    // have a target file
    if(!(i<argc))
      usage_exit("target *.flx-file not specified");
    //  figure target file
    mTarget=std::string(argv[i]);
    // consistency: insist in .flx target
    if(mTarget!="-")
    if(ExtractExtension(mTarget)!="flx") 
      usage_exit("target *.flx-file not specified");
    // doit
    CreateExtensionFile();
    exit(0);
  }

  // install
  if(doi) {
    // have at least one source
    if(!(i<argc-1)) 
      std::cerr << "flxinstall: no sources specified" << std::endl;
    // figure source files 
    for(;i<argc-1;i++) {
      mSourceFiles.insert(std::string(argv[i]));
    }
    // convenience: if its a directory, use all files inside
    if(mSourceFiles.size()==1) {
      std::set< std::string > srcfiles = ReadDirectory(std::string(argv[i-1]));
      srcfiles=EraseHiddenFiles(srcfiles);
      if(srcfiles.size()>0) {
        mSourceFiles.clear();
        for(std::set < std::string >::iterator fit=srcfiles.begin(); fit!=srcfiles.end(); fit++) 
          if(ExtractExtension(*fit)=="flx")   
            mSourceFiles.insert(PrependDirectory(std::string(argv[i-1]),*fit));
      }
    }
    // insist in flx source
    for(std::set < std::string >::iterator fit=mSourceFiles.begin(); fit!=mSourceFiles.end(); fit++) 
      if(ExtractExtension(*fit)!="flx") 
        usage_exit("sources must be *.flx-files: "+ *fit);
    // figure target directory
    mTarget=std::string(argv[i]);
    TestFaudesTarget();
    // doit
    InstallExtensionFiles();
    exit(0);
  }

  // remove
  if(dor) {
    // have at least one file
    if(i!=argc-1)
      usage_exit("target not specified");
    //  figure target file
    mTarget=std::string(argv[i]);
    TestFaudesTarget();
    // doit
    InstallExtensionFiles();
    if(FileExists(mFaudesBinLuaflx)) FileDelete(mFaudesBinLuaflx);
    exit(0);
  }

  // xtract
  if(dox) {
    // insist
    if(!(i<argc-1))
      usage_exit("source and destination must be specified");
    // figure source
    mSourceFile=std::string(argv[i++]);
    if(ExtractExtension(mSourceFile)!="flx") 
      usage_exit("source must be an *.flx-file");
    // destination
    mTarget=std::string(argv[i++]);
    // test consistent args
    if((i<argc))
      usage_exit("too many arguments");
    // doit
    MakeDirectory(".",mTarget);
    ExtractExtensionFile();
    exit(0);
  }

  // test
  if(dot) {
    // insist
    if(!(i<argc-1))
      usage_exit("source and temp dir must be specified");
    // figure source
    mSourceFile=std::string(argv[i++]);
    if(ExtractExtension(mSourceFile)!="flx") 
      usage_exit("source must be an *.flx-file");
    // destination
    mTarget=std::string(argv[i++]);
    // test consistent args
    if((i<argc))
      usage_exit("too many arguments");
    // insist in target to be the current directory and empty
    if((mTarget != ".") && (mTarget != "./"))
      usage_exit("target must be \".\" or \"./\"");
    std::set< std::string > curdir = ReadDirectory(mTarget);
    if(curdir.size()!=0)
      usage_exit("target must be empty");
    // test for luafaudes (expects mFaudesBin)
    TestLuafaudes();
    // do extract and test
    ExtractExtensionFile();
    RunTestCases();
    // done
    exit(0);
  }

  // error
  usage_exit();

  // never get here
  return 1;
}
