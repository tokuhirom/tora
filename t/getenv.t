use t::Util;

local $ENV{FOO} = 'hogehoge';
local $ENV{HOGE};
run_is(<<'...', "$ENV{FOO}\nundef\n");
say($ENV["FOO"]);
say($ENV["HOGE"]);
...

done_testing;

