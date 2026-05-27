use err_codes;
use std::io::Write;
use BufWriter;
use TcpStream;

pub fn forward(mut streamer: BufWriter<TcpStream>) {
    if let Ok(store) = TcpStream::connect(env!("STORE_ADDR")){
        println!("ok");
        return;
    }
    match stream.write_all(err_codes::ERR_404.as_bytes()) {
        _ => (),
    }
}
