use t::Util;

run_is(<<'...', $ENV{HOME});
print(getenv("HOME"))
...

done_testing;

