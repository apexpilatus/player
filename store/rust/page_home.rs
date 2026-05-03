use std::io::Write;
use TcpStream;

pub fn send_home(params: Option<&str>, mut stream: TcpStream) {
    let mut html = String::from(include_str!("../html/home.html"));
    let mut icon = String::from("<link id=\"icon\" rel=\"icon\" href=\"favicon.ico\">");
    if let Some(into) = html.find("</body>") {
        let mut init = true;
        if let Some(params) = params {
            for param in params.split("&") {
                if param.starts_with("scroll=") {
                    init = false;
                    let script = format!("<script>loadalbums(\"{param}\")</script>");
                    html.insert_str(into, &script);
                }
                if param.starts_with("album=") {
                    if let Some(album) = param.split("=").nth(1) {
                        icon = format!(
                            "<link id=\"icon\" rel=\"icon\" href=\"picture?album={album}\">"
                        );
                    }
                }
            }
        }
        if init {
            html.insert_str(into, "<script>loadalbums()</script>");
        }
    }
    if let Some(into) = html.find("</head>") {
        html.insert_str(into, &icon);
    }
    let hdr = format!(
        "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=utf-8\r\n\
Content-Length: {}\r\n\r\n",
        html.len()
    );
    match stream.write_all(hdr.as_bytes()) {
        Ok(_) => match stream.write_all(html.as_bytes()) {
            _ => (),
        },
        _ => (),
    }
}
