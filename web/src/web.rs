mod page_home;
mod resp_static;
use std::io::{BufRead, BufReader, Write};
use std::net::{TcpListener, TcpStream};
use std::thread;

fn selector(stream: TcpStream) {
    let reader = BufReader::new(&stream);
    let req: Vec<_> = reader
        .lines()
        .map(|result| result.unwrap_or(String::from("")))
        .take_while(|line| !line.is_empty())
        .collect();
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
            thread::spawn(|| selector(stream));
        };
    }
}
