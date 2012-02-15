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

done_testing;

