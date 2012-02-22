use strict;
use warnings;
use utf8;
use Test::More;
use t::Util;

run_is('my ($x, $v); say($x); say($v);', "undef\nundef\n", '');
run_is('(my $x, my $v); say($x); say($v);', "undef\nundef\n", '');
run_is('my ($x, $v) = (4, 9); say($x); say($v);', "4\n9\n", '');

done_testing;

