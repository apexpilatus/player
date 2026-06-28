mod err_codes;
mod page_home;
mod page_tracks;
mod player;
mod proxy;
mod volume;
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
                    "/" => page_home::send_home(params, BufWriter::new(stream)),
                    "/tracks" => page_tracks::send_tracks(params, BufWriter::new(stream)),
                    "/stream" => player::play(params, BufWriter::new(stream)),
                    "/getcards" => volume::get_cards(BufWriter::new(stream)),
                    "/getvolume" => volume::get_volume(params, BufWriter::new(stream)),
                    _ => proxy::forward_if_no_static(path, &req, BufWriter::new(stream)),
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
