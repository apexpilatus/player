use err_codes;
use std::io::{Read, Write};
use BufWriter;
use TcpStream;

pub fn forward(mut req: String, mut streamer: BufWriter<TcpStream>) {
    req.push_str("\r\n");
    match TcpStream::connect(env!("STORE_ADDR")) {
        Ok(mut store) => match store.write_all("fuck\r\n".as_bytes()) {
            Ok(_) => match store.flush() {
                Ok(_) => {
                    println!("{req}");
                    let mut buf: Vec<u8> = vec![0; streamer.capacity()];
                    loop {
                        match store.read(&mut buf) {
                            Ok(size) => {
                                if size == 0 {
                                    break;
                                }
                                match streamer.write_all(&buf[..size]) {
                                    Ok(_) => (),
                                    Err(_) => break,
                                }
                            }
                            Err(_) => break,
                        }
                    }
                    return;
                }
                _ => (),
            },
            _ => (),
        },
        Err(_) => match streamer.write_all(err_codes::ERR_404.as_bytes()) {
            _ => (),
        },
    }
}
