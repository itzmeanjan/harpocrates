mod harpocrates;

fn main() {
    let mut lut = [0u8; 256];
    let mut ilut = [0u8; 256];

    let mut txt = Vec::<u8>::with_capacity(16);
    let mut enc = Vec::<u8>::with_capacity(16);
    let mut dec = Vec::<u8>::with_capacity(16);

    unsafe {
        txt.set_len(16);
        enc.set_len(16);
        dec.set_len(16);
    }

    txt.fill(0xffu8);

    harpocrates::generate_lut(&mut lut);
    harpocrates::generate_ilut(&lut, &mut ilut);

    harpocrates::encrypt(&lut, &txt, &mut enc);
    harpocrates::decrypt(&ilut, &enc, &mut dec);

    println!("Text      = {:x?}", txt);
    println!("Encrypted = {:x?}", enc);
    println!("Decrypted = {:x?}", dec);
}
