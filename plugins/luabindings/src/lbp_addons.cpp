/** @file lbp_addons.cpp addon function for Lua integration */

/* 
FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2023 Thomas Moor

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


// my header
#include "lbp_addons.h"

// all lua headers, incl Lua
#include "lbp_include.h"

namespace faudes{


/*
**************************************************
**************************************************
**************************************************

try to be version agnostic

**************************************************
**************************************************
**************************************************
*/


/*
**************************************************
**************************************************
**************************************************

LoopCallback: have Lua-print do LoopCallback;
install LoopCallback as and Lua-line-hook

**************************************************
**************************************************
**************************************************
*/

  
// variation of luaB_print, programmatic registration
// this is still Lua 5.1.3, could be adapted for 5.4.8
int faudes_print(lua_State *L) {
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
      return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));
    if(i>1) line << "\t"; 
    line << s;           
    lua_pop(L, 1);            // pop result string
  }
  // linefeed/flush/restore mute
  line << std::endl;
  faudes::ConsoleOut::G()->Write(line.str(),0,0,0); // verb 0 <> always
  // still do loop callback (the below note on hooks also applies here)
  try{
    LoopCallback();
  } catch(...) {
    lua_pushstring(L,"break on application request");
    lua_error(L);
  }
  return 0;
}

// register pring with Lua
void faudes_print_register(lua_State* L) {
  //lua_pushstring(L, "print");
  //lua_pushcfunction(L, faudes_print);
  //lua_rawset(L, LUA_GLOBALSINDEX);
  lua_pushcfunction(L, faudes_print);
  lua_setglobal(L, "print");
}

// Lua line hook to call faudes LoopCallback()
void faudes_hook(lua_State *L, lua_Debug *ar){
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
void faudes_hook_register(lua_State* L) {
  lua_sethook(L, &faudes_hook, LUA_MASKLINE | LUA_MASKCOUNT, 10);
}

/*
**************************************************
**************************************************
**************************************************

Extension loader interface

**************************************************
**************************************************
**************************************************
*/



// lua style interface to initialize lua state
void faudes_initialize(lua_State* pL) {
  LuaState::Initialize(pL);
}

// lua style interface to install extensions
int faudes_loadext(lua_State* pL, const char* filename) {
  try{
    LuaFunctionDefinition::Register(std::string(filename));
    LuaState::Install(pL,std::string(filename));
  } catch(const Exception& except) {
    return 1;
  }
  return 0;
}

// lua style interface to load default extension
int faudes_loaddefext(lua_State* pL, const char* arg0) {
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
char **faudes_complete(lua_State* pL, const char *text, int start, int end) {
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




} //namespace
