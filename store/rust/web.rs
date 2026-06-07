mod data_flac;
mod data_picture;
mod data_static;
mod err_codes;
mod page_albums;
mod page_home;
mod system_touch;
mod text_meta;
use std::io::{BufRead, BufReader, BufWriter};
use std::net::{TcpListener, TcpStream};
use std::thread;

fn selector(stream: TcpStream) {
    let reader = BufReader::new(&stream);
    let mut req: Vec<String> = Vec::new();
    for line in reader.lines() {
        match line {
            Ok(line) => {
                if line.is_empty() {
                    break;
                }
                req.push(line);
            }
            Err(_) => return,
        }
    }
    if !req.is_empty() {
        if let Some(url) = req[0].split(" ").nth(1) {
            let mut url = url.split("?");
            if let Some(path) = url.next() {
                let params = url.next();
                match path {
                    "/picture" => data_picture::send_picture(params, BufWriter::new(stream)),
                    "/" => page_home::send_home(params, BufWriter::new(stream)),
                    "/meta" => text_meta::send_text(params, BufWriter::new(stream)),
                    "/fetch" => data_flac::send_flac(params, &req, BufWriter::new(stream)),
                    "/albums" => page_albums::send_albums(params, BufWriter::new(stream)),
                    "/touch" => system_touch::touch(params, BufWriter::new(stream)),
                    _ => data_static::send_static(path, BufWriter::new(stream)),
                }
            }
        }
    }
}

fn main() {
    let bind_addr = env!("BIND_ADDR");
    let listener = match TcpListener::bind(bind_addr) {
        Ok(listener) => listener,
        Err(_) => return,
    };
    for stream in listener.incoming() {
        if let Ok(stream) = stream {
            thread::spawn(move || selector(stream));
        };
    }
}
