use Test::Base tests => 8;

my $test = Test::Base->new;

my @blocks = $test->filters('chomp')->spec_file('t/spec1')->blocks;

is $blocks[0]->foo, '42'; 
is $blocks[0]->bar, '44'; 
is $blocks[1]->xxx, '123'; 
is $blocks[1]->yyy, '321'; 

@blocks = Test::Base->new->delimiters('^^^', '###')->blocks;

is $blocks[0]->foo, "42\n"; 
is $blocks[0]->bar, "44\n"; 
is $blocks[1]->xxx, "123\n"; 
is $blocks[1]->yyy, "321\n"; 

__END__
^^^ Test one

### foo
42

### bar
44

^^^ Test two

### xxx
123
### yyy
321
