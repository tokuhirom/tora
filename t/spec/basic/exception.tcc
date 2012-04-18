===
--- code
my ($e, $x) = try { return 3; };
say($e);
say($x);
--- stdout
undef
3

===
--- code
my ($e, $x) = try { die 4; return 3; };
say($e);
say($x);
--- stdout
4
undef

===
--- code
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
--- stdout
REACH HERE
OK!
undef

===
--- code
my $x = "hoge";
say($x/"fuga");
--- exit_status: 1
--- stdout
--- stderr
'hoge' is not numeric. You cannot divide. line 2

===
--- code
foo();
--- exit_status: 1
--- stdout
--- stderr
Unknown function 'foo' line 2.

