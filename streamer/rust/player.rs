use err_codes;
use std::io::{BufRead, Read, Write};
use std::process::{Command, Stdio};
use BufReader;
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
                //.stdout(Stdio::piped())
                .current_dir(env!("STREAMER_PATH"))
                .spawn()
            {
                let mut buf: Vec<u8> = vec![0; streamer.capacity()];
                let hdr = format!(
                    "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=utf-8\r\n\
Cache-control: no-cache\r\n\
X-Content-Type-Options: nosniff\r\n\r\n"
                );
                'get_tracks: loop {
                    let req = format!(
                        "GET /fetch?album={}&track={} HTTP/1.1\r\n\r\n",
                        params.album, params.track
                    );
                    if let Ok(mut store) = TcpStream::connect(env!("STORE_ADDR")) {
                        match store.write_all(req.as_bytes()) {
                            Ok(_) => {
                                let reader = BufReader::new(&store);
                                let mut resp: Vec<String> = Vec::new();
                                for line in reader.lines() {
                                    match line {
                                        Ok(line) => {
                                            if line.is_empty() {
                                                break;
                                            }
                                            resp.push(line);
                                        }
                                        Err(_) => {
                                            println!("err read");
                                            break 'get_tracks;
                                        }
                                    }
                                }
                                if !(resp.is_empty() || resp[0].contains("404 shit happens")) {
                                    println!("{}", resp.join("\r\n"));
                                } else {
                                    println!("empty or shit");
                                    break;
                                }
                            }
                            Err(_) => {
                                println!("err write");
                                break;
                            }
                        }
                    } else {
                        println!("cannot connect");
                        break;
                    }
                    params.track += 1;
                }
                match child.wait() {
                    _ => return,
                }
            }
        }
    }
    match streamer.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }

    // if let Ok(mut store) = TcpStream::connect(env!("STORE_ADDR")) {
    //     if let Ok(mut child) = Command::new("play")
    //         .env("PATH", env!("STREAMER_PATH"))
    //         /*.stdin(Stdio::null())
    //         .stderr(Stdio::null())*/
    //         .stdout(Stdio::piped())
    //         .current_dir(env!("STREAMER_PATH"))
    //         .spawn()
    //     {
    //         if let Some(ref mut stdout) = child.stdout {
    //             let mut buf: Vec<u8> = vec![0; streamer.capacity()];
    //             if let Ok(size) = stdout.read(&mut buf) {
    //                 if size != 0 {
    //                     match streamer.write_all(&buf[..size]) {
    //                         Ok(_) => {
    //                             if let Ok(resp) = str::from_utf8(&buf[..size]) {
    //                                 if !resp.contains("404 shit happens") {
    //                                     match store.write_all(req.as_bytes()) {
    //                                         Ok(_) => {
    //                                             let mut buf: Vec<u8> = vec![0; streamer.capacity()];
    //                                             loop {
    //                                                 match store.read(&mut buf) {
    //                                                     Ok(size) => {
    //                                                         if size == 0 {
    //                                                             break;
    //                                                         }
    //                                                         match streamer.write_all(&buf[..size]) {
    //                                                             Ok(_) => (),
    //                                                             Err(_) => break,
    //                                                         }
    //                                                     }
    //                                                     Err(_) => break,
    //                                                 }
    //                                             }
    //                                         }
    //                                         _ => (),
    //                                     }
    //                                 }
    //                             }
    //                         }
    //                         Err(_) => (),
    //                     }
    //                 }
    //             }
    //         }
    //         match child.kill() {
    //             _ => match child.wait() {
    //                 _ => return,
    //             },
    //         }
    //     }
    // }
    // match streamer.write_all(err_codes::ERR_404.as_bytes()) {
    //     _ => (),
    // }
}
