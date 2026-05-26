use err_codes;
use std::io::Write;
use BufWriter;
use TcpStream;

pub fn forward(mut stream: BufWriter<TcpStream>) {
    match stream.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}
