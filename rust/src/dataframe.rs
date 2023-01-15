use log::info;

#[derive(Debug)]
pub enum Opcode {
    ContinuationFrame = 0x0,
    TextFrame = 0x1,
    BinaryFrame = 0x2,
    ConectionClose = 0x8,
    Ping = 0x9,
    Pong = 0xA,
}

struct DataFrameFlags {
    fin: bool,
    rsv1: bool,
    rsv2: bool,
    rsv3: bool,
    mask: bool,
}
impl DataFrameFlags {
    fn new() -> Self {
        DataFrameFlags {
            fin: false,
            rsv1: false,
            rsv2: false,
            rsv3: false,
            mask: false,
        }
    }
}
pub struct DataFrame {
    pub opcode: Opcode,
    flags: DataFrameFlags,
    masking_key: u32,
    pub payload: Vec<u8>,
}

impl DataFrame {
    pub fn message(opcode: Opcode, msg: String) -> Self {
        DataFrame {
            opcode,
            flags: DataFrameFlags::new(),
            masking_key: 0,
            payload: msg.as_bytes().to_vec(),
        }
    }
    pub fn from_raw(data: &[u8]) -> Result<Self, ()> {
        /*
         0               1               2               3
         0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
        +-+-+-+-+-------+-+-------------+-------------------------------+
        |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
        |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
        |N|V|V|V|       |S|             |   (if payload len==126/127)   |
        | |1|2|3|       |K|             |                               |
        +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
        |     Extended payload length continued, if payload len == 127  |
        + - - - - - - - - - - - - - - - +-------------------------------+
        |                               |Masking-key, if MASK set to 1  |
        +-------------------------------+-------------------------------+
        | Masking-key (continued)       |          Payload Data         |
        +-------------------------------- - - - - - - - - - - - - - - - +
        :                     Payload Data continued ...                :
        + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
        |                     Payload Data continued ...                |
        +---------------------------------------------------------------+ */

        let opcode = Opcode::TextFrame;
        let flags = DataFrameFlags::new();
        let masking_key = 0;
        let payload = Vec::new();

        info!("New DataFrame {:?}", data);
        Ok(DataFrame {
            opcode,
            flags,
            masking_key,
            payload,
        })
    }
    pub fn as_string(&self) -> Result<String, ()> {
        let str = std::str::from_utf8(self.payload.as_slice());
        if let Ok(str) = str {
            return Ok(str.to_string());
        }
        Err(())
    }
}
