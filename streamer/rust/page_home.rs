use std::io::Write;
use BufWriter;
use TcpStream;

pub fn send_home(params: Option<&str>, mut stream: BufWriter<TcpStream>) {
    let mut html = String::from(include_str!("../html/home.html"));
    let hdr = format!(
        "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=utf-8\r\n\
Content-Length: {}\r\n\r\n",
        html.len()
    );
    match stream.write_all(hdr.as_bytes()) {
        Ok(_) => match stream.write_all(html.as_bytes()) {
            _ => (),
        },
        _ => (),
    }
}
