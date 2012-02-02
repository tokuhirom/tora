use strict;
use warnings;
use utf8;
use Test::More;

for my $fname (glob('eg/t/*.tra')) {
    my $src = slurp($fname);
    my $result = `./tora $fname`;
    $src =~ s/.*\n__END__\n//;
    is($result, $src, $fname);
}

sub slurp {
    my $fname = shift;
    open my $fh, '<', $fname;
    do { local $/; <$fh> };
}

done_testing;

