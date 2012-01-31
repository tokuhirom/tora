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

# retval
run_is(<<'...', "123\n");
sub foo() {
    return 123
}

say(foo())
...

# retval(undef)
run_is(<<'...', "undef\n");
sub foo() {
}

say(foo())
...

# retval(undef)
run_is(<<'...', "undef\n");
sub foo($n) {
}

say(foo(5))
...

done_testing;

