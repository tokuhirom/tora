use t::Util;

run_is(<<'...', "YO!\n");
sub foo($n) {
    say("YO!");
}

foo(3);
...

run_is(<<'...', "3\n");
sub foo($n) {
    say($n);
}

foo(3);
...

# no args.
run_is(<<'...', "a\n");
sub foo() {
    say("a");
}

foo();
...

# retval
run_is(<<'...', "123\n");
sub foo() {
    return 123;
}

say(foo());
...

# retval(undef)
run_is(<<'...', "undef\n");
sub foo() {
}

say(foo());
...

# retval(undef)
run_is(<<'...', "undef\n");
sub foo($n) {
}

say(foo(5));
...

# return multiple
run_is(<<'...', "1\n2\n3\n");
sub foo() {
    return 1,2,3;
}

say(foo());
...

# return multiple, and assign
run_is(<<'...', "3\n2\n1\n");
sub foo() {
    return 1,2,3;
}

my ($a, $b, $c) = foo();

say($c, $b, $a);
...

# return multiple, and assign to array
run_is(<<'...', "3\n2\n1\n");
sub foo() {
    return 1,2,3;
}

my $a = [];
($a[3], $a[4], $a[1]) = foo();

say($a);
...

done_testing;

