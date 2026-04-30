mod page_home;
mod resp_static;
use std::io::{BufRead, BufReader};
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
                } else {
                    req.push(line);
                }
            }
            Err(_) => return,
        }
    }
    if !req.is_empty() {
        if let Some(url) = req[0].split(" ").nth(1) {
            match url.split("?").next().unwrap_or_else(|| url) {
                "/" => page_home::send_home(stream),
                _ => resp_static::send_static(url, stream),
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
