use Test::Base;

__END__
=== Test deep sorting
--- (a) eval Sort
[
[
[qw(c d b a)], [qw(foo bar baz)],
]
]
--- (b) eval Reverse
[
[
[qw(d c b a)], [qw(foo baz bar)],
]
]
