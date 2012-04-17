use Test::Base tests => 5;

my $block1 = [blocks]->[0];
my @values = $block1->grocery;
is scalar(@values), 3, 
   'check list context';
is_deeply \@values, ['apples', 'oranges', 'beef jerky'], 
   'list context content';

my $block2 = [blocks]->[1];
is_deeply $block2->todo, 
[
    'Fix YAML', 
    'Fix Inline', 
    'Fix Test::Base',
], 'deep block from index';

my $block3 = [blocks]->[2];
is $block3->perl, 'xxx',
   'scalar context';
is_deeply [$block3->perl], ['xxx', 'yyy', 'zzz'],
   'deep list compare';

__END__

=== One
--- grocery lines chomp
apples
oranges
beef jerky

=== Two
--- todo lines chomp array
Fix YAML
Fix Inline
Fix Test::Base

=== Three
--- perl eval
return qw(
    xxx
    yyy
    zzz
)
