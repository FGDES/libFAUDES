/** @file cfl_token.cpp @brief Class Token */

/* FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2006  Bernd Opitz
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


#include "cfl_token.h"

namespace faudes {


// Token::Token()
Token::Token(void) : 
  mType(None), 
  mStringValue(""), 
  mOptionValue(""), 
  mIntegerValue(0), 
  mFloatValue(0),
  mPreceedingSpace(""),
  mAttributeCount(0)
{ 
}

// copy construct
Token::Token(const Token& rToken) :
  mType(rToken.mType), 
  mStringValue(rToken.mStringValue), 
  mOptionValue(rToken.mOptionValue), 
  mIntegerValue(rToken.mIntegerValue), 
  mFloatValue(rToken.mFloatValue),
  mPreceedingSpace(rToken.mPreceedingSpace),
  mAttributes(rToken.mAttributes),
  mAttributeCount(rToken.mAttributeCount)
{ 
}


// assignment
Token& Token::operator=(const Token& rToken) {
  mType=rToken.mType; 
  mStringValue=rToken.mStringValue; 
  mOptionValue=rToken.mOptionValue; 
  mIntegerValue=rToken.mIntegerValue; 
  mFloatValue=rToken.mFloatValue;
  mPreceedingSpace=rToken.mPreceedingSpace;
  mAttributes=rToken.mAttributes;
  mAttributeCount=rToken.mAttributeCount;
  return *this;
}

// destructor
Token::~Token(void){
}

// Token::SetNone()
void Token::SetNone(void ) {
  mType=None; 
  mStringValue=""; 
  mOptionValue=""; 
  mIntegerValue=0; 
  mFloatValue=0;
  mAttributes.clear();
  mAttributeCount=0;
  mPreceedingSpace="";
}

// Token::SetString(str)
void Token::SetString(const std::string& rName) {
  SetNone();
  mType=String; 
  mStringValue=rName; 
}

// Token::SetBegin(str)
void Token::SetBegin(const std::string& rName) {
  SetNone();
  mType=Begin; 
  mStringValue=rName; 
}

// Token::SetEnd(str)
void Token::SetEnd(const std::string& rName) {
  SetNone();
  mType=End; 
  mStringValue=rName; 
}

// Token::SetEmpty(str)
void Token::SetEmpty(const std::string& rName) {
  SetNone();
  mType=End | Begin; 
  mStringValue=rName; 
}

// Token::SetOption(str)
void Token::SetOption(const std::string& rName) {
  SetNone();
  mType=Option | String; 
  mStringValue="+"+rName+"+"; 
  mOptionValue=rName; 
}

// Token::SetInteger(number)
void Token::SetInteger(const Int number) {
  SetNone();
  mType=Integer | Float; 
  mIntegerValue=number; 
  mFloatValue=number;
}

// Token::SetInteger16(number)
void Token::SetInteger16(const Int number) {
  SetNone();
  mType= Integer16 | Integer | Float; 
  mIntegerValue=number; 
  mFloatValue=number;
}

// Token::SetBoolean(number)
void Token::SetBoolean(const Int number) {
  SetNone();
  mType= Boolean | Integer | Float; 
  mIntegerValue=number; 
  mFloatValue=number;
}

// Token::SetFloat(number)
void Token::SetFloat(const faudes::Float number) {
  SetNone();
  mType=Float; 
  mIntegerValue=(Int) number; 
  mFloatValue=number;
}

// Token::SetBinary
void Token::SetBinary(const char* data, std::size_t len) {
  SetNone();
  mType=Binary | String;
  mStringValue.assign(data,len);
}


// Token::ClrEnd()
void Token::ClrEnd(void) {
  mType&= ~End;
}


// access integer
Int Token::IntegerValue(void) const {
  return(mIntegerValue);
}


// access float
faudes::Float Token::FloatValue(void) const {
  return(mFloatValue);
}

// access string
const std::string& Token::StringValue(void) const {
  if(mType & Option) return(mOptionValue); // compatibility
  return(mStringValue);
}

// access option
const std::string& Token::OptionValue(void) const {
  return(mOptionValue);
}

// access raw data
const std::string& Token::PreceedingSpace(void) const {
  return mPreceedingSpace;
}

// access raw data
void Token::PreceedingSpace(const std::string& sep)  {
  mPreceedingSpace=sep;
}

// access unique type (depreciated!)
Token::TokenType Token::Type(void) const {
  if(mType & Begin)  return Begin;
  if(mType & End)    return End;
  if(mType & Integer16) return Integer16;
  if(mType & Boolean) return Boolean;
  if(mType & Integer) return Integer;
  if(mType & Float) return Float;
  if(mType & Option) return Option;
  if(mType & Binary) return Binary;
  if(mType & String) return String;
  return None;
}


// test type
bool Token::IsNone(void) const {
  return mType == None;
}

// test type
bool Token::IsInteger(void) const {
  return mType & Integer;
}

// test type
bool Token::IsInteger16(void) const {
  return mType & Integer16;
}

// test type
bool Token::IsBoolean(void) const {
  return mType & Boolean;
}

// test type
bool Token::IsFloat(void) const {
  return mType & Float;
}

// test type
bool Token::IsOption(void) const {
  return mType & Option;
}

// test type
bool Token::IsString(void) const {
  return mType & String;
}

// test type
bool Token::IsBinary(void) const {
  return mType & Binary;
}

// test type
bool Token::IsCdata(void) const {
  return mType & Cdata;
}

// test type
bool Token::IsBegin(void) const {
  return mType & Begin;
}

// test type
bool Token::IsBegin(const std::string& tag) const {
  if(! (mType & Begin) ) return false;
  return mStringValue==tag;
}

// test type
bool Token::IsEnd(void) const {
  return mType & End;
}

// test type
bool Token::IsEnd(const std::string& tag) const {
  if(! (mType & End) ) return false;
  return mStringValue==tag;
}

// test type
bool Token::IsEmpty(void) const {
  return (mType & Begin) && (mType & End);
}

// clear attribute
void Token::ClrAttribute(const std::string& name) {
  aiterator ait=mAttributes.find(name);
  if(ait==mAttributes.end()) return;
  mAttributes.erase(ait);
}

// clear all attributes
void Token::ClearAttributes() {
  mAttributes.clear();
  mAttributeCount=0;
}


// insert attribute for interpretation
void Token::InsAttribute(const std::string& name, const std::string& value) {
  AttributeValue aval;
  aval.mStringValue=value;
  aval.mType=None;
  aval.mSort=mAttributeCount++;
  mAttributes[name]=aval;
}


// insert string attribute
void Token::InsAttributeString(const std::string& name, const std::string& value) {
  AttributeValue aval;
  aval.mStringValue=value;
  aval.mType=String;
  aval.mSort=mAttributeCount++;
  mAttributes[name]=aval;
}

// insert integer attribute
void Token::InsAttributeInteger(const std::string& name, Int value) {
  AttributeValue aval;
  aval.mStringValue=ToStringInteger(value);
  aval.mType=Float | Integer;
  aval.mSort=mAttributeCount++;
  mAttributes[name]=aval;
}

// insert integer attribute
void Token::InsAttributeInteger16(const std::string& name, Int value) {
  AttributeValue aval;
  aval.mStringValue=ToStringInteger16(value);
  aval.mType=Float | Integer;
  aval.mSort=mAttributeCount++;
  mAttributes[name]=aval;
}

// insert integer attribute
void Token::InsAttributeBoolean(const std::string& name, Int value) {
  AttributeValue aval;
  if(value==0) aval.mStringValue="false";
  else aval.mStringValue="true";
  aval.mType=Float | Integer;
  aval.mSort=mAttributeCount++;
  mAttributes[name]=aval;
}

// insert float attribute
void Token::InsAttributeFloat(const std::string& name, faudes::Float value) {
  AttributeValue aval;
  aval.mStringValue=ToStringFloat(value);
  aval.mType=Float;
  aval.mSort=mAttributeCount++;
  mAttributes[name]=aval;
}

// test attribute type
bool Token::ExistsAttributeString(const std::string& name) {
  aiterator ait=mAttributes.find(name);
  if(ait==mAttributes.end()) return false;
  InterpretAttribute(ait);
  if(ait->second.mType & String) return true;
  return false;
}


// test attribute type
bool Token::ExistsAttributeInteger(const std::string& name) {
  aiterator ait=mAttributes.find(name);
  if(ait==mAttributes.end()) return false;
  InterpretAttribute(ait);
  if(ait->second.mType & Integer) return true;
  return false;
}


// test attribute type
bool Token::ExistsAttributeFloat(const std::string& name) {
  aiterator ait=mAttributes.find(name);
  if(ait==mAttributes.end()) return false;
  InterpretAttribute(ait);
  if(ait->second.mType & Float) return true;
  return false;
}


// access attribute value
const std::string& Token::AttributeStringValue(const std::string& name) {
  static const  std::string emptystr="";
  aiterator ait=mAttributes.find(name);
  if(ait==mAttributes.end()) return emptystr;
  InterpretAttribute(ait);
  if(!(ait->second.mType & String)) return emptystr;
  return ait->second.mStringValue;
}


// access attribute value
Int Token::AttributeIntegerValue(const std::string& name) {
  aiterator ait=mAttributes.find(name);
  if(ait==mAttributes.end()) return 0;
  InterpretAttribute(ait);
  if(!(ait->second.mType & Integer)) return 0;
  return ait->second.mIntegerValue;
}


// access attribute value
faudes::Float Token::AttributeFloatValue(const std::string& name) {
  aiterator ait=mAttributes.find(name);
  if(ait==mAttributes.end()) return 0;
  InterpretAttribute(ait);
  if(!(ait->second.mType & Float)) return 0;
  return ait->second.mFloatValue;
}


// WriteVerbatim(pStream)
  void Token::WriteVerbatim(std::ostream* pStream, const std::string& rData, bool lfflag) { 
  // markup
  *pStream << "<![CDATA[";
  // optional preceeding linefeed
  if(lfflag) *pStream << std::endl;
  // split up cdata sections
  std::string esc="]]>";
  std::size_t pos=0;
  // loop segments
  while(pos < rData.size()) {
    std::size_t next= rData.find(esc,pos);
    if(next==std::string::npos) next=rData.size()+1;
    // write segment
    *pStream << rData.substr(pos,next-pos);
    // write split
    if(next<=rData.size())
      *pStream << "]]]]><![CDATA[>";
    // proceed
    pos=next+3;
  }
  // optional post linefeed
  if(lfflag) *pStream << std::endl;
  // markup
  *pStream << "]]>";
}



// WriteBinary(pStream,data,len)
  void Token::WriteBinary(std::ostream* pStream,  const char* pData, std::size_t len) {

  // my encoding (hardcoded in read, however)
  static char Base64EncodingTable[]=
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  // start
  *pStream << "=";

  // loop vars
  const char* src = pData;
  std::size_t cnt=len;
  int step=0;
  unsigned char c0=0,c1=0,c2=0,c3=0;

  // encode and output buffer
  while(cnt>0) {
    switch(step) {
    // collect char for 1st byte
    case 0:
      c0=  ((*src & 0xFC) >> 2);
      c1=  ((*src & 0x03) << 4);
      c2=0;
      c3=0;
      step=1;
      break;
    // collect char for 2nd byte
    case 1:
      c1|= ((*src & 0xF0) >> 4);
      c2|= ((*src & 0x0F) << 2);
      step=2;
      break;
    // collect char for 3rd byte, plus output
    case 2:
      c2|= ((*src & 0xC0) >> 6);
      c3|= (*src & 0x3F); 
      *pStream << Base64EncodingTable[c0] << Base64EncodingTable[c1] << 
        Base64EncodingTable[c2]  << Base64EncodingTable[c3]; 
      step=0;
      break;
    default: break;
    }
    cnt--;
    src++;
  }   
  // flush left overs, incl padding
  switch(step) {
   case 0:
     *pStream << "= ";
     break;
   case 1:
     *pStream << Base64EncodingTable[c0] << Base64EncodingTable[c1] << "=== ";
     break;
   case 2:
     *pStream << Base64EncodingTable[c0] << Base64EncodingTable[c1] << 
       Base64EncodingTable[c2]  << "== ";
     break;
  }
}

// WriteBinary(pStream)
void Token::WriteBinary(std::ostream* pStream) const {
  if(mType!=Binary) return;
  WriteBinary(pStream,mStringValue.data(),mStringValue.size());
}


// ReadBinary ... to mStringValue
int Token::ReadBinary(std::istream* pStream) {
  
  // line count
  int lc;

  // swallow leading '='  
  char c1=pStream->get();
  if(c1!='=') return -1;

  // read as string, excl. marks '='
  lc=ReadString(pStream,'=');

  // swallow one trailing '=' 
  c1=pStream->get();
  if(c1!='=') return -1;

  // take any extra trailing padding =
  while(!pStream->eof()) {
    if(pStream->peek()!='=') break;
    pStream->get();
    mStringValue.append("=");
  }

  // loop vars
  std::string::iterator src=mStringValue.begin();
  std::string::iterator dst=mStringValue.begin();
  std::size_t cnt=0;
  int c=0, d0=0, d1=0, d2=0;
  unsigned char cs[4];
  unsigned int step=0;

  // loop
  while(true) {
    
    // get data
    while(true) {
      // sense eof
      if(src==mStringValue.end()) { c = -1; break; }
      // get char
      c=*(src++);
      // decode
      if(c>='A' && c <='Z') { c-='A'; break; }
      if(c>='a' && c <='z') { c-='a'; c+= ('Z'-'A')+1; break; }
      if(c>='0' && c <='9') { c-='0'; c+= 2*('Z'-'A')+2; break; }
      if(c=='+') {c= 62; break; }
      if(c=='/') {c= 63; break; }
      if(c=='=') {c= 0xFF; break; };
    }
    // pass on eof
    if(c== -1) break;
    // record and continue
    cs[step++] = c;
    if(step<=3) continue;
    step=0;
    // sort bits
    d0= ((cs[0] << 2) & 0xFC) | ((cs[1] >> 4) & 0x03);
    d1= ((cs[1] << 4) & 0xF0) | ((cs[2] >> 2) & 0x0F);
    d2= ((cs[2] << 6) & 0xC0) | (cs[3] & 0x3F);
    // record result
    if(cs[0]!= 0xFF && cs[1]!=0xFF) {*(dst++)=d0; cnt++;}
    if(cs[1]!= 0xFF && cs[2]!=0xFF) {*(dst++)=d1; cnt++;}
    if(cs[2]!= 0xFF && cs[3]!=0xFF) {*(dst++)=d2; cnt++;}
    // sense end
    if(cs[3]==0xFF) break;
  }   

  // set data length (sets the length incl termination char??)
  mStringValue.resize(cnt);
 
  // return padding error or line count
  return step==0 ? lc : -1;
}



// ReadSpace(pStream)
int Token::ReadSpace(std::istream* pStream, bool fcomments){
  char c = '\0';
  int lc = 0;
  FD_DV("Token::ReadSpace()");
  // check the whole pStream
  while(*pStream) {
    // swallow white space
    while(*pStream) {
      // check eof
      if(pStream->eof()) return lc; 
      // look one character ahead
      c = pStream->peek();
      // count the next lines
      if(c == '\n') {
        ++lc;
        pStream->get();
        mPreceedingSpace.append(1,c);
	continue;
      }
      // swallow controls
      if(iscntrl(c)) {
        pStream->get();
        mPreceedingSpace.append(1,c);
	continue;
      }
      // swallow space
      if(isspace(c)) {
        pStream->get();
        mPreceedingSpace.append(1,c);
	continue;
      }
      // regard this non-white 
      break;
    }
    // if the next character starts a faudes comment
    if(!fcomments) break;
    if(c != '%') break;
    while(*pStream) {
      // check eof
      if(pStream->eof()) {return(lc);}; 
      // get the next character
      c = pStream->get();
      // count the next lines
      if (c == '\n') ++lc;
      // terminate with the next new line  character
      if (c == '\n') break;
      if (c == '\r') break;
    }
  }
  // termination if the next character is neither a space, a control, or a '%'.
  FD_DV("Token::ReadSpace(): lc " << lc << " c " << c);
  return lc; 
}

// InterpretAttribute(aval)
void Token::InterpretAttribute(aiterator ait) {
  if(ait->second.mType != None) return;
  InterpretNumber(
    ait->second.mStringValue, ait->second.mType, 
    ait->second.mIntegerValue, ait->second.mFloatValue);
  ait->second.mType |= String; 
}

// InterpretNumber(string)
bool Token::InterpretNumber(void) {
  return InterpretNumber(mStringValue,mType,mIntegerValue,mFloatValue);
}

// InterpretNumber(string, ...)
bool Token::InterpretNumber(const std::string& numstr, int& type, Int& ival, faudes::Float& fval) {
  char c, vc;
  faudes::Float fv=0;
  Int iv=0;
  int comma = -1;
  bool minus = false;
  int  base=10;
  bool ok=false;
  int cnt=0;
  type &= ~(Integer | Integer16 | Float);
  // test for special cases
  if(numstr=="inf" || numstr=="+inf") {
    ival = std::numeric_limits<Int>::max();
    fval = std::numeric_limits<faudes::Float>::max();
    type|= (Integer | Float);
    return true;
  }
  // test for special casess
  if(numstr=="-inf") {
    ival=std::numeric_limits<Int>::min()+1; 
    fval = -1* std::numeric_limits<faudes::Float>::max();
    type|= (Integer | Float);
    return true;
  }
  // test for special cases
  if(numstr=="true" || numstr=="True") {
    ival = 1;
    fval = ival;
    type|= (Integer | Boolean);
    return true;
  }
  // test for special cases
  if(numstr=="false" || numstr=="False") {
    ival = 0;
    fval = ival;
    type|= (Integer | Boolean);
    return true;
  }
  // iterate over string
  std::string::const_iterator cit=numstr.begin();
  for(;cit!=numstr.end(); cit++) {
    // check next charakter
    c = *cit;
    cnt++;
    // change base on x
    if(c=='x' && iv==0 && cnt==2 && comma<0 && !minus) 
      {base = 16; continue;}
    // change sign on -
    if(c=='-' && cnt==1) 
      {minus = true; continue;}
    // record comma
    if(c=='.' && comma<0 && base==10) 
      {comma = 0; continue;}
    // break if it is not a digit
    if(!isdigit(c) && base==10) break;
    if(!isxdigit(c) && base==16) break;
    // compute the value of c
    vc=0;
    if(c>='0' && c<= '9') vc = c-'0';
    else if(c>='a' && c<= 'f') vc = c-'a' + 10;
    else if(c>='A' && c<= 'F') vc = c-'A' + 10;
    // compute the corresponding number
    iv = base * iv + vc;
    fv = base * fv + vc;
    if(comma>=0) comma++;
    ok = true; 
  } 
  // detect error
  if(cit!=numstr.end()) ok=false;
  // fix sign
  if(minus) {
    iv=-iv;
    fv=-fv;
  }
  // fix decimal point
  for(;comma>0;comma--) {
    iv/=base;
    fv/=base;
  }
  // assign the numeric value and type in Token
  if(ok) {
    ival = iv;
    fval = fv;
    type |= Float;
    if(comma<=0 && !minus) type |= Integer;
    if(comma<=0 && !minus && base==16) type |= Integer16;
  }
  return ok;
}

// Write(pStream)
void Token::Write(std::ostream* pStream) const {
  FD_DV("Token::Write: mType=" << (int) mType 
	<< "\" mStringValue=\"" << mStringValue 
	<< "\" mIntegerValue=" <<mIntegerValue 
	<< "\" mFloatValue=" <<mFloatValue <<"\n");
  // numerics first
  if(mType & Integer16) {
    *pStream << ExpandString(ToStringInteger16(mIntegerValue), FD_NAMELEN) << " ";
  } else if(mType & Integer) { 
    *pStream << ExpandString(ToStringInteger(mIntegerValue), FD_NAMELEN) << " ";
  } else if(mType & Float) { 
    *pStream << ExpandString(ToStringFloat(mFloatValue), FD_NAMELEN) << " ";
  }
  // mark up: begin
  else if(mType & Begin) {
    *pStream << '<' << mStringValue;
    std::map<int,caiterator> sortnames;
    for(caiterator ait=mAttributes.begin(); ait!=mAttributes.end(); ait++) 
      sortnames[ait->second.mSort]=ait;
    std::map<int,caiterator>::iterator sit;
    for(sit=sortnames.begin(); sit!=sortnames.end(); sit++) {
      caiterator ait=sit->second;
      *pStream << " ";
      WriteEscapedString(pStream,ait->first);
      *pStream << "=\"";
      WriteEscapedString(pStream,ait->second.mStringValue);
      *pStream << "\"";
    }
    if(mType & End) *pStream << "/";
    *pStream << ">";
  } 
  // mark up:end
  else if(mType & End) {
    *pStream << "</" << mStringValue << ">";
  }
  // cdata markup
  else if(mType & Cdata) {
    WriteVerbatim(pStream,mStringValue);
  }
  // string
  else if(mType & Option) {
    WriteString(pStream,""); // '+' is incl. mStringValue
  } else if(mType & Binary) {
    WriteBinary(pStream);
  } else if(mType & String) {
    // figure delimiter
    bool quote=false;
    if(mStringValue.size()==0) quote=true;
    if(mStringValue.size()>0)
      if(!isalpha(mStringValue[0])) quote=true;
    static const std::string white=" \n\r\t\f";
    if(mStringValue.find_first_of(white)!=std::string::npos) 
      quote=true;
    if(quote)
      WriteString(pStream,"\"");
    else
      WriteString(pStream,"");
  } 
  // error (should we have an exception here?)
  else { /*  assert(0) */ };
}

// WriteEscapedString(pStream)
int Token::WriteEscapedString(std::ostream* pStream, const std::string& outstr) {
  // assemble escape character string
  std::string escstr="<>&\"";
  // no escape characters
  if(outstr.find_first_of(escstr)==std::string::npos) {
    *pStream << outstr;
    return outstr.size();
  }
  // do escape substitution
  int cc=0;
  std::string::const_iterator cit=outstr.begin(); 
  for(;cit!=outstr.end(); cit++) {
    if(*cit=='<') 
      { *pStream << "&lt;"; cc+=4; continue;}
    if(*cit=='>') 
      { *pStream << "&gt;"; cc+=4; continue;}
    if(*cit=='&') 
      { *pStream << "&amp;"; cc+=5; continue;}
    if(*cit=='"')  
      { *pStream << "&quot;"; cc+=6; continue;}
    *pStream << *cit; cc++;
  }
  return cc;
}

// WriteString(pStream, delim)
void Token::WriteString(std::ostream* pStream, const std::string& delim) const {
  int cc=0; 
  *pStream << delim;
  cc+=delim.size();
  cc+=WriteEscapedString(pStream,mStringValue);
  *pStream << delim << " ";
  cc+=delim.size()+1;
  while(cc< FD_NAMELEN) {
    *pStream << " "; cc++;
  }  
}

// ReadString(pStream, char)
int Token::ReadString(std::istream* pStream, char stop) {
  return ReadEscapedString(pStream,stop,mStringValue);
}


// ReadEscapedString(pStream, rString, char)
int Token::ReadEscapedString(std::istream* pStream, char stop, std::string& rString) {
  int lc=0;
  char c;
  std::string entref="";
  bool ctrlblank = false;
  rString = "";
  // check the whole pStream
  while (*pStream) {
    // check eof
    if(pStream->eof()) return -1;
    // test one character
    c = pStream->peek();
    // break on mark up
    if(c == '<') break;
    if(c == '>') break;
    // break on stop
    if(c == stop) break;
    if(isblank(c) && stop==' ') break;
    if(iscntrl(c) && stop==' ') break;
    // get one character
    c = pStream->get();
    // count the next lines
    if(c=='\n') ++lc;
    // replace sequence of control characters by one blank
    if(iscntrl(c) && ctrlblank) continue;
    ctrlblank=false;
    if(iscntrl(c)) { c=' '; ctrlblank=true;}
    // if in escape mode ...
    if(entref.size()!=0) {
      //record reference
      entref.append(1,c);
      // error: reference must not contain a white space
      if(c == ' ') return -1;
      // decode reference
      if(c == ';') {
        if(entref=="&amp;") rString.append(1,'&'); 
        else if(entref=="&quot;") rString.append(1,'"'); 
        else if(entref=="&apos;") rString.append(1,'\''); 
        else if(entref=="&lt;") rString.append(1,'<');
        else if(entref=="&gt;") rString.append(1,'>');
        // plain copy unknown
        else rString.append(entref);
        entref="";
      }
      continue;
    }
    // ... sense escape
    if(c == '&') entref.append(1,c);
    // ... add character
    if(c != '&') rString.append(1,c);
    continue;
  } 
  // report
  FD_DV("Token::ReadEscapedString(): lc=" << lc << " val=" << rString);
  // space seperated string must be nontrivial
  if(stop==' ' && rString.size()==0) return -1;
  return lc;
}


// ReadCharacterData(pStream, rString)
int Token::ReadCharacterData(std::istream* pStream, std::string& rString, bool fcomments) {
  rString = "";
  // special case
  if(pStream->eof()) return 0;
  // check the whole pStream
  int lc=0;
  char c;
  bool cm = false;
  while (*pStream) {
    // check other errors
    if(!pStream->good()) { rString="I/O error"; return -1; }
    // test one character
    c = pStream->peek();
    // break on eof
    if(pStream->eof()) break;
    // sense error: markup in faudes comment
    if(fcomments && cm)
      if((c=='<') || (c=='>')) { rString="'<' or '>' in faudes comment"; return -1; }
    // break on mark up
    if(c == '<') break;
    // again: test state (peek may set eof, so dont use good() here) 
    if(pStream->bad()) { rString="I/O error"; return -1; }
    // get one character
    c = pStream->get();
    // count the next lines
    if(c=='\n') ++lc;
    // track faudes comment mode
    if(c=='%') cm=true;
    if(c=='\n') cm=false;
    // ... add character
    if(!(fcomments && cm))
      rString.append(1,c);
  } 
  return lc;
}


// ReadAttributes(pStream)
// (and swallow all space after the last attribute)
int Token::ReadAttributes(std::istream* pStream) {
  int lc=0;
  char c=0;
  FD_DV("Token::ReadAttributes()");
  // scan until excl. '>'
  while (*pStream) {
    // skip space
    while (*pStream) {
      if(pStream->eof()) return -1; 
      c = pStream->peek();
      if(!(isblank(c) || iscntrl(c))) break;
      pStream->get();
      if(c=='\n') ++lc;
    }
    // get attrname
    std::string aname;
    while (*pStream) {
      if(pStream->eof()) return -1; 
      c = pStream->peek();
      if(isblank(c) || iscntrl(c)) break;
      if(c=='=') break;
      if(c=='>') break;
      if(c=='/') break;
      pStream->get();
      aname.append(1,c); 
    }
    FD_DV("Token::ReadAttributes(): aname " << aname);
    // no name so we're done
    if(aname.size()==0) {
      return lc;
    }
    // skip space
    while(*pStream) {
      if(pStream->eof()) return -1; 
      c = pStream->peek();
      if(!(isblank(c) || iscntrl(c))) break;
      pStream->get();
      if(c=='\n') ++lc;
    }
    // insist in eq
    if(c!='=') return -1;
    pStream->get();
    // skip space
    while (*pStream) {
      if(pStream->eof()) return -1; 
      c = pStream->peek();
      if(!(isblank(c) || iscntrl(c))) break;
      pStream->get();
      if(c=='\n') ++lc;
    }
    // strict version, value by '"'
    if(c == '"') {
      pStream->get();
      int ll=ReadString(pStream,'"');
      if(ll<0) return -1;
      pStream->get();
      lc+=ll;    
    } 
    // strict version, value by '''
    else if(c == '\'') {
      pStream->get();
      int ll=ReadString(pStream,'\'');
      if(ll<0) return -1;
      pStream->get();
      lc+=ll;    
    } 
    // relaxed version, value by "space"
    else {
      int ll=ReadString(pStream,' ');
      if(ll<0) return -1;
      lc+=ll;    
    }
    std::string aval=mStringValue;
    FD_DV("Token::ReadAttributes(): aval " << aval);
    // record attribute
    InsAttribute(aname,aval);
  }
  // done
  return lc;
}

// ReadMarkup(pStream)
// (leading "<" has allready been read)
int Token::ReadMarkup(std::istream* pStream) {
  int lc=0;
  int ll;
  char c=0;
  mStringValue = "";
  mType=None;
  // figure indicator character
  char p1=0;
  char p2=0;
  if(pStream->eof()) return -1; 
  c = pStream->peek();
  if(!(isalpha(c) || c=='_' || c==':')) {
    p1 = pStream->get();
    if(pStream->eof()) return -1; 
    p2 = pStream->peek();
  }
  FD_DV("Token::ReadMarkup: " <<  c << "-" << p1 << "-" << p2);
  // its a begin tag ... 
  if(p1==0) {
    FD_DV("Token::ReadMarkup: sensed XML tag");
    // ... get the name
    std::string name;
    while (*pStream) {
      if(pStream->eof()) return -1; 
      c = pStream->peek();
      if(c == '>') break;
      if(c == '/') break;
      if(isblank(c) || iscntrl(c)) break;
      pStream->get();
      name.append(1,c);
    }
    if(name.size()==0) return -1;
    mType = Begin;
    ll=ReadAttributes(pStream);
    if(ll<0) return -1;
    if(pStream->eof()) return -1; 
    c = pStream->peek();
    if(c=='/') {
      mType |= End;
      pStream->get();
    }
    mStringValue=name;
    FD_DV("Token::ReadMarkup: sensed XML tag, type " << mType << " name " << mStringValue);
  }
  // its an end tag: get the name
  if(p1=='/') {
    std::string name;
    while(*pStream) {
      if(pStream->eof()) return -1; 
      c = pStream->peek();
      if(c == '>') break;
      if(c == '/') break;
      if(isblank(c) || iscntrl(c)) break;
      pStream->get();
      name.append(1,c);
    }
    if(name.size()==0) return -1;
    if(c!='>') return -1;
    mType = End;
    mStringValue=name;
  }
  // its an xml comment
  if(p1=='!' && p2=='-') {
    FD_DV("Token::ReadMarkup: sensed XML comment <" << p1 << p2);
    c=pStream->get();
    if(pStream->eof()) return -1; 
    c=pStream->get();
    if(c!='-') return -1;
    char c2=0;
    char c3=0;
    while (*pStream) {
      c3=c2; c2=c;
      if(pStream->eof()) return -1; 
      c = pStream->peek();
      if(c3== '-' && c2=='-' && c == '>') break;
      pStream->get();
      if(c=='\n') ++lc;
    }
    FD_DV("Token::ReadMarkup: sensed XML comment end " << c3 << c2 << c);
  }
  // its an xml doctypedec (which we cannot handle properly)
  if(p1=='!' && (p2=='D' || p2=='d')) {
    FD_DV("Token::ReadMarkup doc.type.dec. not implemented (!)");
    c=pStream->get();
    while(*pStream) {
      if(pStream->eof()) return -1; 
      c = pStream->peek();
      if(c == '>') break;
      pStream->get();
      if(c=='\n') ++lc;
    }
  }
  // its an xml cdata (interpret as string)
  if(p1=='!' && p2=='[' ) {
    FD_DV("Token::ReadMarkup: sense CDATA?");
    // sense "<![CDATA["
    c=pStream->get();
    if(pStream->eof()) return -1; 
    if(pStream->get()!='C') return -1;
    if(pStream->eof()) return -1; 
    if(pStream->get()!='D') return -1;
    if(pStream->eof()) return -1; 
    if(pStream->get()!='A') return -1;
    if(pStream->eof()) return -1; 
    if(pStream->get()!='T') return -1;
    if(pStream->eof()) return -1; 
    if(pStream->get()!='A') return -1;
    if(pStream->eof()) return -1; 
    if(pStream->get()!='[') return -1;
    // read until "]]>"
    FD_DV("Token::ReadMarkup: sense CDATA!");
    char c2=0;
    char c3=0;
    while(*pStream) {
      c3=c2; c2=c;
      if(pStream->eof()) return -1; 
      c = pStream->peek();
      if(c3== ']' && c2==']' && c == '>') break;
      if(pStream->eof()) return -1; 
      pStream->get();
      if(c=='\n') ++lc;
      mStringValue.append(1,c);
    }
    FD_DV("Token::ReadMarkup: sense CDATA:" << mStringValue);
    // drop "]]"
    if(mStringValue.size()>=2)
      mStringValue.erase(mStringValue.size()-2);
    mType |= String;    
    mType |= Cdata;    
    FD_DV("Token::ReadMarkup: sense CDATA:" << mStringValue);
  }
  // its an xml proc.intstruction (which we ignore)
  if(p1=='?') {
    if(pStream->eof()) return -1; 
    c = pStream->get();
    char c2=0;
    while (*pStream) {
      c2=c;
      if(pStream->eof()) return -1; 
      c = pStream->peek();
      if(c2=='?' && c == '>') break;
      pStream->get();
      if(c=='\n') ++lc;
    }
  }
  // error
  if(pStream->eof()) return -1; 
  c = pStream->peek();
  if(c!='>') {
    FD_DV("Token::ReadMarkup: mismatch (?) " << mStringValue);
    while (*pStream) {
      if(pStream->eof()) return -1; 
      c = pStream->peek();
      if(c == '>') break;
      if(c=='\n') ++lc;
      pStream->get();
    }
  }
  // done
  pStream->get();
  FD_DV("Token::ReadMarkup: return type " << mType << " string " << mStringValue << " lc" << lc);
  return lc;
}

// Read(pStream)
int Token::Read(std::istream* pStream, bool fcomments){
  FD_DV("Token::Read(): fcomments=" << fcomments);
  char c1;
  int lc = 0;
  int ll = -1;
  // the token is initialized with the type "None"
  SetNone();
  // check eof
  if(pStream->eof()) return(lc); 
  // read all white space 
  lc += ReadSpace(pStream,fcomments);
  // check eof
  if(pStream->eof()) return(lc);
  // get the first useful character
  c1=pStream->peek();
  // token is a quoted string if it starts with '"'
  if(c1 == '"') {
    pStream->get();
    // read the string until '"'
    ll=ReadString(pStream,'"');
    if(ll>=0) {
      lc+=ll;
      mType |= String;
      pStream->get();
    }
  } 
  // token is a quoted string if it starts with '''
  else if(c1 == '\'') {
    pStream->get();
    // read the string until '''
    ll=ReadString(pStream,'\'');
    if(ll>=0) {
      lc+=ll;
      mType |= String;
      pStream->get();
    }
  } 
  // token is an option string if it starts with '+'
  else if(c1 == '+') {
    pStream->get();
    // read the string until '+'
    ll=ReadString(pStream,'+');
    mOptionValue=mStringValue;
    mStringValue="+" + mOptionValue + "+";
    if(ll>=0) {
      lc+=ll;
      mType |= (Option | String);
      pStream->get();
    }
  } 
  // token is a binary string if it starts with '='
  else if(c1 == '=') {
    // read the string until '=', incl padding
    ll=ReadBinary(pStream);
    if(ll>=0) {
      lc+=ll;
      mType |= (Binary | String);
    }
  } 
  // token is markup  if it starts with <
  else if(c1 == '<') {
    pStream->get();
    // check eof
    if(pStream->eof()) return(lc); 
    // read and interpret
    ll=ReadMarkup(pStream);
    // recurse on non-faudes-recognised but parsable markup (effectively swallowing unrecognised)
    if(ll>=0) {
      lc+=ll;
      if(mType==None) return(Read(pStream));
    }
  } 
  // token is a space seperated string, perhaps a number
  else if(c1 != '%') {
    ll=ReadString(pStream,' ');
    if(ll>=0) {
      mType |= String;
      InterpretNumber();
    }
  } 
  // sense error
  if(ll<0) {
    FD_DV("Token::Read(): failed with '" << c1 <<"'");
    return -1;
  }
  FD_DV("Token::Read(): " << Str());
  return(lc);
}


// Str()
std::string Token::Str(void) const {
  std::stringstream ostr;
  ostr << "Token(--- Type=";
  if(IsNone()) ostr << "None"; 
  if(IsInteger()) ostr << "Integer";
  if(IsInteger16()) ostr << "Integer16";
  if(IsBoolean()) ostr << "Boolean";
  if(IsFloat()) ostr << "Float";
  if(IsString()) ostr << "String";
  if(IsEmpty()) ostr << "Begin/End";
  if(IsBegin()) ostr << "Begin";
  if(IsEnd()) ostr << "End";
  if(!IsNone()) {
    ostr << " Value=\"";
    if(IsFloat()) ostr << FloatValue();
    else if(IsBegin() || IsEnd()) ostr << StringValue();
    else if(IsString()) ostr << StringValue();
    ostr << "\"";
  }
  ostr << " sp #" << mPreceedingSpace.size();
  caiterator ait;
  for(ait=mAttributes.begin(); ait!=mAttributes.end(); ait++)
    ostr << "  attr[" << ait->first << "=\"" << ait->second.mStringValue << "\"]";
  ostr << " ---)";
  return ostr.str();
}



} // namespace faudes
