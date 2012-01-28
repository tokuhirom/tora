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

# no args.
run_is(<<'...', "a\n");
sub foo() {
    say("a")
}

foo()
...

done_testing;
