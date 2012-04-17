use Test::Base tests => 2;

filters {
    in => [qw(eval_all array)],
    out => 'eval',
};

run_is_deeply in => 'out';

__DATA__
===
--- (in)
print "hi";
warn "hello\n";
print "bye";
print STDERR "baby";
die "darn\n";
--- (out)
[undef, "darn\n", "hibye", "hello\nbaby"]

===
--- (in)
[1..3];
--- (out)
[[1,2,3], '', '', '']
