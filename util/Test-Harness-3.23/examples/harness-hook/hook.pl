#!/usr/bin/perl

use strict;
use warnings;
use lib qw( lib ../../lib );
use Harness::Hook;
use TAP::Harness;
use File::Spec;

$| = 1;

my $harness = TAP::Harness->new;

# Install the hook
Harness::Hook->new($harness);

$harness->runtests(
    File::Spec->catfile( split( /\//, '../../t/000-load.t' ) ) );
