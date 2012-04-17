use Test::Base;

plan tests => 1 * blocks() + 1;

for (1..blocks) {
    ok 1, 'Jusk checking my blocking';
}

is scalar(blocks), 2, 
   'correct number of blocks';

sub this_filter_fails {
    confess "Should never get here";
}

__DATA__
this
===
--- foo this_filter_fails
xxxx

===
--- foo this_filter_fails
yyyy
