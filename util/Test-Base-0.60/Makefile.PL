use inc::Module::Install;

name        'Test-Base';
all_from    'lib/Test/Base.pm';
readme_from;
manifest_skip;
version_check;
ack_xxx;

requires    'Spiffy' => '0.30';
requires    'Test::More' => '0.62';
requires    'Filter::Util::Call' => '0';

recommends 'Test::Deep';

clean_files 't/output';

WriteAll;

