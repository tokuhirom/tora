use lib 'lib';
use strict;
use warnings;
use Test::More tests => 2;

package Foo;
use Spiffy -base => -package => 'Bar';

package main;
ok(not defined &Foo::field);
ok(defined &Bar::field);
