use t::Util;

run_tora_is(<<'...', "true\n");
if (true) {
    say(true);
} else {
    say(false);
}
...

run_tora_is(<<'...', "false\n");
if (false) {
    say(true);
} else {
    say(false);
}
...

run_tora_is(<<'...', "IF\n");
if (true) { say("IF"); }
elsif (false) { say("ELSIF") }
else { say("ELSE"); }
...

run_tora_is(<<'...', "ELSIF\n");
if (false) { say("IF"); }
elsif (true) { say("ELSIF") }
else { say("ELSE"); }
...

run_tora_is(<<'...', "ELSE\n");
if (false) { say("IF"); }
elsif (false) { say("ELSIF") }
else { say("ELSE"); }
...

run_tora_is(<<'...', "60\n45\n");
my $x = 45;
if (my $x = 60) { say($x); }
say($x);
...

done_testing;

