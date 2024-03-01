#!/usr/bin/perl

#-- adapted lua2html Perl-script for libFAUDES documentation

#-- faudes add on: extract argument aka filename
($arg1) = @ARGV;
$arg1 =~ s!\\!/!go;  ##  normalise to unix dirsep
use File::Basename;
$fhead= basename($arg1);

#-- Minimal infrastructure, copied from NoNameWiki.
%SaveUrl = ();
$SaveUrlIndex = 0;
$FS = "\x01";

sub QuoteHtml {
  my ($html) = @_;
  $html =~ s/$FS//go;
  $html =~ s/&/&amp;/g; 
  $html =~ s/</&lt;/g; 
  $html =~ s/>/&gt;/g;
  return $html;
}

sub StoreRaw {
  my ($html) = @_;
  $SaveUrl{$SaveUrlIndex} = $html;
  return $FS . $SaveUrlIndex++ . $FS;
}

#-- Functions for Lua 5.1 syntax highlighting:
sub StoreSpan {
  my ($text, $cl) = @_;
  return &StoreRaw("<span class=\"$cl\">$text</span>");
}

sub StoreLuaSyntax {
  my ($code) = @_;
  $code =~ s/((--)?\[(=*)\[.*?\]\3\])|((['"])(\\.|.)*?\5)|(--[^\n]*)/&StoreSpan($&, ($2 or $7)?"lua_comment":"lua_string")/gseo;
  $code =~ s/\b(and|break|do|else|elseif|end|false|for|function|if|in|local|nil|not|or|repeat|return|then|true|until|while)\b/&StoreSpan($1, "lua_keyword")/geo;
  $code =~ s/(?<!\.)\b((string|table|math|coroutine|io|os|package|debug)(\s*\.\s*\w+)?)\b/&StoreSpan($1, "lua_library")/geo;
  $code =~ s/(?<!\.)\b(assert|collectgarbage|dofile|error|gcinfo|getfenv|getmetatable|ipairs|loadfile|load|loadstring|module|next|newproxy|pairs|pcall|print|rawequal|rawget|rawset|require|select|setfenv|setmetatable|tonumber|tostring|type|unpack|xpcall|_G)\b/&StoreSpan($1, "lua_library")/geo;
  return &StoreRaw("<pre class=\"lua_code\">\n$code</pre>\n");
}

#-- Main program:
local $/;
local $txt = &QuoteHtml(<>);

# For Wiki & CommonMarkup: s/{{{!Lua\n(.*?\n)}}}/&StoreLuaSyntax($1)/gse;
$txt = &StoreLuaSyntax($txt);

$txt =~ s/$FS(\d+)$FS/$SaveUrl{$1}/geo;
$txt =~ s/$FS(\d+)$FS/$SaveUrl{$1}/geo;

#  #define LUATUTORIAL(luatut) <div class="registry_index"><a href="luatut.html">luatut</a></div> 

print <<EOF


<ReferencePage chapter="luafaudes" section="tutorials" page="$fhead" title="libFAUDES - luafaudes">

<h1>luafaudes Tutorial: 
$fhead
</h1>
<p>
To run the below Lua script, <tt>cd</tt> to the tutorial
section of the respective plug-in and enter
<tt>luafaudes 
$fhead
</tt>
at the command prompt. The script will read input data
from <tt>./tutorial/data/</tt>.
</p>
$txt

<p>&nbsp;</p>
<p>&nbsp;</p>

</ReferencePage>


EOF
