/** ref2html.cpp  Utility to generate a html documents from fref files */

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
	 

/*

Note: in its current state, this utility is badly organized.

Issues include
- preformance (linear searchs)
- normalised section labels etc
- global configuration data 
- embedded HTML fragments
... however, it does its job

*/



#include <string>
#include <cctype>
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
    std::cerr << rMessage << std::endl;
    std::cout << "" << std::endl;
  }
  std::cerr << "ref2html: " << VersionString()  << std::endl;
  std::cout << "" << std::endl;
  std::cerr << "utility to generate HTML from libFAUDES reference pages (*.fref)" << std::endl;
  std::cerr << std::endl << "usage: " << std::endl;
  std::cerr << " ref2html [options...]  <fref-file> <html-file>" << std::endl;
  std::cerr << " ref2html [options...]  <fref-file 1> [...] <fref-file n> <html-dir>" << std::endl;
  std::cerr << " ref2html [options...]  <fref-dir> <html-dir>" << std::endl;
  std::cerr << "with options as follows:" << std::endl;
  std::cerr << " -rti  <rti-file>   use specified run-time-interface definition" << std::endl;
  std::cerr << " -flx  <flx-file>   use specified lua-extension file" << std::endl;
  std::cerr << " -css  <css-file>   use specified style sheet" << std::endl;
  std::cerr << " -cnav <fref-file>  use specified chapter navigation file" << std::endl;
  std::cerr << " -chapter <label>   overwrite chapter label" << std::endl;
  std::cerr << " -section <label>   overwrite section label" << std::endl;
  std::cerr << " -rel <prefix>      prefix to chapter documentation base" << std::endl;
  std::cerr << " -inc <fref-file>   include table of contents" << std::endl;
  std::cerr << std::endl << std::endl;
  std::cerr << " ref2html -toc <fref-files> <output-file>" << std::endl;
  std::cerr << "to generate table of contents file" << std::endl;
  std::cerr << std::endl << std::endl;
  std::cerr << " ref2html -doxheader <output-file>" << std::endl;
  std::cerr << " ref2html -doxfooter <output-file>" << std::endl;
  std::cerr << "to generate header/footer for c++ api documentation" << std::endl;
  std::cerr << std::endl << std::endl;
  std::cerr << " ref2html -extract <input-file> <output-directory>" << std::endl;
  std::cerr << "to extract multiple reference pages from one file" << std::endl;
  std::cerr << std::endl;
  std::cerr << std::endl << "note: use \"-\" as output file for console output" << std::endl;
  exit(1);
}


// ******************************************************************
// configuration
// ******************************************************************

bool mStandaloneReference = false;

std::string mFrefTitle="";
std::string mFrefChapter="";
std::string mFrefSection="";
std::string mFrefPage="";
std::string mFrefLink="";
std::string mFrefSummary="";

std::string mRtiFile="";
std::string mFlxFile="";
std::string mDstFile="";
std::set< std::string > mSrcFiles;
std::string mChapterFile="";
std::string mIncludeFile="";

std::string mBooksPrefix="../";
std::string mChaptersPrefix="./";
std::string mImagePrefix="./images/";
std::string mReferencePrefix="./reference/";
std::string mCsourcePrefix="./csource/";
std::string mLuafaudesPrefix="./luafaudes/";

/*
std::string mDownloadLink="http://www.rt.eei.uni-erlangen.de/FGdes/download.html";
std::string mFaudesLink="http://www.rt.eei.uni-erlangen.de/FGdes/faudes";
std::string mDestoolLink="http://www.rt.eei.uni-erlangen.de/FGdes/destool";
std::string mLuafaudesLink="http://www.rt.eei.uni-erlangen.de/FGdes/faudes/luafaudes/";
std::string mCsourceLink="http://www.rt.eei.uni-erlangen.de/FGdes/faudes/csource/";
std::string mCssFile="faudes.css";
*/

std::string mDownloadLink="http://www.rt.techfak.fau.de/FGdes/download.html";
std::string mFaudesLink="http://www.rt.techfak.fau.de/FGdes/faudes";
std::string mDestoolLink="http://www.rt.techfak.fau.de/FGdes/destool";
std::string mLuafaudesLink="http://www.rt.techfak.fau.de/FGdes/faudes/luafaudes/";
std::string mCsourceLink="http://www.rt.techfak.fau.de/FGdes/faudes/csource/";
std::string mCssFile="faudes.css";

std::string mThisChapterClass="chapter_this";
std::string mOtherChapterClass="chapter_other";
std::string mExitChapterClass="chapter_exit";

// ******************************************************************
// configure: set my chapter prefix
// ******************************************************************

void ChaptersPrefix(const std::string& prefix) {
  mChaptersPrefix=prefix;
  mBooksPrefix=prefix+"../";
  mImagePrefix=prefix+"images/";
  mReferencePrefix=prefix+"reference/";
  mCsourcePrefix=prefix+"csource/";
  mLuafaudesPrefix=prefix+"luafaudes/";
  mCssFile=prefix+mCssFile;
}

// ******************************************************************
// helper: time stamp
// ******************************************************************

std::string TimeStamp(void) {
  time_t now;
  struct tm * local;
  char buffer[80];
  time(&now );
  local=localtime(&now);
  strftime(buffer,80,"%Y.%m.%d",local);
  return std::string(buffer);
}


// ******************************************************************
// helper: convert page to printable
// ******************************************************************

std::string PrettyPage(const std::string page){
  // swallow page number
  std::string ppage = page;
  std::size_t upos = ppage.find_first_of("_");
  std::size_t spos = ppage.find_first_of(" ");
  std::size_t dpos = 0;
  for(; dpos < ppage.size();dpos++) 
    if(!isdigit(ppage.at(dpos))) break;
  if(upos!=std::string::npos)
    if(upos==dpos)
      if(upos+1<ppage.size()) 
        ppage=ppage.substr(upos+1,ppage.size()-upos-1);
  if(spos!=std::string::npos)
    if(spos==dpos)
      if(spos+1<ppage.size()) 
        ppage=ppage.substr(spos+1,ppage.size()-spos-1);
  // turn underscore to space
  ppage=StringSubstitute(ppage,"_"," ");
  // done
  return ppage;
}
   

// ******************************************************************
// helper: bottom line
// ******************************************************************

void BottomLineHtml(std::ostream* pStream) {
  *pStream << "<p class=\"bottom_line\"> " << std::endl;
  *pStream << "<a href=\"" << mFaudesLink << "\" target=\"_top\">" << VersionString() << "</a> " << std::endl;
  *pStream << "--- " << TimeStamp() <<  "  " << std::endl;
  if(PluginsString()!="" && mFrefChapter!="cppapi")
     *pStream << "--- with &quot;" << PluginsString() << "&quot; " << std::endl;
  if(mFrefChapter=="cppapi")
     *pStream << "--- c++ api documentaion by <a href=\"http://www.doxygen.org\" target=\"_top\">doxygen</a>" << std::endl;
  *pStream << "</p>" << std::endl;
  if(mFrefChapter=="reference") {
    *pStream << "<!--[if IE]>" << std::endl;
    *pStream << "<p class=\"bottom_line_warning\">" << std::endl;
    *pStream << "If MS Internet Explorer fails to display certain mathematical symbols," << std::endl;
    *pStream << "your system misses the corresponding unicode fonts." << std::endl; 
    *pStream << "<br>" << std::endl;
    *pStream << "You may either install &quot;Arial Unicode MS&quot; from a recent MS Office package" << std::endl;
    *pStream << "or the freely available" << std::endl; 
    *pStream << "&quot;<a href=\"http://greekfonts.teilar.gr/\">Symbola</a>&quot;" << std::endl;
    *pStream << "<br>" << std::endl;
    *pStream << "See also <a href=\"http://www.alanwood.net/\">Allan Woods</a> unicode page." << std::endl;
    *pStream << "B.t.w.: <a href=\"http://www.mozilla.com\">Firefox</a> will display" << std::endl;
    *pStream << "all relevant symbols out-of-the-box and nicely render SVG diagrams." << std::endl;
    *pStream << "<br>" << std::endl;
    *pStream << "<br>" << std::endl;
    *pStream << "</p>" << std::endl;
    *pStream << "<![endif]-->" << std::endl;
  }
}


// ******************************************************************
// helper: html header
// ******************************************************************

void HeaderHtml(std::ostream* pStream) {
  *pStream << "<!doctype html>" << std::endl;
  *pStream << "<html xmlns=\"http://www.w3.org/1999/xhtml\">" << std::endl; 
  *pStream << "<head>" << std::endl;
  *pStream << "<title>" << mFrefTitle << "</title>" << std::endl; 
  *pStream << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />" << std::endl;
  *pStream << "<meta name=\"contents\" content=\"discrete event systems, libFAUDES, supervisory control, controller synthesis, automata, software library, regular languages, open source, GPL.\"/>"  << std::endl;
  *pStream << "<link href=\"" << mCssFile << "\" rel=\"stylesheet\" type=\"text/css\" />" << std::endl;
  *pStream << "<link rel=\"shortcut icon\" href=\""<< mImagePrefix << "des.ico\">" << std::endl;
  *pStream << "</head>" << std::endl;
  *pStream << "<body>" << std::endl;
  *pStream << "<div id=\"cwrapper1000\">" << std::endl;
  *pStream << "<div id=\"dwrapper1000\">"  << std::endl;
}

// ******************************************************************
// helper: html footer
// ******************************************************************

void FooterHtml(std::ostream* pStream) {
  *pStream << "</div>" << std::endl;
  *pStream << "</div>" << std::endl;
  *pStream << "</body>" << std::endl;
  *pStream << "</html>" << std::endl;
}

// ******************************************************************
// helper: html for image
// ******************************************************************

void ImageHtml(std::ostream* pStream, const std::string& rFileName) {
  *pStream << "<a class=\"faudes_image\" href=\"" << mImagePrefix << rFileName << ".html\">";
  *pStream << "<img src=\"" << mImagePrefix << rFileName << ".png\"/>";
  *pStream << "</a>"; 
}

// ******************************************************************
// helper: fancy reference for resgistry list
// ******************************************************************


void ListItemHtml(std::ostream* pStream, const std::string& rLink, const std::string& rText) {
   *pStream << "<li class=\"registry_item\">" << std::endl
      << "<a href=\"" << rLink << "\">" << rText << "</a>"  << std::endl
      << "<a href=\"" << rLink << "\" class=\"registry_blinda\">&nbsp;</a>"  << std::endl 
      << "</li>" << std::endl;
  }
 
// ******************************************************************
// helper: html for linked type name
// ******************************************************************

void TypeHtml(std::ostream* pStream, const std::string& rTypeName) {

  // just text if unknown
  if(!TypeRegistry::G()->Exists(rTypeName)) {
    *pStream << rTypeName;
    return;
  }

  // retrieve definition
  const TypeDefinition& tdef=TypeRegistry::G()->Definition(rTypeName);
  std::string tyname = tdef.Name();
  std::string tyhtml = tdef.HtmlDoc();
  if(tyhtml=="" || tyhtml=="none") {
    *pStream << tyname;
  } else {
    *pStream << "<a href=\"" << mReferencePrefix << tyhtml << "\">" << tyname << "</a>";
  }
}


// ******************************************************************
// helper: html for linked function name
// ******************************************************************

void FunctionHtml(std::ostream* pStream, const std::string& rFunctionName) {

  // just text if unknown
  if(!FunctionRegistry::G()->Exists(rFunctionName)) {
    *pStream << rFunctionName;
    return;
  }

  // retrieve definition
  const FunctionDefinition& fdef=FunctionRegistry::G()->Definition(rFunctionName);
  std::string fname = fdef.Name();
  std::string fhtml = fdef.HtmlDoc();
  if(fhtml=="" || fhtml=="none") {
    *pStream << fname;
  } else {
    *pStream << "<a href=\"" << mReferencePrefix << fhtml << "\">" << fname << "</a>";
  }
}

// ******************************************************************
// helper: html for ascii text
// ******************************************************************

void TextHtml(std::ostream* pStream, const std::string& rText) {
  std::string buff;
  buff=StringSubstitute(buff,"<","&lt;");
  buff=StringSubstitute(buff,">","&gt;");
  buff=StringSubstitute(buff,"&","&amp;");
  *pStream << buff;
}

// ******************************************************************
// helper: html for math
// (we really need at least regex here!)
// ******************************************************************

// resolve simple one-arg macros
std::string TexMacroSubstitute1(const std::string& rTexString, const std::string& rMacro, const std::string& rSubst) {
  // prep result
  std::string res;
  std::size_t pos=0; 
  // loop over occurences of "macro"
  while(pos<rTexString.length()) {
    std::size_t next=rTexString.find(rMacro,pos);
    if(next==std::string::npos) break;
    res.append(rTexString.substr(pos,next-pos));
    std::string arg;
    pos=next+rMacro.length();
    if(!(pos+1<rTexString.length())) continue;
    if(rTexString.at(pos)!='{') continue;
    std::size_t aend=rTexString.find('}',pos);
    if(aend==std::string::npos) break;
    arg=rTexString.substr(pos+1,aend-pos-1);
    arg=StringSubstitute(rSubst,"#1",arg);
    res.append(arg);
    pos=aend+1;
  }
  // get end
  if(pos<rTexString.length()) 
    res.append(rTexString.substr(pos));
  // done
  return res;
}
   

// mimique tex spacing
std::string TexSpacing(const std::string& rTexString) {
  // prep result
  std::string res;
  std::size_t pos=0; 
  bool math=true;
  // traverse string 
  while(pos<rTexString.length()) {
    // explict: "\ "
    if(pos+1 < rTexString.length())
    if(rTexString.substr(pos,2)=="\\ ")
      {pos+=2; res.append("&nbsp;"); continue;}
    // explicit: "\," 
    if(pos+1 < rTexString.length())
    if(rTexString.substr(pos,2)=="\\,")
      {pos+=2; res.append("&ensp;"); continue;}
    // explicit: "\;" 
    if(pos+1 < rTexString.length())
    if(rTexString.substr(pos,2)=="\\;")
      {pos+=2; res.append("&ensp;"); continue;}
    // explict: "\quad"
    if(pos+4 < rTexString.length())
    if(rTexString.substr(pos,5)=="\\quad")
      {pos+=5; res.append("&nbsp;&nbsp;&nbsp;&nbsp;"); continue;}
    // math swallow space
    if(math)
    if(isspace(rTexString.at(pos)))
      { pos+=1; continue;}
    // sense end of math mode
    if(math)
    if(pos+5 < rTexString.length())
    if(rTexString.substr(pos,6)=="\\text{")
      {pos+=6; math=false; continue;};
    // sense end of text mode
    if(!math)
    if(rTexString.at(pos)=='}')
      { pos+=1; math=true; continue;}
    // default: copy
    res.append(1,rTexString.at(pos));
    pos+=1;
    }
  return res;
}

// mimique tex scripts
std::string TexScripts(const std::string& rTexString) {
  // prep result
  std::string res;
  std::size_t pos=0; 
  int c0=-1;
  int cm1=-1;
  int cm2=-1;
  // traverse string 
  while(pos<rTexString.length()) {
    // fill buffer
    cm2=cm1; cm1=c0; c0=rTexString.at(pos);
    // full script
    if(cm1=='^' || cm1=='_')
    if(c0=='{') {
      std::size_t aend=rTexString.find('}',pos);
      if(aend==std::string::npos) break;
      std::string script=rTexString.substr(pos+1,aend-pos-1);
      res.append(1,cm1);
      res.append(script);
      cm1=-1; cm2=-1; pos=aend+1;
      continue;
    }
    // superscript uparrow
    if(cm1=='^')
    if(c0=='\\') {
      size_t mpos=rTexString.find("\\uparrow",pos);
      if(mpos==pos) {
        res.append("<span class=\"faudes_sup\">&uarr;</span>");
        cm1=-1; cm2=-1; pos+=8;
        continue;
      }
    }
    // superscript uparrow
    if(cm1=='^')
    if(c0=='\\') {
      size_t mpos=rTexString.find("\\Uparrow",pos);
      if(mpos==pos) {
        res.append("<span class=\"faudes_sup\">&#x21e7;</span>");
        cm1=-1; cm2=-1; pos+=8;
        continue;
      }
    }
    // digit subscript
    if(cm1=='_')
    if(isalpha(cm2)) 
    if(isdigit(c0)) {
      res.append(1,c0);
      cm1=-1; cm2=-1; pos+=1;
      continue;
    }
    // lower subscript
    if(cm1=='_')
    if(islower(c0)) 
    if(isupper(cm2)) {
      res.append(1,c0);
      cm1=-1; cm2=-1; pos+=1;
      continue;
    }
    // super star
    if(cm1=='^')
    if(c0=='*' || c0=='+') { 
      res.append(1,c0);
      cm1=-1; cm2=-1; pos+=1;
      continue;
    }
    // other script
    if(cm1=='^' || cm1=='_') {
      res.append(1,cm1);
      res.append(1,c0);
      cm1=-1; cm2=-1; pos+=1;
      continue;
    }     
    // plain copy default
    if(c0!='_')
    if(c0!='^') {
      res.append(1,c0);
    }
    // continue
    pos+=1;
  }
  return res;
}

  
// over all tex processing
void MathHtml(std::ostream* pStream, const std::string& rMathString) {
  std::string buff=rMathString;
  // xml quote
  buff=StringSubstitute(buff,"&","&amp;");
  // tex quote
  buff=StringSubstitute(buff,"#","&hash;");
  // greek letters 
  buff=StringSubstitute(buff,"\\Sigma","Sigma");
  buff=StringSubstitute(buff,"\\sigma","o");
  buff=StringSubstitute(buff,"\\delta","delta");
  buff=StringSubstitute(buff,"\\epsilon","epsilon");
  buff=StringSubstitute(buff,"\\omega","w");
  // one-arg macros
  buff=TexMacroSubstitute1(buff,"\\ProInv","Pinv#1");
  buff=TexMacroSubstitute1(buff,"\\Pro","P#1");
  buff=TexMacroSubstitute1(buff,"\\Closure","Closure(#1)");
  buff=TexMacroSubstitute1(buff,"\\Prefix","Prefix(#1)");
  // tex math spacing and plain text
  buff=TexMacroSubstitute1(buff,"\\texttt","\\text{<tt>#1</tt>}");
  buff=TexMacroSubstitute1(buff,"\\mathtt","\\text{<tt>#1</tt>}");
  buff=TexMacroSubstitute1(buff,"\\textit","\\text{<i>#1</i>}");
  buff=TexMacroSubstitute1(buff,"\\mathit","\\text{<i>#1</i>}");
  buff=TexMacroSubstitute1(buff,"\\mathsf","\\text{<sf>#1</i>}");
  buff=TexSpacing(buff);
  // super- and subscripts
  buff=TexScripts(buff);
  // symbols
  buff=StringSubstitute(buff,"\\{","{");
  buff=StringSubstitute(buff,"\\}","}");
  buff=StringSubstitute(buff,"\\[","[");
  buff=StringSubstitute(buff,"\\]","]");
  buff=StringSubstitute(buff,"=","&nbsp;=&nbsp;");
  buff=StringSubstitute(buff,"class&nbsp;=&nbsp;","class=");     // fix csss class
  buff=StringSubstitute(buff,":&nbsp;=&nbsp;","&nbsp;:=&nbsp;"); //fix :=
  buff=StringSubstitute(buff,"\\neq","&nbsp&ne;&nbsp;");
  buff=StringSubstitute(buff,"\\lt","&nbsp;&lt;&nbsp;");
  buff=StringSubstitute(buff,"\\gt","&nbsp;&gt;&nbsp;");
  buff=StringSubstitute(buff,"\\le","&nbsp;&le;&nbsp;");
  buff=StringSubstitute(buff,"\\ge","&nbsp;&ge;&nbsp;");
  buff=StringSubstitute(buff,"\\emptyset","0");
  buff=StringSubstitute(buff,"\\times","&nbsp;x&nbsp;");
  buff=StringSubstitute(buff,"\\ldots","...");
  buff=StringSubstitute(buff,"\\cdots","...");
  buff=StringSubstitute(buff,"\\cdot",".");
  buff=StringSubstitute(buff,"\\infty","&infin;");
  buff=StringSubstitute(buff,"\\nin","&nbsp;&notin;&nbsp;");
  buff=StringSubstitute(buff,"\\not\\in","&nbsp;&notin;&nbsp;");
  buff=StringSubstitute(buff,"\\in","&nbsp;&isin;&nbsp;");
  buff=StringSubstitute(buff,"\\subseteq","&nbsp;&sube;&nbsp;");
  buff=StringSubstitute(buff,"\\subset","&nbsp;&sub;&nbsp;");
  buff=StringSubstitute(buff,"\\supseteq","&nbsp;&supe;&nbsp;");
  buff=StringSubstitute(buff,"\\supset","&nbsp;&sup;&nbsp;");
  buff=StringSubstitute(buff,"\\cup","&cup;");
  buff=StringSubstitute(buff,"\\dcup","&cup;"); // should be "&cup;&#775;" for "dot above"
  buff=StringSubstitute(buff,"\\cap","&cap;");
  buff=StringSubstitute(buff,"\\sup","sup");
  buff=StringSubstitute(buff,"\\inf","inf");
  buff=StringSubstitute(buff,"\\max","max");
  buff=StringSubstitute(buff,"\\min","min");
  buff=StringSubstitute(buff,"\\parallel","||");
  buff=StringSubstitute(buff,"\\forall","&forall;&nbsp;");
  buff=StringSubstitute(buff,"\\exists","&exist;&nbsp;");
  buff=StringSubstitute(buff,"\\leftarrow","&larr;");
  buff=StringSubstitute(buff,"\\rightarrow","&rarr;");
  buff=StringSubstitute(buff,"\\leftrightarrow","&harr;");
  buff=StringSubstitute(buff,"\\Leftarrow","&lArr;");
  buff=StringSubstitute(buff,"\\Rightarrow","&rArr;");
  buff=StringSubstitute(buff,"\\Leftrightarrow","&hArr;");
  buff=StringSubstitute(buff,"\\uparrow","&uarr;");
  buff=StringSubstitute(buff,"\\downarrow","&darr;");
  buff=StringSubstitute(buff,"\\Uparrow","&#x21e7;");
  buff=StringSubstitute(buff,"\\Downarrow","&#x21e9;");
  // ie7 fallback symbols
  buff=StringSubstitute(buff,"&isin;","<span class=\"faudes_fmath\">&isin;</span>"); 
  buff=StringSubstitute(buff,"&notin;","<span class=\"faudes_fmath\">&notin;</span>"); 
  buff=StringSubstitute(buff,"&exist;","<span class=\"faudes_fmath\">&exist;</span>"); 
  buff=StringSubstitute(buff,"&forall;","<span class=\"faudes_fmath\">&forall;</span>"); 
  buff=StringSubstitute(buff,"&cup;","<span class=\"faudes_fmath\">&cup;</span>"); 
  buff=StringSubstitute(buff,"&dcup;","<span class=\"faudes_fmath\">&cup;</span>"); // see above
  buff=StringSubstitute(buff,"&cap;","<span class=\"faudes_fmath\">&cap;</span>"); 
  buff=StringSubstitute(buff,"&larr;","<span class=\"faudes_fmath\">&larr;</span>"); 
  buff=StringSubstitute(buff,"&rarr;","<span class=\"faudes_fmath\">&rarr;</span>"); 
  buff=StringSubstitute(buff,"&harr;","<span class=\"faudes_fmath\">&harr;</span>"); 
  buff=StringSubstitute(buff,"&lArr;","<span class=\"faudes_fmath\">&lArr;</span>"); 
  buff=StringSubstitute(buff,"&rArr;","<span class=\"faudes_fmath\">&rArr;</span>"); 
  buff=StringSubstitute(buff,"&hArr;","<span class=\"faudes_fmath\">&hArr;</span>"); 
  buff=StringSubstitute(buff,"&sub;","<span class=\"faudes_fmath\">&sub;</span>"); 
  buff=StringSubstitute(buff,"&sube;","<span class=\"faudes_fmath\">&sube;</span>"); 
  buff=StringSubstitute(buff,"&sup;","<span class=\"faudes_fmath\">&sup;</span>"); 
  buff=StringSubstitute(buff,"&supe;","<span class=\"faudes_fmath\">&supe;</span>"); 
  // done
  *pStream << buff;
}


// ******************************************************************
// record pages
// ******************************************************************


// section lists (from reading rti/flx)
std::set< std::string > mExclLuaSections; 
std::set< std::string > mInclLuaSections; 

// page data
class PageRecord {
public:
  std::string mTitle;
  std::string mChapter;
  std::string mSection;
  std::string mPage;
  std::string mLink;
  std::string mSummary;
};


// record all pages
std::vector<PageRecord> mAllPages;

// record all pages within reference section (keys to lower)
std::map< std::string , std::map< std::string , PageRecord > > mRefSectPages;

// extract page data (works with both, *.flx and *.ftoc)
void RecordPages(TokenReader& rTr) {
  // record my level
  int clevel = rTr.Level();
  // std::cerr << "process level " << clevel << "\n";
  // token loop
  while(true) {
    // skip plain text
    std::string text;
    rTr.ReadCharacterData(text);
    if(text.size()>0) continue;
    // break on no token or end of my level
    Token token;
    if(!rTr.Peek(token)) break;
    if(token.IsEnd() && !token.IsBegin() && rTr.Level()==clevel) 
      break;
    // ignore irrelevant
    if(!token.IsBegin("ReferencePage")) {
      rTr.Get(token);
      continue;
    }
    // take my section
    rTr.ReadBegin("ReferencePage");
    // extract page data      
    mFrefChapter="";
    if(token.ExistsAttributeString("chapter")) 
      mFrefChapter=token.AttributeStringValue("chapter");
    mFrefSection="";
    if(token.ExistsAttributeString("section")) 
      mFrefSection=token.AttributeStringValue("section");
    mFrefPage="";
    if(token.ExistsAttributeString("page")) 
      mFrefPage=token.AttributeStringValue("page");
    mFrefTitle="";
    if(token.ExistsAttributeString("title")) 
      mFrefTitle=token.AttributeStringValue("title");
    mFrefLink=ExtractBasename(rTr.FileName())+".html";;
    if(token.ExistsAttributeString("link")) 
      mFrefLink=token.AttributeStringValue("link");
    // find optional findexdoc
    mFrefSummary="";
    if(rTr.ExistsBegin("fsummary")) {
      rTr.ReadBegin("fsummary");
      rTr.ReadSection(mFrefSummary);
      rTr.ReadEnd("fsummary");
    }  
    // autodetect misslabled index pages
    if(mFrefPage=="") {
      std::string indexfile=mFrefSection + "_index.fref";
      std::transform(indexfile.begin(), indexfile.end(), indexfile.begin(), tolower);
      if(ExtractFilename(rTr.FileName())==indexfile)
	mFrefPage="0_Index";
    }
    // ensure page number for index page
    if(mFrefPage=="index") mFrefPage="Index"; 
    if(mFrefPage=="Index") mFrefPage="0_Index";
    // autogenerate page name
    if(mFrefPage=="") {
      mFrefPage=mFrefTitle;
      std::string title=mFrefTitle;
      std::transform(mFrefTitle.begin(), mFrefTitle.end(), title.begin(), tolower);
      std::string section=mFrefSection;
      std::transform(mFrefSection.begin(), mFrefSection.end(), section.begin(), tolower);
      std::size_t spos = title.find(section);
      if(spos==0 && title.size()>section.size())
         mFrefPage=mFrefPage.substr(section.size(),mFrefPage.size()-section.size());
      for(spos=0; spos<mFrefPage.size(); spos++){
        int c= mFrefPage.at(spos);
        if(c==' ') continue;
        if(c=='-') continue;
        if(c=='_') continue;
        break;
      }
      if(spos<mFrefPage.size())
        mFrefPage=mFrefPage.substr(spos,mFrefPage.size()-spos);
      if(mFrefPage=="Index") mFrefPage="";
    }
    // read end
    rTr.ReadEnd("ReferencePage");
    // report
    // std::cerr << "ref2html: found chapter \"" << mFrefChapter << "\" section \"" << mFrefSection << "\"" << std::endl;
    // record
    PageRecord pagerec;
    pagerec.mChapter = mFrefChapter;
    pagerec.mSection = mFrefSection;
    pagerec.mPage = mFrefPage;
    pagerec.mTitle = mFrefTitle;
    pagerec.mLink = mFrefLink;
    pagerec.mSummary=mFrefSummary;
    // normalize
    std::transform(mFrefChapter.begin(), mFrefChapter.end(), mFrefChapter.begin(), tolower);
    std::transform(mFrefSection.begin(), mFrefSection.end(), mFrefSection.begin(), tolower);
    std::transform(mFrefPage.begin(), mFrefPage.end(), mFrefPage.begin(), tolower);
    // insert entry
    if(mFrefChapter!="")
    if(mFrefChapter!="none")
      mAllPages.push_back(pagerec);
    // insert entry to section pages of reference
    if(mFrefChapter=="reference") 
    if(mFrefPage!="")
    if(mFrefPage!="none")
      mRefSectPages[mFrefSection][mFrefPage]=pagerec;  
  }
}


// dump page records to include file
void DumpPages(TokenWriter& rTw) {
  // loop records
  std::vector<PageRecord>::iterator pit;
  for(pit=mAllPages.begin(); pit!=mAllPages.end(); pit++) {
    Token btag;
    btag.SetEmpty("ReferencePage");
    btag.InsAttributeString("title",pit->mTitle);
    btag.InsAttributeString("chapter",pit->mChapter);
    btag.InsAttributeString("section",pit->mSection);
    btag.InsAttributeString("page",pit->mPage);
    btag.InsAttributeString("link",pit->mLink);
    // if we have no summary, that's it
    if(pit->mSummary=="") {
       rTw << btag;
       continue;
    }
    // summary present
    btag.ClrEnd();
    rTw << btag;
    rTw.WriteBegin("fsummary");
    rTw.WriteCharacterData(pit->mSummary);
    rTw.WriteEnd("fsummary");
    rTw.WriteEnd("ReferencePage");
  }
}


// ******************************************************************
// search for types
// ******************************************************************

void ListTypesHtml(std::ostream* pIndexFile, const std::string& key="") {

  // table output 
  *pIndexFile << "<table class=\"registry_toc\">" << std::endl;
  // traverse type registry
  bool found=false;
  for(TypeRegistry::Iterator tit=TypeRegistry::G()->Begin(); 
    tit!=TypeRegistry::G()->End(); tit++) {
    // test for exact key
    if(key!="") {
      std::string section= tit->second->Name();
      std::transform(section.begin(), section.end(), section.begin(), tolower);
      if(section!=key) continue;
    }
    // test for user doc
    if(tit->second->TextDoc()=="") continue;
    // table row
    *pIndexFile << "<tr><td valign=\"top\">";
    TypeHtml(pIndexFile,tit->second->Name());
    *pIndexFile << "</td><td valign=\"top\">";
    TypeHtml(pIndexFile,tit->second->TextDoc());
    *pIndexFile << "</td></tr>" << std::endl;
    // record success
    found=true;
  }
  // no matches
  if(!found) *pIndexFile << "<tr><td><i>no matches found</i></td></tr>" << std::endl;
  // table output 
  *pIndexFile << "</table>" << std::endl;
}

// ******************************************************************
// search for functions
// ******************************************************************

void ListFunctionsHtml(std::ostream* pIndexFile, const std::string& key="") {

  // table output 
  *pIndexFile << "<table class=\"registry_toc\">" << std::endl;
  // traverse function registry
  bool found=false;
  for(FunctionRegistry::Iterator fit=FunctionRegistry::G()->Begin(); 
    fit!=FunctionRegistry::G()->End(); fit++) {
    // test for exact key
    if(key!="") {
      std::string section= fit->second->Name();
      std::transform(section.begin(), section.end(), section.begin(), tolower);
      if(section!=key) continue;
    }
    // test for user doc
    if(fit->second->TextDoc()=="") continue;
    // table row
    *pIndexFile << "<tr><td valign=\"top\">";
    FunctionHtml(pIndexFile,fit->second->Name());
    *pIndexFile << "</td><td valign=\"top\">";
    TypeHtml(pIndexFile,fit->second->TextDoc());
    *pIndexFile << "</td></tr>" << std::endl;
    // record success
    found=true;
  }
  // no matches
  if(!found) *pIndexFile << "<tr><td><i>no matches found</i></td></tr>" << std::endl;
  // table output 
  *pIndexFile << "</table>" << std::endl;
}

// ******************************************************************
// search for sections
// ******************************************************************

void ListSectionsHtml(std::ostream* pIndexFile, const std::string& key="") {

  // here: use special key "luaext" to filter lua-extensions"

  // traverse pages
  std::set< std::string > sections;
  std::map< std::string , std::string > link;
  std::map< std::string , std::string > summary;
  std::vector< PageRecord >::iterator pit;
  for(pit=mAllPages.begin(); pit != mAllPages.end(); pit++) {
    std::string chap=pit->mChapter;
    std::transform(chap.begin(), chap.end(), chap.begin(), tolower);
    std::string sect=pit->mSection;
    std::transform(sect.begin(), sect.end(), sect.begin(), tolower);
    std::string page=pit->mPage;
    std::transform(page.begin(), page.end(), page.begin(), tolower);
    page=PrettyPage(page);
    // my chapter only
    if(chap!="reference") continue;
    if(sect=="none") continue;
    if(sect=="") continue;
    if(page!="index") continue;
    // lua ext only
    if(key=="luaext") {
      if(mExclLuaSections.find(sect)!=mExclLuaSections.end()) continue;
      if(mInclLuaSections.find(sect)==mInclLuaSections.end()) continue;
    }
    // get nice name
    std::string pname = pit->mSection;
    // use title as fallback summary
    std::string psumm = pit->mSummary;
    if(psumm=="") psumm = pit->mTitle;
    // record
    sections.insert(pname);
    link[pname]=pit->mLink;
    summary[pname]=psumm;
  }

  // produce sorted index with corefaudes first
  std::vector< std::string > sortvec;
  if(sections.find("CoreFaudes")!=sections.end())
    sortvec.push_back("CoreFaudes");
  std::set< std::string >::iterator sit;
  for(sit=sections.begin(); sit != sections.end(); sit++) {
    if(*sit=="CoreFaudes") continue;
    sortvec.push_back(*sit);
  }

  // table output
  *pIndexFile << "<table class=\"registry_toc\">" << std::endl;

  // populate table
  std::vector< std::string >::iterator vit;
  bool found=false;
  for(vit=sortvec.begin(); vit != sortvec.end(); vit++) {
    // get nice name
    std::string sname = *vit;
    std::string shtml = mReferencePrefix+link[sname];
    std::string ssumm = summary[sname];
    // table row
    *pIndexFile << "<tr>" << std::endl;
    *pIndexFile << "<td valign=\"top\"><a href=\"" << shtml << "\">" << sname << "</a></td>";
    *pIndexFile << "<td valign=\"top\">";
    *pIndexFile << ssumm;
    *pIndexFile << "</td></tr>"<< std::endl;
    // record success
    found=true;
  }
  // no matches
  if(!found) *pIndexFile << "<tr><td><i>no matches found</i></td></tr>" << std::endl;

  // table output 
  *pIndexFile << "</table>" << std::endl;
}





// ******************************************************************
// Type index by keyword (aka section name)
// ******************************************************************

void TypeIndexHtml(std::ostream* pIndexFile, const std::string& key="") {

  // traverse type registry
  bool head=false;
  for(TypeRegistry::Iterator tit=TypeRegistry::G()->Begin(); 
    tit!=TypeRegistry::G()->End(); tit++) 
  {
    // test for exact key
    if(key!="") {
      std::string section= tit->second->KeywordAt(0);
      std::transform(section.begin(), section.end(), section.begin(), tolower);
      if(section!=key) continue;
    }
    // get name and reference
    std::string tyname = tit->second->Name();
    std::string tyhtml = tit->second->HtmlDoc();
    // no doc
    if(tyhtml=="") continue;
    if(tyhtml=="none") continue;
    // head line
    if(!head) {
      head=true;
      *pIndexFile << "<li class=\"registry_heading\">Types</li>" << std::endl;
    }
    // index entry for this type
    ListItemHtml(pIndexFile, tyhtml, tyname);
  }
  // done
  if(head) *pIndexFile << "<li class=\"registry_blanc\">&nbsp;</li>" << std::endl;
}

// ******************************************************************
// Function index by keyword (aka plugin)
// ******************************************************************

void FunctionIndexHtml(std::ostream* pIndexFile, const std::string& key="") {

  // have lower case key
  std::string lkey=key;
  std::transform(lkey.begin(), lkey.end(), lkey.begin(), tolower);
  
  // traverse function registry
  bool head=false;
  for(FunctionRegistry::Iterator fit=FunctionRegistry::G()->Begin(); 
    fit!=FunctionRegistry::G()->End(); fit++) 
  {
    // test for key
    if(lkey!="") {
      std::string section= fit->second->KeywordAt(0);
      std::transform(section.begin(), section.end(), section.begin(), tolower);
      if(section!=lkey) continue;
    }
    // test for user doc
    if(fit->second->TextDoc()=="") continue;
    // index entry for this function
    std::string fname = fit->second->Name();
    std::string fhtml = fit->second->HtmlDoc();
    if(fhtml=="") continue;
    // head line
    if(!head){
      head=true;
      *pIndexFile << "<li class=\"registry_heading\">Functions</li>" << std::endl;
    }
    // list entry: no doc
    if(fhtml=="none") { 
      *pIndexFile << "<li class=\"registry_item\"> "<< fname << "</li>" << std::endl;
      continue;
    }
    // list entry: link
    ListItemHtml(pIndexFile, fhtml, fname);
  }

  // done
  if(head) *pIndexFile << "<li class=\"registry_blanc\">&nbsp;</li>" << std::endl;

}

// ******************************************************************
// Reference index by keyword (aka section)
// ******************************************************************

void ReferenceIndexHtml(std::ostream* pIndexFile, const std::string& key="") {

  // prepare list of all sections
  std::map< std::string , std::string > sectlink;
  std::vector< PageRecord >::iterator pit;
  for(pit=mAllPages.begin(); pit != mAllPages.end(); pit++) {
    std::string chap=pit->mChapter;
    std::transform(chap.begin(), chap.end(), chap.begin(), tolower);
    std::string sect=pit->mSection;
    std::transform(sect.begin(), sect.end(), sect.begin(), tolower);
    // my chapter only
    if(chap!="reference") continue;
    if(sect=="none") continue;
    if(sect=="") continue;
    // get nice name
    std::string pname = pit->mSection;
    // have link
    std::string phtml = sect+"_index.html";
    // record
    sectlink[pname]=phtml;
  }

  // produce sorted index, have corefaudes first
  std::vector< std::string > sections;
  if(sectlink["CoreFaudes"]!="") sections.push_back("CoreFaudes");
  std::map< std::string , std::string >::iterator sit;
  for(sit=sectlink.begin(); sit!=sectlink.end(); sit++) {
    std::string psect=sit->first;
    if(psect=="CoreFaudes") continue;
    sections.push_back(psect);
  }

  // sections headline
  if(sections.size()!=0) 
    *pIndexFile << "<li class=\"registry_heading\">Sections</li>" << std::endl;

  // iterate sections
  std::size_t vit;
  for(vit=0; vit<sections.size(); vit++) {
    std::string psect=sections.at(vit);
    std::string plink=sectlink[psect];
    if(plink=="") continue;
    // find all pages within reference that match this section (keys to lower, skip index)
    std::string sect=psect;
    std::transform(sect.begin(), sect.end(), sect.begin(), tolower);
    std::map< std::string , std::map< std::string , PageRecord > > ::iterator spit = mRefSectPages.find(sect);
    int pcnt=0;
    if(spit!=mRefSectPages.end()) { 
      std::map< std::string , PageRecord >::iterator pit = spit->second.begin();
      for(;pit!=spit->second.end();pit++) {
        std::string ppage = pit->second.mPage;
        std::transform(ppage.begin(), ppage.end(), ppage.begin(), tolower);
        if(ppage=="index") continue;
        if(ppage=="0_index") continue;
        if(ppage=="00_index") continue;
        pcnt++;
      }
    }
    // case a) no pages: just a link
    if(pcnt==0) {
      ListItemHtml(pIndexFile, plink, psect);
      continue;
    }
    // case b) generate link for sub menu for pages (do this my self)
    *pIndexFile << "<li class=\"registry_pitem\">" << std::endl
       << "<a href=\"" << plink << "\">" << psect << "</a>"  << std::endl
       << "<a href=\"" << plink << "\" class=\"registry_blinda\">&nbsp;</a>"  << std::endl
       << "<ul>" << std::endl
       << "<li class=\"registry_heading\">" << psect << "</li>" << std::endl;
    std::map< std::string , PageRecord >::iterator pit = spit->second.begin();
    for(;pit!=spit->second.end();pit++) {
      // swallow page number
      std::string ppage = PrettyPage(pit->second.mPage);
      // normalize
      std::string ipage = ppage;
      std::transform(ipage.begin(), ipage.end(), ipage.begin(), tolower);
      // rename indes
      if(ipage=="index") ppage="Introduction";
      // page entry
      *pIndexFile << "<li class=\"registry_item\"><a href=\"" << pit->second.mLink << "\">" 
         << ppage << "</a></li>" << std::endl;
    }
    // close page list
    *pIndexFile << "</ul></li>" << std::endl; 
  }
  
  // sections done
  if(sections.size()!=0) 
    *pIndexFile << "<li class=\"registry_blanc\">&nbsp;</li>" << std::endl;

  // list types
  if(key!="") TypeIndexHtml(pIndexFile,key);

  // list functions
  if(key!="") FunctionIndexHtml(pIndexFile,key);
}


// ******************************************************************
// Section index (aka bottom navigation for key section)
// ******************************************************************


void SectionIndexHtml(std::ostream* pIndexFile, const std::string& key) {

  // find all pages within reference that match this section (keys to lower, skip index)
  std::string sect=key;
  std::transform(sect.begin(), sect.end(), sect.begin(), tolower);
  std::string plink=sect+"_index.html";
  std::string psect=key;
  std::map< std::string , std::map< std::string , PageRecord > > ::iterator spit = mRefSectPages.find(sect);
  int pcnt=0;
  if(spit!=mRefSectPages.end()) { 
    std::map< std::string , PageRecord >::iterator pit = spit->second.begin();
    for(;pit!=spit->second.end();pit++) {
      std::string ppage = pit->second.mPage;
      psect=pit->second.mSection;
      std::transform(ppage.begin(), ppage.end(), ppage.begin(), tolower);
      if(ppage=="index") continue;
      if(ppage=="0_index") continue;
      if(ppage=="00_index") continue;
      pcnt++;
    }
  }

  // bail out if there are no pages
  //if(pcnt==0) return;

  // tweak: key="" refers to the reference_index.html
  if(key=="") psect="Reference";

  // generate menu for pages 
  *pIndexFile << "<li class=\"registry_heading\">" << psect << "</li>" << std::endl;
  std::map< std::string , PageRecord >::iterator pit = spit->second.begin();
  for(;pit!=spit->second.end();pit++) {
    // swallow page number
    std::string ppage = PrettyPage(pit->second.mPage);
    // normalize
    std::string ipage = ppage;
    std::transform(ipage.begin(), ipage.end(), ipage.begin(), tolower);
    // rename index
    if(ipage=="index") ppage="Introduction";
    // page entry
    *pIndexFile << "<li class=\"registry_item\"><a href=\"" << pit->second.mLink << "\">" 
       << ppage << "</a></li>" << std::endl;
  }
}


// ******************************************************************
// signature
// ******************************************************************

void SignatureHtml(std::ostream* pOutFile, std::string function) {

  // bail out on non existence
  if(!FunctionRegistry::G()->Exists(function)) return;

  // function definition
  const FunctionDefinition& fdef=FunctionRegistry::G()->Definition(function);
  
  // bail out if there is no signature
  if(fdef.VariantsSize()==0) return;

  // start signature box
  *pOutFile << "<div class=\"registry_signature\">" << std::endl;
  *pOutFile << "<h5><strong>Signature:</strong></h5>" << std::endl;

  // loop signatures
  for(int i=0; i< fdef.VariantsSize(); i++) {
    const Signature& sigi=fdef.Variant(i);
    *pOutFile << "<p>" << fdef.Name() << "(";
    // loop params
    for(int j=0; j < sigi.Size(); j++) {
      if(j!=0) *pOutFile << ", ";
      const Parameter& parj=sigi.At(j);
      *pOutFile << "<span>+" << Parameter::AStr(parj.Attribute()) << "+ ";
      TypeHtml(pOutFile,parj.Type());
      *pOutFile << " <i>" << parj.Name() << "</i></span>";
    }
    *pOutFile << ")</p>" << std::endl;
  }


  // close signature box
  *pOutFile << std::endl << "</div>" << std::endl;
}

// ******************************************************************
// short doc
// ******************************************************************

void ShortdocHtml(std::ostream* pOutFile, std::string fname) {

  // its a function
  if(FunctionRegistry::G()->Exists(fname)) {

    // function definition
    const FunctionDefinition& fdef=FunctionRegistry::G()->Definition(fname);
  
    // stream doc
    //*pOutFile << "<div class=\"registry_signature\">" << std::endl;
    *pOutFile << "<p>" << fdef.TextDoc() << "</p>" << std::endl;
    //*pOutFile << "</div>" << std::endl;

    // stream signature
    SignatureHtml(pOutFile,fname);
  }

  // its a type
  if(TypeRegistry::G()->Exists(fname)) {

    // type definition
    const TypeDefinition& tdef=TypeRegistry::G()->Definition(fname);
  
    // stream doc
    //*pOutFile << "<div class=\"registry_signature\">" << std::endl;
    *pOutFile << "<p>" << tdef.TextDoc() << "<p>" << std::endl;
    //*pOutFile << "</div>" << std::endl;

  }

}




// ******************************************************************
// record literature
// ******************************************************************

// record
class LiteratureRecord {
public:
  std::string mLabel;
  std::string mLink;
  std::string mAuthors;
  std::string mTitle;
  std::string mJournal;
  std::string mPublisher;
  std::string mYear;
};

// data
std::map<std::string,LiteratureRecord> mLiterature;

// extract all from a section
void RecordLiterature(TokenReader& rTr) {
  // record my level
  int clevel = rTr.Level();
  // std::cerr << "process level " << clevel << "\n";
  // token loop
  while(true) {
    // skip plain text
    std::string text;
    rTr.ReadCharacterData(text);
    if(text.size()>0) continue;
    // break on no token or end of my level
    Token token;
    if(!rTr.Peek(token)) break;
    if(token.IsEnd() && !token.IsBegin() && rTr.Level()==clevel) 
      break;
    // track where we are
    if(token.IsBegin("ReferencePage")) {
      mFrefChapter="";
      if(token.ExistsAttributeString("chapter")) 
        mFrefChapter=token.AttributeStringValue("chapter");
      mFrefSection="";
      if(token.ExistsAttributeString("section")) 
        mFrefSection=token.AttributeStringValue("section");
      std::transform(mFrefChapter.begin(), mFrefChapter.end(), mFrefChapter.begin(), tolower);
      std::transform(mFrefSection.begin(), mFrefSection.end(), mFrefSection.begin(), tolower);
     // report
     // std::cerr << "ref2html: found chapter \"" << mFrefChapter << "\" section \"" << mFrefSection << "\"" << std::endl;
    }
    // ignore other tokens
    if(!token.IsBegin("fliterature")) {
      rTr.Get(token);
      continue;
    }
    // do my special tag: fliterature
    rTr.ReadBegin("fliterature");
    std::string label=token.AttributeStringValue("name");
    //std::cerr << "process literature " << label << "\n";
    LiteratureRecord litrec;
    litrec.mLabel=label;
    litrec.mLink = mFrefSection + "_index.html#lit_" + label;
    //process entries
    while(!rTr.Eos("fliterature")) {
      Token token;
      rTr.Peek(token);
      //std::cerr << "process literature peek " << token.Str() << "\n";
      if(token.IsBegin("fauthors")) {
        rTr.ReadBegin("fauthors");
        rTr.ReadSection(litrec.mAuthors);
        rTr.ReadEnd("fauthors");
        continue;
      }
      if(token.IsBegin("ftitle")) {
        rTr.ReadBegin("ftitle");
        rTr.ReadSection(litrec.mTitle);
        rTr.ReadEnd("ftitle");
        continue;
      }
      if(token.IsBegin("fjournal")) {
        rTr.ReadBegin("fjournal");
        rTr.ReadSection(litrec.mJournal);
        rTr.ReadEnd("fjournal");
        continue;
      }
      if(token.IsBegin("fyear")) {
        rTr.ReadBegin("fyear");
        rTr.ReadSection(litrec.mYear);
        rTr.ReadEnd("fyear");
        continue;
      }
      if(token.IsBegin("flink")) {
        rTr.ReadBegin("flink");
        rTr.ReadSection(litrec.mLink);
        rTr.ReadEnd("flink");
        continue;
      }
      if(token.IsBegin()) {
        rTr.ReadBegin(token.StringValue());
        rTr.ReadEnd(token.StringValue());
        continue;
      }
      rTr.Get(token);
    }
    // insert entry
    if(litrec.mLabel!="")
      mLiterature[litrec.mLabel]=litrec;
    // done
    rTr.ReadEnd("fliterature");
  }
}


// dump literature records to include file
void DumpLiterature(TokenWriter& rTw) {
  // loop records
  std::map<std::string,LiteratureRecord>::iterator lit;
  for(lit=mLiterature.begin(); lit!=mLiterature.end(); lit++) {
    Token btag;
    btag.SetBegin("fliterature");
    btag.InsAttributeString("name",lit->second.mLabel);
    rTw << btag;
    if(lit->second.mAuthors!="") {
      rTw.WriteBegin("fauthors");
      rTw.WriteCharacterData(lit->second.mAuthors); 
      rTw.WriteEnd("fauthors");
    }
    if(lit->second.mTitle!="") {
      rTw.WriteBegin("ftitle");
      rTw.WriteCharacterData(lit->second.mTitle); 
      rTw.WriteEnd("ftitle");
    }
    if(lit->second.mJournal!="") {
      rTw.WriteBegin("fjournal");
      rTw.WriteCharacterData(lit->second.mJournal); 
      rTw.WriteEnd("fjournal");
    }
    if(lit->second.mPublisher!="") {
      rTw.WriteBegin("fpublisher");
      rTw.WriteCharacterData(lit->second.mPublisher); 
      rTw.WriteEnd("fpublisher");
    }
    if(lit->second.mYear!="") {
      rTw.WriteBegin("fyear");
      rTw.WriteCharacterData(lit->second.mYear); 
      rTw.WriteEnd("fyear");
    }
    if(lit->second.mLink!="") {
      rTw.WriteBegin("flink");
      rTw.WriteCharacterData(lit->second.mLink); 
      rTw.WriteEnd("flink");
    }
    rTw.WriteEnd("fliterature"); 
    rTw.Endl();
  }
}


// html for (one) literature reference
void LiteratureHtml(std::ostream* pStream, const std::string& rLabel="") {
  // loop records
  std::map<std::string,LiteratureRecord>::iterator lit;
  for(lit=mLiterature.begin(); lit!=mLiterature.end(); lit++) {
    // skip for no match
    if(rLabel!="")
    if(rLabel!=lit->second.mLabel) continue;
    // produce html
    *pStream << "<p>" << std::endl;
    *pStream << "<a id=\"" << "lit_" << lit->second.mLabel << "\">[" << lit->second.mLabel << "]</a>" << std::endl;
    *pStream << lit->second.mAuthors << ": ";
    *pStream << "<i>" << lit->second.mTitle << "</i>";
    if(lit->second.mJournal!="")   *pStream << ", " << lit->second.mJournal;
    if(lit->second.mPublisher!="") *pStream << ", " << lit->second.mPublisher;
    if(lit->second.mYear!="")      *pStream << ", " << lit->second.mYear;
    *pStream << ".</p>" << std::endl;
  }
}


// html for literature citation
void CiteHtml(std::ostream* pStream, const std::string& rLabel) {
  // prepare
  std::string link="reference_literature.html";
  // find records
  std::map<std::string,LiteratureRecord>::iterator lit;
  lit=mLiterature.find(rLabel);
  if(lit!=mLiterature.end()) link=lit->second.mLink;
  // produce HTML
  *pStream << "<a href=\"" << link << "\">[" << rLabel << "]</a>";
}


// ******************************************************************
// extract pages
// ******************************************************************

void  XtractPages(TokenReader& src,const std::string& rDstDir) {

  // scan for reference page sections
  Token btag;
  while(src.Peek(btag)) {
    // skip tokens
    if(!btag.IsBegin("ReferencePage")) {
      src.Get(btag);
      continue;
    }
    // read begin tag
    src.ReadBegin("ReferencePage",btag);
    // extract title & friends
    mFrefTitle="libFAUDES Reference";
    if(btag.ExistsAttributeString("title")) 
      mFrefTitle=btag.AttributeStringValue("title");
    mFrefChapter="Reference";
    if(btag.ExistsAttributeString("chapter")) 
      mFrefChapter=btag.AttributeStringValue("chapter");
    mFrefSection="";
    if(btag.ExistsAttributeString("section")) 
      mFrefSection=btag.AttributeStringValue("section");
    mFrefPage="";
    if(btag.ExistsAttributeString("page")) 
      mFrefPage=btag.AttributeStringValue("page");
    // insist in page and section
    if(mFrefSection=="" || mFrefPage=="") {
      std::cerr << "ref2html: skipping undefined page at " << src.FileLine() << std::endl;
      src.ReadEnd("ReferencePage");
      continue;
    } 
    // normalize & report 
    std::transform(mFrefChapter.begin(), mFrefChapter.end(), mFrefChapter.begin(), tolower);
    std::transform(mFrefSection.begin(), mFrefSection.end(), mFrefSection.begin(), tolower);
    std::transform(mFrefPage.begin(), mFrefPage.end(), mFrefPage.begin(), tolower);
    // dst file
    std::string dstfile=PrependPath(rDstDir,mFrefSection + "_" + mFrefPage +".fref");
    std::cerr << "ref2html: extracting page to \"" << dstfile << "\"" << std::endl;
    TokenWriter dst(dstfile);
    // copy section
    dst.Write(btag);
    std::string srctext;
    src.ReadSection(srctext);
    dst.WriteCharacterData(srctext);
    dst.WriteEnd("ReferencePage");    
    // read end tag
    src.ReadEnd("ReferencePage");
  }  
}

// ******************************************************************
// extract files
// ******************************************************************

void  XtractFiles(TokenReader& src,const std::string& rDstDir) {

  // scan for file  sections
  Token btag;
  while(src.Peek(btag)) {
    // skip tokens
    if(!btag.IsBegin("ImageFile")) {
      src.Get(btag);
      continue;
    }
    // read begin tag
    src.ReadBegin("ImageFile",btag);
    std::string name=btag.AttributeStringValue("name");
    if(name==""){
      std::cerr << "ref2html: skipping undefined image file at " << src.FileLine() << std::endl;
      src.ReadEnd("ImageFile");
      continue;
    } 
    // read data
    Token data;
    src.Get(data);
    if(!data.IsBinary()){
      std::cerr << "ref2html: skipping invalid image file at " << src.FileLine() << std::endl;
      src.ReadEnd("ImageFile");
      continue;
    } 
    // dst file
    std::string dstfile;
    std::transform(name.begin(), name.end(), name.begin(), tolower);
    dstfile=PrependPath(rDstDir,"images");
    dstfile=PrependPath(dstfile,name);
    std::cerr << "ref2html: extracting image file to \"" << dstfile << "\"" << std::endl;
    // setup stream
    std::fstream fsout;
    fsout.exceptions(std::ios::badbit|std::ios::failbit);
    try{
      fsout.open(dstfile.c_str(), std::ios::out | std::ios::binary); 
      fsout.write(data.StringValue().c_str(),data.StringValue().size());
      fsout.close();
    } 
    catch (std::ios::failure&) {
      std::cerr << "ref2html: file io error when writing  \"" << dstfile << "\"" << std::endl;
    }
    // read end tag
    src.ReadEnd("ImageFile");
  }  
}

// ******************************************************************
// luafaudes index 
// ******************************************************************

void LuafaudesIndexHtml(std::ostream* pIndexFile) {

  // prepare list of all sections
  std::map< std::string , std::string > pages;
  std::vector< PageRecord >::iterator pit;
  for(pit=mAllPages.begin(); pit != mAllPages.end(); pit++) {
    // my chapter only
    if(pit->mChapter!="luafaudes") continue;
    if(pit->mSection=="none") continue;
    if(pit->mSection=="") continue;
    if(pit->mPage=="") continue;
    // get nice name
    std::string pname = pit->mPage;
    // have link
    std::string phtml = pit->mLink;
    // record
    pages[pname]=phtml;
  }
  // produce sorted index
  std::map< std::string , std::string >::iterator sit;
  for(sit=pages.begin(); sit!=pages.end(); sit++) {
    // have entry
    ListItemHtml(pIndexFile,sit->second, sit->first);
  }
  // empty
  if(pages.size()==0) {
    *pIndexFile << "<li class=\"registry_item\">" << "none" << "</li>" << std::endl;
  }
}

// ******************************************************************
// process current section
// ******************************************************************

void ProcessSection(TokenWriter& rTw, TokenReader& rTr) {

  // record my level/mode
  int clevel = rTr.Level();

  // std::cerr << "process level " << clevel << "\n";

  // token copy loop
  while(true) {
    // see whether we can grab and copy some plain text
    std::string text;
    rTr.ReadCharacterData(text);
    if(text.size()>0) {
      //std::cerr << "copy text \"" << text << "\"\n";
      rTw.WriteCharacterData(text);
      continue;
    }
    // break on no token or end of my level
    Token token;
    if(!rTr.Peek(token)) break;
    if(token.IsEnd() && !token.IsBegin() && rTr.Level()==clevel) 
      break;
    // do my special tags: ftype
    if(token.IsBegin("ftype")) {
      std::string ftype;
      rTr.ReadText("ftype",ftype);
      TypeHtml(rTw.Streamp(),ftype);
      continue;
    }
    // do my special tags: ffnct
    if(token.IsBegin("ffnct")) {
      std::string ffnct;
      rTr.ReadText("ffnct",ffnct);
      FunctionHtml(rTw.Streamp(),ffnct);
      continue;
    }
    // do my special tags: fimage
    if(token.IsBegin("fimage")) {
      rTr.ReadBegin("fimage", token);
      std::string fsrc=token.AttributeStringValue("fsrc");
      fsrc = StringSubstitute(fsrc,"FAUDES_IMAGES/",""); // be nice
      fsrc = ExtractBasename(fsrc); // be even niecer
      ImageHtml(rTw.Streamp(),fsrc);
      rTr.ReadEnd("fimage");
      continue;
    }
    // do my special tags: dmath
    if(token.IsBegin("fdmath")) {
        rTr.ReadBegin("fdmath", token);
        std::string mtext;
        rTr.ReadCharacterData(mtext);
        *rTw.Streamp() << "<span class=\"faudes_dmath\">";
        MathHtml(rTw.Streamp(),mtext);
        *rTw.Streamp()<< "</span>";
	rTr.ReadEnd("fdmath");
	continue;
    }
    // do my special tags: dmath
    if(token.IsBegin("fimath")) {
        rTr.ReadBegin("fimath", token);
        std::string mtext;
        rTr.ReadCharacterData(mtext);
        *rTw.Streamp()<< "<span class=\"faudes_imath\">";
        MathHtml(rTw.Streamp(),mtext);
        *rTw.Streamp()<< "</span>";
	rTr.ReadEnd("fimath");
	continue;
    }
    // do my special tags: ffnct_reference
    if(token.IsBegin("ffnct_reference")) {
        rTr.ReadBegin("ffnct_reference", token);
        std::string ffnct = token.AttributeStringValue("name");
        *rTw.Streamp() << "<div class=\"registry_function\"> " << std::endl;
        *rTw.Streamp() << "<h2>" << "<a id=\"" << ffnct << "\">" << ffnct << "</a></h2>" << std::endl;
        ShortdocHtml(rTw.Streamp(),ffnct);
        ProcessSection(rTw,rTr);
        *rTw.Streamp() << "</div>" << std::endl;
	rTr.ReadEnd("ffnct_reference");
	continue;
    }
    // do my special tags: ftype_reference
    if(token.IsBegin("ftype_reference")) {
        rTr.ReadBegin("ftype_reference", token);
        std::string ftype = token.AttributeStringValue("name");
        *rTw.Streamp() << "<div class=\"registry_type\"> " << std::endl;
        *rTw.Streamp() << "<h2>" << "<a id=\"" << ftype << "\">" << ftype << "</a></h2>" << std::endl;
        ShortdocHtml(rTw.Streamp(),ftype);
        ProcessSection(rTw,rTr);
        *rTw.Streamp() << "</div>" << std::endl;
	rTr.ReadEnd("ftype_reference");
	continue;
    }
    // do my special tags: fdetails
    if(token.IsBegin("fdetails")) {
        rTr.ReadBegin("fdetails", token);
        *rTw.Streamp() << "<h5>Detailed description:</h5>";
	rTr.ReadEnd("fdetails");
	continue;
    }
    // do my special tags: fconditions
    if(token.IsBegin("fconditions")) {
        rTr.ReadBegin("fconditions", token);
        *rTw.Streamp() << "<h5>Parameter Conditions:</h5>";
	rTr.ReadEnd("fconditions");
	continue;
    }
    // do my special tags: fexample
    if(token.IsBegin("fexample")) {
        rTr.ReadBegin("fexample", token);
        *rTw.Streamp() << "<h5>Example:</h5>";
	rTr.ReadEnd("fexample");
	continue;
    }
    // do my special tags: falltypes
    if(token.IsBegin("falltypes")) {
        rTr.ReadBegin("falltypes", token);
        ListTypesHtml(rTw.Streamp());
	rTr.ReadEnd("falltypes");
	continue;
    }
    // do my special tags: fallfncts
    if(token.IsBegin("fallfncts")) {
        rTr.ReadBegin("fallfncts", token);
        ListFunctionsHtml(rTw.Streamp());
	rTr.ReadEnd("fallfncts");
	continue;
    }
    // do my special tags: fallsects
    if(token.IsBegin("fallsects")) {
        rTr.ReadBegin("fallsects", token);
        ListSectionsHtml(rTw.Streamp());
	rTr.ReadEnd("fallsects");
	continue;
    }
    // do my special tags: fluasects
    if(token.IsBegin("fluasects")) {
        rTr.ReadBegin("fluasects", token);
        ListSectionsHtml(rTw.Streamp(),"luaext");
	rTr.ReadEnd("fluasects");
	continue;
    }
    // do my special tags: fliteratur (list all)
    if(token.IsBegin("falllit")) {
        rTr.ReadBegin("falllit");
        LiteratureHtml(rTw.Streamp());
	rTr.ReadEnd("falllit");
	continue;
    }
    // do my special tags: fliteratur (definition of)
    if(token.IsBegin("fliterature")) {
        rTr.ReadBegin("fliterature", token);
	std::string label=token.AttributeStringValue("name");
        LiteratureHtml(rTw.Streamp(),label);
	rTr.ReadEnd("fliterature");
	continue;
    }
    // do my special tags: fcontributors
    if(token.IsBegin("fcontributors")) {
        rTr.ReadBegin("fcontributors");
        *rTw.Streamp() << ContributorsString();
	rTr.ReadEnd("fcontributors");
	continue;
    }
    // do my special tags: flink
    if(token.IsBegin("fcite")) {
        rTr.ReadBegin("fcite", token);
        std::string label=token.AttributeStringValue("name");
        CiteHtml(rTw.Streamp(),label);
	rTr.ReadEnd("fcite");
	continue;
    }
    // do my special tags: fix br for HTML
    if(token.IsBegin("br")) {
        rTr.ReadBegin("br");
	Token etag;
        rTr.Peek(etag); 
        if(etag.IsEnd("br")) rTr.ReadEnd("br"); // optionally accept balanced <br>
        token.ClrEnd();
        token.PreceedingSpace("n");
        rTw.Write(token); // intentionall write unbalanced <br> for HTML
	continue;
    }
    // do my special tags: fsummary
    if(token.IsBegin("fsummary")) {
	rTr.ReadBegin("fsummary");
	rTr.ReadEnd("fsummary");
	continue;
    }
    // get token to do my special attributes
    rTr.Get(token);
    //std::cerr << "copy token (lv " << rTr.Level() << "): " << token.Str() << "\n";
    // sense chapter classes
    if(token.ExistsAttributeString("ftcclass")) {
      mThisChapterClass=token.AttributeStringValue("ftcclass");
      token.ClrAttribute("ftcclass");
    }
    if(token.ExistsAttributeString("focclass"))  {
      mOtherChapterClass=token.AttributeStringValue("focclass");  
      token.ClrAttribute("focclass");
    }
    if(token.ExistsAttributeString("fxcclass"))  {
      mExitChapterClass=token.AttributeStringValue("fxcclass");  
      token.ClrAttribute("fxcclass");
    }
    // chapter attribute
    if(token.ExistsAttributeString("fchapter")) {
      std::string chapter = token.AttributeStringValue("fchapter");
      std::string cclass=mOtherChapterClass;
      if(chapter==mFrefChapter) cclass=mThisChapterClass;
      if(chapter=="exit") cclass=mExitChapterClass;
      token.InsAttributeString("class",cclass);
      token.ClrAttribute("fchapter");
    }
    // fhref attribute: ref only
    if(token.ExistsAttributeString("fhref") && mStandaloneReference) {
      std::string href = token.AttributeStringValue("fhref");
      href = StringSubstitute(href,"FAUDES_BOOKS/",mBooksPrefix);
      href = StringSubstitute(href,"FAUDES_CHAPTERS/",mChaptersPrefix);
      href = StringSubstitute(href,"FAUDES_IMAGES/",mImagePrefix);
      href = StringSubstitute(href,"FAUDES_REFERENCE/",mReferencePrefix);
      href = StringSubstitute(href,"FAUDES_CSOURCE/",mCsourceLink);
      href = StringSubstitute(href,"FAUDES_LUAFAUDES/",mLuafaudesLink);
      href = StringSubstitute(href,"FAUDES_ONLINE",mFaudesLink);
      href = StringSubstitute(href,"FAUDES_GETLINUX",mDownloadLink+"#Packages");
      href = StringSubstitute(href,"FAUDES_GETMSWIN",mDownloadLink+"#Packages");
      href = StringSubstitute(href,"DESTOOL_ONLINE",mDestoolLink);
      token.InsAttributeString("href",href);
      token.ClrAttribute("fhref");
    }
    // fhref attribute
    if(token.ExistsAttributeString("fhref")  && !mStandaloneReference) {
      std::string href = token.AttributeStringValue("fhref");
      href = StringSubstitute(href,"FAUDES_BOOKS/",mBooksPrefix);
      href = StringSubstitute(href,"FAUDES_CHAPTERS/",mChaptersPrefix);
      href = StringSubstitute(href,"FAUDES_IMAGES/",mImagePrefix);
      href = StringSubstitute(href,"FAUDES_REFERENCE/",mReferencePrefix);
      href = StringSubstitute(href,"FAUDES_CSOURCE/",mCsourcePrefix);
      href = StringSubstitute(href,"FAUDES_LUAFAUDES/",mLuafaudesPrefix);
      href = StringSubstitute(href,"FAUDES_ONLINE",mFaudesLink);
      href = StringSubstitute(href,"FAUDES_GETLINUX",mDownloadLink+"#Packages");
      href = StringSubstitute(href,"FAUDES_GETMSWIN",mDownloadLink+"#Packages");
      href = StringSubstitute(href,"DESTOOL_ONLINE",mDestoolLink);
      token.InsAttributeString("href",href);
      token.ClrAttribute("fhref");
    }
    // fsrc attribute
    if(token.ExistsAttributeString("fsrc")) {
      std::string fsrc = token.AttributeStringValue("fsrc");
      fsrc = StringSubstitute(fsrc,"FAUDES_IMAGES/",mImagePrefix);
      fsrc = StringSubstitute(fsrc,"FAUDES_CSOURCE/",mCsourcePrefix);
      fsrc = StringSubstitute(fsrc,"FAUDES_LUAFAUDES/",mLuafaudesPrefix);
      token.InsAttributeString("src",fsrc);
      token.ClrAttribute("fsrc");
    }
    token.PreceedingSpace("n");
    rTw.Write(token);
  }
}


// ******************************************************************
// reference page
// ******************************************************************

void RefpageHtml(std::ostream* pOutFile, std::string inputfile) {

  // setup token io
  TokenReader src(inputfile);
  TokenWriter dst(*pOutFile);

  // find the reference page section
  Token btag;
  src.SeekBegin("ReferencePage");
  src.ReadBegin("ReferencePage",btag);

  // extract title & friends
  mFrefTitle="libFAUDES Reference";
  if(btag.ExistsAttributeString("title")) 
    mFrefTitle=btag.AttributeStringValue("title");
  mFrefChapter="";
  if(btag.ExistsAttributeString("chapter")) 
    mFrefChapter=btag.AttributeStringValue("chapter");
  mFrefSection="";
  if(btag.ExistsAttributeString("section")) 
    mFrefSection=btag.AttributeStringValue("section");
  std::transform(mFrefChapter.begin(), mFrefChapter.end(), mFrefChapter.begin(), tolower);
  std::transform(mFrefSection.begin(), mFrefSection.end(), mFrefSection.begin(), tolower);

  // report
  // std::cerr << "ref2html: found chapter \"" << mFrefChapter << "\" section \"" << mFrefSection << "\"" << std::endl;

  // generate generic html header
  dst.Flush();
  HeaderHtml(pOutFile);

  // begin body
  dst.Flush();

  // include chapter level navigation
  if(mChapterFile!="") {
    //std::cerr << "using " << mChapterFile << std::endl;
    TokenReader inc(mChapterFile);
    ProcessSection(dst,inc);
  }

  // include section level navigation, part 1
  if(mFrefChapter=="reference") {
    *pOutFile << "<table id=\"registry_page\">" << std::endl;
    *pOutFile << "<tr id=\"registry_row\">" << std::endl;
    *pOutFile << "<td id=\"registry_index\">" << std::endl;
    *pOutFile << "<ul class=\"registry_list\">" << std::endl;
    *pOutFile << "<li class=\"registry_heading\">libFAUDES</li>" << std::endl;
    ListItemHtml(pOutFile,"reference_index.html", "Reference");
    ListItemHtml(pOutFile,"reference_types.html", "Type Index");
    ListItemHtml(pOutFile,"reference_functions.html", "Function Index");
    ListItemHtml(pOutFile,"reference_literature.html", "Literature");
    *pOutFile << "<li class=\"registry_blanc\">&nbsp;</li>" << std::endl;
    ReferenceIndexHtml(pOutFile, mFrefSection);
    *pOutFile << "</ul></td>" << std::endl;
    *pOutFile << "<td id=\"registry_content\">" << std::endl;
  }

  // include section level navigation, part 1
  if(mFrefChapter=="luafaudes") {
    *pOutFile << "<table id=\"registry_page\">" << std::endl;
    *pOutFile << "<tr id=\"registry_row\">" << std::endl;
    *pOutFile << "<td id=\"registry_index\">" << std::endl;
    *pOutFile << "<ul class=\"registry_list\">" << std::endl;
    *pOutFile << "<li class=\"registry_heading\">luafaudes</li>" << std::endl;
    *pOutFile  
       <<  "<script language=\"JavaScript\"> var luafaudes=function() { "
       <<  "  popupWin = window.open('luafaudes_repl.html','open_window',"
       <<  "    'resizable,dependent, width=720, height=480, left=0, top=0'); } "
       <<  "</script>" << std::endl;
    ListItemHtml(pOutFile,"index.html", "Introduction");   
    ListItemHtml(pOutFile,"javascript:luafaudes();", "Lua-Console");
    ListItemHtml(pOutFile,"faudes_luaext.html", "Lua-Extensions");
    ListItemHtml(pOutFile,"faudes_luatech.html", "Technical Detail");
    *pOutFile << "<li class=\"registry_blanc\">&nbsp;</li>" << std::endl;
    *pOutFile << "<li class=\"registry_heading\">Tutorials</li>" << std::endl;
    LuafaudesIndexHtml(pOutFile);
    *pOutFile << "</ul></td>" << std::endl;
    *pOutFile << "<td id=\"registry_content\">" << std::endl;
  }

  // process src
  ProcessSection(dst,src);
  src.ReadEnd("ReferencePage");
  dst.Flush();

  // track bottom line
  bool bottom=false;

  // include section level navigation, part 2
  if(mFrefChapter=="reference") {
    BottomLineHtml(pOutFile);
    bottom=true;
    *pOutFile << "</td>" << std::endl;
    *pOutFile << "</tr>" << std::endl;
    *pOutFile << "</table>" << std::endl;
  }

  // include section level navigation, part 2
  if(mFrefChapter=="luafaudes") {
    BottomLineHtml(pOutFile);
    bottom=true;
    *pOutFile << "</td>" << std::endl;
    *pOutFile << "</tr>" << std::endl;
    *pOutFile << "</table>" << std::endl;
  }

  // include section level navigation, part 3
  if(mFrefChapter=="reference") {
    *pOutFile << "</div>" << std::endl << "</div>" << std::endl;
    *pOutFile << "<div id=\"cxwrapper1000\">" << std::endl;
    *pOutFile << "<div id=\"dxwrapper1000\">"  << std::endl;
    *pOutFile << "<div class=\"registry_trigger\"> <span>&gt;&gt;</span>"  << std::endl;
    *pOutFile << "<ul class=\"registry_list\">" << std::endl;
    SectionIndexHtml(pOutFile,mFrefSection);
    *pOutFile << "<li class=\"registry_blanc\">&nbsp;</li>" << std::endl;
    ListItemHtml(pOutFile,"#", "Top of Page");
    *pOutFile << "</ul></div>" << std::endl;
  }

  // include section level navigation, part 3  
  if(mFrefChapter=="luafaudes" && mFrefSection=="tutorials") {
    *pOutFile << "</div>" << std::endl << "</div>" << std::endl;
    *pOutFile << "<div id=\"cxwrapper1000\">" << std::endl;
    *pOutFile << "<div id=\"dxwrapper1000\">"  << std::endl;
    *pOutFile << "<div class=\"registry_trigger\"> <span>&gt;&gt;</span>"  << std::endl;
    *pOutFile << "<ul class=\"registry_list\">" << std::endl;
    *pOutFile << "<li class=\"registry_heading\">luafaudes</li>" << std::endl;
    *pOutFile << "<li class=\"registry_item\"><a href=\"faudes_luafaudes.html\">Introduction</a></li>" << std::endl;
    *pOutFile << "<li class=\"registry_item\"><a href=\"faudes_luaext.html\">Lua-Extansions</a></li>" << std::endl;
    *pOutFile << "<li class=\"registry_item\"><a href=\"faudes_luatech.html\">Techn. Details</a></li>" << std::endl;
    *pOutFile << "<li class=\"registry_blanc\">&nbsp;</li>" << std::endl;
    *pOutFile << "<li class=\"registry_item\"><a href=\"#\">Top of Page</a></li>" << std::endl;
    *pOutFile << "</ul></div>" << std::endl;
  }

  // bottom line
  if(!bottom) BottomLineHtml(pOutFile);

  // generic footer
  FooterHtml(pOutFile);

}



// ******************************************************************
// Doxygen header and footer
// ******************************************************************

void DoxygenHeader(std::ostream* pOutFile) {

  // setup token io
  TokenWriter dst(*pOutFile);

  // configure
  mFrefTitle="C++ API";
  mFrefChapter="cppapi";
  mFrefSection="none";

  // generate generic html header
  dst.Flush();
  HeaderHtml(pOutFile);

  // include chapter level navigation
  if(mChapterFile!="") {
    TokenReader inc(mChapterFile);
    ProcessSection(dst,inc);
  }

  // include section level navigation, part 1
  *pOutFile << "<table id=\"registry_page\">" << std::endl;
  *pOutFile << "<tr id=\"registry_row\">" << std::endl;
  *pOutFile << "<td id=\"registry_index\">" << std::endl;
  *pOutFile << "<ul class=\"registry_list\">" << std::endl;
  *pOutFile << "<li class=\"registry_heading\">libFAUDES</li>" << std::endl;
  ListItemHtml(pOutFile, "index.html", "C++ API");
  *pOutFile << "<li class=\"registry_blanc\">&nbsp;</li>" << std::endl;
  *pOutFile << "<li class=\"registry_heading\">Sections</li>" << std::endl;
  ListItemHtml(pOutFile, "group__ContainerClasses.html", "Sets");
  ListItemHtml(pOutFile, "group__GeneratorClasses.html", "Generators");
  ListItemHtml(pOutFile, "group__GeneratorFunctions.html", "Functions");
  ListItemHtml(pOutFile, "group__AllPlugins.html", "PlugIns");
  ListItemHtml(pOutFile, "group__Tutorials.html", "Tutorials");
  *pOutFile << "<li class=\"registry_blanc\">&nbsp;</li>" << std::endl;
  *pOutFile << "<li class=\"registry_heading\">Index</li>" << std::endl;
  ListItemHtml(pOutFile, "classes.html", "Classes");
  ListItemHtml(pOutFile, "files.html", "Files");
  *pOutFile << "<li class=\"registry_blanc\">&nbsp;</li>" << std::endl;
  *pOutFile << "</ul></td>" << std::endl;
  *pOutFile << "<td id=\"registry_content\">" << std::endl;
}


void DoxygenFooter(std::ostream* pOutFile) {

  // setup token io
  TokenWriter dst(*pOutFile);

  // configure
  mFrefTitle="C++ API";
  mFrefChapter="cppapi";
  mFrefSection="none";

  // include section level navigation, part 2
  BottomLineHtml(pOutFile);
  *pOutFile << "</td>" << std::endl;
  *pOutFile << "</tr>" << std::endl;
  *pOutFile << "</table>" << std::endl;

  // include section level navigation, part 3  
  *pOutFile << "</div>" << std::endl << "</div>" << std::endl;
  *pOutFile << "<div id=\"cxwrapper1000\">" << std::endl;
  *pOutFile << "<div id=\"dxwrapper1000\">"  << std::endl;
  *pOutFile << "<div class=\"registry_trigger\"> <span>&gt;&gt;</span>"  << std::endl;
  *pOutFile << "<ul class=\"registry_list\">" << std::endl;
  *pOutFile << "<li class=\"registry_heading\">C++ API</li>" << std::endl;
  *pOutFile << "<li class=\"registry_item\"><a href=\"index.html\">Introduction</a></li>" << std::endl;
  *pOutFile << "<li class=\"registry_item\"><a href=\"group__ContainerClasses.html\">Sets</a></li>" << std::endl;
  *pOutFile << "<li class=\"registry_item\"><a href=\"group__GeneratorClasses.html\">Generators</a></li>" << std::endl;
  *pOutFile << "<li class=\"registry_item\"><a href=\"group__GeneratorFunctions.html\">Functions</a></li>" << std::endl;
  *pOutFile << "<li class=\"registry_item\"><a href=\"group__AllPlugins.html\">PlugIns</a></li>" << std::endl;
  *pOutFile << "<li class=\"registry_item\"><a href=\"group__Tutorials.html\">Tutorials</a></li>" << std::endl;
  *pOutFile << "<li class=\"registry_item\"><a href=\"classes.html\">Classes</a></li>" << std::endl;
  *pOutFile << "<li class=\"registry_item\"><a href=\"files.html\">Files</a></li>" << std::endl;
  *pOutFile << "<li class=\"registry_blanc\">&nbsp;</li>" << std::endl;
  *pOutFile << "<li class=\"registry_item\"><a href=\"#\">Top of Page</a></li>" << std::endl;
  *pOutFile << "</ul></div>" << std::endl;

  // end page
  dst.Flush();
  FooterHtml(pOutFile);

}

// ******************************************************************
// command line ui
// ******************************************************************


int main(int argc, char *argv[]) {

  // local config 
  bool dotoc=false;
  bool dodhd=false;
  bool dodft=false;
  bool xpage=false;

  // min args
  if(argc < 3) usage_exit();

  // primitive commad line parsing
  int i;
  for(i=1; i<argc; i++) {
    std::string option(argv[i]);
    // option: rti file
    if(option=="-rti") { 
      i++; if(i>=argc) usage_exit();
      mRtiFile=argv[i];
      continue;
    }
    // option: flx file
    if(option=="-flx") { 
      i++; if(i>=argc) usage_exit();
      mFlxFile=argv[i];
      continue;
    }
    // option: css file
    if(option=="-css") { 
      i++; if(i>=argc) usage_exit();
      mCssFile=argv[i];
      continue;
    }
    // option: navigation include
    if(option=="-cnav") {
      i++; if(i>=argc) usage_exit();
      mChapterFile=argv[i];
      continue;
    }
    // option: toc include
    if(option=="-inc") { 
      i++; if(i>=argc) usage_exit();
      mIncludeFile=argv[i];
      continue;
    }
    // option: target prefix
    if(option=="-rel") {
      i++; if(i>=argc) usage_exit();
      ChaptersPrefix(argv[i]);
      continue;
    }
    // option: overwrite chapter
    if(option=="-chapter") {
      i++; if(i>=argc) usage_exit();
      mFrefChapter=argv[i];
      continue;
    }
    // option: overwrite section
    if(option=="-section") {
      i++; if(i>=argc) usage_exit();
      mFrefSection=argv[i];
      continue;
    }
    // option: extract multiple pages
    if(option=="-extract") {
      if(i+2!=argc-1) usage_exit();
      xpage=true;
      break;
    }
    // option: generate toc (break)
    if(option=="-toc") {
      i++; if(i+1>=argc) usage_exit();
      dotoc=true;
      mDstFile = argv[argc-1];
      break;
    }
    // option: generate doxygen header (break)
    if(option=="-doxheader") {
      i++; if(i>=argc) usage_exit();
      dodhd=true;
      mDstFile = argv[argc-1];
      break;
    }
    // option: generate doxygen footer (break)
    if(option=="-doxfooter") {
      i++; if(i>=argc) usage_exit();
      dodft=true;
      mDstFile = argv[argc-1];
      break;
    }
    // option: generate standalone reference
    if(option=="-app") {
      mStandaloneReference=true;
      continue;
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
    // non-option: break
    break;
  }

  // figure source
  for(;i<argc-1; i++) {
    std::string option(argv[i]);
    if(option.size()>1)
    if(option.at(0)=='-') {
      usage_exit("missplaced/unknown option " + option);
      continue;
    }
    mSrcFiles.insert(option);
  }

  // figure destination
  for(;i<argc; i++) {
    std::string option(argv[i]);
    if(option.size()>1)
    if(option.at(0)=='-') {
      usage_exit("missplaced/unknown option " + option);
      continue;
    }
    mDstFile=option;
  }

  // test
  if(mDstFile=="") {
    usage_exit("no destination file specified");
  }
  bool dirdst=DirectoryExists(mDstFile);

  // load registry 
  if(mRtiFile!="") {
    LoadRegistry(mRtiFile);
  }

  // record plug-in and buil-in sections
  for(FunctionRegistry::Iterator fit=FunctionRegistry::G()->Begin(); 
    fit!=FunctionRegistry::G()->End(); fit++) {
    std::string section=fit->second->KeywordAt(0);
    std::transform(section.begin(), section.end(), section.begin(), tolower);
    mExclLuaSections.insert(section);
  }

  // extend registry 
  if(mFlxFile!="") {
    FunctionRegistry::G()->MergeDocumentation(mFlxFile);
  }

  // record lua sections
  for(FunctionRegistry::Iterator fit=FunctionRegistry::G()->Begin(); 
    fit!=FunctionRegistry::G()->End(); fit++) {
    std::string section=fit->second->KeywordAt(0);
    std::transform(section.begin(), section.end(), section.begin(), tolower);
    mInclLuaSections.insert(section);
  }


  // include toc
  if(mIncludeFile!="") {
    TokenReader tr(mIncludeFile);
    RecordLiterature(tr);
    tr.Rewind();
    RecordPages(tr);
  }


  // special case: xtract fref
  if(xpage) {
    if(mSrcFiles.size()!=1) {
      usage_exit("extract mode requires one source file");
    }
    if(!dirdst) {
      usage_exit("extract mode requires destination directory");
    }
    std::cerr << "ref2html: extract pages from " << *mSrcFiles.begin() << std::endl;
    TokenReader tr(*mSrcFiles.begin());
    XtractPages(tr,mDstFile);
    tr.Rewind();
    XtractFiles(tr,mDstFile);
    exit(0);
  }


  // special case: generate toc
  if(dotoc) {
    if(dirdst) {
      usage_exit("toc mode requires destination file");
    }
    // output file
    std::ostream* hout= &std::cout;
    std::ofstream fout;
    if(mDstFile != "-") {
      fout.open(mDstFile.c_str(), std::ios::out);
      hout = &fout;
    }
    // process all input 
    std::set< std::string >::iterator sit=mSrcFiles.begin();
    for(;sit!=mSrcFiles.end();++sit) {
      std::cerr << "ref2html: process toc " << *sit << std::endl;
      TokenReader tr(*sit);
      RecordLiterature(tr);
      tr.Rewind();
      RecordPages(tr);
    }
    // dump
    TokenWriter dst(*hout);
    DumpPages(dst);
    DumpLiterature(dst);
    dst.Flush();
    exit(0);
  }

  // special case: generate dox header/footer
  if(dodhd || dodft) {
    if(dirdst) {
      usage_exit("header-footer mode requires destination file");
    }
    // output file
    std::ostream* hout= &std::cout;
    std::ofstream fout;
    if(mDstFile != "-") {
      fout.open(mDstFile.c_str(), std::ios::out);
      hout = &fout;
    }
    // doit
    if(dodhd) DoxygenHeader(hout);
    if(dodft) DoxygenFooter(hout);
    exit(0);
  }


  // convert all source directories
  std::set< std::string > srcfiles;
  std::set< std::string >::iterator sit=mSrcFiles.begin();
  for(;sit!=mSrcFiles.end();++sit) {
    if(!DirectoryExists(*sit)) { srcfiles.insert(*sit); continue;}
    std::set< std::string > dirfiles = ReadDirectory(*sit);
    std::set< std::string >::iterator dit=dirfiles.begin();
    for(;dit!=dirfiles.end();++dit) {
      std::string ext  = ExtractSuffix(*dit);
      std::string base = ExtractBasename(*dit);
      std::string src= PrependPath(*sit,base + ".fref");
      // skip if not an fref file
      if(ext!="fref") continue;
      // record
      srcfiles.insert(src);
    }
  }
  mSrcFiles=srcfiles;
 
  // insist in target dir
  if(mSrcFiles.size()>1 && ! dirdst) {
    usage_exit("multiple source files require destination directory");
  }


  // do loop
  /*std::set< std::string >::iterator*/ sit=mSrcFiles.begin();
  for(;sit!=mSrcFiles.end();++sit) {
    std::string base = ExtractBasename(*sit);
    std::string src= *sit;
    std::string dst= mDstFile;
    if(dirdst) dst=PrependPath(mDstFile,base + ".html");
    // process
    if(mSrcFiles.size()>1)
      std::cout << "ref2html: processing " << src << " to " << dst << std::endl;
    std::ofstream fout;
    fout.open(dst.c_str(), std::ios::out);
    RefpageHtml(&fout,src);
  }
  exit(0);
}
