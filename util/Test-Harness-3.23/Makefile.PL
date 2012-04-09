#!perl

require 5.006;

# This Makefile.PL is provided for installation compatibility.
# Extra developer actions are in the Build.PL.

use ExtUtils::MakeMaker qw/WriteMakefile prompt/;

use strict;

my %mm_args = (
    'NAME'         => 'Test::Harness',
    'VERSION_FROM' => 'lib/Test/Harness.pm',
    'INSTALLDIRS'  => 'perl',
    'PL_FILES'     => {},
    'test'         => { 'TESTS' => 't/*.t t/compat/*.t' },

   # The core autogenerates a Makefile.PL, and finds prove with utils/prove.PL
    'EXE_FILES' => ['bin/prove'],
);

{
    local $^W = 0;    # Silence warning about non-numeric version
    if ( $ExtUtils::MakeMaker::VERSION >= '6.31' ) {
        $mm_args{LICENSE} = 'perl';
    }
}

WriteMakefile(%mm_args);

package MY;

# Lifted from MM_Any.pm and modified so that make test tests against our
# own code rather than the incumbent. If we don't do this we end up
# loading a confused mixture of installed and new modules.
sub test_via_harness {
    my ( $self, $perl, $tests ) = @_;

    return $self->SUPER::test_via_harness(
        qq{$perl "-I\$(INST_LIB)" "-I\$(INST_ARCHLIB)"}, $tests );
}

BEGIN {
    my %deny = (
        manifest => 'dist_basics',
        dist     => 'dist_core',
    );
    while ( my ( $verb, $override ) = each %deny ) {
        my $super = "SUPER::$override";
        no strict 'refs';
        *{"MY::$override"} = sub {
            my ( $self, @args ) = @_;
            my $frag  = $self->$super(@args);
            my $chunk = split_makefile_chunk($frag);
            replace_rule(
                $chunk, $verb,
                ":\n\t\$(NOECHO) \$(ECHO) "
                  . "\"Please use 'Build.PL $verb' instead of 'Makefile.PL $verb'\"\n\n"
            );
            return join_makefile_chunk($chunk);
        };
    }
}

# Returns a reference to a hash containing
#  targets   a reference to an array of makefile section names
#  sections  a reference to a hash mapping makefile section names to the
#            text of those sections.

sub split_makefile_chunk {
    my $chunk    = shift;
    my $target   = ' prefix';
    my @targets  = ();
    my %sections = ();
    for my $ln ( split /\n/, $chunk ) {
        if ( $ln =~ /^(\S+)/ ) {
            $target = $1;
            push @targets, $target;
        }
        $sections{$target} .= "$ln\n";
    }
    return { targets => \@targets, sections => \%sections };
}

sub join_makefile_chunk {
    my $chunk = shift;
    return join '', grep defined,
      map { $chunk->{sections}{$_} } @{ $chunk->{targets} };
}

sub replace_rule {
    my ( $chunk, $name, $body ) = @_;
    $chunk->{sections}{$name} = "$name $body";
}
