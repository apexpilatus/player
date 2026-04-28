use TcpStream;
use Write;

const ERR: &str = "HTTP/1.1 404 shit happens\r\n\
Cache-control: no-cache\r\n\
X-Content-Type-Options: nosniff\r\n\r\n";

pub fn send_static(mut stream: TcpStream) {
    println!("send static");
    match stream.write_all(ERR.as_bytes()) {
        _ => (),
    }
}
