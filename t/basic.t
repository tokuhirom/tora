use t::Util;

run_is(<<'...', "3\n");
say(1+2);
...

run_is(<<'...', "5\n");
say(7-2);
...

run_is(<<'...', "14\n");
say(7*2);
...

run_is(<<'...', "2\n");
say(4/2);
...

run_is(<<'...', "yo\nho\n");
say("yo");
say("ho");
...

run_is(<<'...', "true\nfalse\n");
say(true);
say(false);
...

run_is('', "");

run_is(<<'...', "OK\nHERE\n");
if (true) {
    say("OK");
}
say("HERE");
...

run_is(<<'...', "HERE\n");
if (false) {
    say("NOT OK");
}
say("HERE");
...

run_is(<<'...', "true\nfalse\n");
say(1==1);
say(1==2);
...

my $src = <<'...';
say(4<5);
say(4<1);
say(4<4);

say(4>5);
say(4>1);
say(4>4);

say(4>=5);
say(4>=1);
say(4>=4);

say(4<=5);
say(4<=1);
say(4<=4);
...
run_is($src , join("\n", qw(
    true false false
    false true false
    false true true
    true false true
)). "\n");

run_is(<<'...', "5\n");
my $x=5;
say($x);
...

run_is(<<'...', "7\n");
my $x=3;
my $y=4;
say($x+$y);
...

run_is(<<'...', "5\n");
my $x=2;
$x=$x+3;
say($x);
...

run_is(<<'...', "0\n1\n2\n");
my $i=0;
while ($i<3) {
    say($i);
    $i = $i + 1;
}
...

run_is(<<'...', "0\n1\n2\n");
my $i=0;
while ($i<3) {
    say($i);
    ++$i;
}
...

TODO: {
    local $TODO = 'oops';
    run_is(<<'...', "OK\nHERE\n");
if (true) { say("OK"); }
say("HERE");
...
}

done_testing;

