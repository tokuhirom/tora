use Test::Base;

plan tests => 3 * blocks;

run_is perl => 'dumper';
run_is dumper => 'perl';
run_is dumper => 'dumper';

__DATA__
=== Dumper Test
--- perl eval dumper
[ 1..5, { 'a' .. 'p' }]
--- dumper
[
  1,
  2,
  3,
  4,
  5,
  {
    'a' => 'b',
    'c' => 'd',
    'e' => 'f',
    'g' => 'h',
    'i' => 'j',
    'k' => 'l',
    'm' => 'n',
    'o' => 'p'
  }
]
=== Another Dumper Test
--- perl eval dumper
"i like ike"
--- dumper
'i like ike'
