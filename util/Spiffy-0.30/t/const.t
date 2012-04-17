use lib 't', 'lib';
use strict;
use warnings;

package XXX;
use Spiffy -base;
const foo => 42;

package main;
use Test::More tests => 3;

my $xxx = XXX->new;
is($xxx->foo, 42);
is($xxx->foo(69), 42);
is($xxx->foo, 42);
