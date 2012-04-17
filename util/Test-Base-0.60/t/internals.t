# Each filter should have access to blocks/block internals.
use Test::Base tests => 20 * 2;

run {};

package Test::Base::Filter;
use Test::More;

sub foo {
    my $self = shift;
    my $value = shift;
    
# Test access to Test::Base::Filter object.
    ok ref($self), 
       '$self is an object';
    is ref($self), 
       'Test::Base::Filter', 
       '$self is a Test:Base::Filter object';
    like $value,
         qr/^This is some .*text.\z/,
         'Filter value is correct';   

# Test access to Test::Base::Block object.
    my $block = $self->current_block;
    is ref($block), 
       'Test::Base::Block', 
       'Have a reference to our block object';

    ok not($block->is_filtered),
       'Block is not completely filtered yet';

    my $name = shift || 'One';
    is $block->name,
       $name,
       'name is correct';

    my $description = shift || 'One';
    is $block->description,
       $description,
       'description is correct';

    my $original = shift || "This is some text.";
    is $block->original_values->{xxx},
       $original,
       'Access to the original value';

    my $seq_num = shift || 1;
    cmp_ok $block->seq_num,
           '==',
           $seq_num,
           'Sequence number (seq_num) is correct';

    my $array_xxx = shift || ["This is some text."];
    is_deeply $block->{xxx},
              $array_xxx,
             'Test raw content of $block->{xxx}';

    my $method_xxx = shift || "This is some text.";
    is $block->xxx,
       $method_xxx,
       'Test method content of $block->xxx';

# Test access to Test::Base object.
    my $blocks = $block->blocks_object;
    my $block_list = $blocks->block_list;
    is ref($block_list), 
       'ARRAY',
       'Have an array of all blocks';

    is scalar(@$block_list), 
       '2',
       'Is there 2 blocks?';

    is $blocks->block_class,
       "Test::Base::Block",
       'block class';

    is $blocks->filter_class,
       "Test::Base::Filter",
       'filter class';

    is_deeply
       $blocks->{_filters},
       [qw(norm trim)],
       'default filters are ok';

    is $blocks->block_delim,
       '===',
       'block delimiter';

    is $blocks->data_delim,
       '---',
       'data delimiter';

    my $spec = <<END;
=== One
--- xxx foo: This is some text.
=== Two
This is the 2nd description.
Right here.

--- xxx chomp bar
This is some more text.

END
    is $blocks->spec,
       $spec,
       'spec is ok';

    is $block_list->[$seq_num - 1],
       $block,
       'test block ref in list';
}

sub bar {
    my $self = shift;
    my $value = shift;
    $self->foo($value,
        'Two',
        "This is the 2nd description.\nRight here.",
        "This is some more text.\n\n",
        2,
        ["This is some more text."],
        "This is some more text.",
    );
}

__END__
=== One
--- xxx foo: This is some text.
=== Two
This is the 2nd description.
Right here.

--- xxx chomp bar
This is some more text.

