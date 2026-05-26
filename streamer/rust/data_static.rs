use proxy;
use std::collections::HashMap;
use std::io::Write;
use BufWriter;
use TcpStream;

pub fn send_static(url: &str, mut stream: BufWriter<TcpStream>) {
    proxy::forward(stream);
}
