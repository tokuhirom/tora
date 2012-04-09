#!perl

require 5.006;

use strict;
use Module::Build;

use File::Spec;
use lib 'inc', File::Spec->canonpath('lib/');    # use our self to install

use MyBuilder;

my $builder = MyBuilder->new(
    module_name => 'Test::Harness',
    license     => 'perl',
    installdirs => 'core',
    meta_merge  => {
        resources => {
            homepage => 'http://testanything.org/',
            bugtracker =>
              'http://rt.cpan.org/Public/Dist/Display.html?Name=Test-Harness',
            MailingList => 'mailto:<tapx-dev@hexten.net>',
            repository =>
              'http://github.com/AndyA/Test-Harness/tree/master',
        },
        keywords => [ 'TAP', 'test harness', 'prove' ],
    },
    recursive_test_files => 1,
    recommends           => {
        'Pod::Usage' => '1.12',    # for 'prove'
    },
    build_requires => {},
    add_to_cleanup => ['Test-Harness-*'],
    script_files   => ["bin/prove"],
    ( $Module::Build::VERSION >= 0.34 ) ? ( auto_configure_requires => 0 ) : (),
);

$builder->create_build_script();
