use strict;
use warnings;
use utf8;
use Test::More;
use t::Util;

run_is('print(true ? 1 : 2);',  "1");
run_is('print(false ? 1 : 2);', "2");

done_testing;

