/** @file cfl_tokenwriter.cpp @brief Class TokenWriter */

/* FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2006  Bernd Opitz
Copyright (C) 2006, 2010, 2024  Thomas Moor
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


#include "cfl_tokenwriter.h"

namespace faudes {

// TokenWriter(rFilename, openmode)
TokenWriter::TokenWriter(const std::string& rFilename, std::ios::openmode openmode) 
  : mMode(File), pSStream(0), mHasOutBuffer(false), mFileName(rFilename), mColumns(80/FD_NAMELEN), mColCount(0) {
  // set up mFStream
  mFStream.exceptions(std::ios::badbit|std::ios::failbit);
  try{
    mFStream.open(rFilename.c_str(), openmode); 
  }
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \""<< rFilename << "\"";
    throw Exception("TokenWriter::TokenWriter", errstr.str(), 2);
  }
  // use mFStream
  mpStream=&mFStream;      
  mEndl=true;
}

// TokenWriter(rFilename, doctype)
TokenWriter::TokenWriter(const std::string& rFilename, const std::string& ftype) 
  : mMode(XmlFile), pSStream(0), mHasOutBuffer(false), mFileName(rFilename), mColumns(80/FD_NAMELEN), mColCount(0) {
  // set up mFStream
  mFStream.exceptions(std::ios::badbit|std::ios::failbit);
  try{
    mFStream.open(rFilename.c_str(), std::ios::out|std::ios::trunc); 
    mFStream << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" standalone=\"no\"?>" << std::endl;
    if(ftype!="") 
    if(ftype!="Void") {
      std::string dtdfile=ftype;
      std::transform(dtdfile.begin(), dtdfile.end(), dtdfile.begin(), tolower);
      dtdfile="http://www.faudes.org/dtd/1.0/"+dtdfile+".dtd";
      mFStream << "<!DOCTYPE " << ftype << " SYSTEM \"" << dtdfile << "\">" << std::endl;
    }
  }
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing xml file \""<< rFilename << "\"";
    throw Exception("TokenWriter::TokenWriter", errstr.str(), 2);
  }
  // use mFStream
  mpStream=&mFStream;      
  mEndl=true;
}

// TokenWriter(mode)
TokenWriter::TokenWriter(Mode mode)
  : mMode(mode), pSStream(0), mHasOutBuffer(false), mFileName(""), mColumns(80/FD_NAMELEN), mColCount(0) {
  switch(mode) {
  case Stdout: 
    // set up mFStream
    /*
      try {
      mFStream.copyfmt(std::cout);
      mFStream.clear(std::cout.rdstate());
      typedef std::basic_ios<char> ___basic_ios_char_; // trick for vc++
      mFStream.___basic_ios_char_::rdbuf(std::cout.rdbuf());
      }
      catch (std::ios::failure&) {
      std::stringstream errstr;
      errstr << "Exception opening/writing file \""<< mFileName << "\"";
      throw Exception("TokenWriter::TokenWriter", errstr.str(), 2);
      }
      // use mFStream
      mpStream=&mFStream;
    */
    // use std::cout
    /*
    mpStream= &std::cout;
    mEndl=true;
    */
    // use mSStream to buffer and write to faudes ConsoleOut
    mFileName="stdout";
    mpStream=&mSStream;
    mEndl=true;
    break;
  case String:
    // use mSStream
    mFileName="string";
    mpStream=&mSStream;
    mEndl=false;
    break;
  default:
    std::stringstream errstr;
    errstr << "Invalid Mode / Not Implemented";
    throw Exception("TokenWriter::TokenWriter", errstr.str(), 2);
  }
}


// TokenWriter(stream)
TokenWriter::TokenWriter(std::ostream& stream, const std::string& ftype) 
  : mMode(Stream), pSStream(&stream), mHasOutBuffer(false), mFileName("stream"), mColumns(0), mColCount(0) {
  // xml if there is a doctype
  try {
    if(ftype!="") 
    if(ftype!="Void") {
      std::string dtdfile=ftype;
      std::transform(dtdfile.begin(), dtdfile.end(), dtdfile.begin(), tolower);
      dtdfile="http://www.faudes.org/dtd/1.0/"+dtdfile+".dtd";
      *pSStream << "<!DOCTYPE " << ftype << " SYSTEM \"" << dtdfile << "\">" << std::endl;
      mMode=XmlStream;
    }
  } catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing xml stream";
    throw Exception("TokenWriter::TokenWriter", errstr.str(), 2);
  }
  // use provided stream
  mpStream=pSStream;      
  mEndl=true;
}

// destructor
TokenWriter::~TokenWriter(void) {
  Flush();
  if(mMode==File) mFStream.close();
}

// Flush buffers
void TokenWriter::Flush(void) {
  DoFlush(); // incl linefeed if col>0
  mpStream->flush();
  if(mMode==Stdout) {
    bool m=ConsoleOut::G()->Mute();
    ConsoleOut::G()->Mute(false);
    ConsoleOut::G()->Write(mSStream.str());
    ConsoleOut::G()->Mute(m);
    mSStream.str("");
  }
}

// Str()
std::string TokenWriter::Str(void) {
  if(mMode!=String) {
    std::stringstream errstr;
    errstr << "Not in String Mode";
    throw Exception("TokenWriter::Str()", errstr.str(), 2);
  }
  Flush();
  return mSStream.str();
}

// Stream()
std::ostream* TokenWriter::Streamp(void) {
  DoFlush(0); // no extra formating
  return mpStream;
}
 


// Columns()
int TokenWriter::Columns(void) const {
  return mColumns;
}

// Columns()
void TokenWriter::Columns(int columns) {
  mColumns = columns;
}

// Endl()
void TokenWriter::Endl(void) {
  DoFlush(1);
  try{
    if(mEndl) *mpStream << std::endl;
    else *mpStream << " ";
  }
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << mFileName << "\"";
    throw Exception("Generator::write", errstr.str(), 2);
  }
}

// Endl(bool)
void TokenWriter::Endl(bool on) {
  mEndl=on;
}

// DoFlush: write buffered token
// Note: dont call Endl() since this calls DoFlush
void TokenWriter::DoFlush(bool clf) {
  if(!mHasOutBuffer) return;
  FD_DV("TokenWriter::DoFlush()");
  try{
    // write preceeding space
    for(size_t i=0; i< mOutBuffer.mPreceedingSpace.size(); ++i) {
      char c= mOutBuffer.mPreceedingSpace.at(i);
      if(c=='\n') {
        if(mEndl) *mpStream << std::endl;
        else *mpStream << " ";
        mColCount=0; ;
        continue;
      }
      if(c=='\r') continue;
      if(isspace(c)) mpStream->put(c);
    }
    // do the write
    mOutBuffer.Write(mpStream);
    // count my columns
    mColCount++;
    // have closing linefeed for true flush
    if(clf) {
      if(mOutBuffer.IsBegin() || mOutBuffer.IsEnd()) 
 	if(mColCount==1)
          if(mEndl) { *mpStream << std::endl; mColCount=0;}
      mOutBuffer.SetNone();
    }
  }
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << mFileName << "\"";
    throw Exception("TokenWriter::Write(token)", errstr.str(), 2);
  }
  mHasOutBuffer=false;
}

// Write(rToken)
void TokenWriter::Write(Token& rToken) {
  FD_DV("TokenWriter::Write(token)");
  // figure wether we can merge to an empty section:
  // 1. buffer begin must match the current token end tag
  if(rToken.IsEnd() && !rToken.IsBegin()) 
  if(mHasOutBuffer) 
  if(mOutBuffer.IsBegin() && !mOutBuffer.IsEnd())
  if(mOutBuffer.StringValue()==rToken.StringValue()) 
  // 2. dont do it on HTML tags since it irritates bowsers
  if(mOutBuffer.StringValue()!="b")
  if(mOutBuffer.StringValue()!="i")
  if(mOutBuffer.StringValue()!="tt")
  if(mOutBuffer.StringValue()!="p")
  if(mOutBuffer.StringValue()!="h1")
  if(mOutBuffer.StringValue()!="h2")
  if(mOutBuffer.StringValue()!="h3")
  if(mOutBuffer.StringValue()!="h4")
  if(mOutBuffer.StringValue()!="font")
  if(mOutBuffer.StringValue()!="strong")
  {
    mOutBuffer.mType |= Token::End;
    DoFlush(0);
    return;
  }
  // flush buffer 
  if(mHasOutBuffer) DoFlush(0);  
  // prefer markup in new line before
  if(rToken.PreceedingSpace()=="")
  if(rToken.IsBegin() || rToken.IsEnd()) 
  if(mColCount > 0)
    rToken.PreceedingSpace("\n");
  // prefer markup in new line after 
  if(rToken.PreceedingSpace()=="")
  if(mOutBuffer.IsBegin() || mOutBuffer.IsEnd()) 
  if(mColCount == 1)
    rToken.PreceedingSpace("\n");
  // columncount proposes linefeed
  if(rToken.PreceedingSpace()=="")
  if(mColCount >= mColumns && (mColumns>0) && mEndl) {
    rToken.mPreceedingSpace="\n";
  }
  // insist in space between data tokens
  if(rToken.PreceedingSpace()=="")
  if(!(rToken.IsBegin()) && (!rToken.IsEnd())) 
  if(!(mOutBuffer.IsBegin()) && (!mOutBuffer.IsEnd())) {
    rToken.mPreceedingSpace=" ";
  }
  // record token to buffer
  mOutBuffer=rToken;
  mHasOutBuffer=true;
}


// WriteString(rName)
void TokenWriter::WriteString(const std::string& rName) {
  if((rName == "\n") || (rName == "\r\n")) {
    Endl();
    mColCount = 0;
    return;
  }
  Token token;
  token.SetString(rName);
  Write(token);
}


// WriteText(rText)  --- depreciated, use below variants
void TokenWriter::WriteText(const std::string& rText) {
  try {
    DoFlush();
    Token::WriteEscapedString(mpStream,rText);
  } 
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << mFileName << "\"";
    throw Exception("TokenWriter::WriteText(text)", errstr.str(), 2);
  }
}

// WriteText(rBegin,rText)
void TokenWriter::WriteText(Token& rBeginTag, const std::string& rText) {
  if(!rBeginTag.IsBegin() || rBeginTag.IsEnd()) {
    std::stringstream errstr;
    errstr << "Invalid begin token while writing file \"" << mFileName << "\"";
    throw Exception("TokenWriter::WriteText(label,text)", errstr.str(), 2);
  }
  try {
    Write(rBeginTag);
    Flush();
    Token::WriteEscapedString(mpStream,rText);
    Endl();
    WriteEnd(rBeginTag.StringValue());
  } 
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception writing file \"" << mFileName << "\"";
    throw Exception("TokenWriter::WriteText(label,text)", errstr.str(), 2);
  }
}


// WriteText(rLabel,rText) -- wrapper
void TokenWriter::WriteText(const std::string& rLabel, const std::string& rText) {
  Token btag;
  btag.SetBegin(rLabel);
  WriteText(btag,rText);
}


// WriteVerbatim(rData)
void TokenWriter::WriteVerbatim(Token& rBeginTag, const std::string& rText) {
  FD_DV("TokenWriter::Write(token)");
  if(!rBeginTag.IsBegin() || rBeginTag.IsEnd()) {
    std::stringstream errstr;
    errstr << "Invalid begin token while writing file \"" << mFileName << "\"";
    throw Exception("TokenWriter::WriteVerbatim(tag,text)", errstr.str(), 2);
  }
  Write(rBeginTag);
  Flush();
  try{
    if(mColCount !=0) Endl();
    Token::WriteVerbatim(mpStream,rText,1); // flag: add linefeeds
    mColCount = 0;
    Endl();
  }
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << mFileName << "\"";
    throw Exception("TokenWriter::WriteVerbatim()", errstr.str(), 2);
  }
  WriteEnd(rBeginTag.StringValue());
}

// WriteVerbatim(rLabel,rText) -- wrapper
void TokenWriter::WriteVerbatim(const std::string& rLabel, const std::string& rText) {
  Token btag;
  btag.SetBegin(rLabel);
  WriteVerbatim(btag,rText);
}


// WriteCharacterData(rCData)
void TokenWriter::WriteCharacterData(const std::string& rCharacterData) {
  // bail out on trivial to avoid unnecessary flush
  if(rCharacterData.size()==0) return;
  // do write
  try {
    DoFlush(0); // no extra linefeed formating here
    *mpStream << rCharacterData;
  } 
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << mFileName << "\"";
    throw Exception("TokenWriter::WriteCharacterData(text)", errstr.str(), 2);
  }
  // adjust column count
  mColCount=mColumns;
  if(rCharacterData.size()>=1)
  if(rCharacterData.at(rCharacterData.size()-1)=='\n')
    mColCount=0;
}

// WriteInteger(index)
void TokenWriter::WriteInteger(Idx index) {
  Token token;
  token.SetInteger(index);
  Write(token);
}

// WriteInteger(index)
void TokenWriter::WriteInteger16(long int val) {
  Token token;
  token.SetInteger16(val);
  Write(token);
}

// WriteFloat(float)
void TokenWriter::WriteFloat(const double& val) {
  Token token;
  token.SetFloat(val);
  Write(token);
}


// WriteOption(rOpt)
void TokenWriter::WriteOption(const std::string& rOpt) {
  Token token;
  token.SetOption(rOpt);
  Write(token);
}


// WriteBegin(rLabel)
void TokenWriter::WriteBegin(const std::string& rLabel) {
  Token token;
  token.SetBegin(rLabel);
  Write(token);
}

// WriteEnd(rLabel)
void TokenWriter::WriteEnd(const std::string& rLabel) {
  Token token;
  token.SetEnd(rLabel);
  Write(token);
}

// WriteBegin(rLabel)
void TokenWriter::WriteEmpty(const std::string& rLabel) {
  Token token;
  token.SetEmpty(rLabel);
  Write(token);
}

// WriteComment(comment)
void TokenWriter::WriteComment(const std::string& comment){
  DoFlush();
  // auto xml 
  if(mMode==XmlFile || mMode==XmlStream) { WriteXmlComment(comment); return; }
  // no endl ... no comment
  if(!mEndl) return; 
  try{
    if(mColCount!=0)  Endl();
    // fix comment indicator
    if(comment.length()==0) 
      *mpStream << "% ";
    if(comment.length()>0) 
    if(comment.at(0)!='%')
      *mpStream << "% ";
    // xml/endl escape
    std::string::const_iterator cit=comment.begin(); 
    for(;cit!=comment.end(); cit++) {
      if(*cit=='<') 
        { *mpStream << "&lt;"; continue;}
      if(*cit=='>') 
        { *mpStream << "&gt;"; continue;}
      if(*cit=='&') 
       { *mpStream << "&amp;"; continue;}
      if(*cit=='\n') 
       { *mpStream << " "; continue;}
      if(*cit=='\r') 
       { ; continue;}
      *mpStream << *cit;
    }
    // done
    Endl();
    mColCount = 0;
  }
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << mFileName << "\"";
    throw Exception("TokenWriter::Comment", errstr.str(), 2);
  }
}

// WriteXmlComment(comment)
void TokenWriter::WriteXmlComment(const std::string& comment){
  if(!mEndl) return; //  no endl implies no comments
  DoFlush();
  try{
    if(mColCount!=0)  Endl();
    // begin tag
    *mpStream << "<!-- ";
    // test for multiline
    static const std::string newline="\n\r";
    if(comment.find_first_of(newline)!=std::string::npos) 
      Endl();
    // xml/endl escape
    std::string::const_iterator cit=comment.begin(); 
    for(;cit!=comment.end(); cit++) {
      if(*cit=='>') 
        { *mpStream << "&gt;"; continue;}
      *mpStream << *cit;
    }
    // end tag
    *mpStream << " -->";
    // done
    Endl();
    mColCount = 0;
  }
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << mFileName << "\"";
    throw Exception("TokenWriter::Comment", errstr.str(), 2);
  }
}

// write base64
void TokenWriter::WriteBinary(const char* pData, long int len) {
  DoFlush();
  Token::WriteBinary(mpStream,pData,len);
}

// FileName()
std::string TokenWriter::FileName(void) const {
  return mFileName;
}




} // namespace faudes
