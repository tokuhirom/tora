package Module::Install::TestBase;
use strict;
use warnings;

use Module::Install::Base;

use vars qw($VERSION @ISA);
BEGIN {
    $VERSION = '0.60';
    @ISA     = 'Module::Install::Base';
}

sub use_test_base {
    my $self = shift;
    $self->include('Test::Base');
    $self->include('Test::Base::Filter');
    $self->include('Spiffy');
    $self->include('Test::More');
    $self->include('Test::Builder');
    $self->include('Test::Builder::Module');
    $self->requires('Filter::Util::Call');
}

1;

=encoding utf8

=head1 NAME

Module::Install::TestBase - Module::Install Support for Test::Base

=head1 SYNOPSIS

    use inc::Module::Install;

    name            'Foo';
    all_from        'lib/Foo.pm';

    use_test_base;

    WriteAll;

=head1 DESCRIPTION

This module adds the C<use_test_base> directive to Module::Install.

Now you can get full Test-Base support for you module with no external
dependency on Test::Base.

Just add this line to your Makefile.PL:

    use_test_base;

That's it. Really. Now Test::Base is bundled into your module, so that
it is no longer any burden on the person installing your module.

=head1 AUTHOR

Ingy döt Net <ingy@cpan.org>

=head1 COPYRIGHT

Copyright (c) 2006, 2008, 2011. Ingy döt Net.

This program is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.

See L<http://www.perl.com/perl/misc/Artistic.html>

=cut
