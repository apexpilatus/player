use err_codes;
use std::io::{Read, Write};
use std::process::{Command, Stdio};
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

pub fn play(params: Option<&str>, mut streamer: BufWriter<TcpStream>) {
    if let Some(params) = params {
        let params = parse_params(params);
        if let Some(mut params) = params {
            if let Ok(mut child) = Command::new("play")
                .env("PATH", env!("STREAMER_PATH"))
                .stdin(Stdio::piped())
                //.stderr(Stdio::null())
                //.stdout(Stdio::null())
                .current_dir(env!("STREAMER_PATH"))
                .spawn()
            {
                let hdr = format!(
                    "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=utf-8\r\n\
Cache-control: no-cache\r\n\
X-Content-Type-Options: nosniff\r\n\r\n"
                );
                match streamer.write_all(hdr.as_bytes()) {
                    Ok(_) => {
                        if let Some(ref mut stdin) = child.stdin {
                            let mut buf: Vec<u8> = vec![0; streamer.capacity()];
                            'get_tracks: loop {
                                let req = format!(
                                    "GET /fetch?album={}&track={} HTTP/1.1\r\n\r\n",
                                    params.album, params.track
                                );
                                if let Ok(mut store) = TcpStream::connect(env!("STORE_ADDR")) {
                                    println!("new req {}", req);
                                    match store.write_all(req.as_bytes()) {
                                        Ok(_) => loop {
                                            match store.read(&mut buf) {
                                                Ok(size) => {
                                                    if size == 0 {
                                                        println!("read 0 from store");
                                                        break;
                                                    }
                                                    match stdin.write_all(&buf[..size]) {
                                                        Ok(_) => (),
                                                        Err(_) => {
                                                            println!("err write all to stdin");
                                                            break 'get_tracks;
                                                        }
                                                    }
                                                }
                                                Err(_) => {
                                                    println!("err read from store");
                                                    break;
                                                }
                                            }
                                        },
                                        Err(_) => {
                                            println!("err send req to store");
                                            break;
                                        }
                                    }
                                } else {
                                    println!("cannot connect to store");
                                    break;
                                }
                                params.track += 1;
                            }
                        }
                    }
                    Err(_) => (),
                }
                match child.wait() {
                    _ => {
                        println!("finish");
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
