
// A single line comment

#[cfg(feature = "test")]
fn test(a: i32, b: i32) -> u8 {
    return a+10+b;
}


fn testf(a: f32, b: f32) -> f32 {
    return a+b+10.0;
}

/*
* a multi line comment
*/

fn main2() -> i32 {
    let mut a = 2;
    let b = testf(1.3, 1.4);
    while a <= 10 {
        println!("Hello, world: {} {}!", a, b);
        a += 1;
    }
    return a;
}


fn main() {

    main2();
}

