use err_codes;
use std::io::Write;
use std::process::Command;
use TcpStream;

pub fn send_picture(params: Option<&str>, mut stream: TcpStream) {
    if let Some(params) = params {
        for param in params.split("&") {
            if param.starts_with("album=") {
                if let Some(album) = param.split("=").nth(1) {
                    if let Ok(output) = Command::new("picture")
                        .env("PATH", env!("STORE_PATH"))
                        .arg(album)
                        .current_dir(env!("MUSIC_PATH"))
                        .output()
                    {
                        if output.status.success() {
                            match stream.write_all(&output.stdout) {
                                _ => return,
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
