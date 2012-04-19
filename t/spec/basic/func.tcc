===
--- code
sub foo($n) {
    say("YO!");
}

foo(3);
--- stdout
YO!

===
--- code
sub foo($n) {
    say($n);
}

foo(3);
--- stdout
3

=== no args
--- code
sub foo() {
    say("a");
}

foo();
--- stdout
a

=== retval
--- code
sub foo() {
    return 123;
}

say(foo());
--- stdout
123

=== retval(undef)
--- code
sub foo() {
}

say(foo());
--- stdout
undef

=== retval(undef)
--- code
sub foo($n) {
}

say(foo(5));
--- stdout
undef

=== return multiple
--- code
sub foo() {
    return 1,2,3;
}

say(foo());
--- stdout
1
2
3

=== return multiple, and assign
--- code
sub foo() {
    return 1,2,3;
}

my ($a, $b, $c) = foo();

say($c, $b, $a);
--- stdout
3
2
1

=== return multiple, and assign to array
--- code
sub foo() {
    return 1,2,3;
}

my $a = [];
($a[3], $a[4], $a[1]) = foo();

for (my $i=0; $i<$a.size(); ++$i) {
    say($a[$i]);
}
--- stdout
undef
3
undef
1
2

