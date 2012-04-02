use t::Util;

run_is(<<'...', "undef\n3\n");
my ($e, $x) = try { return 3; };
say($e);
say($x);
...

run_is(<<'...', "4\nundef\n");
my ($e, $x) = try { die 4; return 3; };
say($e);
say($x);
...

run_is(<<'...', "REACH HERE\nOK!\nundef\n");
sub foo() {
    say("REACH HERE");
    die "OK!";
    say("NOT REACH HERE");
}
my ($e, $x) = try {
    foo();
    say("SHOULD NOT REACH HERE");
};
say($e);
say($x);
...

run_is(<<'...', '', qq{'hoge' is not numeric. You cannot divide. line 2\n}, 1);
my $x = "hoge";
say($x/"fuga");
...

run_is(<<'...', '', qq{Unknown function 'foo' in package 'main' line 2.\n}, 1);
foo();
...

done_testing;

