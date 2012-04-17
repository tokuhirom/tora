use Test::Base tests => 10;

run {};

my $count = 0;
sub test {
    my $num = shift;
    chomp $num;
    is $num, ++$count;
    return;
}

__END__
=== One
--- grape test
1
--- iceberg_lettuce test
2
--- fig test
3
--- eggplant test
4
--- jalepeno test
5
--- banana test
6
--- apple test
7
--- carrot test
8
--- hot_pepper test
9
--- date test
10
