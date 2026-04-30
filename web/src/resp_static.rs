use std::collections::HashMap;
use std::io::Write;
use TcpStream;

const ERR: &str = "HTTP/1.1 404 shit happens\r\n\
Cache-control: no-cache\r\n\
X-Content-Type-Options: nosniff\r\n\r\n";

struct StaticData {
    content_type: String,
    bytes: Vec<u8>,
}

fn data_map() -> HashMap<String, StaticData> {
    let mut data = HashMap::new();
    data.insert(
        "/favicon.ico".to_string(),
        StaticData {
            content_type: "image/x-icon".to_string(),
            bytes: include_bytes!("../static/favicon.ico").to_vec(),
        },
    );
    data.insert(
        "/apple-touch-icon.png".to_string(),
        StaticData {
            content_type: "image/png".to_string(),
            bytes: include_bytes!("../static/favicon180.png").to_vec(),
        },
    );
    data.insert(
        "/style_main.css".to_string(),
        StaticData {
            content_type: "text/css; charset=utf-8".to_string(),
            bytes: include_bytes!("../static/style_main.css").to_vec(),
        },
    );
    data
}

pub fn send_static(url: &str, mut stream: TcpStream) {
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
                Err(_) => (),
            }
        }
        None => match stream.write_all(ERR.as_bytes()) {
            _ => (),
        },
    }
}
