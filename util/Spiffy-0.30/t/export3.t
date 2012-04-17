use lib 't';
use strict;
use warnings;

package A;
use Spiffy -base;
BEGIN {@A::EXPORT_OK = qw($A1 $A2)}
$A::A1 = 5;
$A::A2 = 10;

package B;
use base 'A';
BEGIN {@B::EXPORT_OK = qw($A2 $A3)}
$B::A2 = 15;
$B::A3 = 20;

package main;
no warnings;
use Test::More tests => 7;
BEGIN {B->import(qw($A1 $A2 $A3 $A4))}
ok(defined $main::A1);
ok(defined $main::A2);
ok(defined $main::A3);
ok(not defined $main::A4);
is($A1, 5);
is($A2, 10);
is($A3, 20);
