use t::Util;

local $ENV{FOO} = 'hogehoge';
local $ENV{HOGE};
run_is(<<'...', "$ENV{FOO}\nundef\n");
say($ENV["FOO"]);
say($ENV["HOGE"]);
...

run_is(<<'...', "HOGE\nundef\n");
$ENV["HEE"] = "HOGE";
say($ENV["HEE"]);
$ENV["HEE"] = undef;
say($ENV["HEE"]);
...

done_testing;

