use strict;
use warnings;
use utf8;
use Test::More;
use t::Util;

run_is(<<'...', "1\n3\nOK\n");
my $i;
for ($i in [1,3]) {
    say($i);
}
say("OK");
...

run_is('for (my $k in [1,3]) { say($k) }', "1\n3\n");

run_is('for (my $k in 1..3) { say($k) }', "1\n2\n3\n");

run_is('for (my $k in 1..3) {  }', "");
run_is('for (my $i=0; $i<10; $i++) {  }', "");

run_is('my $k; my $v; for (($k, $v) in {"A" => 3, "B" => 4}) { say($k); say($v); }', "A\n3\nB\n4\n");

done_testing;

