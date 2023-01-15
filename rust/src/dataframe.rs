
#[derive(Debug)]
pub enum DataFrameKind {
    Unknown,
    Text
}

pub struct DataFrame {
    pub kind: DataFrameKind
}


impl DataFrame {

    pub fn from_raw(_data: &[u8]) -> DataFrame {
        DataFrame {
            kind: DataFrameKind::Unknown
        }
    }
}
