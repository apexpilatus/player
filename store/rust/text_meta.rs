use err_codes;
use std::io::Write;
use std::process::Command;
use BufWriter;
use TcpStream;

pub fn send_text(params: Option<&str>, mut stream: BufWriter<TcpStream>) {
    if let Some(params) = params {
        if let Ok(output) = Command::new("meta")
            .env("PATH", env!("STORE_PATH"))
            .arg(params)
            .current_dir(env!("MUSIC_PATH"))
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
    match stream.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}
