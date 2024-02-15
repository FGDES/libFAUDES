/** @file commonfaudes.i @brief luabindings basic types */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008  Thomas Moor
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
%typemap(throws) Exception
%{ 
  lua_pushstring(L,$1.Message()); 
  SWIG_fail; 
%}

// catch exceptions
// via swig directive/expection handling
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

// explicitely throw exceptions, implementation
%{
void luafaudes_exception(const std::string& msg) {
  throw Exception("luafaudes script", msg, 49);
}
%}

// explicitely throw exceptions, lua interface
%rename(Error) luafaudes_exception;
void luafaudes_exception(const std::string& msg);


/*
**************************************************
**************************************************
**************************************************

Command logging: Functions/Globals that implement
logging in the luafaudes comand line interpreter.

**************************************************
**************************************************
**************************************************
*/

// c-implementation of logging features
%{

// logging globals
std::ofstream* luafaudes_logstream = NULL;
std::string luafaudes_lastline;

// turn logging on
void luafaudes_initlog(const std::string& filename, const std::string& mode) {
  luafaudes_lastline = ""; // do not write this as a log entry
  luafaudes_closelog();
  try {
    if(mode=="w") {
      luafaudes_logstream = new std::ofstream(filename.c_str(), std::ios::out|std::ios::trunc);
    } else if(mode=="a") {
      luafaudes_logstream = new std::ofstream(filename.c_str(), std::ios::out|std::ios::app);
    } else {
      throw Exception("luafaudes_initlog", "invalid mode specified", 2);
    }
    if(luafaudes_logstream->fail()) {
      throw Exception("luafaudes_initlog", "invalid mode specified", 2);
    }
  } catch (std::ios::failure&) {
    throw Exception("luafaudes_initlog", "cannot log to file "+filename, 2);
  }
}

// turn logging off
void luafaudes_closelog(void) {
  if (luafaudes_logstream == NULL) return;
  try {
    luafaudes_logstream->close();
    delete(luafaudes_logstream);
    luafaudes_logstream = NULL;
  } catch (std::ios::failure&) {
    throw Exception("luafaudes_closelog", "cannot close logfile",2);
  }
}


// do the logging
void luafaudes_logwrite(const char* str) {
  if(luafaudes_logstream == NULL) return;
  try {
    *luafaudes_logstream << str << std::endl;
    luafaudes_logstream->flush();
  } catch (std::ios::failure&) {
    throw Exception("luafaudes_logwrite", "cannot log to file", 2);
  }
}

// do the logging
void luafaudes_logwrite(const std::string& str) {
  if(luafaudes_logstream == NULL) return;
  luafaudes_logwrite(str.c_str());
}

%}

// lua interface
%rename(InitLog) luafaudes_initlog;
void luafaudes_initlog(const std::string& filename, const std::string& mode="w");
%rename(CloseLog) luafaudes_closelog;
void luafaudes_closelog(void);


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

// global switches
bool luafaudes_statenames = true;
std::string luafaudes_dotpath = "dot";

// access functions
void luafaudes_statenames_on(void) {luafaudes_statenames=true; Generator::StateNamesEnabledDefault(true); }
void luafaudes_statenames_off(void) {luafaudes_statenames=false; Generator::StateNamesEnabledDefault(false); }
void luafaudes_dotexecpath(const std::string& filename)  { luafaudes_dotpath=filename; }
std::string  luafaudes_version(void) { return VersionString()+" "+PluginsString(); }

// helper: run dot for test
bool luafaudes_dotready(void) {
  // cache value
  static bool ready=false;  
  static bool known=false;
  if(known) return ready;    
  // test for dot binary
  std::string testdot = luafaudes_dotpath + " -V";
  ready = (system(testdot.c_str()) == 0);
  known = true;
  return ready;
}


%}


// lua interface
%rename(StateNamesOn) luafaudes_statenames_on;
void luafaudes_statenames_on(void);
%rename(StateNamesOff) luafaudes_statenames_off;
void luafaudes_statenames_off(void);
%rename(DotExecPath) luafaudes_dotexecpath;
void luafaudes_dotexecpath(const std::string& filename);
%rename(Version) luafaudes_version;
std::string  luafaudes_version(void);


/*
**************************************************
**************************************************
**************************************************

Faudes global behaviour: console out / loop callback

**************************************************
**************************************************
**************************************************
*/


// c code
%{

// cheap console out, mutable, strings only, SIWG interface
void luafaudes_debug(const std::string& message) {
  // print
  std::ostringstream line;
  line << "FAUDES_LUAPRINT: " <<  message << std::endl;
  faudes::ConsoleOut::G()->Write(line.str());
  // still do loop callback
  LoopCallback();
}

// cheap console out, strings only, no mute, SIWG interface
void luafaudes_console(const std::string& message) {
  int m;
  // un-mute
  m=faudes::ConsoleOut::G()->Mute();
  faudes::ConsoleOut::G()->Mute(false);
  // print
  std::ostringstream line;
  line << "FAUDES_LUAPRINT: " <<  message << std::endl;
  faudes::ConsoleOut::G()->Write(line.str());
  // restore mute
  faudes::ConsoleOut::G()->Mute(m);
  // still do loop callback
  LoopCallback();
}

// variation of luaB_print, programmatic registration
int luafaudes_print(lua_State *L) {
  int n,i,m;
  // line buffer
  std::ostringstream line;
  // loop all args, use Lua's "tostring" for conversion
  n = lua_gettop(L);      // number of args
  lua_getglobal(L, "tostring");
  for(i=1; i<=n; i++) {
    const char *s;
    lua_pushvalue(L, -1);     // push "tostring" fnct
    lua_pushvalue(L, i);      // push i-th arg
    lua_call(L, 1, 1);        // execute
    s = lua_tostring(L, -1);  // retrieve result string
    if(s == NULL)
      return luaL_error(L, LUA_QL("tostring") " must return a string to "
                           LUA_QL("print"));
    if(i>1) line << "\t"; 
    line << s;           
    lua_pop(L, 1);            // pop result string
  }
  // linefeed/flush/restore mute
  line << std::endl;
  m=faudes::ConsoleOut::G()->Mute();
  faudes::ConsoleOut::G()->Mute(false);
  faudes::ConsoleOut::G()->Write(line.str());
  faudes::ConsoleOut::G()->Mute(m);
  // still do loop callback (the below note on hooks also applies here)
  try{
    LoopCallback();
  } catch(...) {
    lua_pushstring(L,"break on application request");
    lua_error(L);
  }
  return 0;
}

// register luafaudes print
void luafaudes_print_register(lua_State* L) {
  lua_pushstring(L, "print");
  lua_pushcfunction(L, luafaudes_print);
  lua_rawset(L, LUA_GLOBALSINDEX);
}

// mute libFAUDES console outout
void luafaudes_mute(bool on) {
  faudes::ConsoleOut::G()->Mute(on);
}


%}


// lua interface
%rename(Print) luafaudes_debug;
void luafaudes_debug(const std::string& message);
%rename(Mute) luafaudes_mute;
void luafaudes_mute(bool on);



// c code
%{

// Lua line hook to call faudes LoopCallback()
void luafaudes_hook(lua_State *L, lua_Debug *ar){
  if(ar->event != LUA_HOOKLINE) return;
  // note: this resides within lua, no SWIG interface, so we must not throw
  // exceptions (this would set the lua interpreter to an inconsistent state);
  // thus, we must catch the exception and convert to a lua error; when e.g. the hook
  // was during a faudes::LuaFunction, DoExceuteC() will sense the error and then 
  // throw the faudes excpetion
  try{
    LoopCallback();
  } catch(...) {
    lua_pushstring(L,"break on application request");
    lua_error(L);
  }
}

// register LoopCallback() as Lua hook
void luafaudes_hook_register(lua_State* L) {
  lua_sethook(L, &luafaudes_hook, LUA_MASKLINE | LUA_MASKCOUNT, 10);
}

%}

// this one is good for lua too
void LoopCallback(void);




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

//global help dictionary topic -> key -> text
std::map< std::string, std::string > luafaudes_dictionary_topics;
std::map< std::string, std::map<std::string, std::vector<std::string> > > luafaudes_dictionary_entries;

// insert topic to dictionary
void luafaudes_dict_insert_topic(const std::string& topic, const std::string& text) {
  luafaudes_dictionary_topics[topic]=text;
}

// insert entry to dictionary
void luafaudes_dict_insert_entry(const std::string& topic, const std::string& key, const std::string& entry) {
  std::string k=key;
  if(k.length()>0) 
    k.at(0)=toupper(k.at(0));
  luafaudes_dictionary_entries[topic][k].push_back(entry);
  if(topic!="")
  if(luafaudes_dictionary_topics[topic]=="") 
    luafaudes_dictionary_topics[topic]=topic;
}

%}

// short form as init code
%define SwigHelpTopic(TOPIC,ENTRY)
%init{ luafaudes_dict_insert_topic(TOPIC,ENTRY); }
%enddef

// short form as init code
%define SwigHelpEntry(TOPIC,KEY,ENTRY)
%init{ luafaudes_dict_insert_entry(TOPIC,KEY,ENTRY); }
%enddef


// c code
%{


// main help text
void luafaudes_help(void) {
  std::stringstream sstr;

  // section: intro
  sstr << std::endl
    << "Luafaudes extends the lua scripting language to execute libFAUDES functions. " << std::endl
    << "For detailed information on algorithms and data structures, please consult" << std::endl
    << "the libFAUDES documentation."<< std::endl
    << std::endl
    << "All faudes bindings are in the lua module 'faudes', ie access is via 'faudes.*'." << std::endl
    << "You may copy faudes bindings to the global name space by 'faudes.MakeGlobal()'." << std::endl
    << std::endl
    << "libFAUDES Bindings:" << std::endl
    << std::endl;

  // section: topics
  std::map< std::string, std::string >::iterator tit; 
  for(tit=luafaudes_dictionary_topics.begin(); tit!=luafaudes_dictionary_topics.end();tit++) {
    std::string left  = "  faudes.Help(\"" + tit->first +"\")";
    std::string right = tit->second;
    while(left.length()<37) left+=" ";
    sstr << left << right << std::endl;
  }
  sstr << std::endl;

  // section: behavioural
  sstr 
    << "libFAUDES Configuration:" << std::endl
    << std::endl
    << "  faudes.StateNamesOn()              enable automatic state names" << std::endl
    << "  faudes.StateNamesOff()             disable automatic state names" << std::endl
    << "  faudes.DotExecPath(\"filename\")     path of dot executable" << std::endl
    << "  faudes.Version()                   return libfaudes version string" << std::endl
    << std::endl
    << "luafaudes Console Commands:" << std::endl
    << std::endl
    << "  faudes.Print(\"message\")            print as faudes debugging message" << std::endl
    << "  faudes.Error(\"message\")            abort script with error message" << std::endl
    << "  faudes.Mute(bool)                  mute faudes debugging messages" << std::endl
    << "  faudes.InitLog(\"filename\")         init logging of commands" << std::endl
    << "  faudes.CloseLog()                  stop logging of commands" << std::endl;

  // do print to stderr
  luafaudes_console(sstr.str());
}


// section text
void luafaudes_help(const std::string& topic) {
  std::stringstream sstr;

  // section: intro
  sstr  
    << std::endl
    << "Luafaudes help topic: \"" << topic << "\"" << std::endl
    << std::endl;

  // section: list entries
  std::map< std::string, std::map< std::string, std::vector<std::string> > >::iterator tit; 
  tit = luafaudes_dictionary_entries.find(topic);
  if(tit!=luafaudes_dictionary_entries.end()) {
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
  luafaudes_console(sstr.str());

}

%}

// announce to swig
%rename(Help) luafaudes_help;
void luafaudes_help(void);
void luafaudes_help(const std::string& topic);



/*
**************************************************
**************************************************
**************************************************

Extension loader interface

**************************************************
**************************************************
**************************************************
*/


%{

// lua style interface to initialize lua state
void luafaudes_initialize(lua_State* pL) {
  LuaState::Initialize(pL);
}

// lua style interface to install extensions
int luafaudes_loadext(lua_State* pL, const char* filename) {
  try{
    LuaFunctionDefinition::Register(std::string(filename));
    LuaState::Install(pL,std::string(filename));
  } catch(const Exception& except) {
    return 1;
  }
  return 0;
}

// lua style interface to load default extension
int luafaudes_loaddefext(lua_State* pL, const char* arg0) {
  std::string flxfile= std::string(arg0)+".flx";
  if(!FileExists(flxfile)) return 1;
  try{
    LuaState::Install(pL,flxfile);
  } catch(const Exception& except) {
    return 1;
  }
  return 0;
}

// lua/libreadline style interface to completer
char **luafaudes_complete(lua_State* pL, const char *text, int start, int end) {
  std::string word(text,end-start);
  std::list< std::string > mlist = LuaState::Complete(pL,word);
  if(mlist.size()==0) return NULL;
  char** res = (char**) malloc(sizeof(char *) * (mlist.size() + 1));
  if(!res) return (char**) NULL;
  std::list< std::string >::iterator lit;
  char** dst=res;
  for(lit=mlist.begin(); lit!=mlist.end(); lit++) 
    *(dst++)=strdup(lit->c_str());
  *dst=NULL;
  return res;
}


%}


/*
**************************************************
**************************************************
**************************************************

Type: faudes base class

**************************************************
**************************************************
**************************************************
*/


// macro for universal faudes base class members
%define SwigTypeMembers()

  // maintenance
  virtual Type& Assign(const Type& rType);
  %extend {  
    Type Copy(void) const { return *($self->Copy());}; // fix swig ownership flag
  }
  virtual void Clear(void);

  // token output
  void Write(const Type* pContext=0) const;
  void Write(const std::string& pFileName, const std::string& rLabel="", const Type* pContext=0) const;
  std::string ToString(const std::string& rLabel="", const Type* pContext=0) const;
  void DWrite(const Type* pContext=0) const;
  void DWrite(const std::string& pFileName, const std::string& rLabel="", const Type* pContext=0) const;
  void SWrite(void) const;
  void XWrite(const Type* pContext=0) const;
  void XWrite(const std::string& pFileName, const std::string& rLabel="", const Type* pContext=0) const;
  // token based input
  void Read(const std::string& rFileName, const std::string& rLabel = "", const Type* pContext=0);
  void FromString(const std::string& rString, const std::string& rLabel = "", const Type* pContext=0);
  // provide lua string conversion
  %extend {
    std::string __str__(void) { return $self->ToString();};
  }

%enddef


// universal faudes base class
class Type {
public:
  // construct/destruct
  Type(void);
  Type(const Type& rType);
  virtual ~Type();

  // have token io
  SwigTypeMembers()
};


// helper: report object type
%rename(TypeName) FaudesTypeName;
const std::string& FaudesTypeName(const Type& rObject);

// helper: test faudes cast
%rename(TypeTest) FaudesTypeTest;
bool FaudesTypeTest(const std::string& rTypeName, const Type& rObject);

/*
**************************************************
**************************************************
**************************************************

Faudes atomics Idx and Transition

**************************************************
**************************************************
**************************************************
*/

// faudes index type
typedef uint32_t Idx;
typedef long int Int;
typedef double Float;

// faudes transition type
class Transition {
 public:
  // construct/destruct  
  Transition(void);
  Transition(Idx x1, Idx ev,Idx x2);
  // lua compatible operators
  bool operator < (const Transition& othertrans) const;
  bool operator == (const Transition& othertrans) const;
  // maintenance/access
  bool Valid(void) const;
  Idx X1;
  Idx Ev; 
  Idx X2;
  // convenience
  std::string Str(void) const;
  // provide lua string conversion
  %extend {
    std::string __str__(void) { return $self->Str();};
  };
};

// convenience: construct transition by event name
%extend Transition {
   Transition(Idx x1, const std::string& rEvent, Idx x2) {
   static EventSet defeset;
   return new Transition(x1,defeset.SymbolTablep()->Index(rEvent),x2); };
};



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


// provide convenient interface
%luacode {
-- FAUDES test protocol interface 
function FAUDES_TEST_DUMP(m,d) 
  local file
  if arg ~= nil then
    file= arg[0]
  else 
    file= "luafaudes_console_dump"
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
    file= "luafaudes_console_dump"
  end
  if not res then
    print("FAUDES_TEST_DIFF: sensed test case error in " .. file)
  end
  return res
end
-- print("FAUDES_TEST_DUMP defined")

}
