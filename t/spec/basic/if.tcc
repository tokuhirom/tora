===
--- code
if (true) {
    say(true);
} else {
    say(false);
}
--- stdout
true
--- stderr

===
--- code
if (false) {
    say(true);
} else {
    say(false);
}
--- stdout
false
--- stderr

===
--- code
if (true) { say("IF"); }
elsif (false) { say("ELSIF") }
else { say("ELSE"); }
--- stdout
IF
--- stderr

===
--- code
if (false) { say("IF"); }
elsif (true) { say("ELSIF") }
else { say("ELSE"); }
--- stdout
ELSIF
--- stderr

===
--- code
if (false) { say("IF"); }
elsif (false) { say("ELSIF") }
else { say("ELSE"); }
--- stdout
ELSE
--- stderr

===
--- code
my $x = 45;
if (my $x = 60) { say($x); }
say($x);
--- stdout
60
45
--- stderr

