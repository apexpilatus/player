use err_codes;
use std::io::Write;
use TcpStream;

pub fn send_extracted(params: Option<&str>, req: &Vec<String>, mut stream: TcpStream) {
    let mut range = String::new();
    for line in req {
        if line.to_lowercase().trim().starts_with("range") {
            range = line
                .split('=')
                .nth(1)
                .unwrap_or_default()
                .trim()
                .to_string();
        }
    }
    println!("{range}");
    match stream.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}
