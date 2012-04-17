package Filter4;
use Spiffy -Base;
# comment

sub foo {
    my $x = $self->$bar;
}

sub one { }
sub uno {}
my sub bar {
    if (1) {
        my $y = 1;
    }
}

sub baz() {
    my $z = 1;
}

my sub bam {
    $self->$bar(42);
}
