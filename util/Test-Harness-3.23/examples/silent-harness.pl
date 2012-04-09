#!/usr/bin/perl
#
# Run some tests and get back a data structure describing them.

use strict;
use warnings;
use TAP::Harness;
use Data::Dumper;

my @tests = glob 't/yaml*.t';

my $harness = TAP::Harness->new( { verbosity => -9, lib => ['blib/lib'] } );

# $aggregate is a TAP::Parser::Aggregator
my $aggregate = $harness->runtests(@tests);
print Dumper($aggregate);
