use t::Util;

run_tora_is(<<'...', "hoge\n");
say("ho" + "ge");
...

done_testing;

