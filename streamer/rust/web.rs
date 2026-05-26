mod data_static;
mod err_codes;
mod proxy;
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
                    _ => data_static::send_static(path, BufWriter::new(stream)),
                }
            }
        }
    }
}

fn main() {
    let bind_addr = env!("LISTEN_ADDR");
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
