===
--- code
say(1+2)
--- stdout
3

===
--- code
say(1+2);say("OK")
--- stdout
3
OK

===
--- code
sub foo() {
    say("OK")
}
foo()
--- stdout
OK

===
--- code
sub foo() {
    say(1+2);
    say("OK")
}
foo()
--- stdout
3
OK

