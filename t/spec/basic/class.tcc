===
--- code
class Hello {
    sub new() {
        self.bless(undef);
    }
    sub world ($n) {
        say("Hello world!: " +$n);
    }
}

my $h = Hello.new();
$h.world("HOGE");
--- stdout
Hello world!: HOGE
