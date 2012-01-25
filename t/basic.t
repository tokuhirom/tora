use t::Util;

run_is(<<'...', "3\n");
say(1+2)
...

run_is(<<'...', "5\n");
say(7-2)
...

run_is(<<'...', "14\n");
say(7*2)
...

run_is(<<'...', "2\n");
say(4/2)
...

=pod
run_is(<<'...', "OK\n");
if (true) {
    say("OK")
}
...
=cut

done_testing;

