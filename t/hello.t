use t::Util;

run_is(<<'...', "Hello, world\n");
say("Hello, world")
...

run_is(<<'...', "Hello\nworld\n");
say("Hello", "world")
...

done_testing;

