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



// convenient interface
%rename(StateNamesOn) faudes_statenames_on;
void faudes_statenames_on(void);
%rename(StateNamesOff) faudes_statenames_off;
void faudes_statenames_off(void);
%rename(DotExecPath) faudes_dotexecpath;
void faudes_dotexecpath(const std::string& filename);
%rename(Version) faudes_version;
std::string  faudes_version(void);
%rename(Build) faudes_build;
std::string  faudes_build(void);

 
/*
**************************************************
**************************************************
**************************************************

Faudes global behaviour: console out 

**************************************************
**************************************************
**************************************************
*/


// convenient interface
void Print(const std::string& message);
void Print(int v, const std::string& message);
void Verbosity(int v);
int Verbosity();


/*
**************************************************
**************************************************
**************************************************

Faudes global behaviour: mini help system

**************************************************
**************************************************
**************************************************
*/

// short form as init code
%define SwigHelpTopic(TOPIC,ENTRY)
%init{ faudes_dict_insert_topic(TOPIC,ENTRY); }
%enddef

// short form as init code
%define SwigHelpEntry(TOPIC,KEY,ENTRY)
%init{ faudes_dict_insert_entry(TOPIC,KEY,ENTRY); }
%enddef



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
