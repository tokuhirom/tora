use Test::Base tests => 2;

filters {
    perl => ['eval', 'bang'],
    value => 'chomp',
    perl2 => 'eval',
    dummy => 'uuu',
};

run_is perl => 'value';
run_is dummy => 'perl2';

sub bang {
    return defined($_) ? ':-(' : '!!!';
}

sub uuu {
    undef($_);
    return undef;
}

__DATA__


=== No warnings for sending undef to filter
--- perl
undef
--- value
!!!

=== No warnings returning undef from filter
--- dummy
--- perl2
undef
