use t::Util;

local $ENV{FOO} = 'hogehoge';
run_is(<<'...', $ENV{FOO});
print(getenv("FOO"))
...

done_testing;

