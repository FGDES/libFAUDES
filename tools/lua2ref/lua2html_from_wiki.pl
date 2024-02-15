lua2html converter, Perl-script, as found on 

http://lua-users.org/lists/lua-l/2006-03/msg00661.html

================================================================


* Subject: Re: test directory
* From: Mike Pall <mikelu-0603@...>
* Date: Thu, 23 Mar 2006 13:52:53 +0100

Hi,

I wrote:
> But it may be easier to modify the current Wiki.

Yup, it is. The Wiki engine used for lua-users.org is pretty
simple and contained in one big Perl file. Adding full support
for Lua 5.1 syntax highlighting was easy (comments, strings,
keywords, standard library functions).

The attached stand-alone lua2html.pl script may be useful for
other purposes, too. Sorry for posting Perl code here, but it was
the appropriate tool for the job. Code donated to the public
domain (without any warranties and so on).

One needs to add the StoreSpan and the StoreLuaSyntax functions
to the Wiki engine and then call them in CommonMarkup (before the
regular {{{...}}} markup). The embedded stylesheet needs to be
added to the Wiki stylesheet, too.

Then you can use:

{{{!Lua
io.write("Hello world!\n")
}}}

... and get nicely highlighted Lua code in the Wiki.

A link to download the raw text of the snippet would be a bonus.
But, as has been pointed out, cut'n'paste may be sufficient.

Provided this is added to the lua-users Wiki, I'm offering to add
all example code I have and keep an eye on the related section
(or extra tree?).

Bye,
     Mike

================================================================

#!/usr/bin/perl

#-- Minimal infrastructure, copied from NoNameWiki.
%SaveUrl = ();
$SaveUrlIndex = 0;
$FS = "\x01";

sub QuoteHtml {
  my ($html) = @_;
  $html =~ s/$FS//go;
  $html =~ s/&/&amp;/g; $html =~ s/</&lt;/g; $html =~ s/>/&gt;/g;
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
  $code =~ s/((--)?\[(=*)\[.*?\]\3\])|((['"])(\\.|.)*?\5)|(--[^\n]*)/&StoreSpan($&, ($2 or $7)?"comment":"string")/gseo;
  $code =~ s/\b(and|break|do|else|elseif|end|false|for|function|if|in|local|nil|not|or|repeat|return|then|true|until|while)\b/&StoreSpan($1, "keyword")/geo;
  $code =~ s/(?<!\.)\b((string|table|math|coroutine|io|os|package|debug)(\s*\.\s*\w+)?)\b/&StoreSpan($1, "library")/geo;
  $code =~ s/(?<!\.)\b(assert|collectgarbage|dofile|error|gcinfo|getfenv|getmetatable|ipairs|loadfile|load|loadstring|module|next|newproxy|pairs|pcall|print|rawequal|rawget|rawset|require|select|setfenv|setmetatable|tonumber|tostring|type|unpack|xpcall|_G)\b/&StoreSpan($1, "library")/geo;
  return &StoreRaw("<pre class=\"code\">\n$code</pre>\n");
}

#-- Main program:
local $/;
local $txt = &QuoteHtml(<>);

# For Wiki &CommonMarkup: s/{{{!Lua\n(.*?\n)}}}/&StoreLuaSyntax($1)/gse;
$txt = &StoreLuaSyntax($txt);

$txt =~ s/$FS(\d+)$FS/$SaveUrl{$1}/geo;
$txt =~ s/$FS(\d+)$FS/$SaveUrl{$1}/geo;

print <<EOF
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/
DTD/xhtml11.dtd">
<html>
<style type="text/css">
background { background: #ffffff; color: #000000; }
pre.code {
  font-size: 10pt;
  font-family: Courier New, Courier, monospace;
  /* Uncomment this for the Wiki stylesheet:
  background: #f8f8f8;
  color: #000000;
  border: 1px solid black;
  padding: 0.5em;
  margin-left: 2em;
  */
}
span.comment { color: #00a000; }
span.string { color: #0000c0; }
span.keyword { color: #a00000; font-weight: bold; }
span.library { color: #a000a0; }
</style>
<body>
$txt
</body>
</html>
EOF


