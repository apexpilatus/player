mod data_static;
mod err_codes;
mod proxy;
mod page_home;
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
                    "/" => page_home::send_home(BufWriter::new(stream)),
                    _ => data_static::send_static(path, &req, BufWriter::new(stream)),
                }
            }
        }
    }
}

fn main() {
    let listener = match TcpListener::bind(env!("LISTEN_ADDR")) {
        Ok(listener) => listener,
        Err(_) => return,
    };
    for stream in listener.incoming() {
        if let Ok(stream) = stream {
            thread::spawn(move || selector(stream));
        };
    }
}
