#!/usr/bin/perl
use strict;
use warnings;
use 5.14.0;
use Cwd;
use FindBin;
use lib (
    "$FindBin::Bin/../util/",
    "FindBin::Bin/../util/Test-Simple-0.98/lib/",
    "FindBin::Bin/../util/Test-Harness-3.23/lib/"
);
use App::Prove;

my $prove = App::Prove->new();
$prove->process_args(qw(
    --source TCC
    --source Tora
    --source Executable
    -r tests/), <t/tra/*.tra>, <t/tra/*/*.tra>, <t/spec/*/*.tcc>,
    qw(--source Perl t
));
$prove->run;

