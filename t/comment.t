use t::Util;

run_tora_is(<<'...', "Hello, world\n");
say("Hello, world");
...

run_tora_is(<<'...', "Hello, world\n");
/* comment
comment2 */
say("Hello, world");
...

run_tora_is(<<'...', "Hello, world\n");
#! comment
say("Hello, world");
...

done_testing;

