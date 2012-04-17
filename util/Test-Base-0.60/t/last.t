use Test::Base tests => 4;

is scalar(blocks), 3, 
   'Does LAST limit tests to 3?';

run {
    is(shift()->test, 'all work and no play');
}

__DATA__
===
--- test: all work and no play

===
--- test: all work and no play

=== 
--- LAST
--- test: all work and no play

===
--- test: all work and no play

===
--- test: all work and no play

===
--- test: all work and no play

===
--- test: all work and no play


