use proxy;
use std::collections::HashMap;
use std::io::Write;
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
    data
}

pub fn send_static(url: &str, req: &Vec<String>, mut stream: BufWriter<TcpStream>) {
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
        None => proxy::forward(req.join("\r\n"), stream),
    }
}
