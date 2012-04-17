use Test::Base;

__DATA__
===
--- (xxx) eval Join
[
    [qw(a b c)],
    [qw(d e f)],
]

--- (yyy) eval
[ qw(abc def) ]

===
--- (xxx) eval Join=x
[
    [
        [qw(a b c)],
        [qw(d e f)],
    ],
    [
        [qw(a b c)],
        [qw(d e f)],
    ]
]

--- (yyy) eval
[
    [ qw(axbxc dxexf) ],
    [ qw(axbxc dxexf) ],
]
