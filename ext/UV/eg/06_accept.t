use UV *;
use Socket *;
use Test::More *;

my $tcp = UV::TCP.new();
my $addr = Socket.sockaddr_in(8888, Socket.inet_aton('localhost'));
$tcp.bind($addr);
$tcp.listen(10, -> $status {
    my $client = $tcp.accept();
    $client.read(-> $buf {
        say($buf);
        UV.default_loop().unref();
    });
});
UV.run();

ok(1);

done_testing;

