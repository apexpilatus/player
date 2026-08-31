use err_codes;
use statics::data_map;
use std::io::Write;
use BufWriter;
use TcpStream;

pub fn send_static(url: &str, mut stream: BufWriter<TcpStream>) {
    let static_data = data_map();
    match static_data.get(url) {
        Some(resourse) => {
            let hdr = format!(
                "\
HTTP/1.1 200 OK\r\n\
X-Content-Type-Options: nosniff\r\n\
Cache-control: max-age=31536000, immutable\r\n\
Content-Type: {}\r\n\
Content-Length: {}\r\n\r\n",
                resourse.content_type,
                resourse.bytes.len()
            );
            match stream.write_all(hdr.as_bytes()) {
                Ok(_) => match stream.write_all(&resourse.bytes) {
                    _ => (),
                },
                Err(_) => return,
            }
        }
        None => match stream.write_all(err_codes::ERR_404.as_bytes()) {
            _ => (),
        },
    }
}
