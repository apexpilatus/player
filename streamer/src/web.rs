use std::net::TcpListener;

fn main() {
    let listener = match TcpListener::bind("127.0.0.1:8888") {
        Ok(listener) => listener,
        Err(..) => loop {},
    };
    for stream in listener.incoming() {
        match stream {
            Ok(..) => println!("Connection established!"),
            Err(..) => continue,
        };
    }
}