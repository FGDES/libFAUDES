#!/usr/bin/perl

#-- adapted py2html Perl-script for libFAUDES documentation

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

#-- Functions for Python syntax highlighting:
sub StoreSpan {
  my ($text, $cl) = @_;
  return &StoreRaw("<span class=\"$cl\">$text</span>");
}

sub StorePythonSyntax {
  my ($code) = @_;
  my $keywords = qr/\b(False|None|True|and|as|assert|async|await|break|class|continue|def|del|elif|else|except|finally|for|from|global|if|import|in|is|lambda|nonlocal|not|or|pass|raise|return|try|while|with|yield)\b/x;
  $code =~ s/((#)?\[(#)\[.*?\]\3\])|((['"])(\\.|.)*?\5)|(#[^\n]*)/&StoreSpan($&, ($2 or $7)?"py_comment":"py_string")/gseo;
  #$code =~ s/(#.*)$/&StoreSpan($1, "py_comment")/gmoe;
  #$code =~ s/('(?:\\.|[^'])*')/&StoreSpan($1, "py_string")/gmeo;
  $code =~ s/$keywords/&StoreSpan($1, "py_keyword")/gseo;
  return &StoreRaw("<pre class=\"py_code\">\n$code</pre>\n");
}


#-- Main program:
local $/;
local $txt = &QuoteHtml(<>);

$txt = &StorePythonSyntax($txt);

$txt =~ s/$FS(\d+)$FS/$SaveUrl{$1}/geo;
$txt =~ s/$FS(\d+)$FS/$SaveUrl{$1}/geo;


print <<EOF


<ReferencePage chapter="pythonmod" section="tutorials" page="$fhead" title="libFAUDES - Python">

<h1>Tutorial: 
$fhead
</h1>
<p>
To run the below Python script, <tt>cd</tt> to the tutorial
section of the respective plug-in and enter
<tt>python3
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
