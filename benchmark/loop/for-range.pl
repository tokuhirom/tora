#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use 5.010000;
use autodie;

my $foo;
for (1..30000000) {
    $foo = $_
}
say($foo);

