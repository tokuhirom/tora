use Test::Base tests => 18;

for my $word (qw(
                 BEGIN
                 DESTROY
                 EXPORT
                 ISA
                 block_accessor
                 blocks_object
                 description
                 is_filtered
                 name
                 new
                 run_filters
                 seq_num
                 set_value
             )) {
    my $blocks = my_blocks($word);
    eval {$blocks->blocks};
    like $@, qr{'$word' is a reserved name}, 
         "$word is a bad name";
}

for my $word (qw(
                 field
                 const
                 stub
                 super
             )) {
    my $blocks = my_blocks($word);
    my @blocks = $blocks->blocks;
    eval {$blocks->blocks};
    is "$@", '',
       "$word is a good name";
}

sub my_blocks {
    my $word = shift;
    Test::Base->new->spec_string(<<"...");
=== Fail test
--- $word
This is a test
--- foo
This is a test
...
}

my $blocks = Test::Base->new->spec_string(<<'...');
=== Fail test
--- bar
This is a test
--- foo
This is a test
...
eval {$blocks->blocks};
is "$@", '';
