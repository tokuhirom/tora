===
--- code
say(1+2);
--- stdout
3
--- stderr

===
--- code
say(7-2);
--- stdout
5
--- stderr

===
--- code
print("HELLO\t\n");
--- stdout
HELLO	
--- stderr

===
--- code
say(7*2);
--- stdout
14
--- stderr

===
--- code
say(4/2);
--- stdout
2
--- stderr

===
--- code
say("yo");
say("ho");
--- stdout
yo
ho
--- stderr

===
--- code
say(true);
say(false);
--- stdout
true
false
--- stderr

===
--- code
--- stdout
--- stderr

===
--- code
if (true) {
    say("OK");
}
say("HERE");
--- stdout
OK
HERE
--- stderr

===
--- code
if (false) {
    say("NOT OK");
}
say("HERE");
--- stdout
HERE
--- stderr

===
--- code
say(1==1);
say(1==2);
--- stdout
true
false
--- stderr

===
--- code
say(4<5);
say(4<1);
say(4<4);

say(4>5);
say(4>1);
say(4>4);

say(4>=5);
say(4>=1);
say(4>=4);

say(4<=5);
say(4<=1);
say(4<=4);
--- stdout
true
false
false
false
true
false
false
true
true
true
false
true
--- stderr

===
--- code
my $x=5;
say($x);
--- stdout
5
--- stderr

===
--- code
my $x=3;
my $y=4;
say($x+$y);
--- stdout
7
--- stderr

===
--- code
my $x=2;
$x=$x+3;
say($x);
--- stdout
5
--- stderr

===
--- code
my $i=0;
while ($i<3) {
    say($i);
    $i = $i + 1;
}
--- stdout
0
1
2
--- stderr

===
--- code
my $x = 10;
$x/= 2;
say($x);
--- stdout
5
--- stderr

===
--- code
my $i=0;
while ($i<3) {
    say($i);
    ++$i;
}
--- stdout
0
1
2
--- stderr

===
--- code
say(0..10);
--- stdout
0..10
--- stderr

===
--- code
my $a;
$a = 1;
say($a);
--- stdout
1
--- stderr

===
--- code
if (true) { say("OK"); }
say("HERE");
--- stdout
OK
HERE
--- stderr

===
--- code
say(typeof("hoge"));
say(typeof({}));
--- stdout
Str
Hash
--- stderr

===
--- code
say(1+"3");
--- stdout
4
--- stderr

