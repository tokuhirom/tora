use strict;
use warnings;
use utf8;
use Test::More;
use t::Util;

run_is(<<'...', "Hello world!: HOGE\n");
class Hello {
    sub new() {
        self.bless(undef);
    }
    sub world ($n) {
        say("Hello world!: " +$n);
    }
}

my $h = Hello.new();
$h.world("HOGE");
...

done_testing;

