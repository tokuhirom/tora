use lib 't', 'lib';
use strict;
use warnings;
use Spiffy ();

package A;
use Spiffy -Base;

sub AUTOLOAD {
    super;
    join '+', $A::AUTOLOAD, @_;
}

package B;
use base 'A';

sub AUTOLOAD {
    super;
}

package C;
use base 'B';

sub AUTOLOAD {
    super;
}

package main;
use Test::More tests => 1;

is(C->foo(42), 'C::foo+42');
