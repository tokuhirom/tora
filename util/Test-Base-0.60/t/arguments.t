use Test::Base tests => 3;

run {};

sub Test::Base::Filter::something {
    my $self = shift;
    my $value = shift;
    my $arguments = $self->current_arguments;
    is $value, 
       "candle\n", 
       'value is ok';
    is $arguments, 
       "wicked", 
       'arguments is ok';
    is $Test::Base::Filter::arguments, 
       "wicked", 
       '$arguments global variable is ok';
}

__END__
=== One
--- foo something=wicked
candle
