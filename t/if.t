use t::Util;

run_is(<<'...', "true\n");
if (true) {
    say(true);
} else {
    say(false);
}
...

run_is(<<'...', "false\n");
if (false) {
    say(true);
} else {
    say(false);
}
...

run_is(<<'...', "IF\n");
if (true) { say("IF"); }
elsif (false) { say("ELSIF") }
else { say("ELSE"); }
...

run_is(<<'...', "ELSIF\n");
if (false) { say("IF"); }
elsif (true) { say("ELSIF") }
else { say("ELSE"); }
...

run_is(<<'...', "ELSE\n");
if (false) { say("IF"); }
elsif (false) { say("ELSIF") }
else { say("ELSE"); }
...

done_testing;

