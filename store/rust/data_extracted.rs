use err_codes;
use std::io::{Read, Write};
use std::process::{Command, Stdio};
use BufWriter;
use TcpStream;

pub fn send_extracted(params: Option<&str>, req: &Vec<String>, mut stream: BufWriter<TcpStream>) {
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
        if let Ok(mut child) = Command::new("flac_extract")
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
                if let Ok(size) = stdout.read(&mut buf) {
                    buf.truncate(size);
                    println!("{}", String::from_utf8(buf).unwrap_or_default());
                }
            }
            match child.kill() {
                _ => match child.wait() {
                    _ => (),
                },
            }
        }
    }
    match stream.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}
