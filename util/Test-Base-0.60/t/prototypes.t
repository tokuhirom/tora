use Test::Base tests => 1;

is foo(), 'scalar_context', 'testing force scalar context';

sub foo {
    wantarray ? 'list_context' : 'scalar_context';
}
