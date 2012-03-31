class Socket
=============

This is a low level socket class.
 

Socket.socket(Int domain, Int type, Int protocol);
Socket.socket(Int domain, Int type);
 

Socket.sockaddr_in(port, value)
Return sockaddr struct in bytes.
 

Socket.inet_aton($ip);
Socket.inet_aton($hostname);
This function calls inet_aton(3) or gethostbyname(3).
 

$socket.read() : Bytes
$socket.read(Int $len) : Bytes
Read $len bytes from the $socket.
 

$sock.bind(Str $addr) : Undef
 

$socket.accept()
 

