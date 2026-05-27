use err_codes;
use std::io::Write;
use BufWriter;
use TcpStream;

pub fn forward(mut streamer: BufWriter<TcpStream>) {
    match TcpStream::connect(env!("STORE_ADDR")) {
              Ok(store) =>  {let mut buf: Vec<u8> = vec![0; streamer.capacity()];
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
        return;},
    Err(_) => match streamer.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
    }
}
