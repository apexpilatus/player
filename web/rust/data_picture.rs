use err_codes;
use std::io::Write;
use TcpStream;

pub fn send_picture(path: Option<&str>, mut stream: TcpStream) {
    if let Some(path) = path {
        println!("{path}");
        match stream.write_all(err_codes::ERR_404.as_bytes()) {
            _ => (),
        }
    }
    match stream.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}
