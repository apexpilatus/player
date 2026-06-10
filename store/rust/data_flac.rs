use err_codes;
use std::io::{Read, Write};
use std::process::{Command, Stdio};
use BufWriter;
use TcpStream;

pub fn send_flac(params: Option<&str>, req: &Vec<String>, mut stream: BufWriter<TcpStream>) {
    if let Some(params) = params {
        let mut range = String::new();
        for line in req {
            if line.to_lowercase().trim().starts_with("range") {
                range = line
                    .split('=')
                    .nth(1)
                    .unwrap_or_default()
                    .trim()
                    .to_string();
            }
        }
        if let Ok(mut child) = Command::new("flac")
            .env("PATH", env!("STORE_PATH"))
            .stdin(Stdio::null())
            .stderr(Stdio::null())
            .stdout(Stdio::piped())
            .arg(params)
            .arg(range)
            .current_dir(env!("MUSIC_PATH"))
            .spawn()
        {
            if let Some(ref mut stdout) = child.stdout {
                let mut buf: Vec<u8> = vec![0; stream.capacity()];
                loop {
                    match stdout.read(&mut buf) {
                        Ok(size) => {
                            if size == 0 {
                                break;
                            }
                            match stream.write_all(&buf[..size]) {
                                Ok(_) => (),
                                Err(_) => {
                                    break;
                                }
                            }
                        }
                        Err(_) => {
                            break;
                        }
                    }
                }
            } else {
                match stream.write_all(err_codes::ERR_404.as_bytes()) {
                    _ => (),
                }
            }
            match child.kill() {
                _ => match child.wait() {
                    _ => {
                        return;
                    }
                },
            }
        }
    }
    match stream.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}
