use err_codes;
use std::collections::HashMap;
use std::io::{Read, Write};
use BufWriter;
use TcpStream;

struct StaticData {
    content_type: String,
    bytes: Vec<u8>,
}

fn data_map() -> HashMap<String, StaticData> {
    let mut data = HashMap::new();
    data.insert(
        "/home.css".to_string(),
        StaticData {
            content_type: "text/css; charset=utf-8".to_string(),
            bytes: include_bytes!("../style/home.css").to_vec(),
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
        "/tracks.css".to_string(),
        StaticData {
            content_type: "text/css; charset=utf-8".to_string(),
            bytes: include_bytes!("../style/tracks.css").to_vec(),
        },
    );
    data.insert(
        "/tracks.js".to_string(),
        StaticData {
            content_type: "text/javascript; charset=utf-8".to_string(),
            bytes: include_bytes!("../js/tracks.js").to_vec(),
        },
    );
    data
}

fn forward(mut req: String, mut streamer: BufWriter<TcpStream>) {
    req.push_str("\r\n\r\n");
    match TcpStream::connect(env!("STORE_ADDR")) {
        Ok(mut store) => match store.write_all(req.as_bytes()) {
            Ok(_) => {
                let mut buf: Vec<u8> = vec![0; streamer.capacity()];
                loop {
                    match store.read(&mut buf) {
                        Ok(size) => {
                            if size == 0 {
                                break;
                            }
                            match streamer.write_all(&buf[..size]) {
                                Ok(_) => (),
                                Err(_) => break,
                            }
                        }
                        Err(_) => break,
                    }
                }
                return;
            }
            _ => (),
        },
        Err(_) => match streamer.write_all(err_codes::ERR_404.as_bytes()) {
            _ => (),
        },
    }
}

pub fn forward_if_no_static(url: &str, req: &Vec<String>, mut stream: BufWriter<TcpStream>) {
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
        None => forward(req.join("\r\n"), stream),
    }
}
