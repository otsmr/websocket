const CHARS: &str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

pub fn encode(input: &Vec<u8>) -> String {
    let mut out = Vec::new();
    let mut tmp = [0; 3];
    let mut index = 0;
    let mut add = 0;

    while input.len() > index {
        for i in 0..3 {
            if input.len() <= i + index {
                tmp[i] = 0x0;
                add += 1;
            } else {
                tmp[i] = input[index + i];
            }
        }

        index += 3;

        let mut a = tmp[0] >> 2 & 63;
        out.push(CHARS.chars().nth(a as usize).unwrap());
        a = ((tmp[0] & 3) << 4 | (tmp[1] >> 4)) & 63;
        out.push(CHARS.chars().nth(a as usize).unwrap());
        a = ((tmp[1] & 15) << 2 | (tmp[2] >> 6)) & 63;
        out.push(CHARS.chars().nth(a as usize).unwrap());
        a = tmp[2] & 63;
        out.push(CHARS.chars().nth(a as usize).unwrap());
    }

    if add > 0 {
        let l = out.len() - 1;
        for i in 0..add {
            out[l - i] = '=';
        }
    }

    // out.resize(out.len() + add, '=');

    out.iter().collect()
}

#[cfg(test)]
mod tests {
    use crate::base64;

    #[test]
    fn test_sha1() {
        let test_strings = [
            ("", ""),
            ("f", "Zg=="),
            ("fo", "Zm8="),
            ("foo", "Zm9v"),
            ("foob", "Zm9vYg=="),
            ("fooba", "Zm9vYmE="),
            ("foobar", "Zm9vYmFy"),
        ];
        for (raw, b64) in test_strings {
            assert_eq!(base64::encode(&raw.as_bytes().to_vec()), b64.to_string());
        }
    }
}
