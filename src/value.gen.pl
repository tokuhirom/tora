#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use 5.010000;
use autodie;

my %a = (
    VALUE_TYPE_UNDEF => 'UndefValue',
    VALUE_TYPE_INT => 'IntValue',
    VALUE_TYPE_BOOL => 'BoolValue',
    VALUE_TYPE_STR => 'StrValue',
    VALUE_TYPE_CODE => 'CodeValue',
    VALUE_TYPE_ARRAY => 'ArrayValue',
    VALUE_TYPE_DOUBLE => 'DoubleValue',
    VALUE_TYPE_REGEXP => 'AbstractRegexpValue',
    VALUE_TYPE_TUPLE => 'TupleValue',
    VALUE_TYPE_FILE => 'FileValue',
    VALUE_TYPE_RANGE => 'RangeValue',
    VALUE_TYPE_ARRAY_ITERATOR => 'ArrayValue::iterator',
    VALUE_TYPE_RANGE_ITERATOR => 'RangeValue::iterator',
    VALUE_TYPE_EXCEPTION => 'ExceptionValue',
    VALUE_TYPE_SYMBOL => 'SymbolValue',
    VALUE_TYPE_HASH => 'HashValue',
    VALUE_TYPE_HASH_ITERATOR => 'HashValue::iterator',
    VALUE_TYPE_OBJECT => 'ObjectValue',
    VALUE_TYPE_POINTER => 'PointerValue',
);

my $ret = '';
$ret .= qq{#include "value.h"\n};
$ret .= qq{#include "package.h"\n};
$ret .= qq{#include "package_map.h"\n};
$ret .= qq{#include "value/range.h"\n};
$ret .= qq{#include "value/symbol.h"\n};
$ret .= qq{#include "value/pointer.h"\n};
$ret .= qq{#include "value/regexp.h"\n};
$ret .= qq{#include "value/hash.h"\n};
$ret .= qq{#include "value/array.h"\n};
$ret .= qq{#include "value/object.h"\n};
$ret .= qq{#include "value/code.h"\n};
$ret .= qq{#include "value/file.h"\n};
$ret .= qq{#include "value/tuple.h"\n};
$ret .= qq{#include "vm.h"\n};
$ret .= "using namespace tora;\n";
$ret .= "const char *tora::Value::type_str() {\n";
$ret .= "    switch (value_type) {\n";
while (my ($k, $v) = each %a) {
$ret .= "    case $k: return this->upcast<${v}>()->type_str();\n";
}
$ret .= "    }\n";
$ret .= "    abort();\n";
$ret .= "}\n";
open my $fh, '>', 'src/value.gen.cc';
print $fh $ret;
close $fh;

