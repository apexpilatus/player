use err_codes;
use std::io::Write;
use TcpStream;

pub fn send_extracted(params: Option<&str>, range: &str, mut stream: TcpStream) {
    println!("{range}");
    match stream.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}
