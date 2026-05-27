use proxy;
use std::collections::HashMap;
use std::io::Write;
use BufWriter;
use TcpStream;

pub fn send_static(url: &str, req: &Vec<String>, mut stream: BufWriter<TcpStream>) {
    proxy::forward(req.join("\r\n"), stream);
}
