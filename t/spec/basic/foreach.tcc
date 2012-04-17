===
--- code
for [1,3] -> $i {
    say($i);
}
say("OK");
--- stdout
1
3
OK
--- stderr

===
--- code
for [1,3] -> $k { say($k) }
--- stdout
1
3
--- stderr

===
--- code
for 1..3 -> $k { say($k) }
--- stdout
1
2
3
--- stderr

===
--- code
for 1..3 -> $k {  }
--- stdout
--- stderr

===
--- code
for (my $i=0; $i<10; $i++) {  }
--- stdout
--- stderr

===
--- code
my $k; my $v; for  {"A" => 3, "B" => 4} -> $k, $v { say($k); say($v); }
--- stdout
A
3
B
4
--- stderr

