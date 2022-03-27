#  Elliptic Curve Digital Signature Algorithm  

Curve: **P-256** Hash Algorithm: **SHA-256**  

Message to be signed: `Example of ECDSA with P-256`  

**Signature Generation**  
H:       `A41A41A12A799548211C410C65D8133AFDE34D28BDD542E4B680CF2899C8A8C4`  
E:       `A41A41A12A799548211C410C65D8133AFDE34D28BDD542E4B680CF2899C8A8C4`  
K:       `7A1A7E52797FC8CAAA435D2A4DACE39158504BF204FBE19F14DBB427FAEE50AE`  
Kinv:    `62159E5BA9E712FB098CCE8FE20F1BED8346554E98EF3C7C1FC3332BA67D87EF`  
R_x:     `2B42F576D07F4165FF65D1F3B1500F81E44C316F1F0B3EF57325B69ACA46104F`  
R_y:     `3CE76603264661EA2F602DF7B4510BBC9ED939233C553EA5F42FB3F1338174B5`  
R:       `2B42F576D07F4165FF65D1F3B1500F81E44C316F1F0B3EF57325B69ACA46104F`  
D:       `C477F9F65C22CCE20657FAA5B2D1D8122336F851A508A1ED04E479C34985BF96`  
S:       `DC42C2122D6392CD3E3A993A89502A8198C1886FE69D262C4B329BDB6B63FAF1`  
Signature  
R:       `2B42F576D07F4165FF65D1F3B1500F81E44C316F1F0B3EF57325B69ACA46104F`  
S:       `DC42C2122D6392CD3E3A993A89502A8198C1886FE69D262C4B329BDB6B63FAF1`  


**Signature Verification**  
Q_x:     `B7E08AFDFE94BAD3F1DC8C734798BA1C62B3A0AD1E9EA2A38201CD0889BC7A19`  
Q_y:     `3603F747959DBF7A4BB226E41928729063ADC7AE43529E61B563BBC606CC5E09`  
H:       `A41A41A12A799548211C410C65D8133AFDE34D28BDD542E4B680CF2899C8A8C4`  
E:       `A41A41A12A799548211C410C65D8133AFDE34D28BDD542E4B680CF2899C8A8C4`  
Sinv:    `F63AFA3939902A4CA9F019CE77E5A59FB48E4CAA50EB9601EF02809E033F9057`  
U:       `B807BF3281DD13849958F444FD9AEA808D074C2C48EE8382F6C47A435389A17E`  
V:       `1777F73443A4D68C23D1FC4CB5F8B7F2554578EE87F04C253DF44EFD181C184C`  
Rprime.X:`2B42F576D07F4165FF65D1F3B1500F81E44C316F1F0B3EF57325B69ACA46104F`  
Rprime.Y:`3CE76603264661EA2F602DF7B4510BBC9ED939233C553EA5F42FB3F1338174B5`  
Rprime:  `2B42F576D07F4165FF65D1F3B1500F81E44C316F1F0B3EF57325B69ACA46104F`  


## Explanation
Signing using ECDSA on P-256 takes as **input**

- private key 𝑑 (the question's D), which is a 32-byte bytestring
- a message, which is bytestring  of 0 to 2^61−1 bytes
- a random number generator

and **outputs**

- a signature 𝑆=(𝑟,𝑠) consisting of 
    - an 𝑟 component (the question's R), which is a 32-byte bytestring
    - an 𝑠 component (the question's S), which is a 32-byte bytestring

**Verifying** a signature using ECDSA on P-256 takes as input
- a trusted public key 𝑄, which should be a point of curve P-256 other than the point at infinity. It was originally computed as 𝑑𝐺 during key generation. It is defined by its Cartesian coordinates
    - 𝑥𝑄 (the question's Qx), which is a 32-byte bytestring
    - 𝑦𝑄 (the question's Qy), which in the question is³ a 32-byte bytestring

- a message 𝑀
- the signature 𝑆=(𝑟,𝑠) in the form output by the signature process.

and outputs valid (if the message matches the one signed and there was no errors) or invalid (in all other cases except a successful forgery).

----

The question's message is the 27-character `Example of ECDSA with P-256` converted to bytestring per some unspecified convention, likely ASCII or UTF-8. Both yield the same 27-byte bytestring 𝑀 `4578616D706C65206F66204543445341207769746820502D323536`.  


Both signing and verification manipulate 𝑀 only to compute it's SHA-256 hash 𝐻 (the question's H), which is a 32-byte bytestring. It is converted to an integer 𝑒 (the question's E), which when using P-256 thus SHA-256 is² 𝐻.


**Signing is per SEC1 section 4.1.3. In a nutshell:**  

- Draw a secret random number 𝑘 (the question's K) in range [1,𝑛), where 𝑛 is the order of the curve P-256. It is critically important that 𝑘 is uniformly distributed on this interval and independent⁴ of other 𝑘.
- Compute the Elliptic Curve point 𝑅=𝑘𝐺 of the curve P-256, where 𝐺 is the generator point. 𝑅 has Cartesian coordinates (𝑥𝑅,𝑦𝑅) (the question's R_x and R_y), but only 𝑥𝑅 is needed.
- Compute 𝑟=𝑥𝑅mod𝑛 (the question's R). If 𝑟=0 something went wrong⁵, ⁶.
- Compute 𝑘−1 modulo 𝑛 (the question's Kinv), that is the integer in range [1,𝑛) with 𝑘𝑘−1−1 a multiple of 𝑛.
- Compute 𝑠=𝑘−1(𝑒+𝑟𝑑) mod 𝑛. If 𝑠=0, something went wrong⁵.
- Output (𝑟,𝑠).

CAUTION: Signing can be the **target of various attacks**, e.g. timing or other side channel, and fault injection. Mitigation of these attacks is difficult.

**Verification is per SEC1 section 4.1.4. In a nutshell:**

- Check that the point 𝑄 of coordinates (𝑥𝑄,𝑦𝑄) is an ordinary point of P-256; otherwise, output invalid.
- Check that 𝑟 and 𝑠 both are in range [1,𝑛); otherwise, output invalid
- Compute 𝑠−1 modulo 𝑛 (the question's Sinv), that is the integer in range [1,𝑛) with 𝑠𝑠−1−1 a multiple of 𝑛.
- Compute 𝑢1=𝑒𝑠−1mod𝑛 (the question's U)
- Compute 𝑢2=𝑟𝑠−1mod𝑛 (the question's V)
- Compute the Elliptic Curve point 𝑅=𝑢1𝐺+𝑢2𝑄 of the curve P-256, where 𝐺 is the generator point, and 𝑄 is as determined by the public key. 𝑅 has Cartesian coordinates (𝑥𝑅,𝑦𝑅) (the question's Rprime.X and Rprime.Y), but only 𝑥𝑅
is needed.
- If 𝑅 is the point at infinity, output invalid.
- If 𝑒mod𝑛≠𝑥𝑅mod𝑛 , output invalid.
- Output valid.

[Source](https://crypto.stackexchange.com/questions/80137/understanding-example-of-ecdsa-p256#80139)

Numbers: https://neuromancer.sk/std/nist/P-256