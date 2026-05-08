use err_codes;
use std::io::Read;
use std::io::Write;
use std::process::Command;
use TcpStream;

pub fn send_extracted(params: Option<&str>, req: &Vec<String>, mut stream: TcpStream) {
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
        println!("{range}");
        if let Ok(child) = Command::new("flac_extract")
            .env("PATH", env!("STORE_PATH"))
            .arg(params)
            .arg(range)
            .current_dir(env!("MUSIC_PATH"))
            .spawn()
        {
            if let Some(stdout) = child.stdout {
                println!("ok");
            } else {println!("not ok")}
        } else {println!("what?")}
    }
    match stream.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}
