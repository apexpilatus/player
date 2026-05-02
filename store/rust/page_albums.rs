use std::fs;
use std::io::Write;
use std::time::SystemTime;
use TcpStream;

struct Album {
    name: String,
    mtime: SystemTime,
}

pub fn send_albums(mut stream: TcpStream) {
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
            albums.sort_by(|alb1, alb2| alb1.mtime.cmp(&alb2.mtime));
            for album in albums {
                let album = format!("<img src=\"picture?album={}\" alt=\"picture\">", album.name);
                html.insert_str(into, &album);
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
