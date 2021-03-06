use strict;
use warnings;
use utf8;
use Test::More;

for my $fname (glob('eg/t/*.tra')) {
    my $src = slurp($fname);
    my $result = `./bin/tora $fname`;
    if (my ($expected) = ($src =~ /\n__END__\n(.+)/s)) {
        is($result, $expected, $fname);
    } else {
        die "Bad test case: $fname: $src";
    }
}

sub slurp {
    my $fname = shift;
    open my $fh, '<', $fname;
    do { local $/; <$fh> };
}

done_testing;

