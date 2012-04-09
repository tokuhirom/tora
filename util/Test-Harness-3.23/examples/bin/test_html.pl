#!/usr/bin/perl

use strict;
use warnings;

use Test::More tests => 1;
use Test::WWW::Mechanize;

my $mech = Test::WWW::Mechanize->new;
my $url  = shift;
$mech->get_ok(
    $url,
    "We should be able to fetch ($url)"
);
