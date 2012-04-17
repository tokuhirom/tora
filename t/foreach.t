use strict;
use warnings;
use utf8;
use Test::More;
use t::Util;

run_tora_is(<<'...', "1\n3\nOK\n");
for [1,3] -> $i {
    say($i);
}
say("OK");
...

run_tora_is('for [1,3] -> $k { say($k) }', "1\n3\n");

run_tora_is('for 1..3 -> $k { say($k) }', "1\n2\n3\n");

run_tora_is('for 1..3 -> $k {  }', "");
run_tora_is('for (my $i=0; $i<10; $i++) {  }', "");

run_tora_is('my $k; my $v; for  {"A" => 3, "B" => 4} -> $k, $v { say($k); say($v); }', "A\n3\nB\n4\n");

done_testing;

