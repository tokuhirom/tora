use t::Util;

run_is(<<'...', "Hello, world\n");
// comment
say("Hello, world")
...

run_is(<<'...', "Hello, world\n");
/* comment
comment2 */
say("Hello, world")
...

run_is(<<'...', "Hello, world\n");
#! comment
say("Hello, world")
...

done_testing;

