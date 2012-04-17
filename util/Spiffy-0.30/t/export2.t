use lib 't';
use strict;
use warnings;
package A;
use Spiffy -base;
BEGIN {@A::EXPORT = qw($A1 $A2)}
$A::A1 = 5;
$A::A2 = 10;

package B;
use base 'A';
BEGIN {@B::EXPORT = qw($A2 $A3)}
$B::A2 = 15;
$B::A3 = 20;

package main;
use strict;
use Test::More tests => 6;
BEGIN {B->import}
ok(defined $main::A1);
ok(defined $main::A2);
ok(defined $main::A3);
is($main::A1, 5);
is($main::A2, 15);
is($main::A3, 20);
