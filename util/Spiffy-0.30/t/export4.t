use lib 't';
use strict;
use warnings;

package A;
# Exporter before 5.8.4 needs the tag as the first thing imported
use Spiffy -base, qw(:XXX const);

package B;
use base 'A';

package C;
use Spiffy -XXX, -base;

package D;
use Spiffy -base;

package E;
use Spiffy -base, 'XXX';

package F;
use Spiffy -base;
use Spiffy 'XXX';

package main;
use Test::More tests => 24;

ok(not defined &A::field);
ok(defined &A::const);
ok(defined &A::XXX);
ok(defined &A::YYY);

ok(defined &B::field);
ok(defined &B::const);
ok(not defined &B::XXX);
ok(not defined &B::YYY);

ok(defined &C::field);
ok(defined &C::const);
ok(defined &C::XXX);
ok(defined &C::YYY);

ok(defined &D::field);
ok(defined &D::const);
ok(not defined &D::XXX);
ok(not defined &D::YYY);

ok(not defined &E::field);
ok(not defined &E::const);
ok(defined &E::XXX);
ok(not defined &E::YYY);

ok(defined &F::field);
ok(defined &F::const);
ok(defined &F::XXX);
ok(not defined &F::YYY);
