use lib 'lib';

package Foo;
use strict;
use Spiffy -base;
field 'xxx';
field 'dog';
field 'bog';

sub new {
    my $self = super;
    $self->xxx('XXX');
    return $self;
}

sub poodle {
    my $self = shift;
    my $count = shift;
    $self->dog("$count poodle");
}

sub doodle {
    my $self = shift;
    my $count = shift;
    $self->bog("$count doodle");
}

package Bar;
use strict;
BEGIN { Foo->base }

sub poodle {
    my $self = shift;
    super;
    $self->dog($self->dog . ' dogs');
}

sub doodle {
    my $self = shift;
    eval 'eval "super"';
    $self->bog($self->bog . ' bogs');
}

package main;
use strict;
use Test::More tests => 3;

my $f = Bar->new;
is($f->{xxx}, 'XXX');

$f->poodle(3);
is($f->{dog}, '3 poodle dogs');

$f->doodle(4);
is($f->{bog}, '4 doodle bogs');
