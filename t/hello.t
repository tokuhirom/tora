use t::Util;

run_tora_is(<<'...', "Hello, world\n");
say("Hello, world");
...

run_tora_is(<<'...', "Hello\nworld\n");
say("Hello", "world");
...

done_testing;

