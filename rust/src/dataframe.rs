use std::io::{Error, ErrorKind};

#[derive(Debug, Clone, Copy)]
pub enum Opcode {
    ContinuationFrame = 0x0,
    TextFrame = 0x1,
    BinaryFrame = 0x2,
    ConectionClose = 0x8,
    Ping = 0x9,
    Pong = 0xA,
}

impl From<u8> for Opcode {
    fn from(byte1: u8) -> Self {
        match byte1 {
            0x0 => Opcode::ContinuationFrame,
            0x1 => Opcode::TextFrame,
            0x2 => Opcode::BinaryFrame,
            0x8 => Opcode::ConectionClose,
            0x9 => Opcode::Ping,
            0xA => Opcode::Pong,
            _ => Opcode::TextFrame,
        }
    }
}

pub struct DataFrameFlags {
    pub fin: bool,
    rsv1: bool,
    rsv2: bool,
    rsv3: bool,
    mask: bool,
}
impl DataFrameFlags {
    fn new() -> Self {
        DataFrameFlags {
            fin: true,
            rsv1: false,
            rsv2: false,
            rsv3: false,
            mask: false,
        }
    }

    fn as_bytes(&self) -> [u8; 2] {
        let mut ret = [0; 2];
        if self.mask {
            ret[1] |= 1 << 7;
        }
        if self.fin {
            ret[0] |= 1 << 7;
        }
        if self.rsv1 {
            ret[0] |= 1 << 6;
        }
        if self.rsv2 {
            ret[0] |= 1 << 5;
        }
        if self.rsv3 {
            ret[0] |= 1 << 4;
        }
        ret
    }

    fn from(byte1: u8, byte2: u8) -> Self {
        let mut ret = DataFrameFlags::new();
        ret.fin = (byte1 & (1 << 7)) > 0;
        ret.rsv1 = byte1 & (1 << 6) > 0;
        ret.rsv2 = byte1 & (1 << 5) > 0;
        ret.rsv3 = byte1 & (1 << 4) > 0;
        ret.mask = byte2 & 0x80 > 0;
        ret
    }
}
pub struct DataFrame {
    pub opcode: Opcode,
    pub flags: DataFrameFlags,
    masking_key: [u8; 4],
    pub payload: Vec<u8>,
    pub payload_size: u64
}

impl DataFrame {
    pub fn text(msg: String) -> Self {
        DataFrame {
            opcode: Opcode::TextFrame,
            flags: DataFrameFlags::new(),
            masking_key: [0; 4],
            payload: msg.as_bytes().to_vec(),
            payload_size: 0
        }
    }
    pub fn closing(statuscode: u16) -> Self {
        DataFrame {
            opcode: Opcode::ConectionClose,
            flags: DataFrameFlags::new(),
            masking_key: [0; 4],
            payload: vec![(statuscode >> 8) as u8, statuscode as u8],
            payload_size: 2
        }
    }
    pub fn from_raw(data: &[u8]) -> Result<Self, Error> {
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

        let mut header_len = 2;
        if data.len() < header_len {
            return Err(Error::new(ErrorKind::InvalidData, "data.len() < header_len"));
        }

        let opcode: Opcode = (data[0] & 0b1111).into();
        let mut flags: DataFrameFlags = DataFrameFlags::from(data[0], data[1]);

        if (data[1] >> 7) == 1 {
            flags.mask = true;
        }
        let mut payload_size = (data[1] & 0x7F) as u64;
        if payload_size == 126 {
            header_len = 4;
            if data.len() < header_len {
                return Err(Error::new(ErrorKind::InvalidData, "data.len() < header_len"));
            }
            payload_size = (data[2] as u64) << 8;
            payload_size += data[3] as u64;
        }

        if payload_size == 127 {
            // if data[0] >> 7 == 1 {
            //     data[0] &= 0x7F;
            // }

            header_len = 10;
            if data.len() < header_len {
                return Err(Error::new(ErrorKind::InvalidData, "data.len() < header_len"));
            }
            payload_size = (data[2] as u64) << 56;
            payload_size += (data[3] as u64) << 48;
            payload_size += (data[4] as u64) << 40;
            payload_size += (data[5] as u64) << 32;
            payload_size += (data[6] as u64) << 24;
            payload_size += (data[7] as u64) << 16;
            payload_size += (data[8] as u64) << 8;
            payload_size += data[9] as u64;
        }

        let mut masking_key = [0; 4];

        if flags.mask {
            masking_key[0] = data[header_len];
            masking_key[1] = data[header_len + 1];
            masking_key[2] = data[header_len + 2];
            masking_key[3] = data[header_len + 3];
            header_len += 4;
        }

        let mut payload = Vec::new();
        payload.append(&mut data[header_len..].to_vec());

        if flags.mask {
            for (i, byte) in payload.iter_mut().enumerate() {
                *byte ^= masking_key[i % 4];
            }
        }

        Ok(DataFrame {
            opcode,
            flags,
            masking_key,
            payload,
            payload_size
        })
    }
    pub fn get_closing_code(&self) -> u16 {
        if self.payload.len() != 2 {
            return 0;
        }
        (self.payload[0] as u16) << 8 & self.payload[1] as u16
    }
    pub fn add_payload(&mut self, data: &[u8]) {
        let index = self.payload.len();
        if self.flags.mask {
            for (i, byte) in data.iter().enumerate() {
                self.payload.push(*byte ^ self.masking_key[(i + index) % 4]);
            }
        } else {
            self.payload.append(&mut data.to_vec());
        }
    }
    pub fn as_bytes(&self) -> Vec<u8> {
        let mut df = Vec::new();
        df.append(&mut self.flags.as_bytes().to_vec());
        df[0] |= self.opcode as u8;
        if self.payload.len() > 0xFFFF {
            df[1] |= 127;
            for i in 0..8 {
                df.push((self.payload.len() >> ((7 - i) * 8)) as u8);
            }
        } else if self.payload.len() > 126 {
            df[1] |= 126;
            df.push((self.payload.len() >> 8) as u8);
            df.push(self.payload.len() as u8);
        } else {
            df[1] |= self.payload.len() as u8;
        }
        df.append(&mut self.payload.clone());
        df
    }
    pub fn as_string(&self) -> Result<String, Error> {
        let payload = self.payload.clone();
        let str = std::str::from_utf8(payload.as_slice());
        if let Ok(str) = str {
            return Ok(str.to_string());
        }
        Err(Error::new(ErrorKind::InvalidData, "data.len() < header_len"))
    }
    pub fn frames_as_string(frames: &[DataFrame]) -> Result<String, Error> {
        frames.iter().map(|f| f.as_string()).filter(|f| f.is_ok()).collect()
    }
}
