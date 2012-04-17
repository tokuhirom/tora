use lib 't', 'lib';
use strict;
use warnings;

package A;
use Spiffy -Base, ':XXX';

package B;
use Spiffy -Base, ':XXX', 'field';

package main;
use Test::More tests => 4;
ok(not defined &A::field);
ok(defined &B::field);
ok(defined &A::XXX);
ok(defined &B::XXX);
