use t::Util;

run_is(<<'...', "YO!\n");
sub foo($n) {
    say("YO!")
}

foo(3)
...

run_is(<<'...', "3\n");
sub foo($n) {
    say($n)
}

foo(3)
...

done_testing;

