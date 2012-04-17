use Test::Base tests => 4;

run_is_deeply in => 'out';

filters_delay;

my ($b3, $b4) = blocks('bad');

eval { $b3->run_filters };
like "$@", qr"Input to the 'flatten' filter must be a scalar";

eval { $b4->run_filters };
like "$@", qr"Can only flatten a hash or array ref";

__END__
===
--- in eval flatten array
{
    one => 'won',
    two => 'too',
    three => 'thrice',
}
--- out lines chomp array
one
won
three
thrice
two
too

===
--- in eval flatten array
[qw(one two three four)]
--- out lines chomp array
one
two
three
four

===
--- bad lines flatten
one
two

===
--- bad flatten: foo bar baz
