use err_codes;
use std::collections::HashMap;
use std::io::Write;
use BufWriter;
use TcpStream;

pub fn send_static(url: &str, mut stream: BufWriter<TcpStream>) {
    match stream.write_all(err_codes::ERR_404.as_bytes()) {
            _ => (),
        }
}
