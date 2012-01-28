use strict;
use warnings;
use utf8;
use Test::More;

run_is(<<'...', "YO!\n");
sub foo($n) {
    say("YO!")
}

foo()
...

done_testing;

