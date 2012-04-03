use strict;
use warnings;
use utf8;
use Test::More;
use t::Util;

run_is(<<'...', "1\n2\n3\n");
my ($a, $b, $c) = *[1,2,3];
say($a);
say($b);
say($c);
...

done_testing;

