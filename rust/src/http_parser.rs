use std::{collections::HashMap, io};

pub struct Url {
    pub protocol: String,
    pub host: String,
    pub path: String,
    pub query: String,
}
pub struct HttpHeader {
    pub method: String,
    pub url: Url,
    pub fields: HashMap<String, String>,
}

impl HttpHeader {
    pub fn new() -> Self {
        HttpHeader {
            method: "GET".to_string(),
            url: Url {
                protocol: "http://".to_string(),
                host: "".to_string(),
                path: "".to_string(),
                query: "".to_string(),
            },
            fields: HashMap::new(),
        }
    }
}

enum ParsingState {
    Method,
    Resource,
    Protocol,
    HeaderName,
    HeaderValue,
    Body,
}

pub fn parse_http_header(raw_data: Vec<u8>) -> Result<HttpHeader, io::Error> {
    let http_header = HttpHeader::new();

    let state = ParsingState::Method;

    for (index, byte) in raw_data.iter().enumerate() {

        match state {
            ParsingState::Method => {}
            ParsingState::Resource => {}
            ParsingState::Protocol => {}
            ParsingState::HeaderName => {}
            ParsingState::HeaderValue => {}
            ParsingState::Body => break,
        }

    }

    Ok(http_header)
}
