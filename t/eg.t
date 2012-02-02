use strict;
use warnings;
use utf8;
use Test::More;

for my $fname (glob('eg/t/*.tra')) {
    my $src = slurp($fname);
    my $result = `./tora $fname`;
    if (my ($expected) = ($src =~ /\n__END__\n(.+)/s)) {
        is($result, $expected, $fname);
    } else {
        die "Bad test case: $src";
    }
}

sub slurp {
    my $fname = shift;
    open my $fh, '<', $fname;
    do { local $/; <$fh> };
}

done_testing;

