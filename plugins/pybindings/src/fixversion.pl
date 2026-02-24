#!/bin/perl
# pragmatic perl scipt to fic libFAUDES version numbers

use strict;
use warnings;

my ($fversion) = @ARGV;

if (not defined $fversion) {
  die "fixversion: need version\n";
}

my $relalph    = substr($fversion, -1);
my $relnum = ord($relalph) - ord("a");
my $mmversion=substr($fversion, 0, length($fversion)-1);
print "$mmversion.$relnum\n";
