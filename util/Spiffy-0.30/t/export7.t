use Test::More;

plan tests => 4;

package B;
use Spiffy -Base, -XXX;

package A;
use Spiffy -Base;

package main;

ok(not defined &A::XXX);
ok(defined &A::field);

ok(defined &B::XXX);
ok(defined &B::field);
