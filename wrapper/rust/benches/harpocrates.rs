extern crate criterion;

use criterion::{black_box, criterion_group, criterion_main, Criterion};
use harpocrates::harpocrates::{decrypt, encrypt, generate_ilut, generate_lut};
use rand::prelude::*;

fn bench_harpocrates(c: &mut Criterion) {
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

    generate_lut(&mut lut);
    generate_ilut(&lut, &mut ilut);

    let mut rng = thread_rng();

    rng.fill_bytes(&mut txt);
    enc.fill(0u8);
    dec.fill(0u8);

    c.bench_function("harpocrates_encrypt", |b| {
        b.iter(|| {
            encrypt(black_box(&lut), black_box(&txt), black_box(&mut enc));
        });
    });

    c.bench_function("harpocrates_decrypt", |b| {
        b.iter(|| {
            decrypt(black_box(&ilut), black_box(&enc), black_box(&mut dec));
        });
    });
}

criterion_group!(harpocrates, bench_harpocrates);
criterion_main!(harpocrates);
