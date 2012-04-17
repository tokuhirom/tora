use Test::Base tests => 2;

filters qw(chomp +bar foo);

is next_block->text, "this,foo,that,bar";
# 2nd test is needed
is next_block->text, "this,foo,that,bar";

sub foo { $_[0] . ",foo" } 
sub bar { $_[0] . ",bar" } 
sub that { $_[0] . ",that" } 

__DATA__
===
--- text that
this
===
--- text that
this
