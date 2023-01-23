
#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub enum Opcode {
    ContinuationFrame = 0x0,
    TextFrame = 0x1,
    BinaryFrame = 0x2,
    ConectionClose = 0x8,
    Ping = 0x9,
    Pong = 0xA,
}

impl Opcode {
    pub fn new(byte1: u8) -> Option<Opcode> {
        match byte1 {
            0x0 => Some(Opcode::ContinuationFrame),
            0x1 => Some(Opcode::TextFrame),
            0x2 => Some(Opcode::BinaryFrame),
            0x8 => Some(Opcode::ConectionClose),
            0x9 => Some(Opcode::Ping),
            0xA => Some(Opcode::Pong),
            _ => None,
        }
    }
}
#[derive(Clone, Copy, Debug)]
#[repr(u16)]
pub enum ControlCloseCode {
    Normal = 1000,
    GoingAway = 1001,
    ProtocolError = 1002,
    DataNotAccept = 1003,
    InvalidData = 1007,
    ViolatePolicy = 1008,
    MessageToBig = 1009,
    ExtensionError = 1010,
    UnexpectedCondition = 1011,
    ApplicationUse(u16),
    PrivateUse(u16),
}

impl ControlCloseCode {
    fn from(raw: u16) -> Self {
        match raw {
            1000 => Self::Normal,
            1001 => Self::GoingAway,
            1003 => Self::DataNotAccept,
            1007 => Self::InvalidData,
            1008 => Self::ViolatePolicy,
            1009 => Self::MessageToBig,
            1010 => Self::ExtensionError,
            1011 => Self::UnexpectedCondition,
            n @ 3000..=3999 => Self::ApplicationUse(n),
            n @ 4000..=4999 => Self::PrivateUse(n),
            _ => Self::ProtocolError,
        }
    }

    pub fn as_u16(self) -> u16 {
        match self {
            ControlCloseCode::Normal => 1000,
            ControlCloseCode::GoingAway => 1001,
            ControlCloseCode::ProtocolError => 1002,
            ControlCloseCode::DataNotAccept => 1003,
            ControlCloseCode::InvalidData => 1007,
            ControlCloseCode::ViolatePolicy => 1008,
            ControlCloseCode::MessageToBig => 1009,
            ControlCloseCode::ExtensionError => 1010,
            ControlCloseCode::UnexpectedCondition => 1011,
            ControlCloseCode::ApplicationUse(n) => n,
            ControlCloseCode::PrivateUse(n) => n,
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
    payload_size: u64,
    header_size: u64,
}

impl DataFrame {
    pub fn text(msg: String) -> Self {
        DataFrame {
            opcode: Opcode::TextFrame,
            flags: DataFrameFlags::new(),
            masking_key: [0; 4],
            payload: msg.as_bytes().to_vec(),
            payload_size: 0,
            header_size: 0,
        }
    }
    pub fn pong() -> Self {
        DataFrame {
            opcode: Opcode::Pong,
            flags: DataFrameFlags::new(),
            masking_key: [0; 4],
            payload: vec![],
            header_size: 0,
            payload_size: 0,
        }
    }
    pub fn closing(statuscode: ControlCloseCode) -> Self {
        let statuscode = statuscode.as_u16();
        DataFrame {
            opcode: Opcode::ConectionClose,
            flags: DataFrameFlags::new(),
            masking_key: [0; 4],
            payload: vec![(statuscode >> 8) as u8, statuscode as u8],
            header_size: 0,
            payload_size: 2,
        }
    }
    pub fn from_raw(data: &[u8]) -> Result<Self, Option<ControlCloseCode>> {
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

        let mut header_size = 2;
        if data.len() < header_size {
            return Err(None);
        }

        let opcode = Opcode::new(data[0] & 0b1111);
        if opcode.is_none() {
            return Err(Some(ControlCloseCode::ProtocolError));
        }
        let opcode = opcode.unwrap();

        let flags: DataFrameFlags = DataFrameFlags::from(data[0], data[1]);

        let mut payload_size = (data[1] & 0x7F) as u64;
        if payload_size == 126 {
            header_size = 4;
            if data.len() < header_size {
                return Err(None);
            }
            payload_size = (data[2] as u64) << 8;
            payload_size += data[3] as u64;
        } else if payload_size == 127 {
            // if data[0] >> 7 == 1 {
            //     data[0] &= 0x7F;
            // }

            header_size = 10;
            if data.len() < header_size {
                return Err(None);
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
            if data.len() < header_size + 4 {
                return Err(None);
            }
            masking_key[0] = data[header_size];
            masking_key[1] = data[header_size + 1];
            masking_key[2] = data[header_size + 2];
            masking_key[3] = data[header_size + 3];
            header_size += 4;
        }

        let mut payload = Vec::new();
        if payload_size > 0 {
            let mut remaining = payload_size as usize + header_size;
            if remaining > data.len() {
                remaining = data.len();
            }
            payload.append(&mut data[header_size..remaining].to_vec());
            if flags.mask {
                for (i, byte) in payload.iter_mut().enumerate() {
                    *byte ^= masking_key[i % 4];
                }
            }
        }

        Ok(DataFrame {
            opcode,
            flags,
            masking_key,
            payload,
            payload_size,
            header_size: header_size as u64,
        })
    }
    pub fn current_len(&self) -> usize {
        self.payload.len() + self.header_size as usize
    }
    // pub fn target_len(&self) -> usize {
    //     (self.payload_size + self.header_size) as usize
    // }
    pub fn is_full(&self) -> bool {
        self.payload.len() as u64 == self.payload_size
    }
    pub fn get_closing_reason(&self) -> Result<String, ControlCloseCode> {
        if self.payload.len() < 2 {
            return Ok("".to_string());
        }
        let reason = self.as_string_with_offset(2);
        if reason.is_err() {
            return Err(ControlCloseCode::InvalidData);
        }
        Ok(reason.unwrap())
    }
    pub fn get_closing_code(&self) -> ControlCloseCode {
        if self.payload.is_empty() {
            return ControlCloseCode::Normal;
        }
        if self.payload.len() < 2 {
            return ControlCloseCode::ProtocolError;
        }
        let statuscode = (self.payload[0] as u16) << 8 | self.payload[1] as u16;
        ControlCloseCode::from(statuscode)
    }
    pub fn add_payload(&mut self, data: &[u8]) -> usize {
        let index = self.payload.len();
        let mut remaining = self.payload_size as usize - index;
        if remaining > data.len() {
            remaining = data.len();
        }
        if self.flags.mask {
            for (i, byte) in data[0..remaining].iter().enumerate() {
                self.payload.push(*byte ^ self.masking_key[(i + index) % 4]);
            }
        } else {
            self.payload.append(&mut data[0..remaining].to_vec());
        }
        remaining
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
        } else if self.payload.len() >= 126 {
            df[1] |= 126;
            df.push((self.payload.len() >> 8) as u8);
            df.push(self.payload.len() as u8);
        } else {
            df[1] |= self.payload.len() as u8;
        }
        df.append(&mut self.payload.clone());
        df
    }
    pub fn as_string(&self) -> Result<String, std::str::Utf8Error> {
        self.as_string_with_offset(0)
    }
    pub fn as_string_with_offset(&self, offset: usize) -> Result<String, std::str::Utf8Error> {
        let payload = &self.payload[offset..];
        let str = std::str::from_utf8(payload);
        if let Ok(str) = str {
            return Ok(str.to_string());
        }
        Err(str.unwrap_err())
    }
    pub fn frames_as_string(frames: &[DataFrame]) -> Result<String, std::str::Utf8Error> {
        let mut str = "".to_string();
        for frame in frames {
            str += &frame.as_string()?;
        }
        Ok(str)
    }
}
