mod base64;

fn main() {
    let a: Vec<u8> = vec![1, 2, 3, 4, 5, 6, 3];
    let b = base64::encode(&a);
    println!("Hello, world! {:?}", b);
}
