use std::io::Write;
use BufWriter;
use TcpStream;

pub fn send_tracks(params: Option<&str>, mut stream: BufWriter<TcpStream>) {
    let mut html = String::from(include_str!("../html/tracks.html"));
    let mut album = None;
    if let Some(params) = params {
        for param in params.split("&") {
            if param.starts_with("album=") {
                album = param.split("=").nth(1);
            }
        }
    }
    if let Some(into) = html.find("</body>") {
        if let Some(album) = album {
            let script = format!("<script>getmeta(\"{album}\")</script>");
            html.insert_str(into, &script);
        }
    }
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
