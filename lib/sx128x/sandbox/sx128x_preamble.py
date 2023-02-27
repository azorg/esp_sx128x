#!/usr/bin/env python3
# -*- encoding: utf-8 -*-

# проверка алгоритма упаковки длины преамбулы в 1 байт
# (мантисcа + экспонента)
def pack_preamble(preamble):
    mant = preamble
    exp = 0

    if 0:
        mant >>= 1
        exp += 1

    while mant > 15:
        mant >>= 1
        exp += 1
        if exp >= 15: break

    if mant > 15: mant = 15

    preamble = mant * (1 << exp)
    pack = (exp << 4) | mant
    return mant, exp, pack, preamble


full_range = tuple(range(32)) + \
             tuple(range(100, 1000, 100)) + \
             tuple(range(1000, 10000, 1000)) + \
             tuple(range(10000, 500000, 10000))


for p in full_range:
    preamble = pack_preamble(p)
    print("mant=%i exp=%i pack=0x%02X preamble=%i <= p=%i" % (preamble + (p,)))

