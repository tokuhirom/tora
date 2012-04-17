===
--- code
say("ho" + "ge");
--- stdout
hoge
--- stderr

===
--- code
say("ho" + 3);
--- stdout
ho3
--- stderr

===
--- code
say("3" + 4);
--- stdout
34
--- stderr

===
--- code
say(3 + '4');
--- stdout
7
--- stderr

