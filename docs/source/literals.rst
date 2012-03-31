Literals
========

Some of the literals are available. Most of literals are same as Perl5.

Integer Literal
---------------

Int Object.

    0
    1

Double Literal
--------------

Double object.

    3.14

Double quote String Literal
---------------------------

String object.

    "foo\r\nbar"
    qq{foo\r\nbar}
    qq[foo\r\nbar]
    qq(foo\r\nbar)

Single quote String Literal
---------------------------

String object.

    'foo\r\nbar'
    q{foo\r\nbar}
    q[foo\r\nbar]
    q(foo\r\nbar)
    q!foo\r\nbar!

Double quote Bytes string literal
---------------------------------

Bytes object.

    b"hogehoge"

Single quote Bytes string literal
---------------------------------

Bytes object.

    b'hogehoge'

Array Literal
-------------

Array object. You can contain any type of values in array.

    []
    [1,2,3,'hoge']

Hash Literal
------------

Hash object. In tora, hash object is ordered. Keys are string.

    {foo => 'bar', baz => 'iyan'}

Regexp Literal
--------------

Regexp object.

    /hogehoge/
    /hogehoge/smi

    qr(hogehoge)smi
    qr/hogehoge/smi
    qr,hogehoge,smi
    qr[hogehoge]smi
    qr!hogehoge!smi

Quoted word literal
-------------------

Array[Str] object.

    qw(hoge fuga)
    qw[hoge fuga]
    qw{hoge fuga}
    qw!hoge fuga!

Following two lines are same semantics.

    qw(hoge fuga)
    ['hoge', 'fuga']

Boolean literal
---------------

Bool object.

    false
    true

