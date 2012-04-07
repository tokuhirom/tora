use UV;

my $tick = 0;

my $idle = UV::Idle.new;
$idle.start(sub() {
    if $tick++ % 5000 == 0 {
        print(".");
    }
});

my $timer = UV::Timer.new;
$timer.start(sub() {
    print("|");
}, 1000, 1000);

UV.run();
