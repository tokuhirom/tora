use UV *;
use Socket *;
use Test::More *;

my $tcp = UV::TCP.new();
my $addr = Socket.sockaddr_in(8888, Socket.inet_aton('localhost'));
$tcp.bind($addr);

my $callback = -> $status {
    my $client = $tcp.accept();
    my $line = b'';
    $client.read(-> $buf {
        if $buf {
            $line += $buf;
            if $buf.substr($buf.length-1) == "\n" {
                say($line);
		        $client.write($line, undef);
                $line = b'';
            }
        }
    });
};
$tcp.listen(10, $callback);
UV.run();

ok(1);

done_testing;

