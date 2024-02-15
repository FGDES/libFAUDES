#!/usr/bin/perl

#-- build fref file to host generator
#-- input: generator file with .gen extension
#-- (.png and .svg files are expected to exist with the same basename)
#-- output: fref file on stdout

#-- Html quote for generator tokens

sub QuoteHtml {
  my ($html) = @_;
  $html =~ s/&/&amp;/g; $html =~ s/</&lt;/g; $html =~ s/>/&gt;/g;
  return $html;
}

#-- Main program:
local $/;
local $gentoken = &QuoteHtml(<>);

#-- file name
use File::Basename;
$genfile= basename($ARGV,".gen");

print <<EOF
<ReferencePage chapter="images" section="none" title="Example Data">
<h1>
Example Data: $genfile.gen
</h1>

<h3>
Dot-processed graph as SVG-image
</h3>

<object type="image/svg+xml" name="graph" data="$genfile.svg">
<a class="faudes_image" href="$genfile.svg">
<img src="$genfile.png" title="Click on image to download SVG formated file." />
</a>
</object>


<h3>
Token IO 
</h3>
<pre>
$gentoken
</pre>

<p>&nbsp;</p>
<p>&nbsp;</p>

</ReferencePage>

EOF
