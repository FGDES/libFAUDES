/** @file cfl_tokenreader.cpp @brief Class TokenReader */

/* FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2006  Bernd Opitz
Copyright (C) 2006  Thomas Moor
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
#include "cfl_tokenreader.h"

namespace faudes {

// TokenReader(mode,instring)
TokenReader::TokenReader(Mode mode, const std::string& rInString)
  : mMode(mode), mpStream(NULL), mFileName("") 
{
  switch(mode) {
  case String: 
    // use mSStream
    FD_DV("TokenReader::Tokenreader(String, ...): " << rInString);
    mpSStream= new std::istringstream(rInString, std::istringstream::in | std::istringstream::binary);
    mpStream= mpSStream;
    Rewind();
    break;
  case File: 
    // set up mFStream
    FD_DV("TokenReader::Tokenreader(File, \"" << rInString <<"\")");
    mFStream.exceptions(std::ios::badbit|std::ios::failbit);
    try{
      mFStream.open(rInString.c_str(), std::ios::in | std::ios::binary); 
    } 
    catch (std::ios::failure&) {
      std::stringstream errstr;
      errstr << "Exception opening/reading file \""<< rInString << "\"";
      throw Exception("TokenReader::TokenReader", errstr.str(), 1);
    }
    mFileName=rInString;
    mpStream=&mFStream;
    Rewind();
    break;
  default:
    std::stringstream errstr;
    errstr << "Invalid Mode / Not implemented";
    throw Exception("TokenReader::TokenReader(mode,instring)", errstr.str(), 1);
  }
}


// TokenReader(rFilename)
TokenReader::TokenReader(const std::string& rFilename) 
  : mMode(File), mpStream(NULL), mFileName(rFilename) 
{
  // set up mFStream
  FD_DV("TokenReader::Tokenreader(File, \"" << rFilename <<"\")");
  mFStream.exceptions(std::ios::badbit|std::ios::failbit);
  try{
    mFStream.open(rFilename.c_str(), std::ios::in | std::ios::binary); 
  } 
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/reading file \""<< rFilename << "\"";
    throw Exception("TokenReader::TokenReader", errstr.str(), 1);
  }
  mFileName=rFilename;
  mpStream=&mFStream;
  Rewind();
}


//  destruct
TokenReader::~TokenReader(void) {
  if(mMode==String) delete mpSStream;
}

// Stream()
std::istream* TokenReader::Streamp(void) {
  return mpStream;
}

// Rewind()
void TokenReader::Rewind(void) {
  FD_DV("TokenReader::Rewind: \"" << mFileName <<"\"");
  try {
    mpStream->clear();
    mpStream->seekg(0);
    mPeekToken.SetNone();
    mLevel=0;
    mLineCount=1;
    mFilePos=0;
    mFaudesComments=true;
    mLevelState.resize(mLevel+1);
    mLevelState.back().mLabel="OUTER";
    mLevelState.back().mStartPosition=mFilePos;
    mLevelState.back().mStartLine=mLineCount;
    mLevelState.back().mStartPeek=mPeekToken;
    mLevelState.back().mFaudesComments=mFaudesComments;
  }
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/reading file in "<< FileLine();
    throw Exception("TokenReader::Rewind", errstr.str(), 1);
  }
}


// FileName()
std::string TokenReader::FileName(void) const {
  return mFileName;
}

// Peek(token)
bool TokenReader::Peek(Token& token) {
  // read to peek buffer
  if(mPeekToken.IsNone()) { 
    try{
      mLineCount += mPeekToken.Read(mpStream,mFaudesComments);
    } catch (std::ios::failure&) {
      std::stringstream errstr;
      errstr << "Exception opening/reading file in "<< FileLine();
      throw Exception("TokenReader::Peek", errstr.str(), 1);
    }
  }
  // get from peek buffer 
  token=mPeekToken;
  // substitute empty sections
  if(token.IsEmpty())  token.ClrEnd();
  // done
  FD_DV("TokenReader: Peek: " << token.Str());
  return !token.IsNone();
}

// Get(token)
bool TokenReader::Get(Token& token) {
  bool res;
  // get token from peek buffer 
  res=Peek(token);
  // invalidate buffer: case a
  if(!mPeekToken.IsEmpty()) mPeekToken.SetNone();
  // invalidate buffer: case b
  if(mPeekToken.IsEmpty()) {
    FD_DV("TokenReader: fake end : " << mPeekToken.Str());
    mPeekToken.SetEnd(std::string(mPeekToken.StringValue()));
  }
  // bail out on error
  if(!res) return false;
  // ignore misbehavong <br> tag in by level management
  if(token.IsBegin("br") || token.IsEnd("br")) return true;
  // track state (level of nested sections, filepos etc)
  mFilePos=mpStream->tellg();
  if(token.IsBegin()) {
    // track level
    mLevel++;
    // update state
    if(token.StringValue()=="ReferencePage") mFaudesComments=false;
    if(token.StringValue()=="html") mFaudesComments=false;
    if(token.StringValue()=="Html") mFaudesComments=false;
    if(token.StringValue()=="HTML") mFaudesComments=false;
    // record state
    mLevelState.resize(mLevel+1);
    mLevelState.back().mLabel=token.StringValue();
    mLevelState.back().mStartPosition=mFilePos;
    mLevelState.back().mStartLine=mLineCount;
    mLevelState.back().mStartPeek=mPeekToken;
    mLevelState.back().mFaudesComments=mFaudesComments;
  }
  if(token.IsEnd()) {
#ifdef FAUDES_CHECKED
    if(token.StringValue()!=mLevelState.back().mLabel)
      FD_WARN("TokenReader::Get(): end of section \"" << token.StringValue() << "\" at " << FileLine() 
	      << " should match \"" << mLevelState.back().mLabel << "\" at line " << mLevelState.back().mStartLine );
#endif
    if(mLevel<1) {
#ifdef FAUDES_CHECKED
      FD_WARN("TokenReader::Get(): Unbalanced end of section \"" << token.StringValue() << "\" at " << FileLine());
#endif
      token.SetNone();
      return false;
    }
    mLevel--;
    mLevelState.pop_back();
    mFaudesComments=mLevelState.back().mFaudesComments;
  }
  FD_DV("TokenReader:Get(): " << token.Str());

  return res;
}

// SeekBegin(label)
void TokenReader::SeekBegin(const std::string& rLabel) {
  Token token;
  SeekBegin(rLabel,token);
}

// SeekBegin(label)
void TokenReader::SeekBegin(const std::string& rLabel,  Token& rToken) {
  // search for begin at any descending level, no rewind
  FD_DV("TokenReader::SeekBegin: " << rLabel << " at " << FileLine() << " level " << mLevel);
  int level=mLevel;
  for (;;) {
    // swollow some plain text (e.g. html may contain plain text that cannot be tokenized properly)
    std::string swallow;
    ReadCharacterData(swallow);
    // exception: did not get a token at all (incl. eof)
    if(!Peek(rToken)) {
      Rewind();
      std::stringstream errstr;
      errstr << "Section \"" << rLabel << "\" expected at " << FileLine() << " no more tokens";
      throw Exception("TokenReader::SeekBegin", errstr.str(), 51);
    }
    // exception: current section ends
    if((rToken.Type() == Token::End) && (mLevel == level)) {
      mpStream->seekg(mLevelState[level].mStartPosition);
      mFilePos=mLevelState[level].mStartPosition;
      mLineCount=mLevelState[level].mStartLine;                       
      mPeekToken=mLevelState[level].mStartPeek;
      mFaudesComments=mLevelState[level].mFaudesComments;
      std::stringstream errstr;
      errstr << "Section \"" << rLabel << "\" expected at " << FileLine() 
	     << "current section ended unexpected. Found: " << rToken.StringValue() << " Type " << rToken.Type();
      throw Exception("TokenReader::SeekBegin", errstr.str(), 51);
    }
    // success: found begin section
    if ((rToken.IsBegin()) && (rToken.StringValue() == rLabel))
      break;
    // go on seeking
    Get(rToken);
  }
}
 
// ReadBegin(label)
void TokenReader::ReadBegin(const std::string& rLabel) {
  Token token;
  ReadBegin(rLabel,token);
}

// ReadBegin(label,token)
void TokenReader::ReadBegin(const std::string& rLabel, Token& rToken) {
  FD_DV("Looking for Section \"" << rLabel << "\"");
  try {
    int level=mLevel;
    int repcnt=0;
    long int startpos=mFilePos;
    FD_DV("section level " << level << " current pos " << startpos << " begin of section " << mLevelState[level].mStartPosition);
    // search for begin at current level
    for (;;) {
      // swallow some plain text (e.g. html may contain plain text that cannot be tokenized properly)
      std::string swallow;
      ReadCharacterData(swallow);
      // exception: did not get a token at all (incl eof)
      if(!Peek(rToken)) {
	std::stringstream errstr;
	errstr << "Section \"" << rLabel << "\" expected at " << FileLine() << ", no token at all";
	throw Exception("TokenReader::ReadBegin Peek", errstr.str(), 51);
      }
      // success: found begin section
      if((rToken.IsBegin()) && (rToken.StringValue() == rLabel) && (mLevel==level)) {
	Get(rToken);
	break;
      }
      // exception: did not find begin label
      if((mFilePos>=startpos) && (repcnt==1)) {
	std::stringstream errstr;
	errstr << "Section \"" << rLabel << "\" expected at " << FileLine() << ", did not find begin label";
	throw Exception("TokenReader::ReadBegin: Missing", errstr.str(), 51);
      }
      // exception: did not find begin label
      if(repcnt>1) {
	std::stringstream errstr;
	errstr << "Section \"" << rLabel << "\" expected at " << FileLine() << ", did not find begin label";
	throw Exception("TokenReader::ReadBegin: Missing", errstr.str(), 51);
      }
      // rewind once when current section ends
      if(rToken.IsEnd() && !rToken.IsBegin() && (mLevel == level)) {
	mpStream->seekg(mLevelState[level].mStartPosition);
        mFilePos=mLevelState[level].mStartPosition;
	mLineCount=mLevelState[level].mStartLine;                       
        mPeekToken=mLevelState[level].mStartPeek;
        mFaudesComments=mLevelState[level].mFaudesComments;
	repcnt++;
	continue;
      }
      // skip this token
      Get(rToken);
    }
  }
  // catch my seek/tell errors
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Section \"" << rLabel << "\" expected at " << FileLine();
    throw Exception("TokenReader::ReadBegin Rewind", errstr.str(), 1);
  }
}
 

// ExistsBegin(label)
bool TokenReader::ExistsBegin(const std::string& rLabel) {
  FD_DV("TokenReader::ExistsBegin(): looking for Section \"" << rLabel << "\"");
  try {
    int level=mLevel;
    int rwcnt=0;
    long int startpos=mFilePos;
    FD_DV("section level " << level << " current pos " << startpos << " begin of section " << mLevelState[level].mStartPosition);
    Token token;
    // search for begin at current level
    for(;;) {
      // swallow some plain text (e.g. html may contain plain text that cannot be tokenized properly)
      std::string swallow;
      ReadCharacterData(swallow);
      // fail: did not get a token at all (e.g. eof)
      if(!Peek(token)) {
        return false;
      }
      // success: found begin section
      if((token.IsBegin()) && (token.StringValue() == rLabel) && (mLevel==level)) {
	return true;
      }
      // rewind once when current section ends
      if(token.IsEnd() && (mLevel == level) && (rwcnt==0)) {
	mpStream->seekg(mLevelState[level].mStartPosition);
        mFilePos=mLevelState[level].mStartPosition;
	mLineCount=mLevelState[level].mStartLine;                       
        mPeekToken=mLevelState[level].mStartPeek;
        mFaudesComments=mLevelState[level].mFaudesComments;
	rwcnt++;;
        if(rwcnt>1) return false; // survive funny mFilePos in e.g. empty sections
   	continue;
      }
      // fail: did not find begin label are one turn around
      if((mFilePos>=startpos) && (rwcnt>0) && (mLevel == level)) {
        return false;
      }
      // skip this token
      Get(token);
    }
  }
  // catch my seek/tell errors
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "IO Error while scanning Section \"" << rLabel << "\" at " << FileLine();
    throw Exception("TokenReader::ExistsBegin IO", errstr.str(), 1);
  }
  return false;
}
 
// ReadEnd(label)
void TokenReader::ReadEnd(const std::string& rLabel) {
  FD_DV("TokenReader::ReadEnd: " << rLabel << " at " << FileLine() );
  // search for end at current level
  int level=mLevel;
  Token token;
  for (;;) {
    // swallow some plain text (e.g. html may contain plain text that cannot be tokenized properly)
    std::string swallow;
    ReadCharacterData(swallow);
    // exception: did not get a token at all
    if(!Peek(token)) {
      std::stringstream errstr;
      errstr << "end of section \"" << rLabel << "\" expected at " << FileLine();
      throw Exception("TokenReader::ReadEnd", errstr.str(), 51);
    }
    // success: found end of current section
    if(token.IsEnd() && !token.IsBegin() && (token.StringValue() == rLabel) && (mLevel==level)) {
      Get(token);
      break;
    }
    // exception: current section ends with unexpected label
    if(mLevel<level) {
      std::stringstream errstr;
      errstr << "end of Section \"" << rLabel << "\" expected at " << FileLine();
      throw Exception("TokenReader::ReadEnd", errstr.str(), 51);
    }
    // get the token and continue
    Get(token);
    //std::cout << token.Str() << "\n";
  }
}

// Recover()
bool TokenReader::Recover(int level) {
  // paranoia
  if(level<0) return false;
  // trivial cases
  if(level>mLevel) return false;
  if(level==mLevel) return true;
  // loop until match
  Token token;
  while(Get(token)) 
    if(mLevel<=level) break;
  // done
  return level==mLevel;
}

// Recover()
bool TokenReader::Reset(int level) {
  // paranoia
  if(level>mLevel) return false;
  // coonvenience arg: negative becomed reset this level
  if(level<0) level=mLevel;
  // trivial case
  if(level==0) {
    Rewind();
    return true;
  }
  // loop until end
  Token token;
  while(Peek(token)) {
    if((mLevel==level) && token.IsEnd()) break;
    if(mLevel<level) return false;
    Get(token);
  }
  // do the rewind
  mpStream->seekg(mLevelState[level].mStartPosition);
  mFilePos=mLevelState[level].mStartPosition;
  mLineCount=mLevelState[level].mStartLine;                       
  mPeekToken=mLevelState[level].mStartPeek;
  mFaudesComments=mLevelState[level].mFaudesComments;
  return true;
}

// Eos(label)
bool TokenReader::Eos(const std::string& rLabel) {
  // peek token and check for end of section
  Token token;
  Peek(token);
  if(! (token.IsEnd() && !token.IsBegin()))
    return false;
  if((token.IsEnd() && !token.IsBegin())  && (token.StringValue() == rLabel))
    return true; 
  std::stringstream errstr;
  errstr << "section end \"" << rLabel << "\" expected at " << FileLine();
  throw Exception("TokenReader::Eos", errstr.str(), 51);
  return false;
}
  

// ReadInteger()
long int TokenReader::ReadInteger(void) {
  Token token;			
  Get(token);
  if(!token.IsInteger()) {
    std::stringstream errstr;
    errstr << "Integer expected at " << FileLine();
    throw Exception("TokenReader::TokenReader", errstr.str(), 50);
  }
  return token.IntegerValue();
}

// ReadFloat()
double TokenReader::ReadFloat(void) {
  Token token;			
  Get(token);
  if((!token.IsFloat()) && (!token.IsInteger())) {
    std::stringstream errstr;
    errstr << "Float expected at " << FileLine();
    throw Exception("TokenReader::TokenReader", errstr.str(), 50);
  }
  return token.FloatValue();
}

// ReadString()
std::string TokenReader::ReadString(void) {
  Token token;			
  Get(token);
  if(!token.IsString()) {
    std::stringstream errstr;
    errstr << "String expected at " << FileLine();
    throw Exception("TokenReader::TokenReader", errstr.str(), 50);
  }
  return token.StringValue();
}

 
// ReadOption()
std::string TokenReader::ReadOption(void) {
  Token token;			
  Get(token);
  if(!token.IsOption()) {
    std::stringstream errstr;
    errstr << "Option expected at " << FileLine();
    throw Exception("TokenReader::TokenReader", errstr.str(), 50);
  }
  return token.OptionValue();
}

// ReadBinary()
void TokenReader::ReadBinary(std::string& rData) {
  Token token;			
  Get(token);
  if(!token.IsBinary()) {
    std::stringstream errstr;
    errstr << "Binary string expected at " << FileLine();
    throw Exception("TokenReader::TokenReader", errstr.str(), 50);
  }
  rData = token.StringValue();
}


// ReadText()
void TokenReader::ReadText(const std::string& rLabel, std::string& rText) {
  // insist in my begin tag
  Token token;
  Peek(token);
  if(!token.IsBegin(rLabel)) {
    std::stringstream errstr;
    errstr << "Text element \""<< rLabel << "\" expected at " << FileLine();
    throw Exception("TokenReader::TokenReader", errstr.str(), 50);
  }
  Get(token);
  // do my text reading
  int ll=Token::ReadEscapedString(mpStream,'<',rText);
  if(ll<0) {
    std::stringstream errstr;
    errstr << "Text expected at " << FileLine();
    throw Exception("TokenReader::TokenReader", errstr.str(), 50);
  }
  mLineCount+=ll;
  // strip leading/trailing linefeeds 
  static const std::string line="\n\r\v";
  std::size_t pos1=rText.find_first_not_of(line);
  if(pos1!=std::string::npos)  
    rText=rText.substr(pos1);
  else
    rText.clear();
  std::size_t pos2=rText.find_last_not_of(line);
  if(pos2!=std::string::npos)  
    rText.erase(pos2+1);
  // strip leading/trailing white if all in one line
  static const std::string white=" \t";
  if(pos1==0) {
    pos1=rText.find_first_not_of(white);
    if(pos1!=std::string::npos)  
      rText=rText.substr(pos1);
    else
      rText.clear();
    std::size_t pos2=rText.find_last_not_of(white);
    if(pos2!=std::string::npos)  
       rText.erase(pos2+1);
  }
  // insist in my end tag
  Peek(token);
  if(!token.IsEnd(rLabel)) {
    std::stringstream errstr;
    errstr << "End of text element \""<< rLabel << "\" expected at " << FileLine();
    throw Exception("TokenReader::TokenReader", errstr.str(), 50);
  }
  Get(token);
}

// ReadVerbatim()
void TokenReader::ReadVerbatim(const std::string& rLabel, std::string& rString) {
  // insist in my tag
  Token token;
  Peek(token);
  if(!token.IsBegin(rLabel)) {
    std::stringstream errstr;
    errstr << "Verbatim element \""<< rLabel << "\" expected at " << FileLine();
    throw Exception("TokenReader::TokenReader", errstr.str(), 50);
  }
  Get(token);
  rString.clear();
  // loop cdata
  int cnt=0;
  rString.clear();
  while(Peek(token)) {
    if(!token.IsString()) break;
    if(cnt>0 && !token.IsCdata()) break;
    Get(token);
    rString.append(token.StringValue());
    cnt++;
  }
  // strip leading/trailing linefeeds 
  static const std::string line="\n\r\v";
  std::size_t pos1=rString.find_first_not_of(line);
  if(pos1!=std::string::npos)  
    rString=rString.substr(pos1);
  else
    rString.clear();
  std::size_t pos2=rString.find_last_not_of(line);
  if(pos2!=std::string::npos)  
    rString.erase(pos2+1);
  // insist in my end tag
  Peek(token);
  if(!token.IsEnd(rLabel)) {
    std::stringstream errstr;
    errstr << "End of verbatim element \""<< rLabel << "\" expected at " << FileLine();
    throw Exception("TokenReader::TokenReader", errstr.str(), 50);
  }
  Get(token);
}

// ReadCharacterData()
void TokenReader::ReadCharacterData(std::string& rData) {
  // if we have a markup token in the buffer there is no character data except white space
  if(mPeekToken.IsBegin() || mPeekToken.IsEnd()) {
    FD_DV("TokenReader::ReadCharacterData(): tag in buffer");
    rData=mPeekToken.PreceedingSpace();
    mPeekToken.PreceedingSpace("");
    return;
  }
  // do my own reading
  int ll=Token::ReadCharacterData(mpStream,rData,mFaudesComments);
  if(ll<0) {
    std::stringstream errstr;
    errstr << "Missformed character data at " << FileLine() << ": " << rData;
    throw Exception("TokenReader::TokenReader", errstr.str(), 50);
  }
  mLineCount+=ll;
  // prepend peek buffers string value (better: need rewind!)
  if(mPeekToken.IsString())
    rData=mPeekToken.StringValue() + " " + rData;
  // invalidate buffer
  mPeekToken.SetNone();
}

// ReadSection()
void TokenReader::ReadSection(std::string& rSectionString) {
  // record current level
  int clevel = Level();
  // setup token writer for destination // need a better interface here: provide string buffer
  TokenWriter tw(TokenWriter::String);
  tw.Endl(true);
  // token copy loop
  while(true) {
    // see whether we can grab and copy some character data
    std::string cdata;
    ReadCharacterData(cdata);
    tw.WriteCharacterData(cdata);
    // break end of my level
    Token token;
    if(!Peek(token)) break;
    if(token.IsEnd() && !token.IsBegin() && Level()==clevel) 
      break;
    // get and copy markup token 
    Get(token);
    token.PreceedingSpace("n"); // explicit no formating
    tw.Write(token);
  }
  // done
  rSectionString=tw.Str();
}


// Line()
int TokenReader::Line(void) const {
  return mLineCount;
}

// FileLine()
std::string TokenReader::FileLine(void) const {
  if(mFileName!="")
    return "("+ mFileName + ":" + ToStringInteger(mLineCount) +")";
  else
    return "(#" + ToStringInteger(mLineCount) +")";
}

} // namespace faudes
