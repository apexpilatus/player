use err_codes;
use std::collections::HashMap;
use std::io::Write;
use TcpStream;

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
            bytes: include_bytes!("../pictures/favicon.ico").to_vec(),
        },
    );
    data.insert(
        "/apple-touch-icon.png".to_string(),
        StaticData {
            content_type: "image/png".to_string(),
            bytes: include_bytes!("../pictures/favicon180.png").to_vec(),
        },
    );
    data.insert(
        "/apple-touch-icon-precomposed.png".to_string(),
        StaticData {
            content_type: "image/png".to_string(),
            bytes: include_bytes!("../pictures/favicon180.png").to_vec(),
        },
    );
    data.insert(
        "/apple-touch-icon-120x120-precomposed.png".to_string(),
        StaticData {
            content_type: "image/png".to_string(),
            bytes: include_bytes!("../pictures/favicon120.png").to_vec(),
        },
    );
    data.insert(
        "/home.css".to_string(),
        StaticData {
            content_type: "text/css; charset=utf-8".to_string(),
            bytes: include_bytes!("../style/home.css").to_vec(),
        },
    );
    data.insert(
        "/albums.css".to_string(),
        StaticData {
            content_type: "text/css; charset=utf-8".to_string(),
            bytes: include_bytes!("../style/albums.css").to_vec(),
        },
    );
    data.insert(
        "/home.js".to_string(),
        StaticData {
            content_type: "text/javascript; charset=utf-8".to_string(),
            bytes: include_bytes!("../js/home.js").to_vec(),
        },
    );
    data.insert(
        "/albums.js".to_string(),
        StaticData {
            content_type: "text/javascript; charset=utf-8".to_string(),
            bytes: include_bytes!("../js/albums.js").to_vec(),
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
                Err(_) => return,
            }
        }
        None => match stream.write_all(err_codes::ERR_404.as_bytes()) {
            _ => (),
        },
    }
}
