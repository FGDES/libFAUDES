/*
********************************************************************
********************************************************************
********************************************************************

 Lua completer, derived from Lua advanced readline 
 support patch, with original copyright al follows: 

 Copyright (C) 2004-2006 Mike Pall. 
 Same license as Lua. See lua.h.

 The below version is adapted to go with SWIG generated
 __index functions (as opposed to the perhaps more common
 __index tables). The interface was changed to a more
 C++ like style, to make a better match with applications
 like DESTool. The current implementation is preliminary
 and requires some clean-up.

 This file is included by lbp_function.cpp and the functions 
 are called by the LuaState methods for completion.

 Thomas Moor, 2011.

********************************************************************
********************************************************************
********************************************************************
*/


// Lua keywords
static const char *const faudes_rl_keywords[] = {
  "and", "break", "do", "else", "elseif", "end", "false",
  "for", "function", "if", "in", "local", "nil", "not", "or",
  "repeat", "return", "then", "true", "until", "while", NULL
};

// Test identifier
static bool faudes_rl_valididentifier(const std::string& str) {
  std::string::const_iterator sit=str.begin();
  if(sit==str.end()) return false;
  if(!(isalpha(*sit) || *sit=='_')) return false;
  for(++sit; sit!=str.end(); ++sit) 
    if(!(isalnum(*sit) || *sit=='_')) return true;
  return true;
}

// Append compound prefix + string + suffix to list and maintain common prefix. 
static void faudes_rl_dmadd(std::list< std::string > & mlist, const std::string& prefix, const std::string& str, const std::string& suffix) {
  // iterate suffxes
  if(suffix.size()>1) {
    for(unsigned int i=0;i<suffix.size();i++)
      faudes_rl_dmadd(mlist,prefix,str,std::string(1,suffix.at(i)));
    return;
  }  		   
  // build compound
  std::string comp;
  comp=prefix + str + suffix;
  // initialize: matching prefix equals first entry
  if(mlist.size()==0) {
    mlist.push_back(comp);
    return;
  }
  // initialize: matching prefix equals first entry
  if(mlist.size()==1) {
    mlist.push_back(*mlist.begin());
    mlist.push_back(comp);
  }
  // figure maximal matching prefix
  std::string& match=*mlist.begin();
  std::string::iterator mit, cit;
  mit=match.begin();
  cit=comp.begin();
  while(mit!=match.end() && cit!=comp.end()) {
    if(*mit!=*cit) break;
    mit++; cit++;
  }
  // adjust matching prefix in list
  if(mit!=match.end())
    match.erase(mit,match.end());   
  // append to list
  mlist.push_back(comp);
}

// Get __index field of metatable of object on top of stack 
// -- return 1 for ok and leave __index field on stack
// -- return 0 for error, nothing on stack
static int faudes_rl_getmetaindex(lua_State *L) {
  // object has no metatable, error
  if(!lua_getmetatable(L, -1)) 
    { lua_pop(L, 1); return 0; } // stack: 
  lua_replace(L, -2);            // stack: metatable 
  // get the __ index field
  lua_pushstring(L, "__index");
  lua_rawget(L, -2);             // stack: metatable,__index
  lua_replace(L, -2);            // stack: __index  
  // invald result, error
  if(lua_isnil(L,-1) || lua_rawequal(L, -1, -2)) 
    { lua_pop(L, 1); return 0; } // stack:
  return 1;                      // stack: __index
}  

// Get .fn fields of metatable of object on top of stack. 
// -- the SWIG __index function retrieves methods from the .fn attribute
// -- allways puts a table on the stack, empty on error
static void faudes_rl_getmetafn(lua_State *L) {
  // object has no metatable: return empty table
  if(!lua_getmetatable(L, -1)) 
    { lua_pop(L, 1); lua_newtable(L); return; }
  lua_replace(L, -2);         // stack: metatable
  // get .fn
  lua_pushstring(L, ".fn");
  lua_rawget(L, -2);          // stack: metatable,.fn
  lua_replace(L, -2);         // stack: .fn
  if(!lua_istable(L, -1))     
    { lua_pop(L, 1); lua_newtable(L); return; }
} 

// Get .get field of metatable of object on top of stack. 
// -- the SWIG __index function retrieves member access from the .get attribute
// -- allways puts a table on the stack, empty on error
static void faudes_rl_getmetaget(lua_State *L) {
  // object has no metatable: just pop the object
  if(!lua_getmetatable(L, -1)) 
    { lua_pop(L, 1); lua_newtable(L); return; }
  lua_replace(L, -2);         // stack: metatable
  // get .get
  lua_pushstring(L, ".get");
  lua_rawget(L, -2);          // stack: metatable,.get
  lua_replace(L, -2);         // stack: .get
  if(!lua_istable(L, -1))     
    { lua_pop(L, 1); lua_newtable(L); return; }
} 

// Get __index/.fn/.get field of metatable of object on top of stack. 
// -- allways puts a table on stack, empty of error
// -- returns 1 to indicate ussage of SWIG .get table
static int faudes_rl_getmeta(lua_State *L, bool has_colon) {
  // try __index first
  lua_pushvalue(L,-1);     // stack: obj,obj
  if(faudes_rl_getmetaindex(L)) {
    // is it a table       // stack: obj,_index
    if(lua_istable(L,-1)) {
      // is it non-empty?    // stack: obj,_index
      lua_pushnil(L); 
      if(lua_next(L, -2)) {
        lua_pop(L, 2);       // stack: obj,_index
        lua_replace(L, -2);  // stack: _index
        return 0;
      }
    }
    lua_pop(L,1);          // stack: obj1
  }                        // stack: obj
  // colon indicates method, so we use .fn
  if(has_colon) {
    faudes_rl_getmetafn(L); // stack: .fn
    return 0;
  }
  // absence of colon indicates member, so we use .get
  faudes_rl_getmetaget(L);  // stack: .get
  return 1;

}


// Get field from object on top of stack (without calling metamethods) 
static int faudes_rl_getfield(lua_State *L, const char *s, size_t n) {
  int i = 20;  // max loop count to limit infinite metatable loops. */
  do {
    if(lua_istable(L, -1)) {       // if obj is a table, try to get the requested field
      lua_pushlstring(L, s, n);
      lua_rawget(L, -2);           // stack: obj,field
      if(!lua_isnil(L, -1))        // ok, got the field
        {lua_replace(L, -2); return 1;} 
      lua_pop(L, 1);               // stack: obj
    }
    if(!faudes_rl_getmetaindex(L)) break;  // stack: _index
  } while (--i > 0);               
  lua_pop(L, 1);
  return 0;
}  /* 1: obj -- val, 0: obj -- */


// actual completer
static std::list< std::string > faudes_rl_complete(lua_State *L, const std::string& word) {

  // C++ style interface
  std::list< std::string > mlist;
  const char* text= word.c_str(); 
  int start=0;
  int end=word.size();

  // other locals
  const char *s;
  size_t i, n, dot, loop; 
  int colon;
  int savetop;
  bool used_swig_get=false;

  // bail out on text that cannot complete to an identifier
  if (!(text[0] == '\0' || isalpha(text[0]) || text[0] == '_')) return mlist;

  // record top of stack
  savetop = lua_gettop(L);

  // figure the right most complete field in "word" and
  // -- leave the table of globals on the stack, if there is no complete field
  lua_pushglobaltable(L);
  for(n = (size_t)(end-start), i = dot = 0, colon=-1; i < n; i++) {
    if(text[i] == '.' || text[i] == ':') {
      if(!faudes_rl_getfield(L, text+dot, i-dot))
	{ lua_settop(L, savetop); return mlist; } // error
      dot = i+1;  // Points to first char after dot/colon.
      if(text[i] == ':') colon=dot; // record whether we have seen a colon
    }
  }
    
  // Append all matches against keywords if there is no dot/colon. 
  if(dot == 0)
    for (i = 0; (s = faudes_rl_keywords[i]) != NULL; i++)
      if(!strncmp(s, text, n)) faudes_rl_dmadd(mlist, "", std::string(s), " ");

  // Append all valid matches from all tables/metatables. 
  loop = 0;  // Avoid infinite metatable loops. 
  do {
    if(lua_istable(L, -1)) 
    if((loop == 0 || 1 /* !lua_rawequal(L, -1, LUA_GLOBALSINDEX) */) ) {  // TODO: this used to be Lua 5.1.3 .. what todo with Lua 5.4.8?
      for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
	if (lua_type(L, -2) == LUA_TSTRING) {
	  s = lua_tostring(L, -2);
	  // Only match names starting with '_' if explicitly requested. 
	  if (!strncmp(s, text+dot, n-dot) && faudes_rl_valididentifier(s) &&
	      (*s != '_' || text[dot] == '_')) {
	    std::string suf = " ";  // Default suffix is a space. 
	    switch (lua_type(L, -1)) {
	    case LUA_TTABLE:	suf = ":."; break;  
	    case LUA_TFUNCTION:	if(!used_swig_get) suf = "("; break;
	    case LUA_TUSERDATA:
	      if (lua_getmetatable(L, -1)) { lua_pop(L, 1); suf = ":."; }
	      break;
	    }
	    faudes_rl_dmadd(mlist, std::string(text,dot), std::string(s), suf);
	  }
	}
      }
    }
    used_swig_get = faudes_rl_getmeta(L,colon>0);
  } while (++loop < 20);

  // Fix stack
  lua_settop(L, savetop);

  // done
  return mlist;
}

