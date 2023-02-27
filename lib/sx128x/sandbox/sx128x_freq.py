#!/usr/bin/env python3
# -*- encoding: utf-8 -*-

step = 52e6 / 2**18 # 52 MHz / 2**18
print("step =", step, "Hz")
print()

# умножение входного кода на шаг 52e6 / 2**18
def c2f(code):
  # c = 0...21651921
  cl = (code >>  0) & 0x3FFF # 0...16383
  ch = (code >> 14) & 0x7FF  # 0...1321
  cl *= 13 * 15625 # 0...3327796875
  ch *= 13 * 15625 # 0...268328125
  cl >>= 9;
  fl = (cl >> 1) + (cl & 1) # 0...3249802
  fh = (ch << 4) & 0xFFFFFFFF # 0...4293250000
  return fl + fh # 0...4296499801 Hz

# деление чатоты на шаг с округлением
def f2c(freq):
  #return round(freq / step)
  freq = round(freq)
  code = (freq // (13 * 15625)) << 11
  freq = (freq %  (13 * 15625)) << 11
  code += freq // (13 * 15625)
  return (code >> 1) + (code & 1)

freq = 2400e6
print("freq =", freq, "Hz")

code = f2c(freq)
print("code =", code)

freq = c2f(code)
print("freq =", freq, "Hz")

print()

freq = 2450e6
print("freq =", freq, "Hz")

code = f2c(freq)
print("code =", code)

freq = c2f(code)
print("freq =", freq, "Hz")

print()

freq = 2500 * 1000 * 1000
print("freq =", freq, "Hz")

code = f2c(freq)
print("code =", code)

freq = c2f(code)
print("freq =", freq, "Hz")

print()

freq = 2**32-1
print("freq =", freq, "Hz")

code = f2c(freq)
print("code =", code)

freq = c2f(code)
print("freq =", freq, "Hz")


