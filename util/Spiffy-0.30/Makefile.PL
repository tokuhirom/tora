use inc::Module::Install;

name        'Spiffy';
all_from    'lib/Spiffy.pm';

requires    perl => '5.6.1';
requires    Filter::Util::Call => '0';
requires    Scalar::Util => '0';

WriteAll;
