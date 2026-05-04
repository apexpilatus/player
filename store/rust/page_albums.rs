use std::fs;
use std::io::Write;
use std::time::SystemTime;
use TcpStream;

struct Album {
    name: String,
    mtime: SystemTime,
}

pub fn send_albums(params: Option<&str>, mut stream: TcpStream) {
    let mut html = String::from(include_str!("../html/albums.html"));
    if let Some(into) = html.find("</body>") {
        if let Ok(dir) = fs::read_dir(env!("MUSIC_PATH")) {
            let mut albums: Vec<Album> = Vec::new();
            for album in dir {
                if let Ok(album) = album {
                    if let Ok(name) = album.file_name().into_string() {
                        if let Ok(meta) = album.metadata() {
                            if let Ok(mtime) = meta.modified() {
                                albums.push(Album { name, mtime });
                            }
                        }
                    }
                }
            }
            albums.sort_by_key(|alb| alb.mtime);
            if let Some(top) = albums.last() {
                let mut init = true;
                if let Some(params) = params {
                    for param in params.split("&") {
                        if param.starts_with("scroll=") {
                            init = false;
                            let mut script = String::from("<script>listalbums(\"");
                            script.push_str(&albums[0].name);
                            if albums.len() > 1 {
                                let albums = &albums[1..];
                                for album in albums {
                                    script.push(';');
                                    script.push_str(&album.name);
                                }
                            }
                            script.push_str("\")</script>");
                            html.insert_str(into, &script);
                        }
                    }
                }
                if init {
                    let script = format!("<script>gettracks(\"{}\")</script>", top.name);
                    html.insert_str(into, &script);
                }
            }
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
