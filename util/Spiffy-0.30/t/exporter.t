package Foo;
use Spiffy -base;

package autouse;

use Test::More tests => 1;

is 'Foo'->can('import'), \&Exporter::import,
    'Spiffy modules support autouse';
