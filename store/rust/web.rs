mod data_picture;
mod data_static;
mod err_codes;
mod page_albums;
mod page_home;
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
            let mut url = url.split("?");
            if let Some(path) = url.next() {
                let params = url.next();
                match path {
                    "/picture" => data_picture::send_picture(params, stream),
                    "/" => page_home::send_home(params, stream),
                    "/albums" => page_albums::send_albums(params, stream),
                    _ => data_static::send_static(path, stream),
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
