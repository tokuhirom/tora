use strict;
use warnings;
use utf8;
use Test::More;
use t::Util;

run_is(<<'...', "hello\n");
say("hello")
__END__
say("YO");
...

done_testing;

