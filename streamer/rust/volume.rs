use err_codes;
use std::io::Write;
use std::process::Command;
use BufWriter;
use TcpStream;

pub fn get_volume(params: Option<&str>, mut stream: BufWriter<TcpStream>) {
    if let Some(params) = params {
        for param in params.split("&") {
            if param.starts_with("name=") {
                if let Some(name) = param.split("=").nth(1) {
                    if let Ok(output) = Command::new("getvolume")
                        .env("PATH", env!("STREAMER_PATH"))
                        .arg(name)
                        .output()
                    {
                        if output.status.success() {
                            match stream.write_all(&output.stdout) {
                                _ => {
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    match stream.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}

pub fn get_cards(mut stream: BufWriter<TcpStream>) {
    if let Ok(output) = Command::new("getvolume")
        .env("PATH", env!("STREAMER_PATH"))
        .output()
    {
        if output.status.success() {
            match stream.write_all(&output.stdout) {
                _ => {
                    return;
                }
            }
        }
    }
    match stream.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}
