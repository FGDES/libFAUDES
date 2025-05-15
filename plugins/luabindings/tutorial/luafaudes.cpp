/** @file luafaudes.cpp Simple Lua stand-alone interpreter for lua/faudes
 
   This lua interpreter is almost a plain copy of the original lua.c 
   provided with the lua 5.1.3 distribution  and under an MIT license;
   see original lua.h or http://www.lua.org

   The Advanced-Readline patch has been applied, also with lua license,
   copyright see below. SWIG based Lua bindings and minor adjustments 
   (wellcome string) have been added (and signed "luafaudes").
   Thomas Moor, 2008.


@ingroup Tutorials


*/


// luafaudes: skip doxygen
#ifndef FAUDES_DOXYGEN

/*
** $Id: lua.c,v 1.160.1.2 2007/12/28 15:32:23 roberto Exp $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/


// luafaudes: include c libs since we dont use lua.h
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>


// luafaudes: use my include file (incl. lpb_include.h)
#include "libfaudes.h"

// luafaudes: import all
using namespace faudes;


// luafaudes: dislike isatty on MSYS
#if defined(LUA_USE_ISATTY)
#include <unistd.h>
#define faudes_stdin_is_tty()	isatty(0)
#else
#define faudes_stdin_is_tty()	1 
#endif



// below this line: all from lua provided interpreter lua.cpp, except tagged "luafaudes"
static lua_State *globalL = NULL;
static const char *progname = LUA_PROGNAME;


static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}


static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}


static void print_usage (void) {
  fprintf(stderr,
  "usage: %s [options] [script [args]].\n"
  "Available options are:\n"
  "  -e stat  execute string " LUA_QL("stat") "\n"
  "  -l name  require library " LUA_QL("name") "\n"
  "  -i       enter interactive mode after executing " LUA_QL("script") "\n"
  "  -v       show version information\n"
  "  -d       pass on libFAUDES messages to console\n"   /* luafaudes: d-option */
  "  -x       load libFAUDES extension\n"                /* luafaudes: x-option */
  "  --       stop handling options\n"
  "  -        execute stdin and stop handling options\n"
  ,
  progname);
  fflush(stderr);
}


static void l_message (const char *pname, const char *msg) {
  if (pname) fprintf(stderr, "%s: ", pname);
  fprintf(stderr, "%s\n", msg);
  fflush(stderr);
}


static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);
  }
  return status;
}


static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}


static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}


// luafaudes: my print version
static void print_version (void) {
  std::stringstream sstr;
  sstr 
    << "Welcome to luafaudes console." << std::endl
    << "Versions: " << VersionString() << " / " << LUA_VERSION << std::endl
    << "Build: " << BuildString() << std::endl
    << "Plug-Ins: " << PluginsString() << std::endl
    << "Credits: This libFAUDES interpreter is based on the projects Lua and SWIG." << std::endl
    << "Type 'faudes.Help()' for a list of faudes related types and functions." << std::endl
    << "Enter Ctrl-C to exit the luafaudes interpreter" << std::endl;
  l_message(NULL, sstr.str().c_str());
}


static int getargs (lua_State *L, char **argv, int n) {
  int narg;
  int i;
  int argc = 0;
  while (argv[argc]) argc++;  /* count total number of arguments */
  narg = argc - (n + 1);  /* number of arguments to the script */
  luaL_checkstack(L, narg + 3, "too many arguments to script");
  for (i=n+1; i < argc; i++)
    lua_pushstring(L, argv[i]);
  lua_createtable(L, narg, n + 1);
  for (i=0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - n);
  }
  return narg;
}


static int dofile (lua_State *L, const char *name) {
  int status = luaL_loadfile(L, name) || docall(L, 0, 1);
  return report(L, status);
}


static int dostring (lua_State *L, const char *s, const char *name) {
  int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
  return report(L, status);
}


static int dolibrary (lua_State *L, const char *name) {
  lua_getglobal(L, "require");
  lua_pushstring(L, name);
  return report(L, docall(L, 1, 1));
}



/* ------------------------------------------------------------------------ */

#ifdef LUA_USE_READLINE
/*

** luafaudes: here used to be advanced readline support patch by Mike Pall. However,
** we required some variations for SWIG __index functions and we also wanted identifier-
** completion in DESTool. Thus, we moved the original completer code to LuaState 
** (see lbp_function.cpp and lbp_completer.cpp) and provide a C interface via
** luafaudes_complete() (see lbp_addons.cpp). The below code is reduced to the libreadline 
** interface. The original patch still ships with libFAUDES for reference and can be 
** inspected in lua.c in the Lua source tree. TMoor. 

** Advanced readline support for the GNU readline and history libraries
** or compatible replacements.
**
** Copyright (C) 2004-2006 Mike Pall. Same license as Lua. See lua.h.
**
** The (original) patch has been successfully tested with:
**
** GNU    readline 2.2.1  (1998-07-17)
** GNU    readline 4.0    (1999-02-18) [harmless compiler warning]
** GNU    readline 4.3    (2002-07-16)
** GNU    readline 5.0    (2004-07-27)
** GNU    readline 5.1    (2005-12-07)
** NETBSD libedit  2.6.5  (2002-03-25)
** NETBSD libedit  2.6.9  (2004-05-01)
*/

#include <ctype.h>

static char *lua_rl_hist;
static int lua_rl_histsize;

static lua_State *lua_rl_L;  /* User data is not passed to rl callbacks. */

/* luafaudes: pass on global state */
static char **faudes_complete_L(const char *text, int start, int end) {
  return faudes_complete(lua_rl_L,text,start,end);
}  

/* Initialize readline library. */
static void lua_rl_init(lua_State *L)
{
  char *s;

  lua_rl_L = L;

  /* This allows for $if lua ... $endif in ~/.inputrc. */
  rl_readline_name = "lua";
  /* Break words at every non-identifier character except '.' and ':'. */
  rl_completer_word_break_characters = (char *) 
    "\t\r\n !\"#$%&'()*+,-/;<=>?@[\\]^`{|}~"; // luafaudes: pragmatic avoidance of compiler warning

  rl_completer_quote_characters = "\"'";
  rl_completion_append_character = '\0';
  rl_attempted_completion_function = faudes_complete_L; // luafaudes: use libFAUDES variant
  rl_initialize();

  /* Start using history, optionally set history size and load history file. */
  using_history();
  if ((s = getenv("LUA_HISTSIZE")) &&
      (lua_rl_histsize = atoi(s))) stifle_history(lua_rl_histsize);
  if ((lua_rl_hist = getenv("LUA_HISTORY"))) read_history(lua_rl_hist);
}

/* Finalize readline library. */
static void lua_rl_exit(lua_State *L)
{
  /* Optionally save history file. */
  if (lua_rl_hist) write_history(lua_rl_hist);
}
#else
#define lua_rl_init(L)		((void)L)
#define lua_rl_exit(L)		((void)L)
#endif

/* ------------------------------------------------------------------------ */


static const char *get_prompt (lua_State *L, int firstline) {
  const char *p;
  lua_getfield(L, LUA_GLOBALSINDEX, firstline ? "_PROMPT" : "_PROMPT2");
  p = lua_tostring(L, -1);
  if (p == NULL) p = (firstline ? LUA_PROMPT : LUA_PROMPT2);
  lua_pop(L, 1);  /* remove global */
  return p;
}


static int incomplete (lua_State *L, int status) {
  if (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const char *msg = lua_tolstring(L, -1, &lmsg);
    const char *tp = msg + lmsg - (sizeof(LUA_QL("<eof>")) - 1);
    if (strstr(msg, LUA_QL("<eof>")) == tp) {
      lua_pop(L, 1);
      return 1;
    }
  }
  return 0;  /* else... */
}


static int pushline (lua_State *L, int firstline) {
  char buffer[LUA_MAXINPUT];
  char *b = buffer;
  size_t l;
  const char *prmt = get_prompt(L, firstline);
  if (lua_readline(L, b, prmt) == 0)
    return 0;  /* no input */
  l = strlen(b);
  if (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[l-1] = '\0';  /* remove it */
  if (firstline && b[0] == '=')  /* first line starts with `=' ? */
    lua_pushfstring(L, "return %s", b+1);  /* change it to `return' */
  else
    lua_pushstring(L, b);
  lua_freeline(L, b);
  return 1;
}


static int loadline (lua_State *L) {
  int status;
  lua_settop(L, 0);
  if (!pushline(L, 1))
    return -1;  /* no input */
  for (;;) {  /* repeat until gets a complete line */
    status = luaL_loadbuffer(L, lua_tostring(L, 1), lua_strlen(L, 1), "=stdin");
    if (!incomplete(L, status)) break;  /* cannot try to add lines? */
    if (!pushline(L, 0))  /* no more input? */
      return -1;
    lua_pushliteral(L, "\n");  /* add a new line... */
    lua_insert(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
  }
  lua_saveline(L, 1);
  lua_remove(L, 1);  /* remove line */
  return status;
}


static void dotty (lua_State *L) {
  int status;
  const char *oldprogname = progname;
  progname = NULL;
  lua_rl_init(L);
  while ((status = loadline(L)) != -1) {
    if (status == 0) status = docall(L, 0, 0);
    report(L, status);
    if (status == 0 && lua_gettop(L) > 0) {  /* any result to print? */
      lua_getglobal(L, "print");
      lua_insert(L, 1);
      if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0)
        l_message(progname, lua_pushfstring(L,
                               "error calling " LUA_QL("print") " (%s)",
                               lua_tostring(L, -1)));
    }
  }
  lua_settop(L, 0);  /* clear stack */
  fputs("\n", stdout);
  fflush(stdout);
  lua_rl_exit(L);
  progname = oldprogname;
}

static int handle_script (lua_State *L, char **argv, int n) {
  int status;
  const char *fname;
  int narg = getargs(L, argv, n);  /* collect arguments */
  lua_setglobal(L, "arg");
  fname = argv[n];
  if (strcmp(fname, "-") == 0 && strcmp(argv[n-1], "--") != 0) 
    fname = NULL;  /* stdin */
  status = luaL_loadfile(L, fname);
  lua_insert(L, -(narg+1));
  if (status == 0)
    status = docall(L, narg, 0);
  else
    lua_pop(L, narg);      
  return report(L, status);
}


/* check that argument has no extra characters at the end */
#define notail(x)	{if ((x)[2] != '\0') return -1;}


// luafaudes: have extra options -d and -x
static int collectargs (char **argv, int *pi, int *pv, int *pe, int *pd, int *px) {
  int i;
  for (i = 1; argv[i] != NULL; i++) {
    if (argv[i][0] != '-')  /* not an option? */
        return i;
    switch (argv[i][1]) {  /* option */
      case '-':
        notail(argv[i]);
        return (argv[i+1] != NULL ? i+1 : 0);
      case '\0':
        return i;
      case 'i':
        notail(argv[i]);
        *pi = 1;  /* go through */
      case 'v':
        notail(argv[i]);
        *pv = 1;
        break;
      case 'e':
        *pe = 1;  /* go through */
      case 'l':
        if (argv[i][2] == '\0') {
          i++;
          if (argv[i] == NULL) return -1;
        }
        break;
      case 'd': // luafaudes: d-option
        notail(argv[i]);
        *pd = 1;
        break;
      case 'x': // luafaudes: x-option
        if (argv[i][2] == '\0') {
          i++;
          if (argv[i] == NULL) return -1;
        }
        *px = 1;
        break;
      default: return -1;  /* invalid option */
    }
  }
  return 0;
}


// luafaudes: have extra option -x
static int runargs (lua_State *L, char **argv, int n) {
  int i;
  for (i = 1; i < n; i++) {
    if (argv[i] == NULL) continue;
    lua_assert(argv[i][0] == '-');
    switch (argv[i][1]) {  /* option */
      case 'e': {
        const char *chunk = argv[i] + 2;
        if (*chunk == '\0') chunk = argv[++i];
        lua_assert(chunk != NULL);
        if (dostring(L, chunk, "=(command line)") != 0)
          return 1;
        break;
      }
      case 'l': {
        const char *filename = argv[i] + 2;
        if (*filename == '\0') filename = argv[++i];
        lua_assert(filename != NULL);
        if (dolibrary(L, filename))
          return 1;  /* stop if file fails */
        break;
      }
      case 'x': {  // luafaudes option x
        const char *filename = argv[i] + 2;
        if (*filename == '\0') filename = argv[++i];
        lua_assert(filename != NULL);
        if (faudes_loadext(L, filename)) {
          l_message("fatal error: failed to load extension", filename);
          return 1;  /* stop if file fails */
        }
        break;
      }
      default: break;
    }
  }
  return 0;
}


static int handle_luainit (lua_State *L) {
  const char *init = getenv(LUA_INIT);
  if (init == NULL) return 0;  /* status OK */
  else if (init[0] == '@')
    return dofile(L, init+1);
  else
    return dostring(L, init, "=" LUA_INIT);
}


struct Smain {
  int argc;
  char **argv;
  int status;
};


static int pmain (lua_State *L) {
  struct Smain *s = (struct Smain *)lua_touserdata(L, 1);
  char **argv = s->argv;
  int script;
  int has_i = 0, has_v = 0, has_e = 0, has_d=0, has_x=0; // luafaudes: -d and -x option
  globalL = L;
  if (argv[0] && argv[0][0]) progname = argv[0];
  faudes_initialize(L); // luafaudes: all of the below plus my namespace
  /*
  // original lua
  lua_gc(L, LUA_GCSTOP, 0);  
  luaL_openlibs(L);  
  lua_gc(L, LUA_GCRESTART, 0);
  */
  s->status = handle_luainit(L);
  if (s->status != 0) return 0;
  script = collectargs(argv, &has_i, &has_v, &has_e, &has_d, &has_x); // luafaudes: -d and -x option
  if (script < 0) {  /* invalid args? */
    print_usage();
    s->status = 1;
    return 0;
  }
  if (has_v) print_version(); 
  if (!has_d) faudes_mute(true); /* luafaudes: mute */
  if (!has_x) faudes_loaddefext(L, argv[0]);  /* luafaudes: default extension */
  s->status = runargs(L, argv, (script > 0) ? script : s->argc);
  if (s->status != 0) return 0;
  if (script)
    s->status = handle_script(L, argv, script);
  if (s->status != 0) return 0;
  if (has_i) {
    dotty(L);
    return 0;
  }
  if (script == 0 && !has_e) {
    if (faudes_stdin_is_tty()) {
      if(!has_v) print_version(); 
      dotty(L);
      return 0;
    }
  }
  dofile(L, NULL);  /* executes stdin as a file */
  return 0;
}


int main (int argc, char **argv) {
  int status;
  struct Smain s;
  lua_State *L = lua_open();  /* create state */
  if (L == NULL) {
    l_message(argv[0], "cannot create state: not enough memory");
    return EXIT_FAILURE;
  }
  s.argc = argc;
  s.argv = argv;
  status = lua_cpcall(L, &pmain, &s);
  report(L, status);
  lua_close(L);
  return (status || s.status) ? EXIT_FAILURE : EXIT_SUCCESS;
}


// luafaudes: end skip doxygen
#endif
