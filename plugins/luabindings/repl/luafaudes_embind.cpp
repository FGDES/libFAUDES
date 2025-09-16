/** @file luafaudes_embind.cpp luafaudes cwith emscripten bindings */

/* 
FAU Discrete Event Systems Library (libfaudes)

Copyright (C) 2014, 2025 Thomas Moor

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

#include <emscripten/bind.h>
#include "libfaudes.h"

using namespace emscripten;

// void declaration to complete forward typedef from lua.h 
class lua_State {};

// wrappers to convert c-strings to std::string
int emb_luaL_loadbuffer(lua_State *L, const std::string& buff,
			const std::string& name) {
  return luaL_loadbuffer(L,buff.c_str(),buff.length(),name.c_str());
}
std::string emb_lua_tolstring(lua_State *L, int idx) {
  size_t len;
  const char* str= lua_tolstring(L,idx,&len);
  return std::string(str,len);
}
const unsigned long  emb_lua_topointer(lua_State *L, int idx) {
  return (unsigned long) lua_topointer(L,idx); //buintptr_t 
}
std::string emb_lua_typename(lua_State *L, int tp) {
  return std::string(lua_typename(L,tp));
}
int emb_luafaudes_loadext(lua_State* L, const std::string& filename) {
  return faudes::faudes_loadext(L, filename.c_str());
}

// wrappers for functions that became macros in Lua 5.4.x
int emb_lua_pcall(lua_State *L, int nargs, int nresults, int errfunc) {
  return lua_pcallk(L,nargs,nresults,errfunc, 0, NULL);
}
long int emb_lua_tonumber(lua_State *L, int idx) {
  return (long int) lua_tonumberx(L,idx,NULL);
}
unsigned long int  emb_lua_objlen(lua_State *L, int idx) {
  return (unsigned long int) lua_rawlen(L,idx);
}

EMSCRIPTEN_BINDINGS(my_module) {
  class_<lua_State>("lua_State");
  function("luaL_newstate", &luaL_newstate, allow_raw_pointers()); 
  function("luaL_openlibs", &luaL_openlibs, allow_raw_pointers()); 
  function("lua_pcall", &emb_lua_pcall, allow_raw_pointers()); 
  function("lua_settop", &lua_settop, allow_raw_pointers()); 
  function("lua_gettop", &lua_gettop, allow_raw_pointers()); 
  function("luaL_loadbuffer", &emb_luaL_loadbuffer, allow_raw_pointers()); 
  function("lua_type", &lua_type, allow_raw_pointers()); 
  function("lua_tolstring", &emb_lua_tolstring, allow_raw_pointers()); 
  function("lua_toboolean", &lua_toboolean, allow_raw_pointers()); 
  function("lua_tonumber", &emb_lua_tonumber, allow_raw_pointers()); 
  function("lua_objlen", &emb_lua_objlen, allow_raw_pointers()); 
  function("lua_typename", &emb_lua_typename, allow_raw_pointers()); 
  function("lua_topointer", &emb_lua_topointer, allow_raw_pointers()); 
  function("luaopen_faudes", &luaopen_faudes, allow_raw_pointers()); 
  function("luaopen_faudes_allplugins", &luaopen_faudes_allplugins, allow_raw_pointers());
  function("luafaudes_loadext", &emb_luafaudes_loadext, allow_raw_pointers());
}

