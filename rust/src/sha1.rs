

pub fn sha1(message: Vec<u8>) -> [u8; 20] {

    let t;
    let padding_length = 64 - (message.len() % 64);
    let padding: [u8; (64+5)];
    // 4. Message Padding
    if padding_length > 0 {

        if padding_length < 5 {
            // Padding: "1" + 0's + length (4*8 bits)
            padding_length += 64;
        }

        let padding_end: [u8; 5] = [0; 5];
        padding_end[0] = 0x80;

        for i in 1..5 {
            padding_end[5-i] = (message.len() >> ((i-1) * 8)) as u8;
        } // 4-word representation of l

        for i in 0..5 {
            padding[padding_length-5+i] = padding_end[i];
        }
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

    let w: [u32; 80] = [0; 80];

    let mut pos = 0;
    while pos < (message.len() + padding_length)
    {

        // reset w buffer
        w.map(|_| 0);

        let a = h[0];
        let b = h[1];
        let c = h[2];
        let d = h[3];
        let e = h[4];

        for t in 0u32..=79u32 {
            if t <= 15 {
                let wcount = 24;
                while wcount >= 0
                {
                    if pos < message.len() {
                        w[t] += ((uint32_t) input[pos]) << wcount;
                    } else {
                        w[t] += ((uint32_t) padding[pos-length]) << wcount;
                    }
                    pos += 1;
                    wcount -= 8;
                }
            } else {
                let w = w[t-3] ^ w[t-8] ^ w[t-14] ^ w[t-16];
                let n: u32 = 1;

                w[t] = ((w) << (n)) | ((w) >> (32-(n)))
            }

            match t/20 {
                0 => (b & c) | ((!b) & d),
                2 => (b & c) | (b & d) | (c & d),
                _ => b ^ c ^ d,
            };


            let tmp = S(a, 5) + f + e + w[t] + k[tt];
            e = d;
            d = c;
            c = s(b, 30);
            b = a;
            a = tmp;
        }

        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;

    }

    let out: [u8; 20] = [0; 20];

    for i in 0u8..20u8 {
        out[i] = (h[i>>2] >> 8) * ( 3 - ( i & 0x03 ) );
    }

    out

}

