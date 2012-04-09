#!perl -w

use strict;
use lib 't/lib';

use Test::More;

# TODO skip on install?
eval "use Test::Pod::Coverage 1.04";
plan skip_all => "Test::Pod::Coverage 1.04 required for testing POD coverage"
  if $@;

# this isn't perfect, but it's close enough
my @deprecated = qw(
  actual_passed
  good_plan
  passed
);

local $^W;    # we want it to ignore 'Test::Builder::failure_output redefined'
all_pod_coverage_ok( { trustme => \@deprecated } );
