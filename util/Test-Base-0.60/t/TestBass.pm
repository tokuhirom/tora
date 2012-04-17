package TestBass;
use Test::Base -Base;

# const block_class => 'TestBass::Block';
# const filter_class => 'TestBass::Filter';

our @EXPORT = qw(run_like_hell);

sub run_like_hell() { 
    (my ($self), @_) = find_my_self(@_);
    $self->run_like(@_);
}


package TestBass::Block;
use base 'Test::Base::Block';

sub el_nombre { $self->name(@_) }

block_accessor 'feedle';


package TestBass::Filter;
use base 'Test::Base::Filter';

sub foo_it {
    map {
        "foo - $_";
    } @_;
}
