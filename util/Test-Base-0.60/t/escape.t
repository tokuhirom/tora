use Test::Base tests => 2;

is next_block->escaped, 
   "line1\nline2";
is next_block->escaped,
   "	foo\n		bar\n";

__END__

===
--- escaped escape chomp
line1\nline2
===
--- escaped escape
\tfoo
\t\tbar

