#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use 5.010000;
use autodie;

my $max = @ARGV > 0 ? $ARGV[0] : 30000000;
say($max);
my $foo;
for (1..$max) {
    $foo = $_
}
say($foo);

