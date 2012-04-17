use lib 't';
use strict;
use warnings;
package A;
use Spiffy -base;
field 'x';
field 'y';

package main;
use Test::More tests => 6;

my $a1 = A->new;
ok(not defined $a1->x);
ok(not defined $a1->y);
my $a2 = A->new(x => 5);
is($a2->x, 5);
ok(not defined $a2->y);
my $a3 = A->new(x => 15, y => 10);
is($a3->x, 15);
is($a3->y, 10);
