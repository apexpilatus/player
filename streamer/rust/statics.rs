use std::collections::HashMap;

pub struct StaticData {
    pub content_type: String,
    pub bytes: Vec<u8>,
}

pub fn data_map() -> HashMap<String, StaticData> {
    let mut data = HashMap::new();
    data.insert(
        "/home.css".to_string(),
        StaticData {
            content_type: "text/css; charset=utf-8".to_string(),
            bytes: include_bytes!("../style/home.css").to_vec(),
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
        "/tracks.css".to_string(),
        StaticData {
            content_type: "text/css; charset=utf-8".to_string(),
            bytes: include_bytes!("../style/tracks.css").to_vec(),
        },
    );
    data.insert(
        "/tracks.js".to_string(),
        StaticData {
            content_type: "text/javascript; charset=utf-8".to_string(),
            bytes: include_bytes!("../js/tracks.js").to_vec(),
        },
    );
    data
}
