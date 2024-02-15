/** @file coreaddons.i @brief bindings global add-ons */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2023  Thomas Moor
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


/*
**************************************************
**************************************************
**************************************************

Exception handling

**************************************************
**************************************************
**************************************************
*/

// catch exceptions
// note: not functional, since libfaudes does not specify throw()
/*
%typemap(throws) Exception
%{ 
  lua_pushstring(L,$1.Message()); 
  SWIG_fail; 
%}
*/

// catch exceptions, alternative
// via swig directive/expection handling
#ifdef SWIGPYTHON
%exception {
  try{
    $action
  }
  catch (const Exception& except) {
    PyErr_SetString(PyExc_Exception, const_cast<char*>(except.Message()));
    SWIG_fail;
  }
  catch (...) {
    PyErr_SetString(PyExc_Exception, "Unknown Error");
    SWIG_fail;
  }
}
#else
%exception {
  try{
    $action
  }
  catch (const Exception& except) {
    SWIG_exception(SWIG_RuntimeError,except.Message());
    return 0;
  }
  catch (...) {
    SWIG_exception(SWIG_RuntimeError,"Unknown Error");
    return 0;
  }
}    
#endif


// programatically throw exceptions, implementation
%{
namespace faudes {
void faudes_throw_exception(const std::string& msg) {
  throw Exception("faudes script", msg, 49);
}
}  
%}

// explicitely throw exceptions, convenient interface
%rename(Error) faudes_throw_exception;
void faudes_throw_exception(const std::string& msg);


/*
**************************************************
**************************************************
**************************************************

Faudes global behaviour: statenames, dot path, ...

**************************************************
**************************************************
**************************************************
*/

// c-implementation of behavioural features
%{

namespace faudes {

// global switches
bool faudes_statenames = true;
std::string faudes_dotpath = "dot";

// access functions
void faudes_statenames_on(void) {faudes_statenames=true; Generator::StateNamesEnabledDefault(true); }
void faudes_statenames_off(void) {faudes_statenames=false; Generator::StateNamesEnabledDefault(false); }
void faudes_dotexecpath(const std::string& filename)  { faudes_dotpath=filename; }
std::string  faudes_version(void) { return VersionString()+" "+PluginsString(); }

// helper: run dot for test
bool faudes_dotready(void) {
  // cache value
  static bool ready=false;  
  static bool known=false;
  if(known) return ready;    
  // test for dot binary
  std::string testdot = faudes_dotpath + " -V";
  ready = (system(testdot.c_str()) == 0);
  known = true;
  return ready;
}

}//namespace
  
%}


// convenient interface
%rename(StateNamesOn) faudes_statenames_on;
void faudes_statenames_on(void);
%rename(StateNamesOff) faudes_statenames_off;
void faudes_statenames_off(void);
%rename(DotExecPath) faudes_dotexecpath;
void faudes_dotexecpath(const std::string& filename);
%rename(Version) faudes_version;
std::string  faudes_version(void);

 
/*
**************************************************
**************************************************
**************************************************

Faudes global behaviour: console out 

**************************************************
**************************************************
**************************************************
*/

// c code
%{

namespace faudes {

// cheap console out, mutable, strings only, SIWG interface
void faudes_debug(const std::string& message) {
  // print
  std::ostringstream line;
  line << "FAUDES_PRINT: " <<  message << std::endl;
  faudes::ConsoleOut::G()->Write(line.str());
  // still do loop callback
  LoopCallback();
}

// cheap console out, strings only, no mute, SIWG interface
void faudes_console(const std::string& message) {
  int m;
  // un-mute
  m=faudes::ConsoleOut::G()->Mute();
  faudes::ConsoleOut::G()->Mute(false);
  // print
  std::ostringstream line;
  line << "FAUDES_PRINT: " <<  message << std::endl;
  faudes::ConsoleOut::G()->Write(line.str());
  // restore mute
  faudes::ConsoleOut::G()->Mute(m);
  // still do loop callback
  LoopCallback();
}

// mute libFAUDES console outout
void faudes_mute(bool on) {
  faudes::ConsoleOut::G()->Mute(on);
}

}//namespace

%}


// convenient interface
%rename(Print) faudes_debug;
void faudes_debug(const std::string& message);
%rename(Mute) faudes_mute;
void faudes_mute(bool on);


/*
**************************************************
**************************************************
**************************************************

Faudes global behaviour: mini help system

**************************************************
**************************************************
**************************************************
*/


// c code
%{

namespace faudes {  

//global help dictionary topic -> key -> text
std::map< std::string, std::string > faudes_dictionary_topics;
std::map< std::string, std::map<std::string, std::vector<std::string> > > faudes_dictionary_entries;

// insert topic to dictionary
void faudes_dict_insert_topic(const std::string& topic, const std::string& text) {
  faudes_dictionary_topics[topic]=text;
}

// insert entry to dictionary
void faudes_dict_insert_entry(const std::string& topic, const std::string& key, const std::string& entry) {
  std::string k=key;
  if(k.length()>0) 
    k.at(0)=toupper(k.at(0));
  faudes_dictionary_entries[topic][k].push_back(entry);
  if(topic!="")
  if(faudes_dictionary_topics[topic]=="") 
    faudes_dictionary_topics[topic]=topic;
}

}//namespace  

%}

// short form as init code
%define SwigHelpTopic(TOPIC,ENTRY)
%init{ faudes_dict_insert_topic(TOPIC,ENTRY); }
%enddef

// short form as init code
%define SwigHelpEntry(TOPIC,KEY,ENTRY)
%init{ faudes_dict_insert_entry(TOPIC,KEY,ENTRY); }
%enddef


// c code
%{

namespace faudes {  

// main help text
void faudes_help(void) {
  std::stringstream sstr;

  // section: intro
  sstr << std::endl
    << "libFAUDES bindings provide access to libFAUDES data structures and functions. " << std::endl
    << "For detailed information, please consult the libFAUDES documentation."<< std::endl
    << std::endl
    << "All libFAUDES bindings are in the module 'faudes', ie access is via 'faudes.*'." << std::endl
    << std::endl
    << "Interface:" << std::endl
    << std::endl;

  // section: topics
  std::map< std::string, std::string >::iterator tit; 
  for(tit=faudes_dictionary_topics.begin(); tit!=faudes_dictionary_topics.end();tit++) {
    std::string left  = "  faudes.Help(\"" + tit->first +"\")";
    std::string right = tit->second;
    while(left.length()<37) left+=" ";
    sstr << left << right << std::endl;
  }
  sstr << std::endl;

  // section: behavioural
  sstr 
    << "Configuration:" << std::endl
    << std::endl
    << "  faudes.StateNamesOn()              enable automatic state names" << std::endl
    << "  faudes.StateNamesOff()             disable automatic state names" << std::endl
    << "  faudes.DotExecPath(\"filename\")     path of dot executable" << std::endl
    << "  faudes.Version()                   return libfaudes version string" << std::endl
    << std::endl
    << "Console Commands:" << std::endl
    << std::endl
    << "  faudes.Print(\"message\")            print as faudes debugging message" << std::endl
    << "  faudes.Error(\"message\")            abort script with error message" << std::endl
    << "  faudes.Mute(bool)                  mute faudes debugging messages" << std::endl;

  // do print to stderr
  faudes_console(sstr.str());
}


// section text
void faudes_help(const std::string& topic) {
  std::stringstream sstr;

  // section: intro
  sstr  
    << std::endl
    << "libFAUDES help topic: \"" << topic << "\"" << std::endl
    << std::endl;

  // section: list entries
  std::map< std::string, std::map< std::string, std::vector<std::string> > >::iterator tit; 
  tit = faudes_dictionary_entries.find(topic);
  if(tit!=faudes_dictionary_entries.end()) {
    std::map< std::string, std::vector<std::string> >::iterator kit;   
    for(kit = tit->second.begin(); kit != tit->second.end(); kit++) {
      if(kit != tit->second.begin()) sstr << std::endl;
      const std::string& line = kit->first;
      sstr << "                 *** " << line << " ***" << std::endl;
      for(unsigned int i=0; i< kit->second.size(); i++) {
        const std::string& line = kit->second[i];
	std::size_t sep = line.find_first_of(' ');
        if(sep==std::string::npos) sep=0;
        while(sep<20) { sstr << " "; sep++; };
        sstr << line << std::endl;
      }
    }
  } else {
    sstr << "  (no entries) " << std::endl;
  }

  // do print to stderr
  faudes_console(sstr.str());
}

}//namespace  

%}

// announce to swig
%rename(Help) faudes_help;
void faudes_help(void);
void faudes_help(const std::string& topic);


/*
**************************************************
**************************************************
**************************************************

Faudes global behaviour: test protocol

**************************************************
**************************************************
**************************************************
*/

// announce helper functions to swig
void TestProtocol(const std::string& rSource); 
bool TestProtocol(void); 
void TestProtocol(const std::string& rMessage, const Type& rData, bool full=false); 
void TestProtocol(const std::string& rMessage, bool data); 
void TestProtocol(const std::string& rMessage, long int data); 
void TestProtocol(const std::string& rMessage, const std::string& data);


// provide convenient interface (Python version)
#ifdef SWIGPYTHON
%pythoncode %{

import sys  

def TEST_DUMP(m,d):
  file = "faudes_console_dump"
  if sys.argv[0] != "": 
    file = sys.argv[0]
  _faudes.TestProtocol(file)
  _faudes.TestProtocol(m + " [at " + file + "]",d)

def TEST_DIFF(): 
  res = _faudes.TestProtocol()
  file = "faudes_console_dump"
  if sys.argv[0] != "": 
    file = sys.argv[0]
  if not res:
    print("FAUDES_TEST_DIFF: sensed test case error in " + file)
  return res

import builtins

builtins.FAUDES_TEST_DUMP = TEST_DUMP
builtins.FAUDES_TEST_DIFF = TEST_DIFF

%}
#endif


// provide convenient interface (Lua versoin)
#ifdef SWIGLUA
%luacode %{

function FAUDES_TEST_DUMP(m,d) 
  local file
  if arg ~= nil then
    file= arg[0]
  else 
    file= "faudes_console_dump"
  end
  faudes.TestProtocol(file)
  faudes.TestProtocol(m .. " [at " .. file .. "]",d)
end

function FAUDES_TEST_DIFF(m,d) 
  local res = faudes.TestProtocol()
  local file
  if arg ~= nil then
    file= arg[0]
  else 
    file= "faudes_console_dump"
  end
  if not res then
    print("FAUDES_TEST_DIFF: sensed test case error in " .. file)
  end
  return res
end
%}
#endif
