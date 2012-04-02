use strict;
use warnings;
use utf8;
use Test::More;
use t::Util;

run_is(<<'...', "hogehoge %  3 HOOO\n");
printf("hogehoge %%  %d %s", 3, "HOOO");
...

done_testing;

