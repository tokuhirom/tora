use Test::Base tests => 2;

my $b = next_block;
is $b->ok, "I am ok. Are you ok?";

$b = next_block;
is_deeply [$b->words], [qw(foo bar baz)];

__DATA__

=== Split a string of lines into words
--- ok split join=\s
I am
ok. Are you
ok?

=== Split on a string
--- words split=x: fooxbarxbaz
--- LAST
The other tests don't work yet.

===
--- ok lines split
I am 
ok. Are you
ok?


===
--- test lines Split Reverse Join reverse join=\n
I Like Ike
Give Peace A Chance
Love Is The Answer
--- flip
Answer The Is Love
Chance A Peace Give
Ike Like I
