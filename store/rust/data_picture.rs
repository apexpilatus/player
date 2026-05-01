use err_codes;
use std::io::Write;
use std::process::Command;
use TcpStream;

pub fn send_picture(path: Option<&str>, mut stream: TcpStream) {
    if let Some(path) = path {
        if let Ok(output) = Command::new("picture")
            .env("PATH", env!("STORE_PATH"))
            .arg(path)
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
    match stream.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}
