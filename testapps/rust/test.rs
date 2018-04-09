
// A single line comment

#[cfg(feature = "test")]
fn test() -> u8 {
    return 99;
}


/*
* a multi line comment
*/

fn main2() -> i32 {
    let mut a = 2;
    while a <= 10 {
        println!("Hello, world: {}!", a);
        a += 1;
    }
    return a;
}


fn main() {

    main2();
}

