use lib 't', 'lib';
use strict;
use warnings;

package A;
use Spiffy -base;
BEGIN {@A::EXPORT_OK = qw(dude)}
const dude => 10;

package B;
use base 'A';
BEGIN {
    @B::EXPORT_OK = qw(dude);
    const dude => 20;
}

package C;
BEGIN {B->import('dude')}

package main;
no warnings;
use Test::More tests => 2;
ok(defined $C::{dude});
is(C::dude(), 20);
