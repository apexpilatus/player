use err_codes;
use std::io::{Read, Write};
use std::process::{ChildStdin, Command, Stdio};
use BufWriter;
use TcpStream;

struct Params {
    album: String,
    track: u32,
}

fn parse_params(params: &str) -> Option<Params> {
    let mut album = None;
    let mut track = None;
    for param in params.split("&") {
        if param.starts_with("album=") {
            album = param.split("=").nth(1);
        }
        if param.starts_with("track=") {
            track = param.split("=").nth(1);
        }
    }
    match album {
        Some(album) => match track {
            Some(track) => match track.parse::<u32>() {
                Ok(track) => Some(Params {
                    album: album.to_string(),
                    track,
                }),
                Err(_) => Some(Params {
                    album: album.to_string(),
                    track: 1,
                }),
            },
            None => Some(Params {
                album: album.to_string(),
                track: 1,
            }),
        },
        None => None,
    }
}

fn get_hdr(mut store: &TcpStream) -> Option<String> {
    let mut buf: Vec<u8> = vec![0; 4096];
    let mut size: usize = 0;
    while size < buf.len() {
        match store.read_exact(&mut buf[size..size + 1]) {
            Ok(_) => {
                size += 1;
                if size > 3 {
                    if let Ok(hdr) = String::from_utf8(buf[..size].to_vec()) {
                        if hdr.ends_with("\r\n\r\n") {
                            break;
                        }
                    }
                }
            }
            Err(_) => return None,
        }
    }
    if size == buf.len() {
        None
    } else {
        match String::from_utf8(buf[..size].to_vec()) {
            Ok(ret) => Some(ret),
            Err(_) => None,
        }
    }
}

fn get_tracks(mut params: Params, mut stdin: &ChildStdin) {
    'get_tracks: loop {
        let req = format!(
            "GET /fetch?album={}&track={} HTTP/1.1\r\n\r\n",
            params.album, params.track
        );
        match TcpStream::connect(env!("STORE_ADDR")) {
            Ok(mut store) => match store.write_all(req.as_bytes()) {
                Ok(_) => {
                    let mut length = String::from("-1\n");
                    if let Some(hdr) = get_hdr(&store) {
                        if hdr.contains("404 shit happens") {
                            match stdin.write_all(length.as_bytes()) {
                                _ => (),
                            }
                            break;
                        } else {
                            for line in hdr.split("\r\n") {
                                if line.to_lowercase().trim().starts_with("content-length") {
                                    length =
                                        line.split(":").nth(1).unwrap_or("-1").trim().to_string()
                                            + "\n";
                                }
                            }
                        }
                    } else {
                        break;
                    }
                    //let mut reader = BufReader::new(store);
                    let mut buf: Vec<u8> = vec![0; /*reader.capacity()*/2048];
                    match stdin.write_all(length.as_bytes()) {
                        Ok(_) => (),
                        Err(_) => {
                            break;
                        }
                    }
                    loop {
                        match store.read(&mut buf) {
                            Ok(size) => {
                                if size == 0 {
                                    break;
                                }
                                match stdin.write_all(&buf[..size]) {
                                    Ok(_) => (),
                                    Err(_) => {
                                        break 'get_tracks;
                                    }
                                }
                            }
                            Err(_) => {
                                break 'get_tracks;
                            }
                        }
                    }
                }
                Err(_) => {
                    break;
                }
            },
            Err(_) => {
                break;
            }
        }
        params.track += 1;
    }
}

pub fn play(params: Option<&str>, mut streamer: BufWriter<TcpStream>) {
    if let Some(params) = params {
        let params = parse_params(params);
        if let Some(params) = params {
            if let Ok(mut child) = Command::new("play")
                .env("PATH", env!("STREAMER_PATH"))
                .stdin(Stdio::piped())
                .stderr(Stdio::null())
                .stdout(Stdio::null())
                .current_dir(env!("STREAMER_PATH"))
                .spawn()
            {
                let resp = format!(
                    "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=utf-8\r\n\
Cache-control: no-cache\r\n\
X-Content-Type-Options: nosniff\r\n\r\n"
                );
                match streamer.write_all(resp.as_bytes()) {
                    Ok(_) => match streamer.flush() {
                        Ok(_) => {
                            if let Some(ref mut stdin) = child.stdin {
                                get_tracks(params, stdin);
                            }
                        }
                        Err(_) => (),
                    },
                    Err(_) => (),
                }
                match child.wait() {
                    _ => {
                        return;
                    }
                }
            }
        }
    }
    match streamer.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}
