use strict;
use warnings;
use utf8;
use Test::More;
use t::Util;

run_is('say(do("eg/3.tra"));', "3\n");

done_testing;

