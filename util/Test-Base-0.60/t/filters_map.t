use Test::Base tests => 7;

eval {
    filters_map {
        perl => ['eval'],
        text => ['chomp', 'lines', 'array'],
    };
};
like $@, qr{Can't locate object method "filters_map"};

filters {
    perl => ['eval'],
    text => ['chomp', 'lines', 'array'],
};

run {
    my $block = shift;
    is ref($block->perl), 'ARRAY';
    is ref($block->text), 'ARRAY';
    is_deeply $block->perl, $block->text;
};

__DATA__
=== One
--- perl
[
    "One\n",
    "2nd line\n",
    "\n",
    "Third time's a charm",
]
--- text
One
2nd line

Third time's a charm
=== Two
--- text
tic tac toe
--- perl
[ 'tic tac toe' ]

