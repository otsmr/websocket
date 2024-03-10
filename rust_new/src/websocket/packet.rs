pub struct Packet {
    pub event: String,
    pub payload: String,
}

impl Packet {
    pub fn to_string(event: String, payload: String) -> String {
        event + ":" + &payload
    }
    pub fn from_string(packet: String) -> Self {
        let mut event = "".to_string();
        let mut payload = "".to_string();

        for (i, c) in packet.chars().enumerate() {
            if c == ':' {
                event = packet[..i].to_string();
                payload = packet[i + 1..].to_string();
                break;
            }
        }

        Self { event, payload }
    }
}
