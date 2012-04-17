use Test::Base;

__DATA__
===
--- xxx) eval Reverse array
[qw(a b c)],
[qw(d e f)],
[qw(g h i j)]
--- yyy) eval
[
[qw(c b a)],
[qw(f e d)],
[qw(j i h g)]
]

===
--- xxx) eval Reverse array
[
    [qw(a b c)],
    [qw(d e f)],
    [qw(g h i j)]
], 
[
    [qw(a b c)],
    [qw(d e f)],
    [qw(g h i j)]
], 

--- yyy) eval
[
    [
        [qw(c b a)],
        [qw(f e d)],
        [qw(j i h g)]
    ],
    [
        [qw(c b a)],
        [qw(f e d)],
        [qw(j i h g)]
    ]
]

