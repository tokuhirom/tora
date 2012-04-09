#!/usr/bin/perl -w

use strict;
use lib 't/lib';

use Test::More tests => 1;

ok -t STDIN, 'STDIN remains a TTY';
