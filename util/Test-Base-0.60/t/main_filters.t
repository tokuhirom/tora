use Test::Base tests => 6;

is next_block->xxx, "I lmike mike\n";
is next_block->xxx, "I like mikey";
is next_block->xxx, "123\n";
is next_block->xxx, "I like MIKEY";
is next_block->xxx, "I like ike\n";

run_is xxx => 'yyy';

sub mike1 {
    s/ike/mike/g;
};

sub mike2 {
    $_ = 'I like mikey';
    return 123;
};

sub mike3 {
    s/ike/heck/;
    return "123\n";
}

sub mike4 {
    $_ = 'I like MIKEY';
    return;
}

sub mike5 {
    return 200;
}

sub yyy { s/x/y/g }

__DATA__
===
--- xxx mike1
I like ike

===
--- xxx mike2
I like ike

===
--- xxx mike3
I like ike

===
--- xxx mike4
I like ike

===
--- xxx mike5
I like ike

===
--- xxx lines yyy
xxx xxx
  xxx xxx
--- yyy
yyy yyy
  yyy yyy
