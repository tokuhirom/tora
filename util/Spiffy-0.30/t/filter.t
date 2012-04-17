use lib 't', 'lib';
use strict;
use warnings;

package XXX;
use Spiffy -Base; #, '-filter_dump';

const name => 'world';

sub foo {
   "Hello, " . $self->name;
}

sub bar() {
    my $self = shift;
    return $self;
}

package main;
use Test::More tests => 2;

my $xxx = XXX->new;
is($xxx->foo, 'Hello, world');
is(XXX::bar(42), 42);
