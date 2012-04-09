package Harness::Hook;

use strict;
use warnings;
use Carp;

sub new {
    my ( $class, $harness ) = @_;
    my $self = bless {}, $class;

    $harness->callback(
        'before_runtests',
        sub {
            my ($aggregate) = @_;
            warn "Before runtests\n";
        }
    );

    $harness->callback(
        'after_runtests',
        sub {
            my ( $aggregate, $results ) = @_;
            warn "After runtests\n";
        }
    );

    return $self;
}

1;
