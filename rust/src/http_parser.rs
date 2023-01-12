use std::{collections::HashMap, io};

// pub struct Url {
//     pub protocol: String,
//     pub host: String,
//     pub path: String,
//     pub query: String,
// }
pub struct HttpHeader {
    pub method: String,
    // pub url: Url,
    pub status_code: usize,
    pub resource: String,
    pub fields: HashMap<String, String>,
}

impl HttpHeader {
    pub fn new() -> Self {
        HttpHeader {
            method: "".to_string(),
            // url: Url {
            //     protocol: "http://".to_string(),
            //     host: "".to_string(),
            //     path: "".to_string(),
            //     query: "".to_string(),
            // },
            status_code: 0,
            resource: "".to_string(),
            fields: HashMap::new(),
        }
    }
    pub fn response_101() -> Self {
        let mut h = HttpHeader::new();
        h.status_code = 101;
        h.fields = HashMap::from([
            ("Upgrade".to_string(), "websocket".to_string()),
            ("Connection".to_string(), "Upgrade".to_string()),
            ("Sec-WebSocket-Version".to_string(), "13".to_string()),
        ]);
        h
    }
    pub fn response_400() -> Self {
        let mut h = HttpHeader::new();
        h.status_code = 400;
        h
    }
    fn status_code_as_string(&self) -> &str {
        match self.status_code {
            101 => "Switching Protocols",
            400 => "Bad Request",
            _ => "Unkown Error",
        }
    }
    fn as_string(&self) -> String {
        let mut ret = format!(
            "HTTP/1.1 {} {}\r\n",
            self.status_code,
            self.status_code_as_string()
        );

        for (k, v) in self.fields.iter() {
            ret.push_str(&format!("{}: {}\r\n", k, v));
        }

        ret.push_str("\r\n");
        ret
    }
    pub fn as_vec(&self) -> Vec<u8> {
        self.as_string().into_bytes()
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
    let mut http_header = HttpHeader::new();

    let mut state = ParsingState::Method;
    let mut tmp = vec![];
    let mut field_key = String::new();

    for byte in raw_data.iter() {
        match state {
            ParsingState::Method => match *byte {
                b' ' => {
                    http_header.method = String::from_utf8(tmp.clone()).unwrap_or_default();
                    tmp.clear();
                    state = ParsingState::Resource;
                }
                b => tmp.push(b),
            },
            ParsingState::Resource => match *byte {
                b' ' => {
                    http_header.resource = String::from_utf8(tmp.clone()).unwrap_or_default();
                    tmp.clear();
                    state = ParsingState::Protocol;
                }
                b => tmp.push(b),
            },
            ParsingState::Protocol => match *byte {
                b'\r' => (),
                b'\n' => {
                    // _ = String::from_utf8(tmp.clone()).unwrap_or_default();
                    // tmp.clear();
                    state = ParsingState::HeaderName;
                }
                _ => (), // b => tmp.push(b),
            },
            ParsingState::HeaderName => match *byte {
                b'\r' => (),
                b'\n' => state = ParsingState::Body,
                b':' => {
                    field_key = String::from_utf8(tmp.clone()).unwrap_or_default();
                    field_key = field_key.to_lowercase();
                    tmp.clear();
                }
                b' ' => state = ParsingState::HeaderValue,
                b => tmp.push(b),
            },
            ParsingState::HeaderValue => match *byte {
                b'\r' => (),
                b'\n' => {
                    let value = String::from_utf8(tmp.clone()).unwrap_or_default();
                    // FIXME: check in rfc if first or last occurent is used
                    http_header.fields.insert(field_key.clone(), value);
                    tmp.clear();
                    state = ParsingState::HeaderName;
                }
                b => tmp.push(b),
            },
            ParsingState::Body => break,
        }
    }

    Ok(http_header)
}

#[cfg(test)]
mod tests {

    #[test]
    fn parse_http_header() {
        let header = b"GET /test?hallo HTTP/1.1
Host: tsmr.eu
";

        let actual = super::parse_http_header(header.to_vec()).unwrap();

        assert_eq!(actual.method, "GET".to_string());
        assert_eq!(actual.resource, "/test?hallo".to_string());

        assert!(actual.fields.contains_key("host"));
        assert_eq!(actual.fields.get("host"), Some(&"tsmr.eu".to_string()));
        assert_eq!(actual.method, "GET".to_string());
        assert_eq!(actual.method, "GET".to_string());
        assert_eq!(actual.method, "GET".to_string());
        assert_eq!(actual.method, "GET".to_string());
        assert_eq!(actual.method, "GET".to_string());
        assert_eq!(actual.method, "GET".to_string());
    }
}
