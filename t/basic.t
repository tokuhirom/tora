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

run_is(<<'...', "yo\nho\n");
say("yo")
say("ho")
...

run_is(<<'...', "true\nfalse\n");
say(true)
say(false)
...

run_is('', "");

run_is(<<'...', "OK\nHERE\n");
if (true) { say("OK") }
say("HERE")
...

run_is(<<'...', "HERE\n");
if (false) { say("NOT OK") }
say("HERE")
...

done_testing;

