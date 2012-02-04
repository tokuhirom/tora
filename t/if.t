use t::Util;

run_is(<<'...', "true\n");
if (true) {
    say(true);
} else {
    say(false);
}
...

run_is(<<'...', "false\n");
if (false) {
    say(true);
} else {
    say(false);
}
...

done_testing;

