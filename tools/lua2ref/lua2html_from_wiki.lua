------------------------------------------------------------------------------
--  Lua 2 HTML
------------------------------------------------------------------------------

local Pkg = { Name = 'Lua2HTML' }

if _VERSION == 'Lua 5.0' then
  setfenv(1, setmetatable(Pkg, {__index=_G}))

  format    = string.format
  gsub      = string.gsub
  strfind   = string.find
  strsub    = string.sub

  write     = io.write
  openfile  = io.open
  closefile = io.close
  read      = function(fh,x) return fh:read(x) end
end

local special_chars = {
  ['&'] = '&amp;',
  ['"'] = '&quot;',
  ['<'] = '&lt;',
  ['>'] = '&gt;',
}

local special
function special(x) return special_chars[x] end

local quote
function quote(str)
  return(gsub(str, '([&"<>])', special))
end

local colour
function colour(color)
  local quote = quote
  local b = '<font color="'.. color ..'">'
  local e = '</font>'
  return function(x) return b .. quote(x) .. e end
end

local html
function html(t, f)
  local b = '<'.. t ..'>'
  local e = '</'.. t ..'>'
  return function(x) return b .. f(x) .. e end
end

local operator = html('B', colour'Red')
local note     = html('B', colour'Blue')
local ctrl     = html('B', colour'Black')
local std      = colour'Teal'
local literal  = colour'Brown'
local acomment = colour'Green'

local keywords_attr = {
  ['nil']       = literal,
  ['true']      = literal,
  ['false']     = literal,

  ['local']     = note,
  ['function']  = ctrl,
  ['return']    = note,
  ['break']     = note,

  ['not']       = ctrl,
  ['and']       = ctrl,
  ['or']        = ctrl,

  ['if']        = ctrl,
  ['then']      = ctrl,
  ['else']      = ctrl,
  ['elseif']    = ctrl,

  ['for']       = ctrl,
  ['in']        = ctrl,
  ['while']     = ctrl,
  ['do']        = ctrl,
  ['end']       = ctrl,
  ['repeat']    = ctrl,
  ['until']     = ctrl,
}

local other_attr = {
  ['%']   = note,
  ['=']   = note,
  ['{']   = ctrl,
  ['}']   = ctrl,
  ['+']   = operator,
  ['-']   = operator,
  ['*']   = operator,
  ['/']   = operator,
  ['^']   = operator,
  ['<']   = operator,
  ['<=']  = operator,
  ['>']   = operator,
  ['>=']  = operator,
  ['==']  = operator,
  ['~=']  = operator,
  ['..']  = operator,
}

local stdfuncs = {
  ['Lua 4.0'] = {
    arg = std,
    self = std,
    PI = std,
    _ALERT = std,
    _ERRORMESSAGE = std,
    _INPUT = std,
    _OUTPUT = std,
    _STDERR = std,
    _STDIN = std,
    _STDOUT = std,
    _VERSION = std,
    abs = std,
    acos = std,
    appendto = std,
    ascii = std,
    asin = std,
    assert = std,
    atan = std,
    atan2 = std,
    call = std,
    ceil = std,
    clock = std,
    closefile = std,
    collectgarbage = std,
    copytagmethods = std,
    cos = std,
    date = std,
    debug = std,
    deg = std,
    dofile = std,
    dostring = std,
    error = std,
    execute = std,
    exit = std,
    exp = std,
    floor = std,
    flush = std,
    foreach = std,
    foreachi = std,
    foreachvar = std,
    format = std,
    frexp = std,
    gcinfo = std,
    getargs = std,
    getenv = std,
    getglobal = std,
    getinfo = std,
    getlocal = std,
    getn = std,
    gettagmethod = std,
    globals = std,
    gsub = std,
    ldexp = std,
    log = std,
    log10 = std,
    max = std,
    min = std,
    mod = std,
    newtag = std,
    next = std,
    nextvar = std,
    openfile = std,
    print = std,
    rad = std,
    random = std,
    randomseed = std,
    rawget = std,
    rawgetglobal = std,
    rawgettable = std,
    rawset = std,
    rawsetglobal = std,
    rawsettable = std,
    read = std,
    readfrom = std,
    remove = std,
    rename = std,
    seek = std,
    setcallhook = std,
    setglobal = std,
    setlinehook = std,
    setlocal = std,
    setlocale = std,
    settag = std,
    settagmethod = std,
    sin = std,
    sort = std,
    sqrt = std,
    strbyte = std,
    strchar = std,
    strfind = std,
    strlen = std,
    strlower = std,
    strrep = std,
    strsub = std,
    strupper = std,
    tag = std,
    tan = std,
    tinsert = std,
    tmpname = std,
    tonumber = std,
    tostring = std,
    tremove = std,
    type = std,
    write = std,
    writeto = std,
  },

  ['Lua 5.0'] = {
    arg = std,
    self = std,
    _G = std,
    _LOADED = std,
    _REQUIREDNAME = std,
    _TRACEBACK = std,
    _VERSION = std,
    assert = std,
    collectgarbage = std,
    coroutine = std,
      create = std,
      resume = std,
      status = std,
      wrap = std,
      yield = std,
    debug = std,
      debug = std,
      gethook = std,
      getinfo = std,
      getlocal = std,
      getupvalue = std,
      sethook = std,
      setlocal = std,
      setupvalue = std,
      traceback = std,
    dofile = std,
    error = std,
    gcinfo = std,
    getfenv = std,
    getmetatable = std,
    io = std,
      close = std,
      flush = std,
      input = std,
      lines = std,
      open = std,
      output = std,
      popen = std,
      read = std,
      stderr = std,
      stdin = std,
      stdout = std,
      tmpfile = std,
      type = std,
      write = std,
    ipairs = std,
    loadfile = std,
    loadlib = std,
    loadstring = std,
    math = std,
      abs = std,
      acos = std,
      asin = std,
      atan = std,
      atan2 = std,
      ceil = std,
      cos = std,
      deg = std,
      exp = std,
      floor = std,
      frexp = std,
      ldexp = std,
      log = std,
      log10 = std,
      max = std,
      min = std,
      mod = std,
      pi = std,
      pow = std,
      rad = std,
      random = std,
      randomseed = std,
      sin = std,
      sqrt = std,
      tan = std,
    newproxy = std,
    next = std,
    os = std,
      clock = std,
      date = std,
      difftime = std,
      execute = std,
      exit = std,
      getenv = std,
      remove = std,
      rename = std,
      setlocale = std,
      time = std,
      tmpname = std,
    pairs = std,
    pcall = std,
    print = std,
    rawequal = std,
    rawget = std,
    rawset = std,
    require = std,
    setfenv = std,
    setmetatable = std,
    string = std,
      byte = std,
      char = std,
      dump = std,
      find = std,
      format = std,
      gfind = std,
      gsub = std,
      len = std,
      lower = std,
      rep = std,
      sub = std,
      upper = std,
    table = std,
      concat = std,
      foreach = std,
      foreachi = std,
      getn = std,
      insert = std,
      remove = std,
      setn = std,
      sort = std,
    tonumber = std,
    tostring = std,
    type = std,
    unpack = std,
    xpcall = std,
    __index = std,
    __newindex = std,
    __gc = std,
    __eq = std,
    __add = std,
    __sub = std,
    __mul = std,
    __div = std,
    __unm = std,
    __pow = std,
    __lt = std,
    __le = std,
    __concat = std,
    __call = std,
    __tostring = std,
    __metatable = std,
    __fenv = std,
  },
}

local stdfunctions_attr = stdfuncs[strsub(_VERSION,1,7)]
                       or error('bad version: '.. _VERSION)

local sqstring
function sqstring(code, e)
  while 1 do
    e = strfind(code, '[\'\\]', e)
    if not e then error'bad single quoted string' end
    if strfind(code, '^\\.', e) then
      e = e+2
    elseif strfind(code, '^\'', e) then
      return e
    else
      error'impossible?'
    end
  end
end

local dqstring
function dqstring(code, e)
  while 1 do
    e = strfind(code, '[\"\\]', e)
    if not e then error'bad double quoted string' end
    if strfind(code, '^\\.', e) then
      e = e+2
    elseif strfind(code, '^\"', e) then
      return e
    else
      error'impossible?'
    end
  end
end

local longstring
function longstring(code, e)
  local count = 0
  local b
  while 1 do
    b,e = strfind(code, '[%[%]].', e)
    if not b then error'bad long comment or string' end
    if strfind(code, '^%[%[', b) then
      e = e+1
      count = count+1
    elseif strfind(code, '^%]%]', b) then
      if count == 0 then
        return e
      end
      e = e+1
      count = count-1
    else
      -- okay
    end
  end
end

local other
function other(code, i)
  local b,e
  b,e = strfind(code, '^\'.', i)
  if b then return sqstring(code, e), literal end

  b,e = strfind(code, '^\".', i)
  if b then return dqstring(code, e), literal end

  b,e = strfind(code, '^%[%[.', i)
  if b then return longstring(code, e), literal end

  b,e = strfind(code, '^%.%.%.', i) if b then return e end
  b,e = strfind(code, '^%.%.', i)   if b then return e end
  b,e = strfind(code, '^%=%=', i)   if b then return e end
  b,e = strfind(code, '^%~%=', i)   if b then return e end
  b,e = strfind(code, '^%>%=', i)   if b then return e end
  b,e = strfind(code, '^%<%=', i)   if b then return e end

  return i
end

local number
function number(code, i)
  local b,e,e1,e2,e3
  b,e1 = strfind(code, '^%d+%.%d*', i)  -- D+ . D*
  b,e2 = strfind(code, '^%d*%.%d+', i)  -- D* . D+
  b,e3 = strfind(code, '^%d+', i)       -- D+
  i = e1 or e2 or e3
  b,e = strfind(code, '^[Ee][+-]?%d+', i+1)
  return e or i, literal
end

local word
function word(code, i)
  local b,e
  b,e = strfind(code, '^[_%a][_%w]*', i)
  local token = strsub(code, b, e)
  return e, keywords_attr[token] or stdfunctions_attr[token]
end

local comment
function comment(code, i)
  local b,e
  b,e = strfind(code, '^%-%-%[%[.', i)
  if b then return longstring(code, e) end

  b,e = strfind(code, '^%-%-[^\n]*', i)
  if b then return e end

  b,e = strfind(code, '^%#[^\n]*', i)
  if b then return e end
end

local highlight
function highlight(code, outfnc)
  code = gsub(code, '\r\n', '\n')  -- DOS is a pain

  local line = { number = 0 }
  local ln
  function ln()
    line.number = line.number+1
    return format('\n%4d  ', line.number)
  end
  local output
  function output(x, fmt)
    fmt = fmt or function(x) return x end
    x = gsub(x, '(%S[^\n]*)', fmt)  -- format groups of non-newline characters
    x = gsub(x, '\n', ln)           -- add line number after a newline
    outfnc(x)
  end

  output'\n'  -- line number 1

  local ws = 1
  local tok, e, fmt
  while 1 do
    e = ws
    while 1 do
      tok = strfind(code, '%S', e)
      if not tok then return end
      if not strfind(code, '^[-#]', tok) then break end
      e = comment(code, tok)
      if not e then break end
      e = e+1
    end

    output(strsub(code, ws, tok-1), acomment) -- white space and comments

    if     strfind(code, '^[_%a]', tok) then e,fmt = word  (code, tok)
    elseif strfind(code, '^%.?%d', tok) then e,fmt = number(code, tok)
    elseif strfind(code, '^%p', tok)    then e,fmt = other (code, tok)
    else
      error'impossible?'
    end

    local token = strsub(code, tok, e)
    output(token, fmt or other_attr[token])

    ws = e+1
  end
end

function Pkg.lua2html(filename, outfnc)
  local fh = openfile(filename, "r")
  local code =  read(fh, '*a')
  closefile(fh)

  outfnc = outfnc or write
  outfnc("<h3>".. filename .."</h3>\n")
  outfnc'<pre>'
  highlight(code, outfnc)
  outfnc'</pre>\n'
end

Pkg.lua2html(arg[1])

return Pkg