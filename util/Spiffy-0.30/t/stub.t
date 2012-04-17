use lib 't', 'lib';
use strict;
use warnings;

package XXX;
use Spiffy -base;
stub 'foo';

package YYY;
use base 'XXX';

package main;
use Test::More tests => 1;

my $y = YYY->new;
eval {$y->foo};
like($@, qr/subclassed/);
