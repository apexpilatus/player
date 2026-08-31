use std::collections::HashMap;

pub struct StaticData {
    pub content_type: String,
    pub bytes: Vec<u8>,
}

pub fn data_map() -> HashMap<String, StaticData> {
    let mut data = HashMap::new();
    data.insert(
        "/favicon.ico".to_string(),
        StaticData {
            content_type: "image/x-icon".to_string(),
            bytes: include_bytes!("../pictures/favicon.ico").to_vec(),
        },
    );
    data.insert(
        "/apple-touch-icon.png".to_string(),
        StaticData {
            content_type: "image/png".to_string(),
            bytes: include_bytes!("../pictures/favicon180.png").to_vec(),
        },
    );
    data.insert(
        "/apple-touch-icon-precomposed.png".to_string(),
        StaticData {
            content_type: "image/png".to_string(),
            bytes: include_bytes!("../pictures/favicon180.png").to_vec(),
        },
    );
    data.insert(
        "/apple-touch-icon-120x120-precomposed.png".to_string(),
        StaticData {
            content_type: "image/png".to_string(),
            bytes: include_bytes!("../pictures/favicon120.png").to_vec(),
        },
    );
    data.insert(
        "/home.css".to_string(),
        StaticData {
            content_type: "text/css; charset=utf-8".to_string(),
            bytes: include_bytes!("../style/home.css").to_vec(),
        },
    );
    data.insert(
        "/albums.css".to_string(),
        StaticData {
            content_type: "text/css; charset=utf-8".to_string(),
            bytes: include_bytes!("../style/albums.css").to_vec(),
        },
    );
    data.insert(
        "/home.js".to_string(),
        StaticData {
            content_type: "text/javascript; charset=utf-8".to_string(),
            bytes: include_bytes!("../js/home.js").to_vec(),
        },
    );
    data.insert(
        "/albums.js".to_string(),
        StaticData {
            content_type: "text/javascript; charset=utf-8".to_string(),
            bytes: include_bytes!("../js/albums.js").to_vec(),
        },
    );
    data
}
