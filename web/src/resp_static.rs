use TcpStream;
use Write;

const ERR: &str = "HTTP/1.1 404 shit happens\r\n\
Cache-control: no-cache\r\n\
X-Content-Type-Options: nosniff\r\n\r\n";

pub fn send_static(url: &str, mut stream: TcpStream) {
    let static_data = [(
        "/favicon.ico",
        "image/x-icon",
        include_bytes!("../static/favicon.ico"),
    )];
    for resourse in static_data {
        if resourse.0.eq(url) {
            let hdr = format!(
                "\
HTTP/1.1 200 OK\r\n\
X-Content-Type-Options: nosniff\r\n\
Cache-control: max-age=31536000, immutable\r\n\
Content-Type: {}\r\n\
Content-Length: {}\r\n\r\n",
                resourse.1,
                resourse.2.len()
            );
            match stream.write_all(hdr.as_bytes()) {
                Ok(_) => match stream.write_all(resourse.2) {
                    _ => return,
                },
                Err(_) => return,
            }
        }
    }
    match stream.write_all(ERR.as_bytes()) {
        _ => return,
    }
}
