use lib 't', 'lib';
use strict;
use warnings;
use Spiffy ();

package Bar;

package Foo;
use base 'Spiffy';
sub new {
    my $self = super;
    field -package => 'Bar', 'xxx';
}

use Test::More tests => 4;

Foo->new;
ok(not defined $Foo::{-package});
ok(not defined &Foo::Bar);
ok(not defined &Foo::xxx);
ok(defined &Bar::xxx);
