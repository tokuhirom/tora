use strict;
use warnings;
use utf8;
use Test::More;
use t::Util;

run_is(q{eval("say(3+2)");}, "5\n");
run_is(q{say(eval("3+2"));}, "5\n");

done_testing;

