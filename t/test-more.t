use strict;
use warnings;
use utf8;
use Test::More;
use t::Util;

run_is(<<'...', "ok 1\nnot ok 2\n1..2\n");
require("Test::More");
ok(1);
ok(false);
done_testing();
...

done_testing;

