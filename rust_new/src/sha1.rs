
fn s(w: u32, n: u32) -> u32 {
    (w << n) | (w >> (32-n))
}

pub fn sha1(message: Vec<u8>) -> [u8; 20] {

    let mut padding_length = 64 - (message.len() % 64);
    let mut padding: [u8; (64+5)] = [0; 69];

    // 4. Message Padding
    if padding_length > 0 {

        if padding_length < 5 {
            // Padding: "1" + 0's + length (4*8 bits)
            padding_length += 64;
        }

        padding[0] = 0x80;

        for i in 1..5 {
            padding[padding_length-i] = ((message.len()*8) >> ((i-1) * 8)) as u8;
        } // 4-word representation of l

    }


    let k: [u32; 4] = [
        0x5A827999,
        0x6ED9EBA1,
        0x8F1BBCDC,
        0xCA62C1D6
    ];

    let mut h: [u32; 5] = [
        0x67452301,
        0xEFCDAB89,
        0x98BADCFE,
        0x10325476,
        0xC3D2E1F0
    ];

    let mut pos = 0;

    while pos < (message.len() + padding_length)
    {

        let mut w: [u32; 80] = [0; 80];

        let mut a = h[0];
        let mut b = h[1];
        let mut c = h[2];
        let mut d = h[3];
        let mut e = h[4];

        for t in 0..=79 {
            if t <= 15 {
                let mut wcount = 24;
                while wcount >= 0
                {
                    if pos < message.len() {
                        w[t] += (message[pos] as u32) << wcount;
                    } else {
                        w[t] += (padding[pos - message.len()] as u32) << wcount;
                    }
                    pos += 1;
                    wcount -= 8;
                }
            } else {
                w[t] = s(w[t-3] ^ w[t-8] ^ w[t-14] ^ w[t-16], 1);
            }

            let f = match t/20 {
                0 => (b & c) | ((b ^ 0xFFFFFFFF) & d),
                2 => (b & c) | (b & d) | (c & d),
                _ => b ^ c ^ d,
            };
            let ov_adding = [f, e, w[t], k[t/20]];
            let mut tmp: u32 = s(a, 5);
            for i in ov_adding {
                (tmp, _) = tmp.overflowing_add(i);
            }
            e = d;
            d = c;
            c = s(b, 30);
            b = a;
            a = tmp;
        }

        (h[0], _) = h[0].overflowing_add(a);
        (h[1], _) = h[1].overflowing_add(b);
        (h[2], _) = h[2].overflowing_add(c);
        (h[3], _) = h[3].overflowing_add(d);
        (h[4], _) = h[4].overflowing_add(e);

    }

    let mut out: [u8; 20] = [0; 20];

    for i in 0u8..20u8 {
        out[i as usize] = (h[(i>>2) as usize] >> (8 * (3-(i & 0x03 ))) as u32) as u8;
    }

    out

}




#[cfg(test)]
mod tests {
    use crate::sha1::sha1;

    fn sha1_as_hex(message: Vec<u8>) -> String {
        sha1(message).iter().map(|x| format!("{:02x}", x)).collect::<String>()
    }

    #[test]
    fn test_sha1() {
        let corpus1 = "The quick brown fox jumps over the lazy dog".to_string();
        let hash = sha1_as_hex(corpus1.as_bytes().to_vec());
        assert_eq!(hash, "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12".to_string());
    }
}
