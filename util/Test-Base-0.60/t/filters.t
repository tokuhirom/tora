use Test::Base;

filters 'upper';
plan tests => 2;

run {
    my $block = shift;
    is($block->one, $block->two);
};

my ($block) = blocks;
is($block->one, "HEY NOW HEY NOW\n");

sub Test::Base::Filter::upper {
    my $self = shift;
    return uc(shift);
}

__END__
===
--- one
Hey now Hey Now

--- two
hEY NoW hEY NoW
