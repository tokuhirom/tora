use lib 't', 'lib';
use strict;
use warnings;

package BOX;
use Spiffy -Base;

package main;
use Test::More tests => 3;

is(scalar <BOX::DATA>, "one\n");
is(scalar <BOX::DATA>, "two\n");
is(scalar <BOX::DATA>, "three\n");

sub foo {
    $self->foo;
}

package BOX;
__DATA__
one
two
three
