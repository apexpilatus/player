mod resp;
use std::io::{BufRead, BufReader, Write};
use std::net::{TcpListener, TcpStream};
use std::thread;

fn selector(mut stream: TcpStream) {
    let reader = BufReader::new(&stream);
    let req: Vec<String> = reader
        .lines()
        .map(|result| result.unwrap_or(String::from("")))
        .take_while(|line| !line.is_empty())
        .collect();
    if !req.is_empty() {
        match req[0].split(" ").nth(1) {
            Some(url) => {
                println!("{url}");
                match stream.write_all(resp::ERR.as_bytes()) {
                    _ => {}
                }
            }
            None => {}
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
        match stream {
            Ok(stream) => thread::spawn(|| selector(stream)),
            Err(_) => continue,
        };
    }
}
