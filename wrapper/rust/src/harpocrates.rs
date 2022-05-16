extern "C" {
    fn generate_harpocrates_lut(lut: *mut u8);
    fn generate_harpocrates_ilut(lut: *const u8, ilut: *mut u8);
    fn harpocrates_encrypt(lut: *const u8, txt: *const u8, enc: *mut u8);
    fn harpocrates_decrypt(ilut: *const u8, enc: *const u8, dec: *mut u8);
}

/// Generates Harpocrates look up table of size 256 -bytes
///
/// Ensure that len(lut) == 256
pub fn generate_lut(lut: &mut [u8]) {
    assert_eq!(lut.len(), 256);
    unsafe {
        generate_harpocrates_lut(lut.as_mut_ptr());
    }
}

/// Computes Harpocrates inverse look up table of size 256 -bytes, from equal sized look up table
///
/// Ensure that len(lut) == len(ilut) == 256
pub fn generate_ilut(lut: &[u8], ilut: &mut [u8]) {
    assert_eq!(lut.len(), 256);
    assert_eq!(ilut.len(), 256);
    unsafe {
        generate_harpocrates_ilut(lut.as_ptr(), ilut.as_mut_ptr());
    }
}

/// Given 256 -bytes look up table & 16 -bytes plain text, this routine computes 16 -bytes cipher data
///
/// Ensure that len(lut) == 256 && len(txt) == len(enc) == 16
pub fn encrypt(lut: &[u8], txt: &[u8], enc: &mut [u8]) {
    assert_eq!(lut.len(), 256);
    assert_eq!(txt.len(), 16);
    assert_eq!(enc.len(), 16);
    unsafe {
        harpocrates_encrypt(lut.as_ptr(), txt.as_ptr(), enc.as_mut_ptr());
    }
}

/// Given 256 -bytes inverse look up table & 16 -bytes encrypted data, this routine computes 16 -bytes decrypted data
///
/// Ensure that len(ilut) == 256 && len(enc) == len(dec) == 16
pub fn decrypt(ilut: &[u8], enc: &[u8], dec: &mut [u8]) {
    assert_eq!(ilut.len(), 256);
    assert_eq!(enc.len(), 16);
    assert_eq!(dec.len(), 16);
    unsafe {
        harpocrates_decrypt(ilut.as_ptr(), enc.as_ptr(), dec.as_mut_ptr());
    }
}

mod test {
    #[test]
    fn harpocrates_cipher() {
        use crate::harpocrates::*;
        use rand::prelude::*;

        let mut lut = Vec::<u8>::with_capacity(256);
        let mut ilut = Vec::<u8>::with_capacity(256);
        let mut txt = Vec::<u8>::with_capacity(16);
        let mut enc = Vec::<u8>::with_capacity(16);
        let mut dec = Vec::<u8>::with_capacity(16);

        unsafe {
            lut.set_len(256);
            ilut.set_len(256);
            txt.set_len(16);
            enc.set_len(16);
            dec.set_len(16);
        }

        generate_lut(&mut lut);
        generate_ilut(&lut, &mut ilut);

        let mut rng = thread_rng();

        rng.fill_bytes(&mut txt);
        enc.fill(0u8);
        dec.fill(0u8);

        encrypt(&lut, &txt, &mut enc);
        decrypt(&ilut, &enc, &mut dec);

        for i in 0..16 {
            assert_eq!(txt[i], dec[i]);
        }
    }
}
