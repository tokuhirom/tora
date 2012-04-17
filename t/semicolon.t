use strict;
use warnings;
use utf8;
use Test::More;
use t::Util;

run_tora_is(<<'...', "3\n");
say(1+2)
...

run_tora_is(<<'...', "3\nOK\n");
say(1+2);say("OK")
...

run_tora_is(<<'...', "OK\n");
sub foo() {
    say("OK")
}
foo()
...

run_tora_is(<<'...', "3\nOK\n");
sub foo() {
    say(1+2);
    say("OK")
}
foo()
...

done_testing;

