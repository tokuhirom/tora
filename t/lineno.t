use strict;
use warnings;
use utf8;
use Test::More;
use t::Util;

run_is(<<'...', "1\n2\n3\n");
say(__LINE__);
say(__LINE__);
say(__LINE__);
...

done_testing;

