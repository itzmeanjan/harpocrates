extern crate criterion;

use criterion::{black_box, criterion_group, criterion_main, Criterion, Throughput};
use harpocrates::harpocrates::{
    decrypt, decrypt_data, encrypt, encrypt_data, generate_ilut, generate_lut, par_decrypt_data,
    par_encrypt_data,
};
use rand::prelude::*;

fn bench_harpocrates_basic(c: &mut Criterion) {
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

    let mut grp = c.benchmark_group("harpocrates");
    grp.throughput(Throughput::Bytes(16));

    grp.bench_function("encrypt", |b| {
        b.iter(|| {
            encrypt(black_box(&lut), black_box(&txt), black_box(&mut enc));
        });
    });

    grp.bench_function("decrypt", |b| {
        b.iter(|| {
            decrypt(black_box(&ilut), black_box(&enc), black_box(&mut dec));
        });
    });

    grp.finish();
}

fn to_readable_data_size(b_len: usize) -> String {
    const GB: f64 = 1073741824.; // 1 << 30 bytes
    const MB: f64 = 1048576.; // 1 << 20 bytes
    const KB: f64 = 1024.; // 1 << 10 bytes

    let b_len_ = b_len as f64;

    if b_len_ >= GB {
        format!("{} GB", b_len_ / GB)
    } else if b_len_ >= MB {
        format!("{} MB", b_len_ / MB)
    } else if b_len_ >= KB {
        format!("{} KB", b_len_ / KB)
    } else {
        format!("{} B", b_len_)
    }
}

fn bench_harpocrates_with_data_chunks(c: &mut Criterion) {
    let mut lut = [0u8; 256];
    let mut ilut = [0u8; 256];

    generate_lut(&mut lut);
    generate_ilut(&lut, &mut ilut);

    let mut rng = thread_rng();
    let mut grp = c.benchmark_group("harpocrates_with_data_chunks");
    grp.sample_size(10);

    const CT_LEN: [usize; 3] = [1 << 24, 1 << 25, 1 << 26];

    for ct_len in CT_LEN {
        let mut txt = Vec::<u8>::with_capacity(ct_len);
        let mut enc = Vec::<u8>::with_capacity(ct_len);
        let mut dec = Vec::<u8>::with_capacity(ct_len);

        unsafe {
            txt.set_len(ct_len);
            enc.set_len(ct_len);
            dec.set_len(ct_len);
        }

        rng.fill_bytes(&mut txt);
        enc.fill(0u8);
        dec.fill(0u8);

        grp.throughput(Throughput::Bytes(ct_len as u64));

        grp.bench_function(format!("encrypt/{}", to_readable_data_size(ct_len)), |b| {
            b.iter(|| {
                encrypt_data(black_box(&lut), black_box(&txt), black_box(&mut enc));
            });
        });
        grp.bench_function(format!("decrypt/{}", to_readable_data_size(ct_len)), |b| {
            b.iter(|| {
                decrypt_data(black_box(&ilut), black_box(&enc), black_box(&mut dec));
            });
        });
    }

    grp.finish();
}

fn bench_par_harpocrates_with_data_chunks(c: &mut Criterion) {
    let mut lut = [0u8; 256];
    let mut ilut = [0u8; 256];

    generate_lut(&mut lut);
    generate_ilut(&lut, &mut ilut);

    let mut rng = thread_rng();
    let mut grp = c.benchmark_group("par_harpocrates_with_data_chunks");
    grp.sample_size(10);

    const CT_LEN: [usize; 3] = [1 << 24, 1 << 25, 1 << 26];

    for ct_len in CT_LEN {
        let mut txt = Vec::<u8>::with_capacity(ct_len);
        let mut enc = Vec::<u8>::with_capacity(ct_len);
        let mut dec = Vec::<u8>::with_capacity(ct_len);

        unsafe {
            txt.set_len(ct_len);
            enc.set_len(ct_len);
            dec.set_len(ct_len);
        }

        rng.fill_bytes(&mut txt);
        enc.fill(0u8);
        dec.fill(0u8);

        grp.throughput(Throughput::Bytes(ct_len as u64));

        grp.bench_function(format!("encrypt/{}", to_readable_data_size(ct_len)), |b| {
            b.iter(|| {
                par_encrypt_data(black_box(&lut), black_box(&txt), black_box(&mut enc));
            });
        });
        grp.bench_function(format!("decrypt/{}", to_readable_data_size(ct_len)), |b| {
            b.iter(|| {
                par_decrypt_data(black_box(&ilut), black_box(&enc), black_box(&mut dec));
            });
        });
    }

    grp.finish();
}

criterion_group!(
    harpocrates,
    bench_harpocrates_basic,
    bench_harpocrates_with_data_chunks,
    bench_par_harpocrates_with_data_chunks
);
criterion_main!(harpocrates);
