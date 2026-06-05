use err_codes;
use std::io::{Read, Write};
use std::process::{Command, Stdio};
use BufWriter;
use TcpStream;

pub fn play(mut req: String, mut streamer: BufWriter<TcpStream>) {
    req.push_str("\r\n\r\n");
    if let Ok(mut store) = TcpStream::connect(env!("STORE_ADDR")) {
        if let Ok(mut child) = Command::new("play")
            .env("PATH", env!("STREAMER_PATH"))
            /*.stdin(Stdio::null())
            .stderr(Stdio::null())*/
            .stdout(Stdio::piped())
            .current_dir(env!("STREAMER_PATH"))
            .spawn()
        {
            if let Some(ref mut stdout) = child.stdout {
                let mut buf: Vec<u8> = vec![0; streamer.capacity()];
                if let Ok(size) = stdout.read(&mut buf) {
                    if size != 0 {
                        match streamer.write_all(&buf[..size]) {
                            Ok(_) => {
                                if let Ok(resp) = str::from_utf8(&buf[..size]) {
                                    if !resp.contains("404 shit happens") {
                                        match store.write_all(req.as_bytes()) {
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
                                            }
                                            _ => (),
                                        }
                                    }
                                }
                            }
                            Err(_) => (),
                        }
                    }
                }
            }
            match child.kill() {
                _ => match child.wait() {
                    _ => return,
                },
            }
        }
    }
    match streamer.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}
